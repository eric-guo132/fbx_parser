#pragma once

using namespace std;

#ifdef _MSC_VER
#ifdef DDL_FBXPRASER_EXPORT
#define FBXPRASER_EXPORT _declspec(dllexport)
#else
#define FBXPRASER_EXPORT _declspec(dllimport)
#endif // DDL_FBXPRASER_EXPORT
#else
#define FBXPRASER_EXPORT 
#endif // _MSC_VER

class  StoreVisitor 
{

public:
	virtual ~StoreVisitor() {};
	virtual void OutputMaterial() = 0;
	virtual void OutputGemetry() = 0;
	virtual void OutputTreeNode() = 0;
};

FBXPRASER_EXPORT class FBXPraserBash
{
public:
	virtual void ApplyProperty(StoreVisitor* propertiesVisitor) = 0;
	virtual void ApplyMaterial(StoreVisitor* materialsVisitor) = 0;
	virtual void ApplyGemetry(StoreVisitor* geometriesVisitor) = 0;
	virtual void ApplyTreeNode(StoreVisitor* treeNodesVisitor) = 0;
public:
	virtual int ProcessingFBXdata(char* filePath) = 0;
};