/*********************************************************************************************************
- File: BuildMenu.h
- Description: �������� ���� ���� ������ ������� � ����������� ������� ������� � ����� ���� � ����
- Created by: ����� ����
- Date: 25.03.2014
- Diko Source File
- ToDo:�������� ���������� ������������� �����
********************************************************************************************************/

#ifndef __UIBuildMenu__
#define __UIBuildMenu__

#include <IFlashUI.h>
#include "IActionMapManager.h"
#include "IGameFramework.h"
#include "Camera.h"
#include "BuildSelectionHelper.h"


//����� ������������ ��� �������� ������� ��� ��
class CUIBuildMenuEventListener : public IUIElementEventListener
{
public:
	CUIBuildMenuEventListener();

	//��������:
	//������� ����������� ��� ��������� UI-�������
	void OnUIEvent(IUIElement* pSender, const SUIEventDesc& Event, const SUIArguments& args);
};


//������ ����� ������������ ����� �������� ������ � ���� ���������:
//�����������, ���������� ������ ������, ������������ ������� � ��� �� ���� �������������.
class CUIBuildMenu : public IGameFrameworkListener, public IActionListener
{
public:
	CUIBuildMenu();
	~CUIBuildMenu();

	//��������:
	//� ������ ������ ���������� ������������� � ��������� ��������� �� ����-����.
	//����������: true, ���� ��� ����, ����� false.
	bool Init();

	//��������:
	//������� ��������/��������� ������� UI-�������(m_pBuildMenu), � ��� �� ��������� ��� ����������� ��� �������� �������
	//���������: true - ����������, flase - ������
	void DisplayBuildMenu(bool bShow);


	//������� ����-������� ����
	//��������:
	//������� ���������� �� ���� ������ ��� ������������� ������� ��������, ������� ����� ���������
	//��� ����� ���������� ��������� ��������� SBuilding � ��������� � � ������ ����������� �-�����.
	//���������: ��������� �� ���������, ����������� ���������� ������.
	//����������: ��� ��������� ������ ������� � XML-�����.
	void SendBuildingsInfo(SBuilding* build);

	//��������:
	//������� ������ ������ ������ �� ����-����.
	void Clear();

	//��������:
	//������� ������������� ��� ������ ���� ������������� ������� ������
	void ShowAcceptMenu(bool bAccept);

	//��������:
	//����������� �������� ����, �� ������� ��������� ������ ��������� ������ ��� �������� ����� �������������.
	//���������: true - ��������, false - ������
	void ShowSlideMenu(bool bShow = false);

	//��������:
	//������� ���� ������� ��� ������� ���� ����� ����� ���������� �������(0.01 ���)
	//��� ���� ������� ��-�� ����, ��� ������� ��������, ���� ������� ���� �� ������.
	void Delay();
	//~

	//������� ������ � �������:
	//������� � ��������� ����� ���������, ������� � ���������� �����.
	//��������:
	//������� �������� ��������� ������, ��� �������� ��������� ������.
	void NextCamView();
	void PriveousCamView();
	//~


	//IGameFrameworkListener
	//��������:
	//� ������ ������ ������������ ��� ������ � ��������.
	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame){}
	void OnLoadGame(ILoadGame* pLoadGame){}
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event){}
	//~


	//������� ��� ������ � ������� �������� �� �����
	//��������:
	//������� ������� ������� ������
	//���������: ������ �� ��������� ActionId, � ������� ������ �����, ������� ��� ������
	//��� ������ ������� � defaultProfile.xml, ��������� � GameActions.action,
	//activationmode: 1 - onMouseDown, 2 - onMouseUp, 4 - Hold.
	void OnAction(const ActionId& action, int activationMode, float value);
	void SetSelectedEntity(IEntity *pEnt){ m_pSelectedEntity = pEnt; }
	IEntity* GetSelectedEntity(){ return m_pSelectedEntity; }
	CSelectionBuildHelper* GetSelectionHelper(){ return m_Helper; }
	//~

	bool IsBuildModeEnable() { return m_bBuildModeEnable; }
	void SetBuildMode(bool seter){ m_bBuildModeEnable = seter; }

	friend class CSelectionBuildHelper;
	friend class CUIBuildMenuEventListener;

	std::vector <SBuilding*> GetPerformedBuildings(){
		return m_PerformedBuildings;
	}
private:
	CUIBuildMenuEventListener g_CUIBuildSystemEventListener; //������ ���������.
	CSelectionBuildHelper* m_Helper; //������, ������� ������
	CBuildCamera *m_BuildCamera; //������� ������ ���������
	IUIElement *m_pBuildMenu; //��������� �� ����-���� ���������
	IEntity* m_pSelectedEntity; //������, ����������� � IEntity

	float m_fInitialTime; //������ ������� ��� �������� ����
	bool m_bCloseEvent; // ���� ���� ������ ������ �������� - true;
	bool m_bBuildModeEnable; //true - ���� ����� �������

	std::vector <SBuilding*> m_PerformedBuildings; // ����������� ������
};

#endif