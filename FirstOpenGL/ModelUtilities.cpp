#include "ModelUtilities.h" 
#include "cVAOMeshManager.h"
#include "cMesh.h"
#include <sstream>
#include <vector>

extern cMesh g_MeshPoolTable;

static const std::string g_MESHLISTFILE = "meshlist.txt";

void ReadFileToToken( std::ifstream &file, std::string token )
{
	bool bKeepReading = true;
	std::string garbage;
	do
	{
		file >> garbage;		// Title_End??
		if ( garbage == token )
		{
			return;
		}
	}while( bKeepReading );
	return;
}



// Takes a file name, loads a mesh
bool LoadPlyFileIntoMesh( std::string filename, cMesh &theMesh )
{
	// Load the vertices
	// c_str() changes a string to a "c style char* string"
	std::ifstream plyFile( filename.c_str() );

	if ( ! plyFile.is_open() )
	{	// Didn't open file, so return
		return false;
	}
	// File is open, let's read it

	ReadFileToToken( plyFile, "vertex" );
//	int numVertices = 0;
	plyFile >> theMesh.numberOfVertices;

	ReadFileToToken( plyFile, "face" );
//	int numTriangles = 0;
	plyFile >> theMesh.numberOfTriangles;		

	ReadFileToToken( plyFile, "end_header" );

	// Allocate the appropriate sized array (+a little bit)
	theMesh.pVertices = new cVertex_xyz_rgb[theMesh.numberOfVertices];
	theMesh.pTriangles = new cTriangle[theMesh.numberOfTriangles];

	// Read vertices
	for ( int index = 0; index < theMesh.numberOfVertices; index++ )
	{
		//end_header
		//-0.0312216 0.126304 0.00514924 0.850855 0.5 		
		float x, y, z, confidence, intensity;

		plyFile >> x;
		plyFile >> y;
		plyFile >> z;
//		plyFile >> confidence;
//		plyFile >> intensity;

		theMesh.pVertices[index].x = x;	// vertices[index].x = x;
		theMesh.pVertices[index].y = y;	// vertices[index].y = y;
		theMesh.pVertices[index].z = z; 
		theMesh.pVertices[index].r = 1.0f;	// vertices[index].g = 1.0f;
		theMesh.pVertices[index].g = 1.0f;	// vertices[index].b = 1.0f;
		theMesh.pVertices[index].b = 1.0f;	// vertices[index].r = 1.0f;
	}

	// Load the triangle (or face) information, too
	for ( int count = 0; count < theMesh.numberOfTriangles; count++ )
	{
		// 3 164 94 98 
		int discard = 0;
		plyFile >> discard;									// 3
		plyFile >> theMesh.pTriangles[count].vertex_ID_0;	// 164
		plyFile >> theMesh.pTriangles[count].vertex_ID_1;	// 94
		plyFile >> theMesh.pTriangles[count].vertex_ID_2;	// 98
	}

	theMesh.CalculateNormals();

	return true;
}

