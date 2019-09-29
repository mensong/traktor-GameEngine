#include "Core/Math/Random.h"
#include "Render/ICubeTexture.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "World/Entity/ProbeFilterer.h"

namespace traktor
{
    namespace world
    {
        namespace
        {

#if !defined(__ANDROID__)
const int32_t c_faceSize = 512;
#else
const int32_t c_faceSize = 128;
#endif
const int32_t c_sampleCount = 1000;

const resource::Id< render::Shader > c_idFilterShader(Guid(L"{D9CC2267-0BDF-4A19-A970-856112821734}"));

double halton(int32_t index, int32_t base)
{
	double f = 1, r = 0;
	while (index > 0)
	{
		f = f / base;
		r = r + f * (index % base);
		index = index / base;
	}
	return r;
}

Vector4 sampleDirectionZ(float roughness, const Vector2& h, Random& rnd)
{
	double rx = clamp< float >(h.x + rnd.nextDouble() * 0.1f - 0.05f, 0.0f, 1.0f);
	double ry = clamp< float >(h.y + rnd.nextDouble() * 0.1f - 0.05f, 0.0f, 1.0f);

	float a = roughness * roughness;
	float phi = 2.0f * PI * rx;
	float cosTheta = std::sqrt((1.0f - ry) / (1.0f + (a * a - 1.0f) * ry));
	float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

	Vector4 v(
		sinTheta * std::cos(phi),
		sinTheta * std::sin(phi),
		cosTheta,
		0.0f
	);

	if (v.z() < 0.0f)
		v = -v;

	return v;
}

        }

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ProbeFilterer", ProbeFilterer, Object)

ProbeFilterer::ProbeFilterer(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:   m_resourceManager(resourceManager)
,   m_renderSystem(renderSystem)
{
}

bool ProbeFilterer::create()
{
    if (!m_resourceManager->bind(c_idFilterShader, m_filterShader))
        return false;

    m_screenRenderer = new render::ScreenRenderer();
    if (!m_screenRenderer->create(m_renderSystem))
        return false;

    const int32_t mipCount = log2(c_faceSize) + 1;
    m_renderTargetSets.resize(mipCount);

    for (int32_t mip = 1; mip < mipCount; ++mip)
    {
        render::RenderTargetSetCreateDesc rtscd;
        rtscd.count = 1;
        rtscd.width = c_faceSize >> mip;
        rtscd.height = c_faceSize >> mip;
        rtscd.multiSample = 0;
        rtscd.createDepthStencil = false;
        rtscd.usingDepthStencilAsTexture = false;
        rtscd.usingPrimaryDepthStencil = false;
        rtscd.preferTiled = false;
        rtscd.ignoreStencil = true;
        rtscd.generateMips = false;
        rtscd.targets[0].format = render::TfR11G11B10F;
        rtscd.targets[0].sRGB = false;

        m_renderTargetSets[mip] = m_renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
        if (!m_renderTargetSets[mip])
            return false;
    }

	m_halton.resize(c_sampleCount);
	for (int32_t i = 0; i < c_sampleCount; ++i)
	{
		m_halton[i] = Vector2(
			(float)halton(i, 2),
			(float)halton(i, 3)
		);
	}

    return true;
}

void ProbeFilterer::render(render::IRenderView* renderView, render::ICubeTexture* probeTexture)
{
	AlignedVector< Vector4 > sampleDirections(c_sampleCount);
	Vector4 corners[4];
	Vector4 tangent;
	Random random;

    const int32_t mipCount = log2(c_faceSize) + 1;

	render::Clear clear = { 0 };
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
    
    for (int32_t mip = 1; mip < mipCount; ++mip)
    {
		float roughness = (float)mip / mipCount;

		for (int32_t i = 0; i < c_sampleCount; ++i)
		{
			const Vector4 c_unit(0.0f, 0.0f, 1.0f);
			sampleDirections[i] = sampleDirectionZ(roughness, m_halton[i], random);
		}

		for (int32_t side = 0; side < 6; ++side)
		{
			switch (side)
			{
			case 0:
				corners[0].set( 1.0f,  1.0f,  1.0f, 0.0f);
				corners[1].set( 1.0f,  1.0f, -1.0f, 0.0f);
				corners[2].set( 1.0f, -1.0f,  1.0f, 0.0f);
				corners[3].set( 1.0f, -1.0f, -1.0f, 0.0f);
				tangent.set(0.0f, 0.0f, -1.0f);
				break;

			case 1:
				corners[0].set(-1.0f,  1.0f, -1.0f, 0.0f);
				corners[1].set(-1.0f,  1.0f,  1.0f, 0.0f);
				corners[2].set(-1.0f, -1.0f, -1.0f, 0.0f);
				corners[3].set(-1.0f, -1.0f,  1.0f, 0.0f);
				tangent.set(0.0f, 0.0f, 1.0f);
				break;

			case 2:
				corners[0].set(-1.0f,  1.0f, -1.0f, 0.0f);
				corners[1].set( 1.0f,  1.0f, -1.0f, 0.0f);
				corners[2].set(-1.0f,  1.0f,  1.0f, 0.0f);
				corners[3].set( 1.0f,  1.0f,  1.0f, 0.0f);
				tangent.set(-1.0f, 0.0f, 0.0f);
				break;

			case 3:
				corners[0].set(-1.0f, -1.0f,  1.0f, 0.0f);
				corners[1].set( 1.0f, -1.0f,  1.0f, 0.0f);
				corners[2].set(-1.0f, -1.0f, -1.0f, 0.0f);
				corners[3].set( 1.0f, -1.0f, -1.0f, 0.0f);
				tangent.set(1.0f, 0.0f, 0.0f);
				break;

			case 4:
				corners[0].set(-1.0f,  1.0f,  1.0f, 0.0f);
				corners[1].set( 1.0f,  1.0f,  1.0f, 0.0f);
				corners[2].set(-1.0f, -1.0f,  1.0f, 0.0f);
				corners[3].set( 1.0f, -1.0f,  1.0f, 0.0f);
				tangent.set(0.0f, -1.0f, 0.0f);
				break;

			case 5:
				corners[0].set( 1.0f,  1.0f, -1.0f, 0.0f);
				corners[1].set(-1.0f,  1.0f, -1.0f, 0.0f);
				corners[2].set( 1.0f, -1.0f, -1.0f, 0.0f);
				corners[3].set(-1.0f, -1.0f, -1.0f, 0.0f);
				tangent.set(0.0f, 1.0f, 0.0f);
				break;
			}

			m_filterShader->setTextureParameter(L"World_ProbeTexture", probeTexture);
			m_filterShader->setVectorParameter(L"World_ProbeSampleTangent", tangent);
			m_filterShader->setVectorArrayParameter(L"World_ProbeSampleDirections", sampleDirections.c_ptr(), sampleDirections.size());
			m_filterShader->setVectorArrayParameter(L"World_ProbeFilterCorners", corners, sizeof_array(corners));

            renderView->begin(m_renderTargetSets[mip], &clear);
            m_screenRenderer->draw(renderView, m_filterShader);
            renderView->end();
            renderView->copy(
                probeTexture,
                side,
                mip,
                m_renderTargetSets[mip]->getColorTexture(0),
                0,
                0
            );
        }
    }
}

    }
}