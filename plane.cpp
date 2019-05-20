/*
 * plane.cpp
 *
 *  Created on: 01 mag 2018
 *      Author: Jacopo Ameli
 */

#include <stdio.h>
#include <math.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <vector>

#include "plane.h"
#include "mesh.h"

// var globali di tipo mesh
Mesh aereomesh((char *)"Objects/MainPlaneTest2.obj");
Mesh elicamesh((char *)"Objects/MainHelixTest.obj");
Mesh flapR1mesh((char *)"Objects/MainDestra1Test.obj");
Mesh flapR2mesh((char *)"Objects/MainDestra2Test.obj");
Mesh flapL1mesh((char *)"Objects/MainSinistra1Test.obj");
Mesh flapL2mesh((char *)"Objects/MainSinistra2Test.obj");
Mesh tailmesh((char *)"Objects/MainTailTest.obj");
Mesh timonemesh((char *)"Objects/MainTimoneTest.obj");
Mesh wheelmesh((char *)"Objects/MainWheelTest.obj");

extern bool useEnvmap;
extern bool usePhysics;

// da invocare quando e' stato premuto/rilasciato il tasto numero "keycode"
void Controller::EatKey(int keycode, int* keymap, bool pressed_or_released)
{
  for (int i=0; i<NKEYS; i++)
  {
    if (keycode==keymap[i]) key[i]=pressed_or_released;
  }
}

// da invocare quando e' stato premuto/rilasciato un jbutton
void Controller::Joy(int keymap, bool pressed_or_released)
{
    key[keymap]=pressed_or_released;
}

// DoStep: facciamo un passo di fisica (a delta_t costante)
//
// Indipendente dal rendering.
// la struttura controller da DoStep
void Plane::DoStep()
{
	float da;
	  // gestione dello sterzo
	  if (controller.key[Controller::LEFT])
	  {
		  timone+=velTimone;
		  normal= Vector3(0,1,0);
		  axis3.fromAxis(normal, +velTimone);//imposto quaternione da asse forward vector e angolo da girare
		  normal = axis3.operator *(normal);		//effettuo trasformazione
		  axis3.normalise();			//normalizzo
		  transform.normalise();
		  transform = transform * axis3;//aggiorno la trasformata totale aereo
	  }
	  if (controller.key[Controller::RIGHT])
	  {
		  timone-=velTimone;
		  normal= Vector3(0,1,0);
		  axis3.fromAxis(normal, -velTimone);//imposto quaternione da asse forward vector e angolo da girare
		  normal = axis3.operator *(normal);		//effettuo trasformazione
		  axis3.normalise();			//normalizzo
		  transform.normalise();
		  transform = transform * axis3;//aggiorno la trasformata totale aereo
	  }
	  timone*=velRitornoTimone; // ritorno a volante dritto

	  if (controller.key[Controller::LEFTROLL])
	  {
		  //Per fare un roll a sinistra, il flap sinistro si alza e quello destro si abbassa
		  if(!usePhysics || planePos[1]>1.01)
		  {
			  flapL+=velFlap;
			  forwardVector = Vector3(0,0,1);  //carico il vettore base forward vector (asse z)
			  barrel = Vector3(-1,0,0);  //carico vettore orizzontale (x)
			  axis1.fromAxis(forwardVector, +velFlap);		 //imposto quaternione da asse forward vector e angolo da girare
			  barrel = axis1.operator *(barrel);
			  axis1.normalise();
			  transform.normalise();
			  transform = transform * axis1;
		  }
	  }
	  if (controller.key[Controller::RIGHTROLL])
	  {
	 	  //Per fare un roll a destra, il flap sinistro si abbassa e quello destro si alza
		  if(!usePhysics || planePos[1]>1.01)
		  {
			  flapL-=velFlap;
			  forwardVector = Vector3(0,0,1);
			  barrel = Vector3(-1,0,0);
			  axis1.fromAxis(forwardVector, -velFlap);
			  barrel = axis1.operator *(barrel);
			  axis1.normalise();
			  transform.normalise();
			  transform = transform * axis1;
		  }
	  }
	  flapL*=velRitornoFlap;

	  if (controller.key[Controller::PITCHDOWN])
	  {
	  	  //Per fare un pitch down
		  if(!usePhysics || planePos[1]>1.01)
		  {
			  tail+=velFlap;
			  forwardVector = Vector3(0,0,1);   //set base vectors
			  barrel = Vector3(-1,0,0);
			  axis2.fromAxis(barrel, +velFlap);	//set rotation axis, angle
			  axis2.normalise();					//normalize the quaternions
	  	  	  transform.normalise();
	  	  	  transform =  transform * axis2;
		  }//update the tranformation quaternion
	  }
	  if (controller.key[Controller::PITCHUP])
	  {
	  	  //Per fare un pitch up
	  	  tail-=velFlap;
	  	  forwardVector = Vector3(0,0,1);		//set base vectors
	  	  barrel = Vector3(-1,0,0);
	  	  axis2.fromAxis(barrel, -velFlap);	//set rotation axis, angle
	  	  axis2.normalise();					//normalize the quaternions
	  	  transform.normalise();
	  	  transform =  transform * axis2;	//upate the transform axis
	    }
	  tail*=velRitornoFlap;
	  if (controller.key[Controller::ACC]) speed-=accMax; // accelerazione in avanti
	  if (controller.key[Controller::DEC])
	  {
		  //l'aereo non può fermarsi completamente a mezz'aria
		  if(usePhysics && planePos[1]>1.01)
		  {
			  if(speed<-0.1)
				  speed+=accMax*0.4;
		  }
		  else
		  {
			  speed+=accMax*0.5; // accelerazione indietro
		  }
		  if(speed>0)
			  speed=0;
	  }
	  if(usePhysics && speed>threshold && planePos[1]>1.01)
	  {
		  	 //simula aereo in caduta libera, no dinamica
		  speedCaduta+=accCaduta;
		  speedCaduta*=attritoAria;
		  planePos[1]=planePos[1]-speedCaduta;
	  }
	  else
	  {
		  speedCaduta=0;
	  }
	  da=(360.0*speed)/(2.0*M_PI*raggioElica);
	  mozzoE+=da;

	  //aggiorna la posizione in base al quaternione e alla velocità
	  Vector3 move(0,0,1);
	  move = transform.operator *(move);
	  move=move.Normalize();
	  speed=speed*attritoAria;
	  planePos[0] = planePos[0] + move.X() * speed;
	  planePos[1] = planePos[1] + move.Y() * speed;
	  planePos[2] = planePos[2] + move.Z() * speed;
	  //impedisce di entrare nel terreno
	  if(usePhysics && planePos[1]<1.001)
	  {
		  transform=Quaternion(0,transform.y,0,transform.w);
		  transform.normalise();
		  planePos[1]=1.001;
	  }
}

