#pragma once

#include "profilemap.h"
#include "wrapper/plugin_api.h"

#include <memory>

namespace IOPlugin {

	class UISettingsController
	{
	public:
		UISettingsController();
		UISettingsController(const HostCodecConfigCommon& p_CommonProps);
		~UISettingsController();
		void Load(IPropertyProvider* p_pValues);
		StatusCode Render(HostListRef* p_pSettingsList);

	private:
		void InitDefaults();
		StatusCode RenderGeneral(HostListRef* p_pSettingsList);
		StatusCode RenderQuality(HostListRef* p_pSettingsList);

	public:
		virtual ProfileMap GetProfile();
		virtual int32_t GetBitsPerSample();
		virtual std::string& GetMarkerColor();

	private:
		HostCodecConfigCommon m_CommonProps;
		std::string m_MarkerColor;
		int32_t m_Profile;
	};

}