#pragma once
#include "SearchDataStruct.h"

// Search algorithm simple implementation
// Searches item within a vector, comparing distance to each element

class SimpleSearchStruct :
	public ISearchDataStruct
{
public:
	SimpleSearchStruct(){};
	virtual ~SimpleSearchStruct(){};

	void AddItem(SearchItem& searchItem);

	void RemoveItem(SearchItem& searchItem);

	const SearchItem* GetNearest(SearchItem& queryPoint);

	std::vector<SearchItem*> GetItemsInRange(SearchItem& queryPoint, double range);

	void Clear();

private:
	std::vector<SearchItem> data;
};

