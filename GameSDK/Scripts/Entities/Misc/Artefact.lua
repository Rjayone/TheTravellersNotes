Artefact =
{
	Properties =
	{
		objModel = "objects/box.cgf",
		sUseText = "Use",
		bUsable = 1,
	},
	Editor = 
	{
		Icon = "user.bmp",
		IconOnTop = 1,
	},
}

function Artefact:GetUsableMessage(idx)
	return "Use Artefact";
end

function Artefact:IsUsable()
	if not self.__usable
		then self.__usable = self.Properties.bUsable
	end;
	return self.__usable;
end

function Artefact:ResetOnUsed()
	self.__usable = nil;
end

function Artefact:OnUsed(user, idx)
	BroadcastEvent(self, "Used");
end
