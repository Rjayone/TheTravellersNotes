Backpack = 
{
		Properties =
		{
			objModel = "objects/props/misc/capacity/outdoor/bag.cgf",
			sUseText = "Use",
			bUsable = 1,
		},
		
		PhysParams = 
		{ 
			mass = 1,
			density = 0,
		},
		
		Editor =
		{
			Icon = "user.bmp",
			IconOnTop = 1,	
		},
}


function Backpack:OnSpawn() 
CryAction.CreateGameObjectForEntity(self.id);
CryAction.ForceGameObjectUpdate(self.id, true); 
self:Reset(1);
end

function Backpack:DoPhysicalize()
if (self.currModel ~= self.Properties.fileModel) then
CryAction.ActivateExtensionForGameObject(self.id, "ScriptControlledPhysics", false);
self:LoadObject( 0,self.Properties.fileModel );
self:Physicalize(0,PE_STATIC,self.PhysParams);
CryAction.ActivateExtensionForGameObject(self.id, "ScriptControlledPhysics", true); 
end

self:SetPhysicParams(PHYSICPARAM_FLAGS, {flags_mask=pef_cannot_squash_players, flags=pef_cannot_squash_players});
self.currModel = self.Properties.fileModel;
end

function Backpack:Reset(onSpawn)
self:DoPhysicalize(); 
end

function Backpack:Event_Change(sender,data)
self:LoadObject( 0,data );
self:ActivateOutput("Done", true);
Log("Done loading");
end
function Backpack:OnPropertyChange()
	self.bActive = self.Properties.bActive;
end

function Backpack:GetUsableMessage(idx)
		return "Use Backpack";
end

function Backpack:IsUsable()
		if not self._usable
			then self._usable = self.Properties.bUsable
		end;
		return 	self._usable;
end

function Backpack:ResetOnUsed()
		self._usable = nil;
end

function Backpack:OnUsed(user, idx)
		BroadcastEvent(self, "Used");
end


