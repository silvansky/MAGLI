//shaderloader.h

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


class ShaderLoaderChunk;
class ShaderLoader;

const int SHADER_LOADER_MAX_CHUNK_SIZE = 160;

class ShaderLoaderChunk
{
private:
	char chunk[SHADER_LOADER_MAX_CHUNK_SIZE];
	ShaderLoaderChunk * next;
protected:
	//
public:
	ShaderLoaderChunk();
	~ShaderLoaderChunk();

	char * getChunk();

	ShaderLoaderChunk * getNext();
	void setNext(ShaderLoaderChunk * n);
};

class ShaderLoader
{
private:
	bool loadsuccess;
	ShaderLoaderChunk head;
	int chunkcount;
	char ** chunklist;
protected:
	//
public:
	ShaderLoader();
	ShaderLoader(const char * fname);
	~ShaderLoader();

	bool loadFile(const char * fname);
	bool getSizeAndList(int * size, char *** list);  //three asterisks - hellya

	static const int MAX_CHUNK_SIZE; // = 140;
};
