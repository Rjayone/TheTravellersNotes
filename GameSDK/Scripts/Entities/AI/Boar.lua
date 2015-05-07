Script.ReloadScript( "SCRIPTS/Entities/AI/Boar_x.lua")
--Script.ReloadScript( "SCRIPTS/Entities/actor/BasicActor.lua")
--Script.ReloadScript( "SCRIPTS/Entities/AI/Shared/BasicAI.lua")
CreateActor(Boar_x)
Boar=CreateAI(Boar_x)

--Script.ReloadScript( "SCRIPTS/AI/Assignments.lua")
InjectAssignmentFunctionality(Boar)
AddDefendAreaAssignment(Boar)
AddCombatMoveAssignment(Boar)

Boar:Expose()