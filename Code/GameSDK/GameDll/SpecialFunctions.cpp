#include "StdAfx.h"
#include "SpecialFunctions.h"
#include "Player.h"


Vec3 CSpecialFunctions::GetBonePos(const char *bname, bool local, IEntity *pEntity)
{
	Vec3 BonePos = ZERO;
	if (pEntity == NULL || bname == NULL)
		return BonePos;

	ICharacterInstance* pCharInst = pEntity->GetCharacter(0);
	if (!pCharInst)
		return BonePos;

	ISkeletonPose *pISkeletonPose = pCharInst->GetISkeletonPose();
	if (pISkeletonPose)
	{
		int16 bid = -1; 
		bid = pEntity->GetCharacter(0)->GetIDefaultSkeleton().GetJointIDByName(bname);
		Vec3 vRootBone = pISkeletonPose->GetAbsJointByID(0).t;
		vRootBone.z = 0;
		if (bid != -1)
		{
			if (local)
			{
				BonePos = (pISkeletonPose->GetAbsJointByID(bid).t - vRootBone);
			}
			else
			{
				BonePos = (pEntity->GetWorldTM() * (pISkeletonPose->GetAbsJointByID(bid).t - vRootBone));
			}
		}
	}
	return BonePos;
}