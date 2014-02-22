//magli_seedfiller.vs

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


uniform float direction;
uniform float depthOffset;
uniform float startX;  //the x at which weight value is 1
uniform float sampleX;  //where to sample to determine depth
uniform sampler2D depthTex;
uniform sampler2D skewTex;

void main(void)
{
	float depth = texture2D(depthTex, vec2(sampleX, gl_TexCoord[0].y)).r;

	float endX = depth * direction + depthOffset;

	float diff = startX - endX;

	float alpha = clamp((1.0 / diff) * gl_TexCoord[0].x - (endX / diff), 0.0, 1.0);

	gl_FragColor = texture2D(skewTex, gl_TexCoord[1].xy) * alpha;
	gl_FragColor.a = alpha;
}
