#ifndef _cTriangle_HG_
#define _cTriangle_HG_

#include <glm/vec3.hpp>

class cTriangle
{
public:
	cTriangle();
	~cTriangle();
	int vertex_ID_0;
	int vertex_ID_1;
	int vertex_ID_2;
};

// This will be used for the phsyics part of the thingy
class cPhysTriangle
{
public:
	glm::vec3 vertex[3];
	glm::vec3 ClosestPtPointTriangle( glm::vec3 p, glm::vec3 a,
		glm::vec3 b, glm::vec3 c );
	/*inline*/ glm::vec3 ClosestPtPointTriangle( glm::vec3 p );
};

#endif
