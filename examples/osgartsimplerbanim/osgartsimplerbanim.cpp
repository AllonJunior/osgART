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

#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgDB/FileUtils>

#include <osg/MatrixTransform>

#include <osgART/Scene>
#include <osgART/GeometryUtils>

#include <osgART/PluginManager>

osg::AnimationPath* carAnimationPath()
{
	osg::AnimationPath* path = new osg::AnimationPath();
	path->setLoopMode(osg::AnimationPath::LOOP);

	/** point 1 - start point **/
	path->insert(0.0, osg::AnimationPath::ControlPoint(osg::Vec3d(-15.0,-15.0,0.0),
		osg::Quat(osg::inDegrees(0.0f),osg::Z_AXIS)));
	path->insert(0.2, osg::AnimationPath::ControlPoint(osg::Vec3d(-15.0,-15.0,0.0),
		osg::Quat(osg::inDegrees(90.0f),osg::Z_AXIS)));
	/** point 2 **/
	path->insert(1.2, osg::AnimationPath::ControlPoint(osg::Vec3d(15.0,-15.0,0.0),
		osg::Quat(osg::inDegrees(90.0f),osg::Z_AXIS)));
	path->insert(1.7, osg::AnimationPath::ControlPoint(osg::Vec3d(15.0,-15.0,0.0),
		osg::Quat(osg::inDegrees(180.0f),osg::Z_AXIS)));

	/** point 3 **/
	path->insert(2.7, osg::AnimationPath::ControlPoint(osg::Vec3d(15.0,15.0,0.0),
		osg::Quat(osg::inDegrees(180.0f),osg::Z_AXIS)));
	path->insert(2.9, osg::AnimationPath::ControlPoint(osg::Vec3d(15.0,15.0,0.0),
		osg::Quat(osg::inDegrees(270.0f),osg::Z_AXIS)));

	/** point 4 **/
	path->insert(3.9, osg::AnimationPath::ControlPoint(osg::Vec3d(-15.0,15.0,0.0),
		osg::Quat(osg::inDegrees(270.0f),osg::Z_AXIS)));
	path->insert(4.1, osg::AnimationPath::ControlPoint(osg::Vec3d(-15.0,15.0,0.0),
		osg::Quat(osg::inDegrees(360.0f),osg::Z_AXIS)));

	/** back to point 1 **/
	path->insert(5.1, osg::AnimationPath::ControlPoint(osg::Vec3d(-15.0,-15.0,0.0),
		osg::Quat(osg::inDegrees(360.0f),osg::Z_AXIS)));

	return path;
}

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

	//AR INIT

	//AR SCENEGRAPH INIT

	//create an osgART::Scene
	osgART::Scene* scene = new osgART::Scene();


	scene->addVideoBackground("osgart_video_dummyvideo","Data/dummyvideo/dummyvideo.png");
	scene->addTracker("osgart_tracker_dummytracker","","mode=0;");

	osg::MatrixTransform* mt = scene->addTrackedTransform("test.pattern;35.2;22.0;0.3");
	
	osg::ref_ptr<osg::MatrixTransform> hitlabMT = new osg::MatrixTransform();
	hitlabMT->addChild(osgART::scaleModel(osgDB::readNodeFile("media/models/hitl_logo.osg"),0.5));
	hitlabMT->addUpdateCallback(new osg::AnimationPathCallback(osg::Vec3(0.0,0.0,0.0), 
		osg::Z_AXIS, 
		osg::inDegrees(45.0f)));
	mt->addChild(hitlabMT.get());

	osg::ref_ptr<osg::MatrixTransform> carMT = new osg::MatrixTransform();
	carMT->addChild(osgART::scaleModel(osgDB::readNodeFile("media/models/car.ive"),0.5));
	carMT->addUpdateCallback(new osg::AnimationPathCallback(carAnimationPath()));
	mt->addChild(carMT.get());

	viewer.setSceneData(scene);

	//run call is equivalent to a while loop with a viewer.frame call
	return viewer.run();
	
}
