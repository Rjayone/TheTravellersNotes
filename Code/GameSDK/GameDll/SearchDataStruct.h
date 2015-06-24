#pragma once

#include "SearchItem.h"

#include <vector>

class ISearchDataStruct
{
public:

	ISearchDataStruct() {};

	virtual ~ISearchDataStruct() {};

	virtual void AddItem(SearchItem& searchItem) = 0;

	virtual void RemoveItem(SearchItem& searchItem) = 0;

	virtual const SearchItem* GetNearest(SearchItem& queryPoint) = 0;

	virtual std::vector<SearchItem*> GetItemsInRange(SearchItem& queryPoint, double range) = 0;

	virtual void Clear() = 0;

private:

};

