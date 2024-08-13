#ifndef PRORES_HQ_H_
#define PRORES_HQ_H_

#include "prores_encoder.h"

using namespace IOPlugin;

namespace IOPlugin {

	class ProResHQEncoder final : public ProResEncoder
	{
	public:
		static const uint8_t s_UUID[];
		static const ProfileMap s_ProfileMap[1];

	public:
		ProResHQEncoder();
		~ProResHQEncoder();

		static StatusCode s_RegisterCodecs(HostListRef* p_pList);
		static StatusCode s_GetEncoderSettings(HostPropertyCollectionRef* p_pValues, HostListRef* p_pSettingsList);
	};

}

#endif // PRORES_HQ_H_