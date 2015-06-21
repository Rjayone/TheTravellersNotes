/****************************************************************************************************************************
* File: BasicObject.h

* Description: Базовый игровой инвентарный объект. Создан в замен базовому дабы была возможность использовать его в архитайп ентити

* Created by: Andrew Medvedev

* Date: 22.08.2013

* -Update: 29.03.14

* Diko Source File

*****************************************************************************************************************************/

#ifndef _IBASIC_OBJECT_
#define _IBASIC_OBJECT_

#include "IGameObject.h"

//Pick-Drop-Use protocols
#include "Pickable.h"
#include "Dropable.h"
#include "Usable.h"

//SInventoryItem declaration
struct SInventoryItem;

class IBasicObject : public CGameObjectExtensionHelper< IBasicObject, IGameObjectExtension >,
	public IDropable,
	public IPickable,
	public IUsable
{
public:
	//Описание:
	//Метод испоьзуется для сброса и перезагрузки параметров сущности
	virtual void Reset() = 0;

	//Описание:
	//Метод возвращает объект SmartScriptTable, который в дальнейшем испоьзуется для получения параметров из скрипта сущности,
	//Или задание этих параметров.
	virtual SmartScriptTable GetSmartScriptTable() = 0;

	//Опиание:
	//Метод возвращает имя сущности. Имя является уникальным.
	//Оно же отображается в инвнетаре
	virtual string GetObjectName() = 0;

	//Описание:
	//Метод возвращает описание предмета, которое используется в инвнетаре
	virtual string GetObjetDescription() = 0;

	//Описание:
	//Метод возвращает путь к моделе(объекту) сущности
	virtual string GetModelPath() = 0;

	//Описание:
	//Метод возвращает тип сущности.
	//В частности это имя определяет некоторые параметры обработки сущности
	virtual string GetEntityType() = 0;

	//Описание:
	//Метод возвращает указатель на структуру SInventoryItem после прочтения XML-файла данной сущности.
	//В XML-файле храняться базовые параметры сущности, которые её характерезуют как таковую.
	virtual SInventoryItem* GetItemParamsXML() = 0;

protected:
	string m_ObjectName;
	string m_ObjectDescr;
	string m_ModelPath;
	string m_EntityType;

	int m_nSize;
	int m_nCost;

	EntityId m_ItemId;
};

#endif