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

//TODO: Добавить функцию отключения возможности сохранить игру во время диалога


//-------------------------------------------------
CUIDialogMenu::CUIDialogMenu()
{
	//Получаем фреймворк и регистрируем наш класс для обработки базовых событий.
	//По большей части здесь нам понадобиться функция Update()/PostUpdate()
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework != NULL)
		pGameFramework->RegisterListener(this, "DialogMenu", FRAMEWORKLISTENERPRIORITY_DEFAULT);

	m_pDialogDispathcer = new CDialogDispatcher();
}

//-------------------------------------------------
CUIDialogMenu::~CUIDialogMenu()
{
	//Удаляем калсс из слушателей, дабы не вылетело, если класс удален
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework != NULL)
		pGameFramework->UnregisterListener(this);
}

//-------------------------------------------------
IUIElement* CUIDialogMenu::GetUIElement()
{
	if (gEnv->pFlashUI != NULL)
		m_pUIDialogMenu = gEnv->pFlashUI->GetUIElement("DialogMenu");
	return m_pUIDialogMenu;
}

//-------------------------------------------------
void CUIDialogMenu::OnUIEvent(IUIElement* pSender, const SUIEventDesc& event, const SUIArguments& args)
{
	if (!strcmp(event.sDisplayName, "OnSkip"))
	{
		Skip();
	}

	if (!strcmp(event.sDisplayName, "OnSelectAnswer"))
	{

	}

	//if (pDialogMenu != NULL)
	//{

	//	pDialogMenu->SetAnswer(sPlayerAnswer);
	//	pDialogMenu->SetAudio(sAudioName, true);
	//	m_pUIDialogMenu->CallFunction("Clear");
	//	fSec = atoi(fFlashSec);
	//	bStartPlayerCount = true;
	//	pDialogMenu->SetDialogCam(g_pGame->GetIGameFramework()->GetClientActor()->GetEntity(), CAMERA_NAME);
	//	pDialogMenu->RotateCamFocus(g_pGame->GetIGameFramework()->GetClientActor()->GetEntity(), BONE_NAME, CAMERA_NAME);


	//	if (!strcmp(CamBack, "true"))
	//	{
	//		IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
	//		view->LinkTo(gEnv->pEntitySystem->FindEntityByName(CAMERA_NAME));
	//		bQCam = false;
	//	}
	//}

}

//-------------------------------------------------
void CUIDialogMenu::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions &actions = g_pGame->Actions();
	if (actions.skip_dialog == action)
	{
		Skip();
	}
}

//-------------------------------------------------
void CUIDialogMenu::OnPostUpdate(float fDeltaTime)
{
	if (m_pDialogDispathcer != NULL)
		m_pDialogDispathcer->Update();
}

//-------------------------------------------------------------------
void CUIDialogMenu::CanDialog(IEntity* pTarget)
{
	m_pTarget = pTarget;
}

void CUIDialogMenu::OnTimerFinished(int id)
{
	Skip();
}

//-------------------------------------------------------------------
void CUIDialogMenu::StartDialog(IEntity *pAIEntity)
{
	GetUIElement();
	SmartScriptTable propertiesTable, dialogPropertiesTable;
	IScriptTable *pAIStript = pAIEntity->GetScriptTable();
	bool hasProperties = pAIStript->GetValue("Properties", propertiesTable);
	if (hasProperties)
	{
		const bool hasPropertiesTable = propertiesTable->GetValue("DialogProperties", dialogPropertiesTable);
		if (hasPropertiesTable)
		{
			dialogPropertiesTable->GetValue("fileDialog", m_pFilePath);
			if (strcmp(m_pFilePath, ""))
			{
				m_pUIDialogMenu->AddEventListener(this, "UIDialogMenu");
				m_pUIDialogMenu->Init();
				m_pUIDialogMenu->SetVisible(true);
				IActionMapManager *pActionMapManager = gEnv->pGame->GetIGameFramework()->GetIActionMapManager();
				if (pActionMapManager != NULL)
					pActionMapManager->EnableFilter("only_ui", true);
				g_pGameActions->FilterNoMove()->Enable(true);
				g_pGameActions->FilterNoMouse()->Enable(true);
				m_pTarget = pAIEntity;
				SetDialogCam(m_pTarget, CAMERA_NAME);
				//g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->SetRotation(Quat::CreateRotationVDir(pAIEntity->GetPos()));
				//
				//IRenderer* pRenderer = gEnv->pRenderer;
				//CRY_ASSERT(pRenderer);
				//pRenderer->EF_SetPostEffectParam("Dof_User_Active", 1.f, true);
				//pRenderer->EF_SetPostEffectParam("Dof_User_FocusDistance", 8000, true);
				//pRenderer->EF_SetPostEffectParam("Dof_User_FocusRange", 0, true);
				//pRenderer->EF_SetPostEffectParam("Dof_User_BlurAmount", 1, true);
				//pRenderer->EF_SetPostEffectParam("Dof_User_ScaleCoc", 12, true);

				m_pNPCAnswer = new SDialogNPCAnswer();
				m_pPlayerAnswer = new SDialogPlayerAnswer();

				SetDialogFromXML(1, m_pFilePath);

				const char* targetName = new char();
				dialogPropertiesTable->GetValue("sAIName", targetName);

				SUIArguments arg;
				arg.SetArguments(targetName);
				m_pUIDialogMenu->CallFunction("SetTargetName", arg);
			}
		}
	}
}

