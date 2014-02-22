//magli.cpp

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
#include <cstdlib>
#include <cmath>
#include "magli.h"

using namespace std;

//whether to set the texture for non-mapped-texture renders
//from a (hard-coded) file, or to generate it randomly

#define TEXTURE_FROM_FILE false

//+++ constructors

MaGLi::MaGLi()
{
	//Constructs class, sets defaults on some of these variables
	cerr << "Constructing MaGLi" << endl;

	//State
	readyGL = false;

	//Dimensions
	stripWidth = DEF_STRIPWIDTH;
	stripHeight = DEF_STRIPHEIGHT;
	startingStrips = DEF_STARTINGSTRIPS;

	//Textures
	rightStripTextures = NULL;
	leftStripTextures = NULL;
	lenStripTextures = -1;
	readyStripTextures = false;

	seedTextures[0] = seedTextures[1] = 0;
	allocatedSeedTextures = false;
	readySeedTextures = false;

	gapTexture = 0;
	allocatedGapTexture = false;
	readyGapTexture = false;

	//Texture FBOs
	stripFBO = 0;
	stripRBO = 0;
	allocatedStripFBO = false;
	readyStripFBO = false;
	attachedIndexStripFBO = -1;

	//Skews
	rightSkewFBO = rightSkewDepth = rightSkewTexture = 0;
	leftSkewFBO = leftSkewDepth = leftSkewTexture = 0;
	allocatedSkews = false;
	readySkews = false;

	//One-way shaders
	rightOneWayVS = rightOneWayFS = rightOneWayProgram = 0;
	leftOneWayVS = leftOneWayFS = leftOneWayProgram = 0;
	allocatedOneWay = false;
	readyOneWay = false;
	renderedIndexOneWay = -1;
	renderRightOneWay = true;

	//Rendering function
	sceneSketcher = NULL;
	specifiedSceneSketcher = false;

	//Toggling what is displayed
	displayMode = 0;
	displayMapTex = false;

	//Aesthetic parameters
	texturePersistence = DEF_TEXTUREPERSISTENCE;
}

MaGLi::~MaGLi()
{
	//Destructs things
	cerr << "Destructing MaGLi" << endl; 

	//TODO FIXME: this doesn't de-allocate anything. If you care, beware!
}

//+++ protected

bool MaGLi::makeReadyStripTextures()
{
	//Textures
	if(!readyStripTextures)
	{
		if(lenStripTextures > 0)
		{
			//in case the strip FBO had a tex bound
			if(readyStripFBO)
				readyStripFBO = false;
			//delete existing
			glDeleteTextures(lenStripTextures, rightStripTextures);
			glDeleteTextures(lenStripTextures, leftStripTextures);
			delete [] rightStripTextures;
			delete [] leftStripTextures;
			rightStripTextures = NULL;
			leftStripTextures = NULL;
			lenStripTextures = -1;
		}
		//make strip textures
		rightStripTextures = new (nothrow) GLuint[startingStrips + 2];
		if(rightStripTextures == NULL)
		{
			cerr << "FAIL: memory allocation" << endl;
			return false;
		}
		leftStripTextures = new (nothrow) GLuint[startingStrips + 2];
		if(leftStripTextures == NULL)
		{
			cerr << "FAIL: memory allocation" << endl;
			return false;
		}
		lenStripTextures = startingStrips + 2;
		for(int i = 0; i < lenStripTextures; i++)
		{
			rightStripTextures[i] = 0;
			leftStripTextures[i] = 0;
		}
		for(int i = 0; i < lenStripTextures; i++)
		{
			//make sure gen doesn't fail
			if(!skg.genSimpleFloatRGBATexture(&(rightStripTextures[i]),
				stripWidth, stripHeight))
			{
				cerr << "FAIL: texture generation" << endl;
				glDeleteTextures(lenStripTextures, rightStripTextures);
				delete [] rightStripTextures;
				rightStripTextures = NULL;
				lenStripTextures = -1;
				return false;
			}
			if(!skg.genSimpleFloatRGBATexture(&(leftStripTextures[i]),
				stripWidth, stripHeight))
			{
				cerr << "FAIL: texture generation" << endl;
				glDeleteTextures(lenStripTextures, rightStripTextures);
				glDeleteTextures(lenStripTextures, leftStripTextures);
				delete [] rightStripTextures;
				delete [] leftStripTextures;
				rightStripTextures = NULL;
				leftStripTextures = NULL;
				lenStripTextures = -1;
				return false;
			}
		}
		//it worked
		readyStripTextures = true;
	}
	return true;
}

bool MaGLi::makeReadySeedTextures()
{
	//Seed textures
	float * randomData;
	if(!readySeedTextures)
	{
		if(allocatedSeedTextures)
		{
			//remove them
			glDeleteTextures(2, seedTextures);
			allocatedSeedTextures = false;
		}
		//allocate
		seedTextures[0] = seedTextures[1] = 0;
		//cout << stripWidth << ", " << stripHeight << endl;//HACKS!

		for(int i = 0; i < 2; i++)
		{
			#if TEXTURE_FROM_FILE
			int tempx, tempy;
			char * fileName = (char * ) (i == 0 ? "../play/wider_left.ppm" : "../play/wider_right.ppm");
			bool success = skg.loadPNM(fileName, &randomData, &tempx, &tempy, stripWidth, stripHeight);
			#else
			//make rand
			randomData = new (nothrow) float[3 * stripWidth
				* stripHeight];

			if(randomData != NULL)
			{
				for(int i = 0; i < 3 * stripWidth
					* stripHeight; i++)
				{
					randomData[i] = rand() / float(RAND_MAX);
				}
			}
			#endif
			if(!skg.genSimpleFloatRGBATexture(&(seedTextures[i]),
				stripWidth, stripHeight, randomData))
			{
				cerr << "FAIL: texture generation" << endl;
				glDeleteTextures(2, seedTextures);
				allocatedSeedTextures = false;
				return false;
			}
			delete [] randomData;
		}
		//it worked
		allocatedSeedTextures = true;
		readySeedTextures = true;
	}
	return true;
}

