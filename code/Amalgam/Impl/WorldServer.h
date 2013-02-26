#ifndef traktor_amalgam_WorldServer_H
#define traktor_amalgam_WorldServer_H

#include "Amalgam/IWorldServer.h"
#include "World/WorldRenderSettings.h"

namespace traktor
{

class PropertyGroup;

	namespace amalgam
	{

class IEnvironment;
class IRenderServer;
class IResourceServer;

class WorldServer : public IWorldServer
{
	T_RTTI_CLASS;

public:
	WorldServer();

	bool create(const PropertyGroup* settings, IRenderServer* renderServer, IResourceServer* resourceServer);

	void destroy();

	void createResourceFactories(IEnvironment* environment);

	void createEntityFactories(IEnvironment* environment);

	int32_t reconfigure(const PropertyGroup* settings);

	virtual void addEntityFactory(world::IEntityFactory* entityFactory);

	virtual void removeEntityFactory(world::IEntityFactory* entityFactory);

	virtual void addEntityRenderer(world::IEntityRenderer* entityRenderer);

	virtual void removeEntityRenderer(world::IEntityRenderer* entityRenderer);

	virtual const world::IEntityBuilder* getEntityBuilder();

	virtual world::WorldEntityRenderers* getEntityRenderers();

	virtual world::IEntityEventManager* getEntityEventManager();

	virtual Ref< world::IWorldRenderer > createWorldRenderer(
		const world::WorldRenderSettings* worldRenderSettings,
		const world::PostProcessSettings* postProcessSettings
	);

	virtual int32_t getFrameCount() const;

private:
	Ref< world::IEntityBuilder > m_entityBuilder;
	Ref< world::WorldEntityRenderers > m_entityRenderers;
	Ref< world::IEntityEventManager > m_eventManager;
	Ref< IRenderServer > m_renderServer;
	Ref< IResourceServer > m_resourceServer;
	const TypeInfo* m_worldType;
	world::Quality m_shadowQuality;
	world::Quality m_ambientOcclusionQuality;
	world::Quality m_antiAliasQuality;
};

	}
}

#endif	// traktor_amalgam_WorldServer_H
