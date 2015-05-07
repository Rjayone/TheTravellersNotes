BasicRecipe = 
{
	Properties = 
	{
		objModel = "Objects/misc/Goods/Recipe/recipe.cgf",
		nItemType = 7, --[0,9,1," Weapon - 1, Armor - 2, 	Food - 3,	QuestItem - 4,	 Flask - 5, Ammo - 6,	Recipe - 7, Special - 8,   Artifact - 9"]
		sRecipeName = "",
		nRecipeType = 1,
		bItemUsable = 1,
	}, 
	
	Editor=
	{
		Icon = "Item.bmp",
		IconOnTop= 1,
	},	
}

function BasicRecipe:OnUsed(user)
	self:Hide(1);
	--BroadcastEvent(self, "Pick");
end


function BasicRecipe:GetUsableMessage(idx)
	return "Pick";
end

function BasicRecipe:OnPropertyChange()
	self:OnReset();
	self:PhysicalizeThis();
end
