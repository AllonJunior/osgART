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

#include <osg/Notify>

#include "osgART/Target"
#include "osgART/Tracker"
#include "osgART/Video"
#include "osgART/Calibration"
#include "osgART/Utils"
#include "osgART/PluginManager"

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osg/io_utils>


class DummyCalibration : public osgART::Calibration
{

public:

	inline DummyCalibration()
	{
	}

	inline ~DummyCalibration()
	{
	}

	inline const DummyCalibration* getCalibration() const
	{
		return this;
	}

	inline bool load(const std::string& filename)
	{
		//specific load of camera parameters that can be used with this
		//tracker

		//in this example we do nothing here

		osg::notify() << "Dummy Calibration load " <<std::endl;

		osg::Matrix dummy_projection_matrix_test;
		//dummy_projection_matrix_test.makePerspective(40.,1.3,1.,10000.);

		dummy_projection_matrix_test.set(
			2, 0, 0, 0,
			0, 2.66667, 0, 0,
			0, 0, -1.0002, -1,
			0, 0, -0.20002, 0);

		_projection.set(dummy_projection_matrix_test);

		//you can also setup the distorsion parameters
		//_distortion.set(val1,val2,val3,val4);

	
		return true;
	}

	inline void setSize(int width, int height)
	{
		osg::notify() << "Dummy Calibration resize '" <<
			width << " x " << height << std::endl;

		//you can adjust the image resolution keeping similar projection matrix
		//you need to modify your camera projection matrix


	}

};


class DummyTracker;


class DummyTarget : public osgART::Target
{

public:

	DummyTarget();

	void init(const std::string& config);

    void update(osg::Matrix mat);

protected:
	virtual ~DummyTarget() { 
		
		//destroy target data 
		; }
};


class DummyTracker : public osgART::Tracker
{

public:

	DummyTracker();
	virtual ~DummyTracker();

	osgART::Calibration* getOrCreateCalibration();

	void setImage(osg::Image* image);

	osgART::Target* addTarget(const std::string& config);

	void update();

	void updateCB(osg::NodeVisitor* nv);

};



/* Implementation */


DummyTarget::DummyTarget()
	: osgART::Target()
{
	//initialize a target
}


void
DummyTarget::init(const std::string& config)
{

	if (config.empty()) {
		osg::notify() << "osgART::DummyTarget::init() Empty config string" << std::endl;
		return;
	}

	//parse the configuration option of the target
	std::vector<std::string> tokens = osgART::tokenize(config,";");

	//check number of required arguments
	///if (tokens.size() < 2) {
	//	OSG_WARN << "Dummy Tracker invalid config string" << std::endl;
	//	return;
	//}

	if (tokens[0] == "id") {

		//do something

		//in this example we do nothing here
	}
}

void
DummyTarget::update(osg::Matrix mat)
{
	osg::notify(osg::DEBUG_INFO) << "osgART::DummyTarget::update()" << std::endl;

	_transform=mat;
	_valid=true;


}



DummyTracker::DummyTracker() :
	osgART::Tracker()
{
	

}

DummyTracker::~DummyTracker()
{
	//clean up the tracker

}

//this call allow to access intrinsics parameters
//for optical tracker
//it's used also for 
inline osgART::Calibration*
DummyTracker::getOrCreateCalibration()
{
	//you have two choices here

	//1. you create a specific calibration class, that can hold
	//more parameters for your tracker
	if (!_calibration.valid()) _calibration = new DummyCalibration;

	//2.you only create a default calibration
	//and you setup the proj matrix
	//_calibration=new osgART::Calibration();
	//_calibration._prjection..etc 
	return osgART::Tracker::getOrCreateCalibration();
}

inline void
DummyTracker::setImage(osg::Image* image)
{

	if (!image) {
		osg::notify() << "osgART::DummyTracker::setImage() called with NULL image" << std::endl;
		return;
	}

	osgART::Tracker::setImage(image);

	this->getOrCreateCalibration()->setSize(*image);

	//you can initialize debug image here

	// Initialise debug image to match video image		
	//m_debugimage->allocateImage(image->s(), image->t(), 1, image->getPixelFormat(), image->getDataType());


}

inline osgART::Target*
DummyTracker::addTarget(const std::string& config)
{
	//create a new targets
	DummyTarget* target = new DummyTarget();
	target->init(config);

	//you can also have different type of targets such as
	//multitarget, imagetarget, etc
	//and use contentof the config string to define them

	//add it to the list
	_targetlist.push_back(target);

	return target;
}

inline void
DummyTracker::update()
{

}

inline void
DummyTracker::updateCB(osg::NodeVisitor* nv)
{
	//check first if we get a new image to update
	//the tracking value
	if (_imagesource->valid() && 
		_imagesource->getModifiedCount() != _modifiedCount && 
		_imagesource->data() != NULL) {

		osg::notify(osg::DEBUG_INFO) << "osgART::DummyTracker::update()" << std::endl;

		osg::Timer t;

		t.setStartTick();

		{
			//lock the video image
			//OpenThreads::ScopedLock<OpenThreads::Mutex> _lock(_imagesource->getMutex());

			//potentially buffer it to a temporary data structure
			//and update your tracker

			//my_external_code->update(_imagesource->data());

		}

		//once it's done, you can update your specific targets with processed information

        for (osgART::Tracker::TargetList::iterator iter = _targetlist.begin();
            iter != _targetlist.end();
            iter++)
        {

             if (DummyTarget* target = dynamic_cast<DummyTarget*>((*iter).get())) {

				 //for this example we create a dummy transformation
				 //and pass that as argument
				 osg::Matrix dummy_mat_for_test;


				 dummy_mat_for_test.identity();
				 dummy_mat_for_test.set(
					 -0.931019,-0.155908,0.329994,0,
					  0.224063,0.46955, 0.854,0,
				     -0.288095,0.86903,-0.40223,0,
					  3.29111,4.70607,-74.1661,1);
			    // dummy_mat_for_test.translate(0.,0.,1.);
                 target->update(dummy_mat_for_test);

              }
		}
		//update the counter of the processed image
		_modifiedCount = _imagesource->getModifiedCount();
	}
}


// initializer for dynamic loading
osgART::PluginProxy<DummyTracker> g_dummytracker("osgart_tracker_dummytracker");


