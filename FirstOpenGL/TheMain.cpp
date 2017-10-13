#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>						// C++ cin, cout, etc.
#include <glm/vec3.hpp>					// glm::vec3
#include <glm/vec4.hpp>					// glm::vec4
#include <glm/mat4x4.hpp>				// glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>			// glm::value_ptr


#include <stdlib.h>
#include <stdio.h>
// Add the file stuff library (file stream>
#include <fstream>
#include <sstream>						// "String stream"
#include <istream>
#include <string>
#include <vector>						// smart array, "array" in most languages
#include <random>
#include <chrono>

#include "Utilities.h"
#include "ModelUtilities.h"
#include "cMesh.h"
#include "cShaderManager.h" 
#include "cGameObject.h"
#include "cVAOMeshManager.h"

#include "Physics.h"

#include "cLightManager.h"

// Euclides: Control selected object for movement
int g_GameObjNumber = 0;				// game object vector position number 
int g_LightObjNumber = 0;				// light object vector position


glm::vec3 CAMERASPEED = glm::vec3( 0.0f, 0.0f, 0.0f );
static const int g_NUMBER_OF_LIGHTS = 5;

bool bIsWireframe = false;

// Remember to #include <vector>...
std::vector< cGameObject* > g_vecGameObjects;


glm::vec3 g_cameraXYZ = glm::vec3( 0.0f, 40.0f, 0.1f ); // Default top pool view
//glm::vec3 g_cameraXYZ = glm::vec3(0.0f, 30.0f, 30.0f); // Side "horizontal"  view
glm::vec3 g_cameraTarget_XYZ = glm::vec3( 0.0f, 0.0f, 0.0f );

// TODO include camera new code

cVAOMeshManager*	g_pVAOManager = 0;		// or NULL or nullptr

cShaderManager*		g_pShaderManager;		// Heap, new (and delete)
cLightManager*		g_pLightManager;

struct sWindowConfig
{
public:
	int height = 480;
	int width = 640;
	std::string title = "Graphics 101 is Awesome!";
};

// Forward declare the Functions
void loadConfigFile( std::string fileName, sWindowConfig& wConfig );
void loadLightObjects();

void PhysicsStep( double deltaTime );

bool LoadModelsIntoScene(std::string &error);
bool Load3DModelsIntoMeshManager(int shaderID, cVAOMeshManager* pVAOManager, std::string &error);

//struct sGOparameters		// for the Game Objects' input file
//{
//	std::string meshname;
//	int nObjects;
//	float x, y, z, scale;
//	std::string random;
//	float rangeX, rangeY, rangeZ, rangeScale;
//};
//
//sGOparameters parseObjLine(std::ifstream &source);
//void loadObjectsFile(std::string fileName);

static void error_callback( int error, const char* description )
{
	fprintf( stderr, "Error: %s\n", description );
}

