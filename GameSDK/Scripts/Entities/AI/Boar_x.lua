Script.ReloadScript( "SCRIPTS/Entities/AI/Shared/AIBase.lua");
Boar_x = 
{
	IsAIControlled = function()
		return true
	end,
	
	ActionController = "Animations/Mannequin/ADB/BoarControllerDefs.xml",
	AnimDatabase3P = "Animations/Mannequin/ADB/Boar.adb",
	SoundDatabase = "Animations/Mannequin/ADB/humanSounds.adb",

	UseMannequinAGState = true,
	UseLegacyCoverLocator = false,
	
	PropertiesInstance = {
 		AITerritoryAndWave = {
			aiterritory_Territory = "<None>",
			aiwave_Wave           = "<None>",
		},
		soclasses_SmartObjectClass = "",
		groupid = -1,
		bAutoDisable = 0,
	}, 
  
	Properties = 
	{	
		fileModel = "objects/characters/animals/wild_boar/wild_boar.chr",
		shadowFileModel = "objects/characters/animals/wild_boar/wild_boar.chr",
		clientFileModel = "objects/characters/animals/wild_boar/wild_boar.chr",
		
		esBehaviorSelection = "",
		esModularBehaviorTree = "",
		esNavigationType = "MediumSizedCharacters",
		fileHitDeathReactionsParamsDataFile = "Libs/HitDeathReactionsData/HitDeathReactions_SDKGrunt.xml",

		aibehavior_behaviour = "",
		aicharacter_character = "",
		
		esVoice = "AI_01",
		esCommConfig = "",
		esFaction = "Grunts",
		commrange = 15.0,

		aicharacter_character = "Player",
		soclasses_SmartObjectClass = "Actor",
		equip_EquipmentPack="",
		
		Damage =
		{
			health = 500,
			fileBodyDamage = "Libs/BodyDamage/BodyDamage_Default.xml",
			fileBodyDamageParts = "Libs/BodyDamage/Bodyparts_HumanMaleShared.xml",
			fileBodyDestructibility = "Libs/BodyDamage/BodyDestructibility_HumanMaleShared.xml",

			CanFall = 1,
			FallSleepTime = 1,
			heatAbsobsion = 0.5,		-- how much real damage is absorbed when actor is hit by mike particles. 0 takes all damage, 1 takes no damage at all.
			heatDissipation = 200,		-- how much heat damage points are dissipated per second
			minHeatDamage = 100,		-- the minumum heat damage needed to burst an enemy
		},

		Perception =
		{
			minAlarmLevel = 0,

			sightEnvScaleNormal = 0.2,
			sightEnvScaleAlarmed = 1,

			cloakMaxDistCrouchedAndMoving = 1.5,
			cloakMaxDistCrouchedAndStill = 1.3,
			cloakMaxDistMoving = 3.0,
			cloakMaxDistStill = 2.0,
			config = "Human",
		},

		useSpecialMovementTransitions = 0,
		bOverrideAllowLookAimStrafing = 0,

		bUseFacialFrameRateLimiting = 0,

		AIBehaviorFlags =
		{
			bSniper = 0,
			bGrenadier = 0,
		},

		AI =
		{
			bUseRadioChatter = 0,
			bPlayDeathSound = 0,
			sndDeathSound = "",
		},

		PlayerInteractions =
		{
			bStealthKill = 0,
			bCanBeGrabbed = 0,
			esGrabType = "Human"
		},

	},
	
	IKLimbs =
	{
	},
	
	gameParams =
	{			
		physicsParams =
		{
			Living = 
			{
				inertia = 0.0,
				inertiaAccel = 0.0,
			},
		},
		
		autoAimTargetParams =
		{
			primaryTargetBone = BONE_SPINE,
			physicsTargetBone = BONE_SPINE,
			secondaryTargetBone = BONE_HEAD,
			fallbackOffset = 1.2,
			innerRadius = 1.0,
			outerRadius = 3.5,
			snapRadius = 1.0,
			snapRadiusTagged = 1.0,
		},
		
		boneIDs =
		{
			BONE_BIP01 = "Bip01",
			BONE_SPINE = "Bip01 spine_1",
			BONE_SPINE2 = "Bip01 spine_2",
			BONE_SPINE3 = "",
			BONE_HEAD = "Bip01 head",
			BONE_EYE_R = "",
			BONE_EYE_L = "",
			BONE_WEAPON = "",
			BONE_WEAPON2 = "",
			BONE_FOOT_R = "",
			BONE_FOOT_L = "",
			BONE_ARM_R = "",
			BONE_ARM_L = "",
			BONE_CALF_R = "",
			BONE_CALF_L = "",
			BONE_CAMERA = "",
			BONE_HUD = "",
		},
		meeleHitRagdollImpulseScale = 1.0, -- Scales melee impulse force (when being hit)
		grenadeLaunchProbability = 0.5,
		grenadeThrowMaxAngle =  45,

		lookFOV				= 200,	-- Total FOV for looking (degrees)
		lookInVehicleFOV	= 110,	-- Total FOV for looking in vehicles (degrees)
		aimFOV				= 200,	-- Total FOV for aiming  (degrees)
		maxLookAimAngle		= 0,	-- Maximum angle between aim and look direction (degrees)
		aimIKFadeDuration = 1, -- time to reach aim pose
		aimIKLayer = -1,

		proceduralLeaningFactor = 1, -- disable procedural leaning by default for all humans (infected don't need it, and the others use turn assets)

		canUseComplexLookIK = false,
		lookAtSimpleHeadBone = "",

		cornerSmoother = 1, -- 1 = C2 method; 2 = C3 method

		stepThresholdTime = 1, -- Duration (seconds) the current position deviation needs to be above stepThresholdDistance before the character steps
		stepThresholdDistance = 0.6, -- Distance (meters) that the character has to deviate from the entity before the step timer (stepThresholdTime) steps

		turnThresholdTime = 0.5, -- Duration (seconds) the current angle deviation needs to be above turnThresholdAngle before the character turns
		turnThresholdAngle = 5,  -- Angle (degrees) that the character has to deviate from the entity before the turn timer (turnThresholdTime) turns

		maxDeltaAngleRateNormal = 180, -- Maximum turnspeed (degrees/second)

		nearbyRange = 10, -- Range used to consider a target as nearby
		middleRange = 30, -- Range used to consider a target as in middle distance

		defaultStance = STANCE_RELAXED,

		stance =
		{
			{
				stanceId = STANCE_STAND,
				normalSpeed = 1.0,
				maxSpeed = 50.0,
				heightCollider = 1.2,
				heightPivot = 0.0,
				size = {x=0.5,y=0.5,z=0.2},
				modelOffset = {x=0,y=-0.0,z=0},
				viewOffset = {x=0,y=0.10,z=1.625},
				weaponOffset = {x=0.2,y=0.0,z=1.35},
				leanLeftViewOffset = {x=-0.5,y=0.10,z=1.525},
				leanRightViewOffset = {x=0.5,y=0.10,z=1.525},
				leanLeftWeaponOffset = {x=-0.45,y=0.0,z=1.30},
				leanRightWeaponOffset = {x=0.65,y=0.0,z=1.30},
				peekOverViewOffset = {x=0.1,y=0.0,z=1.5},
				peekOverWeaponOffset = {x=0.1,y=0.0,z=1.4},
				name = "stand",
				useCapsule = 1,
			},
			{
				stanceId = STANCE_ALERTED,
				normalSpeed = 1.0,
				maxSpeed = 50.0,
				heightCollider = 1.2,
				heightPivot = 0.0,
				size = {x=0.5,y=0.5,z=0.2},
				modelOffset = {x=0,y=-0.0,z=0},
				viewOffset = {x=0,y=0.10,z=1.625},
				weaponOffset = {x=0.2,y=0.0,z=1.35},
				leanLeftViewOffset = {x=-0.5,y=0.10,z=1.525},
				leanRightViewOffset = {x=0.5,y=0.10,z=1.525},
				leanLeftWeaponOffset = {x=-0.45,y=0.0,z=1.30},
				leanRightWeaponOffset = {x=0.65,y=0.0,z=1.30},
				peekOverViewOffset = {x=0.1,y=0.0,z=1.5},
				peekOverWeaponOffset = {x=0.1,y=0.0,z=1.4},
				name = "alerted",
				useCapsule = 1,
			},
			{
				stanceId = STANCE_CROUCH,
				normalSpeed = 0.5,
				maxSpeed = 50.0,
				heightCollider = 0.8,
				heightPivot = 0.0,
				size = {x=0.5,y=0.5,z=0.1},
				modelOffset = {x=0.0,y=0.0,z=0},
				viewOffset = {x=0,y=0.0,z=0.9},
				weaponOffset = {x=0.2,y=0.0,z=0.85},
				leanLeftViewOffset = {x=-0.55,y=0.0,z=1.1},
				leanRightViewOffset = {x=0.65,y=0.0,z=0.9},
				leanLeftWeaponOffset = {x=-0.5,y=0.0,z=0.85},
				leanRightWeaponOffset = {x=0.5,y=0.0,z=0.75},
				peekOverViewOffset = {x=0.1,y=0.0,z=1.5},
				peekOverWeaponOffset = {x=0.1,y=0.0,z=1.4},
				name = "crouch",
				useCapsule = 1,
			},
			{
				stanceId = STANCE_LOW_COVER,
				normalSpeed = 0.5,
				maxSpeed = 50.0,
				heightCollider = 0.8,
				heightPivot = 0.0,
				size = {x=0.5,y=0.5,z=0.1},
				modelOffset = {x=0.0,y=0.0,z=0},
				viewOffset = {x=0,y=0.0,z=0.9},
				weaponOffset = {x=0.2,y=0.0,z=0.85},

				leanLeftViewOffset = {x=-0.75,y=0.0,z=0.9},
				leanRightViewOffset = {x=0.75,y=0.0,z=0.95},
				leanLeftWeaponOffset = {x=-0.75,y=0.0,z=0.6},
				leanRightWeaponOffset = {x=0.8,y=0.0,z=0.8},

				whileLeanedLeftViewOffset = {x=0.2,y=0.4,z=0.85},
				whileLeanedRightViewOffset = {x=0.2,y=0.3,z=1.0},
				whileLeanedLeftWeaponOffset = {x=0.25,y=0.4,z=0.8},
				whileLeanedRightWeaponOffset = {x=0.25,y=0.1,z=0.8},

				peekOverViewOffset = {x=0.1,y=0.0,z=1.5},
				peekOverWeaponOffset = {x=0.1,y=0.0,z=1.4},
				name = "coverLow",
				useCapsule = 1,
			},
			{
				stanceId = STANCE_HIGH_COVER,
				normalSpeed = 1.0,
				maxSpeed = 50.0,
				heightCollider = 1.2,
				heightPivot = 0.0,
				size = {x=0.5,y=0.5,z=0.2},
				modelOffset = {x=0,y=-0.0,z=0},
				viewOffset = {x=0,y=0.10,z=1.625},
				weaponOffset = {x=0.2,y=0.0,z=1.35},

				leanLeftViewOffset = {x=-0.7,y=0.10,z=1.525},
				leanRightViewOffset = {x=0.95,y=0.10,z=1.525},
				leanLeftWeaponOffset = {x=-0.6,y=0.10,z=1.30},
				leanRightWeaponOffset = {x=1.0,y=0.10,z=1.30},

				whileLeanedLeftViewOffset = {x=0.1,y=0.1,z=1.5},
				whileLeanedRightViewOffset = {x=0.25,y=0.2,z=1.55},
				whileLeanedLeftWeaponOffset = {x=0.15,y=0.1,z=1.35},
				whileLeanedRightWeaponOffset = {x=0.3,y=0.2,z=1.45},

				peekOverViewOffset = {x=0.1,y=0.0,z=1.5},
				peekOverWeaponOffset = {x=0.1,y=0.0,z=1.4},

				name = "coverHigh",
				useCapsule = 1,
			},
			{
				stanceId = STANCE_SWIM,
				normalSpeed = 1.0, -- this is not even used?
				maxSpeed = 50.0, -- this is ignored, overridden by pl_swim* cvars.
				heightCollider = 0.9,
				heightPivot = 0.5,
				size = {x=0.5,y=0.5,z=0.1},
				modelOffset = {x=0,y=0,z=0.0},
				viewOffset = {x=0,y=0.1,z=0.5},
				weaponOffset = {x=0.2,y=0.0,z=0.3},
				peekOverViewOffset = {x=0.1,y=0.0,z=1.5},
				peekOverWeaponOffset = {x=0.1,y=0.0,z=1.4},
				name = "swim",
				useCapsule = 1,
			},
			--AI states
			{
				stanceId = STANCE_RELAXED,
				normalSpeed = 1.0,
				maxSpeed = 50.0,
				heightCollider = 1.2,
				heightPivot = 0.0,
				size = {x=0.5,y=0.5,z=0.2},
				modelOffset = {x=0,y=0,z=0},
				viewOffset = {x=0,y=0.10,z=1.625},
				weaponOffset = {x=0.2,y=0.0,z=1.3},
				peekOverViewOffset = {x=0.1,y=0.0,z=1.5},
				peekOverWeaponOffset = {x=0.1,y=0.0,z=1.4},
				name = "relaxed",
				useCapsule = 1,
			},
		},
		
		characterDBAs =
		{
			"Boar3p",
		},
		
		lookFOV = 90,
		aimFOV = 180,
	},
	
AIMovementAbility =
	{
		allowEntityClampingByAnimation = 1,
		usePredictiveFollowing = 1,
		pathLookAhead = 1,
		walkSpeed = 2.0, -- set up for humans
		runSpeed = 6,
		sprintSpeed = 10,
		maneuverSpeed = 1.5,
		b3DMove = 0,
		minTurnRadius = .2,	-- meters
		maxTurnRadius = 3,	-- meters
		pathSpeedLookAheadPerSpeed = -1.5,
		cornerSlowDown = 0.75,
		pathType = AIPATH_HUMAN,
		pathRadius = 0.25,
		passRadius = 0.25,

		distanceToCover = 1, -- needs to be at least 20cm more than max(passRadius, pathRadius)
		inCoverRadius = 0.075,
		effectiveCoverHeight = 1,
		effectiveHighCoverHeight = 1.75,

		pathFindPrediction = 0.5,		-- predict the start of the path finding in the future to prevent turning back when tracing the path.
		maxAccel = 3,
		maxDecel = 4.0,
		velDecay = 1,
		maneuverTrh = 2.0,  -- when cross(dir, desiredDir) > this use manouvering
		resolveStickingInTrace = 1,
		pathRegenIntervalDuringTrace = -1,
		lightAffectsSpeed = 1,

		avoidanceAbilities = AVOIDANCE_ALL,
		pushableObstacleWeakAvoidance = true,
		pushableObstacleAvoidanceRadius = 0.4,

		-- These are actually aiparams (as they may be changed during game and need to get serialized),
		-- but defined here so that designers do not try to change them.
		lookIdleTurnSpeed = 30,
		lookCombatTurnSpeed = 50,
		aimTurnSpeed = -1, --120,
		fireTurnSpeed = -1, --120,

		-- Adjust the movement speed based on the angle between body dir and move dir.
		directionalScaleRefSpeedMin = 1.0,
		directionalScaleRefSpeedMax = 8.0,

		AIMovementSpeeds =
		{
			--            { default, min, max }
			Relaxed =
			{
				Slow =      { .3,     .3, .3 },
				Walk =      { 1.1,     1.1, 1.1 },
				Run =       { 6,     6, 6 },
				Sprint =    { 10,     10, 10 },
			},
			Alerted =
			{
				Slow =      { .3,     .3, .3 },
				Walk =      { 1.1,     1.1, 1.1 },
				Run =       { 6,     6, 6 },
				Sprint =    { 10,     10, 10 },
			},
			Combat =
			{
				Slow =      { .3,     .3, .3 },
				Walk =      { 1.1,     1.1, 1.1 },
				Run =       { 6,     6, 6 },
				Sprint =    { 9,     8, 10 }, -- Min should ideally be: Max - (<Idle2Move duration> * maxAccel)
			},
			Crouch =
			{
				Slow =      { 0.8,     0.8, 0.8 },
				Walk =      { 1.3,     1.3, 1.3 },
				Run =       { 2.0,     2.0, 2.0 },
				Sprint =    { 2.0,     2.0, 2.0 },
			},
			LowCover =
			{
				Slow =      { 0.9,     0.9, 0.9 },
				Walk =      { 0.9,     0.9, 0.9 },
				Run =       { 1.8,     1.8, 1.8 },
				Sprint =    { 1.8,     1.8, 1.8 },
			},
			HighCover =
			{
				Slow =      { 1.3,     1.3, 1.3 },
				Walk =      { 1.3,     1.3, 1.3 },
				Run =       { 1.8,     1.8, 1.8 },
				Sprint =    { 1.8,     1.8, 1.8 },
			},
			Swim =
			{
				Slow =      { 1.0,     1.0, 1.0 },
				Walk =      { 1.0,     1.0, 1.0 },
				Run =       { 3.5,     3.5, 3.5 },
				Sprint =    { 5.0,     5.0, 5.0 },
			},
		},
	},
		
	_Parent={},
}
HumanPostures =
{
	{
		name = "LowCoverPeek",
		templateOnly = true,

		type = POSTURE_PEEK,
		stance = STANCE_LOW_COVER,
		priority = 9.0,

		{
			name = "LowCoverPeekLeft",
			lean = -0.8,
			agInput = "coverLftPeek",
			priority = -0.25,
		},

		{
			name = "LowCoverPeekRight",
			lean = 0.8,
			agInput = "coverRgtPeek",
			priority = -0.25,
		},

		{
			name = "LowCoverPeekCenter",
			peekOver = 0.8,
			agInput = "coverMidPeek",
			priority = -0.5,
		},
	},

	{
		name = "HighCoverPeek",
		templateOnly = true,

		type = POSTURE_PEEK,
		stance = STANCE_HIGH_COVER,
		priority = 9.0,

		{
			name = "HighCoverPeekLeft",
			lean = -0.8,
			agInput = "coverLftPeek",
			priority = -0.15,
		},

		{
			name = "HighCoverPeekRight",
			lean = 0.8,
			agInput = "coverRgtPeek",
			priority = -0.15,
		},
	},

	{
		name = "LowCoverAim",
		templateOnly = true,

		type = POSTURE_AIM,
		stance = STANCE_LOW_COVER,
		priority = 8.0,

		{
			name = "LowCoverAimOverCenter",
			peekOver = 0.8,
			agInput = "coverMidShoot",
		},

		{
			name = "LowCoverAimLeft",
			lean = -0.8,
			agInput = "coverLftShoot",
		},

		{
			name = "LowCoverAimRight",
			lean = 0.8,
			agInput = "coverRgtShoot",
		},
	},

	{
		name = "HighCoverAim",

		type = POSTURE_AIM,
		stance = STANCE_HIGH_COVER,
		priority = 9.0,

		{
			name = "HighCoverAimLeft",
			lean = -0.8,
			agInput = "coverLftShoot",
		},

		{
			name = "HighCoverAimRight",
			lean = 0.8,
			agInput = "coverRgtShoot",
		},
	},

	{
		name = "StandAim",
		type = POSTURE_AIM,
		stance = STANCE_STAND,
		priority = 10.0,

		{
			name = "StandAimCenter",
			lean = 0.0,
			priority = 0.0,
		},
	},

	{
		name = "CrouchAim",
		type = POSTURE_AIM,
		stance = STANCE_CROUCH,
		priority = 8.0,

		{
			name = "CrouchAimCenter",
			lean = 0.0,
			priority = 0.0,
		},
	},

	{
		name = "BlindLowCoverAim",
		templateOnly = true,

		type = POSTURE_AIM,
		stance = STANCE_LOW_COVER,
		priority = 0.0,

		{
			name = "BlindLowCoverAimLeft",
			lean = -0.8,
			agInput = "coverLftBlind",
			priority = -0.25,
		},

		{
			name = "BlindLowCoverAimRight",
			lean = 0.8,
			peekOver = 0.2,
			agInput = "coverRgtBlind",
			priority = -0.25,
		},

		{
			name = "BlindLowCoverAimCenter",
			peekOver = 0.8,
			agInput = "coverMidBlind",
			priority = -0.5,
		},
	},

	{
		name = "BlindHighCoverAim",
		templateOnly = true,

		type = POSTURE_AIM,
		stance = STANCE_HIGH_COVER,
		priority = 0.0,

		{
			name = "BlindHighCoverAimLeft",
			lean = -0.8,
			agInput = "coverLftBlind",
			priority = -0.15,
		},

		{
			name = "BlindHighCoverAimRight",
			lean = 0.8,
			--peekOver = 0.2,
			agInput = "coverRgtBlind",
			priority = -0.15,
		},
	},
}
------------------------------------------------------------------------------------------

mergef(Boar_x, AIBase, 1)

-----------------------------------------------------------------------------------------------------
function Boar_x:OnResetCustom()

	GameAI.UnregisterWithAllModules(self.id)
	AI.SetPostures(self.id, HumanPostures)

	self:ResetSharedSoundIds()

	self.AI.mountedWeaponCheck = true

	if (self.Properties.AI.bUseRadioChatter == 1) then
		GameAI.RegisterWithModule("RadioChatter", self.id)
	end

	GameAI.RegisterWithModule("BattleFront", self.id)
	GameAI.RegisterWithModule("StalkerModule", self.id) -- TODO: Rename StalkerModule to something else since it's used to see if the target can see me
	GameAI.RegisterWithModule("RangeModule", self.id)
	GameAI.AddRange(self.id, 2.5, "OnTargetEnteredMeleeRange", "")
	GameAI.AddRange(self.id, 3, "", "OnTargetLeftMeleeRange")
	GameAI.AddRange(self.id, self.gameParams.nearbyRange, "OnTargetEnteredNearbyRange", "OnTargetLeftNearbyRange")
	GameAI.AddRange(self.id, self.gameParams.middleRange, "OnTargetEnteredMiddleRange", "OnTargetEnteredMiddleRange")
	
	-- Stop advancing if the target is within this range whilst advancing
	GameAI.AddRange(self.id, 8.0, "EnteredTooCloseForComfortRange", "LeftTooCloseForComfortRange")

	if (self.Properties.AIBehaviorFlags.bSniper == 1) then
		GameAI.AddRange(self.id, 10, "OnTargetEnteredCloseRange", "")
		GameAI.AddRange(self.id, 12, "", "OnTargetLeftCloseRange")
	end

	if (self.Properties.AIBehaviorFlags.bGrenadier == 1) then
		GameAI.AddRange(self.id, 20, "OnTargetEnteredCloseRange", "")
		GameAI.AddRange(self.id, 22, "", "OnTargetLeftCloseRange")
	end

	self.lastImmediateThreatPos = {x=0, y=0, z=0}
	self.deadGroupMemberCount = 0
	self.suspiciousSoundInvestigationCount = 0
end

--------------------------------------------------
-- Misc
--------------------------------------------------
function Boar_x:EnableSearchModule()
	local groupId = AI.GetGroupOf(self.id)
	local targetPos = g_Vectors.temp_v1
	if (AI.GetAttentionTargetPosition(self.id, targetPos)) then
		local targetEntityId = NULL_ENTITY
		local targetEntity = AI.GetAttentionTargetEntity(self.id);
		if (targetEntity) then
			targetEntityId = targetEntity.id
		end
		GameAI.StartSearchModuleFor(groupId, targetPos, targetEntityId, 45.0)
		GameAI.RegisterWithModule("SearchModule", self.id)
		self:Log("Started search module for my group.")
	else
		self:Error("No attention target to search for")
	end
end

function Boar_x:SetNextSearchSpotToRefPoint()
	local position = GameAI.GetNextSearchSpot(self.id,
		0.6, -- closenessToAgentWeight
		0.3, -- closenessToTargetWeight
		15.0, -- minDistanceFromAgent
		0.1 -- optional: closenessToTargetCurrentPosWeight
	)
	if (position) then
		AI.SetRefPointPosition(self.id, position);
	else
		AI.Signal(SIGNALFILTER_SENDER, 1, "NoSearchSpotsAvailable", self.id)
	end
end

function Boar_x:DisableSearchModule()
	GameAI.UnregisterWithModule("SearchModule", self.id)
end

function Boar_x:OnPostLoad()
	self:OnResetCustom();
	BasicActor.OnPostLoad(self);
end

function Boar_x:ResetSharedSoundIds()

	self.SharedSoundSignals =
	{
		Feedback_KilledByPlayer = GameAudio.GetSignal("Human_Feedback_KilledByPlayerHit" ),
		KilledByExplosion = GameAudio.GetSignal("Human_KilledByExplosion" ),
		FeedbackHit2D = GameAudio.GetSignal("Human_FeedbackHit2D" ),
		FeedbackHit2D_Head  = GameAudio.GetSignal( "Human_FeedbackHit2D_Head" ),
		FeedbackHit2D_Helmet = GameAudio.GetSignal( "Human_FeedbackHit2D_Helmet" ),	
		FeedbackHit2DMelee = GameAudio.GetSignal("Melee_FeedbackHit2D"),
		FeedbackHit2D_NoDamage = GameAudio.GetSignal("NoDamage_FeedbackHit2D"),
	}

end

-- Set relocation speed depending on distance
function Boar_x:SetHoldGroundMoveSpeed()
	local currentPosition = g_Vectors.temp_v1
	self:GetWorldPos(currentPosition)
	if (DistanceVectors(currentPosition, self.AI.HoldGround.pos) > 10 or -- This 10 value matches the strafe set up to be 5+5 in HumanHoldGroundRelocate
			math.abs(currentPosition.z - self.AI.HoldGround.pos.z) > 1.0) then
		AI.SetSpeed(self.id, SPEED_RUN)
	else
		AI.SetSpeed(self.id, SPEED_WALK)
	end
end

function Boar_x:IsTargetAnEnemy()
	local targetType = AI.GetTargetType(self.id)
	return (targetType == AITARGET_ENEMY)
end

function Boar_x:IsUsable(user)
	if(self:IsDead()) then
		return 0;
	else
		return 1;
	end
end

function Boar_x.AnimationEvent(entity, event, value)
	if (event == "StealthMeleeDeath") then
		AI.PlayCommunication(entity.id, "comm_death_melee_stealth", "Injury", 0.5);
	elseif (BasicAI.AnimationEvent) then
		BasicAI.AnimationEvent(entity, event, value)
	end
end

function Boar_x:GetDistanceToDeadBody()
	return DistanceVectors(self.deadGroupMemberData.currentBodyPosition, self:GetWorldPos())
end

function Boar_x:IsTargetInNearbyRange()
	local dist = AI.GetAttentionTargetDistance(self.id)
	if (dist <= self.gameParams.nearbyRange) then
		return true
	end

	return false
end

function Boar_x:IsTargetInMiddleRange()
	local dist = AI.GetAttentionTargetDistance(self.id)
	if (dist <= self.gameParams.middleRange) then
		return true
	end

	return false
end

function Boar_x:IsInRangeFromTarget(range)

	local dist = AI.GetAttentionTargetDistance(self.id) or 500
	if (range and dist <= range) then
		return true
	end
	return false
end

function Boar_x:GetTargetDistance()
	return AI.GetAttentionTargetDistance(self.id) or 500
end

function Boar_x:SetLastExplosiveTypeAsGrenade()
	self.lastExplosiveType = "grenade"
	return true
end

function Boar_x:SetLastExplosiveTypeAsExplosive()
	self.lastExplosiveType = "explosive"
	return true
end

function Boar_x:ClearCombatMoveAssignmentIfCloseToTheDestination()
	if (DistanceSqVectors(self:GetWorldPos(), self.AI.combatMove.position) < 25) then
		self:ClearAssignment()
	end
end
