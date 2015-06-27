/****************************************
* File: PlaceableEntity.h
* Description: Перемещаемый игровой объект
* Created by: Andrew Medvedev
* Date: 22.08.2013
* Diko Source File
*****************************************/

#ifndef _IPLACEABLE_ENTITY_
#define _IPLACEABLE_ENTITY_

class IPlaceable
{
public:
	virtual void StartPlacing() = 0;
	virtual void StopPlacing() = 0;
	virtual void CheckLocalBounds() = 0;
};

#endif //_IPLACEABLE_ENTITY_