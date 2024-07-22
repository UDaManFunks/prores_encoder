#include "prores_encoder.h"
#include "prores_worker.h"

#include <filesystem>

#pragma comment(lib, "Dxva2.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "evr.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfplay.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")

ProResWorker::ProResWorker(uint32_t ColorModel, std::string ProfileValue, HostCodecConfigCommon CommonProps, AVPixelFormat PixelFormat, int32_t BitsPerSample)
{
	m_pContext = nullptr;
	m_pPkt = nullptr;
	m_pOutFrame = nullptr;
	m_pInFrame = nullptr;
	m_pSwsContext = nullptr;
	m_Error = errNone;

	char logMessagePrefix[] = "ProResWorker :: ()";

	g_Log(logLevelInfo, "%s :: address of this = %I64x", logMessagePrefix, this);

	m_ColorModel = ColorModel;
	m_sProfileValue = ProfileValue;
	m_Width = CommonProps.GetWidth();
	m_Height = CommonProps.GetHeight();
	m_iFrameRate = CommonProps.GetFrameRateNum();
	m_IsFullRange = CommonProps.IsFullRange();
	m_PixelFormat = PixelFormat;
	m_iBitDepth = BitsPerSample;

	if (m_ColorModel == clrAYUV) {
		m_InPixelFormat = AV_PIX_FMT_AYUV64LE;
	} else {
		m_InPixelFormat = AV_PIX_FMT_YUV422P16LE;
	}

	const AVCodec* m_pEncoder = avcodec_find_encoder_by_name("prores_ks");

	m_pContext = avcodec_alloc_context3(m_pEncoder);

	const struct AVRational timeBase = { 1, (int)m_iFrameRate };
	const struct AVRational frameRate = { (int)m_iFrameRate, 1 };

	m_pContext->pix_fmt = m_PixelFormat;
	m_pContext->width = m_Width;
	m_pContext->height = m_Height;
	m_pContext->time_base = timeBase;
	m_pContext->framerate = frameRate;

	av_opt_set(m_pContext->priv_data, "profile", m_sProfileValue.c_str(), 0);
	av_opt_set(m_pContext->priv_data, "vendor", "apl0", 0);

	m_pSwsContext = sws_alloc_context();

	av_opt_set(m_pContext, "out_color_matrix", "bt709", 0);
	av_opt_set_int(m_pSwsContext, "srcw", m_Width, 0);
	av_opt_set_int(m_pSwsContext, "srch", m_Height, 0);
	av_opt_set_int(m_pSwsContext, "dstw", m_Width, 0);
	av_opt_set_int(m_pSwsContext, "dsth", m_Height, 0);
	av_opt_set_int(m_pSwsContext, "src_range", m_IsFullRange, 0);
	av_opt_set_int(m_pSwsContext, "src_format", m_InPixelFormat, 0);
	av_opt_set_int(m_pSwsContext, "dst_range", m_IsFullRange, 0);
	av_opt_set_int(m_pSwsContext, "dst_format", m_PixelFormat, 0);

	if (sws_setColorspaceDetails(m_pSwsContext, sws_getCoefficients(m_InPixelFormat), m_IsFullRange, sws_getCoefficients(m_PixelFormat), m_IsFullRange, 0, 1 << 16, 1 << 16) < 0) {
		m_Error = errNoCodec;
		return;
	}

	if (sws_init_context(m_pSwsContext, nullptr, nullptr) < 0) {
		m_Error = errNoCodec;
		return;
	}

	if (avcodec_open2(m_pContext, m_pEncoder, NULL) < 0) {
		g_Log(logLevelError, "ProResWorker :: SetupContext :: failed to open encoder context");
		m_Error = errNoCodec;
		return;
	}

	m_pPkt = av_packet_alloc();

	m_pInFrame = av_frame_alloc();
	m_pInFrame->format = m_InPixelFormat;
	m_pInFrame->width = m_Width;
	m_pInFrame->height = m_Height;

	m_pOutFrame = av_frame_alloc();
	m_pOutFrame->format = m_PixelFormat;
	m_pOutFrame->width = m_Width;
	m_pOutFrame->height = m_Height;

	if (av_frame_get_buffer(m_pOutFrame, 0) < 0) {
		g_Log(logLevelError, "ProResWorker :: SetupContext :: failed to allocate OUT frame buffer");
		m_Error = errAlloc;
		return;
	}

}

ProResWorker::~ProResWorker()
{
	if (m_pOutFrame != nullptr) {
		av_frame_free(&m_pOutFrame);
		m_pOutFrame = nullptr;
	}

	if (m_pInFrame != nullptr) {
		av_frame_free(&m_pInFrame);
		m_pInFrame = nullptr;
	}

	if (m_pPkt != nullptr) {
		av_packet_free(&m_pPkt);
		m_pPkt = nullptr;
	}

	if (m_pSwsContext != nullptr) {
		sws_freeContext(m_pSwsContext);
		m_pSwsContext = nullptr;
	}

	if (m_pContext != nullptr) {
		avcodec_free_context(&m_pContext);
		m_pContext = nullptr;
	}

}


