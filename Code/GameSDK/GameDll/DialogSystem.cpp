#include "StdAfx.h"
#include "DialogSystem.h"
#include "GameActions.h"
#include "Player.h"
#include "AnimKey.h"
#include "IRenderer.h"
#include "MathUtils.h"
#include "SpecialFunctions.h"


#define FILE_PATH "GameSDK/Libs/DialogSystem/Dialog_default.xml"
#define UI_NAME "DialogMenu"
#define BONE_NAME "Bip01 Head"
#define CAMERA_NAME "Questcamera"
#define AUDIO_TRIGGER "AudioQuestTrigger"
#define TEXTURE "GameSDK/Textures/generic/brick/brick_rounded_diff.dds"


//-------------------------------------------------
CUIDialogMenu::CUIDialogMenu()
{
	StartCount = false;
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework != NULL)
		pGameFramework->RegisterListener(this, "CDialogMenu", FRAMEWORKLISTENERPRIORITY_DEFAULT);

	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->AddExtraActionListener(this);

	//texture = gEnv->pRenderer->EF_LoadTexture(TEXTURE);
}

//-------------------------------------------------
CUIDialogMenu::~CUIDialogMenu()
{
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework != NULL)
		pGameFramework->UnregisterListener(this);
}

void CUIDialogMenuEventListener::OnUIEvent(IUIElement* pSender, const SUIEventDesc& event, const SUIArguments& args)
{
	if (!strcmp(event.sDisplayName, "ButtonPress"))
	{
		CUIDialogMenu *pDialogMenu = g_pGame->GetDialogMenu();
		
		IUIElement *m_pUIDialogMenu = gEnv->pFlashUI->GetUIElement("DialogMenu");
		
		args.GetArg(0, nextDialogId);

		string CamBack;
		args.GetArg(1, CamBack);
			
		string sPlayerAnswer;
		args.GetArg(2, sPlayerAnswer);
		
		string fFlashSec;
		args.GetArg(3, fFlashSec);

		string sAudioName;
		args.GetArg(4, sAudioName);

		if (pDialogMenu != NULL)
		{
			
			pDialogMenu->SetAnswer(sPlayerAnswer);
			pDialogMenu->SetAudio(sAudioName, true);
			m_pUIDialogMenu->CallFunction("Clear");
			fSec = atoi(fFlashSec);
			bStartPlayerCount = true;
			pDialogMenu->SetDialogCam(g_pGame->GetIGameFramework()->GetClientActor()->GetEntity(), CAMERA_NAME);
			pDialogMenu->RotateCamFocus(g_pGame->GetIGameFramework()->GetClientActor()->GetEntity(), BONE_NAME, CAMERA_NAME);


			if (!strcmp(CamBack, "true"))
			{
				IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
				view->LinkTo(gEnv->pEntitySystem->FindEntityByName(CAMERA_NAME));
				bQCam = false;
			}
		}

	}
}

