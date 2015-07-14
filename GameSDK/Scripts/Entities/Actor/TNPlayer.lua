Script.ReloadScript( "SCRIPTS/Entities/actor/player.lua");

TNPlayer = {
	type = "TNPlayer",
	Properties = {
		aicharacter_character = "TNPlayer"
	}
}

-- Inheriting settings from basic player
setmetatable(TNPlayer, player)

-- Creating actor
CreateActor(TNPlayer);

-- Exposing it to the world
TNPlayer:Expose();