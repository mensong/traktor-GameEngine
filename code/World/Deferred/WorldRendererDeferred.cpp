#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/StructBuffer.h"
#include "Render/StructElement.h"
#include "Render/Context/RenderContext.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Render/ImageProcess/ImageProcessTargetPool.h"
#include "Resource/IResourceManager.h"
#include "World/IrradianceGrid.h"
#include "World/WorldContext.h"
#include "World/Deferred/LightRendererDeferred.h"
#include "World/Deferred/WorldRendererDeferred.h"
#include "World/Deferred/WorldRenderPassDeferred.h"
#include "World/Entity/GroupEntity.h"
#include "World/SMProj/UniformShadowProjection.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const resource::Id< render::ImageProcessSettings > c_colorTargetCopy(Guid(L"{7DCC28A2-C357-B54F-ACF4-8159301B1764}"));
const resource::Id< render::ImageProcessSettings > c_ambientOcclusionLow(Guid(L"{ED4F221C-BAB1-4645-BD08-84C5B3FA7C20}"));		//< SSAO, half size
const resource::Id< render::ImageProcessSettings > c_ambientOcclusionMedium(Guid(L"{A4249C8A-9A0D-B349-B0ED-E8B354CD7BDF}"));	//< SSAO, full size
const resource::Id< render::ImageProcessSettings > c_ambientOcclusionHigh(Guid(L"{37F82A38-D632-5541-9B29-E77C2F74B0C0}"));		//< HBAO, half size
const resource::Id< render::ImageProcessSettings > c_ambientOcclusionUltra(Guid(L"{C1C9DDCB-2F82-A94C-BF65-653D8E68F628}"));	//< HBAO, full size
const resource::Id< render::ImageProcessSettings > c_antiAliasNone(Guid(L"{960283DC-7AC2-804B-901F-8AD4C205F4E0}"));
const resource::Id< render::ImageProcessSettings > c_antiAliasLow(Guid(L"{DBF2FBB9-1310-A24E-B443-AF0D018571F7}"));
const resource::Id< render::ImageProcessSettings > c_antiAliasMedium(Guid(L"{3E1D810B-339A-F742-9345-4ECA00220D57}"));
const resource::Id< render::ImageProcessSettings > c_antiAliasHigh(Guid(L"{0C288028-7BFD-BE46-A25F-F3910BE50319}"));
const resource::Id< render::ImageProcessSettings > c_antiAliasUltra(Guid(L"{4750DA97-67F4-E247-A9C2-B4883B1158B2}"));
const resource::Id< render::ImageProcessSettings > c_gammaCorrection(Guid(L"{AB0ABBA7-77BF-0A4E-8E3B-4987B801CE6B}"));
const resource::Id< render::ImageProcessSettings > c_motionBlurPrime(Guid(L"{73C2C7DC-BD77-F348-A6B7-06E0EFB633D9}"));
const resource::Id< render::ImageProcessSettings > c_motionBlurLow(Guid(L"{BDFEFBE0-C5E9-2643-B445-DB02AC5C7687}"));
const resource::Id< render::ImageProcessSettings > c_motionBlurMedium(Guid(L"{A70CBA02-B75A-E246-A9B6-99B8B2B98D2A}"));
const resource::Id< render::ImageProcessSettings > c_motionBlurHigh(Guid(L"{E893B98C-90A3-9848-B4F3-3D8C0CE57CE8}"));
const resource::Id< render::ImageProcessSettings > c_motionBlurUltra(Guid(L"{CD4A0939-233B-2E43-988D-DA6E0DB7A6E6}"));
const resource::Id< render::ImageProcessSettings > c_toneMapFixed(Guid(L"{838922A0-49CE-6645-8A9C-BA0E71081033}"));
const resource::Id< render::ImageProcessSettings > c_toneMapAdaptive(Guid(L"{BC4FA128-A976-4023-A422-637581ADFD7E}"));

render::handle_t s_techniqueDeferredColor = 0;
render::handle_t s_techniqueDeferredGBufferWrite = 0;
render::handle_t s_techniqueReflectionWrite = 0;
render::handle_t s_techniqueIrradianceWrite = 0;
render::handle_t s_techniqueVelocityWrite = 0;
render::handle_t s_techniqueShadow = 0;
render::handle_t s_handleTime = 0;
render::handle_t s_handleView = 0;
render::handle_t s_handleViewInverse = 0;
render::handle_t s_handleProjection = 0;
render::handle_t s_handleColorMap = 0;
render::handle_t s_handleOcclusionMap = 0;
render::handle_t s_handleDepthMap = 0;
render::handle_t s_handleLightMap = 0;
render::handle_t s_handleNormalMap = 0;
render::handle_t s_handleMiscMap = 0;
render::handle_t s_handleReflectionMap = 0;
render::handle_t s_handleFogDistanceAndDensity = 0;
render::handle_t s_handleFogColor = 0;
render::handle_t s_handleLightCount = 0;
render::handle_t s_handleLightSBuffer = 0;
render::handle_t s_handleTileSBuffer = 0;
render::handle_t s_handleIrradianceGridSize = 0;
render::handle_t s_handleIrradianceGridSBuffer = 0;
render::handle_t s_handleIrradianceGridBoundsMin = 0;
render::handle_t s_handleIrradianceGridBoundsMax = 0;

#pragma pack(1)

struct LightShaderData
{
	float typeRangeRadius[4];
	float position[4];
	float direction[4];
	float color[4];
	float viewToLight0[4];
	float viewToLight1[4];
	float viewToLight2[4];
	float viewToLight3[4];
	float atlasTransform[4];
};

struct TileShaderData
{
	float lights[4];
	float lightCount[4];
};

#pragma pack()

resource::Id< render::ImageProcessSettings > getAmbientOcclusionId(Quality quality)
{
	switch (quality)
	{
	default:
	case QuDisabled:
		return resource::Id< render::ImageProcessSettings >();
	case QuLow:
		return c_ambientOcclusionLow;
	case QuMedium:
		return c_ambientOcclusionMedium;
	case QuHigh:
		return c_ambientOcclusionHigh;
	case QuUltra:
		return c_ambientOcclusionUltra;
	}
}

resource::Id< render::ImageProcessSettings > getAntiAliasId(Quality quality)
{
	switch (quality)
	{
	default:
	case QuDisabled:
		return c_antiAliasNone;
	case QuLow:
		return c_antiAliasLow;
	case QuMedium:
		return c_antiAliasMedium;
	case QuHigh:
		return c_antiAliasHigh;
	case QuUltra:
		return c_antiAliasUltra;
	}
}

resource::Id< render::ImageProcessSettings > getMotionBlurId(Quality quality)
{
	switch (quality)
	{
	default:
	case QuDisabled:
		return resource::Id< render::ImageProcessSettings >();
	case QuLow:
		return c_motionBlurLow;
	case QuMedium:
		return c_motionBlurMedium;
	case QuHigh:
		return c_motionBlurHigh;
	case QuUltra:
		return c_motionBlurUltra;
	}
}

