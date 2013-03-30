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

#include <osg/MatrixTransform>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <osgART/Scene>
#include <osgART/PluginManager>
#include <osgART/GeometryUtils>


class KeyboardEventHandler : public osgGA::GUIEventHandler {
 
protected:
	osg::MatrixTransform* _driveCar;

public:
    KeyboardEventHandler(osg::MatrixTransform* drivecar) : osgGA::GUIEventHandler() {_driveCar=drivecar; }      
 
 
    /**
        OVERRIDE THE HANDLE METHOD:
        The handle() method should return true if the event has been dealt with
        and we do not wish it to be handled by any other handler we may also have
        defined. Whether you return true or false depends on the behaviour you 
        want - here we have no other handlers defined so return true.
    **/
    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, 
                        osg::Object* obj, osg::NodeVisitor* nv) { 
 
        switch (ea.getEventType()) {
 
 
            /** KEY EVENTS:
                Key events have an associated key and event names.
                In this example, we are interested in keys up/down/right/left arrow
                and space bar.
                If we detect a press then we modify the transformation matrix 
                of the local transform node. **/
            case osgGA::GUIEventAdapter::KEYDOWN: {
 
                switch (ea.getKey()) {
 
                    case osgGA::GUIEventAdapter::KEY_Up: // Move forward 5mm
                        _driveCar->preMult(osg::Matrix::translate(0, -5, 0));
                        return true;
 
                    case osgGA::GUIEventAdapter::KEY_Down: // Move back 5mm
                        _driveCar->preMult(osg::Matrix::translate(0, 5, 0));
                        return true; 
 
                    case osgGA::GUIEventAdapter::KEY_Left: // Rotate 10 degrees left
                        _driveCar->preMult(osg::Matrix::rotate(osg::DegreesToRadians(10.0f),  
                                                       osg::Z_AXIS));
                        return true;
 
                    case osgGA::GUIEventAdapter::KEY_Right: // Rotate 10 degrees right
                        _driveCar->preMult(osg::Matrix::rotate(osg::DegreesToRadians(-10.0f),  
                                                       osg::Z_AXIS));
                        return true;
 
                    case ' ': // Reset the transformation
                       _driveCar->setMatrix(osg::Matrix::identity());
                        return true;
                }
 
                   default: return false;
            }
 
 
        }
    }
};
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

	//add a video background (video plugin name, video configuration)
	scene->addVideoBackground("osgart_video_dummyvideo","Data/dummyvideo/dummyvideo.png");
	//add a tracker (tracker plugin name,calibration configuration, tracker configuration)
	scene->addTracker("osgart_tracker_dummytracker","","mode=0;");

	osg::ref_ptr<osg::MatrixTransform> mt = scene->addTrackedTransform("test.pattern;35.2;22.0;0.3");
	
	osg::ref_ptr<osg::MatrixTransform> driveCar = new osg::MatrixTransform();
	driveCar->addChild(osgART::scaleModel(osgDB::readNodeFile("media/models/car.ive"),0.5));
	mt->addChild(driveCar.get());

	//add our keyboard event handler
	viewer.addEventHandler(new KeyboardEventHandler(driveCar)); 

	//APPLICATION INIT

	//BOOTSTRAP INIT

	viewer.setSceneData(scene);

	//MAIN LOOP & EXIT CLEANUP

	//run call is equivalent to a while loop with a viewer.frame call
	return viewer.run();
}
