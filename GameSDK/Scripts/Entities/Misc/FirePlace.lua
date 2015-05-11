FirePlace =
{
	Properties =
	{
		sParticle = "",
		objModel = "GameSDK/Objects/misc/Fireplace/fireplace.cgf",
		sUseText = "Start fire",
		bUsable = true,
		clrLightColor = { r = 255, g = 196, b = 3, a = 255 },
		fLightColorMultiplier = 10.0,		
		fFadeLightTime = 3.0
	},

	Editor = 
	{
		Icon = "user.bmp",
		IconOnTop = 1
	}
}

function FirePlace:OnInit(idx)
	Log("====Fireplace initialized====");
	Log("====Fireplace is named as : " .. self:GetName() .. "====");
end

function FirePlace:GetUsableMessage(idx)
	return self.Properties.sUseText;
end

function FirePlace:IsUsable(userId)
	return self.Properties.bUsable and 1 or 0;
end

function FirePlace:OnUsed(user, idx)
	Log("====Fireplace used====");
	BroadcastEvent(self, "Used");
end