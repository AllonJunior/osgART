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

#include "OpenThreads/ScopedLock"

#include "osgART/Calibration"
#include "osgART/Utils"
#include "osgART/Video"
#include "osgART/VideoGeode"
#include "osgART/ImageStreamCallback"


#include <osg/Node>
#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/BlendFunc>
#include <osg/Notify>
#include <osg/Geometry>
#include <osg/ImageStream>

#include <fstream>

namespace osgART {

	namespace Internal {

		class Texture2DCallback : public osg::Texture2D::SubloadCallback
		{
		public:
			
			Texture2DCallback(osg::Texture2D* texture);

			void load(const osg::Texture2D& texture, osg::State&) const;
			void subload(const osg::Texture2D& texture, osg::State&) const;

            inline float getTexCoordX() const { return (_texCoordX);}
            inline float getTexCoordY() const { return (_texCoordY);}
			
			// needed for OSG 3.0
			inline bool textureObjectValid(const osg::Texture2D& texture, osg::State& state) const
            {
				return true;
            }
			

		protected:
			
			float _texCoordX;
			float _texCoordY;

		};


		Texture2DCallback::Texture2DCallback(osg::Texture2D* texture) 
			: osg::Texture2D::SubloadCallback()
			, _texCoordX(texture->getImage()->s() / (float)equalOrGreaterPowerOfTwo((unsigned int)texture->getImage()->s()))
			, _texCoordY(texture->getImage()->t() / (float)equalOrGreaterPowerOfTwo((unsigned int)texture->getImage()->t()))
		{
			texture->setTextureSize(equalOrGreaterPowerOfTwo((unsigned int)texture->getImage()->s()),
				equalOrGreaterPowerOfTwo((unsigned int)texture->getImage()->t()));

			texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
			texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
			texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP);
			texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP);
			
		}

		/*virtual*/ 
		void 
		Texture2DCallback::load(const osg::Texture2D& texture, osg::State& state) const 
		{
			
			const osg::Image* _image = texture.getImage();
/*
			glTexImage2D(GL_TEXTURE_2D, 0, 
				// hse25: internal texture format gets overwritten by the image format 
				// we need just the components - ???
				//osg::Image::computeNumComponents(_image->getInternalTextureFormat()), 
				//osg::Image::computeNumComponents(_image->getPixelFormat()), 
				_image->getPixelFormat(),
				(float)equalOrGreaterPowerOfTwo((unsigned int)_image->s()), 
				(float)equalOrGreaterPowerOfTwo((unsigned int)_image->t()), 
				0, _image->getPixelFormat(), 
				_image->getDataType(), 0);
*/
				int w = equalOrGreaterPowerOfTwo((unsigned int)_image->s());
				int h = equalOrGreaterPowerOfTwo((unsigned int)_image->t());
				
				
				osg::notify() << "Texture2DCallback load: " << _image->s() << "x" << _image->t() << "  ->  " << w << "x" << h << std::endl;
				
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
				
				
				

		}
		
		/*virtual */ 
		void
		Texture2DCallback::subload(const osg::Texture2D& texture, osg::State& state) const 
		{
			
			osg::Image* _image = const_cast<osg::Image*>(texture.getImage());
			osgART::Video* vid = dynamic_cast<osgART::Video*>(_image);
			
			if (vid) {
				OpenThreads::ScopedLock<OpenThreads::Mutex> lock(vid->getMutex());
				//osg::notify() << "Texture2DCallback subload a" << std::endl;
				//texture.applyTexImage2D_subload(state, GL_TEXTURE_2D, _image, _image->s(), _image->t(), _image->getInternalTextureFormat(), 1);		
				
				//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _image->s(), _image->t(), _image->getPixelFormat(), _image->getDataType(), _image->data());
				
				
				osg::notify() << "Texture2DCallback subload a: " << _image->s() << "x" << _image->t() << std::endl;
				
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _image->s(), _image->t(), GL_RGB, GL_UNSIGNED_BYTE, _image->data());
				
				
			} else {
				osg::notify() << "Texture2DCallback subload b" << std::endl;
				texture.applyTexImage2D_subload(state, GL_TEXTURE_2D, _image, _image->s(), _image->t(), _image->getInternalTextureFormat(), 1);
				//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _image->s(), _image->t(), _image->getPixelFormat(), _image->getDataType(), _image->data());
			}
			
		}
	}



	VideoGeode::VideoGeode() : osg::Geode() {
	}

	VideoGeode::VideoGeode(
				osg::Image* image,					// Image to texture
		osgART::Calibration* calibration,	// Calibration object for undistortion calculation
		double width,
		double height,
		int cols,							// Number of columns
		int rows,							// Number of rows
		TextureMode textureMode)			// Mode to texture with
		
		
		 : osg::Geode()
	{ 

		this->addDrawable(createVideoMesh(image, calibration, width, height, cols, rows, textureMode));

	}

	VideoGeode::VideoGeode(const VideoGeode&, const osg::CopyOp& /*copyop = CopyOp::SHALLOW_COPY*/)
	{
	}

	VideoGeode::~VideoGeode()
	{
	}

	osg::Geometry* VideoGeode::createVideoMesh(
		osg::Image* image,					// Image to texture
		osgART::Calibration* calibration,	// Calibration object for undistortion calculation
		double width,
		double height,
		int cols,							// Number of columns
		int rows,							// Number of rows
		TextureMode textureMode)			// Mode to texture with
		
	{

		osg::Geometry* geom = new osg::Geometry();
		geom->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);	

		geom->getOrCreateStateSet()->setMode(GL_CULL_FACE, GL_FALSE);

		osg::Vec2Array* verts = new osg::Vec2Array();
		geom->setVertexArray(verts);
		osg::Vec2Array* tcs = new osg::Vec2Array();
		geom->setTexCoordArray(0, tcs);
		osg::Vec4Array* colors = new osg::Vec4Array();
		colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		geom->setColorArray(colors);
		geom->setColorBinding(osg::Geometry::BIND_OVERALL);

		double imageWidth = (double)image->s();
		double imageHeight = (double)image->t();

		double maxU = 1.0f, maxV = 1.0f;

		switch (textureMode) {
			
			case USE_TEXTURE_RECTANGLE:
				{
					maxU = imageWidth;
					maxV = imageHeight;
					osg::TextureRectangle* texture = new osg::TextureRectangle(image);
					texture->setDataVariance(osg::Object::DYNAMIC);
					geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture);
					break;
				}

			case USE_TEXTURE_2D:
			default:
				{
					maxU = imageWidth / (double)osgART::equalOrGreaterPowerOfTwo((unsigned int)imageWidth);
					maxV = imageHeight / (double)osgART::equalOrGreaterPowerOfTwo((unsigned int)imageHeight);
					osg::Texture2D* texture = new osg::Texture2D(image);
					texture->setDataVariance(osg::Object::DYNAMIC);
					texture->setSubloadCallback(new Internal::Texture2DCallback(texture));
					geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture);
					break;
				}
		}

		for (int r = 0; r < rows; r++) {
			for (int c = 0; c <= cols; c++) {

				double dr1 = (double)r / (double)rows;
				double dr2 = (double)(r+1) / (double)rows;
				double dc = (double)c / (double)cols;

				osg::Vec2d p1(dc, dr1), p2(dc, dr2);	// Normalised coordinates of distorted pixels
				osg::Vec2d u_p1 = p1, u_p2 = p2;		// Normalises coordinates of undistorted pixels (to be computed)

				if (calibration) {
					calibration->undistort(p1[0] * imageWidth, p1[1] * imageHeight, &u_p1[0], &u_p1[1]);
					u_p1[0] /= imageWidth;
					u_p1[1] /= imageHeight;
					calibration->undistort(p2[0] * imageWidth, p2[1] * imageHeight, &u_p2[0], &u_p2[1]);
					u_p2[0] /= imageWidth;
					u_p2[1] /= imageHeight;
				}
			
				// Scale to widthxheight
				u_p1[0] *= width;
				u_p1[1] *= height;
				u_p2[0] *= width;
				u_p2[1] *= height;

				verts->push_back(u_p1);
				verts->push_back(u_p2);

				tcs->push_back(osg::Vec2(dc * maxU, (1.0f - dr1) * maxV));
				tcs->push_back(osg::Vec2(dc * maxU, (1.0f - dr2) * maxV));

			}

			geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, r * 2 * (cols+1), 2 * (cols+1)));
		}




static const char osgARTVideoBackgroundVert[] =
 "varying vec4 texCoord0;\n"
 "void main() \n"
 "{\n"
 "gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; \n"
 "texCoord0 = gl_MultiTexCoord0;\n"
 "} \n";


static const char osgARTVideoBackgroundFrag[] =
 "precision mediump float; \n"
 "varying mediump vec4 texCoord0; \n"
 "uniform sampler2D tex0; \n"
 "void main() \n"
 "{ \n"
 " gl_FragColor = texture2D( tex0, texCoord0.st );\n"
 " gl_FragColor.a = 1.0;\n"
 "} \n";
 
 	osg::Shader* vBGShader = new osg::Shader(osg::Shader::VERTEX, osgARTVideoBackgroundVert);
    osg::Shader* fBGShader = new osg::Shader(osg::Shader::FRAGMENT, osgARTVideoBackgroundFrag);

   
	
  osg::Program * BGprog = new osg::Program;
    BGprog->addShader(vBGShader);
    BGprog->addShader(fBGShader);
    geom->getOrCreateStateSet()->setAttribute(BGprog);

	//geom->getOrCreateStateSet()->addUniform(new osg::Uniform("s_texture", 0));

		return geom;

	}

}