// Takes a file name, loads a mesh
bool LoadPlyFileIntoMeshWithNormals(std::string filename, cMesh &theMesh)
{
	// Load the vertices
	// c_str() changes a string to a "c style char* string"
	std::ifstream plyFile(filename.c_str());

	if (!plyFile.is_open())
	{	// Didn't open file, so return
		return false;
	}
	// File is open, let's read it

	ReadFileToToken(plyFile, "vertex");
	//	int numVertices = 0;
	plyFile >> theMesh.numberOfVertices;

	ReadFileToToken(plyFile, "face");
	//	int numTriangles = 0;
	plyFile >> theMesh.numberOfTriangles;

	ReadFileToToken(plyFile, "end_header");

	// Allocate the appropriate sized array (+a little bit)
	theMesh.pVertices = new cVertex_xyz_rgb[theMesh.numberOfVertices];
	theMesh.pTriangles = new cTriangle[theMesh.numberOfTriangles];

	// Read vertices
	for (int index = 0; index < theMesh.numberOfVertices; index++)
	{
		float x, y, z, nx, ny, nz, confidence, intensity;

		plyFile >> x;
		plyFile >> y;
		plyFile >> z;
		plyFile >> nx >> ny >> nz;
		//		plyFile >> confidence;
		//		plyFile >> intensity;

		theMesh.pVertices[index].x = x;
		theMesh.pVertices[index].y = y;
		theMesh.pVertices[index].z = z;
		theMesh.pVertices[index].r = 1.0f;
		theMesh.pVertices[index].g = 1.0f;
		theMesh.pVertices[index].b = 1.0f;
		theMesh.pVertices[index].nx = nx;
		theMesh.pVertices[index].ny = ny;
		theMesh.pVertices[index].nz = nz;
	}

	// Load the triangle (or face) information, too
	for (int count = 0; count < theMesh.numberOfTriangles; count++)
	{
		// 3 164 94 98 
		int discard = 0;
		plyFile >> discard;								
		plyFile >> theMesh.pTriangles[count].vertex_ID_0;
		plyFile >> theMesh.pTriangles[count].vertex_ID_1;
		plyFile >> theMesh.pTriangles[count].vertex_ID_2;
	}

	//	theMesh.CalculateNormals();

	return true;
}

bool Load3DModelsIntoMeshManager(int shaderID, cVAOMeshManager* pVAOManager, std::string &error)
{
	std::stringstream ssError;
	bool bAnyErrors = false;
		
	std::vector <sMeshparameters> allMeshes;

	std::ifstream objectsFile( g_MESHLISTFILE );
	if (!objectsFile.is_open())
	{	// File didn't open...
		ssError << "Can't find mesh list file >" << g_MESHLISTFILE << "<" << std::endl;
		bAnyErrors = true;
	}
	else
	{	// File DID open, so loop through the file and pushback to structure
		while (!objectsFile.eof() && objectsFile.is_open()) {
			allMeshes.push_back(parseMeshLine(objectsFile));
		}
		objectsFile.close();  //Closing
	}

	for (int index = 0; index != allMeshes.size(); index++)
	{	
		if( allMeshes[index].meshname != "poolsides" )
		{
			cMesh testMesh;
			testMesh.name = allMeshes[index].meshname;
			if( !LoadPlyFileIntoMeshWithNormals( allMeshes[index].meshFilename, testMesh ) )
			{
				ssError << "Didn't load model >" << testMesh.name << "<" << std::endl;
				bAnyErrors = true;

			}
			if( !pVAOManager->loadMeshIntoVAO( testMesh, shaderID ) )
			{
				ssError << "Could not load mesh >" << testMesh.name << "< into VAO" << std::endl;
				bAnyErrors = true;
			}
		}
		else
		{ // This is the pooltable mesh for physics
			::g_MeshPoolTable.name = allMeshes[index].meshname;
			if( !LoadPlyFileIntoMeshWithNormals( allMeshes[index].meshFilename, g_MeshPoolTable ) )
			{
				ssError << "Didn't load model >" << g_MeshPoolTable.name << "<" << std::endl;
				bAnyErrors = true;
			}
			if( !pVAOManager->loadMeshIntoVAO( g_MeshPoolTable, shaderID ) )
			{
				ssError << "Could not load mesh >" << g_MeshPoolTable.name << "< into VAO" << std::endl;
				bAnyErrors = true;
			}

			// For physics, generate the triangles we need for physics
			::g_MeshPoolTable.GeneratePhysicsTriangles();
		}
	}

	if (!bAnyErrors)
	{
		// Copy the error string stream into the error string that
		//	gets "returned" (through pass by reference)
		error = ssError.str();
	}

	return bAnyErrors;
}

// Parse the file line to fit into the structure
sMeshparameters parseMeshLine(std::ifstream &source) {

	sMeshparameters sMeshpar;

	//Scanning a line from the file
	source >> sMeshpar.meshname >> sMeshpar.meshFilename;

	return sMeshpar;
}