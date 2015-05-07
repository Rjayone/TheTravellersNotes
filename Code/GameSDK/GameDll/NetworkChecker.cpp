#include "stdAfx.h"
#include "Nodes\G2FlowBaseNode.h"
#include <Windows.h>
#include <wininet.h> 

#pragma comment(lib, "Wininet.lib")

class CFlowNode_NetworkChecker : public CFlowBaseNode<eNCT_Instanced>
{
	enum INPUTS
	{
		EIP_Trigger = 0,
		EIP_URL,
	};

	enum OUTPUTS
	{
		EOP_Status
	};

public:
	CFlowNode_NetworkChecker(SActivationInfo * pActInfo)
	{
	}

	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_NetworkChecker(pActInfo);
	}

	void GetConfiguration(SFlowNodeConfig& config)
	{
		//in ports --------------------------------------------------------------
		static const SInputPortConfig in_ports[] =
		{
			InputPortConfig_Void("Check", _HELP("Check network status in current moment")),
			InputPortConfig<string>("URL", _HELP("By default using http://google.com")),
			{ 0 }
		};

		//out ports  ------------------------------------------------------------
		static const SOutputPortConfig out_ports[] =
		{
			OutputPortConfig<bool>("Status", _HELP("Return true if connection exist, false otherways")),
			{ 0 }
		};

		config.sDescription = _HELP("Node check internet connection. URL must have same form http://url.com");
		config.pInputPorts  = in_ports;
		config.pOutputPorts = out_ports;
		config.SetCategory(EFLN_APPROVED);
	}

	void ProcessEvent(EFlowEvent event, SActivationInfo * pActInfo)
	{
		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_Trigger))
		{
			string url  = GetPortString(pActInfo, EIP_URL);
			if (!strcmp(url, ""))
				url = "http://google.com";

			//Здесь формируем запрос и тестим подключение
			bool connected = false;
			connected = InternetCheckConnection(url, FLAG_ICC_FORCE_CONNECTION, 0);
			ActivateOutput<bool>(pActInfo, EOP_Status, connected);
		}
	}

	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}
};


REGISTER_FLOW_NODE("System:InternetConnection", CFlowNode_NetworkChecker);