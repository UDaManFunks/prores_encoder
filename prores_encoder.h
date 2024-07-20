#pragma once

#include <memory>
#include <semaphore>

#include "wrapper/plugin_api.h"

extern "C" {
#include "libavutil/avutil.h"
}

using namespace IOPlugin;

struct ProfileMap {
	std::string ProfileValue;
	uint32_t FourCC;
	AVPixelFormat PixelFormat;
	std::string ProfileName;
};

class UISettingsController;

class ProResEncoder : public IPluginCodecRef
{
public:
	static const uint8_t s_UUID[];
	static const ProfileMap s_ProfileMap[4];

public:
	ProResEncoder();
	~ProResEncoder();

	static StatusCode s_RegisterCodecs(HostListRef* p_pList);
	static StatusCode s_GetEncoderSettings(HostPropertyCollectionRef* p_pValues, HostListRef* p_pSettingsList);

	virtual bool IsNeedNextPass() override
	{
		return false;
	}

	virtual bool IsAcceptingFrame(int64_t p_PTS) override
	{
		return false;
	}

protected:
	virtual StatusCode DoInit(HostPropertyCollectionRef* p_pProps) override;
	virtual StatusCode DoOpen(HostBufferRef* p_pBuff) override;
	virtual StatusCode DoProcess(HostBufferRef* p_pBuff) override;
	virtual void DoFlush() override;

private:
	int32_t m_Profile;
	uint32_t m_ColorModel;
	std::unique_ptr<UISettingsController> m_pSettings;
	HostCodecConfigCommon m_CommonProps;
	StatusCode m_Error;
};
