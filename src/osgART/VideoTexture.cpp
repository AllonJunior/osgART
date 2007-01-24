/*
 *	osgART/VideoTexture
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

#include <osgART/VideoManager>
#include <osgART/VideoTexture>
#include <osgART/VideoTexCallback>
#include <osgART/VideoBackground>

#include <osg/Notify>


// using namespace std;
namespace osgART {

	VideoTexture::VideoTexture(GenericVideo* video) : VideoTextureBase(video) 
	{
		this->setDataVariance(osg::Object::DYNAMIC);

		m_vidWidth = m_video->getWidth();
		m_vidHeight = m_video->getHeight();

		m_texWidth = GenericVideoObject::mathNextPowerOf2(m_vidWidth);
		m_texHeight = GenericVideoObject::mathNextPowerOf2(m_vidHeight);

		switch (m_video->pixelFormat())
		{
		case VIDEOFORMAT_RGB24:
			this->setInternalFormat(GL_RGB);
			break;
		case VIDEOFORMAT_BGR24:
			this->setInternalFormat(GL_RGB);
			break;
		case VIDEOFORMAT_RGBA32:
			this->setInternalFormat(GL_RGBA);
			break;
		case VIDEOFORMAT_ABGR32:
			this->setInternalFormat(GL_RGBA);
			break;
		case VIDEOFORMAT_BGRA32:
			this->setInternalFormat(GL_RGBA);
			break;
		case VIDEOFORMAT_ARGB32:
			this->setInternalFormat(GL_RGBA);
			break;
		case VIDEOFORMAT_YUV422:
#ifdef __APPLE__
			//# error Due to lack of support in OpenSceneGraph, 
			// AR_PIX_FORMAT_2vuy is not supported in osgART yet. 
			// Use AR_PIX_FORMAT_ARGB instead.\n
			this->setInternalFormat(GL_RGB);
#endif
				break;
		default: 
			osg::notify(osg::WARN) << "Warning: format not supported for texture mapping!" << std::endl;
			break;
		}

		this->setTextureSize(m_texWidth,m_texHeight);
		this->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		this->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		this->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP);
		this->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP);

		this->setSubloadCallback(new VideoTexCallback(this, 
			m_vidWidth, m_vidHeight, m_texWidth, m_texHeight));

	}

	VideoTexture::~VideoTexture()
	{	    
	}

};
