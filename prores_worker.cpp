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

ProResWorker::ProResWorker(std::string sProfileValue, AVPixelFormat pixelFormat, int iWidth, int iHeight, int iFrameRate, int iBitDepth)
{

	std::string logMessagePrefix = "ProResWorker :: Contructor";
	std::ostringstream logMessage;

	m_sProfileValue = sProfileValue;
	m_PixelFormat = pixelFormat;
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iFrameRate = iFrameRate;
	m_iBitDepth = iBitDepth;

	StatusCode m_Error = errNone;
	AVCodecContext* m_pContext = NULL;
	AVPacket* m_pPkt = NULL;
	AVFrame* m_pOutFrame = NULL;
	AVFrame* m_pInFrame = NULL;
	SwsContext* m_pSwsContext = NULL;

}

ProResWorker::~ProResWorker()
{
	if (m_pContext != NULL) {
		avcodec_free_context(&m_pContext);
	}

	if (m_pInFrame != NULL) {
		av_frame_free(&m_pInFrame);
	}

	if (m_pOutFrame != NULL) {
		av_frame_free(&m_pOutFrame);
	}

	if (m_pPkt != NULL) {
		av_packet_free(&m_pPkt);
	}

	if (m_pSwsContext != NULL) {
		sws_freeContext(m_pSwsContext);
	}
}

void ProResWorker::SetupContext(HostBufferRef* p_pBuff)
{

	const AVCodec* encoder = avcodec_find_encoder_by_name("prores_ks");
	m_pContext = avcodec_alloc_context3(encoder);


	const struct AVRational timeBase = { 1, m_iFrameRate };
	const struct AVRational frameRate = { m_iFrameRate, 1 };

	m_pContext->pix_fmt = m_PixelFormat;
	m_pContext->width = m_iWidth;
	m_pContext->height = m_iHeight;
	m_pContext->time_base = timeBase;
	m_pContext->framerate = frameRate;

	std::string encoderProfileVal = m_sProfileValue;

	av_opt_set(m_pContext->priv_data, "profile", encoderProfileVal.c_str(), 0);
	av_opt_set(m_pContext->priv_data, "vendor", "apl0", 0);

	m_pSwsContext = sws_getContext(m_iWidth, m_iHeight, AV_PIX_FMT_RGB48LE, m_iWidth, m_iHeight, m_PixelFormat, SWS_POINT, NULL, NULL, NULL);

	m_pPkt = av_packet_alloc();

	if (avcodec_open2(m_pContext, encoder, NULL) < 0) {
		g_Log(logLevelError, "ProResWorker :: SetupContext :: failed to open encoder context");
		m_Error = errNoCodec;
		return;
	}

	m_pInFrame = av_frame_alloc();
	m_pInFrame->format = AV_PIX_FMT_RGB48LE;
	m_pInFrame->width = m_iWidth;
	m_pInFrame->height = m_iHeight;

	if (av_frame_get_buffer(m_pInFrame, 0) < 0) {
		g_Log(logLevelError, "ProResoneWorker :: SetupContext :: failed to allocate IN frame buffer");
		m_Error = errAlloc;
		return;
	}

	m_pOutFrame = av_frame_alloc();
	m_pOutFrame->format = m_PixelFormat;
	m_pOutFrame->width = m_iWidth;
	m_pOutFrame->height = m_iHeight;

	if (av_frame_get_buffer(m_pOutFrame, 0) < 0) {
		g_Log(logLevelError, "ProResWorker :: SetupContext :: failed to allocate OUT frame buffer");
		m_Error = errAlloc;
		return;
	}

	m_Error = errNone;

}