void CUIDialogMenu::StartDialog(IEntity *pAIEntity, bool Show)
{
	
	if (gEnv->pFlashUI != NULL)
		m_pUIDialogMenu = gEnv->pFlashUI->GetUIElement("DialogMenu");

	if (Show)
	{
		if (m_pUIDialogMenu != NULL)
		{

			SmartScriptTable propertiesTable, dialogPropertiesTable;
			IScriptTable *pAIStript = pAIEntity->GetScriptTable();
			bool hasProperties = pAIStript->GetValue("Properties", propertiesTable);
			if (hasProperties)
			{

				const bool hasPropertiesTable = propertiesTable->GetValue("DialogProperties", dialogPropertiesTable);
				if (hasPropertiesTable)
				{
					dialogPropertiesTable->GetValue("fileDialog", FilePath);
					if (strcmp(FilePath, ""))
					{
						m_pUIDialogMenu->AddEventListener(&m_UIDialogMenutEventListener, "UIDialogMenu");
						m_pUIDialogMenu->Init();
						m_pUIDialogMenu->SetVisible(true);
						IActionMapManager *pActionMapManager = gEnv->pGame->GetIGameFramework()->GetIActionMapManager();
						if (pActionMapManager != NULL)
							pActionMapManager->EnableFilter("only_ui", false);
						g_pGameActions->FilterNoMove()->Enable(true);
						g_pGameActions->FilterNoMouse()->Enable(true);
						pEntityAI = pAIEntity;
						SetDialogCam(pEntityAI, CAMERA_NAME);
						//g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->SetRotation(Quat::CreateRotationVDir(pAIEntity->GetPos()));

						/*IRenderer* pRenderer = gEnv->pRenderer;
						CRY_ASSERT(pRenderer);
						pRenderer->EF_SetPostEffectParam("Dof_User_Active", 1.f, true);
						pRenderer->EF_SetPostEffectParam("Dof_User_FocusDistance", 8000, true);
						pRenderer->EF_SetPostEffectParam("Dof_User_FocusRange", 0, true);
						pRenderer->EF_SetPostEffectParam("Dof_User_BlurAmount", 1, true);
						pRenderer->EF_SetPostEffectParam("Dof_User_ScaleCoc", 12, true);*/

						SetDialogFromXML(1);						
						
					}
				}
			}

		}
	}
		else
		{
			if (m_pUIDialogMenu != NULL)
			{
				m_pUIDialogMenu->SetVisible(false);
				m_pUIDialogMenu->RemoveEventListener(&m_UIDialogMenutEventListener);
				IActionMapManager *pActionMapManager = gEnv->pGame->GetIGameFramework()->GetIActionMapManager();
				if (pActionMapManager != NULL)
					pActionMapManager->EnableFilter("only_ui", false);
				g_pGameActions->FilterNoMove()->Enable(false);
				g_pGameActions->FilterNoMouse()->Enable(false);
				StartCount = false;
				fSec = 0;
			}
		}

	
}



void CUIDialogMenu::SetAnswer(const char *text)// первый ответ NPC
{
	m_pUIDialogMenu = gEnv->pFlashUI->GetUIElement("DialogMenu");
	SUIArguments args;
	args.AddArgument(string(text));

	

	if (m_pUIDialogMenu != NULL)
		m_pUIDialogMenu->CallFunction("SetMessages", args);

}

void CUIDialogMenu::SetPlayerAnswer(string Answer, string Next, int count, string camback, string sec, string sAudioName)
{
	SUIArguments args;

	args.AddArgument(Answer);
	args.AddArgument(Next);
	args.AddArgument(count);
	args.AddArgument(camback);
	args.AddArgument(sec);
	args.AddArgument(sAudioName);

	if (m_pUIDialogMenu != NULL)
		m_pUIDialogMenu->CallFunction("CreateAnswerItem", args);


}

	
void CUIDialogMenu::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions &actions = g_pGame->Actions();
	if (actions.skip_dialog == action)
	{

		fSec = 0;
	}
}


void CUIDialogMenu::OnPostUpdate(float fDeltaTime)
{

	//gEnv->pRenderer->Draw2dImage(gEnv->pRenderer->GetWidth() / 2.0f, gEnv->pRenderer->GetHeight() / 2.0f, 100.0f, 100.0f, texture->GetTextureID());
	//if (pEntityAI)
	//gEnv->pRenderer->DrawLabel(pEntityAI->GetWorldPos() + Vec3(0, 0, 2.5f), 3.0f, "start dialog");

	if (bStartPlayerCount)
	{
		fSec--;
		if (fSec <= 0)
		{
			CUIDialogMenu *pDialogMenu = g_pGame->GetDialogMenu();
			pDialogMenu->SetDialogFromXML(nextDialogId); //вызов следующего диалога xml
			bStartPlayerCount = false;
			SetAudio("",false);
		}
	}

	if (StartCount)
	{
		fSec--;
		if (fSec <= 0)
		{

			if (bNextPhrase)
			{
				StartCount = false;
				SetDialogFromXML(nextDialogId);
				//bNextPhrase = false;
			}
			else
			{
				for (int k = 0; k < playerAnswerCount; k++)
				{
					SetAudio("", false);
					SetPlayerAnswer(PlayersAnswers[k].answer, PlayersAnswers[k].idNextDialog, k, PlayersAnswers[k].qcamback, PlayersAnswers[k].sec, PlayersAnswers[k].AudioName);
					StartCount = false;
				}
			}
		}
	}
}

void CUIDialogMenu::OnTimer(float sec)
{
	
	while (sec>0)
	{
		sec--; 
	}
	

}

