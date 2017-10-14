// This file is used to laod the models
#include "cGameObject.h"
#include <vector>
#include "Utilities.h"		// getRandInRange()
#include <glm/glm.hpp>
#include <sstream>
#include <fstream>
#include <vector>

static const std::string g_OBJECTSFILE = "objects.txt";

extern std::vector< cGameObject* >  g_vecGameObjects;
extern cGameObject* g_pTheDebugSphere;

extern float generateRandomNumber(float min, float max);

struct sGOparameters		// for the Game Objects' input file
{	
	std::string meshname;
	float x, y, z, scale;
	std::string type;
	float radius;
	std::string isUpdated;
	float red, green, blue, alpha;
	float mass;
	std::string description;
};

sGOparameters parseObjLine(std::ifstream &source);

bool LoadModelsIntoScene(std::string &error)
{
	std::stringstream ssError;
	bool bAnyErrors = false;
	int position = 0;

	std::vector <sGOparameters> allObjects(0);

	std::ifstream objectsFile(g_OBJECTSFILE);
	if (!objectsFile.is_open())
	{	// File didn't open...
		ssError << "Can't find config file >" << g_OBJECTSFILE << "<" << std::endl;
		bAnyErrors = true;
	}
	else
	{	// File DID open, so loop through the file and pushback to structure
		while (!objectsFile.eof() && objectsFile.is_open()) {
			allObjects.push_back(parseObjLine(objectsFile));
			//allObjects[position] = parseObjLine(objectsFile);
			position++;
		}
		objectsFile.close();  //Closing
	}

	for (int index = 0; index != allObjects.size(); index++)
	{
		// Create a new GO
		cGameObject* pTempGO = new cGameObject();

		pTempGO->meshName = allObjects[index].meshname; // Set the name of the mesh

		// HACK add physics mesh hard coded to the sides of the table
		if( pTempGO->meshName == "poolsides" )
		{
			pTempGO->meshName = "physics_poolsides";	//FOR NOW SO WE CAN SEE THE PHYSICS MODEL
			pTempGO->physicsMeshName = "physics_poolsides";
		}
		
		// position and scale from the file
		pTempGO->position.x = allObjects[index].x;
		pTempGO->position.y = allObjects[index].y;
		pTempGO->position.z = allObjects[index].z;
		pTempGO->scale = allObjects[index].scale;

		// Add colour
		pTempGO->diffuseColour = glm::vec4( allObjects[index].red,
											allObjects[index].green,
											allObjects[index].blue,
											allObjects[index].alpha );

		if (allObjects[index].isUpdated == "true")
		{	// Object is updated in Physics step
			pTempGO->bIsUpdatedInPhysics = true;
		}
		else
		{
			pTempGO->bIsUpdatedInPhysics = false;
		}

		// ADD mass
		pTempGO->mass = allObjects[index].mass;
		pTempGO->inverseMass -= allObjects[index].mass;

		// ADD velocity and acceleration
		if (!pTempGO->bIsLight)
		{
			// Use 0.1 as base velocity for x, y and z.
			//pTempGO->vel.x = 0.1f;
			//pTempGO->vel.y = 0.1f;
			//pTempGO->vel.z = 0.1f;
		}

		// This rotation is around it's own axis
		//pTempGO->rotation.x = generateRandomNumber( -0.05f, 0.05f );
		//pTempGO->rotation.y = generateRandomNumber( -0.05f, 0.05f );
		//pTempGO->rotation.z = generateRandomNumber( -0.05f, 0.05f );

		if (allObjects[index].type == "SPHERE")
		{
			pTempGO->typeOfObject = eTypeOfObject::SPHERE;
			pTempGO->radius = allObjects[index].radius;

			//if( index != 2 ){
			//	pTempGO->vel.x = generateRandomNumber( -6.0f, 6.0f );
			//	pTempGO->vel.z = generateRandomNumber( -6.0f, 6.0f );
			//	pTempGO->vel.y = 0.0f;
			//}
			
		}
		else if (allObjects[index].type == "PLANE") {
			pTempGO->typeOfObject = eTypeOfObject::PLANE;
		}

		::g_vecGameObjects.push_back(pTempGO);
	}
	//// Add the debug sphere
	//{// STARTOF: Add the debug sphere
	//	::g_pTheDebugSphere = new cGameObject();
	//	::g_pTheDebugSphere->scale = 1.0f;
	//	::g_pTheDebugSphere->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	//	::g_pTheDebugSphere->meshName = "SphereRadius1";
	//	::g_pTheDebugSphere->typeOfObject = eTypeOfObject::SPHERE;
	//	::g_pTheDebugSphere->radius = 1.0f;
	//	::g_pTheDebugSphere->bIsUpdatedInPhysics = false;
	//	// NOTE: I'm NOT adding it to the vector of objects
	//	//::g_vecGameObjects.push_back( pTempGO );		// Fastest way to add
	//}// ENDOF: Add the debug sphere

	//cGameObject* pTempGO = new cGameObject();
	//pTempGO->position = glm::vec3(0.0f, 0.26f, 0.0f);
	//pTempGO->scale = 1.0f;
	//pTempGO->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	//pTempGO->meshName = "ball";
	//pTempGO->typeOfObject = eTypeOfObject::SPHERE;
	//pTempGO->radius = 0.26f;
	//pTempGO->bIsUpdatedInPhysics = false;
	//::g_vecGameObjects.push_back(pTempGO);

	// Add the cue stick, independent of the loaded objects file.
	cGameObject* pTempGO = new cGameObject();
	pTempGO->scale = 1.0f;
	pTempGO->position = glm::vec3( 0.0f, 0.26f, 0.0f );
	pTempGO->diffuseColour = glm::vec4(7.0f, 7.0f, 0.0f, 1.0f);
	pTempGO->meshName = "cue";
	pTempGO->typeOfObject = eTypeOfObject::CUE;
	pTempGO->bIsUpdatedInPhysics = false;
	::g_vecGameObjects.push_back(pTempGO);

	return bAnyErrors;
}

// Parse the file line to fit into the structure
sGOparameters parseObjLine(std::ifstream &source) {

	sGOparameters sGOpar;

	source >> sGOpar.meshname
		>> sGOpar.x
		>> sGOpar.y
		>> sGOpar.z
		>> sGOpar.scale
		>> sGOpar.type
		>> sGOpar.radius
		>> sGOpar.isUpdated
		>> sGOpar.red
		>> sGOpar.green
		>> sGOpar.blue
		>> sGOpar.alpha
		>> sGOpar.mass
		>> sGOpar.description;

	return sGOpar;
}