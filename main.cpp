#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#include <vector>
#include "plane.h"			//gestisce fisica e rendering aereo
#include "Environment.h"	//gestisce oggetti, terreno, skybox
#include "HUD.h" 			//gestisce mappa, punteggio e fotografia personale
#include "mesh.h"			//gestisce caricamento e rendering mesh

#define CAMERA_BACK_PLANE 0	//telecamera da dietro, non segue l'aereo perfettamente
#define CAMERA_BACK_FIXED 1	//telecamera da dietro, segue perfettamente
#define CAMERA_TOP_FIXED 2	//telecamera dall'alto a destra
#define CAMERA_TOP_PLANE 3	//telecamera dall'alto
#define CAMERA_PILOT 4		//telecamera pilota
#define CAMERA_MOUSE 5		//telecamera libera
#define CAMERA_TYPE_MAX 6
#define NUMBALLOONS 15

float viewAlpha=20, viewBeta=40; 		// angoli che definiscono la vista
float eyeDist=5.0; 						// distanza dell'occhio dall'origine
double cameraHeight = 0.59;             //altezza posizione camera
double cameraReferenceHeight = 0.93;    //quanto è alzato il punto da guardare davanti l'aereo
double cameraforwardVectorFactor = 2.5;	//quanto è distante la telecamera
int cameraType=0;						//telecamera iniziale

int scrH=750, scrW=1000; 	// altezza e larghezza viewport (in pixels)
bool useWireframe=false;	//rendering solo wireframe
bool useEnvmap=true;		//usa Environment Mapping
bool useHeadlight=false;	//utilizza luci aggiuntive
bool useShadow=false;		//accendi/spegni ombra
bool usePhysics=true;		//utilizza fisica e collisioni

void *__gxx_personality_v0; //errore a runtime se variabile non dichiarata, ha a che fare con l'error reporting

SDL_Window *win;

Plane plane; 						// il nostro aereo
Environment environment;			// il nostro ambientee
HUD hud;							// il nostro HUD
int nstep=0; 						// numero di passi di FISICA fatti fin'ora
const int PHYS_SAMPLING_STEP=10; 	// numero di millisec che un passo di fisica simula

const int fpsSampling = 3000;	// lunghezza intervallo di calcolo fps
float fps=0; 					// valore di fps dell'intervallo precedente
int fpsNow=0;					// quanti fotogrammi ho disegnato fin'ora nell'intervallo attuale
Uint32 timeLastInterval=0; 		// quando e' cominciato l'ultimo intervallo

//riferimento texture:
GLuint grassTexture;	//texture erba
GLuint roadTexture;		//texture pista
GLuint logoTexture;		//texture logo applicato
GLuint envMapTexture;	//texture Environment Map
GLuint jacopoTexture;	//texture foto personale

//texture cubemap per la skybox
GLuint skytopTexture;
GLuint skybottomTexture;
GLuint skyleftTexture;
GLuint skyfrontTexture;
GLuint skyrightTexture;
GLuint skybackTexture;

//mesh palloncini
Mesh balloonmesh((char*)"Objects/balloon.obj");
Mesh stringmesh((char*)"Objects/balloonstring.obj");

//posizione palloncini
Vector3 balloons[NUMBALLOONS];
bool popped[NUMBALLOONS];
int totalpopped=0;

