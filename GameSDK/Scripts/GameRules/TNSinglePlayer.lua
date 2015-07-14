Script.ReloadScript("scripts/gamerules/GameRulesUtils.lua");

-- Creating Traveller Notes single player game rules
TNSinglePlayer = {}

GameRulesSetStandardFuncs(TNSinglePlayer);

usableEntityList = {}

if (not g_dmgMult) then
    g_dmgMult = 1.0;
end

function TNSinglePlayer:OnInit()
    Log("[TNSinglePlayer] OnInit called!");
end

function TNSinglePlayer:Init()
    Log("[TNSinglePlayer] Init called!");
end

----------------------------------------------------------------------------------------------------
--- Spawning the player
----------------------------------------------------------------------------------------------------
function TNSinglePlayer.Server:OnClientConnect(channelId)
    Log("[TNSinglePlayer] Player connected!");
    local params =
    {
        name = "Traveller",
        class = "Player",
        position = { x = 0, y = 0, z = 0 },
        rotation = { x = 0, y = 0, z = 0 },
        scale = { x = 1, y = 1, z = 1 },
    };
    player = Actor.CreateActor(channelId, params);

    if (not player) then
        Log("OnClientConnect: Failed to spawn the player!");
        return;
    end

    local spawnId = self.game:GetFirstSpawnLocation(0);
    if (spawnId) then
        local spawn = System.GetEntity(spawnId);
        if (spawn) then
            --set pos
            player:SetWorldPos(spawn:GetWorldPos(g_Vectors.temp_v1));
            --set angles
            spawn:GetAngles(g_Vectors.temp_v1);
            g_Vectors.temp_v1.x = 0;
            g_Vectors.temp_v1.y = 0;
            player.actor:PlayerSetViewAngles(g_Vectors.temp_v1);
            spawn:Spawned(player);

            return;
        end
    end

    System.Log("$1warning: No spawn points; using default spawn location!")
end

----------------------------------------------------------------------------------------------------
--- Function to equip an actor
--- called from BasicActor:Reset
----------------------------------------------------------------------------------------------------
function TNSinglePlayer:EquipActor(actor)
    Log("[TNSinglePlayer]:EquipActor(%s) <<", actor:GetName());

    if (self.game:IsDemoMode() ~= 0) then -- don't equip actors in demo playback mode, only use existing items
    --Log("Don't Equip : DemoMode");
    return;
    end;

    if (actor.inventory) then
        actor.inventory:Destroy();
    end

    if (actor.actor and actor.actor:IsPlayer()) then
        ItemSystem.GiveItemPack(actor.id, "Player_Default", false, true);
    end

    if (actor.actor and not actor.actor:IsPlayer()) then
        if (actor.Properties) then
            local equipmentPack = actor.Properties.equip_EquipmentPack;
            if (equipmentPack and equipmentPack ~= "") then
                ItemSystem.GiveItemPack(actor.id, equipmentPack, false, false);
                if (actor.AssignPrimaryWeapon) then
                    actor:AssignPrimaryWeapon();
                end
            end

            if (not actor.bGunReady) then
                actor.actor:HolsterItem(true);
            end
        end
    end
end


----------------------------------------------------------------------------------------------------
function TNSinglePlayer:OnShoot(shooter)
    if (shooter and shooter.OnShoot) then
        if (not shooter:OnShoot()) then
            return false;
        end
    end

    return true;
end

----------------------------------------------------------------------------------------------------
function TNSinglePlayer:IsUsable(srcId, objId)
    if not objId then return 0 end;

    local obj = System.GetEntity(objId);
    if (obj.IsUsable) then
        if (obj:IsHidden()) then
            return 0;
        end;
        local src = System.GetEntity(srcId);
        if (src and src.actor and (src:IsDead() or (src.actor:GetSpectatorMode() ~= 0))) then
            return 0;
        end
        return obj:IsUsable(src);
    end

    return 0;
end

----------------------------------------------------------------------------------------------------
function TNSinglePlayer:AreUsable(source, entities)

    if (entities) then
        for i, entity in ipairs(entities) do
            usableEntityList[i] = entity:IsUsable(source);
        end
    end

    return usableEntityList;
end

----------------------------------------------------------------------------------------------------
function TNSinglePlayer:OnNewUsable(srcId, objId, usableId)
    if not srcId then return end
    if objId and not System.GetEntity(objId) then objId = nil end

    local src = System.GetEntity(srcId)
    if src and src.SetOnUseData then
        src:SetOnUseData(objId or NULL_ENTITY, usableId)
    end

    if srcId ~= g_localActorId then return end

    if self.UsableMessage then
        HUD.SetInstructionObsolete(self.UsableMessage)
        self.UsableMessage = nil
    end
