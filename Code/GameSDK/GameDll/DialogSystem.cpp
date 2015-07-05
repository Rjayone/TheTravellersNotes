#include "StdAfx.h"
#include "DialogSystem.h"
#include "GameActions.h"
#include "Player.h"
#include "AnimKey.h"
#include "IRenderer.h"
#include "MathUtils.h"
#include "SpecialFunctions.h"
#include "UIVisibleManager.h"
#include "IAIActor.h"


#define FILE_PATH "GameSDK/Libs/DialogSystem/Dialog_default.xml"
#define UI_NAME "DialogMenu"
#define BONE_NAME "Bip01 Head"
#define CAMERA_NAME "Questcamera"
#define AUDIO_TRIGGER "AudioQuestTrigger"

//TODO: Добавить функцию отключения возможности сохранить игру во время диалога


//-------------------------------------------------
CUIDialogMenu::CUIDialogMenu()
{
	m_pDialogTimer = NULL;
	m_pCurrentQuestCamera = NULL;
	m_pFilePath = NULL;
	m_pTarget = NULL;
	m_pAudioTrigger = NULL;
	m_nLastParentDialogId = -1;

	m_pDialogDispathcer = new CDialogDispatcher();
	m_pVisibleManager = new CUIVisibleManager();

	//Получаем фреймворк и регистрируем наш класс для обработки базовых событий.
	//По большей части здесь нам понадобиться функция Update()/PostUpdate()
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework != NULL)
		pGameFramework->RegisterListener(this, "DialogMenu", FRAMEWORKLISTENERPRIORITY_DEFAULT);

	g_pGame->GetEventDispatcher()->AddListener(this);
}

//-------------------------------------------------
CUIDialogMenu::~CUIDialogMenu()
{
	//Удаляем калсс из слушателей, дабы не вылетело, если класс удален
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework != NULL)
		pGameFramework->UnregisterListener(this);

	g_pGame->GetEventDispatcher()->RemoveListener(this);
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
		int nexNPCtDialog = 0;
		args.GetArg(0, nexNPCtDialog);
		Skip(nexNPCtDialog);
	}

	if (!strcmp(event.sDisplayName, "OnSelectAnswer"))
	{
		int nextDialog = 0;
		args.GetArg(0, nextDialog);
		ContinueDialog(nextDialog);
	}

	//if (pDialogMenu != NULL)
	//{
	//
	//	pDialogMenu->SetAnswer(sPlayerAnswer);
	//	pDialogMenu->SetAudio(sAudioName, true);
	//	m_pUIDialogMenu->CallFunction("Clear");
	//	fSec = atoi(fFlashSec);
	//	bStartPlayerCount = true;
	//	pDialogMenu->SetDialogCam(g_pGame->GetIGameFramework()->GetClientActor()->GetEntity(), CAMERA_NAME);
	//	pDialogMenu->RotateCamFocus(g_pGame->GetIGameFramework()->GetClientActor()->GetEntity(), BONE_NAME, CAMERA_NAME);
	//
	//
	//	if (!strcmp(CamBack, "true"))
	//	{
	//		IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
	//		view->LinkTo(gEnv->pEntitySystem->FindEntityByName(CAMERA_NAME));
	//		bQCam = false;
	//	}
	//}

}

//-------------------------------------------------
void CUIDialogMenu::OnPlayerEnterInEditoriGameMode()
{
}

//-------------------------------------------------
void CUIDialogMenu::OnPlayerExitFromEditorGameMode()
{
	GetUIElement()->SetVisible(false);
	m_bDialogStarted = false;
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

	IUIElement* pDot = gEnv->pFlashUI->GetUIElement("Dot");
	if (pDot && m_bDialogStarted == false)
	{
		pDot->SetVisible(true);
		SUIArguments args;
		args.SetArguments("Talk");
		pDot->CallFunction("PlayMessage", args);
	}
}

void CUIDialogMenu::OnTimerFinished(int id)
{
	Skip(id);
}