// All the keyboard input logic is here
static void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods )
{	
	if( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
		glfwSetWindowShouldClose( window, GLFW_TRUE );

	// Change object in g_GameObject
	if ( key == GLFW_KEY_ENTER && action == GLFW_PRESS )
	{
		if ( bIsWireframe ) bIsWireframe = false;
		else bIsWireframe = true;
	}

	//// Change object in g_GameObject
	//if( key == GLFW_KEY_SPACE && action == GLFW_PRESS )
	//{
	//	if( g_GameObjNumber < ( ::g_vecGameObjects.size() - 1 ) ) {
	//		g_GameObjNumber++;
	//	}
	//	else
	//	{
	//		g_GameObjNumber = 0;
	//	}
	//}

	// Change light colour
	if( key == GLFW_KEY_C && action == GLFW_PRESS )
	{
		::g_pLightManager->vecLights[g_LightObjNumber].diffuse.r = getRandInRange<float>( 0.0f, 1.0f );
		::g_pLightManager->vecLights[g_LightObjNumber].diffuse.g = getRandInRange<float>( 0.0f, 1.0f );
		::g_pLightManager->vecLights[g_LightObjNumber].diffuse.b = getRandInRange<float>( 0.0f, 1.0f );
	}
	
	// Change light intensity
	switch ( key )
	{
	case GLFW_KEY_COMMA:		// , (< key)
		::g_pLightManager->vecLights[g_LightObjNumber].attenuation.y *= 0.99f;	// less 1%
		std::cout << "Linear attenuation of light " << g_LightObjNumber << " is " <<
			::g_pLightManager->vecLights[g_LightObjNumber].attenuation.y << std::endl;
		break;
	case GLFW_KEY_PERIOD:		// . (> key)
		::g_pLightManager->vecLights[g_LightObjNumber].attenuation.y *= 1.01f; // more 1%
		std::cout << "Linear attenuation of light " << g_LightObjNumber << " is " <<
			::g_pLightManager->vecLights[g_LightObjNumber].attenuation.y << std::endl;
		break;
	}
	
	// Turn Light on/off
	if ( key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS )
	{	
		if ( ::g_pLightManager->vecLights[g_LightObjNumber].diffuse != glm::vec3( 0.0f, 0.0f, 0.0f ) )
		{	// Turn light ON (Black)
			::g_pLightManager->vecLights[g_LightObjNumber].diffuse = glm::vec3( 0.0f, 0.0f, 0.0f );
		}
		else
		{	// Turn light ON (White)
			::g_pLightManager->vecLights[g_LightObjNumber].diffuse = glm::vec3( 1.0f, 1.0f, 1.0f );
		}
	}

	// Change Camera Velocity
	switch( key )
	{
	case GLFW_KEY_UP:		// Up arrow
		//::g_vecGameObjects[g_GameObjNumber]->position.y += 0.10f;
		::g_pLightManager->vecLights[g_LightObjNumber].position.y += 0.10f;
		break;
	case GLFW_KEY_DOWN:		// Down arrow
		//::g_vecGameObjects[g_GameObjNumber]->position.y -= 0.10f;
		::g_pLightManager->vecLights[g_LightObjNumber].position.y -= 0.10f;
		break;
	case GLFW_KEY_LEFT:		// Left arrow
		//::g_vecGameObjects[g_GameObjNumber]->position.x -= 0.10f;
		::g_pLightManager->vecLights[g_LightObjNumber].position.x -= 0.10f;
		break;
	case GLFW_KEY_RIGHT:	// Right arrow
		//::g_vecGameObjects[g_GameObjNumber]->position.x += 0.10f;
		::g_pLightManager->vecLights[g_LightObjNumber].position.x += 0.10f;
		break;
	case GLFW_KEY_LEFT_BRACKET:		// [{ key
		//::g_vecGameObjects[g_GameObjNumber]->position.z += 0.10f;
		::g_pLightManager->vecLights[g_LightObjNumber].position.z += 0.10f;
		break;
	case GLFW_KEY_RIGHT_BRACKET:		// ]} key
		//::g_vecGameObjects[g_GameObjNumber]->position.z -= 0.10f;
		::g_pLightManager->vecLights[g_LightObjNumber].position.z -= 0.10f;
		break;
	}

	// Change Camera Position
	const float CAMERAMOVEMENT = 0.1f;
	switch( key )
	{
	case GLFW_KEY_A:		// Left
		g_cameraXYZ.x -= CAMERAMOVEMENT;
		break;
	case GLFW_KEY_D:		// Right
		g_cameraXYZ.x += CAMERAMOVEMENT;
		break;
	case GLFW_KEY_W:		// Forward (along z)
		g_cameraXYZ.z += CAMERAMOVEMENT;
		break;
	case GLFW_KEY_S:		// Backwards (along z)
		g_cameraXYZ.z -= CAMERAMOVEMENT;
		break;
	case GLFW_KEY_Q:		// "Down" (along y axis)
		g_cameraXYZ.y -= CAMERAMOVEMENT;
		break;
	case GLFW_KEY_E:		// "Up" (along y axis)
		g_cameraXYZ.y += CAMERAMOVEMENT;
		break;
	}

	// Change camera Acceleration
	switch( key )
	{
	case GLFW_KEY_J:		// Left
		CAMERASPEED.x -= 0.00001f;
		break;
	case GLFW_KEY_L:		// Right
		CAMERASPEED.x += 0.00001f;
		break;
	case GLFW_KEY_I:		// Forward (along z)
		CAMERASPEED.z += 0.00001f;
		break;
	case GLFW_KEY_K:		// Backwards (along z)
		CAMERASPEED.z -= 0.00001f;
		break;
	case GLFW_KEY_U:		// "Down" (along y axis)
		CAMERASPEED.y -= 0.00001f;
		break;
	case GLFW_KEY_O:		// "Up" (along y axis)
		CAMERASPEED.y += 0.00001f;
		break;
	}

	// Stop Camera
	if( key == GLFW_KEY_P && action == GLFW_PRESS )
	{
		CAMERASPEED = glm::vec3( 0.0f, 0.0f, 0.0f );
	}

	// Change Selected Light
	switch ( key )
	{
	case GLFW_KEY_1:
		g_LightObjNumber = 0; 
		break;
	case GLFW_KEY_2:
		g_LightObjNumber = 1;
		break;
	case GLFW_KEY_3:
		g_LightObjNumber = 2;
		break;
	case GLFW_KEY_4:
		g_LightObjNumber = 3;
		break;
	case GLFW_KEY_5:
		g_LightObjNumber = 4;
		break;
	case GLFW_KEY_6:
		g_LightObjNumber = 5;
		break;
	case GLFW_KEY_7:
		g_LightObjNumber = 6;
		break;
	case GLFW_KEY_8:
		g_LightObjNumber = 7;
		break;
	case GLFW_KEY_9:
		g_LightObjNumber = 8;
		break;
	}

	return;
}

int main( void )
{
	GLFWwindow* window;
	//GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location; //vpos_location, vcol_location;
	glfwSetErrorCallback( error_callback );

	// Other uniforms:
	GLint uniLoc_materialDiffuse = -1;
	GLint uniLoc_materialAmbient = -1;
	GLint uniLoc_ambientToDiffuseRatio = -1; 	// Maybe	// 0.2 or 0.3
	GLint uniLoc_materialSpecular = -1;  // rgb = colour of HIGHLIGHT only
										 // w = shininess of the 
	GLint uniLoc_eyePosition = -1;	// Camera position
	GLint uniLoc_mModel = -1;
	GLint uniLoc_mView = -1;
	GLint uniLoc_mProjection = -1;

	if( !glfwInit() )
		exit( EXIT_FAILURE );

	sWindowConfig wConfig;

	loadConfigFile( "config.txt", wConfig );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );

	window = glfwCreateWindow( wConfig.width, wConfig.height,
		wConfig.title.c_str(),
		NULL, // glfwGetPrimaryMonitor(), //
		NULL );
	if( !window )
	{
		glfwTerminate();
		exit( EXIT_FAILURE );
	}

	glfwSetKeyCallback( window, key_callback );
	glfwMakeContextCurrent( window );
	gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress );
	glfwSwapInterval( 1 );

	std::cout << glGetString( GL_VENDOR ) << " "
		<< glGetString( GL_RENDERER ) << ", "
		<< glGetString( GL_VERSION ) << std::endl;
	std::cout << "Shader language version: " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << std::endl;

	::g_pShaderManager = new cShaderManager();

	cShaderManager::cShader vertShader;
	cShaderManager::cShader fragShader;

	vertShader.fileName = "simpleVert.glsl";
	fragShader.fileName = "simpleFrag.glsl";

	::g_pShaderManager->setBasePath( "assets//shaders//" );

	// Shader objects are passed by reference so that
	//	we can look at the results if we wanted to. 
	if( !::g_pShaderManager->createProgramFromFile(
		"mySexyShader", vertShader, fragShader ) )
	{
		std::cout << "Oh no! All is lost!!! Blame Loki!!!" << std::endl;
		std::cout << ::g_pShaderManager->getLastError() << std::endl;
		// Should we exit?? 
		return -1;
		//		exit(
	}
	std::cout << "The shaders compiled and linked OK" << std::endl;

	//Load models
	::g_pVAOManager = new cVAOMeshManager();

	GLint sexyShaderID = ::g_pShaderManager->getIDFromFriendlyName("mySexyShader");	

	//loadMeshesFile( "meshlist.txt", sexyShaderID );

	std::string error;
	if (!Load3DModelsIntoMeshManager(sexyShaderID, ::g_pVAOManager, error))
	{
		std::cout << "Not all models were loaded..." << std::endl;
		std::cout << error << std::endl;
	}

	//loadObjectsFile("objects.txt");

	if (!LoadModelsIntoScene(error))
	{
		std::cout << "Not all objects were loaded..." << std::endl;
		std::cout << error << std::endl;
	}

	GLint currentProgID = ::g_pShaderManager->getIDFromFriendlyName( "mySexyShader" );

	// Get the uniform locations for this shader
	mvp_location = glGetUniformLocation( currentProgID, "MVP" );		// program, "MVP");
	uniLoc_materialDiffuse = glGetUniformLocation( currentProgID, "materialDiffuse" );
	uniLoc_materialAmbient = glGetUniformLocation( currentProgID, "materialAmbient" );
	uniLoc_ambientToDiffuseRatio = glGetUniformLocation( currentProgID, "ambientToDiffuseRatio" );
	uniLoc_materialSpecular = glGetUniformLocation( currentProgID, "materialSpecular" );
	uniLoc_eyePosition = glGetUniformLocation( currentProgID, "eyePosition" );

	uniLoc_mModel = glGetUniformLocation( currentProgID, "mModel" );
	uniLoc_mView = glGetUniformLocation( currentProgID, "mView" );
	uniLoc_mProjection = glGetUniformLocation( currentProgID, "mProjection" );

	::g_pLightManager = new cLightManager();

	::g_pLightManager->CreateLights( g_NUMBER_OF_LIGHTS );	// There are 10 lights in the shader
	::g_pLightManager->LoadShaderUniformLocations( currentProgID );

	// Change ZERO (the SUN) light position
	::g_pLightManager->vecLights[0].position = glm::vec3(0.0f, 45.0f, 0.0f);
	//::g_pLightManager->vecLights[0].diffuse = glm::vec3( 1.0f, 1.0f, 1.0f );
	//::g_pLightManager->vecLights[0].ambient = glm::vec3( 1.0f, 1.0f, 1.0f );
	::g_pLightManager->vecLights[0].attenuation.y = 0.06f;		// Change the linear attenuation


	//set the diffuse light to white
	::g_pLightManager->vecLights[0].diffuse = glm::vec3(1.0f, 1.0f, 1.0f);

	//set the light specular to white
	::g_pLightManager->vecLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	//set the light ambient to black
	::g_pLightManager->vecLights[0].ambient = glm::vec3( 0.0, 0.0, 0.0 );
	

	// ADD 4 MORE LIGHTS========================================
	{

		::g_pLightManager->vecLights[1].position = glm::vec3( -30.0f, 20.0f, 18.0f );
		::g_pLightManager->vecLights[1].diffuse = glm::vec3( 1.0f, 1.0f, 1.0f );

		::g_pLightManager->vecLights[2].position = glm::vec3( -30.0f, 20.0f, -18.0f );
		::g_pLightManager->vecLights[2].diffuse = glm::vec3( 1.0f, 1.0f, 1.0f );

		::g_pLightManager->vecLights[3].position = glm::vec3( 30.0f, 20.0f, 18.0f );
		::g_pLightManager->vecLights[3].diffuse = glm::vec3( 1.0f, 1.0f, 1.0f );

		::g_pLightManager->vecLights[4].position = glm::vec3( 30.0f, 20.0f, -18.0f );
		::g_pLightManager->vecLights[4].diffuse = glm::vec3( 1.0f, 1.0f, 1.0f );

	}
	//=========================================================

	loadLightObjects();

	glEnable( GL_DEPTH );

	// Gets the "current" time "tick" or "step"
	double lastTimeStep = glfwGetTime();

	// Main game or application loop
	while( !glfwWindowShouldClose( window ) )
	{
		float ratio;
		int width, height;
		glm::mat4x4 m, p, mvp;			//  mat4x4 m, p, mvp;

		glfwGetFramebufferSize( window, &width, &height );
		ratio = width / ( float )height;
		glViewport( 0, 0, width, height );

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		// Update all the light uniforms...
		// (for the whole scene)
		::g_pLightManager->CopyLightInformationToCurrentShader(); 

		// "Draw scene" loop
		//for ( int index = 0; index != MAXNUMBEROFGAMEOBJECTS; index++ )

		unsigned int sizeOfVector = ::g_vecGameObjects.size();
		for ( int index = 0; index != sizeOfVector; index++ )
		{
			// Is there a game object? 
			if ( ::g_vecGameObjects[index] == 0 )
			{	// Nothing to draw
				continue;		// Skip all for loop code and go to next
			}

			// Was near the draw call, but we need the mesh name
			std::string meshToDraw = ::g_vecGameObjects[index]->meshName;		//::g_GameObjects[index]->meshName;

			sVAOInfo VAODrawInfo;
			if ( ::g_pVAOManager->lookupVAOFromName( meshToDraw, VAODrawInfo ) == false )
			{	// Didn't find mesh
				continue;
			}

			// Change Light Objects position based on light position
			// The game object sphere that "contains" the light follows the light
			if ( ::g_vecGameObjects[index]->bIsLight == true )
			{
				int lightIndex = ::g_vecGameObjects[index]->myLight;
				::g_vecGameObjects[index]->position = ::g_pLightManager->vecLights[lightIndex].position;
				::g_vecGameObjects[index]->diffuseColour = glm::vec4( ::g_pLightManager->vecLights[lightIndex].diffuse, 1.0f );
			}

			// There IS something to draw
			m = glm::mat4x4( 1.0f );	//		mat4x4_identity(m);

			//::g_vecGameObjects[index]->orientation.z += ( ::g_vecGameObjects[index]->vel.z ) / 30;
			//::g_vecGameObjects[index]->orientation.y += ( ::g_vecGameObjects[index]->vel.y ) / 10;
			//::g_vecGameObjects[index]->orientation.x += ( ::g_vecGameObjects[index]->vel.x ) / 10;

			glm::mat4 matRreRotZ = glm::mat4x4( 1.0f );
			matRreRotZ = glm::rotate( matRreRotZ, ::g_vecGameObjects[index]->orientation.z,
				glm::vec3( 0.0f, 0.0f, 1.0f ) );
			m = m * matRreRotZ;

			//matRreRotZ = glm::rotate( matRreRotZ, ::g_vecGameObjects[index]->orientation.y,
			//	glm::vec3( 0.0f, 1.0f, 0.0f ) );
			//m = m * matRreRotZ;

			//matRreRotZ = glm::rotate( matRreRotZ, ::g_vecGameObjects[index]->orientation.x,
			//	glm::vec3( 1.0f, 0.0f, 0.0f ) );
			//m = m * matRreRotZ;

			glm::mat4 trans = glm::mat4x4( 1.0f );
			trans = glm::translate( trans,
				::g_vecGameObjects[index]->position );
			m = m * trans;

			glm::mat4 matPostRotZ = glm::mat4x4( 1.0f );
			matPostRotZ = glm::rotate( matPostRotZ, ::g_vecGameObjects[index]->orientation2.z,
				glm::vec3( 0.0f, 0.0f, 1.0f ) );
			m = m * matPostRotZ;

			//// IF the game object isn't a light object, it will rotate as normal
			//if ( !::g_vecGameObjects[index]->bIsLight )
			//{
			//	::g_vecGameObjects[index]->orientation2.x += ::g_vecGameObjects[index]->rotation.x;
			//	::g_vecGameObjects[index]->orientation2.y += ::g_vecGameObjects[index]->rotation.y;
			//	::g_vecGameObjects[index]->orientation2.z += ::g_vecGameObjects[index]->rotation.z;
			//}

			glm::mat4 matPostRotY = glm::mat4x4( 1.0f );
			matPostRotY = glm::rotate( matPostRotY, ::g_vecGameObjects[index]->orientation2.y,
				glm::vec3( 0.0f, 1.0f, 0.0f ) );
			m = m * matPostRotY;

			glm::mat4 matPostRotX = glm::mat4x4( 1.0f );
			matPostRotX = glm::rotate( matPostRotX, ::g_vecGameObjects[index]->orientation2.x,
				glm::vec3( 1.0f, 0.0f, 0.0f ) );
			m = m * matPostRotX;
			// TODO: add the other rotation matrix (i.e. duplicate code above)

			float finalScale = VAODrawInfo.scaleForUnitBBox * ::g_vecGameObjects[index]->scale;

			glm::mat4 matScale = glm::mat4x4( 1.0f );
			matScale = glm::scale( matScale,
				glm::vec3( finalScale,
					finalScale,
					finalScale ) );
			m = m * matScale;

			// Change from an orthographic view to a perspective view
			p = glm::perspective( 0.6f,			// FOV
				ratio,		// Aspect ratio
				0.1f,			// Near (as big as possible)
				1000.0f );	// Far (as small as possible)

			// Set Camera Speed according to the user input
			//g_cameraXYZ.z -= 0.0001f;
			g_cameraXYZ.x += CAMERASPEED.x;
			g_cameraXYZ.z += CAMERASPEED.y;
			g_cameraXYZ.y += CAMERASPEED.z;

			// View or "camera" matrix
			glm::mat4 v = glm::mat4( 1.0f );	// identity

			v = glm::lookAt( g_cameraXYZ,			// "eye" or "camera" position
				g_cameraTarget_XYZ,					// "At" or "target"							 
				//glm::vec3( 0.0f, 1.0f, 0.0f ) );	// "up" vector
				glm::vec3( 0.0f, 1.0f, 0.0f ) );	// "up" vector

			//mat4x4_mul(mvp, p, m);
			mvp = p * v * m;			// This way (sort of backwards)

			::g_pShaderManager->useShaderProgram( "mySexyShader" );
			GLint shaderID = ::g_pShaderManager->getIDFromFriendlyName( "mySexyShader" );
			
			glUniformMatrix4fv( uniLoc_mModel, 1, GL_FALSE,
				( const GLfloat* )glm::value_ptr( m ) );
			glUniformMatrix4fv( uniLoc_mView, 1, GL_FALSE,
				( const GLfloat* )glm::value_ptr( v ) );
			glUniformMatrix4fv( uniLoc_mProjection, 1, GL_FALSE,
				( const GLfloat* )glm::value_ptr( p ) );

			glm::mat4 mWorldInTranpose = glm::inverse( glm::transpose( m ) );
			
			glUniform4f( uniLoc_materialDiffuse,
				::g_vecGameObjects[index]->diffuseColour.r,
				::g_vecGameObjects[index]->diffuseColour.g,
				::g_vecGameObjects[index]->diffuseColour.b,
				::g_vecGameObjects[index]->diffuseColour.a );
			//... and all the other object material colours
			

			//		glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );

			// Check if the bIsWireframe is true and set PolygonMonde to GL_LINE
			if ( bIsWireframe )	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			else glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );	// Default

			
			GLfloat whiteSpecularMaterial[] = { 1.0, 1.0, 1.0 };
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, whiteSpecularMaterial);
			GLfloat mShininess[] = { 128 };
			glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mShininess);
			
			glEnable( GL_DEPTH_TEST );	// Test for Z and store in z buffer
			glCullFace( GL_BACK );		// Draw only the normals that are "front-facing"

			glBindVertexArray( VAODrawInfo.VAO_ID );

			glDrawElements( GL_TRIANGLES,
				VAODrawInfo.numberOfIndices,
				GL_UNSIGNED_INT,	// 32 bit int
				0 ); // g_numberOfVertices

			glBindVertexArray( 0 );

		}//for ( int index = 0...


		std::stringstream ssTitle;
		ssTitle << "Camera (xyz): "
			<< g_cameraXYZ.x << ", "
			<< g_cameraXYZ.y << ", "
			<< g_cameraXYZ.z;
		glfwSetWindowTitle( window, ssTitle.str().c_str() );

		glfwSwapBuffers( window );
		glfwPollEvents();

		// Essentially the "frame time"
		// Now many seconds that have elapsed since we last checked
		double curTime = glfwGetTime();
		double deltaTime = curTime - lastTimeStep;
		
		// Physics Calculation
		PhysicsStep( deltaTime );

		lastTimeStep = curTime;

	}// while ( ! glfwWindowShouldClose(window) )


	glfwDestroyWindow( window );
	glfwTerminate();

	// 
	delete ::g_pShaderManager;
	delete ::g_pVAOManager;

	//    exit(EXIT_SUCCESS);
	return 0;
}

