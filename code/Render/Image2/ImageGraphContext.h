#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ScreenRenderer;

/*!
 * \ingroup Render
 */
struct ImageGraphParams
{
    Frustum viewFrustum;
    Matrix44 lastView;
    Matrix44 view;
    Matrix44 viewToLight;
    Matrix44 projection;
    Vector4 godRayDirection;
    Vector4 shadowMapUvTransform;
    int32_t sliceCount;
    int32_t sliceIndex;
    float sliceNearZ;
    float sliceFarZ;
    float shadowFarZ;
    float shadowMapBias;
    float deltaTime;
    float time;

    ImageGraphParams()
    :	shadowMapUvTransform(0.0f, 0.0f, 1.0f, 1.0f)
    ,	sliceCount(0)
    ,	sliceIndex(0)
    ,	sliceNearZ(0.0f)
    ,	sliceFarZ(0.0f)
    ,	shadowFarZ(0.0f)
    ,	shadowMapBias(0.0f)
    ,	deltaTime(0.0f)
    ,   time(0.0f)
    {
    }
};

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageGraphContext
{
public:
	struct TextureTargetSet
	{
		handle_t targetSetId;
		int32_t colorIndex;
	};

    explicit ImageGraphContext(ScreenRenderer* screenRenderer);

    ScreenRenderer* getScreenRenderer() const { return m_screenRenderer; }

	void associateTextureTargetSet(handle_t textureId, handle_t targetSetId, int32_t colorIndex);

    void associateTextureTargetSetDepth(handle_t textureId, handle_t targetSetId);

	TextureTargetSet findTextureTargetSet(handle_t textureId) const;

    void setParams(const ImageGraphParams& params) { m_params = params; }

    const ImageGraphParams& getParams() const { return m_params; }

private:
    Ref< ScreenRenderer > m_screenRenderer;
	SmallMap< handle_t, TextureTargetSet > m_textureTargetSet;
    ImageGraphParams m_params;
};

	}
}