void CUIDialogMenu::SetDialogFromXML(int NextDialogId)
{
 	XmlNodeRef DialogDescriptionFile = gEnv->pSystem->LoadXmlFromFile(FilePath);
	if (DialogDescriptionFile)
	{
		int c = DialogDescriptionFile->getChildCount();
		for (int i = 0; i < c; i++)
		{
			XmlNodeRef Answer = DialogDescriptionFile->getChild(i);
			const char *cNextDialogId = Answer->getAttr("id");
			const char* special = Answer->getAttr("special");
			
			
			if (atoi(cNextDialogId) == NextDialogId && !strcmp(special, ""))
			{
				fSec = atof(Answer->getAttr("sec"));
				StartCount = true;

				const char *AIMessage = Answer->getAttr("message");
				SetAnswer(AIMessage);	// первый ответ NPC
				RotateCamFocus(pEntityAI, BONE_NAME, CAMERA_NAME);
				
				const char *QuestCam = Answer->getAttr("questcam");// взятие камеры	
				SetQuestCam(QuestCam);
				
				const char *Audio = Answer->getAttr("audio");// взятие озвукчки диалоги
				SetAudio(Audio,true);

				playerAnswerCount = Answer->getChildCount();
				if (playerAnswerCount == 0)
				{
					nextDialogId = NextDialogId + 1;
					bNextPhrase = true;
					//OnPostUpdate(0);
					//break;
				}
				else
				{
					PlayersAnswers = new  SAnswer[playerAnswerCount]; // Массив ответов игрока
					bNextPhrase = false;
					if (m_pUIDialogMenu)
						m_pUIDialogMenu->CallFunction("Clear");		//Вызов функции очищения кнопок из флеша

					for (int k = 0; k < playerAnswerCount; k++)
					{
						XmlNodeRef cPlayerAnswer = Answer->getChild(k);
						PlayersAnswers[k].answer = cPlayerAnswer->getAttr("answer");
						PlayersAnswers[k].idNextDialog = cPlayerAnswer->getAttr("next");
						PlayersAnswers[k].qcamback = cPlayerAnswer->getAttr("qcamback");
						PlayersAnswers[k].sec = cPlayerAnswer->getAttr("sec");
						PlayersAnswers[k].AudioName = cPlayerAnswer->getAttr("audio");
						//SetPlayerAnswer(PlayersAnswers[k].answer, PlayersAnswers[k].idNextDialog, k, PlayersAnswers[k].qcamback);
					}
				}
				return;
			}

			if (!strcmp(special, "Exit"))
			{
				//Delay();
				IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
				view->LinkTo(g_pGame->GetIGameFramework()->GetClientActor()->GetEntity());
				fSec = 0;
				StartCount = false;
				//bStartFocus = false;
				StartDialog(NULL,false);
				((CPlayer *)g_pGame->GetIGameFramework()->GetClientActor())->SetThirdPerson(false);
				break;
			}
		}		
	}
}




