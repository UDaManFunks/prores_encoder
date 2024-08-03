#pragma once

#ifndef PRORES_PROFILEMAP_H_
#define PRORES_PROFILEMAP_H_

#include <string>

extern "C" {
#include "libavutil/avutil.h"
}

namespace IOPlugin {

	struct ProfileMap {
		std::string ProfileValue;
		uint32_t FourCC;
		uint32_t BitsPerSample;
		AVPixelFormat PixelFormat;
		std::string ProfileName;
	};

}

#endif // PRORES_PROFILEMAP_H_