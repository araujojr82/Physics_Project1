#include "Physics.h"
#include "cGameObject.h"

#include <glm/glm.hpp>		// glm::distance

bool PenetrationTestSphereSphere( cGameObject* pA, cGameObject* pB, double timeOfColision, double deltaTime )
{
	// If the distance between the two sphere is LT the sum or the radii,
	//	they are touching or penetrating

	float totalRadii = pA->radius + pB->radius;

	// The Pythagorean distance 
	float distance = glm::distance( pA->position, pB->position );

	if( distance <= totalRadii )
	{
		return true;
	}

	return false;

}

void bounceSpheres( cGameObject* pA, cGameObject* pB )
{
	glm::vec3 nv1; // new velocity for sphere 1
	glm::vec3 nv2; // new velocity for sphere 2
				// this can probably be optimised a bit, but it basically swaps the velocity amounts
				// that are perpendicular to the surface of the collistion.
				// If the spheres had different masses, then u would need to scale the amounts of
				// velocities exchanged inversely proportional to their masses.

	nv1 = pA->vel;
	nv1 += projectUonV( pB->vel, ( pB->position - pA->position ) );
	nv1 -= projectUonV( pA->vel, ( pA->position - pB->position ) );

	nv2 = pB->vel;
	nv2 += projectUonV( pA->vel, ( pB->position - pA->position ) );
	nv2 -= projectUonV( pB->vel, ( pA->position - pB->position ) );
	
	pA->vel = nv1;
	pB->vel = nv2;

	return;
}

glm::vec3 scale( const glm::vec3& v, glm::vec3 a ) {
	glm::vec3 r;
	r.x = v.x * a.x;
	//r.y = v.y * a.y;
	r.y = 1.0f;
	r.z = v.z * a.z;
	return r;
}

glm::vec3 projectUonV( const glm::vec3& u, const glm::vec3& v ) {
	glm::vec3 r;
	r = scale( v, ( u * v ) / ( v * v ) );
	return r;
}
