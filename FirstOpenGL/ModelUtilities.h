#ifndef _ModelUtilities_HG_
#define _ModelUtilities_HG_

#include <fstream>
#include <string>
#include "cMesh.h"

struct sMeshparameters		// for the Meshes' input file
{
	std::string meshname;
	std::string meshFilename;
};

// Only "function signature"
void ReadFileToToken( std::ifstream &file, std::string token );

// Note the & symbol which is "by reference" which means
//	it's NOT a copy, but referring to the original object.
// ALSO note that this is a C++ thing (& isn't a thing in C)
bool LoadPlyFileIntoMesh( std::string filename, cMesh &theMesh );

sMeshparameters parseMeshLine(std::ifstream &source);

#endif
