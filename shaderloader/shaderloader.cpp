//shaderloader.cpp

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
#include <fstream>
#include "shaderloader.h"

using namespace std;

//+++ ShaderLoaderChunk

ShaderLoaderChunk::ShaderLoaderChunk()
{
	//cout << "Constructing ShaderLoaderChunk " << this << endl;
	chunk[0] = '\0';
	next = NULL;
}

ShaderLoaderChunk::~ShaderLoaderChunk()
{
	//cout << "Destructing ShaderLoaderChunk " << this << endl;
	if(next != NULL)
		delete next;
}

char * ShaderLoaderChunk::getChunk()
{
	return &(chunk[0]);
}

ShaderLoaderChunk * ShaderLoaderChunk::getNext()
{
	return next;
}

void ShaderLoaderChunk::setNext(ShaderLoaderChunk * n)
{
	next = n;
}

//+++ ShaderLoader

ShaderLoader::ShaderLoader()
{
	//cout << "Constructing ShaderLoader " << this << endl;
	loadsuccess = false;
	chunkcount = 0;
	chunklist = NULL;
}

ShaderLoader::ShaderLoader(const char * fname)
{
	//cout << "Constructing ShaderLoader " << this << " with immediate load" << endl;
	loadsuccess = false;
	chunkcount = 0;
	chunklist = NULL;
	loadFile(fname);
}

ShaderLoader::~ShaderLoader()
{
	//cout << "Destructing ShaderLoader " << this << endl;
	//deallocate chunklist (ONLY the single big array)
	if(chunklist != NULL)
	{
		//cout << "deallocating chunklist" << endl;
		delete [] chunklist;
	}

	//first ShaderLoaderChunk is statically allocated, so it will
	//get destructed, triggering the deallocation
}

bool ShaderLoader::loadFile(const char * fname)
{
	//open the file, confirm open
	ifstream thefile(fname);
	if(!thefile.is_open())
	{
		cerr << "failed to open shader source file" << endl;
		loadsuccess = false;
		return false;
	}
	//load lines and allocate chunks
	chunkcount = 0;
	chunklist = NULL;
	ShaderLoaderChunk * tail = &head;
	char * chunkptr = tail->getChunk();
	int endloc;

	while(thefile.getline(chunkptr, SHADER_LOADER_MAX_CHUNK_SIZE - 1))
	{
		if(thefile.fail())
		{
			cerr << "very unexpected error reading shader source" << endl;
			loadsuccess = false;
			return false;
		}
		for(endloc = 0; (endloc < (SHADER_LOADER_MAX_CHUNK_SIZE - 2)) && chunkptr[endloc];
			endloc++);
		chunkptr[endloc] = '\n';
		chunkptr[endloc + 1] = '\0';
		chunkcount++;
		tail->setNext(new (nothrow) ShaderLoaderChunk());
		tail = tail->getNext();
		if(tail == NULL)
		{
			cerr << "allocation failed while reading shader source" << endl;
			loadsuccess = false;
			return false;
		}
		chunkptr = tail->getChunk();
	}
	if(thefile.fail() && !thefile.eof())
	{
		//something bad happened
		cerr << "Failure during read of shader source" << endl;
		loadsuccess = false;
		return false;
	}
	//things worked. Now, populate the fun stuffs
	tail = &head;
	chunklist = new char * [chunkcount];
	if(chunklist == NULL)
	{
		cerr << "allocation failure in shader source read" << endl;
		loadsuccess = false;
		return false;
	}
	for(int i = 0; i < chunkcount; i++)
	{
		chunklist[i] = tail->getChunk();
		tail = tail->getNext();
	}
	loadsuccess = true;
	return true;
}

bool ShaderLoader::getSizeAndList(int * size, char *** list)
{
	if(!loadsuccess)
	{
		*size = 0;
		*list = NULL;  //may as well
		return false;
	}
	*size = chunkcount;
	*list = chunklist;
	return true;
}
