#include "FBXParser.h"
#include <iostream>
using namespace std;

int FBXParser::ProcessingFBXdata(char* filePath)
{
	FbxManager* sdkManager = FbxManager::Create();
	FbxIOSettings* ioSettings = FbxIOSettings::Create(sdkManager, IOSROOT);
	sdkManager->SetIOSettings(ioSettings);
	FbxImporter* fbxImporter = FbxImporter::Create(sdkManager, "thisfbxScene");
	if (!fbxImporter->Initialize(filePath, -1, sdkManager->GetIOSettings()))
	{
		cout << "Failed to initialize importer:" << fbxImporter->GetStatus().GetErrorString() << endl;
		return 1;
	}
	FbxScene* fbxScene = FbxScene::Create(sdkManager, "scene");
	if (!fbxImporter->Import(fbxScene))
	{
		cout << "Failed to import scene:" << fbxImporter->GetStatus().GetErrorString() << endl;
		return 1;
	}
	ParserNode(fbxScene->GetRootNode());
}

void FBXParser::ParserNode(FbxNode* fbxNode)
{
	for(int i = 0; i < fbxNode->GetNodeAttributeCount(); i++)
	{
		FbxMesh* fbxMesh = static_cast<FbxMesh*>(fbxNode->GetNodeAttributeByIndex(i));
		if (fbxMesh->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			auto&& id = "object" + to_string(globalId++);
			GetGeometryAndMaterial(fbxMesh, id);
		}
	}
	for (int i = 0; i < fbxNode->GetChildCount(); i++)
	{
		auto&& subFbxNode = fbxNode->GetChild(i);
		ParserNode(subFbxNode);
	}
}

void FBXParser::GetGeometryAndMaterial(FbxMesh* fbxMesh, string id)
{
	if (!fbxMesh->IsTriangleMesh())
	{
		FbxGeometryConverter geometryConverter(fbxMesh->GetFbxManager());
		fbxMesh = static_cast<FbxMesh*>(geometryConverter.Triangulate(fbxMesh, true));
	}
	auto&& geometryItem = GetGeometryItem(fbxMesh, id);
	geometryGroup.emplace_back(geometryItem);
	auto&& materialItem = GetMaterialItem(fbxMesh, id);
	auto&& tansformValue = (geometryItem.geometryId, materialItem.materialId, GettansformValue(fbxMesh));
}

GeometryItem FBXParser::GetGeometryItem(FbxMesh* fbxMesh, string id)
{
	GeometryItem geometryItem;
	//	Setting geometryId 
	geometryItem.geometryId = "mesh" + id;
	//	Get vertex coordinates of geometry
	int vertexCount = fbxMesh->GetControlPointsCount();
	if (vertexCount != 0)
	{
		//	Get vertex coordinates of geometry
		FbxVector4* controlPoints = fbxMesh->GetControlPoints();
		for (int i = 0; i < vertexCount; i++)
		{
			FbxVector4 vertex = controlPoints[i];
			geometryItem.vertices.push_back(static_cast<float>(vertex[0]));
			geometryItem.vertices.push_back(static_cast<float>(vertex[1]));
			geometryItem.vertices.push_back(static_cast<float>(vertex[2]));
		}
		//	Get UV of geometry
		FbxGeometryElementUV* geometryElementUV = fbxMesh->GetElementUV(0);
		if (geometryElementUV)
		{
			for (int i = 0; i < vertexCount; i++)
			{
				FbxVector2 uv = geometryElementUV->GetDirectArray().GetAt(i);
				geometryItem.uvs.push_back(static_cast<float>(uv[0]));
				geometryItem.uvs.push_back(static_cast<float>(uv[1]));
			}
		}
		//	Get polygon indices of geometry
		int polygonCount = fbxMesh->GetPolygonCount();
		for (int i = 0; i < polygonCount; ++i) {
			for (int j = 0; j < 3; ++j) {
				int controlPointIndex = fbxMesh->GetPolygonVertex(i, j);
				geometryItem.indices.push_back(controlPointIndex);
			}
		}
		//	Get normal of geometry
		FbxGeometryElementNormal* geometryElementNormal = fbxMesh->GetElementNormal();
		int normalCount = geometryElementNormal->GetDirectArray().GetCount();
		if (normalCount > 0 && normalCount == vertexCount)
		{
			for (int i = 0; i < normalCount; i++)
			{
				int index = i;
				if (geometryElementNormal->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
				{
					index = geometryElementNormal->GetIndexArray().GetAt(i);
				}
				FbxVector4 normal = geometryElementNormal->GetDirectArray().GetAt(index);
				geometryItem.normals.push_back(static_cast<float>(normal[0]));
				geometryItem.normals.push_back(static_cast<float>(normal[1])); 
				geometryItem.normals.push_back(static_cast<float>(normal[2])); 
			}
		}
		else
		{
			for (int i = 0; i < vertexCount; ++i) 
			{
				geometryItem.normals.push_back(static_cast<float>(1.0)); 
				geometryItem.normals.push_back(static_cast<float>(1.0)); 
				geometryItem.normals.push_back(static_cast<float>(1.0)); 
			}
			std::cout << "do not find GeometryElementNormal for " << geometryItem.geometryId << ", create default normal" << std::endl;
		}
	}
	return geometryItem;
}

MaterialItem FBXParser::GetMaterialItem(FbxMesh* fbxMesh, string id)
{
	MaterialItem materialItem;
	//	Setting materialId 
	materialItem.materialId = "material" + id;

	int materialCount = fbxMesh->GetNode()->GetSrcObjectCount<FbxSurfaceMaterial>();
	for (int i = 0; i < materialCount; i++)
	{
		FbxSurfaceMaterial* material = fbxMesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(i);
		// Get textures of material
		materialItem.materialFilename = GetMaterialTextures(material);
		// Get color of material
		materialItem.materialColor = GetMaterialColor(fbxMesh);
	}
	return materialItem;
}

const char* FBXParser::GetMaterialTextures(FbxSurfaceMaterial* material)
{
	const char* filename = nullptr;
	FbxProperty property = material->FindProperty(FbxLayerElement::sTextureChannelNames[0]);
	if (property.IsValid())
	{
		unsigned int textureCount = property.GetSrcObjectCount<FbxTexture>();
		for (unsigned int j = 0; j < textureCount; ++j)
		{
			FbxTexture* texture = property.GetSrcObject<FbxTexture>(j);
			if (texture)
			{
				std::string textureType = property.GetNameAsCStr();
				FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);
				if (fileTexture)
				{
					if (textureType == "DiffuseColor") {
						filename = fileTexture->GetRelativeFileName();
					}
					else if (textureType == "SpecularColor") {
						//fileTexture->GetFileName(); 目前不需要
					}
					else if (textureType == "Bump") {
						//fileTexture->GetFileName(); 目前不需要
					}
				}
			}
			/*else
			{
				FbxLayeredTexture* layeredTexture = property.GetSrcObject<FbxLayeredTexture>(j);
				if (layeredTexture)
				{
					throw std::exception("Layered Texture is currently unsupported\n");
					return NULL;
				}
			}*/
		}
	}
	return filename;
}

