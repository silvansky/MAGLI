//magli_leftoneway.fs

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


uniform sampler2D nearTex;
uniform sampler2D farTex;
uniform sampler2D depthTex;
uniform sampler2D otherDepthTex;
uniform sampler2D gapTex;
uniform float texWidthDepth;

void main(void)
{
	//gl_FragColor = gl_Color - vec4(1.0);
	//modifiedx is the x coordinate in nearTex
	float depth = texture2D(depthTex, gl_TexCoord[0].xy).r;
	float otherDepth = texture2D(otherDepthTex, gl_TexCoord[0].xy
		+ vec2((0.5 + 0.5 * depth) * texWidthDepth, 0)).r;
	float modifiedx = gl_TexCoord[1].x
		+ depth;
	if(depth - otherDepth > 0.1)  //theoretically, a shadow region
	{
		gl_FragColor = texture2D(gapTex, gl_TexCoord[0].xy);
	}
	else if(modifiedx <= 1.0)
	{
		gl_FragColor = texture2D(nearTex, vec2(modifiedx,
			gl_TexCoord[0].y));// + vec4(0.1);
	}
	else
	{
		gl_FragColor = texture2D(farTex, vec2(modifiedx - 1.0,
			gl_TexCoord[0].y));// + vec4(0.1);
	}
	//that's that.
}
