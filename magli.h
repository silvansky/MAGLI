//magli.h

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


#include "skeletonglut/skeletonglut.h"

class MaGLi;

/*
 * Provides the functionality to render a given scene using
 * one-way or mapped-texture autostereograms. The scene to be rendered
 * may be specified by the user, although the scene-rendering function
 * provided to this must satisfy certain restrictions, outlined
 * in the class declaration.
 * 
 * I don't pretend to explain how any significant amount of this works
 * with these comments; a lot of this was for my own reference while coding.
 */

/* NOTES:
 * 
 * IMPORTANT!!!!!!!: Every function that depends on having some particular
 * viewport for the purposes of rendering to some texture MUST set the
 * viewport for the duration of the render.

 * Here, I deftly employ the one-massive-class-with-a-bunch-of-crap-in-it
 * design pattern.
 */

/* Terminology:
 * 
 * Functions starting with "sketch" are functions that perform various
 * OpenGL drawing commands but DO NOT specify the buffer to write
 * the result out to or actually perform the write. These allow the result
 * to be sent to screen, to an FBO, etc. by the caller.
 */

/* Dimensions of textures and such:
 * 
 * All internal textures (depth/texture
 * sources, strips) have the same vertical resolution (stripHeight).
 * Horizontal resolution of things is a function of the stripWidth
 * and the number of strips.
 * 
 * The repeating texture has width 2 * stripWidth. The seed textures
 * (which contain the repeating texture) and the strip textures have
 * width stripWidth.
 * 
 * startingStrips is how many strips wide the normal render is that we
 * want to represent with an autostereogram. The autostereogram render
 * is wider than this, such that the entire scene visible in a normal
 * render of width startingStrips * stripWidth will be visible *with 3D*
 * in the autostereogram render (since the right- and left-most parts
 * of the autostereogram lack the 3D effect).
 * 
 * The autostereogram output has width (startingStrips + 2) * stripWidth.
 * The gap-filling texture has this same width.
 * 
 * The right and left skew textures have the larger
 * width (startingStrips + 6) * stripWidth to provide the extra texture data
 * and gap-identifying information needed when producing the left- and
 * right-most sections of the autostereograms.
 */

class MaGLi
{
private:
	//Provides OpenGL/GLUT/GLEW helper functions galore
	SkeletonGLUT skg;

	//internal dimensions
	int stripWidth;  //one half of "texwidth"
	int stripHeight;
	int startingStrips;

	//texture strips
	GLuint * rightStripTextures;
	GLuint * leftStripTextures;
	int lenStripTextures; //<0 if not ready; should be startingStrips+2
	bool readyStripTextures;  //if ready to use (given other settings)

	//contains repeating texture for non-mapped-texture autostereograms,
	//and off-screen portion of skew render for mapped-texture.
	GLuint seedTextures[2];
	bool allocatedSeedTextures;
	bool readySeedTextures;
	
	//gap texture (for non-mapped-texture)
	GLuint gapTexture;
	bool allocatedGapTexture;
	bool readyGapTexture;

	//texture strip FBO
	GLuint stripFBO;
	GLuint stripRBO;
	bool allocatedStripFBO;
	bool readyStripFBO;
	int attachedIndexStripFBO; //index to stripTextures of what's applied

	//Skew render texture/depth/FBOs
	GLuint rightSkewFBO;
	GLuint rightSkewDepth;
	GLuint rightSkewTexture;
	GLuint leftSkewFBO;
	GLuint leftSkewDepth;
	GLuint leftSkewTexture;
	bool allocatedSkews;
	bool readySkews;

	//One-way shaders
	GLuint rightOneWayVS;
	GLuint rightOneWayFS;
	GLuint rightOneWayProgram;
	GLuint leftOneWayVS;
	GLuint leftOneWayFS;
	GLuint leftOneWayProgram;
	bool allocatedOneWay;
	bool readyOneWay;
	//index to stripTextures of what strip is to be rendered
	int renderedIndexOneWay;  //CURRENTLY UNUSED
	bool renderRightOneWay;

