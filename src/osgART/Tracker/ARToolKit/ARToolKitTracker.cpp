/*
 *	osgART/Tracker/ARToolKit/ARToolKitTracker
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

#include "ARToolKitTracker"


#include <osgDB/ReadFile>
#include <AR/config.h>
#include <AR/video.h>
#include <AR/ar.h>
#include <AR/gsub_lite.h>
#ifndef AR_HAVE_HEADER_VERSION_2_72
#error ARToolKit v2.72 or later is required to build the OSGART ARToolKit tracker.
#endif

#include "SingleMarker"
#include "MultiMarker"

#include <osgART/GenericVideo>
#include <osg/Notify>

#include <iostream>
#include <fstream>


#define PD_LOOP 3

// Make sure that required OpenGL constant definitions are available at compile-time.
// N.B. These should not be used unless the renderer indicates (at run-time) that it supports them.
// Define constants for extensions (not yet core).
#ifndef GL_APPLE_ycbcr_422
#  define GL_YCBCR_422_APPLE				0x85B9
#  define GL_UNSIGNED_SHORT_8_8_APPLE		0x85BA
#  define GL_UNSIGNED_SHORT_8_8_REV_APPLE	0x85BB
#endif
#ifndef GL_EXT_abgr
#  define GL_ABGR_EXT						0x8000
#endif
#ifndef GL_MESA_ycbcr_texture
#  define GL_YCBCR_MESA						0x8757
#  define GL_UNSIGNED_SHORT_8_8_MESA		0x85BA
#  define GL_UNSIGNED_SHORT_8_8_REV_MESA	0x85BB
#endif

template <typename T> 
int Observer2Ideal(	const T dist_factor[4], 
					const T ox, 
					const T oy,
					T *ix, T *iy )
{
    T  z02, z0, p, q, z, px, py;
    register int i = 0;

    px = ox - dist_factor[0];
    py = oy - dist_factor[1];
    p = dist_factor[2]/100000000.0;
    z02 = px*px+ py*py;
    q = z0 = sqrt(px*px+ py*py);

    for( i = 1; ; i++ ) {
        if( z0 != 0.0 ) {
            z = z0 - ((1.0 - p*z02)*z0 - q) / (1.0 - 3.0*p*z02);
            px = px * z / z0;
            py = py * z / z0;
        }
        else {
            px = 0.0;
            py = 0.0;
            break;
        }
        if( i == PD_LOOP ) break;

        z02 = px*px+ py*py;
        z0 = sqrt(px*px+ py*py);
    }

    *ix = px / dist_factor[3] + dist_factor[0];
    *iy = py / dist_factor[3] + dist_factor[1];

    return(0);
}


namespace osgART {

	struct ARToolKitTracker::CameraParameter 
	{
		ARParam cparam;	
	};

	ARToolKitTracker::ARToolKitTracker() : GenericTracker(),
		m_debugimage(new osg::Image),
		m_threshold(100),
		m_marker_num(0),
		m_cparam(new CameraParameter)
	{
		// Assign version and name of the tracker.
		m_name		= "ARToolKit";
		char *version;
		arGetVersion(&version);
		if (version) {
			m_version = version;
			free(version);
		}
		
		// create a new field 
		m_fields["threshold"] = new CallbackField<ARToolKitTracker,int>(this,
			&ARToolKitTracker::getThreshold,
			&ARToolKitTracker::setThreshold);

		// create a field for the debug image
		m_fields["debug_image"] = new TypedField<osg::ref_ptr<osg::Image> >(&m_debugimage);
		
		// attach a new field to the name "debug"
		m_fields["debug"] = new CallbackField<ARToolKitTracker,bool>
			(this,
			&ARToolKitTracker::getDebugMode,
			&ARToolKitTracker::setDebugMode);

		// for statistics
		m_fields["markercount"] = new TypedField<int>(&m_marker_num);
	}

	ARToolKitTracker::~ARToolKitTracker()
	{
		osg::notify() << "ARToolKitTracker::~ARToolKitTracker()"
			<< std::endl;
		try {

			delete this->m_cparam;
		}
		catch (...)
		{
			osg::notify() << "ARToolKitTracker::~ARToolKitTracker() D'tor failed to delete"
				<< std::endl;
		}
	}


	bool ARToolKitTracker::init(int xsize, int ysize, 
		const std::string& pattlist_name, 
		const std::string& camera_name)
	{
		ARParam  wparam;


	    // Set the initial camera parameters.
		cparamName = camera_name;
	    if(arParamLoad((char*)cparamName.c_str(), 1, &wparam) < 0) {
			
			// 
			osg::notify(osg::FATAL) 
				<< "osgART::ARToolKitTracker::init : Error: Can't load camera parameters from '"<<
				camera_name <<"'." << std::endl;
			return false;
	    }

	    arParamChangeSize(&wparam, xsize, ysize,&(m_cparam->cparam));
	    arInitCparam(&(m_cparam->cparam));
	    arParamDisp(&(m_cparam->cparam));

		arFittingMode = AR_FITTING_TO_IDEAL;
	    arImageProcMode = AR_IMAGE_PROC_IN_FULL;

		setProjection(10.0f, 8000.0f);
		setThreshold(m_threshold);

		if (!setupMarkers(pattlist_name)) {
			std::cerr << "ERROR: Marker setup failed." << std::endl;
			return false;
		}

		// Success
		return true;
	}


	std::string trim(std::string& s,const std::string& drop = " ")
	{
		std::string r=s.erase(s.find_last_not_of(drop)+1);
		return r.erase(0,r.find_first_not_of(drop));
	}


	bool ARToolKitTracker::setupMarkers(const std::string& patternListFile)
	{
		std::ifstream markerFile;

		// Need to check whether the passed file even exists

		markerFile.open(patternListFile.c_str());

		// Need to check for error when opening file
		if (!markerFile.is_open()) return false;

		bool ret = true;

		int patternNum = 0;
		markerFile >> patternNum;
		//std::cout << "Loading " << patternNum << " patterns." << std::endl;

		std::string patternName, patternType;

		// Need EOF checking in here... atm it assumes there are really as many markers as the number says

		for (int i = 0; (i < patternNum) && (!markerFile.eof()); i++)
		{
			// jcl64: Get the whole line for the marker file (will handle spaces in filename)
			patternName = "";
			while (trim(patternName) == "" && !markerFile.eof()) {
				getline(markerFile, patternName);
			}
			
			
			// Check whether markerFile exists?

			markerFile >> patternType;

			if (patternType == "SINGLE")
			{
				
				double width, center[2];
				markerFile >> width >> center[0] >> center[1];
				if (addSingleMarker(patternName, width, center) == -1) {
					std::cerr << "Error adding single pattern: " << patternName << std::endl;
					ret = false;
					break;
				}

			}
			else if (patternType == "MULTI")
			{
				if (addMultiMarker(patternName) == -1) {
					std::cerr << "Error adding multi-marker pattern: " << patternName << std::endl;
					ret = false;
					break;
				}

			} 
			else 
			{
				std::cerr << "Unrecognized pattern type: " << patternType << std::endl;
				ret = false;
				break;
			}
		}

		markerFile.close();

		return ret;
	}

	int 
	ARToolKitTracker::addSingleMarker(const std::string& pattFile, double width, double center[2]) {

		SingleMarker* singleMarker = new SingleMarker();

		if (!singleMarker->initialise(pattFile, width, center))
		{
			singleMarker->unref();
			return -1;
		}

		m_markerlist.push_back(singleMarker);

		return m_markerlist.size() - 1;
	}

	int 
	ARToolKitTracker::addMultiMarker(const std::string& multiFile) 
	{
		MultiMarker* multiMarker = new MultiMarker();
		
		if (!multiMarker->initialise(multiFile))
		{
			multiMarker->unref();
			return -1;
		}

		m_markerlist.push_back(multiMarker);

		return m_markerlist.size() - 1;

	}

	void ARToolKitTracker::setThreshold(const int& thresh)	
	{
		m_threshold = osg::clampBetween(thresh,0,255);		
	}

	int ARToolKitTracker::getThreshold() const 
	{
		return m_threshold;
	}


	// simple function for getting the image format 
	// \TODO: Phil ... you might want to put the "inverse" of your
	// original function in here
	unsigned int getARToolKitFormat(const osg::Image& _image)
	{
		switch (_image.getPixelFormat())
		{
		case GL_BGRA:
            return AR_PIXEL_FORMAT_BGRA;
		default:
			osg::notify(osg::WARN) << "Not implemented!" << std::endl;
		}

		return 0;
	}


	void ARToolKitTracker::update()
	{

		ARMarkerInfo    *marker_info;					// Pointer to array holding the details of detected markers.
		
	    register int             j, k;

		if (!m_imagesource.valid())
		{
			osg::notify(osg::WARN) << "No connected image source for the tracker" << std::endl;
			return;
		}

		// Do not update with a null image.
		if (!m_imagesource->valid())
		{
			osg::notify(osg::WARN) << "osgart_artoolkit_tracker: received NULL pointer as image"
				<< std::endl;
			return;
		}

		// hse25: performance measurement: only update if the image was modified
		if (m_imagesource->getModifiedCount() == m_lastModifiedCount)
		{
			return; 
		}
		
		m_lastModifiedCount = m_imagesource->getModifiedCount();

		
		std::cout << "gahhh!" << std::endl;

		// \TODO: hse25: check here for the moment, the function needs to be extended
		if (AR_DEFAULT_PIXEL_FORMAT != getARToolKitFormat(*m_imagesource.get()))
		{
			osg::notify(osg::WARN) << "osgart_artoolkit_tracker::update() Incompatible pixelformat!" << std::endl;
			return;
		}

		std::cout << "Components: " <<
			m_imagesource->data() << ", " <<
			m_imagesource->data()+1 << ", " <<
			m_imagesource->data()+2 << ", " <<
			m_imagesource->data()+3 << ", " << std::endl;

		// Debug Image.
		if (arDebug) {
			GLenum internalformat_GL;
			GLenum format_GL;
			GLenum type_GL;
			getGLPixelFormatForARPixelFormat(m_artoolkit_pixformat, &internalformat_GL, &format_GL, &type_GL);
			if (!m_debugimage->valid()) {
				osg::notify() << "ARToolKitTracker::init() Create Debug Image: " << m_cparam->cparam.xsize << " x " << m_cparam->cparam.ysize  << std::endl;
				m_debugimage->allocateImage(m_cparam->cparam.xsize, m_cparam->cparam.ysize, 1, format_GL, type_GL, 1);
			} 
		
			m_debugimage->setImage(m_debugimage->s(), m_debugimage->t(), 
					1, internalformat_GL, format_GL, type_GL, arImage, 
					osg::Image::NO_DELETE, 1);
		}


		// Detect the markers in the video frame.
		if (arDetectMarker(m_imagesource->data(), m_threshold, &marker_info, &m_marker_num) < 0) {
			osg::notify(osg::FATAL) << "Error detecting markers in image." << std::endl;
			return;
		}

		MarkerList::iterator _end = m_markerlist.end();
			
		// Check through the marker_info array for highest confidence
		// visible marker matching our preferred pattern.
		for (MarkerList::iterator iter = m_markerlist.begin(); 
			iter != _end; 
			++iter)		
		{

			SingleMarker* singleMarker = dynamic_cast<SingleMarker*>((*iter).get());
			MultiMarker* multiMarker = dynamic_cast<MultiMarker*>((*iter).get());

			if (singleMarker)
			{			

				k = -1;
				for (j = 0; j < m_marker_num; j++)	
				{
					if (singleMarker->getPatternID() == marker_info[j].id) 
					{
						if (k == -1) k = j; // First marker detected.
						else 
						if(marker_info[j].cf > marker_info[k].cf) k = j; // Higher confidence marker detected.

					}
				}
					
				if(k != -1) 
				{
					singleMarker->update(&marker_info[k]); 
				} 
				else 
				{
					singleMarker->update(NULL);
				}
			}
			else if (multiMarker)
			{
				multiMarker->update(marker_info, m_marker_num);
				
			} else {
				std::cerr << "ARToolKitTracker::update() : Unknown marker type id!" << std::endl;
			}
		}
// #endif

	}

	void ARToolKitTracker::setDebugMode(const bool& b)
	{
		arDebug = (int)b;
	}
	
	bool ARToolKitTracker::getDebugMode() const
	{
		return (arDebug == 1);
	}
	
	void ARToolKitTracker::setProjection(const double n, const double f) 
	{
		arglCameraFrustumRH(&(m_cparam->cparam), n, f, m_projectionMatrix);
	}

	void ARToolKitTracker::createUndistortedMesh(
		int width, int height,
		float maxU, float maxV,
		osg::Geometry &geometry)
	{

		osg::Vec3Array *coords = dynamic_cast<osg::Vec3Array*>(geometry.getVertexArray());
		osg::Vec2Array* tcoords = dynamic_cast<osg::Vec2Array*>(geometry.getTexCoordArray(0));
						
		unsigned int rows = 20, cols = 20;
		float rowSize = height / (float)rows;
		float colSize = width / (float)cols;
		double x, y, px, py, u, v;

		for (unsigned int r = 0; r < rows; r++) {
			for (unsigned int c = 0; c <= cols; c++) {

				x = c * colSize;
				y = r * rowSize;

				Observer2Ideal(m_cparam->cparam.dist_factor, x, y, &px, &py);
				coords->push_back(osg::Vec3(px, py, 0.0f));

				u = (c / (float)cols) * maxU;
				v = (1.0f - (r / (float)rows)) * maxV;
				tcoords->push_back(osg::Vec2(u, v));

				x = c * colSize;
				y = (r+1) * rowSize;

				Observer2Ideal(m_cparam->cparam.dist_factor, x, y, &px, &py);
				coords->push_back(osg::Vec3(px, py, 0.0f));

				u = (c / (float)cols) * maxU;
				v = (1.0f - ((r+1) / (float)rows)) * maxV;
				tcoords->push_back(osg::Vec2(u, v));

			}

			geometry.addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, 
				r * 2 * (cols+1), 2 * (cols+1)));
		}
	}

	int ARToolKitTracker::getGLPixelFormatForARPixelFormat(const int arPixelFormat, GLenum *internalformat_GL, GLenum *format_GL, GLenum *type_GL)
	{
		// Translate the internal pixelformat to an OpenGL texture2D triplet.
		switch (arPixelFormat) {
			case AR_PIXEL_FORMAT_RGB:
				*internalformat_GL = GL_RGB;
				*format_GL = GL_RGB;
				*type_GL = GL_UNSIGNED_BYTE;
				break;
			case AR_PIXEL_FORMAT_BGR:
				*internalformat_GL = GL_RGB;
				*format_GL = GL_BGR;
				*type_GL = GL_UNSIGNED_BYTE;
				break;
			case AR_PIXEL_FORMAT_RGBA:
				*internalformat_GL = GL_RGBA;
				*format_GL = GL_RGBA;
				*type_GL = GL_UNSIGNED_BYTE;
			case AR_PIXEL_FORMAT_BGRA:
				*internalformat_GL = GL_RGBA;
				*format_GL = GL_BGRA;
				*type_GL = GL_UNSIGNED_BYTE;
				break;
			case AR_PIXEL_FORMAT_ARGB:
				*internalformat_GL = GL_RGBA;
				*format_GL = GL_BGRA;
#ifdef AR_BIG_ENDIAN
				*type_GL = GL_UNSIGNED_INT_8_8_8_8_REV;
#else
				*type_GL = GL_UNSIGNED_INT_8_8_8_8;
#endif
				break;
			case AR_PIXEL_FORMAT_ABGR:
				*internalformat_GL = GL_RGBA;
				*format_GL = GL_ABGR_EXT;
				*type_GL = GL_UNSIGNED_BYTE;
				break;
			case AR_PIXEL_FORMAT_MONO:
				*internalformat_GL = GL_LUMINANCE8;
				*format_GL = GL_LUMINANCE;
				*type_GL = GL_UNSIGNED_BYTE;
				break;
			case AR_PIXEL_FORMAT_2vuy:
				*internalformat_GL = GL_RGB;
				*format_GL = GL_YCBCR_422_APPLE; //GL_YCBCR_MESA
#ifdef AR_BIG_ENDIAN
				*type_GL = GL_UNSIGNED_SHORT_8_8_REV_APPLE; //GL_UNSIGNED_SHORT_8_8_REV_MESA
#else
				*type_GL = GL_UNSIGNED_SHORT_8_8_APPLE; //GL_UNSIGNED_SHORT_8_8_MESA
#endif
				break;
			case AR_PIXEL_FORMAT_yuvs:
				*internalformat_GL = GL_RGB;
				*format_GL = GL_YCBCR_422_APPLE; //GL_YCBCR_MESA
#ifdef AR_BIG_ENDIAN
				*type_GL = GL_UNSIGNED_SHORT_8_8_APPLE; //GL_UNSIGNED_SHORT_8_8_MESA
#else
				*type_GL = GL_UNSIGNED_SHORT_8_8_REV_APPLE; //GL_UNSIGNED_SHORT_8_8_REV_MESA
#endif
				break;
			default:
				return (-1);
				break;
		}
		return (0);
	}
	
}; // namespace osgART
