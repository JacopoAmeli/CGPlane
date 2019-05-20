/*
 * HUD.cpp
 *
 *  Created on: 21 mag 2018
 *      Author: Jacopo Ameli
 */

#include "HUD.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __APPLE__
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <SDL2/SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#endif
#include <vector>
#include "point3.h"
#include "plane.h"

#ifndef NUMBALLOONS
#define NUMBALLOONS 15
#endif
#define DEFAULT_PTSIZE  52
//riferimenti esterni
extern void SetCoordToPixel();
extern GLuint jacopoTexture;
extern GLuint grassTexture;
extern GLuint roadTexture;
extern int totalpopped;
extern Vector3 balloons[NUMBALLOONS];
extern bool popped[NUMBALLOONS];
extern int scrW;
extern int scrH;
extern Plane plane;

//font caricato
TTF_Font *font;
//qualità testo
enum textquality {solid, shaded, blended};

//trova la potenza di due più vicina al valore val
unsigned int power_two_floor(unsigned int val)
{
  unsigned int power = 2, nextVal = power*2;
  while((nextVal *= 2) <= val)
    power*=2;
  return power*2;
}
/************************************************************************
GC_DrawText: funzione di disegno di testo su SDL_Renderer;
Input:
-(arg.1) puntatore al font da usare;
-(arg.2-5) quadrupla RGBA del colore per il testo;
-(arg.6-9) quadrupla RGBA del colore di sfondo del testo;
-(arg.10) stringa di testo da disegnare;
-(arg.11-12) posizione (ascissa e ordinata) schermo in cui mettere
             la stringa (vertice alto sinistro della matrice di pixel);
-(arg.13) qualita' di rendering del font: "solid", "shaded", "blended";
***************************************************************************/
void GC_DrawText(TTF_Font *fonttodraw, char fgR, char fgG, char fgB, char fgA,
              char bgR, char bgG, char bgB, char bgA, char text[], int x, int y,
              enum textquality quality)
{
	SDL_Color tmpfontcolor = {(Uint8)fgR,(Uint8)fgG,(Uint8)fgB,(Uint8)fgA};
	SDL_Color tmpfontbgcolor = {(Uint8)bgR,(Uint8)bgG,(Uint8) bgB,(Uint8) bgA};
	SDL_Surface *resulting_text;
	resulting_text=NULL;
	if (quality == solid)
		resulting_text = TTF_RenderText_Solid(fonttodraw, text, tmpfontcolor);
	if (quality == shaded)
		resulting_text = TTF_RenderText_Shaded(fonttodraw, text, tmpfontcolor, tmpfontbgcolor);
    if (quality == blended)
    	resulting_text = TTF_RenderText_Blended(fonttodraw, text, tmpfontcolor);
	if ( resulting_text != NULL )
	{
		GLuint texId;
		//Generate OpenGL texture
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		//Avoid mipmap filtering
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//Find the first power of two for OpenGL image
		int w = power_two_floor(resulting_text->w)*2;
		int h = power_two_floor(resulting_text->h)*2;
		//Create a surface to the correct size in RGB format, and copy the old image
		SDL_Surface * s = SDL_CreateRGBSurface(0, w, h, 32, 0x000000ff,0x0000ff00,0x00ff0000 ,0xff000000);
		SDL_BlitSurface(resulting_text, NULL, s, NULL);
		float width=resulting_text->w;
		float height=resulting_text->h;
		//Copy the created image into OpenGL format
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glColor4f( 1.0, 1.0, 1.0,1);
		glPushMatrix();
		SetCoordToPixel();
		//Draw the OpenGL texture as a Quad
		glBegin(GL_QUADS);
			glTexCoord2d(0, 1*height/h); glVertex2f(x,y) ;
			glTexCoord2d(1*width/w, 1*height/h); glVertex2f(x + width,y);
			glTexCoord2d(1*width/w, 0); glVertex2f(x + width,y + height);
			glTexCoord2d(0, 0); glVertex2f(x,y + height);
	    glEnd();
		glDisable(GL_TEXTURE_2D);
		//Cleanup
		glDeleteTextures(1, &texId);
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
    	/*glDisable(GL_TEXTURE_GEN_S);
    	glDisable(GL_TEXTURE_GEN_T);
    	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);*/
	}
}
//chiama la GC_DrawText con i parametri corretti, crea la stringa di testo per i punti
void HUD::drawPoints()
{
	char buffer[30];
	sprintf(buffer,"Popped: %d",totalpopped);
	GC_DrawText(font,
			(char)0,(char)0, (char)0, (char)0,
			(char)255, (char)255, (char)255,(char)0,
			buffer,
			0, scrH-100,
			blended);
}
//chiama la GC_DrawText, crea la stringa di game over
void HUD::drawGameOver()
{
	char buffer[30];
		sprintf(buffer,"YOU WON!");
		GC_DrawText(font,
				(char)200,(char)0, (char)0, (char)0,
				(char)255, (char)255, (char)255,(char)0,
				buffer,
				scrW/2-140, 20,
				blended);
}
//disegna la foto personale
void HUD::drawPersonalPhoto()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glPushMatrix();
	SetCoordToPixel();
	  //Foto personale Jacopo Ameli
	glColor3f(1.0,1.0,1.0);
	glEnable (GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glBindTexture (GL_TEXTURE_2D,jacopoTexture);
	glBegin( GL_QUADS );
	glColor3f(1, 1, 0);
	glTexCoord2f(1.0, 1.0);
	glVertex2d(xPhotoStart, 0);
	glTexCoord2f(0.0, 1.0);
	glVertex2d(xPhotoStart+personalPhotoSize, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex2d(xPhotoStart+personalPhotoSize, personalPhotoSize);
	glTexCoord2f(1.0, 0.0);
	glVertex2d(xPhotoStart, personalPhotoSize);
	glEnd();  //end draw
	glDisable (GL_TEXTURE_2D);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}
//disegna la mappa utilizzando texture effettive di terreno e pista
// utilizza dei punti rossi per i palloncini e uno blu per l'aereo
void HUD::drawMap()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glPushMatrix();
	SetCoordToPixel();

	//draw grass
	glColor3f(1.0,1.0,1.0);
	glEnable (GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glBindTexture (GL_TEXTURE_2D,grassTexture);
	glBegin( GL_QUADS );
		  	  glColor3f(1, 1, 0);
		  	  glTexCoord2f(1.0, 1.0);
		  	  glVertex2d(xMapStart, 0);
		  	  glTexCoord2f(0.0, 1.0);
		  	  glVertex2d(xMapStart+mapSize, 0);
		  	  glTexCoord2f(0.0, 0.0);
		  	  glVertex2d(xMapStart+mapSize, mapSize);
		  	  glTexCoord2f(1.0, 0.0);
		  	  glVertex2d(xMapStart, mapSize);
	glEnd();  //end draw

	//draw runway
	int runwayRepeat=6;
	double runwaySize=2.5;
	float xCenter=xMapStart+mapSize/2;
	float yCenter=mapSize/2;
	glColor3f(1.0,1.0,1.0);
	glPolygonMode(GL_BACK, GL_FILL);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, roadTexture);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glBegin( GL_QUADS );
			for(int i=-runwayRepeat;i<runwayRepeat;i++)
			{
				glTexCoord2d(0.0,0.0);glVertex2d(xCenter-runwaySize,yCenter+4*runwaySize+i*2*runwaySize);
				glTexCoord2d(1,0.0);glVertex2d(xCenter+runwaySize,yCenter+4*runwaySize+i*2*runwaySize);
				glTexCoord2d(1,1);glVertex2d(xCenter+runwaySize,yCenter+6*runwaySize+i*2*runwaySize);
				glTexCoord2d(0.0,1);glVertex2d(xCenter-runwaySize,yCenter+6*runwaySize+i*2*runwaySize);
			}
	glEnd();  //end draw

	//draw aereo e palloncini
	float planeWidth=2.5;
	float balloonWidth=2;
	glDisable (GL_TEXTURE_2D); // disable texture mapping
	if(xCenter+plane.planePos[0]-planeWidth>xMapStart && yCenter-plane.planePos[2]+planeWidth<mapSize)
	{
		glColor3f(0,0,1);
		glBegin(GL_QUADS);
			glVertex2d(xCenter+plane.planePos[0]-planeWidth,yCenter-plane.planePos[2]-planeWidth);
			glVertex2d(xCenter+plane.planePos[0]-planeWidth,yCenter-plane.planePos[2]+planeWidth);
			glVertex2d(xCenter+plane.planePos[0]+planeWidth,yCenter-plane.planePos[2]+planeWidth);
			glVertex2d(xCenter+plane.planePos[0]+planeWidth,yCenter-plane.planePos[2]-planeWidth);
			glEnd();
	}
	glColor3f(1,0,0);
	glBegin(GL_QUADS);
		for(int i=0;i<NUMBALLOONS;i++)
		{
			if(!popped[i])
			{
				glVertex2d(xCenter+balloons[i].X()-balloonWidth,yCenter-balloons[i].Z()-balloonWidth);
				glVertex2d(xCenter+balloons[i].X()-balloonWidth,yCenter-balloons[i].Z()+balloonWidth);
				glVertex2d(xCenter+balloons[i].X()+balloonWidth,yCenter-balloons[i].Z()+balloonWidth);
				glVertex2d(xCenter+balloons[i].X()+balloonWidth,yCenter-balloons[i].Z()-balloonWidth);
			}
		}
	glEnd();
	//draw borders
	glColor3f(0.1,0.1,0.1);
	glBegin(GL_QUADS);
	float width=4;
	float xborder=xMapStart-width/2;
		glVertex2d(xborder,0);
		glVertex2d(xborder+width,0);
		glVertex2d(xborder+width,mapSize+width/2);
		glVertex2d(xborder,mapSize+width/2);
		glVertex2d(xborder+width,mapSize+width/2);
		glVertex2d(xborder+width,mapSize-width/2);
		glVertex2d(scrW,mapSize-width/2);
		glVertex2d(scrW,mapSize+width/2);
	glEnd();
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}
void HUD::Init()
{
	/* Initialize the TTF library */
	gameOver=false;
	if(TTF_Init() < 0)
	{
		fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());
		SDL_Quit();
	}
	font = TTF_OpenFont("FreeSans.ttf", DEFAULT_PTSIZE);
	if(font == NULL)
	{
		fprintf(stderr, "Couldn't load font\n");
		SDL_Quit();
	}
	TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
	xMapStart=scrW-200;
	mapSize = 200.0;
	xPhotoStart=0;
	personalPhotoSize = 150.0;
}
void HUD::Render()
{
	drawPersonalPhoto();
	drawMap();
	drawPoints();
	if(gameOver)
		drawGameOver();
}
