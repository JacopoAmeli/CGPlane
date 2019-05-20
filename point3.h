// classe Point3: un punto (o vettore) in 3 dimensioni
// comprende le operazioni fra punti
//utilizzata "as is" dal progettoCar4, unica variazione: è stato rimosso Point3, sono tutti Vector3 per problemi linker
#include <math.h>
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
#ifndef POINT3_H
#define POINT3_H
class Vector3
{
public: 
  
  double coord[3]; // l'UNICO campo: le cooedinate x, y, z
  
  double X() const { return coord[0]; }
  double Y() const { return coord[1]; }
  double Z() const { return coord[2]; }
  
  // costruttore 1
  Vector3( double x, double y, double z ){
    coord[0]=x;
    coord[1]=y;
    coord[2]=z;
  }

  // costruttore vuoto
  Vector3(){
    coord[0]=coord[1]=coord[2]=0;
  }
  // restituisce la versione di se stesso normalizzata
  Vector3 Normalize() const{
    return (*this)/modulo();
  }


  // restituisce il modulo
  double modulo() const{
    return 
      sqrt(coord[0]*coord[0] + coord[1]*coord[1] + coord[2]*coord[2]);
  }
  
  // operatore "/" binario: divisione per uno scalare
  Vector3 operator / (double f)const{
    return Vector3(
      coord[0]/f,   
      coord[1]/f,   
      coord[2]/f   
    );
  }
  
  Vector3 operator * (double f)const{
	  return Vector3(
		coord[0]*f,
		coord[1]*f,
		coord[2]*f
	  );
  }

  // operatore "-" unario: inversione del verso del vettore
  Vector3 operator - ()const{
    return Vector3(
      -coord[0],   
      -coord[1],   
      -coord[2]   
    );
  }

  // operatore "-" binario: differenza fra punti
  Vector3 operator - (const Vector3 &a)const{
    return Vector3(
      coord[0]-a.coord[0],   
      coord[1]-a.coord[1],   
      coord[2]-a.coord[2]   
    );
  }
    
  // somma fra vettori  
  Vector3 operator + (const Vector3 &a)const{
    return Vector3(
      coord[0]+a.coord[0],   
      coord[1]+a.coord[1],   
      coord[2]+a.coord[2]   
    );
  }
  
  
  // ridefinisco l'operatore binario "%" per fare il CROSS PRODUCT
  Vector3 operator % (const Vector3 &a)const{
    return Vector3(
      coord[1]*a.coord[2]-coord[2]*a.coord[1],   
    -(coord[0]*a.coord[2]-coord[2]*a.coord[0]),   
      coord[0]*a.coord[1]-coord[1]*a.coord[0]   
    );
  }
  
  // mandare il punto come vertice di OpenGl
  void SendAsVertex() const{
    glVertex3dv(coord);
  }
  
  // mandare il punto come normale di OpenGl
  void SendAsNormal() const{
    glNormal3dv(coord);
  }
};

inline void glTranslate(Vector3 v){
  glTranslatef(v.X(), v.Y(), v.Z());
}

#endif
