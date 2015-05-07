#include "stdAfx.h"
#include "Nodes\G2FlowBaseNode.h"
#include "PlayerStatsManager.h"

class CFlowNode_PlayerStats : public CFlowBaseNode<eNCT_Instanced>, public IPlayerStatsListener
{
	//Перечисление входных параметров нода
	enum INPUTS
	{
		EIP_Enable = 0,
		EIP_Disable
	};

	//Перечисление выходных параметров нода
	//Ниже описаны те статы, которые могут быть обновлены
	enum OUTPUTS
	{
		EOP_OnHealthChanged,
		EOP_OnStrengthChanged,
		EOP_OnStaminaChanged,
		EOP_OnAgilityChanged,

		//И остальные параметры
	};

	CPlayerStatsManager* m_pPlayerStatsManager;
	SActivationInfo m_pActInfo;
public:

	//Описание:
	//Конструктор по умолчанию
	//Принимает:
	//Информацию об активации нода
	CFlowNode_PlayerStats(SActivationInfo* pActInfo)
	{
		m_pActInfo = NULL;
		m_pPlayerStatsManager = g_pGame->GetPlayerStatsManager();
	}


	//Описание:
	//Метод возвращает копию нода. Вызывается в момент клонирования ctrl+c
	//Принимает:
	//Информацию об активации нода
	//Возвращает указатель на новый нод
	IFlowNodePtr Clone(SActivationInfo* pActInfo)
	{
		return new CFlowNode_PlayerStats(pActInfo);
	}

	//Описание:
	//Деструктор. Удаляем подписку
	~CFlowNode_PlayerStats()
	{
		if (m_pPlayerStatsManager)
			m_pPlayerStatsManager->DeleteListener(this);
		else
			g_pGame->GetPlayerStatsManager()->DeleteListener(this);
	}

	//Описание:
	//Данный метод заполняет структуру SFlowNodeConfig данными, неоходимыми для создания и работы нода
	//Здесь инициализируются входные/выходные части нода, его описание и категория
	//Принимает:
	//Структура-конфигуратор нода
	void GetConfiguration(SFlowNodeConfig& config)
	{
		//in ports --------------------------------------------------------------
		static const SInputPortConfig in_ports[] =
		{
			InputPortConfig_Void("Enable", _HELP("Enable listener to recive updates from player stats manager")),
			InputPortConfig_Void("Disable", _HELP("Disable listen")),
			{ 0 }
		};

		//out ports  ------------------------------------------------------------
		static const SOutputPortConfig out_ports[] =
		{
			OutputPortConfig<int>("OnHealthChanged", _HELP("")),
			OutputPortConfig<int>("OnStrengthChanged", _HELP("")),
			OutputPortConfig<int>("OnStaminaChanged", _HELP("")),
			OutputPortConfig<int>("OnAgilityChanged", _HELP("")),
			{ 0 }
		};

		config.sDescription = _HELP("Node recive updates from PlayerStatsManager");
		config.pInputPorts = in_ports;
		config.pOutputPorts = out_ports;
		config.SetCategory(EFLN_APPROVED);
	}

	//Описание:
	//Метод срабатывает, если что-то либо проиходит с нодом
	void ProcessEvent(EFlowEvent event, SActivationInfo * pActInfo)
	{
		if (event == eFE_Initialize)
			m_pActInfo = *pActInfo;

		if (event == eFE_Activate)
		{
			if (IsPortActive(pActInfo, EIP_Enable))
			{
				if (m_pPlayerStatsManager)
					m_pPlayerStatsManager->AddListener(this);

				//Обновим параметры всех статов
				CPlayerStat* stat = m_pPlayerStatsManager->GetStat(EPS_Health);
				ActivateOutput<int>(&m_pActInfo, EOP_OnHealthChanged, stat->GetStatValue() + stat->GetStatBonus());

				stat = m_pPlayerStatsManager->GetStat(EPS_Strength);
				ActivateOutput<int>(&m_pActInfo, EOP_OnStrengthChanged, stat->GetStatValue() + stat->GetStatBonus());

				stat = m_pPlayerStatsManager->GetStat(EPS_Stamina);
				ActivateOutput<int>(&m_pActInfo, EOP_OnStaminaChanged, stat->GetStatValue() + stat->GetStatBonus());

				stat = m_pPlayerStatsManager->GetStat(EPS_Agila);
				ActivateOutput<int>(&m_pActInfo, EOP_OnAgilityChanged, stat->GetStatValue() + stat->GetStatBonus());
			}
			else if (m_pPlayerStatsManager)
				m_pPlayerStatsManager->DeleteListener(this);
			
		}
		if (event == eFE_Activate && IsPortActive(pActInfo, EIP_Disable))
		{
			if (m_pPlayerStatsManager)
				m_pPlayerStatsManager->DeleteListener(this);
		}
	}

	//Описание:
	//Данный метод вызывается, если идет изменнеие статов
	//Принимает:
	//Указатель на стат, который был изменен
	void OnStatChanged(CPlayerStat* stat)
	{
		int value = stat->GetStatBonus() + stat->GetStatValue();
		switch (stat->GetType()) {
		case EPS_Health: {
			ActivateOutput<int>(&m_pActInfo, EOP_OnHealthChanged, value);
			break;
		}
		case EPS_Strength: {
			ActivateOutput<int>(&m_pActInfo, EOP_OnStrengthChanged, value);
			break;
		}
		case EPS_Stamina: {
			ActivateOutput<int>(&m_pActInfo, EOP_OnStaminaChanged, value);
			break;
		}
		case EPS_Agila: {
			ActivateOutput<int>(&m_pActInfo, EOP_OnAgilityChanged, value);
			break;
		}
		}
	}

	//Описание:
	//Добавляем сайз занимаемой памяти
	void GetMemoryUsage(ICrySizer * s) const
	{
		s->Add(*this);
	}
};


REGISTER_FLOW_NODE("Player:StatsSensor", CFlowNode_PlayerStats);