#include "prores_encoder.h"

#include <assert.h>
#include <cstring>
#include <vector>
#include <stdint.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
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

const uint8_t ProResEncoder::s_UUID[] = { 0x21, 0x42, 0xe8, 0x41, 0xd8, 0xe4, 0x41, 0x4b, 0x87, 0x9e, 0xa4, 0x80, 0xfc, 0x90, 0xda, 0xb5 };
const ProfileMap ProResEncoder::s_ProfileMap[4] = { {'apco', AV_PIX_FMT_YUV422P10LE , "Apple ProRes 422 (Proxy)" }, {'apcs', AV_PIX_FMT_YUV422P10LE , "Apple ProRes 422 (LT)"}, {'apcn', AV_PIX_FMT_YUV422P10LE , "Apple ProRes 422"}, {'apch', AV_PIX_FMT_YUV422P10LE, "Apple ProRes 422 (HQ)"} };

class UISettingsController
{
public:
	UISettingsController()
	{
		InitDefaults();
	}

	explicit UISettingsController(const HostCodecConfigCommon& p_CommonProps)
		: m_CommonProps(p_CommonProps)
	{
		InitDefaults();
	}

	~UISettingsController()
	{
	}

	void Load(IPropertyProvider* p_pValues)
	{
		uint8_t val8 = 0;
		p_pValues->GetUINT8("prores_reset", val8);
		if (val8 != 0) {
			*this = UISettingsController();
			return;
		}

		p_pValues->GetINT32("prores_profile", m_Profile);
		p_pValues->GetString("prores_enc_markers", m_MarkerColor);
	}

	StatusCode Render(HostListRef* p_pSettingsList)
	{
		StatusCode err = RenderGeneral(p_pSettingsList);
		if (err != errNone) {
			return err;
		}

		{
			HostUIConfigEntryRef item("prores_separator");
			item.MakeSeparator();
			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "ProRes Plugin :: Failed to add a separator entry");
				return errFail;
			}
		}

		err = RenderQuality(p_pSettingsList);
		if (err != errNone) {
			return err;
		}

		{
			HostUIConfigEntryRef item("prores_reset");
			item.MakeButton("Reset");
			item.SetTriggersUpdate(true);
			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "ProRes Plugin :: Failed to populate the button entry");
				return errFail;
			}
		}

		return errNone;
	}

private:
	void InitDefaults()
	{
		m_Profile = 2;
	}

	StatusCode RenderGeneral(HostListRef* p_pSettingsList)
	{
		if (0) {
			HostUIConfigEntryRef item("prores_lbl_general");
			item.MakeLabel("General Settings");

			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "ProRes Plugin :: Failed to populate general label entry");
				return errFail;
			}
		}

		// Markers selection
		if (m_CommonProps.GetContainer().size() >= 32) {
			HostUIConfigEntryRef item("prores_enc_markers");
			item.MakeMarkerColorSelector("Chapter Marker", "Marker 1", m_MarkerColor);
			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "ProRes Plugin :: Failed to populate encoder preset UI entry");
				assert(false);
				return errFail;
			}
		}

		// Profile combobox
		{
			HostUIConfigEntryRef item("prores_profile");

			std::vector<std::string> textsVec;
			std::vector<int> valuesVec;

			for (int i = 0; i < 4; i++) {
				valuesVec.push_back(i);
				textsVec.push_back(ProResEncoder::s_ProfileMap[i].ProfileName);
			}

			item.MakeComboBox("Encoder Profile", textsVec, valuesVec, m_Profile);
			if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
				g_Log(logLevelError, "ProRes Plugin :: Failed to populate profile UI entry");
				return errFail;
			}
		}

		return errNone;
	}

	StatusCode RenderQuality(HostListRef* p_pSettingsList)
	{

		return errNone;
	}

public:
	const ProfileMap GetProfile() const
	{
		return ProResEncoder::s_ProfileMap[m_Profile];
	}

	int32_t GetBitDepth() const
	{
		return 10;
	}

	const std::string& GetMarkerColor() const
	{
		return m_MarkerColor;
	}

private:
	HostCodecConfigCommon m_CommonProps;
	std::string m_MarkerColor;
	int32_t m_Profile;
};

