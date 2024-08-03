#pragma once

#include "uisettings_controller.h"
#include "prores_worker.h"

#include <memory>

using namespace IOPlugin;

namespace IOPlugin {

	class ProResPXEncoder final : public IPluginCodecRef
	{
	public:
		static const uint8_t s_UUID[];
		static const ProfileMap s_ProfileMap[1];

	public:
		ProResPXEncoder();
		~ProResPXEncoder();

		static StatusCode s_RegisterCodecs(HostListRef* p_pList);
		static StatusCode s_GetEncoderSettings(HostPropertyCollectionRef* p_pValues, HostListRef* p_pSettingsList);

		virtual bool IsNeedNextPass() override
		{
			return false;
		}

		virtual bool IsAcceptingFrame(int64_t p_PTS) override
		{
			return false;
		}

	protected:
		virtual StatusCode DoInit(HostPropertyCollectionRef* p_pProps) override;
		virtual StatusCode DoOpen(HostBufferRef* p_pBuff) override;
		virtual StatusCode DoProcess(HostBufferRef* p_pBuff) override;
		virtual void DoFlush() override;

	private:
		int32_t m_Profile;
		uint32_t m_ColorModel;
		HostCodecConfigCommon m_CommonProps;
		StatusCode m_Error;
		std::unique_ptr<UISettingsController> m_pSettings;
		std::unique_ptr<ProResWorker> m_pWorker;
	};

}
