/*********************************************************************************************************
- File: BuildSelectionHelper.cpp
- Description: Реализация хэпера стройки, считывание его параметров и непсоредственно спавны зданий
- Created by: Гомза Яков
- Date:   25.03.2014
- Update: 19.07.14 by Медведев Андрей
- Edda Studio
- ToDo:
********************************************************************************************************/
#include "StdAfx.h"
#include "BuildHelper.h"
#include "Game.h"
#include "GameActions.h"

#define FILE_PATH "objects/default/primitive_pyramid.cgf"

CBuildHelper::CBuildHelper() : pScriptable(NULL)
{
}

bool CBuildHelper::Init(IGameObject * pGameObject)
{
	SetGameObject(pGameObject);
	return true;
}


void CBuildHelper::PostInit(IGameObject * pGameObject)
{
	GetEntity()->LoadGeometry(0, FILE_PATH);
	pGameObject->EnableUpdateSlot(this, 0);
	pScriptable = GetEntity()->GetScriptTable();
	Reset();
}


void CBuildHelper::Reset()
{
	if (!pScriptable)
		return;

	SmartScriptTable propertiesTable;
	const bool hasPropertiesTable = pScriptable->GetValue("Properties", propertiesTable);    
	if (!hasPropertiesTable) return;

	const char* model = NULL;
	propertiesTable->GetValue("fileModel", model);                                       
	GetEntity()->LoadGeometry(0, model);

	SEntityPhysicalizeParams pp;
	pp.type = PE_RIGID;
	GetEntity()->Physicalize(pp);
}


void CBuildHelper::ProcessEvent(SEntityEvent& entityEvent)
{
	if (entityEvent.event == ENTITY_EVENT_EDITOR_PROPERTY_CHANGED)
		Reset();
	if(entityEvent.event == ENTITY_EVENT_SCRIPT_EVENT)
	{
		const char* eventName = NULL;
		eventName = reinterpret_cast<const char*>(entityEvent.nParam[0]);
		if (eventName && !strcmp(eventName, "OnPropertyChange"))
		{
		  Reset();
		}
	}
}