end


----------------------------------------------------------------------------------------------------
function TNSinglePlayer:OnUsableMessage(srcId, objId, objEntityId, usableId)
    if srcId ~= g_localActorId then return end

    local msg = "";

    if objId then
        obj = System.GetEntity(objId)
        if obj then

            if obj.GetUsableMessage then
                msg = obj:GetUsableMessage(usableId)
            else
                local state = obj:GetState()
                if state ~= "" then
                    state = obj[state]
                    if state.GetUsableMessage then
                        msg = state.GetUsableMessage(obj, usableId)
                    end
                end
            end
        end
    end

    if (UIAction) then
        UIAction.StartAction("DisplayUseText", { msg }); --this triggers the UIAction "DisplayUseText" and pass the msg as argument (see FlowGraph UIActions how to send msg to flash)
    end
end


----------------------------------------------------------------------------------------------------
function TNSinglePlayer:OnLongHover(srcId, objId)
end


----------------------------------------------------------------------------------------------------
function TNSinglePlayer:EndLevel(params)
    if (not System.IsEditor()) then
        if (not params.nextlevel) then
            Game.PauseGame(true);
            Game.ShowMainMenu();
        end
        g_GameTokenPreviousLevel = GameToken.GetToken("Game.Global.Previous_Level");
    end
end
----------------------------------------------------------------------------------------------------
function TNSinglePlayer:OnReset(toGame)
    AIReset();
end
----------------------------------------------------------------------------------------------------
function TNSinglePlayer:OnSpawn()
end


----------------------------------------------------------------------------------------------------
function TNSinglePlayer.Server:OnInit()

    Log("[TNSinglePlayer] Server OnInit called!");

    self.fallHit = {};
    self.explosionHit = {};
    self.collisionHit = {};
end


----------------------------------------------------------------------------------------------------
function TNSinglePlayer.Client:OnInit()
    Log("[TNSinglePlayer] Traveller notes game rules initialized on client")
end

----------------------------------------------------------------------------------------------------
function TNSinglePlayer.Server:OnClientEnteredGame(channelId, player, loadingSaveGame)
    Log("[TNSinglePlayer] OnClientEnteredGame")
end

----------------------------------------------------------------------------------------------------
function TNSinglePlayer:GetDamageAbsorption(actor, hit)
    return 0
end


----------------------------------------------------------------------------------------------------
function TNSinglePlayer:CanHitIgnoreInvulnerable(hit, target)

    if (self:IsStealthHealthHit(hit.type)) then
        return true;
    elseif (hit.type == "silentMelee") then
        return true;
    end
end


