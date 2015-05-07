#include "StdAfx.h"
//#include "Game.h"
//#include "RPGInventory.h"
//#include "Nodes\G2FlowBaseNode.h"
//
//class CFlowNode_CreateItem : public CFlowBaseNode<eNCT_Singleton>
//{
//	enum INPUTS
//	{
//		EIP_Trigger = 0,
//		EIP_Item = 1,
//		EIP_ArrayString
//	};
//	enum OUTPUTS
//	{
//		EOP_Name = 0,
//		EOP_Descr,
//		EOP_Cost,
//		EOP_Type,
//		EOP_PushSlot,
//		EOP_PrevSlotID
//	};
//
//public:
//
//	CFlowNode_CreateItem(SActivationInfo * pActInfo){ }
//
//	void GetConfiguration(SFlowNodeConfig& config)
//	{
//		//in porst --------------------------------------------------------------
//		static const SInputPortConfig in_ports[] =
//		{
//			InputPortConfig_Void("Call", _HELP("Get item's description")),
//			InputPortConfig<EntityId>("ItemID", _HELP("")),
//			InputPortConfig<string>("ArrayString", _HELP(""))
//		};
//		//out ports  ------------------------------------------------------------
//		static const SOutputPortConfig out_ports[] =
//		{
//			OutputPortConfig<string>("Name", _HELP("Item Name")),
//			OutputPortConfig<string>("Description", _HELP("")),
//			OutputPortConfig<int>("Cost", _HELP("")),
//			OutputPortConfig<int>("Type", _HELP("")),
//			OutputPortConfig<int>("PushSlot", _HELP("Slot in wich would be place item")),
//			OutputPortConfig<int>("PrevSlotID", _HELP(""))
//		};
//		config.sDescription = _HELP("Give exp for player");
//		config.pInputPorts = in_ports;
//		config.pOutputPorts = out_ports;
//		config.SetCategory(EFLN_APPROVED);
//	}
//
//	void ProcessEvent(EFlowEvent event, SActivationInfo * pActInfo)
//	{
//		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_Trigger))
//		{
//			CRPGInventory *inv = g_pGame->GetRPGInventory();
//			inv->OnPickedUp(GetPortEntityId(pActInfo, EIP_Item));
//
//			ActivateOutput<string>(pActInfo, EOP_Name, inv->GetItemsArray()->name);
//			ActivateOutput<string>(pActInfo, EOP_Descr, inv->GetItemsArray()->description);
//			ActivateOutput<int>(pActInfo, EOP_Cost, inv->GetItemsArray()->cost);
//			ActivateOutput<int>(pActInfo, EOP_Type, inv->GetItemsArray()->type);
//			ActivateOutput<int>(pActInfo, EOP_PushSlot, 0);
//			ActivateOutput<int>(pActInfo, EOP_PrevSlotID, 0);
//		}
//		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_ArrayString))
//			string temp = GetPortString(pActInfo, EIP_ArrayString);
//
//	}
//
//
//	virtual void GetMemoryUsage(ICrySizer * s) const
//	{
//		s->Add(*this);
//	}
//};// End __CFlowNode_AddExperience__
//
//REGISTER_FLOW_NODE("Inventory:CreateItem", CFlowNode_CreateItem);