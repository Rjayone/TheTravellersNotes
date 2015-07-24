// CryEngine Header File.
// Copyright (C), Crytek, 1999-2015.


#pragma once


class CViewExtension : public CGameObjectExtensionHelper<CViewExtension, ISimpleExtension>, IGameObjectView
{
public:
	// ISimpleExtension
	virtual void PostInit(IGameObject* pGameObject) override;
	virtual void Release() override;
	// ~ISimpleExtension

	// IGameObjectView
	virtual void UpdateView(SViewParams& params) override;
	virtual void PostUpdateView(SViewParams& viewParams) override {}
	// ~IGameObjectView

	CViewExtension();
	virtual ~CViewExtension();

private:
	void CreateView();

	unsigned int m_viewId;
	float m_camFOV;
};
