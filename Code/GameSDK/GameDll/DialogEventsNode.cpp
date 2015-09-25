#include "stdAfx.h"
#include "Nodes\G2FlowBaseNode.h"
#include "DialogEvents.h"
#include "DialogSystem.h"


class CFlowNode_DialogEvents : public CFlowBaseNode<eNCT_Cloned>, public IDialogEvents
{
	enum INPUTS
	{
		EIP_Activate
	};

	enum OUTPUTS
	{
		//EOP_Started,
		EOP_OnDialogStarted,
		EOP_OnDialogFinished,
		//EOP_OnPlayerSelectAnswer,
		//EOP_OnNPCAnswer
	};

	SActivationInfo m_actInfo;
public:
	CFlowNode_DialogEvents(SActivationInfo* pActInfo)
	{
		m_actInfo = *pActInfo;
		CUIDialogMenu* pDialogSystem = g_pGame->GetDialogSystem();
		if (pDialogSystem != NULL)
		{
			pDialogSystem->AddEventListener(this);
		}
	}
	
	~CFlowNode_DialogEvents()
	{
		CUIDialogMenu* pDialogSystem = g_pGame->GetDialogSystem();
		if (pDialogSystem != NULL)
		{
			pDialogSystem->RemoveEventListener(this);
		}
	}


	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_DialogEvents(pActInfo);
	}


	//DialogEvents
	void OnDialogStarted(EntityId targetNPCId)
	{
		ActivateOutput<EntityId>(&m_actInfo, EOP_OnDialogStarted, targetNPCId);
	}
	void OnDialogFinished(EntityId targetNPCId)
	{
		ActivateOutput<EntityId>(&m_actInfo, EOP_OnDialogFinished, targetNPCId);
	}
	//void OnPlayerSelectAnswer(string messageToAnswer, int selectedAnswerId, int nextNPCAnswer)
	//{
	//}
	//void OnNPCAnswer(string message)
	//{
	//}
	//~


	void GetConfiguration(SFlowNodeConfig& config)
	{
		//in ports --------------------------------------------------------------
		static const SInputPortConfig in_ports[] =
		{
			InputPortConfig_AnyType("Activate", ""),
			{0}
		};

		//out ports  ------------------------------------------------------------
		static const SOutputPortConfig out_ports[] =
		{
			//OnPlayerSelectAnswer,
			//OnNPCAnswer
			OutputPortConfig<EntityId>("OnDialogStarted", _HELP("")),
			OutputPortConfig<EntityId>("OnDialogFinished", _HELP("")),
			{ 0 }
		};

		config.sDescription = _HELP("Node recognize methodes from DialogEvents");
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.SetCategory(EFLN_APPROVED);
	}


	void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
	{
		switch (event)
		{
		case eFE_Initialize:
			m_actInfo = *pActInfo;
			break;
		case eFE_Activate:
			m_actInfo = *pActInfo;
		}
	}
	virtual void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}
};


REGISTER_FLOW_NODE("DialogSystem:DialogEvents", CFlowNode_DialogEvents);