//Load Config.txt
void loadConfigFile( std::string fileName, sWindowConfig& wConfig )
{
	// TODO change this config formating
	std::ifstream infoFile( fileName );
	if( !infoFile.is_open() )
	{	// File didn't open...
		std::cout << "Can't find config file" << std::endl;
		std::cout << "Using defaults" << std::endl;
	}
	else
	{	// File DID open, so read it... 
		std::string a;

		infoFile >> a;	// "Game"	//std::cin >> a;
		infoFile >> a;	// "Config"
		infoFile >> a;	// "width"

		infoFile >> wConfig.width;	// 1080

		infoFile >> a;	// "height"

		infoFile >> wConfig.height;	// 768

		infoFile >> a;		// Title_Start

		std::stringstream ssTitle;		// Inside "sstream"
		bool bKeepReading = true;
		do
		{
			infoFile >> a;		// Title_End??
			if( a != "Title_End" )
			{
				ssTitle << a << " ";
			}
			else
			{	// it IS the end! 
				bKeepReading = false;
				wConfig.title = ssTitle.str();
			}
		} while( bKeepReading );
	}
}

// Generate real random numbers
float generateRandomNumber( float min, float max )
{

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

	std::default_random_engine generator( seed );
	std::uniform_real_distribution<float> distribution( min, max );

	float randomNumber = 0.0;

	randomNumber = distribution( generator );
	return randomNumber;

}


