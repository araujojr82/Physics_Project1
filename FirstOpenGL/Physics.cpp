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

bool AlmostEqualRelativeAndAbs( float A, float B ) {
	
	float maxDiff = 0.005f;
	float maxRelDiff = FLT_EPSILON;

	// Check if the numbers are really close -- needed when comparing numbers near zero.
	float diff = fabs( A - B );
	if( diff <= maxDiff ) {
		return true;
	}

	A = fabs( A );
	B = fabs( B );
	float largest = ( B > A ) ? B : A;

	if( diff <= largest * maxRelDiff ) {
		return true;
	}
	return false;
}


//void bounceSpheres( cGameObject* pA, cGameObject* pB )
//void ApplyImpulse( RigidbodyVolume& A, RigidbodyVolume& B, const CollisionManifold& M, int c ) {
void bounceSphereAgainstPlane( cGameObject* pA, cGameObject* pB, glm::vec3 tNormal )
{
	// Linear impulse
	float invMassSum = pA->inverseMass + pB->inverseMass;

	//if( invMassSum == 0.0f ) {
	//	return; // Both objects have infinate mass!
	//}

	//glm::vec3 r1 = M.contacts[c] - A.position;
	//glm::vec3 r2 = M.contacts[c] - B.position;
	//glm::mat4 i1 = A.InvTensor();
	//glm::mat4 i2 = B.InvTensor();

	// Relative velocity
	//glm::vec3 relativeVel = ( B.velocity + glm::cross( B.angVel, r2 ) ) - ( A.velocity + glm::cross( A.angVel, r1 ) );
	glm::vec3 relativeVel = pB->vel - pA->vel;

	// Relative collision normal
	//glm::vec3 relativeNorm = glm::vec3( 0.0f, 0.0f, 1.0f ); // = M.normal;

	if( tNormal.x > 1.0f ) tNormal.x = -1.0f;
	if( tNormal.y > 1.0f ) tNormal.y = -1.0f;
	if( tNormal.z > 1.0f ) tNormal.z = -1.0f;
	
	if( tNormal.x < -1.0f ) tNormal.x = 1.0f;
	if( tNormal.y < -1.0f ) tNormal.y = 1.0f;
	if( tNormal.z < -1.0f ) tNormal.z = 1.0f;

	glm::normalize( tNormal );

	// Moving away from each other? Do nothing!
	if( glm::dot( relativeVel, tNormal ) > 0.0f ) {
		return;
	}

	//float e = fminf( A.cor, B.cor );
	float e = fminf( 1.0f , 1.0f ); // Perfect ellastic collision have COR = 1.0

	float numerator = ( -( 1.0f + e ) * glm::dot( relativeVel, tNormal ) );
	float d1 = invMassSum;

	//glm::vec3 d2 = Cross( MultiplyVector( glm::cross( r1, relativeNorm ), i1 ), r1 );
	//glm::vec3 d3 = Cross( MultiplyVector( Cross( r2, relativeNorm ), i2 ), r2 );
	//float denominator = d1 + Dot( relativeNorm, d2 + d3 );
	float denominator = d1;

	float j = ( denominator == 0.0f ) ? 0.0f : numerator / denominator;
	//if( M.contacts.size() > 0.0f && j != 0.0f ) {
	//	j /= ( float ) M.contacts.size();
	//}

	glm::vec3 impulse = tNormal * j;
	pA->vel = pA->vel - impulse *  pA->inverseMass;
	pB->vel = pB->vel + impulse *  pB->inverseMass;
	
	// HACK to stop Y velocity
	pA->vel.y = 0.0f;
	pB->vel.y = 0.0f;

	//A.angVel = A.angVel - MultiplyVector( Cross( r1, impulse ), i1 );
	//B.angVel = B.angVel + MultiplyVector( Cross( r2, impulse ), i2 );

	// Friction
	glm::vec3 t = relativeVel - ( tNormal * glm::dot( relativeVel, tNormal ) );
	if( AlmostEqualRelativeAndAbs( glm::dot( t, t ), 0.0f ) ) {
		
		return;
	}
	glm::normalize( t );

	numerator = - glm::dot( relativeVel, t );
	d1 = invMassSum;
	
	//d2 = Cross( MultiplyVector( Cross( r1, t ), i1 ), r1 );
	//d3 = Cross( MultiplyVector( Cross( r2, t ), i2 ), r2 );
	//denominator = d1 + Dot( t, d2 + d3 );
	denominator = d1;

	float jt = ( denominator == 0.0f ) ? 0.0f : numerator / denominator;
	//if( M.contacts.size() > 0.0f && jt != 0.0f ) {
	//	jt /= ( float ) M.contacts.size();
	//}

	if( AlmostEqualRelativeAndAbs( jt, 0.0f ) ) {
		return;
	}

	glm::vec3 tangentImpuse;
//#ifdef DYNAMIC_FRICTION
//	float sf = sqrtf( A.staticFriction * B.staticFriction );
//	float df = sqrtf( A.dynamicFriction * B.dynamicFriction );
//	if( fabsf( jt ) < j * sf ) {
//		tangentImpuse = t * jt;
//	}
//	else {
//		tangentImpuse = t * -j * df;
//	}
//#else
//	float friction = sqrtf( A.friction * B.friction );
//	if( jt > j * friction ) {
//		jt = j * friction;
//	}
//	else if( jt < -j * friction ) {
//		jt = -j * friction;
//	}
//	tangentImpuse = t * jt;
//#endif


	pA->vel = pA->vel - tangentImpuse *  pA->inverseMass;	
	// THIS SHOULD BE ZERO FOR STATIC OBJECTS (INFINITY MASS):
	pB->vel = pB->vel + tangentImpuse *  pB->inverseMass;

	// HACK Y velocity must be 0 for now
	pA->vel.y = 0.0f;
	pB->vel.y = 0.0f;

	//A.angVel = A.angVel - MultiplyVector( Cross( r1, tangentImpuse ), i1 );
	//B.angVel = B.angVel + MultiplyVector( Cross( r2, tangentImpuse ), i2 );
}