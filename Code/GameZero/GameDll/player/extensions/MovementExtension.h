// CryEngine Header File.
// Copyright (C), Crytek, 1999-2015.


#pragma once


class CMovementExtension : public CGameObjectExtensionHelper<CMovementExtension, ISimpleExtension>
{
public:
	// ISimpleExtension
	virtual void PostInit(IGameObject* pGameObject) override;
	virtual void PostUpdate(float frameTime) override;
	virtual void Release() override;
	// ~ISimpleExtension
	
	CMovementExtension();
	virtual ~CMovementExtension();

private:
	float m_movementSpeed;
	float m_boostMultiplier;
};
