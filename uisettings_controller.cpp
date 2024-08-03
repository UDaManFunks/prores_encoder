#include "uisettings_controller.h"
#include "prores_encoder.h"

#include <assert.h>
#include <sstream>

UISettingsController::UISettingsController()
{
	InitDefaults();
}

UISettingsController::UISettingsController(const HostCodecConfigCommon& p_CommonProps)
	: m_CommonProps(p_CommonProps)
{
	InitDefaults();
}

UISettingsController::~UISettingsController()
{
}

void UISettingsController::Load(IPropertyProvider* p_pValues)
{
	uint8_t val8 = 0;
	p_pValues->GetUINT8("prores_reset", val8);
	if (val8 != 0) {
		*this = UISettingsController();
		return;
	}

	p_pValues->GetString("prores_enc_markers", m_MarkerColor);
}

StatusCode UISettingsController::Render(HostListRef* p_pSettingsList)
{
	StatusCode err = RenderGeneral(p_pSettingsList);
	if (err != errNone) {
		return err;
	}

	{
		HostUIConfigEntryRef item("prores_separator");
		item.MakeSeparator();
		if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
			g_Log(logLevelError, "ProRes Plugin :: Failed to add a separator entry");
			return errFail;
		}
	}

	err = RenderQuality(p_pSettingsList);
	if (err != errNone) {
		return err;
	}

	{
		HostUIConfigEntryRef item("prores_reset");
		item.MakeButton("Reset");
		item.SetTriggersUpdate(true);
		if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
			g_Log(logLevelError, "ProRes Plugin :: Failed to populate the button entry");
			return errFail;
		}
	}

	return errNone;
}

void UISettingsController::InitDefaults()
{

}

StatusCode UISettingsController::RenderGeneral(HostListRef* p_pSettingsList)
{
	if (0) {
		HostUIConfigEntryRef item("prores_lbl_general");
		item.MakeLabel("General Settings");

		if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
			g_Log(logLevelError, "ProRes Plugin :: Failed to populate general label entry");
			return errFail;
		}
	}

	// Markers selection
	if (m_CommonProps.GetContainer().size() >= 32) {
		HostUIConfigEntryRef item("prores_enc_markers");
		item.MakeMarkerColorSelector("Chapter Marker", "Marker 1", m_MarkerColor);
		if (!item.IsSuccess() || !p_pSettingsList->Append(&item)) {
			g_Log(logLevelError, "ProRes Plugin :: Failed to populate encoder preset UI entry");
			assert(false);
			return errFail;
		}
	}

	return errNone;
}

StatusCode UISettingsController::RenderQuality(HostListRef* p_pSettingsList)
{

	return errNone;
}

const std::string& UISettingsController::GetMarkerColor() const
{
	return m_MarkerColor;
}