// Funzione che prepara tutto per usare un env map
void SetupEnvmapTexture()
{
	if(useEnvmap)
	{
		// facciamo binding con la texture
		glBindTexture(GL_TEXTURE_2D,envMapTexture);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_GEN_S); // abilito la generazione automatica delle coord texture S e T
		glEnable(GL_TEXTURE_GEN_T);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP); // Env map
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP);
		glColor3f(1,1,1); // metto il colore neutro (viene moltiplicato col colore texture, componente per componente)
		glDisable(GL_LIGHTING); // disabilito il lighting OpenGL standard (lo faccio con la texture)
	}
}
//Funzione che imposta la texture da applicare sul palloncino
void SetupBalloonTexture(Vector3 min, Vector3 max)
{
	if(useEnvmap)
	{
		glBindTexture(GL_TEXTURE_2D,logoTexture);
		  glEnable(GL_TEXTURE_2D);
		  glEnable(GL_TEXTURE_GEN_S);
		  glEnable(GL_TEXTURE_GEN_T);
		  // devo essere in coordinate OGGETTO
		  // cioe' le coordnate originali, PRIMA della moltiplicazione per la ModelView
		  // in modo che la texture sia "attaccata" all'oggetto, e non "proiettata" su esso
		  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE , GL_OBJECT_LINEAR);
		  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE , GL_OBJECT_LINEAR);
		  float sz=(float) (1.0/(max.Z() - min.Z()));
		  float ty=(float) (1.0/(max.Y() - min.Y()));
		  float s[4]={0,0,sz, (float)(- min.Z()*sz) };
		  float t[4]={0,ty,0, (float)(- min.Y()*ty) };
		  glTexGenfv(GL_S, GL_OBJECT_PLANE, s);
		  glTexGenfv(GL_T, GL_OBJECT_PLANE, t);
	}
}
// setta le matrici di trasformazione in modo
// che le coordinate in spazio oggetto siano le coord 
// del pixel sullo schemo
void  SetCoordToPixel()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(-1,-1,0);
  glScalef(2.0/scrW, 2.0/scrH, 1);
}
//carica una texture, collegata a textbind
bool loadTexture(GLuint textbind,char *filename)
{
	GLenum texture_format;
	SDL_Surface *s = IMG_Load(filename);
	if (!s) return false;
	if (s->format->BytesPerPixel == 4)
	{     // contiene canale alpha
		if (s->format->Rmask == 0x000000ff)
		{
			texture_format = GL_RGBA;
		}
		else
		{
			texture_format = GL_RGBA;
		}
	}
	else if (s->format->BytesPerPixel == 3)
	{     // non contiene canale alpha
     if (s->format->Rmask == 0x000000ff)
       texture_format = GL_RGB;
     else
       texture_format = GL_RGB;
    }
	else
	{
        printf("[ERROR] the image is not truecolor\n");
        exit(1);
    }
	glGenTextures(1,&textbind);
	glBindTexture(GL_TEXTURE_2D, textbind);
	gluBuild2DMipmaps(
			GL_TEXTURE_2D,
			3,
			s->w, s->h,
			texture_format,
			GL_UNSIGNED_BYTE,
			s->pixels);
	//clamp to edge: migliora la resa sui bordi
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);
	glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	return true;
}
// setto la posizione della camera
void setCamera()
{
		Vector3 move(0,0,-1);  				//asse z cioè direzione dell'aereo
	    move = plane.transform * move;		//applica il quaternione transform per ottenere la direzione reale
	    move=move.Normalize();				//normalizza

	    //ottiene la normale (verso l'alto) rispetto all'aereo, viewup vector
		plane.normal = Vector3(0,1,0);
		plane.normal = plane.transform * plane.normal;
		plane.normal=plane.normal.Normalize();

		// controllo la posizione della camera a seconda dell'opzione selezionata
        switch (cameraType)
        {
        	case CAMERA_BACK_PLANE:
        	{
        		//camera normale: segue l'aereo da dietro in maniera "naturale"
        		Vector3 dir(-1*move.X()*cameraforwardVectorFactor,
        				-1*move.Y()*cameraforwardVectorFactor,
        				-1*move.Z()*cameraforwardVectorFactor); //inverso del vettore di movimento, scalato
        		double aty = plane.normal.Y() * cameraHeight;	//altezza camera rispetto alla y della normale
        		double looky = plane.normal.Y() * cameraReferenceHeight;
        		gluLookAt(plane.planePos[0] + dir.X() , plane.planePos[1] + dir.Y() + aty,  plane.planePos[2] + dir.Z() ,
        				plane.planePos[0]+ move.X(), plane.planePos[1] + move.Y()+ looky, plane.planePos[2] + move.Z() ,
						plane.normal.X(),plane.normal.Y(),plane.normal.Z());
        		break;
        	}
        	case CAMERA_BACK_FIXED:
        	{
        		//camera che segue l'aereo perfettamente
        		Vector3 dir(-1*move.X()*cameraforwardVectorFactor+plane.normal.X()*cameraHeight*1.5,
        		        	-1*move.Y()*cameraforwardVectorFactor+plane.normal.Y()*cameraHeight*1.5,
        		        	-1*move.Z()*cameraforwardVectorFactor+plane.normal.Z()*cameraHeight*1.5); //posizione camera
        		Vector3 look(move.X()*cameraforwardVectorFactor,
        		        	move.Y()*cameraforwardVectorFactor,
							move.Z()*cameraforwardVectorFactor); //posizione obiettivo
        		 gluLookAt(plane.planePos[0] + dir.X() , plane.planePos[1] + dir.Y(),  plane.planePos[2] + dir.Z() ,
        		        plane.planePos[0]+ look.X(), plane.planePos[1] + look.Y(), plane.planePos[2] + look.Z() ,
        		        plane.normal.X(),plane.normal.Y(),plane.normal.Z());
        		break;
        	}
        	case CAMERA_TOP_FIXED:
        	{
        		//camera fissa dall'alto a destra
        		Vector3 moveRight(1,0,0);					 //asse x per spostamento a destra
        		moveRight = plane.transform * moveRight;	 //relativo all'aereo
        		moveRight=moveRight.Normalize();
        		Vector3 dir(-1*move.X()*cameraforwardVectorFactor*0.3+plane.normal.X()*cameraHeight+moveRight.X()*0.6,
        		        	-1*move.Y()*cameraforwardVectorFactor*0.3+plane.normal.Y()*cameraHeight+moveRight.Y()*0.6,
							-1*move.Z()*cameraforwardVectorFactor*0.3+plane.normal.Z()*cameraHeight+moveRight.Z()*0.6);
        		Vector3 look(move.X()*cameraforwardVectorFactor*0.5,
        		        	move.Y()*cameraforwardVectorFactor*0.5,
							move.Z()*cameraforwardVectorFactor*0.5);
        		gluLookAt(plane.planePos[0] + dir.X() , plane.planePos[1] + dir.Y(),  plane.planePos[2] + dir.Z() ,
        		        	plane.planePos[0]+ look.X(), plane.planePos[1] + look.Y(), plane.planePos[2] + look.Z() ,
							plane.normal.X(),plane.normal.Y(),plane.normal.Z());
                break;
        	}
        	case CAMERA_TOP_PLANE:
        	{
        		//camera che segue l'aereo dall'alto
        		Vector3 dir(-1*move.X()*cameraforwardVectorFactor+plane.normal.X()*cameraHeight*10,
        		        	-1*move.Y()*cameraforwardVectorFactor+plane.normal.Y()*cameraHeight*10,
        		        	-1*move.Z()*cameraforwardVectorFactor+plane.normal.Z()*cameraHeight*10);
        		Vector3 look(move.X()*cameraforwardVectorFactor,
        		        	move.Y()*cameraforwardVectorFactor,
        		        	move.Z()*cameraforwardVectorFactor);
        		gluLookAt(plane.planePos[0] + dir.X() , plane.planePos[1] + dir.Y(),  plane.planePos[2] + dir.Z() ,
        		        plane.planePos[0]+ look.X(), plane.planePos[1] + look.Y(), plane.planePos[2] + look.Z() ,
        				plane.normal.X(),plane.normal.Y(),plane.normal.Z());
                break;
        	}
        	case CAMERA_PILOT:
        	{
        		//camera dalla prospettiva del pilota
        		Vector3 dir(-1*move.X()*cameraforwardVectorFactor*0.25+plane.normal.X()*cameraHeight*1.05,
        		        	-1*move.Y()*cameraforwardVectorFactor*0.25+plane.normal.Y()*cameraHeight*1.05,
							-1*move.Z()*cameraforwardVectorFactor*0.25+plane.normal.Z()*cameraHeight*1.05);
        		Vector3 look(move.X()*cameraforwardVectorFactor,
        		        	move.Y()*cameraforwardVectorFactor,
							move.Z()*cameraforwardVectorFactor);
        		gluLookAt(plane.planePos[0] + dir.X() , plane.planePos[1] + dir.Y(),  plane.planePos[2] + dir.Z() ,
        				plane.planePos[0]+ look.X(), plane.planePos[1] + look.Y(), plane.planePos[2] + look.Z() ,
        		        plane.normal.X(),plane.normal.Y(),plane.normal.Z());
                break;
        	}
        	case CAMERA_MOUSE:
        	{
                glTranslatef(0,0,-eyeDist);
                glRotatef(viewBeta,  1,0,0);
                glRotatef(viewAlpha, 0,1,0);
                break;
        	}
        }
}
//disegna i palloncini che non sono ancora scoppiati
void drawBalloons()
{
	for(int i=0;i<NUMBALLOONS;i++)
	{
		if(!popped[i])
		{
			glPushMatrix();
			glTranslatef(balloons[i].X(),balloons[i].Y(),balloons[i].Z());
			glRotatef(-90,1,0,0);
			glScalef(0.009,0.009,0.009);
			if (useWireframe)
			{
				  glDisable(GL_TEXTURE_2D);
				  glColor3f(0,0,0);
				  glDisable(GL_LIGHTING);
				  glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
				  balloonmesh.RenderNxV();
				  stringmesh.RenderNxV();
				  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
				  glColor3f(1,1,1);
				  glEnable(GL_LIGHTING);
			}
			else
			{
				glDisable(GL_TEXTURE_2D);
				glColor3f(0.8,0.1,0.1);
				SetupBalloonTexture(balloonmesh.bbmin,balloonmesh.bbmax);
				balloonmesh.RenderNxV();
				glDisable(GL_TEXTURE_2D);
				glColor3f(1,1,1);
				stringmesh.RenderNxV();
			}
			glPopMatrix();
		}
	}
}
//controlla se l'aereo interseca uno dei palloncini, in caso lo fa "scoppiare"
//le bounding box sono cubi semplici
void checkBalloons()
{
	float x=plane.planePos[0];
	float y=plane.planePos[1];
	float z=plane.planePos[2];
	float bmin=-0.45;
	float bmax=0.45;
	float offset=2.2;
	for(int i=0;i<NUMBALLOONS;i++)
	{
		if(!popped[i] && x+bmin<=balloons[i].X()+bmax
				&& y+bmin<=balloons[i].Y()+offset+bmax
				&& z+bmin<=balloons[i].Z()+bmax
				&& x+bmax>=balloons[i].X()+bmin
				&& y+bmax>=balloons[i].Y()+offset+bmin
				&& z+bmax>=balloons[i].Z()+bmin)
		{
			popped[i]=true;
			totalpopped++;
		}
	}
}
/* Esegue il Rendering della scena */
void rendering(SDL_Window *win)
{
  // un frame in piu'
  fpsNow++;
  glLineWidth(3); // linee larghe
  // settiamo il viewport
  glViewport(0,0, scrW, scrH);
  // colore sfondo = bianco
  glClearColor(1,1,1,1);
  glEnable(GL_POLYGON_SMOOTH);
  // settiamo la matrice di proiezione
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluPerspective( 70, //fovy,
		((float)scrW) / scrH,//aspect Y/X,
		0.15,//distanza del NEAR CLIPPING PLANE in coordinate vista
		900  //distanza del FAR CLIPPING PLANE in coordinate vista
  );
  glMatrixMode( GL_MODELVIEW ); 
  glLoadIdentity();
  // riempe tutto lo screen buffer di pixel color sfondo
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
 // setto la posizione luce
  float tmpv[4] = {0,1,2,  0}; // ultima comp=0 => luce direzionale
  glLightfv(GL_LIGHT0, GL_POSITION, tmpv );
  float tmpamb[4] = {0.2,0.2,0.2,1};
  glLightfv(GL_LIGHT0, GL_AMBIENT, tmpamb);
  setCamera();
  static float tmpcol[4] = {1,1,1,  1};
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tmpcol);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 127);
  glEnable(GL_LIGHTING);

  //rendering ambiente, aereo,hud
  environment.Render();
  plane.Render();
  if(useShadow)
	  plane.RenderShadow();
  drawBalloons();
  hud.Render();;
  
  glFinish();
  // ho finito: buffer di lavoro diventa visibile
  SDL_GL_SwapWindow(win);
}
//autoinvia un messaggio che (s.o. permettendo)
//fara' ridisegnare la finestra
void redraw()
{
  SDL_Event e;
  e.type=SDL_WINDOWEVENT;
  e.window.event=SDL_WINDOWEVENT_EXPOSED;
  SDL_PushEvent(&e);
}
//inizializza posizioni random dei palloncini e altre variabili
void initBalloons()
{
	float x,y,z;
	srand(SDL_GetTicks());//seed per random generator
	for(int i=0;i<NUMBALLOONS;i++)
	{
		x=(float)(rand()%120 - 60);
		y=(float)(rand()%100 + 5);
		z=(float)(rand()%120 - 60);
		balloons[i]=Vector3(x,y,z);
		popped[i]=false;
		totalpopped=0;
		hud.gameOver=false;
	}
}
//imposta parametri opengl e carica texture e inizializza palloncini
int initGL()
{

	glClearColor(0.53,0.81,0.98,1.0); // clear color is gray
	glEnable (GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE); // opengl, per favore, rinormalizza le normali
	glShadeModel(GL_SMOOTH);     //sets shading model, which is how colors are iterpolated across polygons
	glFrontFace(GL_CW); // consideriamo Front Facing le facce ClockWise
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_POLYGON_OFFSET_FILL); // openGL sposta i
	                                    // frammenti generati dalla
	                                    // rasterizzazione poligoni
	glPolygonOffset(0.9f,0.9f);             // indietro di 1
	logoTexture=1;
	envMapTexture=2;
	roadTexture=3;
	grassTexture=4;
	jacopoTexture=5;
	skytopTexture=6;
	skybottomTexture=7;
	skyleftTexture=8;
	skyfrontTexture=9;
	skyrightTexture=10;
	skybackTexture=11;
	if (!loadTexture(logoTexture,(char *)"Textures/logo.jpg")) return -1;
	if (!loadTexture(envMapTexture,(char *)"Textures/envmap_flipped.jpg")) return -1;
	if (!loadTexture(roadTexture,(char *)"Textures/runway.jpg")) return -1;
	if (!loadTexture(grassTexture,(char *)"Textures/grass2.jpg")) return -1;
	if (!loadTexture(jacopoTexture,(char *)"Textures/jacopo.jpg")) return -1;
	if (!loadTexture(skytopTexture,(char *)"Textures/skytop.jpg")) return -1;
	if (!loadTexture(skybottomTexture,(char *)"Textures/skybottom.jpg")) return -1;
	if (!loadTexture(skyleftTexture,(char *)"Textures/skyleft.jpg")) return -1;
	if (!loadTexture(skyfrontTexture,(char *)"Textures/skyfront.jpg")) return -1;
	if (!loadTexture(skyrightTexture,(char *)"Textures/skyright.jpg")) return -1;
	if (!loadTexture(skybackTexture,(char *)"Textures/skyback.jpg")) return -1;
	initBalloons();
	printf("Init. complete\n");
	return 0;
}

