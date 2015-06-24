#pragma once

#include "Game.h"

/**
	Abstraction to use in search structure	
*/
class SearchItem {
public:
	SearchItem(EntityId entityID_, const Vec3& position_) {
		setPosition(position_);
		this->entityID = entityID_;
	}

	SearchItem& setPosition(const Vec3& position) {
		this->position = position;
		return *this;
	}

	const Vec3& getPosition() const {
		return position;
	}

	const EntityId& getEntityID() const {
		return entityID;
	}

	bool operator==(const SearchItem& other) {
		return this->getEntityID() == other.getEntityID();
	}

private:
	Vec3 position;
	EntityId entityID;
};