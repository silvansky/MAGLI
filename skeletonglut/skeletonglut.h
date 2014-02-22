//skeletonglut.h

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


//I know it's iffy to include stuff within a header, but I checked and
//all the below are IFDEF-trapped, so things should be alright
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

class SkeletonGLUT;

/*
 * SkeletonGLUT is meant to be a set of common-denominator tools that can
 * be built upon to do even more stuff. The guiding rule I'll use for what
 * is or is not included in here is that things included in SkeletonGLUT
 * should not *force* the user into a particular way of doing things.
 * There is some "forcing" that happens from including the GL headers that
 * include above, but, for example, I don't start the glutMainLoop() in
 * SkeletonGLUT, because the end application may have any of a number of
 * ideas about how to configure all the different glut options before
 * starting the loop.
 */

class SkeletonGLUT
{
private:
	void (*cleanupfunction)();
	bool usecleanupfunction;

	//glut/window parameters
	int winxres;
	int winyres;
	bool doneinit;
	int window;

	//conveniences
	int mouseposx;
	int mouseposy;
	int mouseaccumx[3];  //left, middle, right
	int mouseaccumy[3];  //left, middle, right
	int mousechangex;
	int mousechangey;
	int mousebutton;  //probably GLUT_LEFT_BUTTON, GLUT_RIGHT_BUTTON, or GLUT_MIDDLE_BUTTON

	static const int DEF_WINXRES; // = 800;
	static const int DEF_WINYRES; // = 600;
public:
	SkeletonGLUT();  //does little if anything
	SkeletonGLUT(void (*cupf)());
	virtual ~SkeletonGLUT();
	//virtual in the off-chance that I dynamically
	//allocate child classes, so they can use their
	//destructors to do fun destructy stuff
	
	void setCleanupFunction(void (*cupf)());

	//check support of various features
	bool supportedFramebuffers();
	bool supportedShaders();

	//various helper thingies
	//image loading
	// * outxres, outyres are the dimensions of the buffer to be
	//   allocated and stored at dataptrptr. If either are <= 0,
	//   then it will allocate to exactly the size of the image file.
	bool loadPNM(const char * fname, float ** dataptrptr, int * xresptr,
		int * yresptr, int outxres, int outyres);

	//texture generation
	// * They should preserve as bound whichever texture was bound
	//   before them
	// * The Simple versions will also set some reasonable default
	//   glTexParameteri() settings. They will make it so that enough
	//   is set so that the given texture object is useable.
	bool genTexture2D(GLuint * texobjptr, GLint internalformat,
		GLsizei width, GLsizei height, GLint border, float * data);
	bool genTexture2D(GLuint * texobjptr, GLint internalformat,
		GLsizei width, GLsizei height, GLint border);
		//initializes texture to empty
	bool genSimpleTexture2D(GLuint * texobjptr, int width, int height,
		float * bufptr);
		//no mipmapping, sets reasonable defaults for texture stuff
	bool genSimpleTexture2D(GLuint * texobjptr, int width, int height);
	//initializes texture to empty
	bool genSimpleDepthTexture(GLuint * texobjptr, int width,
		int height);
	bool genSimpleFloatRGBATexture(GLuint * texobjptr, int width,
		int height, float * data);
	bool genSimpleFloatRGBATexture(GLuint * texobjptr, int width,
		int height);
	bool loadGenSimpleTexture2D(GLuint * texobjptr, int width,
		int height, const char * fname);
	bool loadGenSimpleTexture2D(GLuint * texobjptr, const char * fname);

