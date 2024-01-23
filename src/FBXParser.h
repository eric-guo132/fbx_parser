#include "Export/FBXPraserBash.h"
#include<vector>
#include<string>
#define FBXSDK_SHARED
#define FBXSDK_NEW_API
#include "fbxsdk.h"
#include "fbxsdk/core/base/fbxstring.h"

enum class TexType
{
	None = -1,
	Ambient = 0, //环境光
	Diffuse = 1, //漫反射
	Specular = 2, //高光
	Emissive = 3, //自发光
	Roughness = 4, //粗糙度
	Reflective = 5, //反射
	Refract = 6,//折射
	BumpMap = 7, //凹凸贴图
	Transparent = 8,//透明度贴图
	DisplacementMap = 9,//置换贴图
	Metalness = 10//金属
};
struct MeshNodeItem
{
	std::string meshObjectId;
	std::vector<double> transforms;
	std::string geometryId;
	std::string materialId;
};
struct GeometryItem 
{
	std::string geometryId;
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	std::vector<float> uvs;
	std::vector<float> normals;
};
struct MaterialItem
{
	std::string materialId;
	vector<float> materialColor;
	double transparency;
	TexType texType;
	string texturePath;
};

class FBXParser : public FBXPraserBash
{
public:
	void ApplyProperty(StoreVisitor* propertiesVisitor) override;
	void ApplyMaterial(StoreVisitor* materialsVisitor) override;
	void ApplyGemetry(StoreVisitor* geometriesVisitor) override;
	void ApplyTreeNode(StoreVisitor* treeNodesVisitor) override;
public:
	int ProcessingFBXdata(char* filePath) override;
	
private:
	void ParserNode(FbxNode* fbxNode);
	void GetGeometryAndMaterial(FbxMesh* fbxMesh, string id);
	GeometryItem GetGeometryItem(FbxMesh* fbxMesh, string id);
	MaterialItem GetMaterialItem(FbxMesh* fbxMesh, string id);
	const char* GetMaterialTextures(FbxSurfaceMaterial* material);
	vector<float> GetMaterialColor(FbxMesh* fbxMesh);
	vector<float> GettansformValue(FbxMesh* fbxMesh);
public:
	vector<GeometryItem> geometryGroup;
	vector<MaterialItem> materialGroup;
	vector<MeshNodeItem> meshNodeGroup;
private:
	int objectglobalId = 0;
	int geometryglobalId = 0;
};