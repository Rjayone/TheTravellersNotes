Script.ReloadScript("scripts/Entities/Sound/Shared/AudioUtils.lua");

AudioTriggerSpot = {
	Editor={
		Model="Editor/Objects/Sound.cgf",
		Icon="Sound.bmp",
	},
	
	Properties = {
		bEnabled = true,
		audioTriggerPlayTriggerName = "",
		audioTriggerStopTriggerName = "",
		bSerializePlayState = true, -- Determines if execution after de-serialization is needed.
		eiSoundObstructionType = 1, -- Anything greater than 1 will be reset to 2.
		bPlayOnX = false,
		bPlayOnY = false,
		bPlayOnZ = false,
		fRadiusRandom = 10.0,
		bPlayRandom = false,
		fMinDelay = 1,
		fMaxDelay = 2,
	},
	
	hOnTriggerID = nil,
	hOffTriggerID = nil,
	tObstructionType = {},
	
	bIsHidden = false,
	bIsPlaying = false,
	bHasMoved = false,
}

----------------------------------------------------------------------------------------
function AudioTriggerSpot:_LookupTriggerIDs()
	self.hOnTriggerID = AudioUtils.LookupTriggerID(self.Properties.audioTriggerPlayTriggerName);	
	self.hOffTriggerID = AudioUtils.LookupTriggerID(self.Properties.audioTriggerStopTriggerName);
end

----------------------------------------------------------------------------------------
function AudioTriggerSpot:_LookupObstructionSwitchIDs()
	-- cache the obstruction switch and state IDs
	self.tObstructionType = AudioUtils.LookupObstructionSwitchAndStates();
end

----------------------------------------------------------------------------------------
function AudioTriggerSpot:_SetObstruction()
	local nStateIdx = self.Properties.eiSoundObstructionType + 1;
	
	if ((self.tObstructionType.hSwitchID ~= nil) and (self.tObstructionType.tStateIDs[nStateIdx] ~= nil)) then
		self:SetAudioSwitchState(self.tObstructionType.hSwitchID, self.tObstructionType.tStateIDs[nStateIdx], self:GetDefaultAuxAudioProxyID());
	end
end

----------------------------------------------------------------------------------------
function AudioTriggerSpot:_PlayFirstTime()	
	self:_SetObstruction();
	self:SetCurrentAudioEnvironments();
	
	if (self.Properties.bEnabled and (not self.bIsHidden)) then
		self:Play();
	end	 
end

----------------------------------------------------------------------------------------
function AudioTriggerSpot:_GenerateOffset()
	local offset = {x=0,y=0,z=0}
	local len = 0
	
	if (self.Properties.bPlayOnX) then
		offset.x=randomF(-1,1);
	end
	if (self.Properties.bPlayOnY) then
		offset.y=randomF(-1,1);
	end
	if (self.Properties.bPlayOnZ) then
		offset.z=randomF(-1,1);
	end
	
	NormalizeVector(offset);
	ScaleVectorInPlace(offset, randomF(0,self.Properties.fRadiusRandom));
	
	return offset;
end

----------------------------------------------------------------------------------------
function AudioTriggerSpot:OnSpawn()	
	self:SetFlags(ENTITY_FLAG_CLIENT_ONLY, 0);
end

----------------------------------------------------------------------------------------
function AudioTriggerSpot:OnSave(save)	
	save.Properties = self.Properties;
	save.bIsPlaying = self.bIsPlaying;
end

----------------------------------------------------------------------------------------
function AudioTriggerSpot:OnLoad(load)	
	self:Stop(); -- ??
	
	self.Properties = load.Properties;
	self.bIsPlaying = load.bIsPlaying;
end

----------------------------------------------------------------------------------------
function AudioTriggerSpot:OnPostLoad()	
	-- these functions call SoundProxy, which gets properly reset AFTER OnLoad is executed
	self:_SetObstruction();
	self:SetCurrentAudioEnvironments();
	
	if (self.bIsPlaying and self.Properties.bSerializePlayState) then
		self.bIsPlaying = false;
		self:Play();
	else
		self.bIsPlaying = false;
	end	
end

----------------------------------------------------------------------------------------
function AudioTriggerSpot:_Init()	
	self.bIsPlaying = false;
	self:SetAudioProxyOffset(g_Vectors.v000, self:GetDefaultAuxAudioProxyID());
	self:NetPresent(0);
end