int main(int argc, char* argv[])
{
	SDL_GLContext mainContext;
	Uint32 windowID;
	SDL_Joystick *joystick;
	//keymapping :
	static int keymap[Controller::NKEYS] =
	//sinistra,destra,accelera,frena, roll sx,roll dx,pitch up,pitch down
	{SDLK_q, SDLK_e, SDLK_UP, SDLK_DOWN,SDLK_a,SDLK_d,SDLK_w,SDLK_s};
	// inizializzazione di SDL
	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	SDL_JoystickEventState(SDL_ENABLE);
	joystick = SDL_JoystickOpen(0);
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	// facciamo una finestra di scrW x scrH pixels
 	win=SDL_CreateWindow(argv[0], 0, 0, scrW, scrH, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
 	plane.Init();
 	environment.Init();
 	hud.Init();
 	//mi serve la bounding box per l'applicazione della texture
 	balloonmesh.ComputeBoundingBox();
 	//crea il contesto opengl principale e collegato alla window
 	mainContext=SDL_GL_CreateContext(win);
 	if(initGL()!=0)
 		return -1;
 	bool done=0;
 	//ciclo principale
 	while (!done)
 	{
 		bool doneSomething=false;
 		SDL_Event e;
 		// guardo se c'e' un evento:
 		if (SDL_PollEvent(&e))
 		{
 			// se si: processa evento
 			switch (e.type)
 			{
 				case SDL_KEYDOWN:
 					plane.controller.EatKey(e.key.keysym.sym, keymap , true);
 					//tasti funzione: cambiano camera, rendering o fisica
 					if (e.key.keysym.sym==SDLK_F1) cameraType=(cameraType+1)%CAMERA_TYPE_MAX;
 					if (e.key.keysym.sym==SDLK_F2) useWireframe=!useWireframe;
 					if (e.key.keysym.sym==SDLK_F3) useEnvmap=!useEnvmap;
 					if (e.key.keysym.sym==SDLK_F4) useHeadlight=!useHeadlight;
 					if (e.key.keysym.sym==SDLK_F5) useShadow=!useShadow;
 					if (e.key.keysym.sym==SDLK_F6) usePhysics=!usePhysics;
 					if (e.key.keysym.sym==SDLK_ESCAPE) done=1;
 					if (e.key.keysym.sym==SDLK_RETURN) initBalloons();
 					doneSomething=true;
 					break;
 				case SDL_KEYUP:
 					plane.controller.EatKey(e.key.keysym.sym, keymap , false);
 					break;
 				case SDL_QUIT:
 					done=1;   break;
 				case SDL_WINDOWEVENT:
 					// dobbiamo ridisegnare la finestra
 					if (e.window.event==SDL_WINDOWEVENT_EXPOSED)
 						rendering(win);
 					else
 					{
 						windowID = SDL_GetWindowID(win);
 						if (e.window.windowID == windowID)
 						{
 							switch (e.window.event)
 							{
 								case SDL_WINDOWEVENT_SIZE_CHANGED:
 								{
 									scrW = e.window.data1;
 									scrH = e.window.data2;
 									glViewport(0,0,scrW,scrH);
 									rendering(win);
 									//redraw(); // richiedi ridisegno
 									break;
 								}
 							}
 						}
 					}
 					break;
 				case SDL_MOUSEMOTION:
 					if ((e.motion.state) &( SDL_BUTTON(1) )& ((cameraType)==(CAMERA_MOUSE)))
 					{
 						viewAlpha+=e.motion.xrel;
 						viewBeta +=e.motion.yrel;
 					}
 					break;
 				case SDL_MOUSEWHEEL:
 					if (e.wheel.y < 0 )
 					{
 						// avvicino il punto di vista (zoom in)
 						eyeDist=eyeDist*0.9;
 						if (eyeDist<1) eyeDist = 1;
 					};
 					if (e.wheel.y > 0 )
 					{
 						// allontano il punto di vista (zoom out)
 						eyeDist=eyeDist/0.9;
 					};
 					break;
 				case SDL_JOYAXISMOTION: /* Handle Joystick Motion */
 					if( e.jaxis.axis == 0)
 					{
 						if ( e.jaxis.value < -3200  )
 						{
 							plane.controller.Joy(4 , true);
 							plane.controller.Joy(5 , false);
 						}
 						if ( e.jaxis.value > 3200  )
 						{
 							plane.controller.Joy(4 , false);
 							plane.controller.Joy(5 , true);
 						}
 						if ( e.jaxis.value >= -3200 && e.jaxis.value <= 3200 )
 						{
 							plane.controller.Joy(4 , false);
 							plane.controller.Joy(5 , false);
 						}
 					}
 					else
 					{
 						if ( e.jaxis.value < -3200  )
 						{
 							plane.controller.Joy(6 , true);
 							plane.controller.Joy(7 , false);
 						 }
 						 if ( e.jaxis.value > 3200  )
 						 {
 						 	plane.controller.Joy(6 , false);
 						 	plane.controller.Joy(7 , true);
 						 }
 						 if ( e.jaxis.value >= -3200 && e.jaxis.value <= 3200 )
 						 {
 						 	plane.controller.Joy(6 , false);
 						 	plane.controller.Joy(7 , false);
 						 }
 					}
 					break;
 				case SDL_JOYBUTTONDOWN: /* Handle Joystick Button Presses */
 					if ( e.jbutton.button == 0 )
 					{
 						plane.controller.Joy(2 , true);
 					}
 					if ( e.jbutton.button == 2 )
 					{
 						plane.controller.Joy(3 , true);
 					}
 					if ( e.jbutton.button ==4)
 					{
 						plane.controller.Joy(0 , true);
 					}
 					if ( e.jbutton.button ==5)
 					{
 						plane.controller.Joy(1 , true);
 					}
 					break;
 				case SDL_JOYBUTTONUP: /* Handle Joystick Button Presses */

 					if ( e.jbutton.button == 0 )
 					{
 					 	plane.controller.Joy(2 , false);
 					}
 					if ( e.jbutton.button == 2 )
 					{
 					 	plane.controller.Joy(3 , false);
 					}
 					if ( e.jbutton.button ==4)
 					{
 					 	plane.controller.Joy(0 , false);
 					}
 					if ( e.jbutton.button ==5)
 					{
 					 	plane.controller.Joy(1 , false);
 					}
 					break;
 			}
 		}
 		else
 		{
 			// nessun evento: siamo IDLE
 			Uint32 timeNow=SDL_GetTicks(); // che ore sono?
 			if (timeLastInterval+fpsSampling<timeNow)
 			{
 				fps = 1000.0*((float)fpsNow) /(timeNow-timeLastInterval);
 				fpsNow=0;
 				timeLastInterval = timeNow;
 			}
 			int guardia=0; // sicurezza da loop infinito
 			// finche' il tempo simulato e' rimasto indietro rispetto
 			// al tempo reale...
 			while (nstep*PHYS_SAMPLING_STEP < timeNow )
 			{
 				//aereo fa uno step di fisica
 				plane.DoStep();
 				//controllo se l'aereo ha colpito un palloncino
 				checkBalloons();
 				if(totalpopped==NUMBALLOONS)
 				{
 					//you won!
 					hud.gameOver=true;
 				}
 				nstep++;
 				doneSomething=true;
 				timeNow=SDL_GetTicks();
 				if (guardia++>1000) {done=true; break;} // siamo troppo lenti!
 			}
 			if (doneSomething)
 				rendering(win);
 			else
 			{
 				// tempo libero: non devo disegnare nulla di nuovo.
 			}
 		}
 	}
 	SDL_GL_DeleteContext(mainContext);
 	SDL_DestroyWindow(win);
 	SDL_Quit ();
 	return (0);
}