StatusCode ProResWorker::EncodeFrame(HostBufferRef* p_pBuff, HostCodecCallbackRef* pCallback)
{
	int encoderRet = 0;
	std::string logMessagePrefix = "ProResWorker :: EncodeFrame";
	std::ostringstream logMessage;

	SetupContext(p_pBuff);

	if (m_Error != errNone) {
		return m_Error;
	}

	try {

		if (p_pBuff == NULL || !p_pBuff->IsValid()) {

			g_Log(logLevelInfo, "ProResWorker :: EncodeFrame :: trying to flush");

			encoderRet = avcodec_send_frame(m_pContext, NULL);

		} else {

			char* pBuf = NULL;
			size_t bufSize = 0;

			if (av_frame_make_writable(m_pInFrame) < 0) {
				g_Log(logLevelError, "ProResWorker :: EncodeFrame :: failed to make IN frame writeable");
				throw errFail;
			}

			if (av_frame_make_writable(m_pOutFrame) < 0) {
				g_Log(logLevelError, "ProResWorker :: EncodeFrame :: failed to make OUT frame writeable");
				throw errFail;
			}

			if (!p_pBuff->LockBuffer(&pBuf, &bufSize)) {
				g_Log(logLevelError, "ProResWorker :: EncodeFrame :: failed to lock the buffer");
				throw errFail;
			}

			if (pBuf == NULL || bufSize == 0) {
				g_Log(logLevelError, "ProResWorker :: EncodeFrame :: no data to encode");
				p_pBuff->UnlockBuffer();
				throw errUnsupported;
			}

			uint32_t width = 0;
			uint32_t height = 0;

			if (!p_pBuff->GetUINT32(pIOPropWidth, width) || !p_pBuff->GetUINT32(pIOPropHeight, height)) {
				g_Log(logLevelError, "ProResWorker :: EncodeFrame :: width/height not set when encoding the frame");
				throw errNoParam;
			}

			// COLORSPACE CONVERSION: clrRGB (16-bit) -> YUV422P10LE (10-bit)

			uint8_t* pSrc = reinterpret_cast<uint8_t*>(const_cast<char*>(pBuf));

			memcpy(m_pInFrame->data[0], pSrc, bufSize);

			if (sws_scale_frame(m_pSwsContext, m_pOutFrame, m_pInFrame) < 0) {
				g_Log(logLevelError, "ProResWorker :: EncodeFrame :: failed to PIXELFMT convert IN to OUT");
				throw errFail;
			}

			// done with COLORSAPCE CONVERSION

			av_frame_free(&m_pInFrame);
			m_pInFrame = NULL;

			int64_t pts = -1;

			if (!p_pBuff->GetINT64(pIOPropPTS, pts)) {
				g_Log(logLevelError, "ProResWorker :: EncodeFrame :: pts not set when encoding the frame");
				throw errNoParam;
			}

			m_pOutFrame->pts = pts;

			encoderRet = avcodec_send_frame(m_pContext, m_pOutFrame);

			p_pBuff->UnlockBuffer();

		}

		if (encoderRet < 0) {
			logMessage.str("");
			logMessage.clear();
			logMessage << logMessagePrefix << " :: EncodeFrame :: frame submission failed";
			g_Log(logLevelError, logMessage.str().c_str());

			throw errFail;
		}

		encoderRet = avcodec_receive_packet(m_pContext, m_pPkt);

		if (encoderRet == AVERROR(EAGAIN)) {
			g_Log(logLevelError, "ProResWorker :: EncodeFrame :: AVERROR(EAGAIN)");
			return errMoreData;
		}

		if (encoderRet == AVERROR_EOF) {
			av_packet_unref(m_pPkt);
			g_Log(logLevelInfo, "ProResWorker :: EncodeFrame :: AVERROR_EOF");
			return errNone;
		}

		if (encoderRet < 0) {
			g_Log(logLevelError, "ProResWorker :: EncodeFrame :: AVERROR");
			throw errFail;
		}

		HostBufferRef outBuf(false);
		if (!outBuf.IsValid() || !outBuf.Resize(m_pPkt->size)) {
			g_Log(logLevelError, "ProResWorker :: EncodeFrame :: could not resize output buffer");
			throw errAlloc;
		}

		char* pOutBuf = NULL;
		size_t outBufSize = 0;

		if (!outBuf.LockBuffer(&pOutBuf, &outBufSize)) {
			g_Log(logLevelError, "ProResWorker :: EncodeFrame :: could not lock output buffer");
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

		{
			logMessage.str("");
			logMessage.clear();
			logMessage << logMessagePrefix << " :: caught an exception" << " :: errorCode = " << errorCode;
			g_Log(logLevelError, logMessage.str().c_str());
		}

		m_Error = errorCode;
		return m_Error;
	}

}

std::string ProResWorker::ConvertUINT8ToHexStr(const uint8_t* v, const size_t s) {

	std::stringstream ss;

	ss << std::hex << std::setfill('0');

	for (int i = 0; i < s; i++) {
		ss << std::hex << std::setw(2) << static_cast<int>(v[i]);
	}

	return ss.str();
}

