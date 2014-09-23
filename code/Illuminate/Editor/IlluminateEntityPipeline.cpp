#include <cstring>
#include <ctime>
#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Core/Math/Triangle.h"
#include "Core/Math/Triangulator.h"
#include "Core/Math/Winding3.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Database/Database.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Drawing/Filters/DilateFilter.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Illuminate/Editor/IlluminateEntityData.h"
#include "Illuminate/Editor/IlluminateEntityPipeline.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Operations/UnwrapUV.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "World/Entity/DirectionalLightEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/PointLightEntityData.h"

namespace traktor
{
	namespace illuminate
	{
		namespace
		{

const Scalar c_traceRayOffset(0.025f);
const int32_t c_outputWidth = 2048;
const int32_t c_outputHeight = 2048;
const int32_t c_jobTileWidth = 128;
const int32_t c_jobTileHeight = 128;

struct GBuffer
{
	Vector4 position;
	Vector4 normal;
};

struct Surface
{
	int32_t count;
	Vector4 points[16];
	Vector2 texCoords[16];
	Vector4 normals[16];
	Vector4 normal;
	Color4f color;
	Scalar emissive;

	Surface()
	:	count(0)
	,	emissive(0.0f)
	{
	}
};

struct Light
{
	int32_t type;		//<! 0 - directional, 1 - point, 2 - emissive surface
	Vector4 position;
	Vector4 direction;
	Color4f sunColor;
	Color4f baseColor;
	Color4f shadowColor;
	Scalar range;
	int32_t surface;

	Light()
	:	type(0)
	,	surface(0)
	{
	}
};

template < typename Visitor >
void line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Visitor& visitor)
{
	const bool steep = (traktor::abs< int32_t >(y2 - y1) > traktor::abs< int32_t >(x2 - x1));
	if (steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}

	const bool inverted = (x1 > x2);
	if (inverted)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	const int32_t dx = x2 - x1;
	const int32_t dy = traktor::abs< int32_t >(y2 - y1);

	float error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	
	int y = y1;
	for(int x = x1; x < x2; x++)
	{
		float f = float(x - x1) / (x2 - x1);
		if (!inverted)
			f = 1.0f - f;

		if (steep)
			visitor(y, x, f);
		else
			visitor(x, y, f);

		error -= dy;
		if(error < 0)
		{
			y += ystep;
			error += dx;
		}
	}
}

class GBufferLineVisitor
{
public:
	GBufferLineVisitor(
		const Vector4 P[2],
		const Vector4 N[2],
		AlignedVector< GBuffer >& outGBuffer
	)
	:	m_outGBuffer(outGBuffer)
	{
		for (int i = 0; i < 2; ++i)
		{
			m_P[i] = P[i];
			m_N[i] = N[i];
		}
	}

	void operator () (int32_t x, int32_t y, float alpha)
	{
		if (x >= 1 && y >= 1 && x < c_outputWidth - 1 && y < c_outputHeight - 1)
		{
			Vector4 position = (m_P[0] * Scalar(alpha) + m_P[1] * Scalar(1.0f - alpha)).xyz1();
			Vector4 normal = (m_N[0] * Scalar(alpha) + m_N[1] * Scalar(1.0f - alpha)).xyz0();
			
			for (int32_t dy = -1; dy <= 1; ++dy)
			{
				for (int32_t dx = -1; dx <= 1; ++dx)
				{
					m_outGBuffer[(x + dx) + (y + dy) * c_outputWidth].position = position;
					m_outGBuffer[(x + dx) + (y + dy) * c_outputWidth].normal = normal;
				}
			}
		}
	}

private:
	Vector4 m_P[2];
	Vector4 m_N[2];
	AlignedVector< GBuffer >& m_outGBuffer;
};

class GBufferVisitor
{
public:
	GBufferVisitor(
		const Vector4 P[3],
		const Vector4 N[3],
		AlignedVector< GBuffer >& outGBuffer
	)
	:	m_outGBuffer(outGBuffer)
	{
		for (int i = 0; i < 3; ++i)
		{
			m_P[i] = P[i];
			m_N[i] = N[i];
		}
	}

