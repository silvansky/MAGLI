//magli_mixer.fs

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

//eventually I could add the weighting factor to favor one side or the other

void main(void)
{
	vec4 rightValue = texture2D(rightStrip, gl_TexCoord[0].xy);
	vec4 leftValue = texture2D(leftStrip, gl_TexCoord[0].xy);
	gl_FragColor = (1.0 / (rightValue.a + leftValue.a))
		* (rightValue + leftValue);
	gl_FragColor.a = 1.0;
}