resource::Id< render::ImageProcessSettings > getToneMapId(WorldRenderSettings::ExposureMode exposureMode)
{
	switch (exposureMode)
	{
	default:
	case WorldRenderSettings::EmFixed:
		return c_toneMapFixed;
	case WorldRenderSettings::EmAdaptive:
		return c_toneMapAdaptive;
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererDeferred", 0, WorldRendererDeferred, IWorldRenderer)

WorldRendererDeferred::WorldRendererDeferred()
:	m_toneMapQuality(QuDisabled)
,	m_motionBlurQuality(QuDisabled)
,	m_shadowsQuality(QuDisabled)
,	m_ambientOcclusionQuality(QuDisabled)
,	m_antiAliasQuality(QuDisabled)
{
	// Techniques
	s_techniqueDeferredColor = render::getParameterHandle(L"World_DeferredColor");
	s_techniqueDeferredGBufferWrite = render::getParameterHandle(L"World_DeferredGBufferWrite");
	s_techniqueReflectionWrite = render::getParameterHandle(L"World_ReflectionWrite");
	s_techniqueIrradianceWrite = render::getParameterHandle(L"World_IrradianceWrite");
	s_techniqueVelocityWrite = render::getParameterHandle(L"World_VelocityWrite");
	s_techniqueShadow = render::getParameterHandle(L"World_ShadowWrite");

	// Global parameters.
	s_handleTime = render::getParameterHandle(L"World_Time");
	s_handleView = render::getParameterHandle(L"World_View");
	s_handleViewInverse = render::getParameterHandle(L"World_ViewInverse");
	s_handleProjection = render::getParameterHandle(L"World_Projection");
	s_handleColorMap = render::getParameterHandle(L"World_ColorMap");
	s_handleOcclusionMap = render::getParameterHandle(L"World_OcclusionMap");
	s_handleDepthMap = render::getParameterHandle(L"World_DepthMap");
	s_handleLightMap = render::getParameterHandle(L"World_LightMap");
	s_handleNormalMap = render::getParameterHandle(L"World_NormalMap");
	s_handleMiscMap = render::getParameterHandle(L"World_MiscMap");
	s_handleReflectionMap = render::getParameterHandle(L"World_ReflectionMap");
	s_handleFogDistanceAndDensity = render::getParameterHandle(L"World_FogDistanceAndDensity");
	s_handleFogColor = render::getParameterHandle(L"World_FogColor");
	s_handleLightCount = render::getParameterHandle(L"World_LightCount");
	s_handleLightSBuffer = render::getParameterHandle(L"World_LightSBuffer");
	s_handleTileSBuffer = render::getParameterHandle(L"World_TileSBuffer");
	s_handleIrradianceGridSize = render::getParameterHandle(L"World_IrradianceGridSize");
	s_handleIrradianceGridSBuffer = render::getParameterHandle(L"World_IrradianceGridSBuffer");
	s_handleIrradianceGridBoundsMin = render::getParameterHandle(L"World_IrradianceGridBoundsMin");
	s_handleIrradianceGridBoundsMax = render::getParameterHandle(L"World_IrradianceGridBoundsMax");
}

bool WorldRendererDeferred::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	m_settings = *desc.worldRenderSettings;
	m_toneMapQuality = desc.toneMapQuality;
	m_motionBlurQuality = desc.motionBlurQuality;
	m_shadowSettings = m_settings.shadowSettings[desc.shadowsQuality];
	m_shadowsQuality = desc.shadowsQuality;
	m_reflectionsQuality = desc.reflectionsQuality;
	m_ambientOcclusionQuality = desc.ambientOcclusionQuality;
	m_antiAliasQuality = desc.antiAliasQuality;

	// Allocate frames.
	m_frames.resize(desc.frameCount);

	// Pack fog parameters.
	m_fogDistanceAndDensity = Vector4(
		m_settings.fogDistanceY,
		m_settings.fogDistanceZ,
		m_settings.fogDensityY,
		m_settings.fogDensityZ
	);
	m_fogColor = m_settings.fogColor;

	// Create post process target pool to enable sharing of targets between multiple processes.
	Ref< render::ImageProcessTargetPool > postProcessTargetPool = new render::ImageProcessTargetPool(renderSystem);

	// Create "gbuffer" targets.
	{
		render::RenderTargetSetCreateDesc rtscd;
		rtscd.count = 4;
		rtscd.width = desc.width;
		rtscd.height = desc.height;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
		rtscd.sharedDepthStencil = desc.sharedDepthStencil;
		rtscd.preferTiled = true;
		rtscd.storeDepthStencil = true;					// Store depth for z-cull later.
		rtscd.targets[0].format = render::TfR16F;		// Depth (R)
		rtscd.targets[1].format = render::TfR16G16F;	// Normals (RG)
		rtscd.targets[2].format = render::TfR11G11B10F;	// Metalness (R), Roughness (G), Specular (B)
		rtscd.targets[3].format = render::TfR11G11B10F;	// Surface color (RGB)

		m_gbufferTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
		if (!m_gbufferTargetSet)
		{
			log::error << L"Unable to create depth render target." << Endl;
			return false;
		}
	}

	// Create "ambient occlusion" target.
	{
		render::RenderTargetSetCreateDesc rtscd;
		rtscd.count = 1;
		rtscd.width = desc.width;
		rtscd.height = desc.height;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
		rtscd.sharedDepthStencil = desc.sharedDepthStencil;
		rtscd.preferTiled = true;
		rtscd.storeDepthStencil = true;
		rtscd.targets[0].format = render::TfR8;			// Ambient occlusion (R)

		m_ambientOcclusionTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
		if (!m_ambientOcclusionTargetSet)
		{
			log::error << L"Unable to create ambient occlusion render target." << Endl;
			return false;
		}
	}

	// Create "velocity" target.
	if (m_motionBlurQuality > QuDisabled)
	{
		render::RenderTargetSetCreateDesc rtscd;
		rtscd.count = 1;
		rtscd.width = desc.width;
		rtscd.height = desc.height;
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
		rtscd.sharedDepthStencil = desc.sharedDepthStencil;
		rtscd.preferTiled = true;
		rtscd.storeDepthStencil = false;
		rtscd.targets[0].format = render::TfR16G16F;

		m_velocityTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
		if (!m_velocityTargetSet)
		{
			log::error << L"Unable to create velocity render target; motion blur disabled." << Endl;
			m_motionBlurQuality = QuDisabled;
		}
	}

	// Create "color read-back" target.
	{
		render::RenderTargetSetCreateDesc rtscd;
		rtscd.count = 1;
		rtscd.width = previousLog2(desc.width);
		rtscd.height = previousLog2(desc.height);
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.preferTiled = true;
		rtscd.storeDepthStencil = false;
		rtscd.ignoreStencil = true;
		rtscd.generateMips = true;
#if !defined(__ANDROID__) && !defined(__IOS__)
		rtscd.targets[0].format = render::TfR16G16B16A16F;
#else
		rtscd.targets[0].format = render::TfR11G11B10F;
#endif

		m_colorTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
		if (!m_colorTargetSet)
		{
			log::error << L"Unable to create color read-back render target." << Endl;
			return false;
		}
	}

	// Create "reflections" target.
	if (m_reflectionsQuality > QuDisabled)
	{
		render::RenderTargetSetCreateDesc rtscd;
		rtscd.count = 1;
#if !defined(__ANDROID__) && !defined(__IOS__)
		rtscd.width = desc.width;
		rtscd.height = desc.height;
#else
		rtscd.width = desc.width / 2;
		rtscd.height = desc.height / 2;
#endif
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.preferTiled = true;
		rtscd.storeDepthStencil = false;
		rtscd.ignoreStencil = true;
		rtscd.generateMips = false;
#if !defined(__ANDROID__) && !defined(__IOS__)
		rtscd.targets[0].format = render::TfR16G16B16A16F;
#else
		rtscd.targets[0].format = render::TfR11G11B10F;
#endif

		m_reflectionsTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
		if (!m_reflectionsTargetSet)
		{
			log::error << L"Unable to create reflections render target." << Endl;
			return false;
		}
	}

	// Create "shadow map" targets.
	if (m_shadowsQuality > QuDisabled)
	{
		render::RenderSystemInformation info;
		renderSystem->getInformation(info);

		int32_t maxResolution = m_shadowSettings.resolution;
		if (info.dedicatedMemoryTotal < 512 * 1024 * 1024)
			maxResolution /= 2;

		int32_t resolution = min< int32_t >(
			nearestLog2((max< int32_t >(desc.width, desc.height) * 190) / 100),
			maxResolution
		);
		T_DEBUG(L"Using shadow map resolution " << resolution);

		m_shadowProjection = new UniformShadowProjection(resolution);

		// Create shadow screen projection processes.
		resource::Proxy< render::ImageProcessSettings > shadowMaskProject;
		if (!resourceManager->bind(m_shadowSettings.maskProject, shadowMaskProject))
		{
			log::warning << L"Unable to create shadow project process; shadows disabled." << Endl;
			m_shadowsQuality = QuDisabled;
		}

		if (m_shadowsQuality > QuDisabled)
		{
			m_shadowMaskProject = new render::ImageProcess();
			if (!m_shadowMaskProject->create(
				shadowMaskProject,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				desc.width / m_shadowSettings.maskDenominator,
				desc.height / m_shadowSettings.maskDenominator,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create shadow project process; shadows disabled." << Endl;
				m_shadowsQuality = QuDisabled;
			}
		}

		// Create shadow render targets.
		if (m_shadowsQuality > QuDisabled)
		{
			render::RenderTargetSetCreateDesc rtscd;

			// Create shadow cascade map target.
			rtscd.count = 0;
			rtscd.width = resolution;
			rtscd.height = resolution;
			rtscd.multiSample = 0;
			rtscd.createDepthStencil = true;
			rtscd.usingDepthStencilAsTexture = true;
			rtscd.usingPrimaryDepthStencil = false;
			rtscd.ignoreStencil = true;
			rtscd.preferTiled = true;
			rtscd.storeDepthStencil = true;	// Used as texture.
			if ((m_shadowCascadeTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W)) == nullptr)
				m_shadowsQuality = QuDisabled;

			// Create shadow screen mask map target.
			rtscd.count = 1;
			rtscd.width = desc.width / m_shadowSettings.maskDenominator;
			rtscd.height = desc.height / m_shadowSettings.maskDenominator;
			rtscd.multiSample = 0;
			rtscd.createDepthStencil = false;
			rtscd.usingDepthStencilAsTexture = false;
			rtscd.usingPrimaryDepthStencil = false;
			rtscd.ignoreStencil = true;
			rtscd.preferTiled = true;
			rtscd.storeDepthStencil = false;
			rtscd.targets[0].format = render::TfR8;
			rtscd.targets[0].sRGB = false;
			if ((m_shadowMaskTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W)) == nullptr)
				m_shadowsQuality = QuDisabled;

			// Create shadow atlas map target.
			rtscd.count = 0;
#if !defined(__ANDROID__) && !defined(__IOS__)
			rtscd.width =
			rtscd.height = 4096;
#else
			rtscd.width =
			rtscd.height = 1024;
#endif
			rtscd.multiSample = 0;
			rtscd.createDepthStencil = true;
			rtscd.usingDepthStencilAsTexture = true;
			rtscd.usingPrimaryDepthStencil = false;
			rtscd.ignoreStencil = true;
			rtscd.preferTiled = true;
			rtscd.storeDepthStencil = true;	// Used as texture.
			if ((m_shadowAtlasTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W)) == nullptr)
				m_shadowsQuality = QuDisabled;
		}

		// Ensure targets are destroyed if something went wrong in setup.
		if (m_shadowsQuality == QuDisabled)
		{
			safeDestroy(m_shadowCascadeTargetSet);
			safeDestroy(m_shadowMaskTargetSet);
			safeDestroy(m_shadowAtlasTargetSet);
		}
	}

	// Create "visual" and "intermediate" target.
	{
		render::RenderTargetSetCreateDesc rtscd;
		rtscd.count = 1;
		rtscd.width = desc.width;
		rtscd.height = desc.height;
		rtscd.multiSample = desc.multiSample;
		rtscd.createDepthStencil = false;
		rtscd.usingPrimaryDepthStencil = (desc.sharedDepthStencil == nullptr) ? true : false;
		rtscd.sharedDepthStencil = desc.sharedDepthStencil;
		rtscd.preferTiled = true;
		rtscd.storeDepthStencil = false;
#if !defined(__ANDROID__) && !defined(__IOS__)
		rtscd.targets[0].format = render::TfR32G32B32A32F;
#else
		rtscd.targets[0].format = render::TfR11G11B10F;
#endif

		m_visualTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
		if (!m_visualTargetSet)
			return false;

		m_intermediateTargetSet = renderSystem->createRenderTargetSet(rtscd, T_FILE_LINE_W);
		if (!m_intermediateTargetSet)
			return false;
	}

	// Create "color read-back" copy processing.
	{
		resource::Proxy< render::ImageProcessSettings > colorTargetCopy;

		if (!resourceManager->bind(c_colorTargetCopy, colorTargetCopy))
			log::warning << L"Unable to create color read-back processing; color read-back disabled." << Endl;

		if (colorTargetCopy)
		{
			m_colorTargetCopy = new render::ImageProcess();
			if (!m_colorTargetCopy->create(
				colorTargetCopy,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create color read-back processing; color read-back disabled." << Endl;
				m_colorTargetCopy = nullptr;
			}
		}
	}

	// Create ambient occlusion processing.
	{
		resource::Id< render::ImageProcessSettings > ambientOcclusionId = getAmbientOcclusionId(m_ambientOcclusionQuality);
		resource::Proxy< render::ImageProcessSettings > ambientOcclusion;

		if (ambientOcclusionId)
		{
			if (!resourceManager->bind(ambientOcclusionId, ambientOcclusion))
				log::warning << L"Unable to create ambient occlusion process; AO disabled." << Endl;
		}

		if (ambientOcclusion)
		{
			m_ambientOcclusion = new render::ImageProcess();
			if (!m_ambientOcclusion->create(
				ambientOcclusion,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create ambient occlusion process; AO disabled." << Endl;
				m_ambientOcclusion = nullptr;
			}
		}
	}

	// Create antialias processing.
	{
		resource::Id< render::ImageProcessSettings > antiAliasId = getAntiAliasId(m_antiAliasQuality);
		resource::Proxy< render::ImageProcessSettings > antiAlias;

		if (antiAliasId)
		{
			if (!resourceManager->bind(antiAliasId, antiAlias))
				log::warning << L"Unable to create antialias process; AA disabled." << Endl;
		}

		if (antiAlias)
		{
			m_antiAlias = new render::ImageProcess();
			if (!m_antiAlias->create(
				antiAlias,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create antialias process; AA disabled." << Endl;
				m_antiAlias = nullptr;
			}
		}
	}

	// Create "visual" post processing filter.
	{
		const resource::Id< render::ImageProcessSettings >& imageProcessSettings = desc.worldRenderSettings->imageProcess[desc.imageProcessQuality];
		if (imageProcessSettings)
		{
			resource::Proxy< render::ImageProcessSettings > imageProcess;
			if (!resourceManager->bind(imageProcessSettings, imageProcess))
				log::warning << L"Unable to create visual post processing image filter; post processing disabled." << Endl;

			if (imageProcess)
			{
				m_visualImageProcess = new render::ImageProcess();
				if (!m_visualImageProcess->create(
					imageProcess,
					postProcessTargetPool,
					resourceManager,
					renderSystem,
					desc.width,
					desc.height,
					desc.allTargetsPersistent
				))
				{
					log::warning << L"Unable to create visual post processing; post processing disabled." << Endl;
					m_visualImageProcess = nullptr;
				}
			}
		}
	}

	// Create gamma correction processing.
	if (
		m_settings.linearLighting &&
		std::abs(desc.gamma - 1.0f) > FUZZY_EPSILON
	)
	{
		resource::Proxy< render::ImageProcessSettings > gammaCorrection;
		if (!resourceManager->bind(c_gammaCorrection, gammaCorrection))
			log::warning << L"Unable to create gamma correction process; gamma correction disabled." << Endl;

		if (gammaCorrection)
		{
			m_gammaCorrectionImageProcess = new render::ImageProcess();
			if (m_gammaCorrectionImageProcess->create(
				gammaCorrection,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				m_gammaCorrectionImageProcess->setFloatParameter(render::getParameterHandle(L"World_Gamma"), desc.gamma);
				m_gammaCorrectionImageProcess->setFloatParameter(render::getParameterHandle(L"World_GammaInverse"), 1.0f / desc.gamma);
			}
			else
			{
				log::warning << L"Unable to create gamma correction process; gamma correction disabled." << Endl;
				m_gammaCorrectionImageProcess = nullptr;
			}
		}
	}

	// Create motion blur prime processing.
	if (m_motionBlurQuality > QuDisabled)
	{
		resource::Proxy< render::ImageProcessSettings > motionBlurPrime;
		if (!resourceManager->bind(c_motionBlurPrime, motionBlurPrime))
		{
			log::warning << L"Unable to create motion blur prime process; motion blur disabled." << Endl;
			m_motionBlurQuality = QuDisabled;
		}

		if (motionBlurPrime)
		{
			m_motionBlurPrimeImageProcess = new render::ImageProcess();
			if (!m_motionBlurPrimeImageProcess->create(
				motionBlurPrime,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create motion blur process; motion blur disabled." << Endl;
				m_motionBlurPrimeImageProcess = nullptr;
				m_motionBlurQuality = QuDisabled;
			}
		}
	}

	// Create motion blur final processing.
	if (m_motionBlurQuality > QuDisabled)
	{
		resource::Id< render::ImageProcessSettings > motionBlurId = getMotionBlurId(desc.motionBlurQuality);
		resource::Proxy< render::ImageProcessSettings > motionBlur;

		if (!resourceManager->bind(motionBlurId, motionBlur))
		{
			log::warning << L"Unable to create motion blur process; motion blur disabled." << Endl;
			m_motionBlurQuality = QuDisabled;
		}

		if (motionBlur)
		{
			m_motionBlurImageProcess = new render::ImageProcess();
			if (!m_motionBlurImageProcess->create(
				motionBlur,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
			{
				log::warning << L"Unable to create motion blur process; motion blur disabled." << Endl;
				m_motionBlurImageProcess = nullptr;
				m_motionBlurQuality = QuDisabled;
			}
		}
	}

	// Create tone map processing.
	if (m_toneMapQuality > QuDisabled)
	{
		resource::Id< render::ImageProcessSettings > toneMapId = getToneMapId(m_settings.exposureMode);
		resource::Proxy< render::ImageProcessSettings > toneMap;

		if (!resourceManager->bind(toneMapId, toneMap))
		{
			log::warning << L"Unable to create tone map process." << Endl;
			m_toneMapQuality = QuDisabled;
		}

		if (toneMap)
		{
			m_toneMapImageProcess = new render::ImageProcess();
			if (m_toneMapImageProcess->create(
				toneMap,
				postProcessTargetPool,
				resourceManager,
				renderSystem,
				desc.width,
				desc.height,
				desc.allTargetsPersistent
			))
				m_toneMapImageProcess->setFloatParameter(
					render::getParameterHandle(L"World_Exposure"),
					m_settings.exposure
				);
			else
			{
				log::warning << L"Unable to create tone map process; tone mapping disabled." << Endl;
				m_toneMapImageProcess = nullptr;
				m_toneMapQuality = QuDisabled;
			}
		}
	}

	// Allocate contexts.
	for (auto& frame : m_frames)
	{
		AlignedVector< render::StructElement > lightShaderDataStruct;
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, typeRangeRadius)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, position)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, direction)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, color)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, viewToLight0)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, viewToLight1)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, viewToLight2)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, viewToLight3)));
		lightShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(LightShaderData, atlasTransform)));
		T_FATAL_ASSERT(sizeof(LightShaderData) == render::getStructSize(lightShaderDataStruct));

		frame.lightSBuffer = renderSystem->createStructBuffer(
			lightShaderDataStruct,
			render::getStructSize(lightShaderDataStruct) * 1024
		);
		if (!frame.lightSBuffer)
			return false;

		AlignedVector< render::StructElement > tileShaderDataStruct;
		tileShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(TileShaderData, lights)));
		tileShaderDataStruct.push_back(render::StructElement(render::DtFloat4, offsetof(TileShaderData, lightCount)));
		T_FATAL_ASSERT(sizeof(TileShaderData) == render::getStructSize(tileShaderDataStruct));

		frame.tileSBuffer = renderSystem->createStructBuffer(
			tileShaderDataStruct,
			render::getStructSize(tileShaderDataStruct) * 16 * 16
		);
		if (!frame.tileSBuffer)
			return false;
	}

	// Create light primitive renderer.
	m_lightRenderer = new LightRendererDeferred();
	if (!m_lightRenderer->create(
		resourceManager,
		renderSystem
	))
	{
		log::error << L"Unable to create light primitive renderer" << Endl;
		return false;
	}

	// Create irradiance grid.
	if (!m_settings.irradianceGrid.isNull())
	{
		if (!resourceManager->bind(m_settings.irradianceGrid, m_irradianceGrid))
			return false;
	}

	// Determine slice distances.
	for (int32_t i = 0; i < m_shadowSettings.cascadingSlices; ++i)
	{
		float ii = float(i) / m_shadowSettings.cascadingSlices;
		float log = powf(ii, m_shadowSettings.cascadingLambda);
		m_slicePositions[i] = lerp(m_settings.viewNearZ, m_shadowSettings.farZ, log);
	}
	m_slicePositions[m_shadowSettings.cascadingSlices] = m_shadowSettings.farZ;

	m_entityRenderers = desc.entityRenderers;
	m_rootEntity = new GroupEntity();

	for (auto& frame : m_frames)
		frame.renderContext = new render::RenderContext(1 * 1024 * 1024);

	return true;
}

