#include "stdafx.h"
#include "SaveLoad.h"

#include <sstream>
#include "MD5.h"

using std::stringstream;

#define QUICKSAVEFILE "quicksave.sav"
#define IS_QUICK true

void SaveLoad::SaveGame(const std::string& tag) {

	// Generate distinct filename
	stringstream stringBuilder;
	stringBuilder << "save" << MD5(tag).hexdigest() << ".sav";
	const char* savefile = stringBuilder.str().c_str();
	 
	// Save
	gEnv->pGame->GetIGameFramework()->SaveGame(savefile);
}

void SaveLoad::QuickSave() {
	gEnv->pGame->GetIGameFramework()->SaveGame(QUICKSAVEFILE, IS_QUICK);
}

void SaveLoad::LoadGame(const std::string& tag) {

	// Recovering save file name
	stringstream stringBuilder;
	stringBuilder << "save" << MD5(tag).hexdigest() << ".sav";
	const char* savefile = stringBuilder.str().c_str();

	// Load
	gEnv->pGame->GetIGameFramework()->LoadGame(savefile);
}

void SaveLoad::QuickLoad() {
	gEnv->pGame->GetIGameFramework()->LoadGame(QUICKSAVEFILE, IS_QUICK);
}


