/*
Copyright (C) 2009 COR Entertainment

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "r_local.h"

#define EXPLOSION 1
#define FLASHDISTORT 2

extern int KillFlags;
extern float v_blend[4];
extern void R_TransformVectorToScreen( refdef_t *rd, vec3_t in, vec2_t out );

image_t *r_framebuffer;
image_t *r_perlinnoise;
image_t *r_distortwave;

float	twodvert_array[MAX_ARRAY][3];

vec3_t r_explosionOrigin;
int r_drawing_fbeffect;
int	r_fbFxType;
float r_fbeffectTime;
int frames;

static int		FB_texture_width, FB_texture_height;

void R_GLSLPostProcess(void)
{
	vec2_t fxScreenPos;
	int offsetX, offsetY;
	vec3_t	vec;
	float	dot;
	vec3_t	forward;

	//to do:  add cvar to disable effect
		
	if(!gl_glsl_shaders->value)
		return;

	if(r_fbFxType == EXPLOSION) {
		AngleVectors (r_newrefdef.viewangles, forward, NULL, NULL);
		VectorSubtract (r_explosionOrigin, r_newrefdef.vieworg, vec);
		VectorNormalize (vec);
		dot = DotProduct (vec, forward);
		if (dot <= 0.3)
			r_drawing_fbeffect = false;
	}

	//if not doing stuff, return
	if(!r_drawing_fbeffect)
		return;
		
	frames++;

	qglViewport(0,0,FB_texture_width,FB_texture_height);

	//we need to grab the frame buffer
	qglBindTexture(GL_TEXTURE_2D, r_framebuffer->texnum);
	qglCopyTexSubImage2D(GL_TEXTURE_2D, 0,
				0, 0, 0, 0, FB_texture_width, FB_texture_height);

	qglViewport(0,0,vid.width, vid.height);	

	//render quad on screen

	offsetY = vid.height - FB_texture_height;
	offsetX = vid.width - FB_texture_width;

	qglColor4f(0.0, 1.0, 1.0, 1.0); //test
	
	qglEnableClientState (GL_VERTEX_ARRAY);
	qglEnableClientState (GL_TEXTURE_COORD_ARRAY);

	qglTexCoordPointer (2, GL_FLOAT, sizeof(tex_array[0]), tex_array[0]); 
	qglVertexPointer (2, GL_FLOAT, sizeof(vert_array[0]), twodvert_array[0]);
	qglColorPointer (4, GL_FLOAT, sizeof(col_array[0]), col_array[0]);

	VA_SetElem2(twodvert_array[0],0, vid.height);
	VA_SetElem2(twodvert_array[1],vid.width-offsetX, vid.height);
	VA_SetElem2(twodvert_array[2],vid.width-offsetX, offsetY);
	VA_SetElem2(twodvert_array[3],0, offsetY); 

	VA_SetElem2(tex_array[0],r_framebuffer->sl, r_framebuffer->tl);
	VA_SetElem2(tex_array[1],r_framebuffer->sh, r_framebuffer->tl);
	VA_SetElem2(tex_array[2],r_framebuffer->sh, r_framebuffer->th);
	VA_SetElem2(tex_array[3],r_framebuffer->sl, r_framebuffer->th);

	//do some glsl 
	glUseProgramObjectARB( g_fbprogramObj );

	qglActiveTextureARB(GL_TEXTURE1); 
	qglBindTexture (GL_TEXTURE_2D,r_framebuffer->texnum);
	glUniform1iARB( g_location_framebuffTex, 1); 
	KillFlags |= KILL_TMU0_POINTER;
   	
	qglActiveTextureARB(GL_TEXTURE0);
	if(r_fbFxType == EXPLOSION)
		qglBindTexture(GL_TEXTURE_2D, r_distortwave->texnum);
	else
		qglBindTexture (GL_TEXTURE_2D, r_perlinnoise->texnum);
	glUniform1iARB( g_location_distortTex, 0); 
	KillFlags |= KILL_TMU1_POINTER;

	qglActiveTextureARB(GL_TEXTURE0);

	glUniform1iARB( g_location_fxType, r_fbFxType); //2 for flash distortions, 1 for warping
	glUniform1fARB( g_location_frametime, rs_realtime);
	glUniform3fARB( g_location_fxColor, v_blend[0], v_blend[1], v_blend[2]);

	VectorClear(fxScreenPos);

	//get position of focal point of warp if we are doing a warp effect
	if(r_fbFxType == EXPLOSION) {
		R_TransformVectorToScreen(&r_newrefdef, r_explosionOrigin, fxScreenPos);
		fxScreenPos[0] -= 32; //texture offset
		fxScreenPos[1] +=32;
		fxScreenPos[0] += frames*5;
		fxScreenPos[1] += frames*5;
		glUniform2fARB( g_location_fxPos, fxScreenPos[0], fxScreenPos[1]);
	}
	else {
		fxScreenPos[0] -= 32; //texture offset
		fxScreenPos[1] -=128;
		fxScreenPos[0] += frames*5;
		fxScreenPos[1] += frames*5;
		glUniform2fARB( g_location_fxPos, fxScreenPos[0], fxScreenPos[1]);
	}

	qglDrawArrays (GL_QUADS, 0, 4);

	glUseProgramObjectARB( NULL );

	R_KillVArrays();

	if(rs_realtime > r_fbeffectTime+.1) {
		frames = 0;
		r_drawing_fbeffect = false; //done effect
	}

	return;
}


/*
=================
R_FB_InitTextures
=================
*/
void R_FB_InitTextures( void )
{
	byte	*data;
	int		size;

	//find closer power of 2 to screen size 
	for (FB_texture_width = 1;FB_texture_width < viddef.width;FB_texture_width *= 2);
	for (FB_texture_height = 1;FB_texture_height < viddef.height;FB_texture_height *= 2);

	//limit
	if(FB_texture_width > 1024)
		FB_texture_width = 1024;

	if(FB_texture_height > 1024)
		FB_texture_height = 1024;

	//init the framebuffer texture
	size = FB_texture_width * FB_texture_height * 4;
	data = malloc( size );
	memset( data, 255, size );
	r_framebuffer = GL_LoadPic( "***r_framebuffer***", (byte *)data, FB_texture_width, FB_texture_height, it_pic, 3 );
	free ( data );

	//init the distortion textures
	r_perlinnoise = GL_FindImage("gfx/perlin_noise.jpg",it_pic);
	if (!r_perlinnoise) {                                
		r_perlinnoise = GL_LoadPic ("***r_notexture***", (byte *)data, 16, 16, it_wall, 32);
    }  
	
	r_distortwave = GL_FindImage("gfx/distortwave.jpg",it_pic);
	if (!r_distortwave) {                                
		r_distortwave = GL_LoadPic ("***r_notexture***", (byte *)data, 16, 16, it_wall, 32);
    }   
}



					




