#include "StdAfx.h"
#include "MouseDirection.h"
#include "IHardwareMouse.h"

CMouseDirection::CMouseDirection()
{
	m_bDragEnable = false;
	m_fLastAngle = 0.f;
}

void CMouseDirection::SetPoint()
{
	IHardwareMouse *pMouse = gEnv->pHardwareMouse;
	pMouse->GetHardwareMouseClientPosition(&m_FirstPoint.x, &m_FirstPoint.y);
	m_bDragEnable = true;

	//m_FirstPoint.x = gEnv->pSystem->GetIRenderer()->GetWidth() / 2;
	//m_FirstPoint.y = gEnv->pSystem->GetIRenderer()->GetHeight() / 2;
	//pMouse->SetHardwareMousePosition(m_FirstPoint.x, m_FirstPoint.y);
}


float CMouseDirection::ClosePath()
{
	bool OverPI = false;
	Vec2 ij(1, 0);
	IHardwareMouse *pMouse = gEnv->pHardwareMouse;

	pMouse->GetHardwareMousePosition(&m_SecondPoint.x, &m_SecondPoint.y);
	m_Direction = m_SecondPoint - m_FirstPoint;

	if (m_SecondPoint.y - m_FirstPoint.y > 0)
		OverPI = true;

	float _ang = 180 / gf_PI * acos(ij * m_Direction.NormalizeSafe());
	if (OverPI)
		_ang = 180 + (180 - _ang);

	//CryLogAlways("Angle: %f", _ang);
	m_bDragEnable = false;
	m_fLastAngle = _ang;
	return _ang;
}


float CMouseDirection::GetAngle(float _x, float _y)
{
	Vec2 SecondPoint(_x, _y);
	Vec2 Direction = SecondPoint - m_FirstPoint;
	Direction.NormalizeSafe();

	bool OverPI = false;
	Vec2 ij(1, 0);
	if (SecondPoint.y - m_FirstPoint.y > 0)
		OverPI = true;

	float _ang = 180 / gf_PI * acos(ij * m_Direction);
	if (OverPI) _ang = 180 + (180 - _ang);
	return _ang;
}

Vec2 CMouseDirection::PerfomClosePath()
{
	IHardwareMouse *pMouse = gEnv->pHardwareMouse;

	pMouse->GetHardwareMousePosition(&m_SecondPoint.x, &m_SecondPoint.y);
	m_Direction = m_SecondPoint - m_FirstPoint;
	return m_Direction;
}