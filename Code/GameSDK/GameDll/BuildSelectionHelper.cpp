/*********************************************************************************************************
- File: BuildSelectionHelper.cpp
- Description: ���������� ������ �������, ���������� ��� ���������� � ��������������� ������ ������
- Created by: ����� ����
- Date: 25.03.2014
- Diko Source File
- ToDo: �������� ���������� �������� ������� ��� �������
		�������� ����� ��� ��������� ������� ���� �������� - ������, � ���������� ������� ������ ���� ������������ ������������
********************************************************************************************************/

#include "StdAfx.h"
#include "IGameFramework.h"
#include "BuildSelectionHelper.h"
#include "Game.h"
#include "Globals.h"
#include "BuildMenu.h"
#include "MouseUtils.h"
#include "Player.h"

#define CATEGORY1 "Towers" 
#define CATEGORY2 "Mines" 
#define CATEGORY3 "Houses" 
#define CATEGORY4 "Other" 

#define BUILDINGS_IN_CATEGORY 4

#define BUILDINGS_PARAMS_FILE "/Libs/BuildSystem/BuildingsParams.xml"

CSelectionBuildHelper::CSelectionBuildHelper()
{
	m_SelectedEntityBuildId = NULL;
}

void CSelectionBuildHelper::Select()
{
	char *temp = "";
	CUIBuildMenu *pBuildMenu = g_pGame->GetBuildMenu();
	IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
	IEntity* pSelection = GetMouseEntityID();
	if (!pSelection  || !view || !pBuildMenu)
		return;

	//���� ��������� �������� �������� BuildHelper'��
	if (!strcmp(pSelection->GetClass()->GetName(), "BuildHelper"))
	{	
		//�������� ���������� �������� �������
		pBuildMenu->Clear();
		m_Buildings.clear();
		m_SelectedEntityBuildId = pSelection;
		pBuildMenu->SetSelectedEntity(m_SelectedEntityBuildId);

		//���������� ���������� ������� �������
		//���� � �� ������ � ������ BuildHelper7, �� � ������ 7,
		//��� ����� ��� ����� ������ � �������. � ������ ����� ����� ������� ������ � ������ BuildCamera7
		temp = (char*)m_SelectedEntityBuildId->GetName();
		char* name = new char;
		strcpy(name, temp);
		int i = 0, j;
		for (i, j = 11; j < strlen(name); i++, j++)
			name[i] = name[j];
		name[i] = '\0';

		//���� ���������� �� ����� ������, ��� �������� � ������� ����
		IEntity* pCam = gEnv->pEntitySystem->FindEntityByName("BuildCamera" + (string)name);
		if (pCam != NULL) view->LinkTo(pCam);

		BuildOptions();
		pBuildMenu->ShowSlideMenu(true);

		for (int i = 0; i < m_Buildings.size(); i++)
			pBuildMenu->SendBuildingsInfo(&m_Buildings[i]);

		//�������������� ������ ������.
		IUIElement *m_pBuildMenu = gEnv->pFlashUI->GetUIElement("BuildMenu");
		SUIArguments arg;
		arg.AddArgument(0);
		m_pBuildMenu->CallFunction("CreateList", arg);
	}
}


void CSelectionBuildHelper::BuildOptions()
{
	IScriptTable *pScriptTable = m_SelectedEntityBuildId->GetScriptTable();
	SmartScriptTable propertiesTable, HPropertiesTable, TPropertiesTable, MPropertiesTable;
	//��� ������� ������ ������(����, ����� � ��) ������� ���� ����� �����.
	//�� ��������� ��� ���������� ���������� 

	if (pScriptTable)
	{
		const bool hasPropertiesTable = pScriptTable->GetValue("Properties", propertiesTable);
		if (!hasPropertiesTable)
			return;

		//�������� ������ ������� ��������
		bool tres = propertiesTable->GetValue("Towers", TPropertiesTable);
		bool mres = propertiesTable->GetValue("Mines", MPropertiesTable);
		bool hres = propertiesTable->GetValue("Houses", HPropertiesTable);

		if (!tres || !mres || !hres)
			return;

		char* sBuildName = new char;
		char* sBuildModel = new char;
		for (int i = 1; i <= BUILDINGS_IN_CATEGORY; ++i)
		{
			char* ii = new char;
			string index = itoa(i, ii, 10);
			TPropertiesTable->GetValue("sBuildingName_" + index, sBuildName);
			TPropertiesTable->GetValue("obj_model_" + index, sBuildModel);
			if (!strcmp(sBuildName, "") || !strcmp(sBuildModel, ""))
				continue;

			SBuilding *build = GetBuildParams(sBuildName);
			if (build)
			{
				build->Model = sBuildModel;
				m_Buildings.push_back(*build);
			}
		}

		for (int i = 1; i <= BUILDINGS_IN_CATEGORY; ++i)
		{
			char* ii = new char;
			string index = itoa(i, ii, 10);
			MPropertiesTable->GetValue("sBuildingName_" + index, sBuildName);
			MPropertiesTable->GetValue("obj_model_" + index, sBuildModel);
			if (!strcmp(sBuildName, "") || !strcmp(sBuildModel, ""))
				continue;

			SBuilding *build = GetBuildParams(sBuildName);
			if (build)
			{
				build->Model = sBuildModel;
				m_Buildings.push_back(*build);
			}
		}

		for (int i = 1; i <= BUILDINGS_IN_CATEGORY; ++i)
		{
			char* ii = new char;
			string index = itoa(i, ii, 10);
			HPropertiesTable->GetValue("sBuildingName_" + index, sBuildName);
			HPropertiesTable->GetValue("obj_model_" + index, sBuildModel);
			if (!strcmp(sBuildName, "") || !strcmp(sBuildModel, ""))
				continue;

			SBuilding *build = GetBuildParams(sBuildName);
			if (build)
			{
				build->Model = sBuildModel;
				m_Buildings.push_back(*build);
			}
		}
	}
}