bool MaGLi::makeReadyGapTexture()
{
	//Gap texture
	float * randomData = NULL;
	if(!readyGapTexture)
	{
		if(allocatedGapTexture)
		{
			//remove it
			glDeleteTextures(1, &gapTexture);
			gapTexture = 0;
			allocatedGapTexture = false;
		}
		//make random filler
		randomData = new (nothrow) float[3 * stripWidth *
			(startingStrips + 2) * stripHeight];
		if(randomData != NULL)
		{
			for(int i = 0; i < 3 * stripWidth *
				(startingStrips + 2) * stripHeight; i++)
			{
				randomData[i] = rand() / float(RAND_MAX);
				//highlight the gap regions
				//if(i % 3 == 2)
					//randomData[i] = 0;
			}
		}
		// This allowed me to set the texture from a file for
		// screenshots
		#if TEXTURE_FROM_FILE
		if(!skg.loadGenSimpleTexture2D(&gapTexture, stripWidth * 
			(startingStrips + 2), stripHeight, "../play/gap_wide.ppm"))
		{
			cerr << "FAIL: gap texture allocation" << endl;
			return false;
		}
		#else
		if(!skg.genSimpleTexture2D(&gapTexture, stripWidth * 
			(startingStrips + 2), stripHeight, randomData))
		{
			cerr << "FAIL: gap texture allocation" << endl;
			return false;
		}
		#endif
		//delete random filler
		if(randomData != NULL)
			delete [] randomData;
		allocatedGapTexture = true;
		readyGapTexture = true;
	}
	return true;
}

bool MaGLi::makeReadyStripFBO()
{
	//Texture FBOs
	GLuint lastFBO;
	if(!readyStripFBO)
	{
		if(allocatedStripFBO)
		{
			glDeleteRenderbuffersEXT(1, &stripRBO);
			glDeleteFramebuffersEXT(1, &stripFBO);
			stripRBO = stripFBO = 0;
			allocatedStripFBO = false;
		}
		if(!skg.genRenderbuffer(&stripRBO, GL_DEPTH_COMPONENT, stripWidth,
			stripHeight))
		{
			cerr << "FAIL: couldn't generate RBO" << endl;
			return false;
		}
		if(!skg.genFramebuffer(&stripFBO))
		{
			cerr << "FAIL: couldn't generate FBO" << endl;
			glDeleteRenderbuffersEXT(1, &stripRBO);
			stripRBO = 0;
			return false;
		}
		
		glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, (GLint *) &lastFBO);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, stripFBO);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
			GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, stripRBO);
		//no texture has been attached - this happens later
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, lastFBO);
		allocatedStripFBO = true;
		readyStripFBO = true;
		attachedIndexStripFBO = -1;
	}
	return true;
}

bool MaGLi::makeReadySkews()
{
	//Skews
	if(!readySkews)
	{
		if(allocatedSkews)
		{
			//nuking time
			glDeleteFramebuffersEXT(1, &rightSkewFBO);
			glDeleteTextures(1, &rightSkewTexture);
			glDeleteTextures(1, &rightSkewDepth);
			glDeleteFramebuffersEXT(1, &leftSkewFBO);
			glDeleteTextures(1, &leftSkewTexture);
			glDeleteTextures(1, &leftSkewDepth);
			allocatedSkews = false;
			rightSkewFBO = rightSkewDepth = rightSkewTexture = 0;
			leftSkewFBO = leftSkewDepth = leftSkewTexture = 0;
		}
		//Right skew
		if(!skg.genSimpleFramebufferAllTextures(&rightSkewFBO,
			&rightSkewDepth, &rightSkewTexture,
			stripWidth * (startingStrips + 6), stripHeight))
		{
			cerr << "FAIL: right skew FBO gen" << endl;
			//stuff
			return false;
		}
		//Left skew
		if(!skg.genSimpleFramebufferAllTextures(&leftSkewFBO,
			&leftSkewDepth, &leftSkewTexture,
			stripWidth * (startingStrips + 6), stripHeight))
		{
			cerr << "FAIL: left skew FBO gen" << endl;
			glDeleteFramebuffersEXT(1, &rightSkewFBO);
			glDeleteTextures(1, &rightSkewTexture);
			glDeleteTextures(1, &rightSkewDepth);
			rightSkewFBO = rightSkewDepth = rightSkewTexture = 0;
			//stuff
			return false;
		}

		//It worked
		allocatedSkews = true;
		readySkews = true;
	}
	return true;
}

bool MaGLi::makeReadyOneWay()
{
	//One-way
	if(!readyOneWay)
	{
		renderedIndexOneWay = -1;
		if(!allocatedOneWay)
		{
			//load/allocate them
			if(!skg.loadGenProgram(&leftOneWayProgram,
				&leftOneWayVS, "magli_leftoneway.vs",
				&leftOneWayFS, "magli_leftoneway.fs"))
			{
				cerr << "FAIL: left one-way shader load"
					<< endl;
				return false;
			}
			//ALSO LOAD RIGHT SHADERS
			if(!skg.loadGenProgram(&rightOneWayProgram,
				&rightOneWayVS, "magli_rightoneway.vs",
				&rightOneWayFS, "magli_rightoneway.fs"))
			{
				cerr << "FAIL: right one-way shader load"
					<< endl;
				return false;
			}
		}
		//ready them
		//well, I think they are ready
		allocatedOneWay = true;
		readyOneWay = true;
	}
	return true;
}

