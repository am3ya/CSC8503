/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Plane.h"

using namespace NCL;
using namespace NCL::Maths;

Plane::Plane(void) {
	normal		= Vector3(0, 1, 0);
	distance	= 0.0f;
};

Plane::Plane(const Vector3 &normal, float distance, bool normalise) {
	if(normalise) {
		float length = Vector::Length(normal);

		this->normal = Vector::Normalise(normal);
		this->distance = distance / length;
	}
	else{
		this->normal = normal;
		this->distance = distance;
	}
}

bool Plane::SphereInPlane(const Vector3 &position, float radius) const {
	if (Vector::Dot(position, normal) + distance <= -radius) {
		return false;
	}
	return true;	
}

bool Plane::PointInPlane(const Vector3 &position) const {
	if (Vector::Dot(position, normal) + distance < -0.00001f) {
		return false;
	}

	return true;
}

Plane Plane::PlaneFromTri(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2) {
	Vector3 v1v0 = v1 - v0;
	Vector3 v2v0 = v2 - v0;

	Vector3 normal = Vector::Cross(v1v0, v2v0);
	normal = Vector::Normalise(normal);

	float d = -Vector::Dot(v0, normal);
	return Plane(normal, d, false);
}

float	Plane::DistanceFromPlane(const Vector3 &in) const{
	return Vector::Dot(in,normal)+distance;
}

Vector3 Plane::ProjectPointOntoPlane(const Vector3 &point) const {
	float distance = DistanceFromPlane(point);

	return point - (normal * distance);
}