void Controller::Init()
{
  for (int i=0; i<NKEYS; i++) key[i]=false;
}

void Plane::Init()
{
	// inizializzo lo stato aereo
	normal = Vector3(0,1,0);
	forwardVector = Vector3(0,0,1);
	barrel = Vector3(-1,0,0);
	transform.fromAxis(forwardVector,0);
	planePos[0]=0;
	planePos[1]=1;
	planePos[2]=0;
	timone=flapL=tail=0;
	controller.Init();
	velTimone=0.01;
	velRitornoTimone=0.93;
	timoneRate=100;
	velFlap=0.01;
  	velRitornoFlap=0.93;
  	flapRate=100;
  	raggioElica=0.1;
  	accMax = 0.001;
  	speed=0;
  	attritoAria=0.99;
  	threshold=-0.04;
  	accCaduta=0.001;
  	speedCaduta=0;
}
extern void SetupEnvmapTexture();
//disegna l'elica
void Plane::drawElica(bool usecolor) const
{
	 glPushMatrix();
	 glTranslate(  elicamesh.Center() );
	 glRotatef(mozzoE,0,0,1);
	 glTranslate( -elicamesh.Center() );
	 if (usecolor) glColor3f(0.9,0,0);
	 elicamesh.RenderNxV();
	 glPopMatrix();
}
//disegna i 4 flap
void Plane::drawFlaps(bool usecolor) const
{
	//flap left 1
	glPushMatrix();
	glTranslate(flapL1mesh.Center());
	glTranslatef(0,0,20);
	glRotatef(flapL*flapRate,1,0,0);
	glTranslatef(0,0,-20);
	glTranslate(-flapL1mesh.Center());
	if (usecolor) glColor3f(0.9,0,0);
	flapL1mesh.RenderNxV();
	glPopMatrix();

	//flap left 2
	glPushMatrix();
	glTranslate(flapL2mesh.Center());
	glTranslatef(0,0,20);
	glRotatef(flapL*flapRate,1,0,0);
	glTranslatef(0,0,-20);
	glTranslate(-flapL2mesh.Center());
	if (usecolor) glColor3f(0.9,0,0);
	flapL2mesh.RenderNxV();
	glPopMatrix();

	//flap right 1
	glPushMatrix();
	glTranslate(flapR1mesh.Center());
	glTranslatef(0,0,20);
	glRotatef(-flapL*flapRate,1,0,0);
	glTranslatef(0,0,-20);
	glTranslate(-flapR1mesh.Center());
	if (usecolor) glColor3f(0.9,0,0);
	flapR1mesh.RenderNxV();
	glPopMatrix();

	//flap right 1
	glPushMatrix();
	glTranslate(flapR2mesh.Center());
	glTranslatef(0,0,20);
	glRotatef(-flapL*flapRate,1,0,0);
	glTranslatef(0,0,-20);
	glTranslate(-flapR2mesh.Center());
	if (usecolor) glColor3f(0.9,0,0);
	flapR2mesh.RenderNxV();
	glPopMatrix();
}
//disegna la coda
void Plane::drawTail(bool usecolor) const
{
	glPushMatrix();
	glTranslate(tailmesh.Center());
	glTranslatef(0,0,20);
	glRotatef(tail*flapRate,1,0,0);
	glTranslatef(0,0,-20);
	glTranslate(-tailmesh.Center());
	if (usecolor) glColor3f(0.9,0,0);
	tailmesh.RenderNxV();
	glPopMatrix();
}
//disegna il timone
void Plane::drawTimone(bool usecolor) const
{
	  glPushMatrix();
	  glTranslate(  timonemesh.Center() );
	  glTranslatef(0,0,20);
	  glRotatef(-timone*timoneRate,0,1,0);
	  glTranslatef(0,0,-20);
	  glTranslate( -timonemesh.Center() );
	  if (usecolor) glColor3f(0.9,0,0);
	  timonemesh.RenderNxV();
	  glPopMatrix();
}
//disegna le ruote
void Plane::drawWheels(bool usecolor) const
{
	glPushMatrix();
	if (usecolor) glColor3f(0.3,0.3,0.3);
	glDisable(GL_TEXTURE_2D);
	wheelmesh.RenderNxF();
	glPopMatrix();
}
// funzione che disegna tutti i pezzi dell'aereo
void Plane::RenderAllParts(bool usecolor) const
{
  glPushMatrix();
  glScalef(-0.003,0.003,-0.003); // patch: riscaliamo la mesh
  glTranslatef(0,125,0);
  if (!useEnvmap)
  {
    if (usecolor) glColor3f(0,0.9,0.9);     // colore rosso, da usare con Lighting
  }
  else {
    if (usecolor)
    	{
    		SetupEnvmapTexture();
    		glColor3f(0,0.9,0.9);
    		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    	}
  }
  if (usecolor) glEnable(GL_LIGHTING);
  aereomesh.RenderNxV(); // rendering delle mesh usando normali per vertice

  //render elica
  drawElica(usecolor);

  //render timone
  drawTimone(usecolor);

  //render flaps
  drawFlaps(usecolor);

  //render tail
  drawTail(usecolor);

  //render wheels
  drawWheels(usecolor);

  glPopMatrix();
}
void Plane::RenderShadow()
{
	float height=1.051;
	glPushMatrix();
	glColor4f(0.2,0.2,0.2,0.5); // colore fisso
	glTranslatef(planePos[0],height,planePos[2]); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
	Quaternion shadowTransform= Quaternion(transform.x,transform.y,transform.z,transform.w);
	glScalef(1.01,0,1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X
	glMultMatrixf((GLfloat*) shadowTransform.getMatrix());  //apply quaternion transformation?
	glDisable(GL_LIGHTING); // niente lighing per l'ombra
	RenderAllParts(false);  // disegno aereo
	glEnable(GL_LIGHTING);
	glPopMatrix();
}
// disegna a schermo
void Plane::Render() {
  // sono nello spazio mondo
	glPushMatrix ();
	glMatrixMode(GL_MODELVIEW);
	glTranslatef (planePos[0],planePos[1],planePos[2]);
	glMultMatrixf((GLfloat*) transform.getMatrix());  //apply quaternion transformation?
	RenderAllParts(true);
	glPopMatrix ();
}

