//skeletonglut.cpp

/* Copyright 2011 Daniel Schroeder
 *
 * This file is part of MaGLi.
 *
 * MaGLi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MaGLi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MaGLi.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <iostream>
#include <stdlib.h>
#include "skeletonglut.h"
#include "../pnm_tools/pnm_tools.h"
#include "../shaderloader/shaderloader.h"

using namespace std;

//+++ SkeletonGLUT

const int SkeletonGLUT::DEF_WINXRES = 800;
const int SkeletonGLUT::DEF_WINYRES = 600;

SkeletonGLUT::SkeletonGLUT()
{
	cout << "Constructing SkeletonGLUT " << this << endl;
	usecleanupfunction = false;
	winxres = SkeletonGLUT::DEF_WINXRES;
	winyres = SkeletonGLUT::DEF_WINYRES;
	doneinit = false;
	window = 0;
	mouseposx = mouseposy = 0;
	mouseaccumx[0] = mouseaccumx[1] = mouseaccumx[2] = 0;
	mouseaccumy[0] = mouseaccumy[1] = mouseaccumy[2] = 0;
	mousechangex = mousechangey = 0;
	mousebutton = GLUT_LEFT_BUTTON;
}

SkeletonGLUT::SkeletonGLUT(void (*cupf)())
{
	cout << "Constructing SkeletonGLUT " << this << " with cleanup function" << endl;
	cleanupfunction = cupf;
	usecleanupfunction = true;
	winxres = SkeletonGLUT::DEF_WINXRES;
	winyres = SkeletonGLUT::DEF_WINYRES;
	doneinit = false;
	window = 0;
	mouseposx = mouseposy = 0;
	mouseaccumx[0] = mouseaccumx[1] = mouseaccumx[2] = 0;
	mouseaccumy[0] = mouseaccumy[1] = mouseaccumy[2] = 0;
	mousechangex = mousechangey = 0;
	mousebutton = GLUT_LEFT_BUTTON;
}

SkeletonGLUT::~SkeletonGLUT()
{
	cout << "Destructing SkeletonGLUT " << this << endl;
	if(usecleanupfunction)
		cleanupfunction();
}

void SkeletonGLUT::setCleanupFunction(void (*cupf)())
{
	cleanupfunction = cupf;
	usecleanupfunction = true;
}

bool SkeletonGLUT::supportedFramebuffers()
{
	//query GLEW
	if(!GLEW_EXT_framebuffer_object)
	{
		cerr << "EXT_framebuffer_object unsupported" << endl;
		return false;
	}
	return true;
}

bool SkeletonGLUT::supportedShaders()
{
	//query GLEW
	if(!GLEW_ARB_shading_language_100 || !GLEW_ARB_shader_objects) //may add more
	{
		cerr << "shaders unsupported" << endl;
		return false;
	}
	return true;
}

bool SkeletonGLUT::loadPNM(const char * fname, float ** dataptrptr, int * xresptr, int * yresptr,
	int outxres, int outyres)
{
	string emsg("No error");
	ReaderPNM reader(fname);
	int tempxres;
	int tempyres;
	float * tempdataptr;
	float * tempdatastepptr;
	if(!reader.checkError(&emsg))
	{
		//well that didn't work
		cerr << "Error reading " << fname << ": " << emsg << endl;
		*dataptrptr = NULL;
		//we don't know for sure what size this is
		*xresptr = *yresptr = 0;
		return false;
		//how many ways are there to say "THIS DIDN'T WORK"?
	}
	tempxres = reader.getXRes();
	tempyres = reader.getYRes();
	if((outxres <= 0) || (outyres <= 0))
	{
		//set output size to size of file
		outxres = tempxres;
		outyres = tempyres;
	}

	//allocate and confirm allocation for loaded data
	tempdataptr = new (nothrow) float [(3 * outxres) * outyres];
	if(tempdataptr == NULL)
	{
		cerr << "Failed to allocate memory while loading " << fname << endl;
		*dataptrptr = NULL;
		//we don't know for sure what size this is
		*xresptr = *yresptr = 0;
		return false;
		//how many ways are there to say "THIS DIDN'T WORK"?
	}

	//read the pixels
	//the excess bottom of the output image

	float dummy[3];

	for(int j = outyres - 1; j >= 0; j--)  //every row of the output
	{
		tempdatastepptr = &(tempdataptr[j * (3 * outxres)]);
		if(j < tempyres)  //in range to grab image data
		{
			if(outxres >= tempxres)
			{
				for(int i = 0; i < tempxres; i++)  //pixels of tex in row
				{
					reader.getColorFloat(tempdatastepptr);
					tempdatastepptr += 3;
				}
				for(int i = tempxres; i < outxres; i++)  //remaining pixels in out
				{
					//tempdatastepptr[0] = tempdatastepptr[1] = tempdatastepptr[2] = 0.0f;
					//CHANGE - now I tile the image instead
					//of padding with black
					for(int k = 0; k < 3; k++)
						tempdatastepptr[k] = tempdatastepptr[k - 3 * tempxres];
					tempdatastepptr += 3;
				}
			}
			else
			{
				for(int i = 0; i < outxres; i++)
				{
					reader.getColorFloat(tempdatastepptr);
					tempdatastepptr += 3;
				}
				for(int i = outxres; i < tempxres; i++)
					reader.getColorFloat(&(dummy[0]));
			}
		}
		else  //pad with black
		{
			for(int a = 0; a < 3 * outxres; a++)
				tempdatastepptr[a] = 0.0f;
		}
	}
	//CHANGE! tile the image vertically if needed
	for(int j = tempyres; j < outyres; j++)  //unfilled rows
	{
		tempdatastepptr = &(tempdataptr[j * (3 * outxres)]);
		for(int i = 0; i < 3 * outxres; i++)
		{
			//stuff
			tempdatastepptr[i] = tempdatastepptr[i - 3 * outxres * tempyres];
		}
	}

	if(!reader.checkError(&emsg))
	{
		//well that didn't work
		cerr << "Error by the end of reading " << fname << ": " << emsg << endl;
		delete [] tempdataptr;
		*dataptrptr = NULL;  //we don't know for sure what size this is
		*xresptr = *yresptr = 0;
		return false;  //how many ways are there to say "THIS DIDN'T WORK"?
	}

	//then it worked
	*dataptrptr = tempdataptr;
	*xresptr = tempxres;
	*yresptr = tempyres;
	return true;
}

bool SkeletonGLUT::genTexture2D(GLuint * texobjptr, GLint internalformat, GLsizei width, GLsizei height,
	GLint border, float * data)
{
	if(data == NULL)
	{
		*texobjptr = 0;
		cerr << "genTexture2D received invalid data" << endl;
		return false;
	}
	glPushAttrib(GL_TEXTURE_BIT);
	GLuint tempint;
	glGenTextures(1, &tempint);
	glBindTexture(GL_TEXTURE_2D, tempint);
	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, GL_RGB, GL_FLOAT, data);
	*texobjptr = tempint;
	glPopAttrib();
	return true;
}

bool SkeletonGLUT::genTexture2D(GLuint * texobjptr, GLint internalformat, GLsizei width, GLsizei height,
	GLint border)
{
	float * tempfloatptr = new (nothrow) float [3 * width * height];
	if(tempfloatptr == NULL)
	{
		*texobjptr = 0;
		cerr << "memory allocation failed in genTexture2D" << endl;
		return false;
	}
	for(int j = 0; j < height; j++)
	{
		for(int i = 0; i < width; i++)
		{
			tempfloatptr[3 * (j * width + i) + 0] = 
				tempfloatptr[3 * (j * width + i) + 1] = 
				tempfloatptr[3 * (j * width + i) + 2] =
					(((i + j) % 8) & 4) ? 1.0f : 0.0f;
		}
	}
	bool success = genTexture2D(texobjptr, internalformat, width, height, border, tempfloatptr);
	delete [] tempfloatptr;
	return success;
}

bool SkeletonGLUT::genSimpleTexture2D(GLuint * texobjptr, int width, int height, float * bufptr)
{
	if(bufptr == NULL)
	{
		*texobjptr = 0;
		cerr << "invalid data passed to genSimpleTexture2D" << endl;
		return false;
	}
	GLuint tempint;
	bool itworked = genTexture2D(&tempint, GL_RGB, width, height, 0, bufptr);
	if(itworked)
	{
		*texobjptr = tempint;
		glPushAttrib(GL_TEXTURE_BIT);
		glBindTexture(GL_TEXTURE_2D, tempint);

		//set some reasonable texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glPopAttrib();

		return true;
	}
	*texobjptr = 0;
	return false;
}

bool SkeletonGLUT::genSimpleTexture2D(GLuint * texobjptr, int width, int height)
{
	float * tempfloatptr = new (nothrow) float [3 * width * height];
	if(tempfloatptr == NULL)
	{
		*texobjptr = 0;
		cout << "allocation failed in genSimpleTexture2D" << endl;
		return false;
	}
	for(int j = 0; j < height; j++)
	{
		for(int i = 0; i < width; i++)
		{
			tempfloatptr[3 * (j * width + i) + 0] = 
				tempfloatptr[3 * (j * width + i) + 1] = 
				tempfloatptr[3 * (j * width + i) + 2] =
					(((i + j) % 8) & 4) ? 1.0f : 0.0f;
		}
	}

	bool success = genSimpleTexture2D(texobjptr, width, height, tempfloatptr);
	delete [] tempfloatptr;

	//formerly I had set texture parameters here, but that is already done
	//by the above call of genSimpleTexture2D with tempfloatptr
	return success;
}

bool SkeletonGLUT::genSimpleDepthTexture(GLuint * texobjptr, int width, int height)
{
	float * tempfloatptr = new (nothrow) float [width * height];
	if(tempfloatptr == NULL)
	{
		*texobjptr = 0;
		cout << "allocation failed in genSimpleDepthTexture" << endl;
		return false;
	}
	//fill it with something interesting - why not
	for(int j = 0; j < height; j++)
	{
		for(int i = 0; i < width; i++)
		{
			tempfloatptr[j * width + i] = 
					(((i + j) % 8) & 4) ? 1.0f : 0.0f;
		}
	}

	glPushAttrib(GL_TEXTURE_BIT);
	GLuint tempint;
	glGenTextures(1, &tempint);
	glBindTexture(GL_TEXTURE_2D, tempint);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT,
		GL_FLOAT, tempfloatptr);
	*texobjptr = tempint;
	//now I try setting some properties of the texture
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	//ok
	glPopAttrib();
	delete [] tempfloatptr;
	return true;
}

bool SkeletonGLUT::genSimpleFloatRGBATexture(GLuint * texobjptr, int width,
	int height, float * data)
{
	if(data == NULL)
	{
		*texobjptr = 0;
		cerr << "genSimpleFloatTexture received invalid data" << endl;
		return false;
	}

	GLuint tempint;   // UBERHACK BELOW!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
	bool itworked = genTexture2D(&tempint, GL_RGBA16F_ARB, width, height, 0, data);
	if(itworked)
	{
		*texobjptr = tempint;
		glPushAttrib(GL_TEXTURE_BIT);
		glBindTexture(GL_TEXTURE_2D, tempint);

		//set some reasonable texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glPopAttrib();

		return true;
	}
	*texobjptr = 0;
	return false;
}

bool SkeletonGLUT::genSimpleFloatRGBATexture(GLuint * texobjptr, int width,
	int height)
{
	float * tempfloatptr = new (nothrow) float [3 * width * height];
	if(tempfloatptr == NULL)
	{
		*texobjptr = 0;
		cout << "allocation failed in genSimpleFloatTexture" << endl;
		return false;
	}
	for(int j = 0; j < height; j++)
	{
		for(int i = 0; i < width; i++)
		{
			tempfloatptr[3 * (j * width + i) + 0] = 
				tempfloatptr[3 * (j * width + i) + 1] = 
				tempfloatptr[3 * (j * width + i) + 2] =
					(((i + j) % 8) & 4) ? 1.0f : 0.0f;
		}
	}

	bool success = genSimpleFloatRGBATexture(texobjptr, width, height, tempfloatptr);
	delete [] tempfloatptr;

	return success;
}

bool SkeletonGLUT::loadGenSimpleTexture2D(GLuint * texobjptr, int width, int height, const char * fname)
{
	float * tempfloatptr;
	int tempx;
	int tempy;
	bool success = loadPNM(fname, &tempfloatptr, &tempx, &tempy, width, height);
	if(!success)
	{
		*texobjptr = 0;
		return false;
	}
	success = genSimpleTexture2D(texobjptr, width, height, tempfloatptr);
	delete [] tempfloatptr;
	return success;
}

bool SkeletonGLUT::loadGenSimpleTexture2D(GLuint * texobjptr, const char * fname)
{
	float * tempfloatptr;
	int tempx;
	int tempy;
	bool success = loadPNM(fname, &tempfloatptr, &tempx, &tempy, 0, 0);
	if(!success)
	{
		*texobjptr = 0;
		return false;
	}
	success = genSimpleTexture2D(texobjptr, tempx, tempy, tempfloatptr);
	delete [] tempfloatptr;
	return success;
}

bool SkeletonGLUT::genRenderbuffer(GLuint * rbobjptr, GLenum internalformat, int width, int height)
{
	//query GLEW
	if(!supportedFramebuffers())
	{
		cerr << "renderbuffer gen. failed" << endl;
		*rbobjptr = 0;
		return false;
	}
	GLuint tempobj;
	GLuint prevrenderbuf;
	glGetIntegerv(GL_RENDERBUFFER_BINDING_EXT, (GLint *) &prevrenderbuf);
	glGenRenderbuffersEXT(1, &tempobj);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, tempobj);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internalformat, width, height);
	*rbobjptr = tempobj;
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, prevrenderbuf);
	return true;
}

bool SkeletonGLUT::genFramebuffer(GLuint * fbobjptr)
{
	if(!supportedFramebuffers())
	{
		cerr << "framebuffer gen. failed" << endl;
		*fbobjptr = 0;
		return false;
	}
	GLuint tempobj;
	GLuint prevframebuf;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, (GLint *) &prevframebuf);
	glGenFramebuffersEXT(1, &tempobj);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, tempobj);
	*fbobjptr = tempobj;
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, prevframebuf);
	return true;
}

bool SkeletonGLUT::genSimpleFramebufferSubstitute(GLuint * fbobjptr, GLuint * rbobjptr, GLuint * texobjptr,
	int width, int height)
{
	GLuint tempfbo;
	GLuint temprbo;
	GLuint temptex;
	GLuint lastfbo;
	if(!genRenderbuffer(&temprbo, GL_DEPTH_COMPONENT, width, height))
	{
		cerr << "simple framebuffer generation: renderbuffer gen failed" << endl;
		*fbobjptr = 0;
		*rbobjptr = 0;
		*texobjptr = 0;
		return false;
	}
	if(!genSimpleTexture2D(&temptex, width, height))
	{
		cerr << "simple framebuffer generation: texture gen failed" << endl;
		glDeleteRenderbuffersEXT(1, &temprbo);
		*fbobjptr = 0;
		*rbobjptr = 0;
		*texobjptr = 0;
		return false;
	}
	if(!genFramebuffer(&tempfbo))
	{
		cerr << "simple framebuffer generation: framebuffer gen failed" << endl;
		glDeleteRenderbuffersEXT(1, &temprbo);
		glDeleteTextures(1, &temptex);
		*fbobjptr = 0;
		*rbobjptr = 0;
		*texobjptr = 0;
		return false;
	}
	//all the components were successfully made
	//put 'em together
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, (GLint *) &lastfbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, tempfbo);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
		GL_RENDERBUFFER_EXT, temprbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
		GL_TEXTURE_2D, temptex, 0);
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, lastfbo);
	*fbobjptr = tempfbo;
	*rbobjptr = temprbo;
	*texobjptr = temptex;
	return true;
}

bool SkeletonGLUT::genSimpleFramebufferAllTextures(GLuint * fbobjptr, GLuint * deptexptr,
	GLuint * texobjptr, int width, int height)
{
	//let's see if this works.
	GLuint tempfbo;
	GLuint tempdep;
	GLuint temptex;
	GLuint lastfbo;

	if(!genSimpleDepthTexture(&tempdep, width, height))
	{
		cerr << "simple framebuffer generation: depth texture gen failed" << endl;
		*fbobjptr = 0;
		*deptexptr = 0;
		*texobjptr = 0;
		return false;
	}
	if(!genSimpleTexture2D(&temptex, width, height))
	{
		cerr << "simple framebuffer generation: texture gen failed" << endl;
		glDeleteTextures(1, &tempdep);
		*fbobjptr = 0;
		*deptexptr = 0;
		*texobjptr = 0;
		return false;
	}
	if(!genFramebuffer(&tempfbo))
	{
		cerr << "simple framebuffer generation: framebuffer gen failed" << endl;
		glDeleteTextures(1, &tempdep);
		glDeleteTextures(1, &temptex);
		*fbobjptr = 0;
		*deptexptr = 0;
		*texobjptr = 0;
		return false;
	}
	//they should all be good. Cool.

	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, (GLint *) &lastfbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, tempfbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
		GL_TEXTURE_2D, tempdep, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
		GL_TEXTURE_2D, temptex, 0);
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, lastfbo);
	*fbobjptr = tempfbo;
	*deptexptr = tempdep;
	*texobjptr = temptex;
	return true;
}

bool SkeletonGLUT::checkFramebufferStatus()
{
	//check that FBOs are supported
	if(!supportedFramebuffers())
	{
		cerr << "framebuffer status check failed" << endl;
		return false;
	}
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch(status)
	{
		case GL_FRAMEBUFFER_COMPLETE_EXT : //complete
			break;  //will do further tests(?)
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT : //missing or invalid attachments
			cerr << "FBO: invalid/incomplete attachments" << endl;
			return false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT : //nothing attached
			cerr << "FBO: apparently no attachments" << endl;
			return false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT : //variation in dimensions
			cerr << "FBO: inconsistent dimensions" << endl;
			return false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT : //variation in color attachment formats
			cerr << "FBO: inconsistent color attachment formats" << endl;
			return false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT : //tried to draw to unassigned buffers
			cerr << "FBO: last draw referenced unused color buffers" << endl;
			return false;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT : //tried to read from unassigned buffers
			cerr << "FBO: last glReadBuffer referenced unused color attachment" << endl;
			return false;
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT : //usage violated implementation-specific rules
			cerr << "FBO: settings violate implementation-specific rules" << endl;
			return false;
			break;
		default : //unknown error
			cerr << "FBO: unknown error with status" << endl;
			return false;
			break;
	}
	//it was good
	return true;
}

bool SkeletonGLUT::checkFramebufferStatus(GLuint fbobj)
{
	if(!supportedFramebuffers())
	{
		cerr << "framebuffer status check failed" << endl;
		return false;
	}
	GLuint lastfbo;
	bool success;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, (GLint *) &lastfbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbobj);
	success = checkFramebufferStatus();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, lastfbo);
	return success;  //I'll stick with whatever error was printed by checkFramebufferStatus()
}

bool SkeletonGLUT::loadGenShader(GLuint * shaderptr, GLenum type, const char * fname)
{
	if(!supportedShaders() || ((type != GL_VERTEX_SHADER) && (type != GL_FRAGMENT_SHADER)))
	{
		//shaders unsupported, or attempting to create other than vertex or fragment shader
		cerr << "shader generation failed" << endl;
		*shaderptr = 0;
		return false;
	}
	//shaders supported, and type is valid
	GLuint tempshader = glCreateShader(type);
	if(tempshader == 0)
	{
		cerr << "OpenGL failed to generate shader" << endl;
		*shaderptr = 0;
		return false;
	}
	ShaderLoader shaderload;
	GLchar ** listptr;
	int listsize;
	shaderload.loadFile(fname);
	if(!shaderload.getSizeAndList(&listsize, (char ***) &listptr))
	{
		cout << "load of shader source file " << fname << " failed" << endl;
		*shaderptr = 0;
		glDeleteShader(tempshader);
		return false;
	}
	//we have the shader text
	glShaderSource(tempshader, listsize, (const GLchar **) listptr, NULL);
	glCompileShader(tempshader);
	//check for compile errors
	GLint success;
	glGetShaderiv(tempshader, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		cerr << "shader compile of " << fname << " failed" << endl;
		const int MAX_INFO_LOG_SIZE = 400;
		GLchar infolog[MAX_INFO_LOG_SIZE];
		glGetShaderiv(tempshader, GL_INFO_LOG_LENGTH, &success);
		if(success > MAX_INFO_LOG_SIZE)
		{
			cerr << "...and info log is too big" << endl;
		}
		glGetShaderInfoLog(tempshader, MAX_INFO_LOG_SIZE, NULL, &(infolog[0]));
		cerr << "----------------------------------------" << endl;
		cerr << infolog << endl;
		cerr << "----------------------------------------" << endl;
		glDeleteShader(tempshader);
		*shaderptr = 0;
		return false;
	}
	//then it worked
	*shaderptr = tempshader;
	return true;
}

bool SkeletonGLUT::loadGenProgram(GLuint * programptr, GLuint * vsptr, const char * vsfname,
	GLuint * fsptr, const char * fsfname)
{
	const int MAX_INFO_LOG_SIZE = 400;
	GLint success;
	if(!supportedShaders())
	{
		cerr << "shader program generation failed" << endl;
		*programptr = *vsptr = *fsptr = 0;
		return false;
	}
	GLuint tempvs;
	GLuint tempfs;
	GLuint tempprogram = glCreateProgram();
	if(tempprogram == 0)
	{
		cerr << "OpenGL failed to generate shader program" << endl;
		*programptr = *vsptr = *fsptr = 0;
		return false;
	}
	//we have shader program. Now generate appropriate shaders
	if(vsfname != NULL)
	{
		if(!loadGenShader(&tempvs, GL_VERTEX_SHADER, vsfname))
		{
			cerr << "failed to generate vertex shader for program" << endl;
			glDeleteProgram(tempprogram);
			*programptr = *vsptr = *fsptr = 0;
			return false;
		}
		glAttachShader(tempprogram, tempvs);
	}
	if(fsfname != NULL)
	{
		if(!loadGenShader(&tempfs, GL_FRAGMENT_SHADER, fsfname))
		{
			cerr << "failed to generate fragment shader for program" << endl;
			glDeleteProgram(tempprogram);
			*programptr = *vsptr = *fsptr = 0;
			return false;
		}
		glAttachShader(tempprogram, tempfs);
	}
	//now link
	glLinkProgram(tempprogram);
	glGetProgramiv(tempprogram, GL_LINK_STATUS, &success);
	if(!success)
	{
		cerr << "program link of " << vsfname << " and " << fsfname << " failed" << endl;
		GLchar infolog[MAX_INFO_LOG_SIZE];
		glGetProgramiv(tempprogram, GL_INFO_LOG_LENGTH, &success);
		if(success > MAX_INFO_LOG_SIZE)
		{
			cerr << "...and info log is too big" << endl;
		}
		glGetProgramInfoLog(tempprogram, MAX_INFO_LOG_SIZE, NULL, infolog);
		cerr << "----------------------------------------" << endl;
		cerr << infolog << endl;
		cerr << "----------------------------------------" << endl;
		glDeleteProgram(tempprogram);
		if(vsfname != NULL)
			glDeleteShader(tempvs);
		if(fsfname != NULL)
			glDeleteShader(tempfs);
		*programptr = *vsptr = *fsptr = 0;
		return false;
	}
	//it linked successfully
	*vsptr = tempvs;
	*fsptr = tempfs;
	*programptr = tempprogram;
	return true;  //now, we still have to validate before rendering w/ the shader
}

bool SkeletonGLUT::checkProgramStatus(GLuint program)
{
	const int MAX_INFO_LOG_SIZE = 400;
	GLint success;
	if(!supportedShaders())
	{
		cerr << "failed to check program status" << endl;
		return false;
	}
	GLboolean isprogram = glIsProgram(program);
	if(isprogram == GL_FALSE)
	{
		cerr << "invalid program - program is not a program" << endl;  //yay confusion
		return false;
	}
	//validate the program
	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
	if(!success)
	{
		cerr << "validation of program failed" << endl;
		GLchar infolog[MAX_INFO_LOG_SIZE];
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &success);
		if(success > MAX_INFO_LOG_SIZE)
		{
			cerr << "...and info log is too big" << endl;
		}
		glGetProgramInfoLog(program, MAX_INFO_LOG_SIZE, NULL, infolog);
		cerr << "----------------------------------------" << endl;
		cerr << infolog << endl;
		cerr << "----------------------------------------" << endl;
		//don't delete the program - this may tell the user to change uniform values
		//or something
		return false;
	}
	//success!
	return true;
}

void SkeletonGLUT::sketchWarning()
{
	glPushAttrib(GL_VIEWPORT_BIT | GL_TEXTURE_BIT | GL_DEPTH_BUFFER_BIT
		| GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TRANSFORM_BIT);

	glDisable(GL_DEPTH_TEST);

	glClearColor(0.2, 0.2, 0.2, 0.2);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.5, 1.5);
	//gluPerspective(60.0f, 0.5, 1.0, 40.0);
	//glTranslatef(0.0f, 0.0f, -5.0f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	//white backing triangle
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLES);
		glVertex3f(1.0 / 2.0, 7.0 / 8.0 + 1.0 / 16.0, -1.0);
		glVertex3f(1.0 / 8.0 - 1.0 / 16.0, 1.0 / 8.0 - 1.0 / 16.0, -1.0);
		glVertex3f(7.0 / 8.0 + 1.0 / 16.0, 1.0 / 8.0 - 1.0 / 16.0, -1.0);
	glEnd();

	//black backing triangle
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_TRIANGLES);
		glVertex3f(1.0 / 2.0, 7.0 / 8.0 + 1.0 / 32.0, -1.0);
		glVertex3f(1.0 / 8.0 - 1.0 / 32.0, 1.0 / 8.0 - 1.0 / 32.0, -1.0);
		glVertex3f(7.0 / 8.0 + 1.0 / 32.0, 1.0 / 8.0 - 1.0 / 32.0, -1.0);
	glEnd();

	//yellow main triangle
	glColor3f(1.0f, 1.0f, 0.0f);
	glBegin(GL_TRIANGLES);
		glVertex3f(1.0 / 2.0, 7.0 / 8.0, -1.0);
		glVertex3f(1.0 / 8.0, 1.0 / 8.0, -1.0);
		glVertex3f(7.0 / 8.0, 1.0 / 8.0, -1.0);
	glEnd();

	//white !
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glVertex3f(1.0 / 2.0 + 1.0 / 32.0, 5.0 / 8.0 + 1.0 / 32.0, -1.0);
		glVertex3f(1.0 / 2.0 - 1.0 / 32.0, 5.0 / 8.0 + 1.0 / 32.0, -1.0);
		glVertex3f(1.0 / 2.0 - 1.0 / 32.0, 3.0 / 8.0 - 1.0 / 32.0, -1.0);
		glVertex3f(1.0 / 2.0 + 1.0 / 32.0, 3.0 / 8.0 - 1.0 / 32.0, -1.0);

		glVertex3f(1.0 / 2.0 + 1.0 / 32.0, 2.0 / 8.0 + 1.0 / 32.0, -1.0);
		glVertex3f(1.0 / 2.0 - 1.0 / 32.0, 2.0 / 8.0 + 1.0 / 32.0, -1.0);
		glVertex3f(1.0 / 2.0 - 1.0 / 32.0, 2.0 / 8.0 - 1.0 / 32.0, -1.0);
		glVertex3f(1.0 / 2.0 + 1.0 / 32.0, 2.0 / 8.0 - 1.0 / 32.0, -1.0);
	glEnd();

	//black !
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
		glVertex3f(1.0 / 2.0 + 1.0 / 64.0, 5.0 / 8.0 + 1.0 / 64.0, -1.0);
		glVertex3f(1.0 / 2.0 - 1.0 / 64.0, 5.0 / 8.0 + 1.0 / 64.0, -1.0);
		glVertex3f(1.0 / 2.0 - 1.0 / 64.0, 3.0 / 8.0 - 1.0 / 64.0, -1.0);
		glVertex3f(1.0 / 2.0 + 1.0 / 64.0, 3.0 / 8.0 - 1.0 / 64.0, -1.0);

		glVertex3f(1.0 / 2.0 + 1.0 / 64.0, 2.0 / 8.0 + 1.0 / 64.0, -1.0);
		glVertex3f(1.0 / 2.0 - 1.0 / 64.0, 2.0 / 8.0 + 1.0 / 64.0, -1.0);
		glVertex3f(1.0 / 2.0 - 1.0 / 64.0, 2.0 / 8.0 - 1.0 / 64.0, -1.0);
		glVertex3f(1.0 / 2.0 + 1.0 / 64.0, 2.0 / 8.0 - 1.0 / 64.0, -1.0);
	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();  //this may not work
	return;
}

void SkeletonGLUT::sketchSingleTex()
{
	glPushAttrib(GL_VIEWPORT_BIT | GL_TEXTURE_BIT | GL_DEPTH_BUFFER_BIT
		| GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TRANSFORM_BIT);

	glDisable(GL_DEPTH_TEST);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.5, 1.5);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(1.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(1.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(0.0f, 1.0f, -1.0f);
	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

int SkeletonGLUT::getWinXRes()
{
	return winxres;
}

int SkeletonGLUT::getWinYRes()
{
	return winyres;
}

bool SkeletonGLUT::setWinXRes(int wxr)
{
	if(wxr <= 0)
	{
		cerr << "attempted to set invalid x resolution " << wxr << endl;
		return false;
	}
	winxres = wxr;
	return true;
}

bool SkeletonGLUT::setWinYRes(int wyr)
{
	if(wyr <= 0)
	{
		cerr << "attempted to set invalid y resolution " << wyr << endl;
		return false;
	}
	winyres = wyr;
	return true;
}

bool SkeletonGLUT::initGLUTGLEW(int * argcp, char ** argv, int wxr, int wyr)
{
	if(doneinit)
		return true;
	glutInit(argcp, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	setWinXRes(wxr);
	setWinYRes(wyr);
	glutInitWindowSize(getWinXRes(), getWinYRes());
	window = glutCreateWindow(argv[0]);
	GLenum errthing = glewInit();
	if(errthing == GLEW_OK)
	{
		doneinit = true;
		cout << "GLEW init passed" << endl;
	}
	else
	{
		cerr << "GLEW init failed" << endl;
		doneinit = false;
	}
	return doneinit;
}

void SkeletonGLUT::mouseFunc(int button, int state, int x, int y)
{
	mouseposx = x;
	mouseposy = y;

	mousechangex = mousechangey = 0;

	mousebutton = button;
}

void SkeletonGLUT::motionFunc(int x, int y)
{
	mousechangex = x - mouseposx;
	mousechangey = y - mouseposy;
	mouseposx = x;
	mouseposy = y;
	int index = 0;
	if(mousebutton == GLUT_LEFT_BUTTON)
		index = 0;
	else if(mousebutton == GLUT_MIDDLE_BUTTON)
		index = 1;
	else if(mousebutton == GLUT_RIGHT_BUTTON)
		index = 2;
	else
		return;
	mouseaccumx[index] += mousechangex;
	mouseaccumy[index] += mousechangey;
}

bool SkeletonGLUT::keyboardFunc(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 'q' : //quit
			cout << "quitting" << endl;
			exit(0);
			break;
		default : 
			return false;
			break;
	}
	return true;
}

void SkeletonGLUT::displayFunc() //it will try to be low-impact
{
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, getWinXRes(), getWinYRes());
	sketchWarning();
	glPopAttrib();
	glutSwapBuffers();
}

int SkeletonGLUT::getMousePosX()
{
	return mouseposx;
}

int SkeletonGLUT::getMousePosY()
{
	return mouseposy;
}

bool SkeletonGLUT::getMouseAccumX(int button, int * accumptr)
{
	switch(button)
	{
		case GLUT_LEFT_BUTTON :
			*accumptr = mouseaccumx[0];
			return true;
			break;
		case GLUT_MIDDLE_BUTTON :
			*accumptr = mouseaccumx[1];
			return true;
			break;
		case GLUT_RIGHT_BUTTON :
			*accumptr = mouseaccumx[2];
			return true;
			break;
		default :
			cerr << "accum is not stored for that button" << endl;
			return false;
			break;
	}
	return false;  //why the hell not
}

bool SkeletonGLUT::getMouseAccumY(int button, int * accumptr)
{
	switch(button)
	{
		case GLUT_LEFT_BUTTON :
			*accumptr = mouseaccumy[0];
			return true;
			break;
		case GLUT_MIDDLE_BUTTON :
			*accumptr = mouseaccumy[1];
			return true;
			break;
		case GLUT_RIGHT_BUTTON :
			*accumptr = mouseaccumy[2];
			return true;
			break;
		default :
			cerr << "accum is not stored for that button" << endl;
			return false;
			break;
	}
	return false;  //why the hell not
}

int SkeletonGLUT::getMouseAccumX(int button)
{
	int temp = 0;
	getMouseAccumX(button, &temp);
	return temp;
}

int SkeletonGLUT::getMouseAccumY(int button)
{
	int temp = 0;
	getMouseAccumY(button, &temp);
	return temp;
}

int SkeletonGLUT::getMouseChangeX()
{
	return mousechangex;
}

int SkeletonGLUT::getMouseChangeY()
{
	return mousechangey;
}

int SkeletonGLUT::getMouseButton()
{
	return mousebutton;
}

float SkeletonGLUT::getMouseAngleX(int button, int revint, float revfloat)
{
	if((revint <= 0) || (revfloat <= 0))
		return 0.0f;
	int temp = getMouseAccumX(button) % revint;
	if(temp < 0)
		temp = revint + temp;
	return (temp / (float) revint) * revfloat;
}

float SkeletonGLUT::getMouseAngleY(int button, int revint, float revfloat)
{
	if((revint <= 0) || (revfloat <= 0))
		return 0.0f;
	int temp = getMouseAccumY(button) % revint;
	if(temp < 0)
		temp = revint + temp;
	return (temp / (float) revint) * revfloat;
}

void SkeletonGLUT::clearBuffer(float r, float g, float b)
{
	glClearColor(r, g, b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SkeletonGLUT::multiplyMatrixMouseAngle(int button, float revpercent)
{
	if(revpercent > 1.0)
		revpercent = 1.0;
	if(revpercent < 0)
		revpercent = 0;
	glRotatef(getMouseAngleY(button, (int) (getWinYRes() / revpercent), 360.0),
		1.0f, 0.0f, 0.0f);
	glRotatef(getMouseAngleX(button, (int) (getWinXRes() / revpercent), 360.0),
		0.0f, 1.0f, 0.0f);
}

void SkeletonGLUT::multiplyProjectionMouseAngle(int button, float fov, float ratio,
		float near, float far, float pullback, float revpercent)
{
	//theoretically I could use glGet and find out the viewport to set ratio automatically
	if(ratio <= 0.0)
		ratio = (float) getWinXRes() / getWinYRes();
	gluPerspective(fov, ratio, near, far);
	glTranslatef(0.0f, 0.0f, -pullback);
	multiplyMatrixMouseAngle(button, revpercent);
}

void SkeletonGLUT::setProjectionMouseAngle(int button, float fov, float ratio, float near, float far,
	float pullback, float revpercent)
{
	//theoretically I could use glGet and find out the viewport to set ratio automatically
	//if(ratio <= 0.0)
		//ratio = (float) getWinXRes() / getWinYRes();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	multiplyProjectionMouseAngle(button, fov, ratio, near, far, pullback,
		revpercent);
	//gluPerspective(fov, ratio, near, far);
	//glTranslatef(0.0f, 0.0f, -pullback);
	//multiplyMatrixMouseAngle(button, revpercent);
}

bool SkeletonGLUT::uniformVec3(GLuint program, const char * name, float a, float b, float c)
{
	GLuint oldprogram;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &oldprogram);
	glUseProgram(program);
	//do this thing
	GLint unifloc = glGetUniformLocation(program, (const GLchar *) name);
	if(unifloc < 0)  //the uniform doesn't exist
	{
		cerr << "uniform " << name << " doesn't exist" << endl;
		glUseProgram(oldprogram);
		return false;
	}
	glUniform3f(unifloc, a, b, c);
	glUseProgram(oldprogram);
	return true;
}

bool SkeletonGLUT::uniformVec3(GLuint program, const char * name, float * vals)
{
	return uniformVec3(program, name, vals[0], vals[1], vals[2]);
}

bool SkeletonGLUT::uniformInt(GLuint program, const char * name, int n)
{
	GLuint oldprogram;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &oldprogram);
	glUseProgram(program);
	//do this thing
	GLint unifloc = glGetUniformLocation(program, (const GLchar *) name);
	if(unifloc < 0)  //the uniform doesn't exist
	{
		cerr << "uniform " << name << " doesn't exist" << endl;
		glUseProgram(oldprogram);
		return false;
	}
	glUniform1i(unifloc, n);
	glUseProgram(oldprogram);
	return true;
}

bool SkeletonGLUT::uniformFloat(GLuint program, const char * name, float n)
{
	GLuint oldprogram;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &oldprogram);
	glUseProgram(program);
	//do this thing
	GLint unifloc = glGetUniformLocation(program, (const GLchar *) name);
	if(unifloc < 0)  //the uniform doesn't exist
	{
		cerr << "uniform " << name << " doesn't exist" << endl;
		glUseProgram(oldprogram);
		return false;
	}
	glUniform1f(unifloc, n);
	glUseProgram(oldprogram);
	return true;
}

bool SkeletonGLUT::uniformSampler2D(GLuint program, const char * name,
	int value)
{
	GLuint oldprogram;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &oldprogram);
	glUseProgram(program);
	//do this thing
	GLint unifloc = glGetUniformLocation(program, (const GLchar *) name);
	if(unifloc < 0)  //the uniform doesn't exist
	{
		cerr << "uniform " << name << " doesn't exist" << endl;
		glUseProgram(oldprogram);
		return false;
	}
	glUniform1i(unifloc, value);
	glUseProgram(oldprogram);
	return true;
}

void SkeletonGLUT::inverseMultiplyMatrixMouseAngle(int button, float revpercent)
{
	if(revpercent > 1.0)
		revpercent = 1.0;
	if(revpercent < 0)
		revpercent = 0;
	glRotatef(-getMouseAngleX(button, (int) (getWinXRes() / revpercent), 360.0),
		0.0f, 1.0f, 0.0f);
	glRotatef(-getMouseAngleY(button, (int) (getWinYRes() / revpercent), 360.0),
		1.0f, 0.0f, 0.0f);
}
