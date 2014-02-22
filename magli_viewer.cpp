//magli_viewer.cpp

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
#include <cstdio>
#include <cmath>
#include "magli.h"
#include "pnm_tools/pnm_tools.h"

using namespace std;

MaGLi mg;

bool windowResized = true;
int resizeCounter = 0;
int desiredWidth = 1000;
int desiredHeight = 600;
const int MAX_RESIZE_COUNTER = 15;

bool isMiddleMouse = false;
int screenshotsWanted = 0;

//GL functions
void displayFunc();
void reshapeFunc(int width, int height);
void keyboardFunc(unsigned char key, int x, int y);
void motionFunc(int x, int y);
void mouseFunc(int button, int state, int x, int y);
void timerFunc(int value);

void altScene();

//Screenshots
void takeScreenshot(char * fname, int xStart, int yStart, int xWidth, int xHeight);

int main(int argc, char ** argv);

//For an interesting demo scene to render
void recursiveCircles(GLfloat radius, GLfloat startingradius, int xd, int yd, int zd, int parentdir, int numrecs, int maxrecs);
void beginRecursiveCircles(GLfloat radius, int maxrecs);
float animationAngle = 0;
bool animateScene = false;


void displayFunc()
{
	mg.computeOutput();
	mg.sketchOutput();
	glutSwapBuffers();
	char fname[80];
	if(screenshotsWanted > 0)
	{
		sprintf(fname, "test-%03i.ppm", screenshotsWanted);
		takeScreenshot(fname, 0, 0, desiredWidth, desiredHeight);
		//takeScreenshot(fname, 1200, 50, 200, 200);
		screenshotsWanted--;
	}
}

void reshapeFunc(int width, int height)
{
	cout << "RESHAPE func with " << width << ", " << height << endl;
	if((width == desiredWidth) && (height == desiredHeight))
	{
		//ignore
		return;
	}
	if((width < 10) || (height < 10))
		return;
	//literally do nothing for now
	windowResized = true;
	resizeCounter = 0;
	desiredWidth = width;
	desiredHeight = height;
}

void keyboardFunc(unsigned char key, int x, int y)
{
	int temp;
	int xres, yres;
	if((key >= '0') && (key <= '9'))
	{
		//cout << (key - '0') << endl;
		mg.setDisplayMode(key - '0');
		return;
	}
	switch(key)
	{
		case 'q':
			exit(0);
			break; //why not
		case 't':
			cout << "Enter new texwidth: ";
			cin >> temp;
			mg.setStripWidth(temp / 2);
			//DIRTY HACK ENSURES!!!
			desiredWidth--;
			glutReshapeWindow(desiredWidth + 1, desiredHeight);
			glutPostRedisplay();
			break;
		case 'r':
			mg.setRenderRightOneWay(!mg.getRenderRightOneWay());
			glutPostRedisplay();
			break;
		case ' ':
		case 'k':
			mg.cycleDisplayMode();
			glutPostRedisplay();
			break;
		case 'j':
			mg.toggleMapTex();
			glutPostRedisplay();
			break;
		case 'm':
			cout << "enter space-separated desired x,y dimensions: ";
			cin >> xres >> yres;
			reshapeFunc(xres, yres);
			break;
		case 's':
			screenshotsWanted = 1;
			break;
		case 'S':
			cout << "enter # of screenshots desired: ";
			cin >> temp;
			screenshotsWanted = temp;
			break;
		case 'a':
			animateScene = !animateScene;
			cout << "animating? " << animateScene << endl;
			break;
		default:
			cout << "Unknown key " << key << " pressed" << endl;
			break;
	}
}

void motionFunc(int x, int y)
{
	if(isMiddleMouse)
	{
		//cout << y << endl;
		mg.setTexturePersistence((desiredHeight - y) / float(desiredHeight));
	}
	mg.motionFunc(x, y);
}

void mouseFunc(int button, int state, int x, int y)
{
	if(button == GLUT_MIDDLE_BUTTON)
		isMiddleMouse = true;
	else
		isMiddleMouse = false;
	mg.mouseFunc(button, state, x, y);
}

void timerFunc(int value)
{
	if(windowResized)
	{
		resizeCounter++;
		if(resizeCounter >= MAX_RESIZE_COUNTER)
		{
			cout << "WOULD RESIZE WINDOW" << endl;
			cout << desiredWidth << ", " << desiredHeight << endl;
			mg.resizeToFit(desiredWidth, desiredHeight);
//			int desiredStrips = desiredWidth
//				/ mg.getStripWidth() + 1;
//			//desiredWidth = desiredStrips * mg.getStripWidth();
//			//mg.setStripWidth(MaGLi::DEF_STRIPWIDTH);
//			mg.setStripHeight(desiredHeight);
//			mg.setStartingStrips(desiredStrips - 2);
			//resize this window to match
			glutReshapeWindow(desiredWidth, desiredHeight);
			windowResized = false;
			resizeCounter = 0;
		}
	}
	if(animateScene)
	{
		animationAngle += 1.25;
		if(animationAngle >= 360)
			animationAngle -= 360;
	}
	glutPostRedisplay();
	glutTimerFunc(33, timerFunc, 1);
}

// Below are two functions I copied from an old OpenGL test program;
// I use them here as a test scene for the autostereogram effect.
// The test scenes I had been using before were getting
// a bit stale and boring.

void beginRecursiveCircles(GLfloat radius, int maxrecs)
{
	recursiveCircles(radius, radius, 0, 0, 0, 0, 0, maxrecs);
}

