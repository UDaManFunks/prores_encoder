#pragma once

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
