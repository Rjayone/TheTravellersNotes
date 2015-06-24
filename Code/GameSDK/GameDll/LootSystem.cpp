#include "StdAfx.h"
#include "LootSystem.h"
#include "Actor.h"

#define LOOTLIST_FOLDER "/Libs/Items/Library/GeneralItemsLootList.xml"
#define ITEMS_FOLDER "/Libs/Items/Library/ItemsDescription.xml"

CLootSystem::CLootSystem()
{
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework != NULL)
		pGameFramework->RegisterListener(this, "CLootSystem", FRAMEWORKLISTENERPRIORITY_DEFAULT);

	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->AddExtraActionListener(this);
}

CLootSystem::~CLootSystem()
{
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework != NULL)
		pGameFramework->UnregisterListener(this);
}

void CLootSystem::InitLootSystem(CActor* pActor)
{
	pNPCActor = pActor;

	pNpcEntity = pActor->GetEntity();

	int LootId = GetLoodId(pNpcEntity);

	if (LootId <= -1)
		return;

	SetItemsOptionsFromXml(LootId);
}

void CLootSystem::OnAction(const ActionId& action, int activationMode, float value)
{
	const auto& actions = g_pGame->Actions();
	if (actions.Loot == action)
	{
		StartLoot();
	}
}

bool CLootSystem::CheckForLoot(CActor* pdActor)
{
	ray_hit ray;
	Vec3  cameraPos = gEnv->pSystem->GetViewCamera().GetPosition() + gEnv->pSystem->GetViewCamera().GetViewdir();
	Vec3 cameraDir = gEnv->pSystem->GetViewCamera().GetViewdir();

	gEnv->pPhysicalWorld->RayWorldIntersection(cameraPos, cameraDir * lootDistance, ent_all, rwi_stop_at_pierceable | rwi_colltype_any, &ray, 1);

	IEntity *pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(ray.pCollider);

	if (!pEntity)
		return false;

	auto pActor = (CActor*)pEntity;

	if (pActor == NULL) return false;
	else pActor->IsDead();
}

void CLootSystem::StartLoot()
{
	if (!CheckForLoot(pNPCActor))
		return;

	if (GetItemsForLoot())
	{
		CRPGInventory *pUIInventory = g_pGame->GetRPGInventory();
		int i = 0;
		for each (CItemOptions item in ItemsList)
		{
			if (item.isLooted) return;
			SInventoryItem *pItem = new SInventoryItem();
			pItem->name = item.ItemName;
			pItem->description = item.ItemDescription;
			pItem->size = item.ItemSize;
			pItem->cost = item.ItemCost;
			pItem->size = item.ItemSize;
			pItem->type = item.ItemType;

			CryLog(itoa(i++, new char[10], 10));

			pUIInventory->AddItem(pItem, 2);
			//item.isLooted = true;
		}
	}
}

int CLootSystem::GetLoodId(IEntity* pNpcEntity)
{
	IScriptTable *pAIStript = pNpcEntity->GetScriptTable();

	if (!pAIStript)
		return -1;

	SmartScriptTable propertiesTable;
	SmartScriptTable LootPropertiesTable;

	bool hasProperties = pAIStript->GetValue("Properties", propertiesTable);
	if (!hasProperties)
		return -1;

	const bool hasPropertiesTable = propertiesTable->GetValue("LootProperties", LootPropertiesTable);
	if (!hasPropertiesTable)
		return -1;

	int LootId;
	LootPropertiesTable->GetValue("LootId", LootId);

	return LootId;
}

void CLootSystem::SetItemsOptionsFromXml(int LootId)
{
	XmlNodeRef LootDescriptionFile = gEnv->pSystem->LoadXmlFromFile(PathUtil::GetGameFolder() + LOOTLIST_FOLDER);
	if (LootDescriptionFile)
	{
		auto count = LootDescriptionFile->getChildCount();
		for (int i = 0; i < count; i++)
		{
			XmlNodeRef Loot = LootDescriptionFile->getChild(i);

			const char *pItemLootId = Loot->getAttr("id");

			if (atoi(pItemLootId) == LootId)
			{
				auto ItemsCount = Loot->getChildCount();

				for (auto i = 0; i < ItemsCount; i++)
				{
					XmlNodeRef Item = Loot->getChild(i);

					CLootOptions LootOption = { atoi(Item->getAttr("id")), atoi(Item->getAttr("count")) };
					LootOptions.insert(LootOptions.begin(), LootOption);
				}
			}
		}
	}
}

bool CLootSystem::GetItemsForLoot()
{
	XmlNodeRef ItemsDescriptionFile = gEnv->pSystem->LoadXmlFromFile(PathUtil::GetGameFolder() + ITEMS_FOLDER);
	if (ItemsDescriptionFile)
	{
		if (ItemsDescriptionFile == NULL)
			return false;

		for (int i = 0; i < ItemsDescriptionFile->getChildCount(); i++)
		{
			XmlNodeRef cItems = ItemsDescriptionFile->getChild(i);//<item ... />
			if (cItems != NULL)
			{
				for (auto i = 0; i < LootOptions.size(); i++)
				{
					if (LootOptions[i].ItemId == atoi(cItems->getAttr("id")))
					{
						CItemOptions ItemOption = { cItems->getAttr("class"), cItems->getAttr("name"), cItems->getAttr("descr"), atof(cItems->getAttr("cost")),
							(EInventoryItemType)(atoi(cItems->getAttr("type"))), atoi(cItems->getAttr("size")) };
						ItemsList.insert(ItemsList.begin(), ItemOption);
						return true;
					}
				}
			}
		}
	}
	return false;
}