	void operator () (int32_t x, int32_t y, float alpha, float beta, float gamma)
	{
		if (x >= 0 && y >= 0 && x < c_outputWidth && y < c_outputHeight)
		{
			Vector4 position = (m_P[0] * Scalar(alpha) + m_P[1] * Scalar(beta) + m_P[2] * Scalar(gamma)).xyz1();
			Vector4 normal = (m_N[0] * Scalar(alpha) + m_N[1] * Scalar(beta) + m_N[2] * Scalar(gamma)).xyz0();
			m_outGBuffer[x + y * c_outputWidth].position = position;
			m_outGBuffer[x + y * c_outputWidth].normal = normal;
		}
	}

private:
	Vector4 m_P[3];
	Vector4 m_N[3];
	AlignedVector< GBuffer >& m_outGBuffer;
};




class JobTraceDirect
{
public:
	JobTraceDirect(
		int32_t tileX,
		int32_t tileY,
		const SahTree& sah,
		const AlignedVector< GBuffer >& gbuffer,
		const AlignedVector< Surface >& surfaces,
		const AlignedVector< Light >& lights,
		drawing::Image* outputImageDirect
	)
	:	m_tileX(tileX)
	,	m_tileY(tileY)
	,	m_sah(sah)
	,	m_gbuffer(gbuffer)
	,	m_surfaces(surfaces)
	,	m_lights(lights)
	,	m_outputImageDirect(outputImageDirect)
	{
	}

	void execute()
	{
		RandomGeometry random(std::clock());
		SahTree::QueryCache cache;
		SahTree::QueryResult result;
		Color4f tmp;

		for (int32_t y = m_tileY; y < m_tileY + c_jobTileHeight; ++y)
		{
			for (int32_t x = m_tileX; x < m_tileX + c_jobTileWidth; ++x)
			{
				const Vector4& position = m_gbuffer[x + y * c_outputWidth].position;
				if (position.w() < FUZZY_EPSILON)
					continue;

				const Vector4& normal = m_gbuffer[x + y * c_outputWidth].normal;

				Vector4 rayOrigin = (position + normal * c_traceRayOffset).xyz1();

				Color4f direct(0.0f, 0.0f, 0.0f, 1.0f);

				for (AlignedVector< Light >::const_iterator i = m_lights.begin(); i != m_lights.end(); ++i)
				{
					if (i->type == 0)
					{
						Scalar phi = dot3(-i->direction, normal);
						if (phi > 0.0f)
						{
							if (m_sah.queryAnyIntersection(rayOrigin, -i->direction, 0.0f, cache))
								phi = Scalar(0.0f);

							Scalar k1 = clamp(phi * Scalar(2.0f), Scalar(0.0f), Scalar(1.0f));
							Color4f c1 = i->shadowColor * (Scalar(1.0f) - k1) + i->baseColor * k1;

							Scalar k2 = clamp(phi * Scalar(2.0f) - Scalar(1.0f), Scalar(0.0f), Scalar(1.0f));
							Color4f c2 = c1 * (Scalar(1.0f) - k2) + i->sunColor * k2;

							direct += Color4f(1.0f, 1.0f, 1.0f, 0.0f) * c2;
						}
						else
							direct += Color4f(1.0f, 1.0f, 1.0f, 0.0f) * i->shadowColor;
					}
					else if (i->type == 1)
					{
						Vector4 lightDirection = (i->position - rayOrigin).xyz0();
						Scalar lightDistance = lightDirection.normalize();
						if (lightDistance >= i->range)
							continue;

						Scalar phi = dot3(lightDirection, normal);
						if (phi <= 0.0f)
							continue;

						if (m_sah.queryAnyIntersection(rayOrigin, lightDirection, lightDistance - FUZZY_EPSILON, cache))
							continue;

						Scalar attenuate = Scalar(1.0f) - lightDistance / i->range;
						direct += Color4f(phi, phi, phi, 0.0f) * i->sunColor * attenuate;
					}
					else if (i->type == 2)
					{
						const Surface& emissiveSurface = m_surfaces[i->surface];

						Vector4 u = emissiveSurface.points[2] - emissiveSurface.points[0];
						Vector4 v = emissiveSurface.points[1] - emissiveSurface.points[0];

						const int32_t c_steps = 16;

						Color4f lightAcc(0.0f, 0.0f, 0.0f, 0.0f);
						for (int32_t iv = 0; iv < c_steps; ++iv)
						{
							Scalar fv = Scalar(float(iv) / (c_steps - 1));
							for (int32_t iu = 0; iu < c_steps; ++iu)
							{
								Scalar fu = Scalar(float(iu) / (c_steps - 1));
								Vector4 lightPosition = (emissiveSurface.points[0] + u * fu + v * fv).xyz1();

								Vector4 lightDirection = (lightPosition - rayOrigin).xyz0();
								Scalar lightDistance = lightDirection.normalize();
								if (lightDistance >= i->range)
									continue;

								Scalar phi = dot3(lightDirection, normal);
								if (phi <= 0.0f)
									continue;

								if (m_sah.queryAnyIntersection(rayOrigin, lightDirection, lightDistance - FUZZY_EPSILON, cache))
									continue;

								Scalar attenuate = Scalar(1.0f) - lightDistance / i->range;
								lightAcc += Color4f(phi, phi, phi, 0.0f) * i->sunColor * attenuate;
							}
						}

						direct += lightAcc / Scalar(float(c_steps * c_steps));
					}
				}

				m_outputImageDirect->setPixel(x, y, direct);
			}
		}
	}

private:
	int32_t m_tileX;
	int32_t m_tileY;
	const SahTree& m_sah;
	const AlignedVector< GBuffer >& m_gbuffer;
	const AlignedVector< Surface >& m_surfaces;
	const AlignedVector< Light >& m_lights;
	drawing::Image* m_outputImageDirect;
};



class JobTraceIndirect
{
public:
	JobTraceIndirect(
		int32_t tileX,
		int32_t tileY,
		const SahTree& sah,
		const AlignedVector< GBuffer >& gbuffer,
		const AlignedVector< Surface >& surfaces,
		const drawing::Image* imageDirect,
		drawing::Image* outputImageIndirect,
		int32_t indirectTraceSamples
	)
	:	m_tileX(tileX)
	,	m_tileY(tileY)
	,	m_sah(sah)
	,	m_gbuffer(gbuffer)
	,	m_surfaces(surfaces)
	,	m_imageDirect(imageDirect)
	,	m_outputImageIndirect(outputImageIndirect)
	,	m_indirectTraceSamples(indirectTraceSamples)
	{
	}

