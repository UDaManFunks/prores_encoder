#pragma once

using namespace IOPlugin;

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

class ProResWorker
{
public:
	ProResWorker(uint32_t vColorModel, std::string sProfileValue, AVPixelFormat pixelFormat, uint32_t iWidth, uint32_t iHeight, int iFrameRate, int iBitDepth, bool IsFullRange);
	~ProResWorker();
	StatusCode EncodeFrame(HostBufferRef* p_pBuff, HostCodecCallbackRef* pCallback);

private:
	void SetupContext(HostBufferRef* p_pBuff);
	std::string ConvertUINT8ToHexStr(const uint8_t* v, const size_t s);
private:

	std::string m_sProfileValue;
	AVPixelFormat m_InPixelFormat;
	AVPixelFormat m_PixelFormat;
	bool m_IsFullRange;
	uint32_t m_Width;
	uint32_t m_Height;
	uint32_t m_ColorModel;
	int m_iFrameRate;
	int m_iBitDepth;

	StatusCode m_Error;
	AVCodecContext* m_pContext;
	AVFrame* m_pInFrame;
	AVFrame* m_pOutFrame;
	SwsContext* m_pSwsContext;
	AVPacket* m_pPkt;
};