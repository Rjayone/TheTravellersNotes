/*********************************************************************************************************
- File: MathUtils.h
- Description: �������������� ����������
- Created by: ����� ����/ ����� Richmar
- Date: 25.03.2014
- Diko Source File
- ToDo:�������� ������������ ��� �������� �����
********************************************************************************************************/

#ifndef _MathUtils_
#define _MathUtils_

#include "Cry_Math.h"

struct MathUtils
{
	static Vec3 Ang2Dir(Ang3 angle)
	{
		float nx = +(cos(angle.z)*sin(angle.y)*sin(angle.x) - sin(angle.z)*cos(angle.x));
		float ny = +(sin(angle.z)*sin(angle.y)*sin(angle.x) + cos(angle.z)*cos(angle.x));
		float nz = +(cos(angle.y)*sin(angle.x));

		Vec3 ConvertedAngle(nx, ny, nz);

		return ConvertedAngle;
	}

	static Vec3 Ang2DirIgnoreZ(Ang3 angle)
	{
		float nx = +(cos(angle.z)*sin(angle.y)*sin(angle.x) - sin(angle.z)*cos(angle.x));
		float ny = +(sin(angle.z)*sin(angle.y)*sin(angle.x) + cos(angle.z)*cos(angle.x));
		float nz = +(cos(angle.y)*sin(angle.x));

		Vec3 ConvertedAngle(nx, ny, 0);

		return ConvertedAngle;
	}
};

#endif