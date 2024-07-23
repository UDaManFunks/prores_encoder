#pragma once

#include <string>

extern "C" {
#include "libavutil/avutil.h"
}

namespace IOPlugin {

	struct ProfileMap {
		std::string ProfileValue;
		uint32_t FourCC;
		AVPixelFormat PixelFormat;
		std::string ProfileName;
	};

}