//-------------------------------------------------------------------
void CUIDialogMenu::WillStartDialog(IEntity *pAIEntity)
{
	//Прячем прочие элементы худа
	std::vector<string> elements;
	elements.push_back(_DOT_ELEMENT_NAME);
	elements.push_back(_HEALTH_BAR_ELEMENT_NAME);
	elements.push_back(_LOCATION_ELEMENT_NAME);
	elements.push_back(_SCREEN_SLOTS_ELEMENT_NAME);
	m_pVisibleManager->HideList(elements);

	GetUIElement();
	SmartScriptTable propertiesTable, dialogPropertiesTable;
	IScriptTable *pAIStript = pAIEntity->GetScriptTable();
	bool hasProperties = pAIStript->GetValue("Properties", propertiesTable);
	if (hasProperties == true)
	{
		const bool hasPropertiesTable = propertiesTable->GetValue("DialogProperties", dialogPropertiesTable);
		if (hasPropertiesTable)
		{
			bool bDialogEnable, bEnemy;

			dialogPropertiesTable->GetValue("bDialogEnable", bDialogEnable);

			if (!bDialogEnable || m_pTarget->GetAI()->IsHostile(m_pTarget->GetAI()) ) //Если диалог недоступен то прерываем
			{
				CryLog("[DialogSystem]: Dialog not available or Target is enemy");
				return;
			}
			dialogPropertiesTable->GetValue("fileDialog", m_pFilePath);
			if (strcmp(m_pFilePath, ""))
			{
				m_bDialogStarted = true;
				m_pTarget = pAIEntity;
				m_pUIDialogMenu->AddEventListener(this, "UIDialogMenu");
				m_pUIDialogMenu->SetVisible(true);

				IActionMapManager *pActionMapManager = gEnv->pGame->GetIGameFramework()->GetIActionMapManager();
				if (pActionMapManager != NULL)
					pActionMapManager->EnableFilter("only_ui", true);

				g_pGameActions->FilterNoMove()->Enable(true);
				g_pGameActions->FilterNoMouse()->Enable(true);

				Vec3 playerPosition = g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetAI()->GetPos();
				if (IPipeUser* pipeUser = m_pTarget->GetAI()->CastToIPipeUser())
				{
					IPipeUser::LookTargetPtr m_lookTarget;
					if (!m_lookTarget.get())
					{
						m_lookTarget = pipeUser->CreateLookTarget();
					}

					*m_lookTarget = playerPosition;
				}

				
				m_pTarget->GetAI()->CastToIAIActor()->SetLookAtPointPos(playerPosition, false);

				SetDialogCam(m_pTarget, CAMERA_NAME);
				//g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->SetRotation(Quat::CreateRotationVDir(pAIEntity->GetPos()));
				//

				m_pNPCAnswer.clear();
				m_pPlayerAnswer.clear();
				SetRenderStatus(true);
				ParseXML();
				StartDialog();

				const char* targetName = new char();
				dialogPropertiesTable->GetValue("sAIName", targetName);

				SUIArguments arg;
				arg.SetArguments(targetName);
				m_pUIDialogMenu->CallFunction("SetTargetName", arg);
			}
		}
	}
}

//-------------------------------------------------------------------
void CUIDialogMenu::StartDialog()
{
	//Диалог всегда начинается с 0 элемента фраз нпс
	//если такой фразы нет - выход
	if (!m_pNPCAnswer[0])
		return;

	//Устанавливаем ид родительского элемента(фразы)
	//С помощью этого мы можем в дальнейшем устанавливать фразы игрока
	m_nLastParentDialogId = m_pNPCAnswer[0]->currentPhraseId;

	//Если фраза без текста, и сразу ответ игрока, то сразу отображаем ответы игрока и проигрываем фразу нпс
	if (m_pNPCAnswer[0]->withoutMessage == true)
	{
		SetAudio(m_pNPCAnswer[0]->audioName, true);

		//В цикле вызываем метод установления ответов игрока
		for (int i = 0; i < m_pPlayerAnswer.size(); i++)
		if (m_pPlayerAnswer[i]->parentId == m_nLastParentDialogId)
			SetPlayerAnswer(m_pPlayerAnswer[i]);
		m_pUIDialogMenu->CallFunction("BeginShowAnswers"); //Выводим на экран ответы
		return;
	}

	//Таймер будет автоматически переключать фразы
	//Ид таймера использется как следующий диалог. Если следующий не задан, то передается -1
	if (m_pDialogTimer == NULL)
		m_pDialogTimer = new CTimer();

	m_pDialogTimer->StartTimer(m_pNPCAnswer[0]->audioDialogDuration / 100, m_pNPCAnswer[0]->nextPhraseId);
	m_pDialogTimer->AttachEventListener(this);

	SetAnswer(m_pNPCAnswer[0]->answer, m_pNPCAnswer[0]->nextPhraseId);
}

