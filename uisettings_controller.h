#pragma once

#include "profilemap.h"
#include "wrapper/plugin_api.h"

#include <memory>

namespace IOPlugin {

	class UISettingsController final
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
		const std::string& GetMarkerColor() const;

	private:
		HostCodecConfigCommon m_CommonProps;
		std::string m_MarkerColor;
	};

}