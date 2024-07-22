#pragma once

using namespace IOPlugin;

#include <memory>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

class ProResWorker
{
public:
	ProResWorker();
	~ProResWorker();
	void Init(uint32_t ColorModel, std::string ProfileValue, HostCodecConfigCommon CommonProps, AVPixelFormat PixelFormat, int32_t BitPerSample);
	StatusCode EncodeFrame(HostBufferRef* p_pBuff, HostCodecCallbackRef* pCallback);

private:

	std::string m_sProfileValue;
	AVPixelFormat m_InPixelFormat;
	AVPixelFormat m_PixelFormat;
	bool m_IsFullRange;
	bool m_IsInitialized;
	uint32_t m_Width;
	uint32_t m_Height;
	uint32_t m_ColorModel;
	int m_iFrameRate;
	int32_t m_iBitDepth;

	StatusCode m_Error;
	AVCodecContext* m_pContext;
	AVFrame* m_pInFrame;
	AVFrame* m_pOutFrame;
	SwsContext* m_pSwsContext;
	AVPacket* m_pPkt;
	const AVCodec* m_Encoder;
};