#include "StdAfx.h"
//#include "Game.h"
//#include "Camp.h"
//#include "IParticles.h"
//#include "Nodes\G2FlowBaseNode.h"
//
//
////// Node which add experience to player
//////using Progress.h and CharDevSys
//class CFlowNode_AddExperience : public CFlowBaseNode<eNCT_Singleton>
//{
//	enum INPUTS
//	{
//		EIP_Trigger = 0,
//		EIP_CNT = 1
//	};
//
//public:
//
//	CFlowNode_AddExperience(SActivationInfo * pActInfo){ }
//
//	void GetConfiguration(SFlowNodeConfig& config)
//	{
//		//in porst --------------------------------------------------------------
//		static const SInputPortConfig in_ports[] =
//		{
//			InputPortConfig_Void("Active", _HELP("Give exp for player")),
//			InputPortConfig<int>("Amount", _HELP("How much exp needs to give"))
//		};
//		//out ports  ------------------------------------------------------------
//
//		config.sDescription = _HELP("Give exp for player");
//		config.pInputPorts = in_ports;
//		config.SetCategory(EFLN_APPROVED);
//	}
//
//	void ProcessEvent(EFlowEvent event, SActivationInfo * pActInfo)
//	{
//		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_Trigger))
//		{
//			CCharacterDevelopmentSystem *cdv = g_pGame->GetCharDevSys();
//			int exp = GetPortInt(pActInfo, EIP_CNT);
//			cdv->SetExp(exp, true);
//		}
//	}
//
//	virtual void GetMemoryUsage(ICrySizer * s) const
//	{
//		s->Add(*this);
//	}
//};// End __CFlowNode_AddExperience__
//
////// Node returns player level
//////using Progress.h and CharDevSys
//class CFlowNode_GetPlayerLevel : public CFlowBaseNode<eNCT_Singleton>
//{
//	enum INPUTS
//	{
//		EIP_Trigger = 0,
//	};
//	enum OUTPUT
//	{
//		EOP_VALUE = 0,
//	};
//
//public:
//
//	CFlowNode_GetPlayerLevel(SActivationInfo * pActInfo){ }
//
//	void GetConfiguration(SFlowNodeConfig& config)
//	{
//		//in porst --------------------------------------------------------------
//		static const SInputPortConfig in_ports[] =
//		{
//			InputPortConfig_Void("Get", _HELP("Give exp for player"))
//		};
//		//out ports  ------------------------------------------------------------
//		static const SOutputPortConfig out_ports[] =
//		{
//			OutputPortConfig<int>("Done", _HELP("V"))
//		};
//
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
//			CCharacterDevelopmentSystem *cdv = g_pGame->GetCharDevSys();
//			const int lvl = cdv->GetCurrentLevel();
//			ActivateOutput(pActInfo, EOP_VALUE, lvl);
//		}
//	}
//
//	virtual void GetMemoryUsage(ICrySizer * s) const
//	{
//		s->Add(*this);
//	}
//};// End __CFlowNode_GetPlayerLevel__
//
////// Node returns player experience
//////using Progress.h and CharDevSys
//class CFlowNode_GetPlayerExperience : public CFlowBaseNode<eNCT_Singleton>
//{
//	enum INPUTS
//	{
//		EIP_Trigger = 0,
//	};
//	enum OUTPUT
//	{
//		EOP_VALUE = 0,
//	};
//
//public:
//
//	CFlowNode_GetPlayerExperience(SActivationInfo * pActInfo){ }
//
//	void GetConfiguration(SFlowNodeConfig& config)
//	{
//		//in porst --------------------------------------------------------------
//		static const SInputPortConfig in_ports[] =
//		{
//			InputPortConfig_Void("Get", _HELP("Get player experience"))
//		};
//		//out ports  ------------------------------------------------------------
//		static const SOutputPortConfig out_ports[] =
//		{
//			OutputPortConfig<int>("Done", _HELP("Value of player experience"))
//		};
//
//		config.sDescription = _HELP("Get player experience");
//		config.pInputPorts = in_ports;
//		config.pOutputPorts = out_ports;
//		config.SetCategory(EFLN_APPROVED);
//	}
//
//	void ProcessEvent(EFlowEvent event, SActivationInfo * pActInfo)
//	{
//		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_Trigger))
//		{
//			CCharacterDevelopmentSystem *cdv = g_pGame->GetCharDevSys();
//			const int exp = cdv->GetCurrentExp();
//			ActivateOutput(pActInfo, EOP_VALUE, exp);
//		}
//	}
//
//	virtual void GetMemoryUsage(ICrySizer * s) const
//	{
//		s->Add(*this);
//	}
//};// End __CFlowNode_GetPlayerExp__
//
////
////// Node returns player experience
//////using Progress.h and CharDevSys
//class CFlowNode_GetPlayerHungryStance : public CFlowBaseNode<eNCT_Singleton>
//{
//	enum INPUTS
//	{
//		EIP_TriggerGet = 0,
//		EIP_TriggerSet,
//		EIP_Value
//	};
//	enum OUTPUT
//	{
//		EOP_VALUESTR = 0,
//		EOP_VALUEINT
//	};
//
//public:
//
//	CFlowNode_GetPlayerHungryStance(SActivationInfo * pActInfo){ }
//
//	void GetConfiguration(SFlowNodeConfig& config)
//	{
//		//in porst --------------------------------------------------------------
//		static const SInputPortConfig in_ports[] =
//		{
//			InputPortConfig_Void("Get", _HELP("Get player hungry stance")),
//			InputPortConfig_Void("SetHungryLevel", _HELP("Set player hungry level")),
//			InputPortConfig_Void("Value", _HELP("How much set"))
//		};
//		//out ports  ------------------------------------------------------------
//		static const SOutputPortConfig out_ports[] =
//		{
//			OutputPortConfig<string>("Stance", _HELP("Return hungry stance")),
//			OutputPortConfig<int>("Stance", _HELP(""))
//		};
//
//		config.sDescription = _HELP("Return player hungry stance");
//		config.pInputPorts = in_ports;
//		config.pOutputPorts = out_ports;
//		config.SetCategory(EFLN_APPROVED);
//	}
//
//	void ProcessEvent(EFlowEvent event, SActivationInfo * pActInfo)
//	{
//		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_TriggerGet))
//		{
//			CCharacterDevelopmentSystem *cdv = g_pGame->GetCharDevSys();
//			if (!cdv)
//				return;
//
//			int hungryLevel = cdv->GetPlayerParams()->HungryLevel;
//			string hungryType;
//			switch ((int)cdv->GetPlayerParams()->HungryLevel)
//			{
//			case EHSatiety:
//			{
//							  hungryType = "Satiety";
//							  break;
//			}
//			case EHNromal:
//			{
//							 hungryType = "Normal";
//							 break;
//			}
//			case EHHungry:
//			{
//							 hungryType = "Hungry";
//							 break;
//			}
//			case EHVeryHungry:
//			{
//								 hungryType = "Vary hunger";
//								 break;
//			}
//			};//Switch
//			ActivateOutput(pActInfo, EOP_VALUESTR, hungryType);
//			ActivateOutput(pActInfo, EOP_VALUEINT, cdv->GetPlayerParams()->HungryLevel);
//		}
//		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_TriggerSet))
//		{
//			CCharacterDevelopmentSystem *cdv = g_pGame->GetCharDevSys();
//			if (cdv && cdv->GetPlayerParams()->HungryLevel != EHSatiety)
//			{
//				int HLevel = GetPortInt(pActInfo, EIP_Value);
//				(cdv->GetPlayerParams()->HungryLevel + HLevel)>4 ? cdv->GetPlayerParams()->HungryLevel = EHSatiety : cdv->GetPlayerParams()->HungryLevel + HLevel;
//			}
//		}
//	}
//
//	virtual void GetMemoryUsage(ICrySizer * s) const
//	{
//		s->Add(*this);
//	}
//};// End __CFlowNode_GetPlayerExp__
////
////
//class CFlowNode_FoodUsing : public CFlowBaseNode<eNCT_Singleton>
//{
//	enum INPUTS
//	{
//		EIP_TriggerSet = 0,
//		EIP_Value,
//	};
//	enum OUTPUT
//	{
//		EOP_VALUE = 0,
//	};
//
//public:
//
//	CFlowNode_FoodUsing(SActivationInfo * pActInfo){ }
//
//	void GetConfiguration(SFlowNodeConfig& config)
//	{
//		//in porst --------------------------------------------------------------
//		static const SInputPortConfig in_ports[] =
//		{
//			InputPortConfig_Void("Use", _HELP("Use food")),
//			InputPortConfig<int>("Value", _HELP("Value of hungry level"))
//
//		};
//		//out ports  ------------------------------------------------------------
//		static const SOutputPortConfig out_ports[] =
//		{
//			OutputPortConfig<int>("Done", _HELP("Done"))
//		};
//
//		config.sDescription = _HELP("Use food");
//		config.pInputPorts = in_ports;
//		config.pOutputPorts = out_ports;
//		config.SetCategory(EFLN_APPROVED);
//	}
//
//	void ProcessEvent(EFlowEvent event, SActivationInfo * pActInfo)
//	{
//		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_TriggerSet))
//		{
//			int value = GetPortInt(pActInfo, EIP_Value);
//			CCharacterDevelopmentSystem *cdv = g_pGame->GetCharDevSys();
//			cdv->ChangeHugryStance(value);
//			ActivateOutput(pActInfo, EOP_VALUE, 1);
//		}
//	}
//
//	virtual void GetMemoryUsage(ICrySizer * s) const
//	{
//		s->Add(*this);
//	}
//};// End __CFlowNode_FoodUsing__
//
//
//class CFlowNode_GetHPProcent : public CFlowBaseNode<eNCT_Singleton>
//{
//	enum INPUTS
//	{
//		EIP_Trigger = 0,
//	};
//
//	enum OUTPUT
//	{
//		EOP_VALUE = 0,
//	};
//
//public:
//
//	CFlowNode_GetHPProcent(SActivationInfo *pActInfo){ }
//
//	void GetConfiguration(SFlowNodeConfig& config)
//	{
//		//in porst --------------------------------------------------------------
//		static const SInputPortConfig in_ports[] =
//		{
//			InputPortConfig_Void("Get", _HELP(""))
//		};
//		////out ports  ------------------------------------------------------------
//		static const SOutputPortConfig outports[] =
//		{
//			OutputPortConfig<int>("Done", _HELP("Value of player experience"))
//		};
//
//		config.sDescription = _HELP("");
//		config.pInputPorts = in_ports;
//		config.pOutputPorts = outports;
//		config.SetCategory(EFLN_ADVANCED);
//	}
//
//	void ProcessEvent(EFlowEvent event, SActivationInfo * pActInfo)
//	{
//		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_Trigger))
//		{
//			CCharacterDevelopmentSystem *cdv = g_pGame->GetCharDevSys();
//			IActor *player = g_pGame->GetIGameFramework()->GetClientActor();
//			ActivateOutput(pActInfo, EOP_VALUE, (((int)player->GetHealth() / player->GetMaxHealth()) * 100));
//		}
//	}
//
//	virtual void GetMemoryUsage(ICrySizer * s) const
//	{
//		s->Add(*this);
//	}
//};
//
//
//class CFlowNode_OnLevelUp : public CFlowBaseNode<eNCT_Singleton>
//{
//	enum INPUTS
//	{
//		EIP_Start = 0,
//		EIP_Stop = 1,
//	};
//
//	enum OUTPUT
//	{
//		EOP_VALUE = 0,
//	};
//
//public:
//
//	CFlowNode_OnLevelUp(SActivationInfo *pActInfo){ }
//
//	void GetConfiguration(SFlowNodeConfig& config)
//	{
//		//in porst --------------------------------------------------------------
//		static const SInputPortConfig in_ports[] =
//		{
//			InputPortConfig_Void("StartCheck", _HELP("Check on level up. Also use with time:timer")),
//			InputPortConfig_Void("StopCheck", _HELP("Stop listen level"))
//		};
//		////out ports  ------------------------------------------------------------
//		static const SOutputPortConfig outports[] =
//		{
//			OutputPortConfig<bool>("Upped", _HELP("True if up"))
//		};
//
//		config.sDescription = _HELP("Check level up");
//		config.pInputPorts = in_ports;
//		config.pOutputPorts = outports;
//		config.SetCategory(EFLN_ADVANCED);
//	}
//
//	void ProcessEvent(EFlowEvent event, SActivationInfo * pActInfo)
//	{
//		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_Start))
//		{
//			CCharacterDevelopmentSystem *cdv = g_pGame->GetCharDevSys();
//			if (cdv->CheckLevel())
//			{
//				ActivateOutput(pActInfo, EOP_VALUE, true);
//			}
//		}
//		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_Stop))
//		{
//			ActivateOutput(pActInfo, EOP_VALUE, false);
//		}
//	}
//
//	virtual void GetMemoryUsage(ICrySizer * s) const
//	{
//		s->Add(*this);
//	}
//};
//
//class CFlowNode_FirePlaceDrag : public CFlowBaseNode<eNCT_Singleton>
//{
//	enum INPUTS
//	{
//		EIP_Start = 0,
//		EIP_Stop = 1,
//	};
//
//public:
//	CFlowNode_FirePlaceDrag(SActivationInfo *pActInfo) : pEntity(NULL)
//	{
//	}
//	void GetConfiguration(SFlowNodeConfig& config)
//	{
//		//in porst --------------------------------------------------------------
//		static const SInputPortConfig in_ports[] =
//		{
//			InputPortConfig_Void("InitObject", _HELP("Init and start draging fireplace model")),
//			InputPortConfig_Void("StopDrag", _HELP("Stop draging"))
//		};
//
//		config.sDescription = _HELP("Drag controller");
//		config.pInputPorts = in_ports;
//		config.SetCategory(EFLN_ADVANCED);
//	}
//
//	void ProcessEvent(EFlowEvent event, SActivationInfo * pActInfo)
//	{
//
//		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_Start))
//		{
//			//Спаун того самого объекта.
//			//Данный код потом нужно переместить в другую функцию-меню
//			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//			SEntitySpawnParams spawnItem;
//			spawnItem.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("FirePlace");
//			if (spawnItem.pClass)
//				pEntity = gEnv->pEntitySystem->SpawnEntity(spawnItem);
//			pEntity->Activate(true);
//			CFirePlace::dynamiclyInit = true;
//			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		}
//		if (eFE_Activate == event && IsPortActive(pActInfo, EIP_Stop))
//		{
//			//Очень полезная функция!
//			if (pEntity)
//			{
//				CFirePlace *pFP = (CFirePlace*)gEnv->pGame->GetIGameFramework()->GetGameObject(pEntity->GetId())->QueryExtension("FirePlace");
//				if (pFP)	 pFP->StopDrag(pEntity);
//			}
//		}
//	}
//
//	virtual void GetMemoryUsage(ICrySizer * s) const
//	{
//		s->Add(*this);
//	}
//	IEntity *pEntity;
//};
//
//REGISTER_FLOW_NODE("Player:Add Experience", CFlowNode_AddExperience);
//REGISTER_FLOW_NODE("Player:Get Experience", CFlowNode_GetPlayerExperience);
//REGISTER_FLOW_NODE("Player:Get Level", CFlowNode_GetPlayerLevel);
//REGISTER_FLOW_NODE("Player:GetHPProcent", CFlowNode_GetHPProcent);
//REGISTER_FLOW_NODE("Player:On Level Up", CFlowNode_OnLevelUp);
//
//REGISTER_FLOW_NODE("Player:Get Hungry Stance", CFlowNode_GetPlayerHungryStance);
//REGISTER_FLOW_NODE("Player:Food:Use food", CFlowNode_FoodUsing);
//REGISTER_FLOW_NODE("Player:Drag Fireplace", CFlowNode_FirePlaceDrag);