void WorldRendererDeferred::destroy()
{
	for (auto& frame : m_frames)
	{
		safeDestroy(frame.lightSBuffer);
		safeDestroy(frame.tileSBuffer);
	}
	m_frames.clear();

	safeDestroy(m_lightRenderer);
	safeDestroy(m_toneMapImageProcess);
	safeDestroy(m_motionBlurPrimeImageProcess);
	safeDestroy(m_motionBlurImageProcess);
	safeDestroy(m_gammaCorrectionImageProcess);
	safeDestroy(m_visualImageProcess);
	safeDestroy(m_antiAlias);
	safeDestroy(m_ambientOcclusion);
	safeDestroy(m_colorTargetCopy);
	safeDestroy(m_shadowMaskProject);

	safeDestroy(m_shadowAtlasTargetSet);
	safeDestroy(m_shadowMaskTargetSet);
	safeDestroy(m_shadowCascadeTargetSet);
	safeDestroy(m_colorTargetSet);
	safeDestroy(m_velocityTargetSet);
	safeDestroy(m_ambientOcclusionTargetSet);
	safeDestroy(m_gbufferTargetSet);
	safeDestroy(m_intermediateTargetSet);
	safeDestroy(m_visualTargetSet);

	m_shadowProjection = nullptr;

	m_irradianceGrid.clear();
}

