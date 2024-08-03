#ifndef PRORES_BASE_H_
#define PRORES_BASE_H_

#include "uisettings_controller.h"
#include "prores_worker.h"

#include <memory>

using namespace IOPlugin;

namespace IOPlugin {

	class ProResEncoder : public IPluginCodecRef
	{
	public:
		virtual bool IsNeedNextPass() override
		{
			return false;
		}

		virtual bool IsAcceptingFrame(int64_t p_PTS) override
		{
			return false;
		}

	protected:
		ProResEncoder();
		~ProResEncoder();

	protected:
		virtual StatusCode DoInit(HostPropertyCollectionRef* p_pProps) override;
		virtual StatusCode DoOpen(HostBufferRef* p_pBuff) override;
		virtual StatusCode DoProcess(HostBufferRef* p_pBuff) override;
		virtual void DoFlush() override;

	protected:
		ProfileMap m_ProfileMap;

	private:
		uint32_t m_ColorModel;
		HostCodecConfigCommon m_CommonProps;
		StatusCode m_Error;
		std::unique_ptr<UISettingsController> m_pSettings;
		std::unique_ptr<ProResWorker> m_pWorker;
	};

}

#endif // PRORES_BASE_H_