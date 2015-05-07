/****************************************

* File: EconomicResources.h/cpp

* Description:  ласс ресурсов

* Created by: Dmitriy Miroshnichenko & Andrew Medvedev

* Date: 24.11.2014

* Diko Source File

* ToDo: 1. —делать возможность прибавл€ть разные ресурсы в разные промежутки времени?

*****************************************/

#include "StdAfx.h"
#include "EconomicResources.h"
#include "BuildMenu.h"

CEconomicResource::CEconomicResource(){
	m_type = 0;
	m_count = 0;
	m_cost = 0;
	m_maxCount = 0;
	m_name = "BasicResource";
	m_modifier = 1;
}
CEconomicResource::CEconomicResource(int type, int count, int cost, int maxCount, string name){
	m_type = type;
	m_count = count;
	m_cost = cost;
	m_maxCount = maxCount;
	m_name = name;
	m_modifier = 1;
}

void CEconomicResource::SetType(int type){
	m_type = type;
}
void CEconomicResource::SetCount(int count, bool add){
	if (add)
		m_count += count;
	else
		m_count = count;
}
void CEconomicResource::SetMaxCount(int maxCount){
	m_maxCount = maxCount;
}
void CEconomicResource::SetCost(int cost){
	m_cost = cost;
}
void CEconomicResource::SetName(string name){
	m_name = name;
}
void CEconomicResource::SetModifier(int modifier){
	m_modifier = modifier;
}

int CEconomicResource::GetType(){
	return m_type;
}
int CEconomicResource::GetCount(){
	return m_count;
}
int CEconomicResource::GetMaxCount(){
	return m_maxCount;
}
int CEconomicResource::GetCost(){
	return m_cost;
}
string CEconomicResource::GetName(){
	return m_name;
}
int CEconomicResource::GetModifier(){
	CUIBuildMenu *menu = g_pGame->GetBuildMenu();
	if (menu){
		m_modifier = 0;
		std::vector<SBuilding*> buildings = menu->GetPerformedBuildings();
		for (int i = 0; i < buildings.size(); i++){
			SBuilding *building = buildings[i];
			for (int j = 0; j < building->ProvidingResources.size(); j++){
				if (building->ProvidingResources[i].m_type == m_type){
					m_modifier += building->ProvidingResources[i].m_count;
				}
			}
		}
	}
	return m_modifier;
}