StatusCode ProResEncoder::s_GetEncoderSettings(HostPropertyCollectionRef* p_pValues, HostListRef* p_pSettingsList)
{
	HostCodecConfigCommon commonProps;
	commonProps.Load(p_pValues);

	UISettingsController settings(commonProps);
	settings.Load(p_pValues);

	return settings.Render(p_pSettingsList);
}

StatusCode ProResEncoder::s_RegisterCodecs(HostListRef* p_pList)
{

	std::string logMessagePrefix = "ProRes Plugin :: s_RegisterCodecs :: ";
	std::ostringstream logMessage;

	{
		logMessage << logMessagePrefix;
		g_Log(logLevelInfo, logMessage.str().c_str());
	}

	HostPropertyCollectionRef codecInfo;
	if (!codecInfo.IsValid()) {
		return errAlloc;
	}

	codecInfo.SetProperty(pIOPropUUID, propTypeUInt8, ProResEncoder::s_UUID, 16);

	const char* pCodecName = "Auto";
	codecInfo.SetProperty(pIOPropName, propTypeString, pCodecName, strlen(pCodecName));

	const char* pCodecGroup = "ProRes";
	codecInfo.SetProperty(pIOPropGroup, propTypeString, pCodecGroup, strlen(pCodecGroup));

	uint32_t vFourCC = 'apcn';
	codecInfo.SetProperty(pIOPropFourCC, propTypeUInt32, &vFourCC, 1);

	uint32_t vMediaVideo = mediaVideo;
	codecInfo.SetProperty(pIOPropMediaType, propTypeUInt32, &vMediaVideo, 1);

	uint32_t vDirection = dirEncode;
	codecInfo.SetProperty(pIOPropCodecDirection, propTypeUInt32, &vDirection, 1);

	uint32_t vColorModel = clrYUVp;
	codecInfo.SetProperty(pIOPropColorModel, propTypeUInt32, &vColorModel, 1);

	uint8_t hSampling = 2;
	uint8_t vSampling = 2;
	codecInfo.SetProperty(pIOPropHSubsampling, propTypeUInt8, &hSampling, 1);
	codecInfo.SetProperty(pIOPropVSubsampling, propTypeUInt8, &vSampling, 1);

	// Optionally enable both Data Ranges, Video will be default for "Auto" thus "0" value goes first
	std::vector<uint8_t> dataRangeVec;
	dataRangeVec.push_back(0);
	dataRangeVec.push_back(1);
	codecInfo.SetProperty(pIOPropDataRange, propTypeUInt8, dataRangeVec.data(), dataRangeVec.size());

	uint32_t vBitDepth = 10;
	codecInfo.SetProperty(pIOPropBitDepth, propTypeUInt32, &vBitDepth, 1);

	vBitDepth = 16;
	codecInfo.SetProperty(pIOPropBitsPerSample, propTypeUInt32, &vBitDepth, 1);

	const uint8_t fieldSupport = (fieldProgressive | fieldTop | fieldBottom);
	codecInfo.SetProperty(pIOPropFieldOrder, propTypeUInt8, &fieldSupport, 1);

	std::vector<std::string> containerVec;
	containerVec.push_back("mov");
	std::string valStrings;
	for (size_t i = 0; i < containerVec.size(); ++i) {
		valStrings.append(containerVec[i]);
		if (i < (containerVec.size() - 1)) {
			valStrings.append(1, '\0');
		}
	}

	codecInfo.SetProperty(pIOPropContainerList, propTypeString, valStrings.c_str(), valStrings.size());

	if (!p_pList->Append(&codecInfo)) {
		return errFail;
	}

	return errNone;
}

ProResEncoder::ProResEncoder()
	: m_ColorModel(-1)
	, m_Error(errNone)
	, m_pContext(NULL)
	, m_pPkt(NULL)
	, m_pFrame(NULL)
	, m_bFlushed(false)
	, m_pSwsContext(NULL)
{

}

ProResEncoder::~ProResEncoder()
{

}

StatusCode ProResEncoder::DoInit(HostPropertyCollectionRef* p_pProps)
{
	g_Log(logLevelInfo, "ProRes Plugin :: DoInit ::");

	return errNone;
}