	//Mapped-texture shaders
	GLuint rightMapTexVS;
	GLuint rightMapTexFS;
	GLuint rightMapTexProgram;
	GLuint leftMapTexVS;
	GLuint leftMapTexFS;
	GLuint leftMapTexProgram;
	GLuint seedFillerVS;
	GLuint seedFillerFS;
	GLuint seedFillerProgram;
	GLuint mixerVS;
	GLuint mixerFS;
	GLuint mixerProgram;
	bool allocatedMapTex;
	bool readyMapTex;

	//Scene rendering function
	void (*sceneSketcher)();
	bool specifiedSceneSketcher;
	//REQUIREMENTS for this function:
	// * Doesn't set projection matrix
	// * Doesn't set viewport
	// * Doesn't do the final output of the render

	//State information
	bool readyGL;  //system has requisite openGL abilities

	int displayMode; // 0 = magic-eye; 1 = render; 2 = depth buffer
	bool displayMapTex;

	//Aesthetic variables
	float texturePersistence;
protected:
	//Making things ready
	//Return value indicates whether successful
	bool makeReadyStripTextures();
	bool makeReadySeedTextures();
	bool makeReadyGapTexture();
	bool makeReadyStripFBO();
	bool makeReadySkews();
	bool makeReadyOneWay();
	bool makeReadyMapTex();
	bool makeReadyComponents(); //make everything ready

	//Preparing things
	bool setAttachedIndexStripFBO(int index, bool right);
	bool setAttachedIndexSeedFBO(int index);
	bool setRenderedIndexOneWay(int index);  // not really used at this point

	//Computing
	void setProjection(float shiftMultiplier);
	bool computeSkews();
	GLuint getNearTexture(int stripIndex, bool right);
	GLuint getFarTexture(int stripIndex, bool right);
	bool mapTexRenderSeed(bool near, bool right);
	bool renderStrip(int stripIndex, bool right, bool useMapTex);
	bool computeOneWay(bool right); bool computeOneWay(); bool computeMapTex(bool right);
	bool computeMapTex();

	//Output
	void sketchRawStrips(bool right); // I should be consistent on whether
	void sketchMixedStrips();         // "right" is a param
	void sketchSkew();
	void sketchSkewDepth();

	//Scene sketching
	bool defaultSceneSketcher();
	bool sketchScene();
public:
	//public stuff and such and stuff and like as
	MaGLi();
	~MaGLi();

	//Computing/Output
	bool computeOutput();  //does render work; doesn't draw it anywhere
	bool sketchOutput();  //draws render
	//runs output, but leaves open possibilities like render to
	//texture

	//Setup
	bool initGLUTGLEW(int * argcp, char ** argv);

	//get/setters
	int getStripWidth();
	int getStripHeight();
	int getStartingStrips();
	bool getRenderRightOneWay();

	bool setStripWidth(int sw);
	bool setStripHeight(int sh);
	bool setStartingStrips(int ss);
	bool setSceneSketcher(void (*ss)());
	void setRenderRightOneWay(bool rrow);
	void setTexturePersistence(float tp);

	bool resizeToFit(int desiredWidth, int desiredHeight);

	//Interaction
	void motionFunc(int x, int y);
	void mouseFunc(int button, int state, int x, int y);

	//Status
	bool isReadyGL();
	bool areReadyComponents();
	//are necessary components, like textures, shaders, etc. ready
	//and up-to-date with resolution/other parameters?

	//For custom sketchers
	void rotateCameraByAccums();

	//Constants
	static const int DEF_STRIPWIDTH;
	static const int DEF_STRIPHEIGHT;
	static const int DEF_STARTINGSTRIPS;
	static const float DEF_TEXTUREPERSISTENCE;
	static const int NUM_DISPLAYMODES;

	//For setting rendering mode
	void cycleDisplayMode();
	void setDisplayMode(int dm);
	void toggleMapTex();
};
