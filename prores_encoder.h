#pragma once

#include <memory>

#include "wrapper/plugin_api.h"

using namespace IOPlugin;

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
}

struct ProfileMap {
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
	void SetupContext();
	void CleanUp();
	std::string ProResEncoder::ConvertUINT8ToHexStr(const uint8_t* v, const size_t s);

private:

	int m_ColorModel;
	int32_t m_Profile;
	StatusCode m_Error;
	bool m_bFlushed;
	std::unique_ptr<UISettingsController> m_pSettings;
	HostCodecConfigCommon m_CommonProps;
	AVCodecContext* m_pContext;
	AVPacket* m_pPkt;
	AVFrame* m_pFrame;
	SwsContext* m_pSwsContext;
};