void WorldRendererDeferred::attach(Entity* entity)
{
	m_rootEntity->addEntity(entity);
}

void WorldRendererDeferred::build(WorldRenderView& worldRenderView, int32_t frame)
{
	WorldContext wc(
		m_entityRenderers,
		m_frames[frame].renderContext,
		m_rootEntity
	);

	// Ensure no lights in view.
	worldRenderView.resetLights();

	// Reset render context by flushing it.
	wc.getRenderContext()->flush();

	// \tbd Flush all entity renderers first, only used by probes atm and need to render to targets.
	// Until we have RenderGraph properly implemented we need to make sure
	// rendering probes doesn't nest render passes.
	wc.flush();
	wc.getRenderContext()->merge(render::RpAll);

	buildGBuffer(worldRenderView, frame);
	buildVelocity(worldRenderView, frame);
	buildAmbientOcclusion(worldRenderView, frame);
	buildLights(worldRenderView, frame);
	buildReflections(worldRenderView, frame);
	buildVisual(worldRenderView, frame);
	buildCopyFrame(worldRenderView, frame);
	buildEndFrame(worldRenderView, frame);

	m_rootEntity->removeAllEntities();
}

void WorldRendererDeferred::render(render::IRenderView* renderView, int32_t frame)
{
	m_frames[frame].renderContext->render(renderView);
}

render::ImageProcess* WorldRendererDeferred::getVisualImageProcess()
{
	return m_visualImageProcess;
}

void WorldRendererDeferred::getDebugTargets(std::vector< render::DebugTarget >& outTargets) const
{
	if (m_visualTargetSet)
		outTargets.push_back(render::DebugTarget(L"Visual", render::DtvDefault, m_visualTargetSet->getColorTexture(0)));

	if (m_intermediateTargetSet)
		outTargets.push_back(render::DebugTarget(L"Intermediate", render::DtvDefault, m_intermediateTargetSet->getColorTexture(0)));

	if (m_gbufferTargetSet)
	{
		outTargets.push_back(render::DebugTarget(L"GBuffer depth", render::DtvViewDepth, m_gbufferTargetSet->getColorTexture(0)));
		outTargets.push_back(render::DebugTarget(L"GBuffer normals", render::DtvNormals, m_gbufferTargetSet->getColorTexture(1)));
		outTargets.push_back(render::DebugTarget(L"GBuffer metalness", render::DtvDeferredMetalness, m_gbufferTargetSet->getColorTexture(2)));
		outTargets.push_back(render::DebugTarget(L"GBuffer roughness", render::DtvDeferredRoughness, m_gbufferTargetSet->getColorTexture(2)));
		outTargets.push_back(render::DebugTarget(L"GBuffer specular", render::DtvDeferredSpecular, m_gbufferTargetSet->getColorTexture(2)));
		outTargets.push_back(render::DebugTarget(L"GBuffer surface color", render::DtvDefault, m_gbufferTargetSet->getColorTexture(3)));
	}

	if (m_ambientOcclusionTargetSet)
		outTargets.push_back(render::DebugTarget(L"Ambient occlusion", render::DtvDefault, m_ambientOcclusionTargetSet->getColorTexture(0)));

	if (m_velocityTargetSet)
		outTargets.push_back(render::DebugTarget(L"Velocity", render::DtvVelocity, m_velocityTargetSet->getColorTexture(0)));

	if (m_colorTargetSet)
		outTargets.push_back(render::DebugTarget(L"Color read-back copy", render::DtvDefault, m_colorTargetSet->getColorTexture(0)));

	if (m_reflectionsTargetSet)
		outTargets.push_back(render::DebugTarget(L"Reflections", render::DtvDefault, m_reflectionsTargetSet->getColorTexture(0)));

	if (m_shadowCascadeTargetSet)
		outTargets.push_back(render::DebugTarget(L"Shadow map (cascade)", render::DtvShadowMap, m_shadowCascadeTargetSet->getDepthTexture()));

	if (m_shadowMaskTargetSet)
		outTargets.push_back(render::DebugTarget(L"Shadow mask (cascade)", render::DtvDefault, m_shadowMaskTargetSet->getColorTexture(0)));

	if (m_shadowAtlasTargetSet)
		outTargets.push_back(render::DebugTarget(L"Shadow map (atlas)", render::DtvShadowMap, m_shadowAtlasTargetSet->getDepthTexture()));

	if (m_colorTargetCopy)
		m_colorTargetCopy->getDebugTargets(outTargets);

	if (m_ambientOcclusion)
		m_ambientOcclusion->getDebugTargets(outTargets);

	if (m_antiAlias)
		m_antiAlias->getDebugTargets(outTargets);

	if (m_visualImageProcess)
		m_visualImageProcess->getDebugTargets(outTargets);

	if (m_gammaCorrectionImageProcess)
		m_gammaCorrectionImageProcess->getDebugTargets(outTargets);

	if (m_motionBlurPrimeImageProcess)
		m_motionBlurPrimeImageProcess->getDebugTargets(outTargets);

	if (m_motionBlurImageProcess)
		m_motionBlurImageProcess->getDebugTargets(outTargets);

	if (m_toneMapImageProcess)
		m_toneMapImageProcess->getDebugTargets(outTargets);
}

