/* -*-c++-*-
 *
 * osgART - AR for OpenSceneGraph
 * Copyright (C) 2005-2009 Human Interface Technology Laboratory New Zealand
 * Copyright (C) 2009-2013 osgART Development Team
 *
 * This file is part of osgART
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


#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgDB/FileUtils>

#include <osgART/Scene>
#include <osgART/GeometryUtils>

#include <osgART/PluginManager>


int main(int argc, char* argv[])  {

	osgViewer::Viewer viewer;

	// add relevant handlers to the viewer
	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.addEventHandler(new osgViewer::ThreadingHandler);
	viewer.addEventHandler(new osgViewer::HelpHandler);


	osgART::PluginManager::instance()->load("osgart_video_dummyvideo");
	osgART::PluginManager::instance()->load("osgart_tracker_dummytracker");


	osgART::Scene* scene = new osgART::Scene();

	scene->addVideoBackground("osgart_video_dummyvideo","Data/dummyvideo/dummyvideo.png");
	scene->addTracker("osgart_tracker_dummytracker","","mode=0;");
	scene->addTrackedTransform("test.pattern;35.2;22.0;0.3")->addChild(osgART::testCube(20));
	
	//or for being able to further add/modify the target transformation:
	//osg::ref_ptr<osg::MatrixTransform> mt = scene->addTrackedTransform("test.pattern;35.2;22.0;0.3");
	//mt->addChild(osgART::testCube(20));

	viewer.setSceneData(scene);

	//run call is equivalent to a while loop with a viewer.frame call
	return viewer.run();
	
}
