#ifndef PRORES_LT_H_
#define PRORES_LT_H_

#include "prores_encoder.h"

#include <memory>

using namespace IOPlugin;

namespace IOPlugin {

	class ProResLTEncoder final : public ProResEncoder
	{
	public:
		static const uint8_t s_UUID[];
		static const ProfileMap s_ProfileMap[1];

	public:
		ProResLTEncoder();
		~ProResLTEncoder();

		static StatusCode s_RegisterCodecs(HostListRef* p_pList);
		static StatusCode s_GetEncoderSettings(HostPropertyCollectionRef* p_pValues, HostListRef* p_pSettingsList);
	};

}

#endif // PRORES_LT_H_