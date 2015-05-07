/***********************************
*
*
***********************************/
#ifndef _ARROW_H_
#define _ARROW_H_

#if _MSC_VER > 1000
# pragma once
#endif

#include "Bullet.h"


class CArrow : public CProjectile
{
public:
	CArrow();
	virtual ~CArrow();

	void Pickalize(bool pick, Vec3 pos, Quat rot);

	void OnHit(const HitInfo&);
	void Launch(const Vec3 &pos, const Vec3 &dir, const Vec3 &velocity, float speedScale = 1.0f);
	void HandleEvent(const SGameObjectEvent &event);
	void Update(SEntityUpdateContext &ctx, int updateSlot);

	const char* ProcessHit(CGameRules& gameRules, const EventPhysCollision& collision, IEntity& target, float damage, int hitMatId, const Vec3& hitDir);
	void SetParams(const SProjectileDesc& projectileDesc);

	void AttachArrow(IEntity* pTarget, const char* boneName);
private:
	const SAmmoParams* m_pAmmoParams;
	IEntity* pArrowEntity;
	Quat launchRotation;
};

#endif // _ARROW_H_