void CUIDialogMenu::StopDialog()
{
	GetUIElement();
	m_pUIDialogMenu->SetVisible(false);
	m_pUIDialogMenu->RemoveEventListener(this);

	IActionMapManager *pActionMapManager = gEnv->pGame->GetIGameFramework()->GetIActionMapManager();
	if (pActionMapManager != NULL)
		pActionMapManager->EnableFilter("only_ui", false);
	g_pGameActions->FilterNoMove()->Enable(false);
	g_pGameActions->FilterNoMouse()->Enable(false);

	SAFE_DELETE(m_pDialogTimer);
}

//-------------------------------------------------------------------
void CUIDialogMenu::SetAnswer(const char *text)
{
	GetUIElement();

	SUIArguments args;
	args.AddArgument(string(text));

	if (m_pUIDialogMenu != NULL)
		m_pUIDialogMenu->CallFunction("SetMessages", args);
}

//-------------------------------------------------------------------
void CUIDialogMenu::SetPlayerAnswer(SDialogPlayerAnswer* pPlayerAnswer, int index)
{
	GetUIElement();

	SUIArguments args;
	args.AddArgument(pPlayerAnswer[index].answer);
	args.AddArgument(pPlayerAnswer[index].nextDialogId);

	if (m_pUIDialogMenu != NULL)
		m_pUIDialogMenu->CallFunction("CreateAnswerItem", args);
}

//-------------------------------------------------------------------
void CUIDialogMenu::Skip()
{
	if (m_pNPCAnswer == NULL)
		return;

	SetDialogFromXML(m_pNPCAnswer->nextPhraseId, m_pFilePath);
}