void CSelectionBuildHelper::SpawnBuilding(string model)
{
	CUIBuildMenu* pBuildMenu = g_pGame->GetBuildMenu();

	//�������������� ��������� ������
	SEntitySpawnParams Spawn;
	Spawn.vPosition = m_SelectedEntityBuildId->GetPos();
	Spawn.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("BasicEntity"); //��. todo, 2-�� �����
	Spawn.bCreatedThroughPool = true;
	Spawn.nFlags |= ENTITY_FLAG_CASTSHADOW;
	IEntity* pSpawnObj = gEnv->pEntitySystem->SpawnEntity(Spawn, true);
	pSpawnObj->LoadGeometry(0, model);

	//�������������� �������� ��������� ��� ������
	SEntityPhysicalizeParams params;
	params.mass = 10000;
	params.type = PE_STATIC;
	pSpawnObj->Physicalize(params);
	pSpawnObj->SetPos(m_SelectedEntityBuildId->GetPos());
	pSpawnObj->SetRotation(m_SelectedEntityBuildId->GetRotation());
	pBuildMenu->ShowSlideMenu(false);
	pBuildMenu->Clear();
	m_Buildings.clear();

	//������������ ������ ��������������
	if (IEntityRenderProxy* pProxy = (IEntityRenderProxy *)pSpawnObj->GetProxy(ENTITY_PROXY_RENDER))
	{
		pProxy->SetOpacity(1.0f);
		pBuildMenu->ShowAcceptMenu(true);
	}
}


SBuilding* CSelectionBuildHelper::GetBuildParams(const char* name)
{
	//���� ���� � ����������� ������.
	XmlNodeRef Root = gEnv->pSystem->LoadXmlFromFile("GameSDK/Libs/BuildSystem/Buildings.xml");
	if (Root == NULL)
		return NULL;

	//���� ���� ����������, �� �������� ���������� ������� � �������� ����������
	for (int i = 0; i < Root->getChildCount(); i++)
	{
		XmlNodeRef param = Root->getChild(i);
		const char* id = param->getAttr("id");
		if (!strcmp(id, name))
		{
			SBuilding *build = new SBuilding;
			const char* sBuildName = param->getAttr("name");
			const char* sDescr = param->getAttr("descr");
			const char* sAdd = param->getAttr("add");
			param->getAttr("type", build->Type);
			param->getAttr("cost", build->Cost);
			param->getAttr("wood", build->Wood);
			param->getAttr("stone", build->Stone);
			param->getAttr("iron", build->Iron);

			for (int j = 0; j < param->getChildCount(); j++){
				XmlNodeRef provRes = param->getChild(j);
				if (provRes){
					SResource res;
					provRes->getAttr("type", res.m_type);
					provRes->getAttr("count", res.m_count);
					build->ProvidingResources.push_back(res);
				}
			}

			build->Name = sBuildName;
			build->Descr = sDescr;
			build->Additional = sAdd;
			return build;
		}
	}
	return NULL;
}

IEntity* CSelectionBuildHelper::GetMouseEntityID()
{
	ray_hit hit;
	float x, y, xMouse, yMouse, zMouse = 0.0f;
	if (!gEnv->pHardwareMouse || !gEnv->pRenderer || !gEnv->p3DEngine || !gEnv->pSystem || !gEnv->pEntitySystem || !(CPlayer*)g_pGame->GetIGameFramework()->GetClientActor())
		return 0;

	gEnv->pHardwareMouse->GetHardwareMouseClientPosition(&x, &y);
	y = gEnv->pRenderer->GetHeight() - y;

	gEnv->pRenderer->UnProjectFromScreen(x, y, 0.0f, &xMouse, &yMouse, &zMouse);


	const unsigned int flags = rwi_stop_at_pierceable | rwi_colltype_any;
	float  fRange = gEnv->p3DEngine->GetMaxViewDistance();
	Vec3  vCamPos = gEnv->pSystem->GetViewCamera().GetPosition();
	Vec3     vDir = (Vec3(xMouse, yMouse, zMouse) - vCamPos);
	gEnv->pPhysicalWorld->RayWorldIntersection(vCamPos, vDir * fRange, ent_all, flags, &hit, 1);


	IEntity *pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(hit.pCollider);
	if (pEntity)
		return pEntity;

	return NULL;
}