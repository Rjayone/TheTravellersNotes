Props = 
{
		Properties =
		{
			objModel = "objects/box.cgf",
			sUseText = "Use",
			bUsable = 1,
			bSit=1,
			bSleep=0,
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


function Props:OnSpawn() 
CryAction.CreateGameObjectForEntity(self.id);
CryAction.ForceGameObjectUpdate(self.id, true); 
self:Reset(1);
end

function Props:DoPhysicalize()
if (self.currModel ~= self.Properties.fileModel) then
CryAction.ActivateExtensionForGameObject(self.id, "ScriptControlledPhysics", false);
self:LoadObject( 0,self.Properties.fileModel );
self:Physicalize(0,PE_STATIC,self.PhysParams);
CryAction.ActivateExtensionForGameObject(self.id, "ScriptControlledPhysics", true); 
end

self:SetPhysicParams(PHYSICPARAM_FLAGS, {flags_mask=pef_cannot_squash_players, flags=pef_cannot_squash_players});
self.currModel = self.Properties.fileModel;
end

function Props:Reset(onSpawn)
self:DoPhysicalize(); 
end

function Props:Event_Change(sender,data)
self:LoadObject( 0,data );
self:ActivateOutput("Done", true);
Log("Done loading");
end
function Props:OnPropertyChange()
	self.bActive = self.Properties.bActive;
end

function Props:GetUsableMessage(idx)
		return "Use Props";
end

function Props:IsUsable()
		if not self._usable
			then self._usable = self.Properties.bUsable
		end;
		return 	self._usable;
end

function Props:ResetOnUsed()
		self._usable = nil;
end

function Props:OnUsed(user, idx)
		BroadcastEvent(self, "Used");
end