----------------------------------------------------------------------------------------------------
function TNSinglePlayer:ProcessActorDamage(hit)
    local target = hit.target;
    local shooter = hit.shooter;
    local shooterId = hit.shooterId or NULL_ENTITY;
    local weapon = hit.weapon;
    local health = target.actor:GetHealth();

    if (target.IsInvulnerable and target:IsInvulnerable() and not self:CanHitIgnoreInvulnerable(hit, target)) then
        return (health <= 0);
    end;

    local isMultiplayer = self.game:IsMultiplayer();
    local totalDamage = g_dmgMult * hit.damage;

    local splayer = shooter and shooter.actor and shooter.actor:IsPlayer();
    local tplayer = target and target.actor and target.actor:IsPlayer();

    if (not isMultiplayer) then

        local sai = (not splayer) and shooter and shooter.actor;
        local tai = (not tplayer) and target and target.actor;

        local dmgMult = 1.0;
        if (tplayer) then
            dmgMult = g_dmgMult;
        end

        if (shooter and shooter.actor and tai) then
            -- Make the target AI alarmed
            AI.SetAlarmed(target.id);
        end

        if (AI) then
            if (sai and not tai) then
                -- AI vs. player
                totalDamage = AI.ProcessBalancedDamage(shooterId, target.id, dmgMult * hit.damage, hit.type);
                totalDamage = totalDamage * (1 - self:GetDamageAbsorption(target, hit));
                --totalDamage = dmgMult*hit.damage*(1-target:GetDamageAbsorption(hit.type, hit.damage));
            elseif (sai and tai) then
                -- AI vs. AI
                totalDamage = AI.ProcessBalancedDamage(shooterId, target.id, dmgMult * hit.damage, hit.type);
                totalDamage = totalDamage * (1 - self:GetDamageAbsorption(target, hit));
            else
                totalDamage = dmgMult * hit.damage * (1 - self:GetDamageAbsorption(target, hit));
            end
        else
            totalDamage = dmgMult * hit.damage * (1 - self:GetDamageAbsorption(target, hit));
        end
    end

    if (tplayer and (hit.damage > 0) and (hit.type == "collision")) then
        if (hit.velocity and hit.velocity > 0.5) then
            totalDamage = 0;
        end
    end

    --update the health
    local newhealth = math.floor(health - totalDamage);
    local useMercyTime = not isMultiplayer and (hit.type ~= "fall") and (hit.type ~= "punish") and (hit.type ~= "vehicleDestruction");
    if (tplayer and useMercyTime) then
        local threshold = target.actor:GetLowHealthThreshold();
        if (health > threshold and newhealth <= 0) then
            --Log("Prevented %s's one-shot!", target:GetName())
            --newhealth=math.floor(threshold*0.5); --enable this if you want to have mercy on the player for one-shotting him through falling or vehicleDestroyed damage
        end
    end

    -- For boss fights and such it is sometimes desirable that the health
    -- cannot drop below a certain minimum threshold (but never ignore
    -- custom kill events from FlowGraph and stuff!)
    if (hit.type ~= "event") then
        if (target.GetForcedMinHealthThreshold) then
            local forcedMinHealth = target:GetForcedMinHealthThreshold()
            if (newhealth < forcedMinHealth) then
                newhealth = forcedMinHealth
            end
        end
    end

    health = newhealth;

    --keep debug features commented out if not frequently used (useless c++ call in release)
    --if (self.game:DebugCollisionDamage()>0) then
    --  Log("<%s> hit damage: %d // absorbed: %d // health: %d", target:GetName(), hit.damage, hit.damage*self:GetDamageAbsorption(target, hit), health);
    --end

    -- Check for player god / demi-god death
    if ((not isMultiplayer) and (target.id == g_localActorId) and (health <= 0)) then
        self.game:DemiGodDeath(); -- Attemp demi-god death if enabled (SP only internally)

        local isGod = target.actor:IsGod();
        if (isGod and isGod > 0) then
            target.actor:SetHealth(0); --is only called to count deaths in GOD mode within C++
            health = target.Properties.Damage.health;
        end
    end

    target.actor:SetHealth(health);
    health = target.actor:GetHealth();

    if (not isMultiplayer) then
        -- Death wall bloodsplats are a prototype feature: Only supported on SinglePlayer for now
        -- Enabling it on Multiplayer will involve placing this call in an area executed in both server
        -- and client sides, where we would have the final damage caused by the hit and can tell if
        -- the actor is dead or not (harder than it seems :P)
        target:WallBloodSplat(hit, health <= 0);
    end

    local weaponId = (weapon and weapon.id) or NULL_ENTITY;
    local projectileId = hit.projectileId or NULL_ENTITY;
    target.actor:DamageInfo(shooterId, target.id, weaponId, projectileId, totalDamage, hit.typeId, hit.dir);

    -- feedback the information about the hit to the AI system.
    if (not isMultiplayer and AI) then
        if (hit.material_type) then
            AI.DebugReportHitDamage(target.id, shooterId, totalDamage, hit.material_type);
        else
            AI.DebugReportHitDamage(target.id, shooterId, totalDamage, "");
        end
    end

    return (health <= 0);
end

----------------------------------------------------------------------------------------------------
function TNSinglePlayer.Server:OnStartLevel()
    CryAction.SendGameplayEvent(NULL_ENTITY, eGE_GameStarted);
    if (g_GameTokenPreviousLevel) then
        GameToken.SetToken("Game.Global.Previous_Level", g_GameTokenPreviousLevel);
        g_GameTokenPreviousLevel = nil;
    end
end


----------------------------------------------------------------------------------------------------
function TNSinglePlayer.Client:OnStartLevel()
end

----------------------------------------------------------------------------------------------------
function TNSinglePlayer.Client:OnHit(hit)
    local trg = hit.target;

    -- send hit to target
    if (trg and (not hit.backface) and trg.Client and trg.Client.OnHit) then
        trg.Client.OnHit(trg, hit);
    end
end

----------------------------------------------------------------------------------------------------
function TNSinglePlayer:PrecacheLevel()
end

----------------------------------------------------------------------------------------------------
function TNSinglePlayer:IsStealthHealthHit(hitType)
    return (hitType == "stealthKill") or (hitType == "stealthKill_Maximum");
end