bool MaGLi::makeReadyMapTex()
{
	if(!readyMapTex)
	{
		if(!allocatedMapTex)
		{
			//load/allocate
			if(!skg.loadGenProgram(&seedFillerProgram,
				&seedFillerVS, "magli_seedfiller.vs",
				&seedFillerFS, "magli_seedfiller.fs"))
			{
				cerr << "FAIL: seed filler shader load"
					<< endl;
				return false;
			}
			if(!skg.loadGenProgram(&mixerProgram,
				&mixerVS, "magli_mixer.vs",
				&mixerFS, "magli_mixer.fs"))
			{
				cerr << "FAIL: mixer shader load"
					<< endl;
				return false;
			}
			if(!skg.loadGenProgram(&rightMapTexProgram,
				&rightMapTexVS, "magli_rightmaptex.vs",
				&rightMapTexFS, "magli_rightmaptex.fs"))
			{
				cerr << "FAIL: right map tex shader load"
					<< endl;
				return false;
			}
			if(!skg.loadGenProgram(&leftMapTexProgram,
				&leftMapTexVS, "magli_leftmaptex.vs",
				&leftMapTexFS, "magli_leftmaptex.fs"))
			{
				cerr << "FAIL: left map tex shader load"
					<< endl;
				return false;
			}
		}
		allocatedMapTex = true;
		readyMapTex = true;
	}
	return true;
}

bool MaGLi::makeReadyComponents()
{
	GLuint lastFBO;
	if(areReadyComponents())
	{
		return true;
	}
	//Check openGL
	if(!isReadyGL())
	{
		cerr << "FAIL: cannot ready components - no GL" << endl;
		return false;
	}
	if(!makeReadyStripTextures())
		return false;
	if(!makeReadySeedTextures())
		return false;
	if(!makeReadyGapTexture())
		return false;
	if(!makeReadyStripFBO())
		return false;
	if(!makeReadySkews())
		return false;
	if(!makeReadyOneWay())
		return false;
	if(!makeReadyMapTex())
		return false;
	//good
	return true;
}

bool MaGLi::setAttachedIndexStripFBO(int index, bool right)
{
	if(!makeReadyComponents())
	{
		cerr << "FAIL: components not ready" << endl;
		return false;
	}

	if((index < 0) || (index >= lenStripTextures))
	{
		cerr << "FAIL: index out of range" << endl;
		return false;
	}

	GLuint * stripTextures = (right ? rightStripTextures : leftStripTextures);

	GLuint lastFBO;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, (GLint *) &lastFBO);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, stripFBO);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
		GL_TEXTURE_2D, stripTextures[index], 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, lastFBO);

	return true;
}

bool MaGLi::setAttachedIndexSeedFBO(int index)
{
	//attach one of the seed textures to the strip fbo
	if(!makeReadyComponents())
	{
		cerr << "FAIL: components not ready" << endl;
		return false;
	}
	if((index < 0) || (index > 1))
	{
		cerr << "FAIL: index out of range" << endl;
		return false;
	}

	GLuint lastFBO;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, (GLint *) &lastFBO);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, stripFBO);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
		GL_TEXTURE_2D, seedTextures[index], 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, lastFBO);
}

bool MaGLi::setRenderedIndexOneWay(int index)
{
	if(!makeReadyComponents())
	{
		cerr << "FAIL: components not ready" << endl;
		return false;
	}
	if((index < 0) || (index >= lenStripTextures))
	{
		cerr << "FAIL: index out of range" << endl;
		return false;
	}
	renderedIndexOneWay = index;
	return true;
}

void MaGLi::setProjection(float shiftMultiplier)
{
	//set the projection for the skewing

	//In openGL, subsequent multiplications are multiplied
	//on to the right of the existing matrix
	//as in, moving from A to AB

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	int relevantStrips = startingStrips + 6;
	int effectiveWidth = stripWidth * relevantStrips;

	int fov = 60;
	float pi = 3.1415926535;
	float zNear = 5.0;
	float zFar = 100.0;

	//calculate shiftamount - the amount in eye coords to move
	float f = (1.0 / tan((fov / 2.0) * (2.0 * pi / 360)));
	float aspect = float(effectiveWidth) / stripHeight;
	float shiftAmount = -(2 * aspect) /
		(relevantStrips * f * (1.0 / zNear - 1.0 / zFar));
	
	shiftAmount *= shiftMultiplier;

	//finally, shift the far depth out by the appropriate amount
	//shift by shiftmultiplier * texwidthndc (texwidthndc = 4.0 / PRJ_VS)
	glTranslatef(shiftMultiplier * 4.0 / relevantStrips, 0.0f, 0.0f);

	//(second to) last thing to be applied is the shift in ncd to keep zfar
	//in place.
	//in ndc, something at PRJ_ZFAR is shifted by
	//((f/aspect) * shiftamount) / PRJ_ZFAR, so undo this
	glTranslatef(-((f / aspect) * shiftAmount) / zFar, 0.0f, 0.0f);


	//apply projection
	gluPerspective(fov, aspect, zNear, zFar);
	//do the shiftamount
	glTranslatef(shiftAmount, 0.0f, 0.0f);
}

