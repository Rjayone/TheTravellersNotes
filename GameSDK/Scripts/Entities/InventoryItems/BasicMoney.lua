BasicMoney = 
{
	Properties = 
	{
		objModel = "Objects/misc/Money/Coin_single_a.cgf", --["All items defined in ItemsDescription.xml"]                            	                                	
		bItemUsable = 1,    
		
		bRandom = 1, --["Set random value when money pick up."]
		bSparkEnable = 0, --["Special spaks for attract attantion"]
		nMoney = 10, --[0,,1,"Disabled when actaveted random"]
		nBagSize = 1, --[0,1,4, "Bag size: 1 - single coin, 2 - 3 coins, 4 - little bag, 5 - medium bag"]
		sParticle = "bullet.hit_metal.c",
	},                                                  
	Editor=                                             
	{                                                    
		Icon = "Item.bmp",                             
		IconOnTop= 1,
	},	
}

function BasicMoney:GetUsableMessage(idx)
	return "Pick";
end

function BasicMoney:OnPropertyChange()
	self:OnReset();
end

function BasicMoney:OnReset()
	BroadcastEvent(self, "Reset");
end