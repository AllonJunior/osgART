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

// std include

// OpenThreads include

// OSG include
#include <osg/MatrixTransform>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

// osgART include
#include <osgART/PluginManager>
#include <osgART/Scene>
#include <osgART/GeometryUtils>

// local include



int main(int argc, char* argv[])  {

	//ARGUMENTS INIT

	//VIEWER INIT

	//create a default viewer
	osgViewer::Viewer viewer;

	//setup default threading mode
	viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

	// add relevant handlers to the viewer
	viewer.addEventHandler(new osgViewer::StatsHandler);//stats, press 's'
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);//resize, fullscreen 'f'
	viewer.addEventHandler(new osgViewer::ThreadingHandler);//threading mode, press 't'
	viewer.addEventHandler(new osgViewer::HelpHandler);//help menu, press 'h'

	//AR SCENEGRAPH INIT

	osgART::Scene* scene = new osgART::Scene();

    scene->addVideo("osgart_artoolkit2","osgart_video_artoolkit");
    scene->addVisualTracker("osgart_artoolkit2","osgart_tracker_artoolkit","data/artoolkit2/camera_para.dat");
    scene->addTrackedTransform("single;data/artoolkit2/patt.hiro;80;0;0")->addChild(osgART::createTopCube(80));

	//or for being able to further add/modify the target transformation:
	//osg::ref_ptr<osg::MatrixTransform> mt = scene->addTrackedTransform("single;data/artoolkit/patt.hiro;80;0;0");
	//mt->addChild(osgART::createTopCube(80));


	//APPLICATION INIT

	//BOOTSTRAP INIT

	viewer.setSceneData(scene);


	//MAIN LOOP & EXIT CLEANUP

	//run call is equivalent to a while loop with a viewer.frame call
	return viewer.run();
	
}