bool MaGLi::computeSkews()
{
	//says what it does
	if(!makeReadyComponents())
		return false;
	
	//viewport
	glViewport(0, 0, stripWidth * (startingStrips + 6), stripHeight);
	
	//right and left skews
	GLuint lastFBO;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, (GLint *) &lastFBO);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rightSkewFBO);
	setProjection(0.5);  //could make a variable for this
	sketchScene();
	glFlush();
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, leftSkewFBO);
	setProjection(-0.5);
	sketchScene();
	glFlush();
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, lastFBO);
}

GLuint MaGLi::getNearTexture(int stripIndex, bool right)
{
	//gives the appropriate texture
	if(!makeReadyComponents())
		return 0;
	if((stripIndex < 0) || (stripIndex >= lenStripTextures))
	{
		cerr << "FAIL: index out of range" << endl;
		return 0;
	}
	if(right)
	{
		//seeks back to farther to the left
		if(stripIndex == 0)
			return seedTextures[1];
		else
			return rightStripTextures[stripIndex - 1];
	}
	else
	{
		//seeks back to farther to the right
		if(stripIndex == lenStripTextures - 1)
			return seedTextures[0];
		else
			return leftStripTextures[stripIndex + 1];
	}
}

GLuint MaGLi::getFarTexture(int stripIndex, bool right)
{
	//gives the appropriate texture
	if(!makeReadyComponents())
		return 0;
	if((stripIndex < 0) || (stripIndex >= lenStripTextures))
	{
		cerr << "FAIL: index out of range" << endl;
		return 0;
	}
	if(right)
	{
		//seeks back to farther to the left
		if(stripIndex == 0)
			return seedTextures[0];
		else if(stripIndex == 1)
			return seedTextures[1];
		else
			return rightStripTextures[stripIndex - 2];
	}
	else
	{
		//seeks back to farther to the right
		if(stripIndex == lenStripTextures - 1)
			return seedTextures[1];
		else if(stripIndex == lenStripTextures - 2)
			return seedTextures[0];
		else
			return leftStripTextures[stripIndex + 2];
	}
}

bool MaGLi::mapTexRenderSeed(bool near, bool right)
{
	if(!makeReadyComponents())
	{
		cerr << "FAIL: couldn't ready components in "
			<< "mapTexRenderSeed" << endl;
		return false;
	}
	// these variable settings only have a prayer of making sense if you
	// look at the diagram for this, in a notebook in my room somewhere.
	// Alas.

	int texIndex = (((right && near) || (!right && !near)) ? 1 : 0);
	float direction = (right ? -1 : 1);
	float depthOffset = ((texIndex == 0) ? 1 : 0);
	float startX = (right ? 2 : 0) - texIndex;
	//float sampleX = (right ? 0 : 1);
	// ^^^^ THOSE ARE UNAMBIGUOUSLY WRONG
	//float sampleX = 0.5;
	float sampleX = (right ? 2 : startingStrips + 4) / float(startingStrips + 6);

	// now we either need to commandeer the strip fbo (most likely)
	// or make another one.

	//bind fbo
	setAttachedIndexSeedFBO(texIndex);

	GLuint lastFBO;
	GLuint lastProgram;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, (GLint *) &lastFBO);
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &lastProgram);

	//bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, (right ? rightSkewDepth : leftSkewDepth));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, (right ? rightSkewTexture : leftSkewTexture));
	glEnable(GL_TEXTURE_2D);

	//set uniforms
	//uniform float direction;
	//uniform float depthOffset;
	//uniform float startX;  //the x at which weight value is 1
	//uniform float sampleX;  //where to sample to determine depth
	//uniform sampler2D depthTex;

	bool uniformSuccess = true;

	if(!skg.uniformSampler2D(seedFillerProgram, "depthTex", 0))
	{
		cerr << "FAIL: assigning uniform depthTex" << endl;
		uniformSuccess = false;
	}
	if(!skg.uniformSampler2D(seedFillerProgram, "skewTex", 1))
	{
		cerr << "FAIL: assigning uniform skewTex" << endl;
		uniformSuccess = false;
	}
	if(!skg.uniformFloat(seedFillerProgram, "direction", direction))
	{
		cerr << "FAIL: assigning uniform direction" << endl;
		uniformSuccess = false;
	}
	if(!skg.uniformFloat(seedFillerProgram, "depthOffset", depthOffset))
	{
		cerr << "FAIL: assigning uniform depthOffset" << endl;
		uniformSuccess = false;
	}
	if(!skg.uniformFloat(seedFillerProgram, "startX", startX))
	{
		cerr << "FAIL: assigning uniform startX" << endl;
		uniformSuccess = false;
	}
	if(!skg.uniformFloat(seedFillerProgram, "sampleX", sampleX))
	{
		cerr << "FAIL: assigning uniform sampleX" << endl;
		uniformSuccess = false;
	}

	//check program
	if(!skg.checkProgramStatus(seedFillerProgram))
	{
		cerr << "FAIL: error with program status" << endl;
		uniformSuccess = false;
	}

	//handle if uniforms fail
	if(!uniformSuccess)
	{
		//return to happy state
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);
		return false;
	}

	//bind fbo
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, stripFBO);
	glUseProgram(seedFillerProgram);

	//viewports/projection, etc.
	glViewport(0, 0, stripWidth, stripHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.5, 1.5);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//draw the thing - set texture coords
	int leftIndex = texIndex + (right ? 0 : startingStrips + 4);
	int rightIndex = leftIndex + 1;
	float leftX = leftIndex / float(startingStrips + 6);
	float rightX = rightIndex / float(startingStrips + 6);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
		glMultiTexCoord2f(GL_TEXTURE1, leftX, 0.0f);
		glVertex3f(0.0f, 0.0f, -1.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
		glMultiTexCoord2f(GL_TEXTURE1, rightX, 0.0f);
		glVertex3f(1.0f, 0.0f, -1.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f);
		glMultiTexCoord2f(GL_TEXTURE1, rightX, 1.0f);
		glVertex3f(1.0f, 1.0f, -1.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
		glMultiTexCoord2f(GL_TEXTURE1, leftX, 1.0f);
		glVertex3f(0.0f, 1.0f, -1.0f);
	glEnd();

	//draw out
	glFlush();
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

	glDisable(GL_TEXTURE_2D);

	//bind last framebuffer and shader program
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, lastFBO);
	glUseProgram(lastProgram);
	
	//return true
	return true;
}

