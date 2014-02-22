//pnm_tools.cpp

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


#include "pnm_tools.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>

using namespace std;

//+++ WriterPNM

WriterPNM::WriterPNM()
{
	//file = ofstream("fasdlkfgja", ios::
	string emsg("No filename given");
	reportError(&emsg);
	//allocate outputbuffer so delete [] doesn't whine
	outbufsize = 1000;
	outputbuffer = new char [outbufsize];
	outbufpos = 0;
}

WriterPNM::WriterPNM(const char * fname, int xr, int yr, bool ua, bool uc)
{
	string emsg;
	xres = xr;
	yres = yr;
	currentx = currenty = 0;
	useascii = ua;
	usecolor = uc;
	file.open(fname, ios::out | ios::binary);
	alliswell = true;
	nomore = false;
	if(!file.is_open())
	{
		emsg = string("Failed to open file for writing");
		reportError(&emsg);
		return;
	}
	
	//configure write system
	outbufsize = 1000;
	outputbuffer = new char [outbufsize];
	outbufpos = 0;

	//do initial write of header stuff
	//the header is almost identical between ascii and binary versions
	char tempcstring[30];
	addToOutput('P');
	if(useascii)                        //magic number
	{
		if(usecolor)  //PPM ascii
			addToOutput('3');
		else  //PGM ascii
			addToOutput('2');
	}
	else //use binary
	{
		if(usecolor)  //PPM binary
			addToOutput('6');
		else  //PGM binary
			addToOutput('5');
	}
	addToOutput('\n');
	sprintf(tempcstring, "%d", xres);
	for(int i = 0; tempcstring[i] != 0; i++)  //x resolution
		addToOutput(tempcstring[i]);
	addToOutput(' ');
	sprintf(tempcstring, "%d", yres);
	for(int i = 0; tempcstring[i] != 0; i++)  //y resolution
		addToOutput(tempcstring[i]);
	addToOutput('\n');
	sprintf(tempcstring, "%d", 255);
	for(int i = 0; tempcstring[i] != 0; i++)  //maximum color value
		addToOutput(tempcstring[i]);
	addToOutput('\n');
}

WriterPNM::~WriterPNM()
{
	//cout << "bye\n";
	int dummy[3] = {0, 0, 0};
	if(alliswell)
	{
		//ensure everything was written
		while(!nomore)
			addColorInt(dummy);

		//flush the remainder of the output buffer
		file.write(outputbuffer, outbufpos);
	}
	file.close();
	delete [] outputbuffer;
}

void WriterPNM::reportError(string * emsg)
{
	errormessage = *emsg;
	//cout << errormessage << endl;  //COMMENT OUT AT SOME POINT
	alliswell = false;
	nomore = true;
}

void WriterPNM::addToOutput(char nextchar)
{
	//this is protected, so assumes writing is possibe/should be done
	string emsg;
	outputbuffer[outbufpos] = nextchar;
	outbufpos++;
	if(outbufpos >= outbufsize)
	{
		file.write(outputbuffer, outbufsize);
		if(file.fail())
		{
			emsg = string("Write to file failed");
			reportError(&emsg);
		}
		outbufpos = 0;
	}
}

bool WriterPNM::checkError(string * emsg)
{
	if(!alliswell)
		*emsg = errormessage;
	return alliswell;
}

void WriterPNM::getFileType(string * ftype)
{
	if(!alliswell)
		return;
	string workingstring("P");
	if(usecolor)
	{
		workingstring += (useascii ? '3' : '6');
		workingstring += ": PPM, ";
	}
	else  //is gray
	{
		workingstring += (useascii ? '2' : '5');
		workingstring += ": PGM, ";
	}
	if(useascii)
		workingstring += "ascii formatting";
	else
		workingstring += "binary formatting";
	*ftype = workingstring;
}

bool WriterPNM::addGrayInt(int gray)
{
	if(!alliswell || nomore)
		return false;
	int tempcolor[3];
	char tempcstring[10];
	if(usecolor)
	{
		tempcolor[0] = tempcolor[1] = tempcolor[2] = gray;
		return addColorInt(tempcolor);
	}
	else  //use gray
	{
		if(gray < 0)
			gray = 0;
		else if(gray > 255)
			gray = 255;
		if(useascii)
		{
			sprintf(tempcstring, "%d", gray);
			for(int i = 0; tempcstring[i] != 0; i++)
			{
				addToOutput(tempcstring[i]);
			}
			addToOutput('\n');
		}
		else
		{
			addToOutput(gray);
		}
	}
	//update currentx/y
	currentx++;
	if(currentx >= xres)
	{
		currenty++;
		currentx = 0;
		if(currenty >= yres)
			nomore = true;
	}
	return alliswell;
}

bool WriterPNM::addGrayFloat(float gray)
{
	return addGrayInt((int) (gray * 255));
}

