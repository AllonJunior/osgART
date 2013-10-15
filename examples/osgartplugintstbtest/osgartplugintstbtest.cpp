/* -*-c++-*- 
 * 
 * osgART - Augmented Reality ToolKit for OpenSceneGraph
 * 
 * Copyright (C) 2005-2009 Human Interface Technology Laboratory New Zealand
 * Copyright (C) 2010-2013 Raphael Grasset, Julian Looser, Hartmut Seichter
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the osgart.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

// A simple example to demonstrate the StbTracker plugin with osgART

#include <osgART/Foundation>
#include <osgART/VideoLayer>
#include <osgART/PluginManager>
#include <osgART/VideoGeode>
#include <osgART/Utils>
#include <osgART/GeometryUtils>
#include <osgART/TargetCallback>
#include <osgART/TransformFilterCallback>
#include <osgART/ImageStreamCallback>
#include <osgART/VideoUtils>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <iostream>
#include <sstream>

int main(int argc, char* argv[])
{

	osg::ArgumentParser args(&argc,argv);

	std::string videoName = "sstt";
	std::string trackerName = "stbnx";

	//std::string trackerConfig = "Frame,0,80"; // can use ID or Frame
	//std::string trackerConfig = "single,soccerSet,soccer,hyper_FCBarcelona";
	std::string trackerConfig = "single,multiset,multiset,target_vienna3";

	//std::string trackerNameFeature = "stbnx";
	std::string trackerNameFeature = "stbnx_nft2";


	while(args.read("--video",videoName)) {}
	while(args.read("--tracker",trackerName,trackerConfig)) {}
	while(args.read("--tracker_config",trackerConfig)) {}

	osg::ref_ptr<osg::Group> root = new osg::Group;
	osgViewer::Viewer viewer;

	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.setSceneData(root.get());

	// just for debugging we need a single window
	viewer.setUpViewInWindow(50,50,640,480);

	viewer.realize();

	osgViewer::Viewer::Windows windows;
	viewer.getWindows(windows);

	for (osgViewer::Viewer::Windows::iterator wi = windows.begin(); wi != windows.end(); ++wi)
	{
		(*wi)->setWindowName("osgART - StbNX demo");
	}


	// preload the video and tracker
	osgART::PluginManager::instance()->load("osgart_video_" + videoName);
	osgART::PluginManager::instance()->load("osgart_tracker_stbnx");

	// Load a video plugin.
	osg::ref_ptr<osgART::Video> video =
		dynamic_cast<osgART::Video*>(osgART::PluginManager::instance()->get("osgart_video_" + videoName));


	// check if an instance of the video stream could be started
	if (!video.valid())
	{
		// Without video an AR application can not work. Quit if none found.
		osg::notify(osg::FATAL) << "Could not initialize video plugin!" << std::endl;
		exit(-1);
	}


	OSG_NOTICE << "Now opening the video" << std::endl;

	// Open the video. This will not yet start the video stream but will
	// get information about the format of the video which is essential
	// for the connected tracker
	video->open();


	osg::ref_ptr<osgART::Tracker> tracker =
		dynamic_cast<osgART::Tracker*>(osgART::PluginManager::instance()->get("osgart_tracker_" + trackerNameFeature));


	if (!tracker.valid())
	{
		// Without tracker an AR application can not work. Quit if none found.
		OSG_FATAL << "Could not initialize tracker plugin!" << std::endl;
		exit(-1);
	}

	// get the tracker calibration object
	osg::ref_ptr<osgART::Calibration> calibration = tracker->getOrCreateCalibration();
	calibration->load("data/DefaultCalibration.cal");

	tracker->setImage(video.get());

	osgART::TrackerCallback::addOrSet(root.get(),tracker.get());

	if (osg::ImageStream* imagestream = dynamic_cast<osg::ImageStream*>(video.get()))
	{
		osgART::addEventCallback(root.get(), new osgART::ImageStreamCallback(imagestream));
	}

	osg::ref_ptr<osg::Camera> cam = calibration->createCamera();
	root->addChild(cam.get());


	osg::ref_ptr<osgART::Target> marker = tracker->addTarget(trackerConfig);
	marker->setActive(true);

	osg::ref_ptr<osg::MatrixTransform> arTransform = new osg::MatrixTransform();
	osgART::attachDefaultEventCallbacks(arTransform.get(), marker.get());

	// generate random colour
	osg::Vec4 c = osg::Vec4((double)rand() / (double)RAND_MAX, (double)rand() / (double)RAND_MAX, (double)rand() / (double)RAND_MAX, 1.0f);
	// create a cube
	arTransform->addChild(osgART::testCube(20,c));
	
	// need a higher renderbin to be on top of the video background
	arTransform->getOrCreateStateSet()->setRenderBinDetails(100, "RenderBin");

	cam->addChild(arTransform.get());

	osg::ref_ptr<osg::Group> videoBackground = osgART::createImageBackground(video.get());
	videoBackground->getOrCreateStateSet()->setRenderBinDetails(0, "RenderBin");
	cam->addChild(videoBackground.get());

	video->start();
	return viewer.run();

}
