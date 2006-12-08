#ifndef ARSCENE_H
#define ARSCENE_H

#include <Producer/RenderSurface>
#include <osgProducer/Viewer>

#include <osg/Node>
#include <osg/Group>
#include <osg/Projection>

#include <osgART/Foundation>
#include <osgART/VideoManager>
#include <osgART/ARTTransform>
#include <osgART/VideoBackground>
#include <osgART/VideoTextureRectangle>

#include "ARNode.h"
#include "DummyImageLayer.h"
#include "FBOManager.h"

#include <vector>
using namespace std;


class ARScene : public osg::Group
{
public:
	ARScene();
	virtual ~ARScene();

	virtual void init(osg::ref_ptr<osgART::GenericTracker> tracker, int _trakerID = 0);

	void addARNode(osg::ref_ptr<ARNode> arnode, int binNum, bool addToSceneGraph = true);

	virtual osg::ref_ptr<ARNode> addNewARNodeWith(osg::ref_ptr<osg::Node> node, int binNum = 20);
	
	void addToBackgroundGroup(osg::Node *aNode);
	void addToBackgroundTextureGroup(osg::Node *aNode, bool isARNode);
	

	// choose to use only one!
	osg::ref_ptr<osgART::VideoBackground> initDefaultVideoBackground(int id);
	osg::ref_ptr<osg::Texture> initTextureVideoBackground(int id, int colNum = 1, int rowNum = 1, bool addDummyLayer = true );

	void initDefaultForeground();
	osg::ref_ptr<osg::Texture> initTextureForeground(int colNum = 1, int rowNum = 1 , bool addDummyLayer = true);


	osg::ref_ptr<osg::Group> getSceneGroup()
	{
		return sceneGroup;
	};
	
	osg::ref_ptr<osg::Group> getBackgroundTextureGroup()
	{
		return backgroundGroup;
	};

	osg::ref_ptr<osg::Group> getForegroundGroup()
	{
		return foregroundGroup;
	};

	osg::ref_ptr<osg::Texture> getBackgroundTexture()
	{
		return backgroundTexture;
	};

	osg::ref_ptr<ARNode> at(int id);
	int size();

	osg::ref_ptr<DummyImageLayer> getBackgroundDummyLayer()
	{
		return backgroundDummyLayer;
	};
	osg::ref_ptr<DummyImageLayer> getForegroundDummyLayer()
	{
		return foregroundDummyLayer;
	};
	
protected:

	osg::ref_ptr<osgART::VideoBackground> makeVideoBackground(int id);

	osg::ref_ptr<FBOManager> fboManager;
	vector< osg::ref_ptr<ARNode> > arNodes;

	int trackerID;
	osg::ref_ptr<osg::Projection> projectionMatrix;
	osg::ref_ptr<osg::Projection> projectionMatrixForFBO;
	
	osg::ref_ptr<osg::Group> sceneGroup;
	osg::ref_ptr<osg::Group> backgroundGroup;
	osg::ref_ptr<osg::Group> foregroundGroup;


	// for background
	int bgWidth, bgHeight;

	osg::ref_ptr<osg::Texture> backgroundTexture;
	osg::ref_ptr<osg::Texture> foregroundTexture;

	osg::ref_ptr<DummyImageLayer> backgroundDummyLayer;
	osg::ref_ptr<DummyImageLayer> foregroundDummyLayer;

};

#endif