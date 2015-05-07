Food = 
{
	Properties = 
	{
		fileModel = "Objects/Weapons/equipment/medical_Kit/medical_kit_bag_tp.cgf",
		fUseDistance = 2.5,
		fRespawnTime = 5.0,
		fHealth = 20.0,
		bUsable = 1,
		sItemName = "Chiken",
		nItemType = 1
	},
  	
	PhysParams = 
	{ 
		mass = 0.5,
		density = 0,
	},
	
	Editor=
	{
		Icon = "Item.bmp",
		IconOnTop=1,
	},	
}
function Food:OnReset()
	self:Reset();
end

function Food:OnSpawn()	
	CryAction.CreateGameObjectForEntity(self.id);
	CryAction.ForceGameObjectUpdate(self.id, true);		
	self:Reset(1);
end



function Food:IsUsable()
	if not self.__usable
		then self.__usable = self.Properties.bUsable
	end;
	return self.__usable;
end

function Food:DoPhysicalize()
	if (self.currModel ~= self.Properties.fileModel) then
		CryAction.ActivateExtensionForGameObject(self.id, "ScriptControlledPhysics", false);
		self:LoadObject( 0,self.Properties.fileModel );
		self:Physicalize(0,PE_RIGID,self.PhysParams);
		CryAction.ActivateExtensionForGameObject(self.id, "ScriptControlledPhysics", true);			
	end
	
		self:SetPhysicParams(PHYSICPARAM_FLAGS, {flags_mask=pef_cannot_squash_players, flags=pef_cannot_squash_players});
	self.currModel = self.Properties.fileModel;
end


function Food:Reset(onSpawn)
	self:DoPhysicalize();	
end


function Food:GetUsableMessage(idx)
	return "@GetFood";
end


function Food:OnUsed(user)
	user.actor:SetHealth(user.actor:GetHealth() + self.Properties.fHealth);
	self:Hide(1);

end