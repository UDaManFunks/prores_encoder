#ifndef PRORES_422_H_
#define PRORES_422_H_

#include "prores_encoder.h"

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

#endif // PRORES_422_H_