void WorldRendererDeferred::buildGBuffer(WorldRenderView& worldRenderView, int32_t frame)
{
	WorldContext wc(
		m_entityRenderers,
		m_frames[frame].renderContext,
		m_rootEntity
	);

	const float clearZ = m_settings.viewFarZ;

	auto tb = wc.getRenderContext()->alloc< render::TargetBeginRenderBlock >("World GBuffer; begin");
	tb->renderTargetSet = m_gbufferTargetSet;
	tb->renderTargetIndex = -1;
	tb->clear.mask = render::CfColor | render::CfDepth;
	tb->clear.colors[0] = Color4f(clearZ, clearZ, clearZ, clearZ);	// depth
	tb->clear.colors[1] = Color4f(0.0f, 0.0f, 1.0f, 0.0f);	// normal
	tb->clear.colors[2] = Color4f(0.0f, 1.0f, 0.0f, 1.0f);	// misc
	tb->clear.colors[3] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);	// surface
	tb->clear.depth = 1.0f;
	wc.getRenderContext()->enqueue(tb);

	auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
	sharedParams->beginParameters(wc.getRenderContext());
	sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
	sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
	sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
	sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
	sharedParams->endParameters(wc.getRenderContext());

	WorldRenderPassDeferred gbufferPass(
		s_techniqueDeferredGBufferWrite,
		sharedParams,
		worldRenderView,
		IWorldRenderPass::PfFirst,
		false
	);

	T_ASSERT(!wc.getRenderContext()->havePendingDraws());
	wc.build(worldRenderView, gbufferPass, m_rootEntity);
	wc.flush(worldRenderView, gbufferPass);
	wc.getRenderContext()->merge(render::RpAll);

	auto te = wc.getRenderContext()->alloc< render::TargetEndRenderBlock >("World GBuffer; end");
	wc.getRenderContext()->enqueue(te);
}

void WorldRendererDeferred::buildVelocity(const WorldRenderView& worldRenderView, int32_t frame)
{
	WorldContext wc(
		m_entityRenderers,
		m_frames[frame].renderContext,
		m_rootEntity
	);

	if (m_motionBlurQuality == QuDisabled)
		return;

	auto tb = wc.getRenderContext()->alloc< render::TargetBeginRenderBlock >("World velocity; begin");
	tb->renderTargetSet = m_velocityTargetSet;
	tb->renderTargetIndex = -1;
	tb->clear.mask = render::CfColor;
	tb->clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	wc.getRenderContext()->enqueue(tb);

	if (m_motionBlurPrimeImageProcess)
	{
		render::ImageProcessStep::Instance::RenderParams params;
		params.viewFrustum = worldRenderView.getViewFrustum();
		params.lastView = worldRenderView.getLastView();
		params.view = worldRenderView.getView();
		params.projection = worldRenderView.getProjection();
		params.deltaTime = 0.0f;

		auto lrb = wc.getRenderContext()->alloc< render::LambdaRenderBlock >("World velocity; prime");
		lrb->lambda = [&, params](render::IRenderView* renderView)
		{
			m_motionBlurPrimeImageProcess->render(
				renderView,
				nullptr,	// color
				m_gbufferTargetSet->getColorTexture(0),	// depth
				nullptr,	// normal
				nullptr,	// velocity
				nullptr,	// shadow mask
				params
			);
		};
		wc.getRenderContext()->enqueue(lrb);
	}

	WorldRenderView velocityRenderView = worldRenderView;
	velocityRenderView.resetLights();

	auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
	sharedParams->beginParameters(wc.getRenderContext());
	sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
	sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
	sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
	sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
	sharedParams->endParameters(wc.getRenderContext());

	WorldRenderPassDeferred velocityPass(
		s_techniqueVelocityWrite,
		sharedParams,
		velocityRenderView,
		IWorldRenderPass::PfNone,
		false
	);

	T_ASSERT(!wc.getRenderContext()->havePendingDraws());
	wc.build(velocityRenderView, velocityPass, m_rootEntity);
	wc.flush(velocityRenderView, velocityPass);
	wc.getRenderContext()->merge(render::RpAll);

	auto te = wc.getRenderContext()->alloc< render::TargetEndRenderBlock >("World velocity; end");
	wc.getRenderContext()->enqueue(te);
}

void WorldRendererDeferred::buildAmbientOcclusion(WorldRenderView& worldRenderView, int32_t frame)
{
	WorldContext wc(
		m_entityRenderers,
		m_frames[frame].renderContext,
		m_rootEntity
	);

	auto tb = wc.getRenderContext()->alloc< render::TargetBeginRenderBlock >("World ambient occlusion; begin");
	tb->renderTargetSet = m_ambientOcclusionTargetSet;
	tb->renderTargetIndex = -1;
	tb->clear.mask = render::CfColor;
	tb->clear.colors[0] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	wc.getRenderContext()->enqueue(tb);

	if (m_ambientOcclusion)
	{
		render::ImageProcessStep::Instance::RenderParams params;
		params.viewFrustum = worldRenderView.getViewFrustum();
		params.view = worldRenderView.getView();
		params.projection = worldRenderView.getProjection();
		params.deltaTime = 0.0f;

		auto lrb = wc.getRenderContext()->alloc< render::LambdaRenderBlock >("World ambient occlusion; process");
		lrb->lambda = [&, params](render::IRenderView* renderView)
		{
			m_ambientOcclusion->render(
				renderView,
				nullptr,	// color
				m_gbufferTargetSet->getColorTexture(0),	// depth
				m_gbufferTargetSet->getColorTexture(1),	// normal
				nullptr,	// velocity
				nullptr,	// shadow mask
				params
			);
		};
		wc.getRenderContext()->enqueue(lrb);
	}

	auto te = wc.getRenderContext()->alloc< render::TargetEndRenderBlock >("World ambient occlusion; end");
	wc.getRenderContext()->enqueue(te);
}