vector<float> FBXParser::GetMaterialColor(FbxMesh* fbxMesh)
{
	vector<float> materialColor = { 1, 1, 1};
	int materialCount = fbxMesh->GetNode()->GetSrcObjectCount<FbxSurfaceMaterial>();
	for (int i = 0; i < materialCount; ++i)
	{
		FbxSurfaceMaterial* material = fbxMesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(i);
		if (material && material->GetClassId().Is(FbxSurfaceLambert::ClassId))
		{
			FbxSurfaceLambert* lambertMaterial = static_cast<FbxSurfaceLambert*>(material);
			FbxDouble3 diffuseColor = lambertMaterial->Diffuse.Get();
			materialColor[0] = static_cast<float>(diffuseColor[0]);
			materialColor[1] = static_cast<float>(diffuseColor[1]);
			materialColor[2] = static_cast<float>(diffuseColor[2]);
			return materialColor;
		}
	}
	return materialColor;
}

vector<float> FBXParser::GettansformValue(FbxMesh* fbxMesh)
{
	vector<float> tansformValue;
	FbxNode* currentNode = fbxMesh->GetNode();
	FbxAMatrix parentMatrix;
	parentMatrix.SetIdentity();
	while (currentNode != nullptr) {
		FbxNode::EPivotSet pivotSet = FbxNode::EPivotSet::eSourcePivot;
		FbxAMatrix geometricTransformMatrix;
		FbxVector4 geometricTranslation, geometricRotation, geometricScaling;
		geometricRotation = currentNode->GetGeometricRotation(pivotSet);
		geometricScaling = currentNode->GetGeometricScaling(pivotSet);
		geometricTranslation = currentNode->GetGeometricTranslation(pivotSet);
		geometricTransformMatrix.SetT(geometricTranslation);
		geometricTransformMatrix.SetR(geometricRotation);
		geometricTransformMatrix.SetS(geometricScaling);
		parentMatrix = parentMatrix * geometricTransformMatrix;
		currentNode = currentNode->GetParent();
	}
	//转置变换矩阵
	// lod transform
	// / 0  3  6  9  \
	// | 1  4  7  10 |
	// | 2  5  8  11 |
	// \ -  -  -  -  /
	// fbx transform
	// fbxMatrix.mData[0] = FbxDouble4(transformArray[0], transformArray[1], transformArray[2], 0);
	// fbxMatrix.mData[1] = FbxDouble4(transformArray[3], transformArray[4], transformArray[5], 0);
	// fbxMatrix.mData[2] = FbxDouble4(transformArray[6], transformArray[7], transformArray[8], 0);
	// fbxMatrix.mData[3] = FbxDouble4(transformArray[9], transformArray[10], transformArray[11], 1);
	//parentMatrix.Transpose();
	tansformValue = {
		(float)parentMatrix.mData[0].mData[0] ,(float)parentMatrix.mData[0].mData[1],(float)parentMatrix.mData[0].mData[2],
		(float)parentMatrix.mData[1].mData[0] ,(float)parentMatrix.mData[1].mData[1],(float)parentMatrix.mData[1].mData[2],
		(float)parentMatrix.mData[2].mData[0] ,(float)parentMatrix.mData[2].mData[1],(float)parentMatrix.mData[2].mData[2],
		(float)parentMatrix.mData[3].mData[0] ,(float)parentMatrix.mData[3].mData[1],(float)parentMatrix.mData[3].mData[2]
	};
	return tansformValue;
}
