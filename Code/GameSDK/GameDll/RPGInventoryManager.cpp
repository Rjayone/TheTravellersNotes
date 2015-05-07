#include "StdAfx.h"
#include "RPGInventoryManager.h"
#include "GameActions.h"
#include "MouseUtils.h"

#define USABLE_DISTANT 5

CRPGInventoryManager::CRPGInventoryManager()
{
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (!pGameFramework)
		return;
	pGameFramework->RegisterListener(this, "RPGInventoryManager", EFRAMEWORKLISTENERPRIORITY::FRAMEWORKLISTENERPRIORITY_DEFAULT);
	
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->AddExtraActionListener(this);
}

void CRPGInventoryManager::OnPostUpdate(float fDeltaTime)
{
	if (!gEnv->IsEditorGameMode() && gEnv->IsEditor() && m_pBasicObjects.size() != 0)
	{
		Clear();
	}
}

void CRPGInventoryManager::OnSaveGame(ISaveGame* pSaveGame)
{
}

void CRPGInventoryManager::OnLoadGame(ILoadGame* pLoadGame)
{
}

void CRPGInventoryManager::OnLevelEnd(const char* nextLevel)
{
}

void CRPGInventoryManager::OnActionEvent(const SActionEvent& event)
{
}

void CRPGInventoryManager::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions &actions = g_pGame->Actions();
	if (actions.use == action && activationMode == 1)
	{
		IEntity *pEntity = CMouseUtils::GetMouseEntity(USABLE_DISTANT);
		if (pEntity != NULL)
		{
			IBasicObject *pBasicObject = GetBasicObject(pEntity->GetId());
			if (!pBasicObject)
				return;

			pBasicObject->OnPickUp(pEntity->GetId());
		}
	}
}

void CRPGInventoryManager::AddObjects(IBasicObject *pBasicObject)
{
	if (GetBasicObject(pBasicObject->GetEntityId()))
		return;

	m_pBasicObjects.push_back(pBasicObject);
	CryLogAlways("[RPGInventoryManager]: Object %s are added.", pBasicObject->GetObjectName());
}
void CRPGInventoryManager::DeleteObject(EntityId id)
{
	int index = GetObjectIndex(id);
	if (index == -1) return;
	//CryLogAlways("[RPGInventoryManager]: Object %s are erased.", str);
	m_pBasicObjects.erase(m_pBasicObjects.begin()+index);
}

void CRPGInventoryManager::DeleteObject(char* sObjectName)
{
	int index = GetObjectIndex(sObjectName);
	if (index == -1) return;
	m_pBasicObjects.erase(m_pBasicObjects.begin() + index);
	CryLogAlways("[RPGInventoryManager]: Object %s are erased.", m_pBasicObjects[index]->GetObjectName());
}

IBasicObject* CRPGInventoryManager::GetBasicObject(EntityId id)
{
	int index = GetObjectIndex(id);
	if (index == -1)
		return NULL;

	return m_pBasicObjects[index];
}

int CRPGInventoryManager::GetObjectIndex(EntityId id)
{
	for (int i = 0; i < m_pBasicObjects.size(); i++)
		if (m_pBasicObjects[i]->GetEntityId() == id) //Тут надо бы проверить какое ид
			return i;
	return -1;
}

int CRPGInventoryManager::GetObjectIndex(char* name)
{
	for (int i = 0; i < m_pBasicObjects.size(); i++)
	if (strcmp(m_pBasicObjects[i]->GetObjectName(), name) == 0) //Тут надо бы проверить какое ид
		return i;
	return -1;
}

bool CRPGInventoryManager::Clear()
{
	m_pBasicObjects.clear();
	return true;
}