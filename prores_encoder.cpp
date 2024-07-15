#include "prores_encoder.h"
#include "prores_worker.h"

#include <cstring>
#include <vector>
#include <assert.h>
#include <stdint.h>
#include <sstream>

const uint8_t ProResEncoder::s_UUID[] = { 0x21, 0x42, 0xe8, 0x41, 0xd8, 0xe4, 0x41, 0x4b, 0x87, 0x9e, 0xa4, 0x80, 0xfc, 0x90, 0xda, 0xb5 };
const ProfileMap ProResEncoder::s_ProfileMap[4] = { {"0", 'apco', AV_PIX_FMT_YUV422P10LE , "ProRes 422 (Proxy)"}, {"1", 'apcs', AV_PIX_FMT_YUV422P10LE , "ProRes 422 (LT)"}, {"2",'apcn', AV_PIX_FMT_YUV422P10LE , "ProRes 422"}, {"3",'apch', AV_PIX_FMT_YUV422P10LE, "ProRes 422 (HQ)"} };
std::counting_semaphore<10> g_MaxConcurrencyLimit(12);

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

	std::string logMessagePrefix = "ProRes Plugin :: s_RegisterCodecs";
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

	uint32_t vColorModel = clrRGB;
	codecInfo.SetProperty(pIOPropColorModel, propTypeUInt32, &vColorModel, 1);

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

	uint8_t vThreadSafe = 1;
	codecInfo.SetProperty(pIOPropThreadSafe, propTypeUInt8, &vThreadSafe, 1);

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
{

}

ProResEncoder::~ProResEncoder()
{

}

StatusCode ProResEncoder::DoInit(HostPropertyCollectionRef* p_pProps)
{
	g_Log(logLevelInfo, "ProRes Plugin :: DoInit");

	return errNone;
}

StatusCode ProResEncoder::DoOpen(HostBufferRef* p_pBuff)
{

	std::string logMessagePrefix = "ProRes Plugin :: DoOpen";
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
		logMessage << logMessagePrefix << " :: vFourCC = " << vFourCC;
		g_Log(logLevelInfo, logMessage.str().c_str());
	}

	p_pBuff->SetProperty(pIOPropFourCC, propTypeUInt32, &vFourCC, 1);

	uint8_t vBitDepth = m_pSettings->GetBitDepth();
	p_pBuff->SetProperty(pIOPropBitDepth, propTypeUInt32, &vBitDepth, 1);

	vBitDepth = 16;
	p_pBuff->SetProperty(pIOPropBitsPerSample, propTypeUInt32, &vBitDepth, 1);

	uint8_t isMultiPass = 0;
	StatusCode sts = p_pBuff->SetProperty(pIOPropMultiPass, propTypeUInt8, &isMultiPass, 1);
	if (sts != errNone) {
		return sts;
	}

	if (m_Error != errNone) {
		return m_Error;
	}

	return errNone;
}

StatusCode ProResEncoder::DoProcess(HostBufferRef* p_pBuff)
{

	// this should be limited to 12 concurrent requests running at the same time

	g_MaxConcurrencyLimit.acquire();

	ProResWorker worker(m_pSettings->GetProfile().ProfileValue, m_pSettings->GetProfile().PixelFormat, m_CommonProps.GetWidth(), m_CommonProps.GetHeight(), m_CommonProps.GetFrameRateNum(), m_pSettings->GetBitDepth());

	StatusCode returnCode = worker.EncodeFrame(p_pBuff, m_pCallback);

	g_MaxConcurrencyLimit.release();

	return returnCode;
}

void ProResEncoder::DoFlush()
{

	g_Log(logLevelInfo, "ProRes Plugin :: DoFlush");

	if (m_Error != errNone) {
		return;
	}

	StatusCode sts = DoProcess(NULL);

}