StatusCode ProResEncoder::DoOpen(HostBufferRef* p_pBuff)
{

	std::string logMessagePrefix = "ProRes Plugin :: DoOpen :: ";
	std::ostringstream logMessage;

	{
		logMessage << logMessagePrefix;
		g_Log(logLevelInfo, logMessage.str().c_str());
	}

	m_CommonProps.Load(p_pBuff);

	m_pSettings.reset(new UISettingsController(m_CommonProps));
	m_pSettings->Load(p_pBuff);

	int32_t vFourCC = m_pSettings->GetProfile().FourCC;

	{
		logMessage.str("");
		logMessage.clear();
		logMessage << logMessagePrefix << "vFourCC = " << vFourCC;
		g_Log(logLevelInfo, logMessage.str().c_str());
	}

	p_pBuff->SetProperty(pIOPropFourCC, propTypeUInt32, &vFourCC, 1);

	uint8_t vBitDepth = 10;
	p_pBuff->SetProperty(pIOPropBitDepth, propTypeUInt32, &vBitDepth, 1);

	vBitDepth = 16;
	p_pBuff->SetProperty(pIOPropBitsPerSample, propTypeUInt32, &vBitDepth, 1);

	uint8_t isMultiPass = 0;
	StatusCode sts = p_pBuff->SetProperty(pIOPropMultiPass, propTypeUInt8, &isMultiPass, 1);
	if (sts != errNone) {
		return sts;
	}

	SetupContext();

	if (m_Error != errNone) {
		return m_Error;
	}

	return errNone;
}

void ProResEncoder::SetupContext()
{

	std::string logMessagePrefix = "ProRes Plugin :: SetupContext :: ";
	std::ostringstream logMessage;

	{
		logMessage << logMessagePrefix;
		g_Log(logLevelInfo, logMessage.str().c_str());
	}

	if (m_pContext != NULL) {
		avcodec_free_context(&m_pContext);
		av_frame_free(&m_pFrame);
		av_packet_free(&m_pPkt);
		sws_freeContext(m_pSwsContext);
	}

	const AVCodec* encoder = avcodec_find_encoder_by_name("prores_ks");

	{
		logMessage.str("");
		logMessage.clear();
		logMessage << logMessagePrefix << "encoder = " << encoder;
		g_Log(logLevelInfo, logMessage.str().c_str());
	}

	m_pContext = avcodec_alloc_context3(encoder);

	{
		logMessage.str("");
		logMessage.clear();
		logMessage << logMessagePrefix << "m_pContext = " << m_pContext;
		g_Log(logLevelInfo, logMessage.str().c_str());
	}

	const struct AVRational timeBase = { 1, (int)m_CommonProps.GetFrameRateNum() };
	const struct AVRational frameRate = { (int)m_CommonProps.GetFrameRateNum(), 1 };

	m_pContext->pix_fmt = m_pSettings->GetProfile().PixelFormat;
	m_pContext->width = m_CommonProps.GetWidth();
	m_pContext->height = m_CommonProps.GetHeight();
	m_pContext->time_base = timeBase;
	m_pContext->framerate = frameRate;

	av_opt_set(m_pContext->priv_data, "profile", std::to_string(m_Profile).c_str(), 0);
	av_opt_set(m_pContext->priv_data, "bits_per_mb", "8000", 0);
	av_opt_set(m_pContext->priv_data, "vendor", "apl0", 0);

	m_pSwsContext = sws_getContext(m_pContext->width, m_pContext->height, AV_PIX_FMT_YUV422P16LE, m_pContext->width, m_pContext->height, m_pSettings->GetProfile().PixelFormat, SWS_POINT, NULL, NULL, NULL);

	{
		logMessage.str("");
		logMessage.clear();
		logMessage << logMessagePrefix << "m_pSwsContext = " << m_pSwsContext;
		g_Log(logLevelInfo, logMessage.str().c_str());
	}

	m_pPkt = av_packet_alloc();

	{
		logMessage.str("");
		logMessage.clear();
		logMessage << logMessagePrefix << "allocated" << " :: m_pPkt = " << m_pPkt;
		g_Log(logLevelInfo, logMessage.str().c_str());
	}

	int iRet = avcodec_open2(m_pContext, encoder, NULL);

	{
		logMessage.str("");
		logMessage.clear();
		logMessage << logMessagePrefix << "encoder opened";
		g_Log(logLevelInfo, logMessage.str().c_str());
	}


	if (iRet < 0) {
		m_Error = errNoCodec;
		return;
	}

	m_pFrame = av_frame_alloc();
	m_pFrame->format = m_pContext->pix_fmt;
	m_pFrame->width = m_pContext->width;
	m_pFrame->height = m_pContext->height;

	iRet = av_frame_get_buffer(m_pFrame, 0);

	if (iRet < 0) {
		g_Log(logLevelInfo, "ProRes Plugin :: DoProcess :: failed to allocate frame get buffer");
		m_Error = errAlloc;
	}

	m_bFlushed = false;

	m_Error = errNone;

}

