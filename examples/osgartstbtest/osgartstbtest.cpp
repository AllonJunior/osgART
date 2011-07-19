/* -*-c++-*- 
 * 
 * osgART - ARToolKit for OpenSceneGraph
 * Copyright (C) 2005-2008 Human Interface Technology Laboratory New Zealand
 * 
 * This file is part of osgART 2.0
 *
 * osgART 2.0 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * osgART 2.0 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with osgART 2.0.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// A simple example to demonstrate the StbTracker plugin with osgART

#include <osgART/Foundation>
#include <osgART/VideoLayer>
#include <osgART/PluginManager>
#include <osgART/VideoGeode>
#include <osgART/Utils>
#include <osgART/GeometryUtils>
#include <osgART/MarkerCallback>
#include <osgART/TransformFilterCallback>
#include <osgART/ImageStreamCallback>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <iostream>
#include <sstream>

osg::Group* createImageBackground(osg::Image* video, bool useTextureRectangle = false) {
	osgART::VideoLayer* _layer = new osgART::VideoLayer();
	osgART::VideoGeode* _geode = new osgART::VideoGeode(video, NULL, 1, 1, 20, 20, 
		useTextureRectangle ? osgART::VideoGeode::USE_TEXTURE_RECTANGLE : osgART::VideoGeode::USE_TEXTURE_2D);
	_layer->addChild(_geode);
	return _layer;
}

int main(int argc, char* argv[])  
{
	
	osg::ArgumentParser args(&argc,argv);

	std::string videoName = "sstt";

	while(args.read("--video",videoName)) {}

	osg::ref_ptr<osg::Group> root = new osg::Group;
	osgViewer::Viewer viewer;
	
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.setSceneData(root.get());
	
	viewer.realize();
	
	osgViewer::Viewer::Windows windows;
	viewer.getWindows(windows);
	
	for (osgViewer::Viewer::Windows::iterator wi = windows.begin(); wi != windows.end(); ++wi)
	{
		(*wi)->setWindowName("osgART - StbNX demo");
	}

	// preload the video and tracker
	int _video_id = osgART::PluginManager::instance()->load("osgart_video_" + videoName);
	int _tracker_id = osgART::PluginManager::instance()->load("osgart_tracker_stbnx");

	// Load a video plugin.
	osg::ref_ptr<osgART::Video> video = 
		dynamic_cast<osgART::Video*>(osgART::PluginManager::instance()->get(_video_id));

	// check if an instance of the video stream could be started
	if (!video.valid()) 
	{   
		// Without video an AR application can not work. Quit if none found.
		osg::notify(osg::FATAL) << "Could not initialize video plugin!" << std::endl;
		exit(-1);
	}
	

	// Open the video. This will not yet start the video stream but will
	// get information about the format of the video which is essential
	// for the connected tracker
	video->open();

	osg::ref_ptr<osgART::Tracker> tracker = 
		dynamic_cast<osgART::Tracker*>(osgART::PluginManager::instance()->get(_tracker_id));

	if (!tracker.valid()) 
	{
		// Without tracker an AR application can not work. Quit if none found.
		osg::notify(osg::FATAL) << "Could not initialize tracker plugin!" << std::endl;
		exit(-1);
	}

	// get the tracker calibration object
	osg::ref_ptr<osgART::Calibration> calibration = tracker->getOrCreateCalibration();
	calibration->load("data/WebCam.cal");

	tracker->setImage(video.get());

	osgART::TrackerCallback::addOrSet(root.get(),tracker.get());
	
	if (osg::ImageStream* imagestream = dynamic_cast<osg::ImageStream*>(video.get())) {
		osgART::addEventCallback(root.get(), new osgART::ImageStreamCallback(imagestream));
	}

	osg::ref_ptr<osg::Camera> cam = calibration->createCamera();
	root->addChild(cam.get());

	for (int i = 0; i < 32; i++) {

		std::stringstream ss;
		//ss << "ID;" << i << ";20";
		ss << "Frame;" << i << ";80";

		osg::ref_ptr<osgART::Marker> marker = tracker->addMarker(ss.str());
		marker->setActive(true);

		osg::ref_ptr<osg::MatrixTransform> arTransform = new osg::MatrixTransform();
		osgART::attachDefaultEventCallbacks(arTransform.get(), marker.get());
		
		osg::Vec4 c = osg::Vec4((double)rand() / (double)RAND_MAX, (double)rand() / (double)RAND_MAX, (double)rand() / (double)RAND_MAX, 1.0f);
		arTransform->addChild(osgART::testCube(20, c));
		arTransform->getOrCreateStateSet()->setRenderBinDetails(100, "RenderBin");
		cam->addChild(arTransform.get());

	}

	osg::ref_ptr<osg::Group> videoBackground = createImageBackground(video.get());
	videoBackground->getOrCreateStateSet()->setRenderBinDetails(0, "RenderBin");
	cam->addChild(videoBackground.get());
	
	video->start();
	return viewer.run();
	
}