StatusCode ProResWorker::EncodeFrame(HostBufferRef* p_pBuff, HostCodecCallbackRef* pCallback)
{
	char logMessagePrefix[] = "ProResWorker :: EncodeFrame";
	int encoderRet = 0;
	int64_t pts = -1;

	if (m_Error != errNone) {
		return m_Error;
	}

	try {

		if (p_pBuff == NULL || !p_pBuff->IsValid()) {
			g_Log(logLevelInfo, "%s :: trying to flush", logMessagePrefix);
			encoderRet = avcodec_send_frame(m_pContext, NULL);
		} else {

			char* pBuf = NULL;
			size_t bufSize = 0;

			if (av_frame_make_writable(m_pOutFrame) < 0) {
				g_Log(logLevelError, "%s%s", logMessagePrefix, " :: failed to make OUT frame writeable");
				throw errFail;
			}

			if (!p_pBuff->LockBuffer(&pBuf, &bufSize)) {
				g_Log(logLevelError, "%s%s", logMessagePrefix, " :: failed to lock the buffer");
				throw errFail;
			}

			if (pBuf == NULL || bufSize == 0) {
				g_Log(logLevelError, "%s%s", logMessagePrefix, " :: no data to encode");
				p_pBuff->UnlockBuffer();
				throw errUnsupported;
			}

			uint32_t width = 0;
			uint32_t height = 0;

			if (!p_pBuff->GetUINT32(pIOPropWidth, width) || !p_pBuff->GetUINT32(pIOPropHeight, height)) {
				g_Log(logLevelError, "%s%s", logMessagePrefix, " :: width / height not set when encoding the frame");
				throw errNoParam;
			}

			// COLORSPACE CONVERSION: clrAYUV (16-bit) -> YUV422P10LE (10-bit)

			uint8_t* pSrc = reinterpret_cast<uint8_t*>(const_cast<char*>(pBuf));

			if (av_image_fill_arrays(m_pInFrame->data, m_pInFrame->linesize, pSrc, m_InPixelFormat, width, height, 1) < 0) {
				g_Log(logLevelError, "%s%s", logMessagePrefix, " :: failed to populate INFRAME");
				throw errFail;
			};

			if (sws_scale_frame(m_pSwsContext, m_pOutFrame, m_pInFrame) < 0) {
				g_Log(logLevelError, "%s%s", logMessagePrefix, " :: failed to PIXELFMT convert IN to OUT");
				throw errFail;
			}

			// done with COLORSAPCE CONVERSION

			if (!p_pBuff->GetINT64(pIOPropPTS, pts)) {
				g_Log(logLevelError, "%s%s", logMessagePrefix, " :: pts not set when encoding the frame");
				throw errNoParam;
			}

			m_pOutFrame->pts = pts;

			encoderRet = avcodec_send_frame(m_pContext, m_pOutFrame);

			p_pBuff->UnlockBuffer();

		}

		if (encoderRet < 0) {
			g_Log(logLevelError, "%s%s", logMessagePrefix, " :: frame submission failed");
			throw errFail;
		}

		encoderRet = avcodec_receive_packet(m_pContext, m_pPkt);

		if (encoderRet == AVERROR(EAGAIN)) {
			av_packet_unref(m_pPkt);
			g_Log(logLevelError, "%s%s", logMessagePrefix, " :: AVERROR(EAGAIN)");
			return errMoreData;
		}

		if (encoderRet == AVERROR_EOF) {
			av_packet_unref(m_pPkt);
			g_Log(logLevelInfo, "%s%s", logMessagePrefix, " :: AVERROR_EOF");
			return errNone;
		}

		if (encoderRet < 0) {
			av_packet_unref(m_pPkt);
			g_Log(logLevelError, "%s%s", logMessagePrefix, " :: AVERROR");
			throw errFail;
		}

		HostBufferRef outBuf(false);
		if (!outBuf.IsValid() || !outBuf.Resize(m_pPkt->size)) {
			av_packet_unref(m_pPkt);
			g_Log(logLevelError, "%s%s", logMessagePrefix, " :: could not resize output buffer");
			throw errAlloc;
		}

		char* pOutBuf = NULL;
		size_t outBufSize = 0;

		if (!outBuf.LockBuffer(&pOutBuf, &outBufSize)) {
			av_packet_unref(m_pPkt);
			g_Log(logLevelError, "%s%s", logMessagePrefix, " :: could not lock output buffer");
			throw errAlloc;
		}

		memcpy(pOutBuf, m_pPkt->data, m_pPkt->size);

		av_packet_unref(m_pPkt);

		int64_t ePts = m_pPkt->pts;

		outBuf.SetProperty(pIOPropPTS, propTypeInt64, &ePts, 1);

		int64_t eDts = m_pPkt->pts + 1;

		outBuf.SetProperty(pIOPropDTS, propTypeInt64, &eDts, 1);

		uint8_t isKeyFrame = 1;
		outBuf.SetProperty(pIOPropIsKeyFrame, propTypeUInt8, &isKeyFrame, 1);

		return pCallback->SendOutput(&outBuf);

	}
	catch (StatusCode errorCode) {

		g_Log(logLevelError, "%s :: caught an exception :: errCode = %d", logMessagePrefix, errorCode);

		m_Error = errorCode;
		return m_Error;
	}

}
