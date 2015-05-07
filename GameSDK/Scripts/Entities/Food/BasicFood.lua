BasicFood = 
{
	Properties = 
	{
		objModel = "Objects/Weapons/equipment/medical_Kit/medical_kit_bag_tp.cgf",
		bItemUsable = 1,
		nItemType = 3, --[0,9,1," Weapon-1, Armor-2, Food-3, QuestItem-4, Flask-5, Ammo-6, Recipe-7, Special-8, Artifact-9"]
		nFoodType = 2, --[0,4,1, "Light - 0, Normal - 1,  Satiety - 2,  HPFlask - 3, StrenghtFlask - 4"]
		sItemName = "Chicken",    			
	},                            			
	Editor=                      			
	{                             
		Icon = "Item.bmp",
		IconOnTop= 1,
	},	
}

function BasicFood:GetUsableMessage(idx)
	return "@GetFood";
end

function BasicFood:OnUsed(user)
	BroadcastEvent(self, "Used");
end

function BasicFood:OnPropertyChange()
	self:OnReset();
end

function BasicFood:OnReset()
	BroadcastEvent(self, "Reset");
end