	//framebuffer/renderbuffer generation
	// * They should preserve bound textures/framebuffers/renderbuffers
	// * Again, the Simple ones should be more or less ready-to-use,
	//   except for having to bind the framebuffer
	bool genRenderbuffer(GLuint * rbobjptr, GLenum internalformat,
		int width, int height);
	bool genFramebuffer(GLuint * fbobjptr);
	bool genSimpleFramebufferSubstitute(GLuint * fbobjptr,
		GLuint * rbobjptr, GLuint * texobjptr,
		int width, int height);
		//makes an FBO with depth renderbuffer and texture stored
		//at GL_COLOR_ATTACHMENT0_EXT; alternate place to render to
	bool genSimpleFramebufferAllTextures(GLuint * fbobjptr,
		GLuint * deptexptr, GLuint * texobjptr,
		int width, int height);
	
	//framebuffer tools
	bool checkFramebufferStatus();
	//applied to currently-active framebuffer
	bool checkFramebufferStatus(GLuint fbobj);
	//applied to specified framebuffer. This is safe
	//to run even if in the midst of writing to a
	//different framebuffer

	//shader object/program object generation
	bool loadGenShader(GLuint * shaderptr, GLenum type, const char * fname);
	//create/load/compile
	bool loadGenProgram(GLuint * programptr, GLuint * vsptr,
		const char * vsfname, GLuint * fsptr, const char * fsfname);
		//generates a full shader program based on input files.
		//make either file name argument NULL to not include
		//a shader

	//shader tools
	bool checkProgramStatus(GLuint program); //validate the program

	//drawing presets
	void sketchWarning();
	//sketch means it never does the output, so as to let the caller
	//determine where it will be drawn.
	void sketchSingleTex();
	//draws the currently bound single texture to fill the screen

	//the more controvercial features - things that really take over
	//(but optional to use)
	
	//get/set
	int getWinXRes();
	int getWinYRes();
	bool setWinXRes(int wxr);
	bool setWinYRes(int wyr);

	//init
	virtual bool initGLUTGLEW(int * argcp, char ** argv,
		int wxr, int wyr);
	//does not do glEnable
	//stuff, or set glut*Func, or enter the main loop
	
	//default handlers for glut things
	virtual void mouseFunc(int button, int state, int x, int y);
	virtual void motionFunc(int x, int y);
	virtual bool keyboardFunc(unsigned char key, int x, int y);
	//changed to return bool; after all, this function can't be
	//used directly, so doesn't need to follow directly
	virtual void displayFunc();
	//assumes to set viewport to winxres, winyres

	//mouse things
	int getMousePosX();
	int getMousePosY();
	bool getMouseAccumX(int button, int * accumptr);
	bool getMouseAccumY(int button, int * accumptr);
	int getMouseAccumX(int button);
	int getMouseAccumY(int button);
	int getMouseChangeX();
	int getMouseChangeY();
	int getMouseButton();

	float getMouseAngleX(int button, int revint, float revfloat);
	float getMouseAngleY(int button, int revint, float revfloat);

	//change gl settings and maybe draw
	//for matrix things, 'multiply' ones add on to matrix like any
	//gl or glu matrix function.
	//this compared to 'set' functions that reset the matrix, and
	//possibly a specific one (e.g. perspective if
	//'perspective' in name)
	void clearBuffer(float r, float g, float b);
	void multiplyMatrixMouseAngle(int button, float revpercent);
	void multiplyProjectionMouseAngle(int button, float fov, float ratio,
		float near, float far, float pullback, float revpercent);
	void setProjectionMouseAngle(int button, float fov, float ratio, float near, float far,
		float pullback, float revpercent);
		//set ratio < 0 to have it generated from winxres, winyres
		//revpercent: a mouse move of windowxres will produce
		//revpercent * 360.0 of rotation
	
	//uniform helpers
	bool uniformVec3(GLuint program, const char * name, float a,
		float b, float c);
	bool uniformVec3(GLuint program, const char * name, float * vals);
	bool uniformInt(GLuint program, const char * name, int n);
	bool uniformFloat(GLuint program, const char * name, float n);
	bool uniformSampler2D(GLuint program, const char * name, int value);

	//temporary (?)
	void inverseMultiplyMatrixMouseAngle(int button, float revpercent);
};
