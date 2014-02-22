//pnm_tools.h

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


#include <fstream>

#ifndef PNM_TOOLS_H
#define PNM_TOOLS_H

using namespace std;

class WriterPNM;  //outputs PPM files
class ReaderPNM;  //reads PPM and PGM files

class WriterPNM
{
private:
	ofstream file;
	bool alliswell;
	bool nomore;  //takes precedence over currentx, currenty
	string errormessage;

	int xres;
	int yres;
	bool useascii;
	bool usecolor;

	int currentx;
	int currenty;

	char * outputbuffer;
	int outbufsize;
	int outbufpos;
protected:
	void reportError(string * emsg);
	void addToOutput(char nextchar);
public:
	WriterPNM();
	WriterPNM(const char * fname, int xr, int yr, bool ua, bool uc);
	~WriterPNM();

	bool checkError(string * emsg);
	void getFileType(string * ftype);  //returns a string description of file

	bool addGrayInt(int gray);
	bool addGrayFloat(float gray);
	bool addColorInt(int * color);
	bool addColorFloat(float * color);
};

class ReaderPNM
{
private:
	ifstream file;
	bool alliswell;  //should be set by reportError()
	bool nomore;  //takes precedence over currentx/y
	string errormessage;

	int xres;
	int yres;
	bool isascii;
	bool iscolor;
	int maxpixel;

	int currentx;
	int currenty;

	char * inputbuffer;
	int inbufsize;
	int inbufpos;
	bool lastload;
protected:
	void reportError(string * emsg);
	char getFromInput();
	char peekFromInput();
	void stripInsignificant();  //get the input to the next non-comment number
	int getNumber();  //DOES NOT stripInsignificant() at least now
public:
	ReaderPNM();
	ReaderPNM(const char * fname); //xr/yr/ia/ic found from file
	~ReaderPNM();
	bool checkError(string * emsg);  //Returns false if there IS an error (description in emsg)
	int getXRes();
	int getYRes();
	int getMaxPixel();
	bool isASCII();
	bool isColor();
	void getFileType(string * ftype);  //returns a string description of file

	int getGrayInt();
	void getColorInt(int * color);
	float getGrayFloat();
	void getColorFloat(float * color);
};


#endif //PNM_TOOLS_H
