FirePlace =
{
	Properties =
	{
		sParticle = "",
		objModel = "GameSDK/Objects/misc/Fireplace/fireplace.cgf",
		sUseText = "Start fire",
		bUsable = true,
		clrLightColor = { r = 255, g = 187, b = 50, a = 255 },
		fadeLightTime = 5
	},

	Editor = 
	{
		Icon = "user.bmp",
		IconOnTop = 1
	}
}

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