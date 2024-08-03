#pragma once

#include "uisettings_controller.h"
#include "prores_encoder.h"
#include "prores_worker.h"

#include <memory>

using namespace IOPlugin;

namespace IOPlugin {

	class ProRes422Encoder final : public ProResEncoder
	{
	public:
		static const uint8_t s_UUID[];
		static const ProfileMap s_ProfileMap[1];

	public:
		ProRes422Encoder();
		~ProRes422Encoder();

		static StatusCode s_RegisterCodecs(HostListRef* p_pList);
		static StatusCode s_GetEncoderSettings(HostPropertyCollectionRef* p_pValues, HostListRef* p_pSettingsList);
	};

}

