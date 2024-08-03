#pragma once

#include "uisettings_controller.h"

#include <memory>


extern "C" {
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

using namespace IOPlugin;

namespace IOPlugin {

	class ProResWorker final
	{
	public:
		ProResWorker(uint32_t ColorModel, HostCodecConfigCommon* pCommonProps, UISettingsController* pSettings, ProfileMap ProfileMap);
		~ProResWorker();
		StatusCode EncodeFrame(HostBufferRef* p_pBuff, HostCodecCallbackRef* pCallback);

	private:
		bool m_IsFlushed;
		bool m_IsFullRange;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_ColorModel;
		uint32_t m_iFrameRate;
		int32_t m_iBitsPerSample;
		std::string m_sProfileValue;
		StatusCode m_Error;
		AVPixelFormat m_InPixelFormat;
		AVPixelFormat m_PixelFormat;
		AVCodecContext* m_pContext;
		AVFrame* m_pInFrame;
		AVFrame* m_pOutFrame;
		SwsContext* m_pSwsContext;
		AVPacket* m_pPkt;
	};

}