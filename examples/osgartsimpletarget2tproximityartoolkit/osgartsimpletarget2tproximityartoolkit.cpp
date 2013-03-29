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

class TargetProximityUpdateCallback : public osg::NodeCallback {
 
	private:
		osg::MatrixTransform* mtA;
		osg::MatrixTransform* mtB;
 
		osg::Switch* mSwitchA;
		osg::Switch* mSwitchB;

		osgART::Scene* mScene;
 
		float mThreshold;
 
	public:
 
		TargetProximityUpdateCallback(
		osg::MatrixTransform* mA, osg::MatrixTransform* mB, 
			osg::Switch* switchA, osg::Switch* switchB,
			float threshold,osgART::Scene* scene) : 
			                     osg::NodeCallback(), 
					    mtA(mA), mtB(mB),
					    mSwitchA(switchA), mSwitchB(switchB),
					    mThreshold(threshold),mScene(scene) { }
 
 
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) {
 
			/** CALCULATE INTER-TARGET PROXIMITY:
				Here we obtain the current position of each target, and the
				distance between them by examining
				the translation components of their parent transformation 
				matrices **/
			osg::Vec3 posA = mtA->getMatrix().getTrans();
			osg::Vec3 posB = mtB->getMatrix().getTrans();
			osg::Vec3 offset = posA - posB;
			float distance = offset.length();
			mScene->setUpdateCallback(new TargetProximityUpdateCallback(mtA, mtB, mSwitchA,
																	    mSwitchB, 200.0f,mScene)); 
 
			/** LOAD APPROPRIATE MODELS:
				Here we use each target's OSG Switch node to swap between
				models, depending on the inter-target distance we have just 
				calculated. **/
			if (distance <= mThreshold) {
				if (mSwitchA->getNumChildren() > 1) mSwitchA->setSingleChildOn(1);
				if (mSwitchB->getNumChildren() > 1) mSwitchB->setSingleChildOn(1);
			} else {
				if (mSwitchA->getNumChildren() > 0) mSwitchA->setSingleChildOn(0);
				if (mSwitchB->getNumChildren() > 0) mSwitchB->setSingleChildOn(0);
			}
 
 
			traverse(node,nv);
 
		}
};




int main(int argc, char* argv[])  {

	osgViewer::Viewer viewer;

	// add relevant handlers to the viewer
	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.addEventHandler(new osgViewer::ThreadingHandler);
	viewer.addEventHandler(new osgViewer::HelpHandler);


	osgART::PluginManager::instance()->load("osgart_video_artoolkit2");
	osgART::PluginManager::instance()->load("osgart_tracker_artoolkit2");


	osgART::Scene* scene = new osgART::Scene();

	scene->addVideoBackground("osgart_video_artoolkit2");
	scene->addTracker("osgart_tracker_artoolkit2","data/artoolkit2/camera_para.dat");
	
	osg::MatrixTransform* mtA = scene->addTrackedTransform("single;data/artoolkit2/patt.hiro;80;0;0");
	osg::MatrixTransform* mtB = scene->addTrackedTransform("single;data/artoolkit2/patt.kanji;80;0;0");

	osg::ref_ptr<osg::Switch> switchA = new osg::Switch();
	switchA->addChild(osgDB::readNodeFile("media/models/voltmeter_low.osg"), true);
	switchA->addChild(osgDB::readNodeFile("media/models/voltmeter_high.osg"), false);
	mtA->addChild(switchA.get());

	osg::ref_ptr<osg::Switch> switchB = new osg::Switch();
	switchB->addChild(osgDB::readNodeFile("media/models/battery.osg"), true);
	switchB->addChild(osgDB::readNodeFile("media/models/battery_spark.osg"), false);
	mtB->addChild(switchB.get());

	scene->setUpdateCallback(new TargetProximityUpdateCallback(mtA, mtB, 
		switchA.get(), switchB.get(), 200.0f,scene));

	viewer.setSceneData(scene);

	return viewer.run();
	
}