----------------------------------------------------------------------------------------
function AudioTriggerSpot:OnPropertyChange()
	self:Stop();
	self:_LookupTriggerIDs();
	
	if (self.Properties.eiSoundObstructionType < 0) then
		self.Properties.eiSoundObstructionType = 0;
	elseif (self.Properties.eiSoundObstructionType > 1) then
		self.Properties.eiSoundObstructionType = 2;
	end
	
	self:SetAudioProxyOffset(g_Vectors.v000, self:GetDefaultAuxAudioProxyID());
	self:_PlayFirstTime();
end

----------------------------------------------------------------------------------------
function AudioTriggerSpot:OnReset()
	self:Stop();	
	self:_PlayFirstTime();
end

----------------------------------------------------------------------------------------
AudioTriggerSpot["Server"] = {
	OnInit= function (self)
		self:_Init();
	end,
	OnShutDown= function (self)
	end,
}

----------------------------------------------------------------------------------------
AudioTriggerSpot["Client"] = {
	----------------------------------------------------------------------------------------
	OnInit = function(self)
		self:_Init();
		self:_LookupObstructionSwitchIDs();
		self:_LookupTriggerIDs();		
		self:_PlayFirstTime();
	end,
	
	----------------------------------------------------------------------------------------
	OnShutDown = function(self)
		self:Stop();
	end,
	
	----------------------------------------------------------------------------------------
	OnSoundDone = function(self, hTriggerID)
		if (self.hOnTriggerID == hTriggerID) then
			self:ActivateOutput( "Done",true );
		end
	end,
	
	----------------------------------------------------------------------------------------
    OnTimer = function(self, timerid, msec)    
        if (timerid == 0) then
            self:Play();
        end
    end,
	
	----------------------------------------------------------------------------------------
	OnHidden = function(self)
		self:Stop();
		self.bIsHidden = true;
	end,
 
	----------------------------------------------------------------------------------------
	OnUnHidden = function(self)
		self.bIsHidden = false;
		self:_PlayFirstTime();
	end,
	
	----------------------------------------------------------------------------------------
	OnMove = function(self)
		self.bHasMoved = true;
	end,
}

----------------------------------------------------------------------------------------
function AudioTriggerSpot:Play()
	if ((self.hOnTriggerID ~= nil) and self.Properties.bEnabled) then
		self:Stop();
		
		local offset = self:_GenerateOffset();
		if (LengthSqVector(offset) > 0.00001) then-- offset is longer than 1cm
			self:SetAudioProxyOffset(offset, self:GetDefaultAuxAudioProxyID());
			self:SetCurrentAudioEnvironments();
		elseif (self.bHasMoved) then
			self:SetCurrentAudioEnvironments();
		end
		
		self:ExecuteAudioTrigger(self.hOnTriggerID, self:GetDefaultAuxAudioProxyID());
		self.bIsPlaying = true;
		self.bHasMoved = false;
		
		if (self.Properties.bPlayRandom) then
			self:SetTimer(0, 1000 * randomF(self.Properties.fMinDelay, self.Properties.fMaxDelay));
		end
	end
end

----------------------------------------------------------------------------------------
function AudioTriggerSpot:Stop()	
	if (self.bIsPlaying) then
		if (self.hOffTriggerID ~= nil) then
			self:ExecuteAudioTrigger(self.hOffTriggerID, self:GetDefaultAuxAudioProxyID());
		else
			self:StopAudioTrigger(self.hOnTriggerID, self:GetDefaultAuxAudioProxyID());
		end
		
		self.bIsPlaying = false;
		self:KillTimer(0);
	end
end

------------------------------------------------------------------------------------------------------
-- Event Handlers
------------------------------------------------------------------------------------------------------
function AudioTriggerSpot:Event_Enable( sender )	
	if (not self.Properties.bEnabled) then
		self.Properties.bEnabled = true;
		self:Play();
	end
	--BroadcastEvent( self,"Enable" );
end

----------------------------------------------------------------------------------------
function AudioTriggerSpot:Event_Disable( sender )	
	self:Stop();
	self.Properties.bEnabled = false;
  --BroadcastEvent( self,"Disable" );
end

----------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------
AudioTriggerSpot.FlowEvents =
{
	Inputs =
	{
		Enable = { AudioTriggerSpot.Event_Enable, "bool" },
		Disable = { AudioTriggerSpot.Event_Disable, "bool" },
	},
	
	Outputs =
	{
	  Done = "bool",
	},
}


