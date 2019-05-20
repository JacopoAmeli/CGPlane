/*
 * Environment.cpp
 *
 *  Created on: 21 mag 2018
 *      Author: Jacopo Ameli
 */

#include "Environment.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#ifdef __APPLE__
#include <SDL2_image/SDL_image.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <SDL2/SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "mesh.h"
//riferimenti esterni
extern bool useWireframe;
extern bool useHeadlight;
extern GLuint grassTexture;
extern GLuint roadTexture;
extern GLuint skytopTexture;
extern GLuint skybottomTexture;
extern GLuint skyleftTexture;
extern GLuint skyfrontTexture;
extern GLuint skyrightTexture;
extern GLuint skybackTexture;
//meshes
Mesh  benchmesh((char *)"Objects/classic_pack/classic park bench.obj");
Mesh  lampmesh((char *)"Objects/classic_pack/classic quad light.obj");
Mesh  binmesh((char *)"Objects/classic_pack/classic bin.obj");

extern void SetupEnvmapTexture();

//attiva la luce del lampione
void Environment::drawLights()
{
	glPushMatrix();
	if(useHeadlight)
	{
		glEnable(GL_LIGHT1);
		//luce bianca
		float col0[4]= {0.9,0.9,0.9,  1};
		glLightfv(GL_LIGHT1, GL_DIFFUSE, col0);
		//niente componente ambientale
		float col1[4]= {0,0,0, 1};
		glLightfv(GL_LIGHT1, GL_AMBIENT, col1);
		//posizione centrata sul lampione
		float tmpPos[4] = {posxLamp,posyLamp+5,poszLamp,  1}; // ultima comp=1 => luce posizionale
		glLightfv(GL_LIGHT1, GL_POSITION, tmpPos );
		//direzione, verso il basso
		float tmpDir[4] = {0,-1,0,  1}; // ultima comp=1 => luce posizionale
		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, tmpDir );
		//45 gradi di apertura ed esponente (concentrazione)
		glLightf (GL_LIGHT1, GL_SPOT_CUTOFF, 45);
		glLightf (GL_LIGHT1, GL_SPOT_EXPONENT, 4);
		//attenuazioni
		glLightf(GL_LIGHT1,GL_CONSTANT_ATTENUATION,0);
		glLightf(GL_LIGHT1,GL_LINEAR_ATTENUATION,1);
	}
	else
	{
		glDisable(GL_LIGHT1);
	}
	glPopMatrix();
}
//disegna il lampione
void Environment::drawLamp()
{
	glPushMatrix();
	glTranslatef(posxLamp,posyLamp,poszLamp);
	glScalef(0.0023,0.0023,0.0023);
	 if (useWireframe)
	 {
	    glDisable(GL_TEXTURE_2D);
	    glColor3f(0,0,0);
	    glDisable(GL_LIGHTING);
	    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	    lampmesh.RenderNxV();
	    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	    glColor3f(1,1,1);
	    glEnable(GL_LIGHTING);
	 }
	 else
	 {
	    SetupEnvmapTexture();
	    glColor3f(0.2,0.2,0.2);
	    glEnable(GL_LIGHTING);
	    //modulate: cambia il colore della texture in base al colore corrente
	    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	    lampmesh.RenderNxV();
	  }
	glPopMatrix();
}
//disegna panchina
void Environment::drawBench()
{

	glPushMatrix();
	glTranslatef(benchPosx,benchPosy,benchPosz);
	glScalef(0.0023,0.0023,0.0023);
	 if (useWireframe)
	 {
	    glDisable(GL_TEXTURE_2D);
	    glColor3f(0,0,0);
	    glDisable(GL_LIGHTING);
	    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	    benchmesh.RenderNxV();
	    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	    glColor3f(1,1,1);
	    glEnable(GL_LIGHTING);
	 }
	 else
	 {
	    SetupEnvmapTexture();
	    glColor3f(-12,0.3,0.3);
	    glEnable(GL_LIGHTING);
	    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	    benchmesh.RenderNxV();
	  }
	glPopMatrix();
}
void Environment::drawBin()
{
	glPushMatrix();
	glTranslatef(posxBin,posyBin,poszBin);
	glScalef(0.0023,0.0023,0.0023);
	glDisable(GL_TEXTURE_2D);
	 if (useWireframe)
	 {
	    glColor3f(0,0,0);
	    glDisable(GL_LIGHTING);
	    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	    binmesh.RenderNxV();
	    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	    glColor3f(1,1,1);
	    glEnable(GL_LIGHTING);
	 }
	 else
	 {
	    glColor3f(0.1,0.1,0.1);
	    glEnable(GL_LIGHTING);
	    //no environment map
	    binmesh.RenderNxF();
	  }
	glPopMatrix();
}
//disegna la pista
void Environment::drawRunway()
{
	glPushMatrix();
	if (useWireframe)
	{
		    glDisable(GL_TEXTURE_2D);
		    glColor3f(0,0,0);
		    glDisable(GL_LIGHTING);
		    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		    glTranslatef(runwayPosx,runwayPosy,runwayPosz);
		    glBegin( GL_QUADS );
		    for(int i=-runwayRepeat;i<runwayRepeat;i++)
		    {
		    		glVertex3d(-runwaySize,runwayHeight,-runwaySize+i*2*runwaySize);
		    		glVertex3d(runwaySize,runwayHeight,-runwaySize+i*2*runwaySize);
		    		glVertex3d(runwaySize,runwayHeight,runwaySize+i*2*runwaySize);
		    		glVertex3d(-runwaySize,runwayHeight,runwaySize+i*2*runwaySize);
		    }
	    	glEnd();  //end draw
		    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		    glColor3f(1,1,1);
		    glEnable(GL_LIGHTING);
	}
	else
	{
		glColor3f(1.0,1.0,1.0);
		glPolygonMode(GL_BACK, GL_FILL);
		glEnable (GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		glBindTexture (GL_TEXTURE_2D, roadTexture);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
		glTranslatef(runwayPosx,runwayPosy,runwayPosz);
		glBegin( GL_QUADS );
		for(int i=-runwayRepeat;i<runwayRepeat;i++)
		{
			glTexCoord2d(0.0,0.0);glVertex3d(-runwaySize,runwayHeight,-runwaySize+i*2*runwaySize);
			glTexCoord2d(1,0.0);glVertex3d(runwaySize,runwayHeight,-runwaySize+i*2*runwaySize);
			glTexCoord2d(1,1);glVertex3d(runwaySize,runwayHeight,runwaySize+i*2*runwaySize);
			glTexCoord2d(0.0,1);glVertex3d(-runwaySize,runwayHeight,runwaySize+i*2*runwaySize);
		}
		glEnd();  //end draw
		glDisable (GL_TEXTURE_2D); // disable texture mapping
	}
	glPopMatrix();
}
//disegna il terreno
void Environment::drawGrass()
{
	glPushMatrix ();
	if (useWireframe)
	{
	    glDisable(GL_TEXTURE_2D);
	    glColor3f(0,0,0);
	    glDisable(GL_LIGHTING);
	    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	}
	else
	{
		glColor3f(0,0.5,0);
		glPolygonMode(GL_BACK, GL_FILL);
		glEnable (GL_TEXTURE_2D);
		glBindTexture (GL_TEXTURE_2D, grassTexture);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
		glEnable(GL_LIGHTING);
	}
	glBegin( GL_QUADS );
	for(int i=-subdivision;i<subdivision;i++)
	{
		for(int j=-subdivision;j<subdivision;j++)
		{
			if(i==0 && j<4 && j>-8)
			{
				//draw runway
			}
			else
			{
				if(useWireframe)
				{
					glVertex3d(-quadSize+i*2*quadSize,terrainHeight,-quadSize+j*2*quadSize);
					glVertex3d(quadSize+i*2*quadSize,terrainHeight,-quadSize+j*2*quadSize);
					glVertex3d(quadSize+i*2*quadSize,terrainHeight,quadSize+j*2*quadSize);
					glVertex3d(-quadSize+i*2*quadSize,terrainHeight,quadSize+j*2*quadSize);
				}
				else
				{
					glTexCoord2d(0.0,0.0); glVertex3d(-quadSize+i*2*quadSize,terrainHeight,-quadSize+j*2*quadSize);
					glTexCoord2d(1,0.0);   glVertex3d(quadSize+i*2*quadSize,terrainHeight,-quadSize+j*2*quadSize);
					glTexCoord2d(1,1); glVertex3d(quadSize+i*2*quadSize,terrainHeight,quadSize+j*2*quadSize);
					glTexCoord2d(0.0,1); glVertex3d(-quadSize+i*2*quadSize,terrainHeight,quadSize+j*2*quadSize);
				}
			}
		}
	}
	glEnd();  //end draw
	if(useWireframe)
	{
	    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		glColor3f(1,1,1);
		glEnable(GL_LIGHTING);
	}
	else
	{
		glDisable (GL_TEXTURE_2D); // disable texture mapping
	}
	 glPopMatrix();
}
//disegna lo skybox
void Environment::drawSkyCube()
{
	glPushMatrix();
	if (useWireframe)
	{
			glDisable(GL_TEXTURE_2D);
			glColor3f(0,0,0);
			glDisable(GL_LIGHTING);
			glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	}
	else
	{
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_LIGHTING);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
			glColor3f(1,1,1);
	}
	//top
	glBindTexture(GL_TEXTURE_2D,skytopTexture);
	glBegin(GL_QUADS);
		glTexCoord2d(0,1); glVertex3d(-terrainSize,terrainSize-2,-terrainSize);
		glTexCoord2d(1,1); glVertex3d(terrainSize,terrainSize-2,-terrainSize);
		glTexCoord2d(1,0);  glVertex3d(terrainSize,terrainSize-2,terrainSize);
		glTexCoord2d(0,0); glVertex3d(-terrainSize,terrainSize-2,terrainSize);
	glEnd();
	//left
	glBindTexture(GL_TEXTURE_2D,skyleftTexture);
	glBegin(GL_QUADS);
		glTexCoord2d(0,1); glVertex3d(-terrainSize,-terrainSize,terrainSize);
		glTexCoord2d(1,1);  glVertex3d(-terrainSize,-terrainSize,-terrainSize);
		glTexCoord2d(1,0); glVertex3d(-terrainSize,terrainSize,-terrainSize);
		glTexCoord2d(0,0);  glVertex3d(-terrainSize,terrainSize,terrainSize);
	glEnd();
	//front
	glBindTexture(GL_TEXTURE_2D,skyfrontTexture);
	glBegin(GL_QUADS);
		glTexCoord2d(0,0); glVertex3d(-terrainSize,terrainSize,-terrainSize);
		glTexCoord2d(1,0);  glVertex3d(terrainSize,terrainSize,-terrainSize);
		glTexCoord2d(1,1); glVertex3d(terrainSize,-terrainSize,-terrainSize);
		glTexCoord2d(0,1); glVertex3d(-terrainSize,-terrainSize,-terrainSize);
	glEnd();
	//left
	glBindTexture(GL_TEXTURE_2D,skyrightTexture);
	glBegin(GL_QUADS);
		glTexCoord2d(0,1); glVertex3d(terrainSize,-terrainSize,-terrainSize);
		glTexCoord2d(1,1);  glVertex3d(terrainSize,-terrainSize,terrainSize);
		glTexCoord2d(1,0); glVertex3d(terrainSize,terrainSize,terrainSize);
		glTexCoord2d(0,0);  glVertex3d(terrainSize,terrainSize,-terrainSize);
	glEnd();
	//back
	glBindTexture(GL_TEXTURE_2D,skybackTexture);
	glBegin(GL_QUADS);
		glTexCoord2d(0,0); glVertex3d(terrainSize,terrainSize,terrainSize);
		glTexCoord2d(1,0); glVertex3d(-terrainSize,terrainSize,terrainSize);
		glTexCoord2d(1,1); glVertex3d(-terrainSize,-terrainSize,terrainSize);
		glTexCoord2d(0,1);  glVertex3d(terrainSize,-terrainSize,terrainSize);
	glEnd();
	//top
	glBindTexture(GL_TEXTURE_2D,skybottomTexture);
	glBegin(GL_QUADS);
		glTexCoord2d(0,1); glVertex3d(-terrainSize,-terrainSize,-terrainSize);
		glTexCoord2d(1,1); glVertex3d(terrainSize,-terrainSize,-terrainSize);
		glTexCoord2d(1,0);  glVertex3d(terrainSize,-terrainSize,terrainSize);
		glTexCoord2d(0,0); glVertex3d(-terrainSize,-terrainSize,terrainSize);
	glEnd();
	if (useWireframe)
	{
			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
			glColor3f(1,1,1);
	}
	else
	{
			 glDisable(GL_TEXTURE_2D);
	}
	glPopMatrix();
}
void Environment::Init()
{
	benchPosx=10,benchPosy=1.01,benchPosz=-10;
	posxLamp=10,posyLamp=1.01,poszLamp=-9.5;
	posxBin=10,posyBin=1.01,poszBin=-10.5;
	runwaySize = 2.5;
	runwayRepeat=6;
	runwayPosx=0,runwayPosy=0,runwayPosz=-(runwayRepeat-2)*runwaySize;
	runwayHeight=1;
	terrainSize = 400.0;
	subdivision = 160.0;
	quadSize=(double)terrainSize/subdivision;
	terrainHeight = 1;
}

void Environment::Render()
{
	drawSkyCube();
	drawLights();
	drawGrass();
	drawRunway();
	drawBench();
	drawBin();
	drawLamp();
}

