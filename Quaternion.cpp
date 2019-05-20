/*
Derivata da:
http://gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation
*/
#include "Quaternion.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
//matrice di trasformazione da passare ad opengl, serve riferimento esterno
float matrixMul[16];
//costruttore default
Quaternion::Quaternion(void)
{
	x = y = z = w = 0.0;
}
//costruttore
Quaternion::Quaternion(double x1, double y1, double z1, double w1)
{
	x = x1;
	y = y1;
	z = z1;
	w = w1;
}
//ritorna la matrice di trasformazione, può essere passata direttamente allo stack
float * Quaternion::getMatrix()
{
	matrixMul[0]=(float)(w*w + x*x - y*y - z*z);matrixMul[1]=(float)(2*x*y + 2*w*z);matrixMul[2]=(float)(2*x*z - 2*w*y);
	matrixMul[3]=0;
	matrixMul[4]=(float)(2*x*y - 2*w*z);matrixMul[5]=(float)(w*w - x*x + y*y - z*z);matrixMul[6]=(float)(2*y*z + 2*w*x);
	matrixMul[7]=0;
	matrixMul[8]=(float)(2*x*z + 2*w*y);matrixMul[9]=(float)(2*y*z - 2*w*x);matrixMul[10]=(float)(w*w - x*x - y*y + z*z);
	matrixMul[11]=0;
	matrixMul[12]=0;matrixMul[13]=0;matrixMul[14]=0;matrixMul[15]=(float)(w*w + x*x + y*y + z*z);
	return matrixMul;
}
//imposta il quaternione a partire da un asse e un angolo
void Quaternion::fromAxis(const Vector3 &v1, double angle)
{
	double sinAngle;
	Vector3 vn = Vector3(v1.X(),v1.Y(),v1.Z());
	sinAngle = sin(angle * .5);	//seno di angolo/2
	x = (vn.X() * sinAngle);  //imposta i componenti del quaternione
	y = (vn.Y() * sinAngle);
	z = (vn.Z() * sinAngle);
	w = cos(angle);	
	
	//Questo è per essere sicuri di avere modulo 1
	double q = x *x + y*y + z*z + w*w;  
	double xtra = sqrt((double)1.000000000/q);
	x *= xtra;
	y *= xtra;
	z *= xtra;
	w  *= xtra;
	q = x *x + y*y + z*z + w*w;
	xtra = sqrt((double)1.000000000/q);
	x *= xtra;
	y *= xtra;
	z *= xtra;
	w  *= xtra;
}
//ritorna il quaternione coniugato
Quaternion Quaternion::getConjugate() const
{
	return Quaternion(-x, -y, -z, w);
}

//normalizza il quaternione, per avere modulo 1
void Quaternion::normalise()
{	
	double mag2 = w * w + x * x + y * y + z * z;
	if (  mag2!=0.f && (fabs(mag2 - 1.0f) > .000001f)) {
		double mag = sqrt(mag2);
		w /= mag;
		x /= mag;
		y /= mag;
		z /= mag;
	}		
}
//costruttore "copia"
Quaternion::Quaternion(const Quaternion& p)
{
	x = p.x;
	y = p.y;
	z = p.z;
	w = p.w;
}
//overload operatore moltiplicazione: combina le due trasformazioni
Quaternion Quaternion::operator* (const Quaternion &rq) const
{
	return Quaternion(w * rq.x + x * rq.w + y * rq.z - z * rq.y,
	                  w * rq.y + y * rq.w + z * rq.x - x * rq.z,
	                  w * rq.z + z * rq.w + x * rq.y - y * rq.x,
	                  w * rq.w - x * rq.x - y * rq.y - z * rq.z);
}
//overload operatore * su vettore: trasforma il vettore
Vector3 Quaternion::operator* (const Vector3 &vec) const
{
	Vector3 vn = Vector3(vec);
	Quaternion vecQuat(vn.X(), vn.Y(), vn.Z(), 0);
	Quaternion resQuat;	 
	resQuat = vecQuat * getConjugate();
	resQuat = *this * resQuat;
	Vector3 toreturn(resQuat.x, resQuat.y, resQuat.z);
	return toreturn;
}
