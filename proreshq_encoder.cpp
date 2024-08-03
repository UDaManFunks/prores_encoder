#include "uisettings_controller.h"
#include "proreshq_encoder.h"

#include <assert.h>
#include <sstream>

const uint8_t ProResHQEncoder::s_UUID[] = { 0x21, 0x42, 0xe8, 0x41, 0xd8, 0xe4, 0x41, 0x4b, 0x87, 0x9e, 0xa4, 0x80, 0xfc, 0x90, 0xda, 0xb6 };

const ProfileMap ProResHQEncoder::s_ProfileMap[1] = { {"3",'apch', AV_PIX_FMT_YUV422P10LE, "ProRes 422 (HQ)"} };

StatusCode ProResHQEncoder::s_GetEncoderSettings(HostPropertyCollectionRef* p_pValues, HostListRef* p_pSettingsList)
{
	HostCodecConfigCommon commonProps;
	commonProps.Load(p_pValues);

	UISettingsController settings(commonProps);
	settings.Load(p_pValues);

	return settings.Render(p_pSettingsList);
}

StatusCode ProResHQEncoder::s_RegisterCodecs(HostListRef* p_pList)
{

	const char* logMessagePrefix = "ProResHQ Plugin :: s_RegisterCodecs";

	{
		g_Log(logLevelInfo, "%s", logMessagePrefix);
	}

	HostPropertyCollectionRef codecInfo;
	if (!codecInfo.IsValid()) {
		return errAlloc;
	}

	codecInfo.SetProperty(pIOPropUUID, propTypeUInt8, ProResHQEncoder::s_UUID, 16);

	const char* pCodecName = "Auto";
	codecInfo.SetProperty(pIOPropName, propTypeString, pCodecName, static_cast<int>(strlen(pCodecName)));

	const char* pCodecGroup = s_ProfileMap[0].ProfileName.c_str();
	codecInfo.SetProperty(pIOPropGroup, propTypeString, pCodecGroup, static_cast<int>(strlen(pCodecGroup)));

	uint32_t vFourCC = s_ProfileMap[0].FourCC;
	codecInfo.SetProperty(pIOPropFourCC, propTypeUInt32, &vFourCC, 1);

	g_Log(logLevelInfo, "ProResHQ Plugin :: s_RegisterCodecs :: fourCC = %d", vFourCC);

	uint32_t vMediaVideo = mediaVideo;
	codecInfo.SetProperty(pIOPropMediaType, propTypeUInt32, &vMediaVideo, 1);

	uint32_t vDirection = dirEncode;
	codecInfo.SetProperty(pIOPropCodecDirection, propTypeUInt32, &vDirection, 1);

	uint8_t vHWAcc = 1;
	codecInfo.SetProperty(pIOPropHWAcc, propTypeUInt8, &vHWAcc, 1);

	/* clrAYUV == 4:4:4 */
	uint32_t vColorModel = clrAYUV;

	/* clrYUVp == 4:2:2 */
	// uint32_t vColorModel = clrYUVp;

	codecInfo.SetProperty(pIOPropColorModel, propTypeUInt32, &vColorModel, 1);

	if (vColorModel == clrYUVp) {
		uint8_t hSampling = 2;
		uint8_t vSampling = 1;
		codecInfo.SetProperty(pIOPropHSubsampling, propTypeUInt8, &hSampling, 1);
		codecInfo.SetProperty(pIOPropVSubsampling, propTypeUInt8, &vSampling, 1);
	}

	uint32_t vBitDepth = 16;
	codecInfo.SetProperty(pIOPropBitsPerSample, propTypeUInt32, &vBitDepth, 1);

	std::vector<uint8_t> dataRangeVec;
	dataRangeVec.push_back(0);
	dataRangeVec.push_back(1);
	codecInfo.SetProperty(pIOPropDataRange, propTypeUInt8, dataRangeVec.data(), static_cast<int>(dataRangeVec.size()));

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

	codecInfo.SetProperty(pIOPropContainerList, propTypeString, valStrings.c_str(), static_cast<int>(valStrings.size()));

	if (!p_pList->Append(&codecInfo)) {
		return errFail;
	}

	return errNone;
}

ProResHQEncoder::ProResHQEncoder()
	: m_Error(errNone)
{

}

ProResHQEncoder::~ProResHQEncoder()
{

}

StatusCode ProResHQEncoder::DoInit(HostPropertyCollectionRef* p_pProps)
{

	uint32_t vFourCC = 0;
	p_pProps->GetUINT32(pIOPropFourCC, vFourCC);

	g_Log(logLevelInfo, "ProResHQ Plugin :: DoInit :: fourCC = %d", vFourCC);

	return errNone;
}

StatusCode ProResHQEncoder::DoOpen(HostBufferRef* p_pBuff)
{
	m_CommonProps.Load(p_pBuff);

	m_pSettings.reset(new UISettingsController(m_CommonProps));
	m_pSettings->Load(p_pBuff);

	p_pBuff->GetUINT32(pIOPropColorModel, m_ColorModel);

	int16_t vColorMatrix = 1;
	int16_t vColorPrimaries = 1;
	int16_t vTransferFunction = 1;

	p_pBuff->SetProperty(pIOColorMatrix, propTypeInt16, &vColorMatrix, 1);
	p_pBuff->SetProperty(pIOPropColorPrimaries, propTypeInt16, &vColorPrimaries, 1);
	p_pBuff->SetProperty(pIOTransferCharacteristics, propTypeInt16, &vTransferFunction, 1);

	// multi-pass not supported by prores

	uint8_t isMultiPass = 0;
	StatusCode sts = p_pBuff->SetProperty(pIOPropMultiPass, propTypeUInt8, &isMultiPass, 1);
	if (sts != errNone) {
		return sts;
	}

	if (m_Error != errNone) {
		return m_Error;
	}

	m_pWorker.reset(new ProResWorker(m_ColorModel, &m_CommonProps, m_pSettings.get(), s_ProfileMap[0]));

	return errNone;
}

StatusCode ProResHQEncoder::DoProcess(HostBufferRef* p_pBuff)
{
	return m_pWorker->EncodeFrame(p_pBuff, m_pCallback);
}

void ProResHQEncoder::DoFlush()
{

	g_Log(logLevelInfo, "ProResHQ Plugin :: DoFlush");

	if (m_Error != errNone) {
		return;
	}

	StatusCode sts = DoProcess(NULL);

}