StatusCode ProResEncoder::DoProcess(HostBufferRef* p_pBuff)
{
	int encoderRet = 0;
	std::string logMessagePrefix = "ProRes Plugin :: DoProcess :: ";
	std::ostringstream logMessage;

	{
		logMessage << logMessagePrefix;
		g_Log(logLevelInfo, logMessage.str().c_str());
	}

	if (m_Error != errNone) {
		return m_Error;
	}

	if (m_bFlushed) {
		return errMoreData;
	}

	if (p_pBuff == NULL || !p_pBuff->IsValid()) {

		g_Log(logLevelInfo, "ProRes Plugin :: DoProcess :: trying to flush");

		encoderRet = avcodec_send_frame(m_pContext, NULL);

		{
			logMessage.str("");
			logMessage.clear();
			logMessage << logMessagePrefix << "after flushing" << " :: encoderRet = " << encoderRet;
			g_Log(logLevelInfo, logMessage.str().c_str());
		}

	} else {

		char* pBuf = NULL;
		size_t bufSize = 0;

		if (av_frame_make_writable(m_pFrame) < 0) {
			g_Log(logLevelError, "ProRes Plugin :: DoProcess :: Failed to make frame writeable");
			return errFail;
		}

		if (!p_pBuff->LockBuffer(&pBuf, &bufSize)) {
			g_Log(logLevelError, "ProRes Plugin :: Failed to lock the buffer");
			return errFail;
		}

		if (pBuf == NULL || bufSize == 0) {
			g_Log(logLevelError, "ProRes Plugin :: No data to encode");
			p_pBuff->UnlockBuffer();
			return errUnsupported;
		}

		uint32_t width = 0;
		uint32_t height = 0;

		if (!p_pBuff->GetUINT32(pIOPropWidth, width) || !p_pBuff->GetUINT32(pIOPropHeight, height)) {
			g_Log(logLevelError, "ProRes Plugin :: Width/Height not set when encoding the frame");
			return errNoParam;
		}

		uint8_t* pSrc = reinterpret_cast<uint8_t*>(const_cast<char*>(pBuf));

		uint32_t iPixelBytes = m_pSettings->GetBitDepth() > 8 ? 2 : 1;

		// clrYUVp 16-bit 4:2:2 (16-bit) -> YUV422P10LE (10-bit)

		g_Log(logLevelInfo, "ProRes Plugin :: DoProcess :: allocating IN FRAME");

		AVFrame* pInFrame = av_frame_alloc();
		pInFrame->format = AV_PIX_FMT_YUV422P16LE;
		pInFrame->width = m_pContext->width;
		pInFrame->height = m_pContext->height;

		if (av_frame_get_buffer(pInFrame, 0) < 0) {
			g_Log(logLevelInfo, "ProRes Plugin :: DoProcess :: failed to allocate IN frame buffer");
			return errAlloc;
		}

		if (av_frame_make_writable(pInFrame) < 0) {
			g_Log(logLevelError, "ProRes Plugin :: DoProcess :: Failed to make IN frame writeable");
			return errFail;
		}

		g_Log(logLevelInfo, "ProRes Plugin :: DoProcess :: populating IN FRAME");

		memcpy(pInFrame->data[0], pSrc, width * height * iPixelBytes);

		pSrc += width * height * iPixelBytes;

		memcpy(pInFrame->data[1], pSrc, (width * height * iPixelBytes) / 4);

		pSrc += (width * height * iPixelBytes) / 4;

		memcpy(pInFrame->data[2], pSrc, (width * height * iPixelBytes) / 4);

		g_Log(logLevelInfo, "ProRes Plugin :: DoProcess :: performing scaling");

		if (sws_scale_frame(m_pSwsContext, m_pFrame, pInFrame) < 0) {
			av_frame_free(&pInFrame);
			g_Log(logLevelInfo, "ProRes Plugin :: DoProcess :: failed to convert IN to TRANSIENT");
			return errFail;
		}

		av_frame_free(&pInFrame);

		int64_t pts = -1;

		if (!p_pBuff->GetINT64(pIOPropPTS, pts)) {
			g_Log(logLevelError, "ProRes Plugin :: PTS not set when encoding the frame");
			return errNoParam;
		}

		m_pFrame->pts = pts;

		g_Log(logLevelInfo, "ProRes Plugin :: DoProcess :: sent frame to ENCODER");

		encoderRet = avcodec_send_frame(m_pContext, m_pFrame);

		p_pBuff->UnlockBuffer();

	}

	if (encoderRet < 0) {

		logMessage.str("");
		logMessage.clear();
		logMessage << logMessagePrefix << "frame submission failed";
		g_Log(logLevelInfo, logMessage.str().c_str());

		return errFail;

	}

	encoderRet = avcodec_receive_packet(m_pContext, m_pPkt);

	if (encoderRet == AVERROR(EAGAIN)) {
		g_Log(logLevelError, "ProRes Plugin :: DoProcess :: AVERROR(EAGAIN)");
		return errMoreData;
	}

	if (encoderRet == AVERROR_EOF) {
		m_bFlushed = true;
		g_Log(logLevelError, "ProRes Plugin :: DoProcess :: AVERROR_EOF");
		return errNone;
	}

	if (encoderRet < 0) {
		g_Log(logLevelError, "ProRes Plugin :: DoProcess :: AVERROR");
		return errFail;
	}

	HostBufferRef outBuf(false);
	if (!outBuf.IsValid() || !outBuf.Resize(m_pPkt->size)) {
		g_Log(logLevelError, "ProRes Plugin :: Could not resize output buffer");
		return errAlloc;
	}

	char* pOutBuf = NULL;
	size_t outBufSize = 0;

	if (!outBuf.LockBuffer(&pOutBuf, &outBufSize)) {
		g_Log(logLevelError, "ProRes Plugin :: Could not lock output buffer");
		return errAlloc;
	}

	memcpy(pOutBuf, m_pPkt->data, m_pPkt->size);

	av_packet_unref(m_pPkt);

	int64_t ePts = m_pPkt->pts;

	outBuf.SetProperty(pIOPropPTS, propTypeInt64, &ePts, 1);

	int64_t eDts = m_pPkt->pts + 1;

	outBuf.SetProperty(pIOPropDTS, propTypeInt64, &eDts, 1);

	// WHY? libavcodec is returning ts and dts at the same value which resolve does not like

	uint8_t isKeyFrame = 1;
	outBuf.SetProperty(pIOPropIsKeyFrame, propTypeUInt8, &isKeyFrame, 1);

	av_packet_unref(m_pPkt);

	return m_pCallback->SendOutput(&outBuf);

}

void ProResEncoder::DoFlush()
{

	g_Log(logLevelInfo, "ProRes Plugin :: DoFlush");

	if (m_Error != errNone) {
		return;
	}

	StatusCode sts = DoProcess(NULL);
	while (sts == errNone) {
		sts = DoProcess(NULL);
	}

}

std::string ProResEncoder::ConvertUINT8ToHexStr(const uint8_t* v, const size_t s) {

	std::stringstream ss;

	ss << std::hex << std::setfill('0');

	for (int i = 0; i < s; i++) {
		ss << std::hex << std::setw(2) << static_cast<int>(v[i]);
	}

	return ss.str();
}