void recursiveCircles(GLfloat radius, GLfloat startingradius, int xd, int yd, int zd, int parentdir, int numrecs, int maxrecs)
{
	if(numrecs > maxrecs)
		return;
	//cout << "hey\n";
	glPushMatrix();
	GLfloat offset = 3 * radius;
	//float animationAngle = 0;
	switch(parentdir)
	{
		case 1: glTranslatef(offset, 0.0f, 0.0f);
			glRotatef(animationAngle, 1.0f, 0.0f, 0.0f);
			break;
		case 2: glTranslatef(-offset, 0.0f, 0.0f);
			glRotatef(animationAngle, -1.0f, 0.0f, 0.0f);
			break;
		case 3: glTranslatef(0.0f, offset, 0.0f);
			glRotatef(-animationAngle, 0.0f, 1.0f, 0.0f);
			break;
		case 4: glTranslatef(0.0f, -offset, 0.0f);
			glRotatef(-animationAngle, 0.0f, -1.0f, 0.0f);
			break;
		case 5: glTranslatef(0.0f, 0.0f, offset);
			glRotatef(animationAngle, 0.0f, 0.0f, 1.0f);
			break;
		case 6: glTranslatef(0.0f, 0.0f, -offset);
			glRotatef(animationAngle, 0.0f, 0.0f, -1.0f);
			break;
		default: glRotatef(animationAngle, 0.0f, 1.0f, 0.0f);
			//glRotatef(animationAngle, 1.0f, 0.0f, 0.0f);
			break;
	}

	GLfloat tempcolor[3] = {static_cast<GLfloat>(xd), static_cast<GLfloat>(yd), static_cast<GLfloat>(zd)};
	for(int i = 0; i < 3; i++)
	{
		tempcolor[i] = 1 - abs(tempcolor[i]) / (4 * startingradius);
		tempcolor[i] = tempcolor[i] * tempcolor[i];
		tempcolor[i] = 0.8 * tempcolor[i] * tempcolor[i];
		tempcolor[i] *= 0.5 + 0.5 * (1 - numrecs / (float) maxrecs);
	}
	
	glColor3f(tempcolor[0], tempcolor[1], 0.1 + 0.9 * tempcolor[2]);
	
	int polydetail = 5 + 10 * (radius / startingradius);
	
	glutSolidSphere(radius, polydetail, polydetail);
	
	GLfloat nextradius = radius / 2;
	GLfloat nextcenter = radius + nextradius;
	if(parentdir != 1)  //x neg
		recursiveCircles(nextradius, startingradius, xd - nextcenter, yd, zd, 2, numrecs + 1, maxrecs);
	if(parentdir != 2)  //x pos
		recursiveCircles(nextradius, startingradius, xd + nextcenter, yd, zd, 1, numrecs + 1, maxrecs);
	if(parentdir != 3)  //y neg
		recursiveCircles(nextradius, startingradius, xd, yd - nextcenter, zd, 4, numrecs + 1, maxrecs);
	if(parentdir != 4)  //y pos
		recursiveCircles(nextradius, startingradius, xd, yd + nextcenter, zd, 3, numrecs + 1, maxrecs);
	if(parentdir != 5)  //z neg
		recursiveCircles(nextradius, startingradius, xd, yd, zd - nextcenter, 6, numrecs + 1, maxrecs);
	if(parentdir != 6)  //z pos
		recursiveCircles(nextradius, startingradius, xd, yd, zd + nextcenter, 5, numrecs + 1, maxrecs);
	glPopMatrix();
}

// Callback for the MaGLi object to use the test scene above

void altScene()
{
	//cout << "calling altScene" << endl;
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -20.0f);
	mg.rotateCameraByAccums();
	beginRecursiveCircles(5.0f, 3);
	glPopMatrix();
}

void takeScreenshot(char * fname, int xStart, int yStart, int xWidth, int xHeight)
{
	cout << fname << endl;
	WriterPNM writer(fname, xWidth, xHeight, false, true); //binary, color

	string emsg("No error");
	if(!writer.checkError(&emsg))
	{
		cout << "Error: " << emsg << endl;
		return;
	}

	float * screenieBuf = new float [3 * xWidth * xHeight];
	if(screenieBuf == NULL)
		return;

	// get screen contents
	glReadPixels(xStart, yStart, xWidth, xHeight, GL_RGB, GL_FLOAT, screenieBuf);

	for(int j = xHeight - 1; j >= 0; j--)
	{
		for(int i = 0; i < xWidth; i++)
		{
			writer.addColorFloat(&(screenieBuf[3 * (xWidth * j + i)]));
		}
	}
	if(!writer.checkError(&emsg))
	{
		cout << "Error during write: " << emsg << endl;
		return;
	}
}

int main(int argc, char ** argv)
{
	cout << "Hello" << endl;
	mg.resizeToFit(desiredWidth, desiredHeight);
	if(!mg.initGLUTGLEW(&argc, argv))
		exit(0);
	
	mg.setSceneSketcher(altScene); // render the alternate scene above

	glEnable(GL_DEPTH_TEST);

	cout << "OpenGL ready? " << mg.isReadyGL() << endl;
	cout << "Components ready? " << mg.areReadyComponents() << endl;
	cout << "\"Rendering\" scene? " << mg.computeOutput() << endl;
	cout << "Now, components ready? " << mg.areReadyComponents() << endl;
	
	glutDisplayFunc(displayFunc);
	glutReshapeFunc(reshapeFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMotionFunc(motionFunc);
	glutMouseFunc(mouseFunc);
	glutTimerFunc(33, timerFunc, 1);

	glutMainLoop();
}