//-------------------------------------------------------------------
void CUIDialogMenu::SetDialogFromXML(int NextDialogId, const char* filePath)
{
	XmlNodeRef DialogDescriptionFile = gEnv->pSystem->LoadXmlFromFile(filePath);
	if (DialogDescriptionFile == NULL)
		return;

	int count = DialogDescriptionFile->getChildCount();
	for (int i = 0; i < count; i++)
	{
		XmlNodeRef pAnswer = DialogDescriptionFile->getChild(i);
		if (!pAnswer)
			return;

		//Получаем параметры первого диалога(фразы)
		m_pNPCAnswer->currentPhraseId = atoi(pAnswer->getAttr("id"));
		const char* special = pAnswer->getAttr("special");	//Нужен для перехода к меню торговли или ещё чему
		const char *QuestCam = pAnswer->getAttr("questcam");// взятие камеры
		int initWithoutMessage = atoi(pAnswer->getAttr("initWithoutMessage"));

		if (m_pNPCAnswer->currentPhraseId == NextDialogId && !strcmp(special, ""))
		{
			int fCurrentPhraseDuration = atof(pAnswer->getAttr("duration"));	//получаем длинну аудиофайла в милисеках
			if (initWithoutMessage == 0)	//Если данная фраза отображает текста
			{
				if (m_pDialogTimer == NULL)
					m_pDialogTimer = new CTimer();
				m_pDialogTimer->StartTimer(fCurrentPhraseDuration, NextDialogId);
				m_pDialogTimer->AttachEventListener(this);

				const char *AIMessage = pAnswer->getAttr("message");
				SetAnswer(AIMessage);	// первый ответ NPC
				RotateCamFocus(m_pTarget, BONE_NAME, CAMERA_NAME);
				SetQuestCam(QuestCam); //Устанавливаем камеру, если она указана. Иначе функция ничего не выполнит
			}
			const char *pAudio = pAnswer->getAttr("audio");// взятие озвукчки диалоги
			//TODO: SetAudio(pAudio, true);

			//Варианты ответа у игрока
			int playerAnswerCount = pAnswer->getChildCount();

			if (playerAnswerCount == 0)  //Если игроку нечего овтетить
			{
				m_pNPCAnswer->nextPhraseId = NextDialogId + 1;
			}
			else
			{
				m_pPlayerAnswer = new  SDialogPlayerAnswer[playerAnswerCount]; // Массив ответов игрока
				m_pUIDialogMenu->CallFunction("Clear");		//Вызов функции очищения вариантов ответов

				for (int k = 0; k < playerAnswerCount; k++)
				{
					XmlNodeRef cPlayerAnswer = pAnswer->getChild(k);
					m_pPlayerAnswer[k].answer = cPlayerAnswer->getAttr("answer");
					m_pPlayerAnswer[k].nextDialogId = atoi(cPlayerAnswer->getAttr("next"));
					m_pPlayerAnswer[k].audioDialogDuration = atoi(cPlayerAnswer->getAttr("duration"));
					m_pPlayerAnswer[k].audioName = cPlayerAnswer->getAttr("audio");
					SetPlayerAnswer(m_pPlayerAnswer, k);
				}
			}
			return;
		}

		if (!strcmp(special, "Exit"))
		{
			//Delay();
			IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
			view->LinkTo(g_pGame->GetIGameFramework()->GetClientActor()->GetEntity());
			SAFE_DELETE(m_pDialogTimer);
			StopDialog();
			((CPlayer *)g_pGame->GetIGameFramework()->GetClientActor())->SetThirdPerson(false);
			break;
		}
	}
}



//-------------------------------------------------------------------
void CUIDialogMenu::SetDialogCam(IEntity *pEntity, const char *CameraName)
{
	SmartScriptTable propertiesTable, dialogPropertiesTable;
	IScriptTable *pAIStript = m_pTarget->GetScriptTable();
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

//-------------------------------------------------------------------
void CUIDialogMenu::RotateCamFocus(IEntity *pEntity, const char *BoneName, const char *CameraName)
{
	Vec3 cameraPosition;
	if (!m_pCurrentQuestCamera)
		m_pCurrentQuestCamera = gEnv->pEntitySystem->FindEntityByName(CameraName);

	Vec3 bonepos = CSpecialFunctions::GetBonePos(BoneName, false, pEntity);
	cameraPosition = (bonepos - cameraPosition.normalize());
	m_pCurrentQuestCamera->SetRotation(Quat::CreateRotationVDir(cameraPosition));
	IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
}

//-------------------------------------------------------------------
void CUIDialogMenu::SetQuestCam(string camname)
{
	if (strcmp(camname, ""))//Если имя камеры не равно пустой строке
	{
		m_pCurrentQuestCamera = gEnv->pEntitySystem->FindEntityByName(camname);
		if (!m_pCurrentQuestCamera)
			return;

		IView* pView = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
		pView->LinkTo(m_pCurrentQuestCamera);
		m_bQuestCameraIsEnabled = true;
	}
}

//-------------------------------------------------------------------
void CUIDialogMenu::SetAudio(const char * AudioName, bool bEnabled)
{
	IEntity *pAudioTrigger = gEnv->pEntitySystem->FindEntityByName(AUDIO_TRIGGER);
	SmartScriptTable propertiesTable;
	if (strcmp(AudioName, "") && pAudioTrigger)
	{
		pAudioTrigger->SetPos(m_pTarget->GetPos() + Vec3(0, 0, 3));
		IScriptTable *pStriptTable = pAudioTrigger->GetScriptTable();

		bool hasProperties = pStriptTable->GetValue("Properties", propertiesTable);
		if (hasProperties)
		{
			propertiesTable->SetValue("audioTriggerPlayTriggerName", AudioName);
			propertiesTable->SetValue("bEnabled", bEnabled);

			Script::CallMethod(pStriptTable, "OnPropertyChange");
		}
	}
	else
	{
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

