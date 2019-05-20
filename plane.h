/*
 * plane.h
 *
 *  Created on: 01 mag 2018
 *      Author: Jacopo Ameli
 */
#include "Quaternion.h"

class Controller{
public:
  enum { LEFT=0, RIGHT=1, ACC=2, DEC=3, LEFTROLL=4, RIGHTROLL=5, PITCHUP=6, PITCHDOWN=7,
	  NKEYS=8};
  bool key[NKEYS];
  void Init();
  void EatKey(int keycode, int* keymap, bool pressed_or_released);
  void Joy(int keymap, bool pressed_or_released);
  Controller(){Init();} // costruttore
};

//la classe più importante, l'aereo stesso
class Plane{

  void RenderAllParts(bool usecolor) const;
  void drawTimone(bool usecolor) const;
  void drawElica(bool usecolor) const;
  void drawFlaps(bool usecolor) const;
  void drawTail(bool usecolor) const;
  void drawWheels(bool usecolor) const;
                         // disegnano le parti dell'aereo

public:
  // Metodi
  void Init(); // inizializza variabili
  void Render(); // disegna a schermo
  void RenderShadow(); //disegna ombra
  void DoStep(); // computa un passo del motore fisico
  Plane(){Init();} // costruttore

  Controller controller;

  // STATO DELL'AEREO
  // (DoStep fa evolvere queste variabili nel tempo)
  float flapL,timone,tail; // stato interno rotazione componenti
  //aggiunte quaternioni
  Vector3 normal;
  Vector3 forwardVector;
  Vector3 barrel;
  Quaternion transform;
  Quaternion axis1;
  Quaternion axis2;
  Quaternion axis3;
  double planePos[3];
  double speed;
  float accCaduta;
  float speedCaduta;
  float mozzoE,velTimone, velRitornoTimone,velFlap,velRitornoFlap, accMax, attritoAria,
        raggioElica,timoneRate,flapRate,threshold;
};