	void execute()
	{
		RandomGeometry random(std::clock());
		SahTree::QueryCache cache;
		SahTree::QueryResult result;
		Color4f tmp;

		const Scalar c_maxIndirectDistance(100.0f);

		for (int32_t y = m_tileY; y < m_tileY + c_jobTileHeight; ++y)
		{
			for (int32_t x = m_tileX; x < m_tileX + c_jobTileWidth; ++x)
			{
				const Vector4& position = m_gbuffer[x + y * c_outputWidth].position;
				if (position.w() < FUZZY_EPSILON)
					continue;

				const Vector4& normal = m_gbuffer[x + y * c_outputWidth].normal;

				Vector4 rayOrigin = (position + normal * c_traceRayOffset).xyz1();
				Color4f indirect(0.0f, 0.0f, 0.0f, 0.0f);

				for (int32_t i = 0; i < m_indirectTraceSamples; )
				{
					Vector4 rayDirection = random.nextUnit();
					Scalar phi = dot3(rayDirection, normal);
					if (phi <= 0.2f)
						continue;

					if (m_sah.queryClosestIntersection(rayOrigin, rayDirection, result, cache))
					{
						const Vector4& P = result.position;

						Scalar distance = (P - position).xyz0().length();
						if (distance >= c_maxIndirectDistance)
							continue;

						Scalar attenuate = Scalar(1.0f) - distance / c_maxIndirectDistance;

						const Surface& hitSurface = m_surfaces[result.index];

						const Vector4& A = hitSurface.points[0];
						const Vector4& B = hitSurface.points[1];
						const Vector4& C = hitSurface.points[2];

						const Vector2& texCoord0 = hitSurface.texCoords[0];
						const Vector2& texCoord1 = hitSurface.texCoords[1];
						const Vector2& texCoord2 = hitSurface.texCoords[2];

						Vector4 v0 = C - A;
						Vector4 v1 = B - A;
						Vector4 v2 = P - A;

						Scalar d00 = dot3(v0, v0);
						Scalar d01 = dot3(v0, v1);
						Scalar d02 = dot3(v0, v2);
						Scalar d11 = dot3(v1, v1);
						Scalar d12 = dot3(v1, v2);

						Scalar invDenom = Scalar(1.0f) / (d00 * d11 - d01 * d01);
						Scalar u = (d11 * d02 - d01 * d12) * invDenom;
						Scalar v = (d00 * d12 - d01 * d02) * invDenom;

						Vector2 texCoord = texCoord0 + (texCoord2 - texCoord0) * u + (texCoord1 - texCoord0) * v;

						int32_t lx = int32_t(texCoord.x * m_imageDirect->getWidth());
						int32_t ly = int32_t(texCoord.y * m_imageDirect->getHeight());
						m_imageDirect->getPixel(lx, ly, tmp);

						indirect += tmp * attenuate;
					}

					++i;
				}

				indirect /= Scalar(m_indirectTraceSamples);
				m_outputImageIndirect->setPixel(x, y, indirect);
			}
		}
	}

private:
	int32_t m_tileX;
	int32_t m_tileY;
	const SahTree& m_sah;
	const AlignedVector< GBuffer >& m_gbuffer;
	const AlignedVector< Surface >& m_surfaces;
	const drawing::Image* m_imageDirect;
	drawing::Image* m_outputImageIndirect;
	int32_t m_indirectTraceSamples;
};



Ref< ISerializable > resolveAllExternal(editor::IPipelineCommon* pipeline, const ISerializable* object)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (const world::ExternalEntityData* externalEntityDataRef = dynamic_type_cast< const world::ExternalEntityData* >(objectMember->get()))
		{
			Ref< const ISerializable > externalEntityData = pipeline->getObjectReadOnly(externalEntityDataRef->getEntityData());
			if (!externalEntityData)
			{
				log::error << L"IlluminateEntityPipeline failed; Unable to read external entity." << Endl;
				return 0;
			}

			Ref< world::EntityData > resolvedEntityData = dynamic_type_cast< world::EntityData* >(resolveAllExternal(pipeline, externalEntityData));
			if (!resolvedEntityData)
			{
				log::error << L"IlluminateEntityPipeline failed; Unable to resolve external entity." << Endl;
				return 0;
			}

			resolvedEntityData->setName(externalEntityDataRef->getName());
			resolvedEntityData->setTransform(externalEntityDataRef->getTransform());

			objectMember->set(resolvedEntityData);
		}
		else if (objectMember->get())
		{
			objectMember->set(resolveAllExternal(pipeline, objectMember->get()));
		}
	}