bool MaGLi::renderStrip(int stripIndex, bool right, bool useMapTex)
{
	if(!makeReadyComponents())
	{
		cerr << "FAIL: couldn't ready components in "
			<< "renderStrip" << endl;
		return false;
	}
	if((stripIndex < 0) || (stripIndex >= lenStripTextures))
	{
		cerr << "FAIL: stripIndex out of range" << endl;
		return false;
	}

	bool uniformSuccess = true;

	//set the texture in stripFBO
	setAttachedIndexStripFBO(stripIndex, right);
	setRenderedIndexOneWay(stripIndex); //currently does nothing interesting

	//bind and do nifty stuff with FBO
	GLuint lastFBO;
	GLuint lastProgram;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, (GLint *) &lastFBO);
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &lastProgram);

	GLuint chosenProgram = (useMapTex
		? (right ? rightMapTexProgram : leftMapTexProgram)
		: (right ? rightOneWayProgram : leftOneWayProgram));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, getNearTexture(stripIndex, right));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, getFarTexture(stripIndex, right));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, (right ? rightSkewDepth
		: leftSkewDepth));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, (right ? leftSkewDepth
		: rightSkewDepth));
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, (useMapTex ?
		(right ? rightSkewTexture : leftSkewTexture) : gapTexture));
	glEnable(GL_TEXTURE_2D);
	
	//uniforms
	if(!skg.uniformSampler2D(chosenProgram, "nearTex", 0))
	{
		cerr << "FAIL: assigning uniform nearTex" << endl;
		uniformSuccess = false;
	}
	if(!skg.uniformSampler2D(chosenProgram, "farTex", 1))
	{
		cerr << "FAIL: assigning uniform farTex" << endl;
		uniformSuccess = false;
	}
	if(!skg.uniformSampler2D(chosenProgram, "depthTex", 2))
	{
		cerr << "FAIL: assigning uniform depthTex" << endl;
		uniformSuccess = false;
	}
	if(!skg.uniformSampler2D(chosenProgram, "otherDepthTex", 3))
	{
		cerr << "FAIL: assigning uniform otherDepthTex" << endl;
		uniformSuccess = false;
	}
	if(!skg.uniformFloat(chosenProgram, "texWidthDepth",
		2.0 / (startingStrips + 6)))
	{
		cerr << "FAIL: assigning uniform depthTex" << endl;
		uniformSuccess = false;
	}
	if(useMapTex)
	{
		if(!skg.uniformSampler2D(chosenProgram, "skewTex", 4))
		{
			cerr << "FAIL: assigning uniform skewTex" << endl;
			uniformSuccess = false;
		}
		if(!skg.uniformFloat(chosenProgram, "texturePersistence",
			texturePersistence))
		{
			cerr << "FAIL: assigning uniform texturePersistence" << endl;
			uniformSuccess = false;
		}
	}
	else
	{
		if(!skg.uniformSampler2D(chosenProgram, "gapTex", 4))
		{
			cerr << "FAIL: assigning uniform gapTex" << endl;
			uniformSuccess = false;
		}
	}

	if(!skg.checkProgramStatus(chosenProgram))
	{
		cerr << "FAIL: error with program status" << endl;
		uniformSuccess = false;
	}
	
	if(!uniformSuccess)
	{
		//return to happy state
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE3);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);
		return false;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, stripFBO);
	glUseProgram(chosenProgram);

	glViewport(0, 0, stripWidth, stripHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.5, 1.5);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//draw
	glColor3f(1.0f, 1.0f, 1.0f);
	float startx = (2 + stripIndex) / float(startingStrips + 6);
	float endx = (2 + stripIndex + 1) / float(startingStrips + 6);
	glBegin(GL_QUADS);
		glMultiTexCoord2f(GL_TEXTURE0, startx, 0.0f);
		glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, -1.0f);
		glMultiTexCoord2f(GL_TEXTURE0, endx, 0.0f);
		glMultiTexCoord2f(GL_TEXTURE1, 1.0f, 0.0f);
		glVertex3f(1.0f, 0.0f, -1.0f);
		glMultiTexCoord2f(GL_TEXTURE0, endx, 1.0f);
		glMultiTexCoord2f(GL_TEXTURE1, 1.0f, 1.0f);
		glVertex3f(1.0f, 1.0f, -1.0f);
		glMultiTexCoord2f(GL_TEXTURE0, startx, 1.0f);
		glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 1.0f);
		glVertex3f(0.0f, 1.0f, -1.0f);
	glEnd();

	glFlush();
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE3);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE2);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, lastFBO);
	glUseProgram(lastProgram);

	return true;
}

