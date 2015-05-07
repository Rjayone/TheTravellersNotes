/****************************************
* File: Tent.cpp
* Description: Перемещаемый игровой объект полатка
* Created by: Andrew Medvedev
* Date: 22.08.2013
* Diko Source File
*****************************************/
#include <StdAfx.h>
#include "Tent.h"

#define FILE_PATH "Game/Objects/misc/tent.cgf"

bool CTent::dynamiclyInit = false;

//--------------------------------------
CTent::CTent()
{
	m_bStartDrag = false;
	m_bFullInitDone = false;
	m_pObject = NULL;
	m_pScriptTable = NULL;
}

//--------------------------------------
CTent::~CTent()
{
}

bool CTent::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);
	return true;
}

//--------------------------------------
void CTent::PostInit(IGameObject * pGameObject)
{
	GetEntity()->LoadGeometry(0, FILE_PATH);
	pGameObject->EnableUpdateSlot(this, 0);
	m_pObject = GetEntity();
	m_pScriptTable = GetEntity()->GetScriptTable();

	Reset();
}

//--------------------------------------
void CTent::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_RESET:
		Reset();
		break;
	case ENTITY_EVENT_SCRIPT_EVENT:
	{
									  const char* eventName = NULL;
									  eventName = reinterpret_cast< const char* >(event.nParam[0]);
									  if (eventName && !strcmp(eventName, "Used"))
									  {
										  if (!m_bFullInitDone)
										  {
											  CTent *pTent = (CTent*)g_pGame->GetIGameFramework()->GetGameObject(m_pObject->GetId())->QueryExtension("Tent");
											  if (pTent)
											  {
												  //Какой-то евент если объект был использован
												  OnUse();
											  }
										  }
									  }
	}break;
	};
}

//--------------------------------------
void CTent::StartPlacing(IEntity *pObject)
{
	if (!dynamiclyInit)
		return;

	if (m_pScriptTable)
	{
		SmartScriptTable propertiesTable;

		const bool hasPropertiesTable = m_pScriptTable->GetValue("Properties", propertiesTable);
		if (!hasPropertiesTable)
			return;
	}

	ray_hit ray;
	Vec3  cameraPos = gEnv->pSystem->GetViewCamera().GetPosition() + gEnv->pSystem->GetViewCamera().GetViewdir();
	Vec3 cameraDir = gEnv->pSystem->GetViewCamera().GetViewdir();

	float lenght = 2;
	gEnv->pPhysicalWorld->RayWorldIntersection(cameraPos, cameraDir * lenght, ent_all, rwi_stop_at_pierceable | rwi_colltype_any, &ray, 1);

	//Нужно проверять не залазит ли наш объект в другой
	Vec3 VecLenght = m_pObject->GetPos() - gEnv->pEntitySystem->GetEntity(LOCAL_PLAYER_ENTITY_ID)->GetPos();
	if (m_pObject)
		m_pObject->SetPos(ray.pt);
	if (VecLenght.GetLength() > lenght || !ray.bTerrain)
		m_pObject->Invisible(true);
	else
		m_pObject->Invisible(false);
	//todo: фильтр движения игрока. Нужно заморозить игрока
}

//--------------------------------------
void CTent::StopPlacing(IEntity *pEntity)
{
	//отключаем апдейт
	pEntity->Activate(false);

	CTent::dynamiclyInit = false;
	SEntityPhysicalizeParams pp;
	pp.type = 2;
	pEntity->Physicalize(pp);

	if (m_pScriptTable)
	{
		SmartScriptTable propertiesTable;

		const bool hasPropertiesTable = m_pScriptTable->GetValue("Properties", propertiesTable);
		if (!hasPropertiesTable)
			return;
		propertiesTable->SetValue("bUsable", true);
	}
}

//--------------------------------------
void CTent::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	if (m_pObject->IsActive() && dynamiclyInit)
		StartPlacing(m_pObject);
}


//--------------------------------------
void CTent::Reset()
{
	if (m_pScriptTable == NULL)
		return;

	SmartScriptTable propertiesTable;
	const bool hasPropertiesTable = m_pScriptTable->GetValue("Properties", propertiesTable);
	if (!hasPropertiesTable) return;

	const char* modelName = NULL;
	propertiesTable->GetValue("objModel", modelName);

	m_pObject->LoadGeometry(0, modelName);
}

//--------------------------------------
void CTent::OnUse()
{
}

void CTent::CheckLocalBounds()
{

}