void loadLightObjects()
{
	for ( int index = 0; index < g_NUMBER_OF_LIGHTS; index++ )
	{
		// Create a new GO
		cGameObject* pTempGO = new cGameObject();

		pTempGO->meshName = "lightsphere"; // Set the name of the mesh
		
		// position is based on light position
		pTempGO->position = ::g_pLightManager->vecLights[index].position;

		if ( index == 0 ) pTempGO->scale = 3.0f;
		else pTempGO->scale = 1.0f;

		// Each light is initially white
		pTempGO->diffuseColour = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );

		pTempGO->bIsLight = true;
		pTempGO->myLight = index;

		::g_vecGameObjects.push_back( pTempGO );
	}
}

// Update the world 1 "step" in time
void PhysicsStep( double deltaTime )
{
	// Distance                          m
	// Velocity = distance / time		 m/s
	// Accleration = velocity / time     m/s/s

	// Distance = time * velocity
	// velocity = time * acceleration

	//// HACK: Change all objects back to white (before collision test)
	//for ( int index = 0; index != ::g_vecGameObjects.size(); index++ )
	//{
	//	cGameObject* pCurGO = ::g_vecGameObjects[index];
	//	pCurGO->diffuseColour = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
	//}//for ( int index

	const glm::vec3 GRAVITY = glm::vec3( 0.0f, -2.0f, 0.0f );

	// Identical to the 'render' (drawing) loop
	for ( int index = 0; index != ::g_vecGameObjects.size(); index++ )
	{
		cGameObject* pCurGO = ::g_vecGameObjects[index];

		// Is this object to be updated?
		if ( !pCurGO->bIsUpdatedInPhysics )
		{	// DON'T update this
			continue;		// Skip everything else in the for
		}

		// Explicity Euler integration (RK4)
		// New position is based on velocity over time
		glm::vec3 deltaPosition = ( float )deltaTime * pCurGO->vel;
		pCurGO->position += deltaPosition;

		// New velocity is based on acceleration over time
		glm::vec3 deltaVelocity = ( ( float )deltaTime * pCurGO->accel )
			+ ( ( float )deltaTime * GRAVITY );

		pCurGO->vel += deltaVelocity;

		// HACK: Collision step
		switch ( pCurGO->typeOfObject )
		{
		case eTypeOfObject::SPHERE:
			//	// Comare this to EVERY OTHER object in the scene
			for ( int indexEO = 0; indexEO != ::g_vecGameObjects.size(); indexEO++ )
			{
				// Don't test for myself
				if ( index == indexEO )
					continue;		// It's me!!

				cGameObject* pOtherObject = ::g_vecGameObjects[indexEO];
				// Is another object
				switch ( pOtherObject->typeOfObject )
				{
				case eTypeOfObject::SPHERE:
					//
					if ( PenetrationTestSphereSphere( pCurGO, pOtherObject ) )
					{
						////std::cout << "Collision!" << std::endl;
						//pCurGO->diffuseColour = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
						//pOtherObject->diffuseColour = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
					}

					break;
				}
			}

			//		switch ( pGO_to_Compare->typeOfObject )
			//		{
			//		case eTypeOfObject::SPHERE:
			//			CalcSphereSphereColision( pCurGO, pGO_to_Compare );
			//			break;
			//		case eTypeOfObject::PLANE:
			//			CalcSpherePlaneColision( pCurGO, pGO_to_Compare );
			//			break;
			//		// More if I'd like that.
			//
			//		}
			//	}

			// HACK
			const float SURFACEOFGROUND = 0.0f;
			const float RIGHTSIDEWALL = 5.0f;
			const float LEFTSIDEWALL = -5.0f;
			 
			//Sphere-Plane detection

			// HACK (Ground is table (game object 0)
			if ( ( pCurGO->position.y - pCurGO->radius ) <= ::g_vecGameObjects[0]->position.z)
			{	// Object has "hit" the ground 
				//pCurGO->diffuseColour = glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f );
				pCurGO->vel.y = +( fabs( pCurGO->vel.y ) );
			}

			//if ( ( pCurGO->position.x + pCurGO->radius ) >= RIGHTSIDEWALL )
			//{	// Object too far to the right
			//	// Object has penetrated the right plane
			//	//pCurGO->diffuseColour = glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f );
			//	pCurGO->vel.x = -( fabs( pCurGO->vel.x ) );
			//}
			if ( ( pCurGO->position.x - pCurGO->radius ) <= LEFTSIDEWALL )
			{	// Object too far to the left
				// Object has penetrated the left plane
				//pCurGO->diffuseColour = glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f );
				pCurGO->vel.x = +( fabs( pCurGO->vel.x ) );
			}
			break;
		};

	}//for ( int index...

	return;
}