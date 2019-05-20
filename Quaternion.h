#ifndef QUATERNION_H
#define QUATERNION_H
#include "point3.h"
//Quaternioni, utilizzati per orientamento/rotazione corretti nell'aereo
class Quaternion
{
public:
	 Quaternion();	
	 Quaternion(double x1, double y1, double z1, double w1);
	 Quaternion(const Quaternion& p);	
	 void normalise();	 
     float * getMatrix();
     Quaternion operator* (const Quaternion &rq) const;
     Quaternion getConjugate() const;
	 void fromAxis(const Vector3 &v1, double angle);
	 Vector3 operator* (const Vector3 &vec) const;
	 double x;
	 double y;
	 double z;
	 double w;

	private:
};
#endif
