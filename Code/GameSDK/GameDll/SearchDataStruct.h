#pragma once

#include "SearchItem.h"

#include <vector>

// Interface, that defines interaction with search algorithm implementation
// Used in  nearest fireplace search

class ISearchDataStruct
{
public:

	ISearchDataStruct() {};
	virtual ~ISearchDataStruct() {};

	// Adds item to search pool
	virtual void AddItem(SearchItem& searchItem) = 0;

	// Removes items from search pool
	virtual void RemoveItem(SearchItem& searchItem) = 0;

	// Queries item nearest to query point
	virtual const SearchItem* GetNearest(SearchItem& queryPoint) const = 0;

	// Queries all items within particular range from query point
	virtual std::vector<SearchItem*> GetItemsInRange(SearchItem& queryPoint, double range) = 0;

	// Clears the search pool
	virtual void Clear() = 0;
};

