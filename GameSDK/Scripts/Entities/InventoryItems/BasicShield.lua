BasicShield = 
{
	Properties = 
	{
		objModel = "objects/weapons/kaplewidni_hit/kaplewidni_hit.cgf", --["All items defined in ItemsDescription.xml"]
		nItemType = 4, --[0,9,1," Weapon-1, Armor-2, Food-3, QuestItem-4, Flask-5, Ammo-6, Recipe-7, Special-8, Artifact-9"]
		sItemName = "",                               	                                	
		bItemUsable = 1, 
		nWearout = 0, --[0,99,1, ""]

		fMass = 5,
	},                                                  
	Editor=                                             
	{                                                    
		Icon = "Item.bmp",                             
		IconOnTop= 1,
	},	
}

function BasicShield:GetUsableMessage(idx)
	return "Pick";
end

function BasicShield:OnPropertyChange()
	BroadcastEvent(self, "OnPropertyChange");
end

function BasicShield:OnReset()
	BroadcastEvent(self, "Reset");
end