bool WriterPNM::addColorInt(int * color)
{
	if(!alliswell || nomore)
		return false;
	int temp;
	char tempcstring[10];
	if(usecolor)
	{
		for(int a = 0; a < 3; a++)
		{
			temp = color[a];
			if(temp < 0)
				temp = 0;
			else if(temp > 255)
				temp = 255;
			if(useascii)
			{
				sprintf(tempcstring, "%d", temp);
				for(int i = 0; tempcstring[i] != 0; i++)
				{
					addToOutput(tempcstring[i]);
				}
				addToOutput('\n');
			}
			else
			{
				addToOutput(temp);
			}
		}
	}
	else  //use gray
	{
		return addGrayInt((color[0] + color[1] + color[2]) / 3);
	}

	//update currentx/y
	currentx++;
	if(currentx >= xres)
	{
		currenty++;
		currentx = 0;
		if(currenty >= yres)
			nomore = true;
	}
	return alliswell;
}

bool WriterPNM::addColorFloat(float * color)
{
	int tempint[3];
	for(int a = 0; a < 3; a++)
		tempint[a] = color[a] * 255;
	return addColorInt(tempint);
}

//+++ ReaderPNM

ReaderPNM::ReaderPNM()
{
	string emsg("No filename given");
	reportError(&emsg);
	lastload = false;
	inbufsize = 1000;
	inputbuffer = new char [inbufsize];
	inbufpos = 1000;
	maxpixel = 1;
}

ReaderPNM::ReaderPNM(const char * fname)
{
	string emsg;
	file.open(fname, ios::in | ios::binary);
	alliswell = true;
	lastload = false;
	currentx = currenty = 0;
	nomore = false;
	inbufsize = 1000;
	inputbuffer = new char [inbufsize];
	for(int i = 0; i < inbufsize; i++)
		inputbuffer[i] = 0;
	inbufpos = 1000;
	maxpixel = 1;
	if(!file.is_open() || file.fail())  //not bad(), since no reading yet
	{
		emsg = string("Failed to open file (may not exist)");
		reportError(&emsg);
		return;
	}
	//now, let's (try to) read the header
	char tempchar;
	int tempint;
	tempchar = getFromInput();
	if(tempchar != 'P')
	{
		emsg = string("File is of wrong type");
		reportError(&emsg);
		return;
	}
	tempchar = getFromInput();
	if(tempchar == '2')  //PGM ascii
	{
		isascii = true; iscolor = false;
	}
	else if(tempchar == '3')  //PPM ascii
	{
		isascii = true; iscolor = true;
	}
	else if(tempchar == '5')  //PGM binary
	{
		isascii = false; iscolor = false;
	}
	else if(tempchar == '6')  //PPM binary
	{
		isascii = false; iscolor = true;
	}
	else
	{
		emsg = string("File is of wrong type");
		reportError(&emsg);
		return;
	}
	//===grab xres
	stripInsignificant();
	tempint = getNumber();
	if(!alliswell)
		return;
	if(tempint <= 0)
	{
		emsg = string("Invalid x resolution");
		reportError(&emsg);
		return;
	}
	xres = tempint;
	//===grab yres
	stripInsignificant();
	tempint = getNumber();
	if(!alliswell)
		return;
	if(tempint <= 0)
	{
		emsg = string("Invalid y resolution");
		reportError(&emsg);
		return;
	}
	yres = tempint;
	//===grab max pixel
	stripInsignificant();
	tempint = getNumber();
	if(!alliswell)
		return;
	if((tempint <= 0) || (tempint > 255))
	{
		emsg = string("Maximum color value out of parser's range");
		reportError(&emsg);
		return;
	}
	maxpixel = tempint;
	//strip the first of at least one (or at most one, for binary) whitespace
	//characters before the real data comes in
	getFromInput();
}

ReaderPNM::~ReaderPNM()
{
	//cout << "bye from reader\n";
	file.close();
	delete [] inputbuffer;
}

void ReaderPNM::reportError(string * emsg)
{
	errormessage = *emsg;
	//cout << errormessage << endl;  //COMMMENT OUT AT SOME POINT
	alliswell = false;
	nomore = true;
}

char ReaderPNM::getFromInput()
{
	string emsg;
	//protected function: assumes alliswell and nomore were already checked
	if(!alliswell)
		return 0;
	if(inbufpos >= inbufsize)
	{
		if(lastload)
		{
			emsg = string("Kept reading past EOF");
			reportError(&emsg);
			return 0;
		}
		inbufpos = 0;
		file.read(inputbuffer, inbufsize);
		if(file.bad())  //things went wrong
		{
			emsg = string("File read went bad in ifstream");
			reportError(&emsg);
			return 0;
		}
		else if(file.eof())
		{
			lastload = true;
		}
	}
	char tempchar = inputbuffer[inbufpos];
	inbufpos++;
	return tempchar;
}

char ReaderPNM::peekFromInput()
{
	char tempchar = getFromInput();
	inbufpos--;
	return tempchar;
}

