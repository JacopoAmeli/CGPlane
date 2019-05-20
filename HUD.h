/*
 * HUD.h
 *
 *  Created on: 21 mag 2018
 *      Author: Jacopo Ameli
 */

#ifndef HUD_H_
#define HUD_H_
//disegna mappa e foto personale
class HUD {
	void drawPersonalPhoto();
	void drawMap();
public:
	void Init();
	void Render();
	void drawGameOver();
	void drawPoints();
	float xMapStart,mapSize;
	bool gameOver;
	float xPhotoStart,personalPhotoSize;
};

#endif /* HUD_H_ */
