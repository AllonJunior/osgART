/*
 *	osgART/Example_SimpleCamera
 *	osgART: AR ToolKit for OpenSceneGraph
 *
 *	Copyright (c) 2005-2007 ARToolworks, Inc. All rights reserved.
 *	
 *	Rev		Date		Who		Changes
 *  1.0   	2006-12-08  ---     Version 1.0 release.
 *
 */
// @@OSGART_LICENSE_HEADER_BEGIN@@
// @@OSGART_LICENSE_HEADER_END@@

/*
 *
 * A simple example to demonstrate the most basic functionality of osgART.
 * By Julian Looser, Philip Lamb, Raphael Grasset, Hartmut Seichter.
 *
 */

#include <osg/Version>
#if (OSG_VERSION_MAJOR == 1 && OSG_VERSION_MINOR < 3)
#error OSG version 1.3 or later is required.
#endif

#include <Producer/RenderSurface>
#include <osgProducer/Viewer>

#include <osg/Notify>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Projection>
#include <osg/AutoTransform>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Image>
#include <osg/Camera>

#include <osgART/Foundation>
#include <osgART/VideoManager>
#include <osgART/ARTTransform>
#include <osgART/TrackerManager>
#include <osgART/VideoBackground>
#include <osgART/VideoPlane>
#include <osgART/VideoForeground>

// Please read documentation for setting video parameters
#define MY_VCONF ""

int main(int argc, char* argv[]) 
{

	osg::setNotifyLevel(osg::FATAL);

	osgARTInit(&argc, argv);
	
	osgProducer::Viewer viewer;
	viewer.setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE);
	viewer.getCullSettings().setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

#ifndef __linux
	// somehow on Ubuntu Dapper this ends up in a segmentation fault
	viewer.getCamera(0)->getRenderSurface()->fullScreen(false);
#endif

	// load a video plugin
	osg::ref_ptr<osgART::GenericVideo> video = 
	osgART::VideoManager::createVideoFromPlugin("osgart_artoolkit");

	// check if loading the plugin was successful
	if (!video.valid()) 
	{        
		// without video an AR application can not work
		osg::notify(osg::FATAL) << "Could not initialize video plugin!" << std::endl;

		// quit the program
		exit(1);
	}

	//get the video configuration
	osgART::VideoConfiguration* _config = video->getVideoConfiguration();

	//update it with the configuration of our camera
	if (_config)
	{
		_config->deviceconfig = MY_VCONF;
	}

	
	/* load a tracker plugin */
	osg::ref_ptr<osgART::GenericTracker> tracker = 
		osgART::TrackerManager::createTrackerFromPlugin("osgart_artoolkit_tracker");

	if (tracker.valid()) 
	{

		// access a field within the tracker
		osg::ref_ptr< osgART::TypedField<int> > _threshold = 
			reinterpret_cast< osgART::TypedField<int>* >(tracker->get("threshold"));

		// values can only be accessed through a get()/set() mechanism
		if (_threshold.valid()) 
		{			
			// set the threshold
			_threshold->set(100);

			/* check what we actually get */
			osg::notify() << "Field 'threshold' = " << _threshold->get() << std::endl;

		} else 
		{
			osg::notify() << "Field 'threshold' supported for this tracker" << std::endl;
		}		

	} else 
	{
        // this example needs a tracker
		std::cerr << "Could not initialize tracker plugin!" << std::endl;

		// quit the program
		exit(-1);
	}	
	
	// Open the video. This will not yet start the video stream but will
	// get information about the format of the video which is essential
	// for the connected tracker
	video->open();

	// Initialise the tracker with the dimensions of the video image
	tracker->init(video->getWidth(), video->getHeight());

	// From here on the scene is going to be built

	// Create a camera to handle the projection and modelview matrices.
	// The camera is a group that the rest of the AR scene can be added to.
	osg::ref_ptr<osg::Camera> arCamera = new osg::Camera();
    arCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	arCamera->setProjectionMatrix(osg::Matrix(tracker->getProjectionMatrix()));
    arCamera->setViewMatrix(osg::Matrix::identity());
	arCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// Create a video background and add it to the camera
	osg::ref_ptr<osgART::VideoBackground> videoBackground = new osgART::VideoBackground(video.get());
	videoBackground->setTextureMode(osgART::GenericVideoObject::USE_TEXTURE_RECTANGLE);
	videoBackground->init();
	arCamera->addChild(videoBackground.get());

	// create marker with id number '0'
	osg::ref_ptr<osgART::Marker> marker = tracker->getMarker(0);
		
	// check before accessing the linked marker
	if (!marker.valid()) {
		osg::notify(osg::FATAL) << "No Marker defined!" << std::endl;
		exit(-1);
	}

	// activate the marker
	marker->setActive(true);

	// create a matrix transform related to the marker
	osg::ref_ptr<osg::MatrixTransform> markerTrans = new osgART::ARTTransform(marker.get());

	//and simply create a blue cube object
	float boxSize = 40.0f;
	osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0, 0, boxSize / 2.0f), boxSize));
	sd->setColor(osg::Vec4(0, 0, 1, 1));
	
	osg::Geode* geode = new osg::Geode();
	geode->addDrawable(sd);
	markerTrans->addChild(geode);

	// Assemble our scene and add it to the camera
	osg::Group* sceneGroup = new osg::Group();
	sceneGroup->getOrCreateStateSet()->setRenderBinDetails(5, "RenderBin");
	sceneGroup->addChild(markerTrans.get());
	arCamera->addChild(sceneGroup);
	
	// Put this all under a root node
	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(arCamera.get());
	viewer.setSceneData(root.get());

	viewer.realize();
	
	video->start();
	
    while (!viewer.done()) 
	{
		
		viewer.sync();	
		
		//update the video (get new frame)
		video->update();

		//update the tracker with the new image
		tracker->setImage(video.get());
		tracker->update();
		
        viewer.update();
        viewer.frame();
	
    }
    
	viewer.sync();
    viewer.cleanup_frame();
    viewer.sync();

	//stop the video
	video->stop();
	//close the video
	video->close();
	
}