//-------------------------------------------------------------------
void CUIDialogMenu::ContinueDialog(int nextNPCDialogId)
{
	if (m_bDialogStarted == false)
		return;

	//Если след. диалог нпс != -1, то бегаем по цику и ищем следующий абзац.
	//данная функция может вызываться и в таймере
	for (int i = 0; i < m_pNPCAnswer.size(); i++)
	{
		if (m_pNPCAnswer[i]->currentPhraseId == nextNPCDialogId /*&& !strcmp(m_pNPCAnswer[i]->specialEvent, "")*/)
		{
			//TODO: Обработка экшенов
			if (m_pNPCAnswer[i]->inventoryActions.size() > 0)
			{
				//~
			}
			if (m_pNPCAnswer[i]->questActions.size() > 0)
			{
				//~
			}

			if (!strcmp(m_pNPCAnswer[i]->specialEvent, "Exit"))
			{
				StopDialog();
				return;
			}

			m_nLastParentDialogId = m_pNPCAnswer[i]->currentPhraseId;

			if (m_pDialogTimer == NULL)
				m_pDialogTimer = new CTimer();

			m_pDialogTimer->StartTimer(m_pNPCAnswer[i]->audioDialogDuration / 100, m_pNPCAnswer[i]->nextPhraseId);
			m_pDialogTimer->AttachEventListener(this);

			SetAnswer(m_pNPCAnswer[i]->answer, m_pNPCAnswer[i]->nextPhraseId);		
			return;
		}
	}
	

	//Если следующего абзаца нет, то отображаем ответы игрока по последнему родительскому тегу(в котоом назодимся)
	if (nextNPCDialogId == 0)
	{
		m_pUIDialogMenu->CallFunction("Clear");
		for (int i = 0; i < m_pPlayerAnswer.size(); i++)
		{
			if (m_pPlayerAnswer[i]->parentId == m_nLastParentDialogId)
				SetPlayerAnswer(m_pPlayerAnswer[i]);
		}
		m_pDialogTimer->StopTimer();
		m_pUIDialogMenu->CallFunction("BeginShowAnswers"); //Выводим на экран ответы
		//TODO: Сделать отчистку предыдущих вариков ответа
	}
}

//-------------------------------------------------------------------
void CUIDialogMenu::StopDialog()
{
	m_bDialogStarted = false;
	m_pVisibleManager->ShowHidenUIElements(false);

	GetUIElement();
	m_pUIDialogMenu->SetVisible(false);
	m_pUIDialogMenu->RemoveEventListener(this);

	IActionMapManager *pActionMapManager = gEnv->pGame->GetIGameFramework()->GetIActionMapManager();
	if (pActionMapManager != NULL)
		pActionMapManager->EnableFilter("only_ui", false);
	g_pGameActions->FilterNoMove()->Enable(false);
	g_pGameActions->FilterNoMouse()->Enable(false);

	m_pNPCAnswer.clear();
	m_pPlayerAnswer.clear();
	SetRenderStatus(false);
	m_pUIDialogMenu->CallFunction("Clear");

	//SAFE_DELETE(m_pDialogTimer);
}

//-------------------------------------------------------------------
void CUIDialogMenu::SetAnswer(const char *text, int id)
{
	GetUIElement();

	SUIArguments args;
	args.AddArgument(string(text));
	args.AddArgument(id);

	if (m_pUIDialogMenu != NULL)
		m_pUIDialogMenu->CallFunction("SetMessage", args);
}

//-------------------------------------------------------------------
void CUIDialogMenu::SetPlayerAnswer(SDialogPlayerAnswer* pPlayerAnswer)
{
	GetUIElement();

	SUIArguments args;
	args.AddArgument(pPlayerAnswer->answer);
	args.AddArgument(pPlayerAnswer->nextDialogId);

	if (m_pUIDialogMenu != NULL)
		m_pUIDialogMenu->CallFunction("CreateAnswerItem", args);
}

//-------------------------------------------------------------------
void CUIDialogMenu::Skip(int nextDialogId)
{
	//SAFE_DELETE(m_pDialogTimer);
	ContinueDialog(nextDialogId);
}

void CUIDialogMenu::Exit()
{
	IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
	view->LinkTo(g_pGame->GetIGameFramework()->GetClientActor()->GetEntity());
	((CPlayer *)g_pGame->GetIGameFramework()->GetClientActor())->SetThirdPerson(false);
	m_pUIDialogMenu->CallFunction("ClearPlayerAnswers");

	SAFE_DELETE(m_pDialogTimer);
	StopDialog();
}

