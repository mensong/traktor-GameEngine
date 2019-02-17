#pragma once

#include <fbxsdk.h>
#include "Core/Math/Color4f.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector2.h"

namespace traktor
{
	namespace model
	{

Vector2 convertVector2(const FbxVector2& v);

Vector4 convertVector4(const FbxVector4& v);

Vector4 convertNormal(const FbxVector4& v);

Vector4 convertPosition(const FbxVector4& v);

Quaternion convertQuaternion(const FbxQuaternion& v);

Matrix44 convertMatrix(const FbxMatrix& m);

Matrix44 convertMatrix(const FbxAMatrix& m);

traktor::Transform convertTransform(const FbxAMatrix& m);

Color4f convertColor(const FbxColor& c);

Matrix44 calculateAxisTransform(const FbxAxisSystem& axisSystem);

	}
}
