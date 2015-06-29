#pragma once

// Author : DesertEagle

namespace SaveLoad
{
	// Save logic incapsulation
	void SaveGame(const std::string& tag);

	// Quick save logic incapsulation
	void QuickSave();

	// Load logic incapsulation
	void LoadGame(const std::string& tag);

	// Quick load logic incapsulation
	void QuickLoad();
};