void WorldRendererDeferred::buildLights(const WorldRenderView& worldRenderView, int32_t frame)
{
	WorldContext wc(
		m_entityRenderers,
		m_frames[frame].renderContext,
		m_rootEntity
	);

	Matrix44 view = worldRenderView.getView();
	Matrix44 viewInverse = view.inverse();
	Frustum viewFrustum = worldRenderView.getViewFrustum();

	LightShaderData* lightShaderData = (LightShaderData*)m_frames[frame].lightSBuffer->lock();
	T_FATAL_ASSERT(lightShaderData != nullptr);

	TileShaderData* tileShaderData = (TileShaderData*)m_frames[frame].tileSBuffer->lock();
	T_FATAL_ASSERT(tileShaderData != nullptr);

	bool castShadow = bool(m_shadowsQuality > QuDisabled);
	bool haveCascade = false;
	int32_t resolution = castShadow ? m_shadowCascadeTargetSet->getWidth() : 0;
	int32_t atlasIndex = 0;

	m_frames[frame].lightCount = worldRenderView.getLightCount();

	for (int32_t i = 0; i < worldRenderView.getLightCount(); ++i)
	{
		const Light& light = worldRenderView.getLight(i);

		// Write shared information about light into sbuffer.
		lightShaderData->typeRangeRadius[0] = (float)light.type;
		lightShaderData->typeRangeRadius[1] = light.range;
		lightShaderData->typeRangeRadius[2] = light.radius / 2.0f;
		lightShaderData->typeRangeRadius[3] = 0.0f;
		
		(view * light.position.xyz1()).storeUnaligned(lightShaderData->position);
		(view * light.direction.xyz0()).storeUnaligned(lightShaderData->direction);
		light.color.storeUnaligned(lightShaderData->color);

		// Prepare shadows for each light, add information about shadow map into sbuffer.
		if (
			castShadow &&
			!haveCascade &&
			light.castShadow &&
			light.type == LtDirectional
		)
		{
			Matrix44 shadowLightView;
			Matrix44 shadowLightProjection;

			for (int32_t slice = 0; slice < m_shadowSettings.cascadingSlices; ++slice)
			{
				Scalar zn(max(m_slicePositions[slice], m_settings.viewNearZ));
				Scalar zf(min(m_slicePositions[slice + 1], m_shadowSettings.farZ));

				// Render shadow map for current slice.
				{
					auto tb = wc.getRenderContext()->alloc< render::TargetBeginRenderBlock >("World cascade shadows; begin");
					tb->renderTargetSet = m_shadowCascadeTargetSet;
					tb->renderTargetIndex = -1;
					tb->clear.mask = render::CfDepth;
					tb->clear.depth = 1.0f;
					wc.getRenderContext()->enqueue(tb);

					// Create sliced view frustum.
					Frustum sliceViewFrustum = viewFrustum;
					sliceViewFrustum.setNearZ(zn);
					sliceViewFrustum.setFarZ(zf);

					// Calculate shadow map projection.
					Frustum shadowFrustum;
					m_shadowProjection->calculate(
						viewInverse,
						light.position,
						light.direction,
						sliceViewFrustum,
						m_shadowSettings.farZ,
						m_shadowSettings.quantizeProjection,
						shadowLightView,
						shadowLightProjection,
						shadowFrustum
					);

					// Render shadow map.
					WorldRenderView shadowRenderView;
					shadowRenderView.resetLights();
					shadowRenderView.setProjection(shadowLightProjection);
					shadowRenderView.setView(shadowLightView, shadowLightView);
					shadowRenderView.setViewFrustum(shadowFrustum);
					shadowRenderView.setCullFrustum(shadowFrustum);
					shadowRenderView.setTimes(
						worldRenderView.getTime(),
						worldRenderView.getDeltaTime(),
						worldRenderView.getInterval()
					);

					auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
					sharedParams->beginParameters(wc.getRenderContext());
					sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
					sharedParams->setMatrixParameter(s_handleView, shadowLightView);
					sharedParams->setMatrixParameter(s_handleViewInverse, shadowLightView.inverse());
					sharedParams->setMatrixParameter(s_handleProjection, shadowLightProjection);
					sharedParams->endParameters(wc.getRenderContext());

					WorldRenderPassDeferred shadowPass(
						s_techniqueShadow,
						sharedParams,
						shadowRenderView,
						IWorldRenderPass::PfNone,
						false
					);

					// Set viewport to current cascade.
					auto svrb = wc.getRenderContext()->alloc< render::SetViewportRenderBlock >("World cascade shadows; set viewport");
					svrb->viewport = render::Viewport(
						0,
						0,
						resolution,
						resolution,
						0.0f,
						1.0f
					);
					wc.getRenderContext()->enqueue(svrb);	

					T_ASSERT(!wc.getRenderContext()->havePendingDraws());
					wc.build(shadowRenderView, shadowPass, m_rootEntity);
					wc.flush(shadowRenderView, shadowPass);
					wc.getRenderContext()->merge(render::RpAll);

					auto te = wc.getRenderContext()->alloc< render::TargetEndRenderBlock >("World cascade shadows; end");
					wc.getRenderContext()->enqueue(te);
				}

				// Project slice shadow map onto screen space shadow mask.
				{
					auto tb = wc.getRenderContext()->alloc< render::TargetBeginRenderBlock >("World cascade shadow mask; begin");
					tb->renderTargetSet = m_shadowMaskTargetSet;
					tb->renderTargetIndex = -1;

					if (slice == 0)
					{
						tb->clear.mask = render::CfColor;
						tb->clear.colors[0] = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
					}
					else
						tb->clear.mask = 0;

					wc.getRenderContext()->enqueue(tb);

					render::ImageProcessStep::Instance::RenderParams params;
					params.viewFrustum = worldRenderView.getViewFrustum();
					params.viewToLight = shadowLightProjection * shadowLightView * viewInverse;
					params.projection = worldRenderView.getProjection();
					params.sliceCount = m_shadowSettings.cascadingSlices;
					params.sliceIndex = slice;
					params.sliceNearZ = zn;
					params.sliceFarZ = zf;
					params.shadowFarZ = m_shadowSettings.farZ;
					params.shadowMapBias = m_shadowSettings.bias + slice * m_shadowSettings.biasCoeff;
					params.deltaTime = 0.0f;

					auto lrb = wc.getRenderContext()->alloc< render::LambdaRenderBlock >("World cascade shadow mask; process");
					lrb->lambda = [&, params](render::IRenderView* renderView)
					{
						m_shadowMaskProject->render(
							renderView,
							m_shadowCascadeTargetSet->getDepthTexture(),	// color
							m_gbufferTargetSet->getColorTexture(0),	// depth
							m_gbufferTargetSet->getColorTexture(1),	// normal
							nullptr,	// velocity
							nullptr,	// shadow mask
							params
						);
					};
					wc.getRenderContext()->enqueue(lrb);

					auto te = wc.getRenderContext()->alloc< render::TargetEndRenderBlock >("World cascade shadow mask; end");
					wc.getRenderContext()->enqueue(te);
				}
			}

			haveCascade = true;
		}
		else if (castShadow && light.castShadow && light.type == LtPoint)
		{
			// \tbd
		}
		else if (castShadow && light.castShadow && light.type == LtSpot)
		{
			Matrix44 shadowLightView;
			Matrix44 shadowLightProjection;
			Frustum shadowFrustum;

			shadowFrustum.buildPerspective(
				light.radius,
				1.0f,
				0.1f,
				light.range
			);

			shadowLightProjection = perspectiveLh(light.radius, 1.0f, 0.1f, light.range);

			Vector4 lightAxisX, lightAxisY, lightAxisZ;
			lightAxisZ = -light.direction.xyz0().normalized();
			lightAxisX = cross(viewInverse.axisZ(), lightAxisZ).normalized();
			lightAxisY = cross(lightAxisX, lightAxisZ).normalized();

			shadowLightView = Matrix44(
				lightAxisX,
				lightAxisY,
				lightAxisZ,
				light.position.xyz1()
			);
			shadowLightView = shadowLightView.inverse();

			WorldRenderView shadowRenderView;
			shadowRenderView.resetLights();
			shadowRenderView.setProjection(shadowLightProjection);
			shadowRenderView.setView(shadowLightView, shadowLightView);
			shadowRenderView.setViewFrustum(shadowFrustum);
			shadowRenderView.setCullFrustum(shadowFrustum);
			shadowRenderView.setTimes(
				worldRenderView.getTime(),
				worldRenderView.getDeltaTime(),
				worldRenderView.getInterval()
			);

			auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
			sharedParams->beginParameters(wc.getRenderContext());
			sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
			sharedParams->setMatrixParameter(s_handleView, shadowLightView);
			sharedParams->setMatrixParameter(s_handleViewInverse, shadowLightView.inverse());
			sharedParams->setMatrixParameter(s_handleProjection, shadowLightProjection);
			sharedParams->endParameters(wc.getRenderContext());

			WorldRenderPassDeferred shadowPass(
				s_techniqueShadow,
				sharedParams,
				shadowRenderView,
				IWorldRenderPass::PfNone,
				false
			);

			// Bind shadow atlas map.
			auto tb = wc.getRenderContext()->alloc< render::TargetBeginRenderBlock >("World atlas shadows; begin");
			tb->renderTargetSet = m_shadowAtlasTargetSet;
			tb->renderTargetIndex = -1;
			tb->clear.mask = render::CfDepth;
			tb->clear.depth = 1.0f;
			wc.getRenderContext()->enqueue(tb);

			// Set viewport to light atlas slot.
			auto svrb = wc.getRenderContext()->alloc< render::SetViewportRenderBlock >("World atlas shadows; set viewport");
			svrb->viewport = render::Viewport(
				(atlasIndex & 3) * 1024,
				(atlasIndex / 4) * 1024,
				1024,
				1024,
				0.0f,
				1.0f
			);
			wc.getRenderContext()->enqueue(svrb);	

			T_ASSERT(!wc.getRenderContext()->havePendingDraws());
			wc.build(shadowRenderView, shadowPass, m_rootEntity);
			wc.flush(shadowRenderView, shadowPass);
			wc.getRenderContext()->merge(render::RpAll);

			auto te = wc.getRenderContext()->alloc< render::TargetEndRenderBlock >("World atlas shadows; end");
			wc.getRenderContext()->enqueue(te);

			// Write transposed matrix to shaders as shaders have row-major order.
			Matrix44 viewToLightSpace = shadowLightProjection * shadowLightView * viewInverse;
			Matrix44 vls = viewToLightSpace.transpose();
			vls.axisX().storeUnaligned(lightShaderData->viewToLight0);
			vls.axisY().storeUnaligned(lightShaderData->viewToLight1);
			vls.axisZ().storeUnaligned(lightShaderData->viewToLight2);
			vls.translation().storeUnaligned(lightShaderData->viewToLight3);

			// Write atlas coordinates to shaders.
			Vector4(
				(atlasIndex & 3) / 4.0f,
				(atlasIndex / 4) / 4.0f,
				1.0f / 4.0f,
				1.0f / 4.0f
			).storeUnaligned(lightShaderData->atlasTransform);

			++atlasIndex;
		}

		++lightShaderData;
	}

	// Update tile data.
	{
		const float dx = 1.0f / 16.0f;
		const float dy = 1.0f / 16.0f;

		Vector4 nh = viewFrustum.corners[1] - viewFrustum.corners[0];
		Vector4 nv = viewFrustum.corners[3] - viewFrustum.corners[0];
		Vector4 fh = viewFrustum.corners[5] - viewFrustum.corners[4];
		Vector4 fv = viewFrustum.corners[7] - viewFrustum.corners[4];

		Frustum tileFrustum;
		for (int32_t y = 0; y < 16; ++y)
		{
			float fy = float(y) * dy;
			for (int32_t x = 0; x < 16; ++x)
			{
				float fx = float(x) * dx;

				Vector4 corners[] =
				{
					// Near
					viewFrustum.corners[0] + nh * Scalar(fx) + nv * Scalar(fy),				// l t
					viewFrustum.corners[0] + nh * Scalar(fx + dx) + nv * Scalar(fy),		// r t
					viewFrustum.corners[0] + nh * Scalar(fx + dx) + nv * Scalar(fy + dy),	// r b
					viewFrustum.corners[0] + nh * Scalar(fx) + nv * Scalar(fy + dy),		// l b
					// Far
					viewFrustum.corners[4] + fh * Scalar(fx) + fv * Scalar(fy),				// l t
					viewFrustum.corners[4] + fh * Scalar(fx + dx) + fv * Scalar(fy),		// r t
					viewFrustum.corners[4] + fh * Scalar(fx + dx) + fv * Scalar(fy + dy),	// r b
					viewFrustum.corners[4] + fh * Scalar(fx) + fv * Scalar(fy + dy)			// l b
				};

				tileFrustum.buildFromCorners(corners);

				int32_t count = 0;
				for (uint32_t i = 0; i <  worldRenderView.getLightCount(); ++i)
				{
					const Light& light = worldRenderView.getLight(i);

					if (light.type == LtDirectional)
					{
						tileShaderData[x + y * 16].lights[count++] = float(i);
					}
					else if (light.type == LtPoint)
					{
						Vector4 lvp = worldRenderView.getView() * light.position.xyz1();
						if (tileFrustum.inside(lvp, Scalar(light.range)) != Frustum::IrOutside)
							tileShaderData[x + y * 16].lights[count++] = float(i);
					}
					else if (light.type == LtSpot)
					{
						tileShaderData[x + y * 16].lights[count++] = float(i);
					}

					if (count >= 4)
						break;
				}
				tileShaderData[x + y * 16].lightCount[0] = float(count);
			}
		}
	}

	m_frames[frame].tileSBuffer->unlock();
	m_frames[frame].lightSBuffer->unlock();
}