void CUIDialogMenu::SetDialogCam(IEntity *pEntity, const char *CameraName)
{

	SmartScriptTable propertiesTable, dialogPropertiesTable;
	IScriptTable *pAIStript = pEntity->GetScriptTable();
	bool hasProperties = pAIStript->GetValue("Properties", propertiesTable);
	if (hasProperties)
	{

		const bool hasPropertiesTable = propertiesTable->GetValue("DialogProperties", dialogPropertiesTable);
		if (hasPropertiesTable)
		{
			float camDistance;
			bool bIsLeft;
			bool bIsSideCam;
			dialogPropertiesTable->GetValue("camDistance", camDistance);
			dialogPropertiesTable->GetValue("bIsLeft", bIsLeft);
			dialogPropertiesTable->GetValue("bIsSideCam", bIsSideCam);

			if (!bIsSideCam)
			{
				if (camDistance)
				{

					Vec3  vDir, vNPC, vP, vMid, vCamNPC, HitPos;
					float length = 0, Ang;
					vNPC = pEntity->GetWorldPos();
					vP = g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetWorldPos();

					vDir = vNPC - vP;
					vMid = vP + vDir / 2;



					/*	vDir = (vNPC - vP).normalize();
						length = vDir.len() / 2;
						vMid = vP + vDir * length;*/

					if (bIsLeft)
						HitPos = MathUtils::Ang2Dir(g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetWorldAngles() + Ang3(0, 0, 90));
					else
						HitPos = MathUtils::Ang2Dir(g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetWorldAngles() + Ang3(0, 0, -90));

					HitPos.normalize();
					HitPos.SetLength(camDistance); //2.0f

					IEntity *Qcam = gEnv->pEntitySystem->FindEntityByName(CameraName);
					if (Qcam)
					{
						((CPlayer *)g_pGame->GetIGameFramework()->GetClientActor())->SetThirdPerson(true);
						Qcam->SetPos(vMid + HitPos + Vec3(0, 0, 1.5));
						/*Vec3 bonepos = CSpecialFunctions::GetBonePos(BoneName, false, pEntity);
						vCamNPC = (bonepos - Qcam->GetPos()).normalize();
						Qcam->SetRotation(Quat::CreateRotationVDir(vCamNPC));
						IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
						view->LinkTo(Qcam);*/
					}
				}
			}
			//else
			//{
			//	Vec3   HitPos;
			//	float length = 0, Ang;

			//	if (bIsLeft)
			//		HitPos = MathUtils::Ang2Dir(pEntity->GetWorldAngles() + Ang3(0, 0, 90));
			//	else
			//		HitPos = MathUtils::Ang2Dir(pEntity->GetWorldAngles() + Ang3(0, 0, -90));

			//	HitPos.normalize();
			//	HitPos.SetLength(camDistance); //2.0f

			//	IEntity *Qcam = gEnv->pEntitySystem->FindEntityByName(CameraName);
			//	if (Qcam)
			//	{
			//		((CPlayer *)g_pGame->GetIGameFramework()->GetClientActor())->SetThirdPerson(true);
			//		Qcam->SetPos(pEntity->GetWorldPos()+HitPos + Vec3(0, 0, 1.8));
			//	}

			//}			

		}
	}
}

void CUIDialogMenu::RotateCamFocus(IEntity *pEntity, const char *BoneName, const char *CameraName)
{
	Vec3 vCam;
	IEntity *Qcam = gEnv->pEntitySystem->FindEntityByName(CameraName);
	if (Qcam)
	{
		Vec3 bonepos = CSpecialFunctions::GetBonePos(BoneName, false, pEntity);
		vCam = (bonepos - Qcam->GetPos()).normalize();
		Qcam->SetRotation(Quat::CreateRotationVDir(vCam));
		IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
		view->LinkTo(Qcam);
	}
}


void CUIDialogMenu::SetQuestCam(string camname)
{

	/* Блок показа камерой квеста */
	if (strcmp(camname, ""))
	{		
		IEntity *Qcam = gEnv->pEntitySystem->FindEntityByName(camname);
		if (!Qcam)
			return;

		IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
		view->LinkTo(Qcam);
		bQCam = true;
	}
	/*Конец блока*/
}

void CUIDialogMenu::SetAudio(const char * AudioName, bool bEnabled)
{
	IEntity *pAudioTrigger = gEnv->pEntitySystem->FindEntityByName(AUDIO_TRIGGER);

    if (strcmp(AudioName, ""))
	{
		
		if (pAudioTrigger)	
		{
			pAudioTrigger->SetPos(pEntityAI->GetPos() + Vec3(0, 0, 3));
			SmartScriptTable propertiesTable;
			IScriptTable *pStriptTable = pAudioTrigger->GetScriptTable();					
			
			bool hasProperties = pStriptTable->GetValue("Properties", propertiesTable);
			if (hasProperties)
			{
				propertiesTable->SetValue("audioTriggerPlayTriggerName", AudioName);
				propertiesTable->SetValue("bEnabled", bEnabled);

				Script::CallMethod(pStriptTable, "OnPropertyChange");
				//Script::Call();
			}
		}
	}
	else
	{
		if (pAudioTrigger)
		{
			SmartScriptTable propertiesTable;
			IScriptTable *pStriptTable = pAudioTrigger->GetScriptTable();
			bool hasProperties = pStriptTable->GetValue("Properties", propertiesTable);
			if (hasProperties)
			{			
				propertiesTable->SetValue("audioTriggerPlayTriggerName", "");
				propertiesTable->SetValue("bEnabled", bEnabled);
				Script::CallMethod(pStriptTable, "OnPropertyChange");				
			}
		}
	}

}