#ifndef _Physics_HG_
#define _Physics_HG_

#include <glm/vec3.hpp>

//#include "cGameObject.h"
class cGameObject;


// Sphere-Sphere
// Sphere-Plane
// Sphere-Triangle
// Triangle-Triangle
// Mesh-Mesh
// Sphere-Mesh

// Our objects are vectors of pointers, so we might as well pass pointers
bool PenetrationTestSphereSphere( cGameObject* pA, cGameObject* pB, double collisionTime, double deltaTime );

void bounceSpheres( cGameObject* pA, cGameObject* pB );

glm::vec3 scale( const glm::vec3& v, glm::vec3 a );

glm::vec3 projectUonV( const glm::vec3& u, const glm::vec3& v );


#endif