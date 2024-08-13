#include "prores_encoder.h"

#include <assert.h>

ProResEncoder::ProResEncoder()
{
	m_Error = errNone;
}

ProResEncoder::~ProResEncoder()
{

}

StatusCode ProResEncoder::DoInit(HostPropertyCollectionRef* p_pProps)
{
	uint32_t vFourCC = 0;
	p_pProps->GetUINT32(pIOPropFourCC, vFourCC);

	g_Log(logLevelInfo, "ProRes Plugin :: DoInit :: fourCC = %d", vFourCC);

	return errNone;
}

StatusCode ProResEncoder::DoOpen(HostBufferRef* p_pBuff)
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

	m_pWorker.reset(new ProResWorker(m_ColorModel, &m_CommonProps, m_pSettings.get(), &m_ProfileMap));

	return errNone;
}

StatusCode ProResEncoder::DoProcess(HostBufferRef* p_pBuff)
{
	return m_pWorker->EncodeFrame(p_pBuff, m_pCallback);
}

void ProResEncoder::DoFlush()
{

	g_Log(logLevelInfo, "ProRes Plugin :: DoFlush");

	if (m_Error != errNone) {
		return;
	}

	StatusCode sts = DoProcess(NULL);

}