void ReaderPNM::stripInsignificant()
{
	char tempchar;
	char lf = 10;  //line feed
	char ff = 12;  //form feed
	char cr = 13;  //carriage return
	char zero = 48;
	char nine = 57;
	while(true)  //the big loop - one execution per whitespace character or comment line
	{
		tempchar = peekFromInput();
		if(!alliswell)
			return;
		if(tempchar == '#')
		{
			//remove entire comment line
			while((tempchar != lf) && (tempchar != ff) && (tempchar != cr))
			{
				tempchar = getFromInput();
				tempchar = peekFromInput();
				if(!alliswell)
					return;
			}
			continue;  //after all, there may be two comments in a row
		}
		if((tempchar >= zero) && (tempchar <= nine))  //next char is a number character
			return;
		tempchar = getFromInput();
	}
}

int ReaderPNM::getNumber()
{
	string emsg;
	char numberchar[10];
	char zero = 48;
	char nine = 57;
	for(int i = 0; i < 9; i++)
	{
		numberchar[i] = peekFromInput();
		if(!alliswell)
			return 0;
		if((numberchar[i] < zero) || (numberchar[i] > nine))  //not a number
		{
			if(i == 0)
			{
				emsg = string("Expected a number");
				reportError(&emsg);
			}
			numberchar[i] = ' ';
			return atoi(numberchar);
		}
		//was a number
		getFromInput();
	}
	emsg = string("Encountered too-long ASCII-formatted number");
	reportError(&emsg);
	return 0;  //the number was too big
}

bool ReaderPNM::checkError(string * emsg)
{
	if(!alliswell)
		*emsg = errormessage;
	return alliswell;
}

int ReaderPNM::getXRes()
{
	if(!alliswell)
		return 0;
	return xres;
}

int ReaderPNM::getYRes()
{
	if(!alliswell)
		return 0;
	return yres;
}

int ReaderPNM::getMaxPixel()
{
	if(!alliswell)
		return 0;
	return maxpixel;
}

bool ReaderPNM::isASCII()
{
	return isascii;
}

bool ReaderPNM::isColor()
{
	return iscolor;
}

void ReaderPNM::getFileType(string * ftype)
{
	if(!alliswell)
		return;
	string workingstring("P");
	if(iscolor)
	{
		workingstring += (isascii ? '3' : '6');
		workingstring += ": PPM, ";
	}
	else  //is gray
	{
		workingstring += (isascii ? '2' : '5');
		workingstring += ": PGM, ";
	}
	if(isascii)
		workingstring += "ascii formatting";
	else
		workingstring += "binary formatting";
	*ftype = workingstring;
}

int ReaderPNM::getGrayInt()
{
	string emsg;
	int colortriple[3];
	int colorint;
	int maskchar = (1 << (8 * sizeof(char))) - 1;
	//see if it's possible
	if(!alliswell)
		return 0;
	if(nomore)
	{
		emsg = string("Requested pixels past end of image size");
		reportError(&emsg);
		return 0;
	}
	//see what color format is in use
	if(iscolor)
	{
		getColorInt(colortriple);
		return (colortriple[0] + colortriple[1] + colortriple[2]) / 3;
	}
	else  //is b&w
	{
		//see what encoding is used
		if(isascii)
		{
			stripInsignificant();
			colorint = getNumber();
			if(!alliswell)
				return 0;
		}
		else  //binary
		{
			colorint = maskchar & getFromInput();
			if(!alliswell)
				return 0;
		}
	}

	currentx++;
	if(currentx >= xres)
	{
		currentx = 0;
		currenty++;
		if(currenty >= yres)
		{
			nomore = true;
		}
	}

	return colorint;
}

void ReaderPNM::getColorInt(int * color)
{
	string emsg;
	int colorint;
	int maskchar = (1 << (8 * sizeof(char))) - 1;
	//see if it's possible
	if(!alliswell)
	{
		color[0] = color[1] = color[2] = 0;
		return;
	}
	if(nomore)
	{
		emsg = string("Requested pixels past end of image size");
		reportError(&emsg);
		color[0] = color[1] = color[2] = 0;
		return;
	}
	//see what color format is in use
	if(iscolor)
	{
		//see what encoding is used
		if(isascii)
		{
			for(int a = 0; a < 3; a++)
			{
				stripInsignificant();
				color[a] = maskchar & getNumber();
			}
			if(!alliswell)
				color[0] = color[1] = color[2] = 0;
		}
		else  //binary
		{
			for(int a = 0; a < 3; a++)
			{
				color[a] = maskchar & getFromInput();
			}
			if(!alliswell)
				color[0] = color[1] = color[2] = 0;
		}
	}
	else  //is b&w
	{
		colorint = getGrayInt();
		color[0] = color[1] = color[2] = colorint;
		return;
	}

	currentx++;
	if(currentx >= xres)
	{
		currentx = 0;
		currenty++;
		if(currenty >= yres)
		{
			nomore = true;
		}
	}
	return;
}

float ReaderPNM::getGrayFloat()
{
	return getGrayInt() / (float) maxpixel;
}

void ReaderPNM::getColorFloat(float * color)
{
	int temp[3];
	getColorInt(temp);
	for(int i = 0; i < 3; i++)
		color[i] = temp[i] / (float) maxpixel;
}
