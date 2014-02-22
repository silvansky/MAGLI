//magli_mixer.vs

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


uniform sampler2D rightStrip;
uniform sampler2D leftStrip;

void main(void)
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
