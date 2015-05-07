BasicObject = 
{
	Properties = 
	{
		objModel = "Objects/misc/Goods/Recipe/recipe.cgf", --["All items defined in ItemsDescription.xml"]
		eiItemType = 1,
		sItemName = "",                               	                                	
		bItemUsable = 1,    

		fMass = 1,
	},                                                  
	Editor=                                             
	{                                                    
		Icon = "Item.bmp",                             
		IconOnTop= 1,
	},	
}

function BasicObject:GetUsableMessage(idx)
	return "Pick";
end

function BasicObject:OnPropertyChange()
	BroadcastEvent(self, "OnPropertyChange");
end

function BasicObject:OnReset()
	BroadcastEvent(self, "Reset");
end