	return reflection->clone();
}

void collectTraceEntities(
	const ISerializable* object,
	RefArray< world::DirectionalLightEntityData >& outDirectionalLightEntityData,
	RefArray< world::PointLightEntityData >& outPointLightEntityData,
	RefArray< mesh::MeshEntityData >& outMeshEntityData
)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (mesh::MeshEntityData* meshEntityData = dynamic_type_cast< mesh::MeshEntityData* >(objectMember->get()))
			outMeshEntityData.push_back(meshEntityData);
		else if (world::DirectionalLightEntityData* directionalLightEntityData = dynamic_type_cast< world::DirectionalLightEntityData* >(objectMember->get()))
			outDirectionalLightEntityData.push_back(directionalLightEntityData);
		else if (world::PointLightEntityData* pointLightEntityData = dynamic_type_cast< world::PointLightEntityData* >(objectMember->get()))
			outPointLightEntityData.push_back(pointLightEntityData);
		else if (objectMember->get())
			collectTraceEntities(objectMember->get(), outDirectionalLightEntityData, outPointLightEntityData, outMeshEntityData);
	}
}

Guid advanceGuid(const Guid& seed, int32_t iterations)
{
	uint8_t data[16];
	std::memcpy(data, seed, 16);
	*(uint32_t*)data += iterations;
	return Guid(data);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.illuminate.IlluminateEntityPipeline", 0, IlluminateEntityPipeline, world::EntityPipeline)

IlluminateEntityPipeline::IlluminateEntityPipeline()
:	m_targetEditor(false)
{
}

bool IlluminateEntityPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	m_targetEditor = false; // settings->getProperty< PropertyBoolean >(L"Pipeline.TargetEditor", false);
	return true;
}

