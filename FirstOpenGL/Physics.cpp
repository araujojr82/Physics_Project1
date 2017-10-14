#include "Physics.h"
#include "cGameObject.h"
#include "cTriangle.h"

#include <glm/glm.hpp>		// glm::distance

bool PenetrationTestSphereSphere( cGameObject* pA, cGameObject* pB, double deltaTime )
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

// inline?? 
glm::vec3 cPhysTriangle::ClosestPtPointTriangle( glm::vec3 p )
{
	return this->ClosestPtPointTriangle( p, this->vertex[0],
		this->vertex[1],
		this->vertex[2] );
}

glm::vec3 cPhysTriangle::ClosestPtPointTriangle( glm::vec3 p, glm::vec3 a,
	glm::vec3 b, glm::vec3 c )
{
	// Check if P in vertex region outside A
	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ap = p - a;
	float d1 = glm::dot( ab, ap );
	float d2 = glm::dot( ac, ap );
	if( d1 <= 0.0f && d2 <= 0.0f ) return a; // barycentric coordinates (1,0,0)

											 // Check if P in vertex region outside B
	glm::vec3 bp = p - b;
	float d3 = glm::dot( ab, bp );
	float d4 = glm::dot( ac, bp );
	if( d3 >= 0.0f && d4 <= d3 ) return b; // barycentric coordinates (0,1,0)

										   // Check if P in edge region of AB, if so return projection of P onto AB
	float vc = d1*d4 - d3*d2;
	if( vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f ) {
		float v = d1 / ( d1 - d3 );
		return a + v * ab; // barycentric coordinates (1-v,v,0)
	}

	// Check if P in vertex region outside C
	glm::vec3 cp = p - c;
	float d5 = glm::dot( ab, cp );
	float d6 = glm::dot( ac, cp );
	if( d6 >= 0.0f && d5 <= d6 ) return c; // barycentric coordinates (0,0,1)

										   // Check if P in edge region of AC, if so return projection of P onto AC
	float vb = d5*d2 - d1*d6;
	if( vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f ) {
		float w = d2 / ( d2 - d6 );
		return a + w * ac; // barycentric coordinates (1-w,0,w)
	}

	// Check if P in edge region of BC, if so return projection of P onto BC
	float va = d3*d6 - d5*d4;
	if( va <= 0.0f && ( d4 - d3 ) >= 0.0f && ( d5 - d6 ) >= 0.0f ) {
		float w = ( d4 - d3 ) / ( ( d4 - d3 ) + ( d5 - d6 ) );
		return b + w * ( c - b ); // barycentric coordinates (0,1-w,w)
	}

	// P inside face region. Compute Q through its barycentric coordinates (u,v,w)
	float denom = 1.0f / ( va + vb + vc );
	float v = vb * denom;
	float w = vc * denom;
	return a + ab * v + ac * w; // = u*a + v*b + w*c, u = va * denom = 1.0f - v - w
}