void WorldRendererDeferred::buildReflections(const WorldRenderView& worldRenderView, int32_t frame)
{
	if (m_reflectionsQuality == QuDisabled)
		return;

	WorldContext wc(
		m_entityRenderers,
		m_frames[frame].renderContext,
		m_rootEntity
	);

	auto tb = wc.getRenderContext()->alloc< render::TargetBeginRenderBlock >("World reflections; begin");
	tb->renderTargetSet = m_reflectionsTargetSet;
	tb->renderTargetIndex = -1;
	tb->clear.mask = render::CfColor;
	tb->clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	wc.getRenderContext()->enqueue(tb);

	WorldRenderView reflectionsRenderView = worldRenderView;
	reflectionsRenderView.resetLights();

	auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
	sharedParams->beginParameters(wc.getRenderContext());
	sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
	sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
	sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
	sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
	sharedParams->setTextureParameter(s_handleDepthMap, m_gbufferTargetSet->getColorTexture(0));
	sharedParams->setTextureParameter(s_handleNormalMap, m_gbufferTargetSet->getColorTexture(1));
	sharedParams->setTextureParameter(s_handleMiscMap, m_gbufferTargetSet->getColorTexture(2));
	sharedParams->setTextureParameter(s_handleColorMap, m_gbufferTargetSet->getColorTexture(3));
	sharedParams->endParameters(wc.getRenderContext());

	WorldRenderPassDeferred reflectionsPass(
		s_techniqueReflectionWrite,
		sharedParams,
		reflectionsRenderView,
		IWorldRenderPass::PfNone,
		false
	);

	T_ASSERT(!wc.getRenderContext()->havePendingDraws());
	wc.build(reflectionsRenderView, reflectionsPass, m_rootEntity);
	wc.flush(reflectionsRenderView, reflectionsPass);
	wc.getRenderContext()->merge(render::RpAll);

	// Render screenspace reflections.
	if (m_reflectionsQuality >= QuHigh)
	{
		m_lightRenderer->renderReflections(
			wc.getRenderContext(),
			worldRenderView.getProjection(),
			worldRenderView.getView(),
			worldRenderView.getLastView(),
			m_colorTargetSet->getColorTexture(0),	// \tbd using last frame copy without reprojection...
			m_gbufferTargetSet->getColorTexture(0),	// depth
			m_gbufferTargetSet->getColorTexture(1),	// normals
			m_gbufferTargetSet->getColorTexture(2)	// metalness, roughness and specular
		);
	}

	auto te = wc.getRenderContext()->alloc< render::TargetEndRenderBlock >("World reflections; end");
	wc.getRenderContext()->enqueue(te);
}

