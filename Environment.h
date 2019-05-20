/*
 * Environment.h
 *
 *  Created on: 21 mag 2018
 *      Author:Jacopo Ameli
 */
//Disegna vari elementi dell'ambiente
#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

class Environment {

	void drawBench();
	void drawLamp();
	void drawLights();
	void drawRunway();
	void drawGrass();
	void drawSkyCube();
	void drawBin();

	public:
  // Metodi
  void Init(); // inizializza variabili
  void Render(); // disegna a schermo
  float benchPosx,benchPosy,benchPosz;
  float posxLamp,posyLamp,poszLamp;
  float posxBin,posyBin,poszBin;
  double runwaySize;
  int runwayRepeat;
  float runwayPosx,runwayPosy,runwayPosz,runwayHeight;
  double terrainSize,subdivision,quadSize,terrainHeight;

};

#endif