bool MaGLi::computeOneWay(bool right)
{
	if(right)
	{
		for(int i = 0; i < lenStripTextures; i++)
		{
			renderStrip(i, true, false);
		}
	}
	else
	{
		for(int i = lenStripTextures - 1; i >= 0; i--)
		{
			renderStrip(i, false, false);
		}
	}
	return true;
}

bool MaGLi::computeOneWay()
{
	return computeOneWay(renderRightOneWay);
}

bool MaGLi::computeMapTex(bool right)
{
	mapTexRenderSeed(true, right);
	mapTexRenderSeed(false, right);
	if(right)
	{
		for(int i = 0; i < lenStripTextures; i++)
		{
			renderStrip(i, true, true);
		}
	}
	else
	{
		for(int i = lenStripTextures - 1; i >= 0; i--)
		{
			renderStrip(i, false, true);
		}
	}
	return true;
}

bool MaGLi::computeMapTex()
{
	bool success = true;
	success = success && computeMapTex(true);
	success = success && computeMapTex(false);
	return success;
}

void MaGLi::sketchRawStrips(bool right)
{
	//sketch all the meant-to-be-visible strips as-are
	//glPushAttrib(GL_VIEWPORT_BIT | GL_TEXTURE_BIT | GL_DEPTH_BUFFER_BIT
		//| GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TRANSFORM_BIT);
	
	//glDisable(GL_DEPTH_TEST);


	GLuint * stripTextures = (right ? rightStripTextures : leftStripTextures);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.5, 1.5);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	float xstart = 0.0f;
	float xend = 1.0f;

	glColor3f(1.0f, 1.0f, 1.0f);
	for(int i = 0; i < lenStripTextures; i++)
	{
		//glColor3f(1.0f, 1.0f, i / float(lenStripTextures));
		glBindTexture(GL_TEXTURE_2D, stripTextures[i]);
		xend = (i + 1) / float(lenStripTextures);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(xstart, 0.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(xend, 0.0f, -1.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(xend, 1.0f, -1.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(xstart, 1.0f, -1.0f);
		glEnd();
		xstart = xend;
	}
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

void MaGLi::sketchMixedStrips()
{
	//sketch the strips mixed together as needed by mapped-texture
	GLuint lastProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *) &lastProgram);

	bool uniformSuccess = true;

	if(!skg.uniformSampler2D(mixerProgram, "rightStrip", 0))
	{
		cerr << "FAIL: assigning uniform rightStrip" << endl;
		uniformSuccess = false;
	}
	if(!skg.uniformSampler2D(mixerProgram, "leftStrip", 1))
	{
		cerr << "FAIL: assigning uniform leftStrip" << endl;
		uniformSuccess = false;
	}

	if(!skg.checkProgramStatus(mixerProgram))
	{
		cerr << "FAIL: error with program status" << endl;
		uniformSuccess = false;
	}

	if(!uniformSuccess)
	{
		return;
	}

	glUseProgram(mixerProgram);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.5, 1.5);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	float xstart = 0.0f;
	float xend = 1.0f;

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);

	glColor3f(1.0f, 1.0f, 1.0f);
	for(int i = 0; i < lenStripTextures; i++)
	{
		//glColor3f(1.0f, 1.0f, i / float(lenStripTextures));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rightStripTextures[i]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, leftStripTextures[i]);

		xend = (i + 1) / float(lenStripTextures);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(xstart, 0.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(xend, 0.0f, -1.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(xend, 1.0f, -1.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(xstart, 1.0f, -1.0f);
		glEnd();
		xstart = xend;
	}
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();

	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);

	glUseProgram(lastProgram);

	//and pray
}

void MaGLi::sketchSkew()
{
	//Sketch the relevant skew
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.5, 1.5);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	float xstart;
	float xend;

	glColor3f(1.0f, 1.0f, 1.0f);
	xstart = 2.0 / (startingStrips + 6);
	xend = (startingStrips + 4.0) / (startingStrips + 6);
	GLuint desiredSkew = (renderRightOneWay ?
		rightSkewTexture : leftSkewTexture);
	glBindTexture(GL_TEXTURE_2D, desiredSkew);
	glBegin(GL_QUADS);
		glTexCoord2f(xstart, 0.0f);
		glVertex3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(xend, 0.0f);
		glVertex3f(1.0f, 0.0f, -1.0f);
		glTexCoord2f(xend, 1.0f);
		glVertex3f(1.0f, 1.0f, -1.0f);
		glTexCoord2f(xstart, 1.0f);
		glVertex3f(0.0f, 1.0f, -1.0f);
	glEnd();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

void MaGLi::sketchSkewDepth()
{
	//Sketch the relevant skew depth
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.5, 1.5);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	float xstart;
	float xend;

	glColor3f(1.0f, 1.0f, 1.0f);
	xstart = 2.0 / (startingStrips + 6);
	xend = (startingStrips + 4.0) / (startingStrips + 6);
	GLuint desiredSkew = (renderRightOneWay ?
		rightSkewDepth : leftSkewDepth);
	glBindTexture(GL_TEXTURE_2D, desiredSkew);
	glBegin(GL_QUADS);
		glTexCoord2f(xstart, 0.0f);
		glVertex3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(xend, 0.0f);
		glVertex3f(1.0f, 0.0f, -1.0f);
		glTexCoord2f(xend, 1.0f);
		glVertex3f(1.0f, 1.0f, -1.0f);
		glTexCoord2f(xstart, 1.0f);
		glVertex3f(0.0f, 1.0f, -1.0f);
	glEnd();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

bool MaGLi::defaultSceneSketcher()
{
	//sketch *something*
	//here
	//sketches inner, without setting projection matrix
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -50.0f);
	//skg.multiplyMatrixMouseAngle(GLUT_RIGHT_BUTTON, 0.2);
	glRotatef(skg.getMouseAngleY(GLUT_RIGHT_BUTTON, (int) (skg.getWinYRes() / 1), 360.0),
		1.0f, 0.0f, 0.0f);
	glRotatef(skg.getMouseAngleY(GLUT_LEFT_BUTTON, (int) (skg.getWinYRes() / 0.2), 360.0),
		1.0f, 0.0f, 0.0f);
	glRotatef(skg.getMouseAngleX(GLUT_RIGHT_BUTTON, (int) (skg.getWinXRes() / 1), 360.0),
		0.0f, 1.0f, 0.0f);
	glRotatef(skg.getMouseAngleX(GLUT_LEFT_BUTTON, (int) (skg.getWinXRes() / 0.2), 360.0),
		0.0f, 1.0f, 0.0f);
	//let's draw some stuff
	glPushMatrix();
	glTranslatef(-7.0f, 14.0f, -18.0f);
	glColor3f(0.5f, 0.5f, 0.0f);
	glutSolidTeapot(8.0);
	glPopMatrix();
	glPushMatrix();
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
		glVertex3f(0.0f, -1.0f, -100.0f);
		glVertex3f(2.0f, -1.0f, -100.0f);
		glVertex3f(2.0f, -1.0f, 100.0f);
		glVertex3f(0.0f, -1.0f, 100.0f);
	glEnd();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(20.0f, 0.0f, 0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glutSolidSphere(10.0f, 10, 10);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(-30.0f, 10.0f, -3.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glutSolidSphere(7.0f, 10, 10);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(3.0f, -4.0f, 25.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	glutSolidTorus(6.0f, 13.0f, 10, 10);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(14.0f, 23.0f, -10.0f);
	glColor3f(1.0f, 1.0f, 0.0f);
	glutSolidSphere(1.0f, 10, 10);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0f, -20.0f, 0.0f);
	glColor3f(0.5f, 0.5f, 0.5f);
	float xStart = -100;
	float xEnd = 100;
	float zStart = -100;
	float zEnd = 100;
	float stripesPerSide = 11;
	float stripeRadius = 2.5;
	glBegin(GL_QUADS);
		glVertex3f(-xStart, 0.0f, -zStart);
		glVertex3f(xStart, 0.0f, -zStart);
		glVertex3f(xStart, 0.0f, zStart);
		glVertex3f(-xStart, 0.0f, zStart);
	glEnd();
	glColor3f(0.0f, 0.0f, 0.0f);
	for(int i = 0; i < stripesPerSide; i++)
	{
		float tStart = xStart + (i / float(stripesPerSide - 1)) * (xEnd - xStart);
		//float tEnd = ((i+1) / float(stripesPerSide - 1)) * (xEnd - xStart);
		glBegin(GL_QUADS);
			glVertex3f(tStart - stripeRadius, 0.1f, -zStart);
			glVertex3f(tStart + stripeRadius, 0.1f, -zStart);
			glVertex3f(tStart + stripeRadius, 0.1f, zStart);
			glVertex3f(tStart - stripeRadius, 0.1f, zStart);
		glEnd();
		tStart = zStart + (i / float(stripesPerSide - 1)) * (zEnd - zStart);
		glBegin(GL_QUADS);
			glVertex3f(-xStart, 0.1f, tStart - stripeRadius);
			glVertex3f(xStart, 0.1f, tStart - stripeRadius);
			glVertex3f(xStart, 0.1f, tStart + stripeRadius);
			glVertex3f(-xStart, 0.1f, tStart + stripeRadius);
		glEnd();
	}
	glPopMatrix();
}

bool MaGLi::sketchScene()
{
	//sketches the scene
	//DANGER: doesn't properly return true or false or whatever
	if(specifiedSceneSketcher)
	{
		sceneSketcher();
	}
	else
		defaultSceneSketcher();
	return true;
}

//+++ public

bool MaGLi::computeOutput()
{
	//Computes, but doesn't draw out, the render.
	if(!isReadyGL())
	{
		cerr << "FAIL: cannot run render: no openGL" << endl;
		return false;
	}
	if(!makeReadyComponents())
	{
		cerr << "FAIL: no render: components un-ready-able" << endl;
		return false;
	}

	//this is where the meaty algorithmic bits will go / be called from

	computeSkews();

	if(displayMapTex)
		computeMapTex();
	else
		computeOneWay();

	return true;
}

bool MaGLi::sketchOutput()
{
	//Draws the output of the program. It does not do the final
	//output to screen, so that the output may actually be to
	//a texture or something else.

	//IMPORTANT: this DOES NOT calculate the output. It only
	//displays the last calculated output.

	//There will be issues of whether the output resolution matches
	//up precisely with the scale and number of texture strips.
	//That is all pretty secondary and can be handled later.
	if(!isReadyGL())
		return false;
	if(!areReadyComponents())
	{
		skg.sketchWarning();
		return false;
	}
	//do something cool
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	glViewport(0, 0, stripWidth * (startingStrips + 2), stripHeight);
	//glBindTexture(GL_TEXTURE_2D, rightSkewDepth);
	//skg.sketchSingleTex();
	if(displayMode == 0)
	{
		if(displayMapTex)
			sketchMixedStrips();
		else
			sketchRawStrips(renderRightOneWay);
	}
	else if(displayMode == 1)
		sketchSkew();
	else if(displayMode == 2)
		sketchSkewDepth();
	else
	{
		glDisable(GL_TEXTURE_2D);
		skg.sketchWarning();
		glEnable(GL_TEXTURE_2D);
	}
	glDisable(GL_TEXTURE_2D);
	return true;
}

bool MaGLi::initGLUTGLEW(int * argcp, char ** argv)
{
	//basically calls skg thing
	int wxr = stripWidth * (startingStrips + 2);
	int wyr = stripHeight;
	cout << "wxrwyr " << wxr << ", " << wyr << endl;
	readyGL = skg.initGLUTGLEW(argcp, argv, wxr, wyr);
	if(!readyGL)
	{
		cerr << "FAIL: during glut/glew init" << endl;
	}
	else
	{
		if(!skg.supportedFramebuffers() || !skg.supportedShaders())
		{
			cerr << "FAIL: system does not support framebuffers"
				<< " and/or shaders" << endl;
			readyGL = false;
		}
	}
	return readyGL;
}

int MaGLi::getStripWidth()
{
	return stripWidth;
}

int MaGLi::getStripHeight()
{
	return stripHeight;
}

int MaGLi::getStartingStrips()
{
	return startingStrips;
}

bool MaGLi::getRenderRightOneWay()
{
	return renderRightOneWay;
}

bool MaGLi::setStripWidth(int sw)
{
	if(sw < 5)
	{
		cerr << "FAIL: strip width too small" << endl;
		return false;
	}
	stripWidth = sw;
	//What depends on stripWidth?
	readyStripTextures = false;
	readySeedTextures = false;
	readyGapTexture = false;
	readyStripFBO = false;
	readySkews = false;
	return true;
}

bool MaGLi::setStripHeight(int sh)
{
	if(sh < 5)
	{
		cerr << "FAIL: strip height too small" << endl;
		return false;
	}
	stripHeight = sh;
	//What depends on stripHeight?
	readyStripTextures = false;
	readySeedTextures = false;
	readyGapTexture = false;
	readyStripFBO = false;
	readySkews = false;
	return true;
}

bool MaGLi::setStartingStrips(int ss)
{
	if(ss < 2)
	{
		cerr << "FAIL: starting strips too small" << endl;
		return false;
	}
	startingStrips = ss;
	//What depends on startingStrips?
	readyStripTextures = false;
	readyGapTexture = false;
	readySkews = false;
	return true;
}

bool MaGLi::setSceneSketcher(void (*ss)())
{
	if(ss == NULL)
	{
		sceneSketcher = NULL;
		specifiedSceneSketcher = false;
	}
	else
	{
		sceneSketcher = ss;
		specifiedSceneSketcher = true;
	}
	return true;
}

void MaGLi::setRenderRightOneWay(bool rrow)
{
	renderRightOneWay = rrow;
}

void MaGLi::setTexturePersistence(float tp)
{
	if(tp < 0)
		texturePersistence = 0;
	else if(tp > 1)
		texturePersistence = 1;
	else
		texturePersistence = tp;
}

bool MaGLi::resizeToFit(int desiredWidth, int desiredHeight)
{
	int desiredStrips = desiredWidth
		/ getStripWidth() + 1;
	setStripHeight(desiredHeight);
	setStartingStrips(desiredStrips - 2);
	return true;
}

void MaGLi::motionFunc(int x, int y)
{
	//passthrough to skg
	skg.motionFunc(x, y);
}

void MaGLi::mouseFunc(int button, int state, int x, int y)
{
	//passthrough to skg
	skg.mouseFunc(button, state, x, y);
}

bool MaGLi::isReadyGL()
{
	//Tells whether the class is ready for OpenGL operations
	return readyGL;
}

bool MaGLi::areReadyComponents()
{
	//Returns true if all the machinery used by the class seems
	//operable and up-to-date with settings (i.e. the textures,
	//fbos, shaders are allocated, and correctly), and false otherwise.

	//WARNING: it is up to the functions that modify parameters like
	//strip dimensions/counts to update component readiness bools.

	if(!isReadyGL() || !readyStripTextures || !readySeedTextures
		|| !readyGapTexture || !readyStripFBO || !readySkews
		|| !readyOneWay || !readyMapTex)
		return false;
	
	return true;
}

void MaGLi::rotateCameraByAccums()
{

	glRotatef(skg.getMouseAngleY(GLUT_RIGHT_BUTTON, (int) (skg.getWinYRes() / 1), 360.0),
		1.0f, 0.0f, 0.0f);
	glRotatef(skg.getMouseAngleY(GLUT_LEFT_BUTTON, (int) (skg.getWinYRes() / 0.2), 360.0),
		1.0f, 0.0f, 0.0f);
	glRotatef(skg.getMouseAngleX(GLUT_RIGHT_BUTTON, (int) (skg.getWinXRes() / 1), 360.0),
		0.0f, 1.0f, 0.0f);
	glRotatef(skg.getMouseAngleX(GLUT_LEFT_BUTTON, (int) (skg.getWinXRes() / 0.2), 360.0),
		0.0f, 1.0f, 0.0f);
}

void MaGLi::cycleDisplayMode()
{
	displayMode = (displayMode + 1) % 3;
}

void MaGLi::setDisplayMode(int dm)
{
	if((dm >= 0) && (dm < MaGLi::NUM_DISPLAYMODES))
	{
		displayMode = dm;
	}
}

void MaGLi::toggleMapTex()
{
	displayMapTex = !displayMapTex;

	readySeedTextures = false;
}