TypeInfoSet IlluminateEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IlluminateEntityData >());
	return typeSet;
}

bool IlluminateEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	// FIXME
	// Prevent building mesh;es which will be replaced by this entity.

	return world::EntityPipeline::buildDependencies(
		pipelineDepends,
		sourceInstance,
		sourceAsset,
		outputPath,
		outputGuid
	);
}

Ref< ISerializable > IlluminateEntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	RandomGeometry random;

	if (!m_targetEditor)
	{
		const IlluminateEntityData* sourceIlluminateEntityData = checked_type_cast< const IlluminateEntityData* >(sourceAsset);

		// Flatten entire hierarchy of illuminate entity.
		Ref< IlluminateEntityData > illumEntityData = checked_type_cast< IlluminateEntityData* >(resolveAllExternal(pipelineBuilder, sourceIlluminateEntityData));
		if (!illumEntityData)
		{
			log::error << L"IlluminateEntityPipeline failed; unable to resolve all external entities" << Endl;
			return 0;
		}

		// Get all trace entities.
		RefArray< world::DirectionalLightEntityData > directionalLightEntityData;
		RefArray< world::PointLightEntityData > pointLightEntityData;
		RefArray< mesh::MeshEntityData > meshEntityData;
		collectTraceEntities(illumEntityData, directionalLightEntityData, pointLightEntityData, meshEntityData);

		// Setup lights.
		AlignedVector< Light > lights;

		for (RefArray< world::DirectionalLightEntityData >::const_iterator i = directionalLightEntityData.begin(); i != directionalLightEntityData.end(); ++i)
		{
			Light light;
			light.type = 0;
			light.position = Vector4::origo();
			light.direction = (*i)->getTransform().rotation() * Vector4(0.0f, -1.0f, 0.0f);
			light.sunColor = Color4f((*i)->getSunColor());
			light.baseColor = Color4f((*i)->getBaseColor());
			light.shadowColor = Color4f((*i)->getShadowColor());
			light.range = Scalar(0.0f);
			lights.push_back(light);
		}

		for (RefArray< world::PointLightEntityData >::const_iterator i = pointLightEntityData.begin(); i != pointLightEntityData.end(); ++i)
		{
			Light light;
			light.type = 1;
			light.position = (*i)->getTransform().translation().xyz1();
			light.direction = Vector4::zero();
			light.sunColor = Color4f((*i)->getSunColor());
			light.range = Scalar((*i)->getRange());
			lights.push_back(light);
		}

		// Bake light onto each mesh entity; need to create unique mesh;es for each entity.
		for (RefArray< mesh::MeshEntityData >::const_iterator i = meshEntityData.begin(); i != meshEntityData.end(); ++i)
		{
			Ref< mesh::MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< mesh::MeshAsset >((*i)->getMesh());
			if (!meshAsset)
				continue;

			Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), meshAsset->getFileName().getOriginal());
			if (!file)
			{
				log::warning << L"Unable to open file \"" << meshAsset->getFileName().getOriginal() << L"\"" << Endl;
				continue;
			}

			Ref< model::Model > model = model::ModelFormat::readAny(
				file,
				meshAsset->getFileName().getExtension()
			);
			if (!model)
			{
				log::warning << L"Unable to read model \"" << meshAsset->getFileName().getOriginal() << L"\"" << Endl;
				continue;
			}

			if (!model::UnwrapUV(1, 5.0f).apply(*model))
				continue;

			// Setup tracer.
			const std::vector< model::Polygon >& polygons = model->getPolygons();
			std::vector< model::Vertex > vertices = model->getVertices();

			// Create 3d windings.
			AlignedVector< Winding3 > windings(polygons.size());
			for (uint32_t j = 0; j < polygons.size(); ++j)
			{
				Winding3& w = windings[j];
				
				const std::vector< uint32_t >& vertexIndices = polygons[j].getVertices();
				if (vertexIndices.size() < 3 || vertexIndices.size() > 16)
					continue;

				for (std::vector< uint32_t >::const_iterator k = vertexIndices.begin(); k != vertexIndices.end(); ++k)
				{
					const model::Vertex& polyVertex = model->getVertex(*k);
					const Vector4& polyVertexPosition = model->getPosition(polyVertex.getPosition());
					w.push(polyVertexPosition);
				}
			}

			// Create traceable surfaces.
			AlignedVector< Surface > surfaces(polygons.size());
			for (uint32_t j = 0; j < polygons.size(); ++j)
			{
				Surface& s = surfaces[j];

				const std::vector< uint32_t >& vertexIndices = polygons[j].getVertices();
				if (vertexIndices.size() < 3 || vertexIndices.size() > 16)
					continue;

				s.count = int32_t(vertexIndices.size());
				for (int32_t k = 0; k < s.count; ++k)
				{
					const model::Vertex& polyVertex = model->getVertex(vertexIndices[k]);

					s.points[k] = model->getPosition(polyVertex.getPosition());
					s.texCoords[k] = model->getTexCoord(polyVertex.getTexCoord(1));
					s.normals[k] = model->getNormal(polyVertex.getNormal());
				}

				Plane plane;
				if (windings[j].getPlane(plane))
					s.normal = plane.normal();
				else
					s.normal = Vector4::zero();

				uint32_t materialId = polygons[j].getMaterial();
				const model::Material& material = model->getMaterial(materialId);

				float rgba[4];
				material.getColor().getRGBA32F(rgba);

				s.color = Color4f(rgba);
				s.emissive = Scalar(material.getEmissive());

				// Add each emissive surface as a light source.
				if (s.emissive > FUZZY_EPSILON)
				{
					Light light;
					light.type = 2;
					light.position = Vector4::zero();
					light.direction = Vector4::zero();
					light.sunColor = s.color;
					light.range = Scalar(100.0f);
					light.surface = j;
					lights.push_back(light);
				}
			}

			// Build acceleration tree.
			SahTree sah;
			sah.build(windings);

			// Create GBuffer images.
			const GBuffer zero = { Vector4::zero(), Vector4::zero() };
			AlignedVector< GBuffer > gbuffer(c_outputWidth * c_outputHeight, zero);

			// Trace each polygon in UV space.
			Vector2 dim(c_outputWidth, c_outputHeight);
			Vector2 uv[3];
			Vector4 P[3], N[3];

			// Trace first, direct illumination, pass.
			log::info << L"Generating g-buffer..." << Endl;

			// First draw triangle poly-line to draw a big gutter around triangle.
			for (uint32_t j = 0; j < surfaces.size(); ++j)
			{
				const Surface& s = surfaces[j];

				AlignedVector< Vector2 > texCoords(s.count);
				for (int32_t k = 0; k < s.count; ++k)
					texCoords[k] = s.texCoords[k] * dim;

				std::vector< Triangulator::Triangle > triangles;
				Triangulator().freeze(
					texCoords,
					triangles
				);

				for (std::vector< Triangulator::Triangle >::const_iterator k = triangles.begin(); k != triangles.end(); ++k)
				{
					for (int32_t ii = 0; ii < 3; ++ii)
					{
						size_t i0 = k->indices[ii];
						size_t i1 = k->indices[(ii + 1) % 3];

						uv[0] = texCoords[i0];
						uv[1] = texCoords[i1];

						P[0] = s.points[i0];
						P[1] = s.points[i1];

						N[0] = s.normals[i0];
						N[1] = s.normals[i1];

						GBufferLineVisitor visitor(P, N, gbuffer);
						line(uv[0].x, uv[0].y, uv[1].x, uv[1].y, visitor);
					}
				}
			}

			// Then draw solid triangles to fill with correct data.
			for (uint32_t j = 0; j < surfaces.size(); ++j)
			{
				const Surface& s = surfaces[j];

				AlignedVector< Vector2 > texCoords(s.count);
				for (int32_t k = 0; k < s.count; ++k)
					texCoords[k] = s.texCoords[k] * dim;

				std::vector< Triangulator::Triangle > triangles;
				Triangulator().freeze(
					texCoords,
					triangles
				);

				for (std::vector< Triangulator::Triangle >::const_iterator k = triangles.begin(); k != triangles.end(); ++k)
				{
					size_t i0 = k->indices[0];
					size_t i1 = k->indices[1];
					size_t i2 = k->indices[2];

					uv[0] = texCoords[i0];
					uv[1] = texCoords[i1];
					uv[2] = texCoords[i2];

					P[0] = s.points[i0];
					P[1] = s.points[i1];
					P[2] = s.points[i2];

					N[0] = s.normals[i0];
					N[1] = s.normals[i1];
					N[2] = s.normals[i2];

					GBufferVisitor visitor1(P, N, gbuffer);
					triangle(uv[0], uv[1], uv[2], visitor1);

					std::swap(P[0], P[2]);
					std::swap(N[0], N[2]);
					std::swap(uv[0], uv[2]);

					GBufferVisitor visitor2(P, N, gbuffer);
					triangle(uv[0], uv[1], uv[2], visitor2);
				}
			}

			// Create output image.
			Ref< drawing::Image > outputImageDirect = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), c_outputWidth, c_outputHeight);
			Ref< drawing::Image > outputImageIndirect = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), c_outputWidth, c_outputHeight);
			
			outputImageDirect->clear(Color4f(0.0f, 0.0f, 1.0f, 0.0f));
			outputImageIndirect->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

			RefArray< Job > jobs;

			log::info << L"Tracing direct lighting..." << Endl;
			std::list< JobTraceDirect* > tracesDirect;
			for (int32_t y = 0; y < c_outputHeight; y += c_jobTileHeight)
			{
				for (int32_t x = 0; x < c_outputWidth; x += c_jobTileWidth)
				{
					JobTraceDirect* trace = new JobTraceDirect(
						x,
						y,
						sah,
						gbuffer,
						surfaces,
						lights,
						outputImageDirect
					);

					Ref< Job > job = JobManager::getInstance().add(makeFunctor< JobTraceDirect >(trace, &JobTraceDirect::execute));
					if (!job)
						return 0;

					tracesDirect.push_back(trace);
					jobs.push_back(job);
				}
			}

			for (RefArray< Job >::iterator j = jobs.begin(); j != jobs.end(); ++j)
				(*j)->wait();

			for(std::list< JobTraceDirect* >::iterator j = tracesDirect.begin(); j != tracesDirect.end(); ++j)
				delete *j;

			tracesDirect.clear();
			jobs.clear();

			log::info << L"Dilating direct light map..." << Endl;
			drawing::DilateFilter dilateFilter(4);
			outputImageDirect->apply(&dilateFilter);

			if (illumEntityData->traceIndirectLighting())
			{
				log::info << L"Tracing indirect lighting..." << Endl;
				std::list< JobTraceIndirect* > tracesIndirect;
				for (int32_t y = 0; y < c_outputHeight; y += c_jobTileHeight)
				{
					for (int32_t x = 0; x < c_outputWidth; x += c_jobTileWidth)
					{
						JobTraceIndirect* trace = new JobTraceIndirect(
							x,
							y,
							sah,
							gbuffer,
							surfaces,
							outputImageDirect,
							outputImageIndirect,
							illumEntityData->getIndirectTraceSamples()
						);

						Ref< Job > job = JobManager::getInstance().add(makeFunctor< JobTraceIndirect >(trace, &JobTraceIndirect::execute));
						if (!job)
							return 0;

						tracesIndirect.push_back(trace);
						jobs.push_back(job);
					}
				}

				for (RefArray< Job >::iterator j = jobs.begin(); j != jobs.end(); ++j)
					(*j)->wait();

				for(std::list< JobTraceIndirect* >::iterator j = tracesIndirect.begin(); j != tracesIndirect.end(); ++j)
					delete *j;

				tracesIndirect.clear();
				jobs.clear();

				log::info << L"Dilating indirect light map..." << Endl;
				drawing::DilateFilter dilateFilter(4);
				outputImageIndirect->apply(&dilateFilter);

				log::info << L"Convolving indirect lighting..." << Endl;
				outputImageIndirect->apply(drawing::ConvolutionFilter::createGaussianBlur5());
			}

			outputImageDirect->save(illumEntityData->getSeedGuid().format() + L"_direct.png");
			outputImageIndirect->save(illumEntityData->getSeedGuid().format() + L"_indirect.png");

			// Merge light-maps.
			for (int32_t y = 0; y < c_outputHeight; ++y)
			{
				for (int32_t x = 0; x < c_outputWidth; ++x)
				{
					Color4f inA, inB;
					outputImageIndirect->getPixelUnsafe(x, y, inA);
					outputImageDirect->getPixelUnsafe(x, y, inB);
					outputImageDirect->setPixelUnsafe(x, y, (inA + inB) * Color4f(1.0f, 1.0f, 1.0f, 0.0f) + Color4f(0.0f, 0.0f, 0.0f, 1.0f));
				}
			}

			outputImageIndirect = 0;

			// Save light-map for debugging.
			outputImageDirect->save(illumEntityData->getSeedGuid().format() + L".png");
			model::ModelFormat::writeAny(illumEntityData->getSeedGuid().format() + L".obj", model);

			log::info << L"Creating resources..." << Endl;

			// Create a texture build step.
			Ref< render::TextureOutput > textureOutput = new render::TextureOutput();
			textureOutput->m_keepZeroAlpha = false;
			textureOutput->m_hasAlpha = false;
			textureOutput->m_ignoreAlpha = true;
			textureOutput->m_linearGamma = true;
			textureOutput->m_systemTexture = true;
			textureOutput->m_enableCompression = illumEntityData->compressLightMap();
			pipelineBuilder->buildOutput(
				textureOutput,
				L"Generated/__Illumination__Texture__" + advanceGuid(illumEntityData->getSeedGuid(), 0).format(),
				advanceGuid(illumEntityData->getSeedGuid(), 0),
				outputImageDirect
			);

			// Modify model materials to use our illumination texture.
			std::vector< model::Material > materials = model->getMaterials();
			for (std::vector< model::Material >::iterator j = materials.begin(); j != materials.end(); ++j)
			{
				j->setLightMap(model::Material::Map(L"__Illumination__", 1, false));
				j->setEmissive(0.0f);
			}
			model->setMaterials(materials);

			// Create a new mesh asset which use the fresh baked illumination texture.
			std::map< std::wstring, Guid > meshMaterialTextures = meshAsset->getMaterialTextures();
			meshMaterialTextures[L"__Illumination__"] = advanceGuid(illumEntityData->getSeedGuid(), 0);

			Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
			outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
			outputMeshAsset->setMaterialTextures(meshMaterialTextures);
			outputMeshAsset->setGenerateOccluder(false);

			pipelineBuilder->buildOutput(
				outputMeshAsset,
				L"Generated/__Illumination__Mesh__" + advanceGuid(illumEntityData->getSeedGuid(), 1).format(),
				advanceGuid(illumEntityData->getSeedGuid(), 1),
				model
			);

			// Then replace mesh used by mesh entity with the illuminated version.
			(*i)->setMesh(resource::Id< mesh::IMesh >(advanceGuid(illumEntityData->getSeedGuid(), 1)));
		}

		// Replace illumination entity data with a plain group.
		Ref< world::GroupEntityData > groupEntityData = new world::GroupEntityData();
		groupEntityData->setName(illumEntityData->getName());
		groupEntityData->setTransform(illumEntityData->getTransform());
		groupEntityData->setEntityData(illumEntityData->getEntityData());
		return groupEntityData;
	}
	else
		return world::EntityPipeline::buildOutput(
			pipelineBuilder,
			sourceAsset
		);
}

	}
}
