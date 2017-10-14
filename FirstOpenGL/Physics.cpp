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
	float deltaMass, deltaVelX_2, a,
		deltaPosX, deltaPosZ, 
		deltaVelX, deltaVelZ, 
		velX_cm, velZ_cm;

	deltaMass = pB->mass / pA->mass;
	deltaPosX = pB->position.x - pA->position.x;
	deltaPosZ = pB->position.z - pA->position.z;
	deltaVelX = pB->vel.x - pA->vel.x;
	deltaVelZ = pB->vel.z - pA->vel.z;

	velX_cm = ( pA->mass * pA->vel.x + pB->mass * pB->vel.x ) / ( pA->mass + pB->mass );
	velZ_cm = ( pA->mass * pA->vel.z + pB->mass * pB->vel.z ) / ( pA->mass + pB->mass );

	// Check division by zero
	if( deltaPosX == 0.0f )     a = deltaPosZ;
	else						a = deltaPosZ / deltaPosX;

	deltaVelX_2 = -2 * ( deltaVelX + a * deltaVelZ ) / ( ( 1 + a * a ) * ( 1 + deltaMass ) );
	
	// Update the velocities of X and Z for pA and pB
	pA->vel.x -= deltaMass * deltaVelX_2;
	pA->vel.z -= a  * deltaMass * deltaVelX_2;

	pB->vel.x += deltaVelX_2;
	pB->vel.z += a * deltaVelX_2;

	return;
}