//-------------------------------------------------------------------
void CUIDialogMenu::SetDialogFromXML(int NextDialogId, const char* filePath)
{
	//if (filePath == NULL && m_pFilePath != NULL)
	//	filePath = m_pFilePath;

	//XmlNodeRef DialogDescriptionFile = gEnv->pSystem->LoadXmlFromFile(filePath);
	//if (DialogDescriptionFile == NULL)
	//	return;

	//int count = DialogDescriptionFile->getChildCount();
	//for (int i = 0; i < count; i++)
	//{
	//	XmlNodeRef pAnswer = DialogDescriptionFile->getChild(i);
	//	if (!pAnswer)
	//		return;

	//	//Получаем параметры первого диалога(фразы)
	//	m_pNPCAnswer->currentPhraseId = atoi(pAnswer->getAttr("id"));
	//	const char* special = pAnswer->getAttr("special");	//Нужен для перехода к меню торговли или ещё чему
	//	const char *QuestCam = pAnswer->getAttr("questcam");// взятие камеры
	//	int initWithoutMessage = atoi(pAnswer->getAttr("initWithoutMessage"));

	//	if (m_pNPCAnswer->currentPhraseId == NextDialogId && !strcmp(special, ""))
	//	{
	//		int fCurrentPhraseDuration = atof(pAnswer->getAttr("duration"));	//получаем длинну аудиофайла в милисеках
	//		if (initWithoutMessage == 0)	//Если данная фраза отображает текста
	//		{
	//			if (m_pDialogTimer == NULL)
	//				m_pDialogTimer = new CTimer();
	//			m_pDialogTimer->StartTimer(fCurrentPhraseDuration / 100, NextDialogId);
	//			m_pDialogTimer->AttachEventListener(this);

	//			const char *AIMessage = pAnswer->getAttr("message");
	//			SetAnswer(AIMessage);	// первый ответ NPC
	//			RotateCamFocus(m_pTarget, BONE_NAME, CAMERA_NAME);
	//			SetQuestCam(QuestCam); //Устанавливаем камеру, если она указана. Иначе функция ничего не выполнит
	//		}
	//		else
	//			m_pUIDialogMenu->CallFunction("ToPlayerAnswers");
	//		const char *pAudio = pAnswer->getAttr("audio");// взятие озвукчки диалоги
	//		//TODO: SetAudio(pAudio, true);

	//		//Варианты ответа у игрока
	//		int playerAnswerCount = pAnswer->getChildCount();
	//		if (playerAnswerCount == 0)  //Если игроку нечего овтетить
	//		{
	//			m_pNPCAnswer->nextPhraseId = NextDialogId + 1;
	//		}
	//		else
	//		{
	//			m_pPlayerAnswer = new  SDialogPlayerAnswer[playerAnswerCount]; // Массив ответов игрока
	//			m_pUIDialogMenu->CallFunction("Clear");		//Вызов функции очищения вариантов ответов
	//			for (int k = 0; k < playerAnswerCount; k++)
	//			{
	//				XmlNodeRef cPlayerAnswer = pAnswer->getChild(k);
	//				m_pPlayerAnswer[k].answer = cPlayerAnswer->getAttr("answer");
	//				m_pPlayerAnswer[k].nextDialogId = atoi(cPlayerAnswer->getAttr("next"));
	//				m_pPlayerAnswer[k].audioDialogDuration = atoi(cPlayerAnswer->getAttr("duration"));
	//				m_pPlayerAnswer[k].audioName = cPlayerAnswer->getAttr("audio");
	//				SetPlayerAnswer(m_pPlayerAnswer, k);
	//			}
	//			m_pUIDialogMenu->CallFunction("BeginShowAnswers"); //Выводим на экран ответы
	//		}
	//		return;
	//	}

	//	if (!strcmp(special, "Exit"))
	//	{
	//		Exit();	break;
	//	}
	//}
}

