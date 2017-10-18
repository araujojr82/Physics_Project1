#ifndef _Physics_HG_
#define _Physics_HG_

#include <glm/vec3.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

//#include "cGameObject.h"
class cGameObject;


// Sphere-Sphere
// Sphere-Plane
// Sphere-Triangle
// Triangle-Triangle
// Mesh-Mesh
// Sphere-Mesh

// Our objects are vectors of pointers, so we might as well pass pointers
bool PenetrationTestSphereSphere( cGameObject* pA, cGameObject* pB, double deltaTime );

void bounceSpheres( cGameObject* pA, cGameObject* pB );

void bounceSphereAgainstPlane( cGameObject* pA, cGameObject* pB, glm::vec3 tNormal );

//bool AlmostEqualRelativeAndAbs( float A, float B );

glm::vec3 calculateFriction( float angle, float friction );

float calculateMovAngle( glm::vec3 vel );

glm::vec3 calculateXZVelocity( int angle, float force );

#endif