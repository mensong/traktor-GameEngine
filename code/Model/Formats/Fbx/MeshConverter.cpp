#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/TString.h"
#include "Model/Model.h"
#include "Model/Formats/Fbx/Conversion.h"
#include "Model/Formats/Fbx/MaterialConverter.h"
#include "Model/Formats/Fbx/MeshConverter.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

FbxMatrix getGeometricTransform(const FbxNode* fbxNode)
{
	FbxVector4 t = fbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 r = fbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxVector4 s = fbxNode->GetGeometricScaling(FbxNode::eSourcePivot);
	return FbxMatrix(t, r, s);
}

uint32_t uvChannel(Model& outModel, const std::string& uvSet)
{
	// In case "default" uv set requested then assume first but do NOT add to inoutChannels
	// in order to when first named UVSet is extracted it will get 0 index.
	if (uvSet != "default")
		return outModel.addUniqueTexCoordChannel(mbstows(uvSet));
	else
		return 0;
}

		}

bool convertMesh(
	Model& outModel,
	FbxScene* scene,
	FbxNode* meshNode,
	const Matrix44& axisTransform
)
{
	int32_t vertexId = 0;

	FbxMesh* mesh = static_cast< FbxMesh* >(meshNode->GetNodeAttribute());
	if (!mesh)
		return false;

	uint32_t positionBase = uint32_t(outModel.getPositions().size());

	// Convert materials.
	std::map< int32_t, int32_t > materialMap;
	if (!convertMaterials(outModel, materialMap, meshNode))
		return false;

	FbxAMatrix nodeTransform = meshNode->EvaluateGlobalTransform();

	Matrix44 Mnode = convertMatrix(nodeTransform);
	Matrix44 Mglobal = axisTransform * Mnode;

	// Convert vertex positions.
	FbxVector4* controlPoints = mesh->GetControlPoints();
	if (!controlPoints)
		return false;

	int32_t controlPointsCount = mesh->GetControlPointsCount();
	for (int32_t i = 0; i < controlPointsCount; ++i)
	{
		Vector4 v = Mglobal * convertVector4(controlPoints[i]).xyz1();
		outModel.addPosition(v);
	}

	// Convert joint vertex weights; model must contain joints.
	typedef std::map< uint32_t, float > bone_influences_t;

	AlignedVector< bone_influences_t > vertexJoints;
	vertexJoints.resize(controlPointsCount);

	int32_t deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
	for (int32_t i = 0; i < deformerCount; ++i)
	{
		FbxSkin* skinDeformer = (FbxSkin*)mesh->GetDeformer(i, FbxDeformer::eSkin);
		if (!skinDeformer)
			continue;

		int32_t clusterCount = skinDeformer->GetClusterCount();
		for (int32_t j = 0; j < clusterCount; ++j)
		{
			FbxCluster* cluster = skinDeformer->GetCluster(j);
			if (!cluster)
				continue;

			int32_t weightCount = cluster->GetControlPointIndicesCount();
			if (weightCount <= 0)
				continue;

			const FbxNode* jointNode = cluster->GetLink();
			T_ASSERT (jointNode);
			
			std::wstring jointName = mbstows(jointNode->GetName());
			
			size_t p = jointName.find(L':');
			if (p != std::wstring::npos)
				jointName = jointName.substr(p + 1);

			uint32_t jointIndex = outModel.findJointIndex(jointName);
			if (jointIndex == c_InvalidIndex)
			{
				log::warning << L"Unable to set vertex weight; no such joint \"" << jointName << L"\"." << Endl;
				continue;
			}

			const double* weights = cluster->GetControlPointWeights();
			const int32_t* indices = cluster->GetControlPointIndices();

			for (int32_t k = 0; k < weightCount; ++k)
			{
				int32_t vertexIndex = indices[k];
				float jointWeight = float(weights[k]);
				if (jointWeight > FUZZY_EPSILON)
					vertexJoints[vertexIndex].insert(std::pair< uint32_t, float >(jointIndex, jointWeight));
			}
		}
	}

	// Convert polygons.
	int32_t polygonCount = mesh->GetPolygonCount();
	for (int32_t i = 0; i < polygonCount; ++i)
	{
		Polygon polygon;

		// Assign material; \fixme should probably create a polygon for each layer of material.
		for (int32_t j = 0; j < mesh->GetLayerCount(); ++j)
		{
			FbxLayerElementMaterial* layerMaterials = mesh->GetLayer(j)->GetMaterials();
			if (layerMaterials)
			{
				int32_t materialIndex = layerMaterials->GetIndexArray().GetAt(i);
				polygon.setMaterial(materialMap[materialIndex]);
			}
		}

		// // If no material found but mesh have a material defined then set first material.
		// if (materialCount > 0 && polygon.getMaterial() == c_InvalidIndex)
		// 	polygon.setMaterial(materialBase);

		int32_t polygonSize = mesh->GetPolygonSize(i);
		for (int32_t j = 0; j < polygonSize; ++j, ++vertexId)
		{
			int32_t pointIndex = mesh->GetPolygonVertex(i, j);

			Vertex vertex;
			vertex.setPosition(positionBase + pointIndex);

			if (pointIndex < int32_t(vertexJoints.size()))
			{
				for (bone_influences_t::const_iterator k = vertexJoints[pointIndex].begin(); k != vertexJoints[pointIndex].end(); ++k)
					vertex.setJointInfluence(k->first, k->second);
			}

			for (int32_t k = 0; k < mesh->GetLayerCount(); ++k)
			{
				FbxLayerElementVertexColor* layerVertexColors = mesh->GetLayer(k)->GetVertexColors();
				if (layerVertexColors)
				{
					switch (layerVertexColors->GetMappingMode())
					{
					case FbxGeometryElement::eByControlPoint:
						switch (layerVertexColors->GetReferenceMode())
						{
						case FbxLayerElement::eDirect:
							{
								FbxColor color = layerVertexColors->GetDirectArray().GetAt(pointIndex);
								vertex.setColor(outModel.addUniqueColor(convertColor(color)));
							}
							break;

						case FbxLayerElement::eIndexToDirect:
							{
								int32_t id = layerVertexColors->GetIndexArray().GetAt(pointIndex);
								FbxColor color = layerVertexColors->GetDirectArray().GetAt(id);
								vertex.setColor(outModel.addUniqueColor(convertColor(color)));
							}
							break;

						default:
							break;
						}
						break;

					case FbxGeometryElement::eByPolygonVertex:
						int32_t textureUVIndex = mesh->GetPolygonVertexIndex(i) + j;
						switch (layerVertexColors->GetReferenceMode())
						{
						case FbxLayerElement::eDirect:
							{
								FbxColor color = layerVertexColors->GetDirectArray().GetAt(textureUVIndex);
								vertex.setColor(outModel.addUniqueColor(convertColor(color)));
							}
							break;

						case FbxLayerElement::eIndexToDirect:
							{
								int32_t id = layerVertexColors->GetIndexArray().GetAt(textureUVIndex);
								FbxColor color = layerVertexColors->GetDirectArray().GetAt(id);
								vertex.setColor(outModel.addUniqueColor(convertColor(color)));
							}
							break;

						default:
							break;
						}
						break;
					}
				}

				// Vertex texture UVs.
				FbxLayerElementUV* layerUVs = mesh->GetLayer(k)->GetUVs();
				if (layerUVs)
				{
					uint32_t channel = uvChannel(outModel, layerUVs->GetName());
					switch (layerUVs->GetMappingMode())
					{
					case FbxLayerElement::eByControlPoint:
						switch (layerUVs->GetReferenceMode())
						{
						case FbxLayerElement::eDirect:
							{
								Vector2 uv = convertVector2(layerUVs->GetDirectArray().GetAt(pointIndex)) * Vector2(1.0f, -1.0f) + Vector2(0.0f, 1.0f);
								vertex.setTexCoord(channel, outModel.addUniqueTexCoord(uv));
							}
							break;

						case FbxLayerElement::eIndexToDirect:
							{
								int32_t id = layerUVs->GetIndexArray().GetAt(pointIndex);
								Vector2 uv = convertVector2(layerUVs->GetDirectArray().GetAt(id)) * Vector2(1.0f, -1.0f) + Vector2(0.0f, 1.0f);
								vertex.setTexCoord(channel, outModel.addUniqueTexCoord(uv));
							}
							break;

						default:
							break;
						}
						break;

					case FbxLayerElement::eByPolygonVertex:
						{
							int32_t textureUVIndex = mesh->GetTextureUVIndex(i, j);
							switch (layerUVs->GetReferenceMode())
							{
							case FbxLayerElement::eDirect:
							case FbxLayerElement::eIndexToDirect:
								{
									Vector2 uv = convertVector2(layerUVs->GetDirectArray().GetAt(textureUVIndex)) * Vector2(1.0f, -1.0f) + Vector2(0.0f, 1.0f);
									vertex.setTexCoord(channel, outModel.addUniqueTexCoord(uv));
								}
								break;

							default:
								break;
							}
						}
						break;
						
					default:
						break;
					}
				}

				// Vertex normals.
				FbxLayerElementNormal* layerNormals = mesh->GetLayer(k)->GetNormals();
				if (layerNormals)
				{
					if (layerNormals->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
					{
						switch (layerNormals->GetReferenceMode())
						{
						case FbxLayerElement::eDirect:
							{
								FbxVector4 n = layerNormals->GetDirectArray().GetAt(vertexId);
								Vector4 normal = Mglobal * convertNormal(n);
								vertex.setNormal(outModel.addUniqueNormal(normal.normalized()));
							}
							break;

						case FbxLayerElement::eIndexToDirect:
							{
								int32_t id = layerNormals->GetIndexArray().GetAt(vertexId);
								FbxVector4 n = layerNormals->GetDirectArray().GetAt(id);
								Vector4 normal = Mglobal * convertNormal(n);
								vertex.setNormal(outModel.addUniqueNormal(normal.normalized()));
							}
							break;

						default:
							break;
						}
					}
				}

				// Vertex tangents.
				FbxLayerElementTangent* layerTangents = mesh->GetLayer(k)->GetTangents();
				if (layerTangents)
				{
					if (layerTangents->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
					{
						switch (layerTangents->GetReferenceMode())
						{
						case FbxLayerElement::eDirect:
							{
								FbxVector4 t = layerTangents->GetDirectArray().GetAt(vertexId);
								Vector4 tangent = Mglobal * convertNormal(t);
								vertex.setTangent(outModel.addUniqueNormal(tangent.normalized()));
							}
							break;

						case FbxLayerElement::eIndexToDirect:
							{
								int32_t id = layerTangents->GetIndexArray().GetAt(vertexId);
								FbxVector4 t = layerTangents->GetDirectArray().GetAt(id);
								Vector4 tangent = Mglobal * convertNormal(t);
								vertex.setTangent(outModel.addUniqueNormal(tangent.normalized()));
							}
							break;

						default:
							break;
						}
					}
				}

				// Vertex binormals.
				FbxLayerElementBinormal* layerBinormals = mesh->GetLayer(k)->GetBinormals();
				if (layerBinormals)
				{
					if (layerBinormals->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
					{
						switch (layerBinormals->GetReferenceMode())
						{
						case FbxLayerElement::eDirect:
							{
								FbxVector4 b = layerBinormals->GetDirectArray().GetAt(vertexId);
								Vector4 binormal = Mglobal * convertNormal(b);
								vertex.setBinormal(outModel.addUniqueNormal(binormal.normalized()));
							}
							break;

						case FbxLayerElement::eIndexToDirect:
							{
								int32_t id = layerBinormals->GetIndexArray().GetAt(vertexId);
								FbxVector4 b = layerBinormals->GetDirectArray().GetAt(id);
								Vector4 binormal = Mglobal * convertNormal(b);
								vertex.setBinormal(outModel.addUniqueNormal(binormal.normalized()));
							}
							break;

						default:
							break;
						}
					}
				}
			}

			polygon.addVertex(outModel.addUniqueVertex(vertex));
		}

		outModel.addPolygon(polygon);
	}

	return true;
}

	}
}