//-------------------------------------------------------------------
void CUIDialogMenu::ParseXML()
{
	XmlNodeRef DialogDescriptionFile = gEnv->pSystem->LoadXmlFromFile(m_pFilePath);
	if (DialogDescriptionFile == NULL)
		return;

	int rootChildCount = DialogDescriptionFile->getChildCount();
	for (int i = 0; i < rootChildCount; i++)
	{
		XmlNodeRef pDialogItem = DialogDescriptionFile->getChild(i);
		if (!pDialogItem)
			return;

		const char* tagName = pDialogItem->getTag();
		if (!strcmp(tagName, "Phrase"))
		{
			SDialogNPCAnswer *npcAnswer = new SDialogNPCAnswer();
			npcAnswer->currentPhraseId = atoi(pDialogItem->getAttr("id"));
			npcAnswer->nextPhraseId = atoi(pDialogItem->getAttr("next"));
			npcAnswer->answer = pDialogItem->getAttr("message");
			npcAnswer->audioDialogDuration = atoi(pDialogItem->getAttr("duration"));
			npcAnswer->audioName = pDialogItem->getAttr("audio");
			npcAnswer->withoutMessage = (atoi(pDialogItem->getAttr("initWithoutMessage")) == 1) ? true : false;
			npcAnswer->specialEvent = pDialogItem->getAttr("special");

			int phraseChildCount = pDialogItem->getChildCount();
			for (int j = 0; j < phraseChildCount; j++)
			{
				XmlNodeRef pAnswer = pDialogItem->getChild(j);
				if (!pAnswer)
					return;

				//Обработка экшенов
				if (!strcmp(pAnswer->getTag(), "actions"))
				{
					int actionsCount = pAnswer->getChildCount();
					for (int k = 0; k < actionsCount; k++)
					{
						XmlNodeRef action = pAnswer->getChild(k);
						if (action == NULL)
							return;

						const char* actionName = action->getTag();
						if (!strcmp(actionName, "inventory"))
						{
							SDialogNPCAnswer::SDialogActionInventory invAction;
							invAction.actionType = atoi(action->getAttr("type"));
							invAction.count = atoi(action->getAttr("count"));
							invAction.itemName = action->getAttr("item");
							npcAnswer->inventoryActions.push_back(invAction);
						}
						if (!strcmp(actionName, "quest"))
						{
							SDialogNPCAnswer::SDialogActionQuest questAction;
							questAction.status = atoi(action->getAttr("status"));
							questAction.questTitle = action->getAttr("title");
						}
					}
				}// ~actions

				//Player answers
				if (!strcmp(pAnswer->getTag(), "PlayerAnswer"))
				{
					SDialogPlayerAnswer* pPlayerAnswer = new SDialogPlayerAnswer();
					pPlayerAnswer->answer = pAnswer->getAttr("answer");
					pPlayerAnswer->nextDialogId = atoi(pAnswer->getAttr("next"));
					pPlayerAnswer->audioDialogDuration = atoi(pAnswer->getAttr("duration"));
					pPlayerAnswer->audioName = pAnswer->getAttr("audio");
					pPlayerAnswer->parentId = atoi(pAnswer->getAttr("parentId"));
					m_pPlayerAnswer.push_back(pPlayerAnswer);
				}
			}//~for j
			m_pNPCAnswer.push_back(npcAnswer);
		}//~if Phrase
	}//~for i
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
	Vec3 cameraPosition = Vec3(0);
	m_pCurrentQuestCamera = gEnv->pEntitySystem->FindEntityByName(CameraName);
	if (!m_pCurrentQuestCamera)
		return;

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
	if (m_pAudioTrigger == NULL)
	{
		SEntitySpawnParams spawnParams;
		spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("AudioTriggerSpot");
		m_pAudioTrigger = gEnv->pEntitySystem->SpawnEntity(spawnParams);
	}
	
	if (strcmp(AudioName, "") && m_pAudioTrigger != NULL)
	{
		SmartScriptTable propertiesTable;
		m_pAudioTrigger->SetPos(m_pTarget->GetPos() + Vec3(0, 0, 3));
		IScriptTable *pStriptTable = m_pAudioTrigger->GetScriptTable();

		bool hasProperties = pStriptTable->GetValue("Properties", propertiesTable);
		if (hasProperties)
		{
			propertiesTable->SetValue("audioTriggerPlayTriggerName", AudioName);
			propertiesTable->SetValue("bEnabled", bEnabled);

			Script::CallMethod(pStriptTable, "OnPropertyChange");
		}
	}
}

//-------------------------------------------------------------------
void CUIDialogMenu::SetRenderStatus(bool bOn)
{
	IRenderer* pRenderer = gEnv->pRenderer;
	CRY_ASSERT(pRenderer);
	pRenderer->EF_SetPostEffectParam("Dof_User_Active", 1.f, true);
	pRenderer->EF_SetPostEffectParam("Dof_User_FocusDistance", 8000, true);
	pRenderer->EF_SetPostEffectParam("Dof_User_FocusRange", 0, true);
	pRenderer->EF_SetPostEffectParam("Dof_User_BlurAmount", 1, true);
	pRenderer->EF_SetPostEffectParam("Dof_User_ScaleCoc", 12, true);
}