#include "stdafx.h"
#include "SimpleSearchStruct.h"

#include <limits>

using std::vector;

void SimpleSearchStruct::AddItem(SearchItem& searchItem) {
	data.push_back(searchItem);
}

void SimpleSearchStruct::RemoveItem(SearchItem& searchItem) {
	for (auto i = data.begin(); i != data.end(); ++i) {
		if (*i == searchItem)
			data.erase(i);
	}
}

const SearchItem* SimpleSearchStruct::GetNearest(SearchItem& queryPoint) {

	if (data.size() == 0)
	{
		CryLog("No entries in the search pool");
		return nullptr;
	}

	// Setting frist element as nearest by default
	auto nearestElement = data.begin();
	double nearestDistance = (queryPoint.getPosition() - nearestElement->getPosition()).GetLengthSquared();

	// Search for closest element
	for (auto current = data.begin(); current != data.end(); ++current) {

		double distanceToElement = (queryPoint.getPosition() - current->getPosition()).GetLengthSquared();

		if (distanceToElement < nearestDistance) {
			nearestElement = current;
			nearestDistance = (queryPoint.getPosition() - nearestElement->getPosition()).GetLengthSquared();
		}
	}

	return nearestElement._Ptr;
}

vector<SearchItem*> SimpleSearchStruct::GetItemsInRange(SearchItem& queryPoint, double range) {

	vector<SearchItem*> resultSet;
	double squaredRange = range*range;

	if (data.size() == 0)
	{
		CryLog("No entries in the search pool");
		return resultSet;
	}

	// Search for the elements in range
	for (auto current = data.begin(); current != data.end(); ++current) {
		double distanceToElement = (queryPoint.getPosition() - current->getPosition()).GetLengthSquared();
		if (distanceToElement < squaredRange) {
			resultSet.push_back(current._Ptr);
		}
	}
}

void SimpleSearchStruct::Clear() {
	data.clear();
}