void WorldRendererDeferred::buildVisual(const WorldRenderView& worldRenderView, int32_t frame)
{
	WorldContext wc(
		m_entityRenderers,
		m_frames[frame].renderContext,
		m_rootEntity
	);

	auto tb = wc.getRenderContext()->alloc< render::TargetBeginRenderBlock >("World visual; begin");
	tb->renderTargetSet = m_visualTargetSet;
	tb->renderTargetIndex = -1;
	tb->clear.mask = render::CfColor;
	tb->clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	wc.getRenderContext()->enqueue(tb);

	// Irradiance
	{
		WorldRenderView irradianceRenderView = worldRenderView;
		irradianceRenderView.resetLights();

		auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
		sharedParams->beginParameters(wc.getRenderContext());

		sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
		sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
		sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
		sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
		sharedParams->setTextureParameter(s_handleDepthMap, m_gbufferTargetSet->getColorTexture(0));
		sharedParams->setTextureParameter(s_handleNormalMap, m_gbufferTargetSet->getColorTexture(1));
		sharedParams->setTextureParameter(s_handleMiscMap, m_gbufferTargetSet->getColorTexture(2));
		sharedParams->setTextureParameter(s_handleColorMap, m_gbufferTargetSet->getColorTexture(3));
		sharedParams->setTextureParameter(s_handleOcclusionMap, m_ambientOcclusionTargetSet->getColorTexture(0));

		if (m_irradianceGrid)
		{
			const auto size = m_irradianceGrid->getSize();
			sharedParams->setVectorParameter(s_handleIrradianceGridSize, Vector4((float)size[0], (float)size[1], (float)size[2], 0.0f));
			sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMin, m_irradianceGrid->getBoundingBox().mn);
			sharedParams->setVectorParameter(s_handleIrradianceGridBoundsMax, m_irradianceGrid->getBoundingBox().mx);
			sharedParams->setStructBufferParameter(s_handleIrradianceGridSBuffer, m_irradianceGrid->getBuffer());
		}

		sharedParams->endParameters(wc.getRenderContext());

		WorldRenderPassDeferred irradiancePass(
			s_techniqueIrradianceWrite,
			sharedParams,
			irradianceRenderView,
			IWorldRenderPass::PfNone,
			(bool)m_irradianceGrid
		);

		T_ASSERT(!wc.getRenderContext()->havePendingDraws());
		wc.build(irradianceRenderView, irradiancePass, m_rootEntity);
		wc.flush(irradianceRenderView, irradiancePass);
		wc.getRenderContext()->merge(render::RpAll);
	}

	// Analytical lights; resolve with gbuffer.
	{
		m_lightRenderer->renderLights(
			wc.getRenderContext(),
			worldRenderView.getTime(),
			m_frames[frame].lightCount,
			worldRenderView.getProjection(),
			worldRenderView.getView(),
			m_frames[frame].lightSBuffer,
			m_frames[frame].tileSBuffer,
			m_irradianceGrid,
			m_gbufferTargetSet->getColorTexture(0),	// depth
			m_gbufferTargetSet->getColorTexture(1),	// normals
			m_gbufferTargetSet->getColorTexture(2),	// metalness/roughness
			m_gbufferTargetSet->getColorTexture(3),	// surface color
			m_ambientOcclusionTargetSet->getColorTexture(0),	// ambient occlusion
			m_shadowMaskTargetSet != nullptr ? m_shadowMaskTargetSet->getColorTexture(0) : nullptr,		// shadow mask
			m_shadowAtlasTargetSet != nullptr ? m_shadowAtlasTargetSet->getDepthTexture() : nullptr,	// shadow map atlas,
			m_reflectionsTargetSet != nullptr ? m_reflectionsTargetSet->getColorTexture(0) : nullptr	// reflection map
		);
	}

	// Fog
	if (dot4(m_fogDistanceAndDensity, Vector4(0.0f, 0.0f, 1.0f, 1.0f)) > FUZZY_EPSILON)
	{
		m_lightRenderer->renderFog(
			wc.getRenderContext(),
			worldRenderView.getProjection(),
			worldRenderView.getView(),
			m_fogDistanceAndDensity,
			m_fogColor,
			m_gbufferTargetSet->getColorTexture(0),
			m_gbufferTargetSet->getColorTexture(1),
			m_gbufferTargetSet->getColorTexture(2),
			m_gbufferTargetSet->getColorTexture(3)
		);
	}

	// Forward visuals; not included in GBuffer.
	{
		WorldRenderView visualRenderView = worldRenderView;
		visualRenderView.resetLights();

		auto sharedParams = wc.getRenderContext()->alloc< render::ProgramParameters >();
		sharedParams->beginParameters(wc.getRenderContext());
		sharedParams->setFloatParameter(s_handleTime, worldRenderView.getTime());
		sharedParams->setMatrixParameter(s_handleView, worldRenderView.getView());
		sharedParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
		sharedParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
		sharedParams->setFloatParameter(s_handleLightCount, (float)m_frames[frame].lightCount);
		sharedParams->setVectorParameter(s_handleFogDistanceAndDensity, m_fogDistanceAndDensity);
		sharedParams->setVectorParameter(s_handleFogColor, m_fogColor);
		sharedParams->setTextureParameter(s_handleColorMap, m_colorTargetSet->getColorTexture(0));
		sharedParams->setTextureParameter(s_handleDepthMap, m_gbufferTargetSet->getColorTexture(0));
		sharedParams->setTextureParameter(s_handleNormalMap, m_gbufferTargetSet->getColorTexture(1));
		sharedParams->setStructBufferParameter(s_handleLightSBuffer, m_frames[frame].lightSBuffer);
		sharedParams->setStructBufferParameter(s_handleTileSBuffer, m_frames[frame].tileSBuffer);
		sharedParams->endParameters(wc.getRenderContext());

		WorldRenderPassDeferred deferredColorPass(
			s_techniqueDeferredColor,
			sharedParams,
			visualRenderView,
			IWorldRenderPass::PfLast,
			m_settings.fog,
			m_gbufferTargetSet->getColorTexture(0) != nullptr
		);

		T_ASSERT(!wc.getRenderContext()->havePendingDraws());
		wc.build(visualRenderView, deferredColorPass, m_rootEntity);
		wc.flush(visualRenderView, deferredColorPass);
		wc.getRenderContext()->merge(render::RpAll);
	}

	auto te = wc.getRenderContext()->alloc< render::TargetEndRenderBlock >("World visual; end");
	wc.getRenderContext()->enqueue(te);
}

void WorldRendererDeferred::buildCopyFrame(const WorldRenderView& worldRenderView, int32_t frame)
{
	WorldContext wc(
		m_entityRenderers,
		m_frames[frame].renderContext,
		m_rootEntity
	);

	auto tb = wc.getRenderContext()->alloc< render::TargetBeginRenderBlock >("World copy-frame; begin");
	tb->renderTargetSet = m_colorTargetSet;
	tb->renderTargetIndex = -1;
	tb->clear.mask = 0;
	wc.getRenderContext()->enqueue(tb);

	render::ImageProcessStep::Instance::RenderParams params;
	params.viewFrustum = worldRenderView.getViewFrustum();
	params.projection = worldRenderView.getProjection();
	params.deltaTime = 0.0f;

	auto lrb = wc.getRenderContext()->alloc< render::LambdaRenderBlock >();
	lrb->lambda = [&, params](render::IRenderView* renderView)
	{
		m_colorTargetCopy->render(
	 		renderView,
	 		m_visualTargetSet->getColorTexture(0),	// color
	 		nullptr,	// depth
	 		nullptr,	// normal
	 		nullptr,	// velocity
	 		nullptr,	// shadow mask
	 		params
		);
	};
	wc.getRenderContext()->enqueue(lrb);

	auto te = wc.getRenderContext()->alloc< render::TargetEndRenderBlock >("World copy-frame; end");
	wc.getRenderContext()->enqueue(te);
}

void WorldRendererDeferred::buildEndFrame(WorldRenderView& worldRenderView, int32_t frame)
{
	WorldContext wc(
		m_entityRenderers,
		m_frames[frame].renderContext,
		m_rootEntity
	);

	if (!m_visualTargetSet)
		return;

	render::ImageProcessStep::Instance::RenderParams params;
	params.viewFrustum = worldRenderView.getViewFrustum();
	params.viewToLight = Matrix44::identity(); //f.viewToLightSpace;
	params.view = worldRenderView.getView();
	params.projection = worldRenderView.getProjection();
	params.deltaTime = 1.0f / 60.0f; // deltaTime;

	render::IRenderTargetSet* sourceTargetSet = m_visualTargetSet;
	render::IRenderTargetSet* outputTargetSet = m_intermediateTargetSet;
	T_ASSERT(sourceTargetSet);

	StaticVector< render::ImageProcess*, 5 > processes;
	if (m_motionBlurImageProcess)
		processes.push_back(m_motionBlurImageProcess);
	if (m_toneMapImageProcess)
		processes.push_back(m_toneMapImageProcess);
	if (m_visualImageProcess)
		processes.push_back(m_visualImageProcess);
	if (m_gammaCorrectionImageProcess)
		processes.push_back(m_gammaCorrectionImageProcess);
	if (m_antiAlias)
		processes.push_back(m_antiAlias);

	for (size_t i = 0; i < processes.size(); ++i)
	{
		render::ImageProcess* process = processes[i];

		bool haveNext = bool((i + 1) < processes.size());
		if (haveNext)
		{
			auto tb = wc.getRenderContext()->alloc< render::TargetBeginRenderBlock >("World end-frame; begin");
			tb->renderTargetSet = outputTargetSet;
			tb->renderTargetIndex = -1;
			tb->clear.mask = 0;
			wc.getRenderContext()->enqueue(tb);
		}

		auto lrb = wc.getRenderContext()->alloc< render::LambdaRenderBlock >("World end-frame; process");
		lrb->lambda = [&, process, sourceTargetSet, params](render::IRenderView* renderView)
		{
			process->render(
				renderView,
				sourceTargetSet->getColorTexture(0),	// color
				m_gbufferTargetSet->getColorTexture(0),	// depth
				m_gbufferTargetSet->getColorTexture(1),	// normal
				m_velocityTargetSet ? m_velocityTargetSet->getColorTexture(0) : nullptr,	// velocity
				nullptr,	// shadow mask
				params
			);
		};
		wc.getRenderContext()->enqueue(lrb);

		if (haveNext)
		{
			auto te = wc.getRenderContext()->alloc< render::TargetEndRenderBlock >("World end-frame; end");
			wc.getRenderContext()->enqueue(te);

			std::swap(sourceTargetSet, outputTargetSet);
		}
	}
}

	}
}
