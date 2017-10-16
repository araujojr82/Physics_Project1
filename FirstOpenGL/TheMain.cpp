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
#include <string>
//#include <istream>

#include <vector>						// smart array, "array" in most languages
#include "Utilities.h"
#include "ModelUtilities.h"
#include "cMesh.h"
#include "cShaderManager.h" 
#include "cGameObject.h"
#include "cVAOMeshManager.h"

#include "Physics.h"

#include "cLightManager.h"

#include <random>
#include <chrono>

#define _USE_MATH_DEFINES
#include <math.h>

// Euclides: Control selected object for movement
int g_GameObjNumber = 0;				// game object vector position number 
int g_LightObjNumber = 0;				// light object vector position

int angle = 0;
float speed = 12.0f;

float increaseAngle( float angle )
{
	if( angle < 360 ) angle += 1; // 0.0027777777777778f;
	else angle = 0;

	return angle;

}
float decreaseAngle( float angle )
{
	if( angle > 0 ) angle -= 1; // 0.0027777777777778f;
	else angle = 360;
	return angle;
}
float increaseSpeed( float speed )
{
	if( speed < 12.0f ) speed += 1.0f;
	else speed = 12.0f;

	if( speed > 12.0f ) speed = 12.0f;
	return speed;
}
float decreaseSpeed( float speed )
{
	if( speed > 1.0f ) speed -= 1.0f;
	else speed = 1.0f;

	if( speed < 1.0f ) speed = 1.0f;
	return speed;
}

cMesh g_MeshPoolTable;

//glm::vec3 CAMERASPEED = glm::vec3( 0.0f, 0.0f, 0.0f );
static const int g_NUMBER_OF_LIGHTS = 1;
static const float g_FRICTION_FORCE = 0.01f;

bool bIsWireframe = false;
bool g_isThereMovement = false;

// Remember to #include <vector>...
std::vector< cGameObject* > g_vecGameObjects;

struct pointTriangles{
	glm::vec3 point;
	cPhysTriangle triangle;
};

//std::vector< glm::vec3 > g_vecPoints;
std::vector< pointTriangles > g_vecPoints;

cGameObject* g_pTheDebugSphere;
cGameObject* g_pTheCueGO;

glm::vec3 g_cameraXYZ = glm::vec3( 0.0f, 40.0f, 0.1f ); // Default top pool view
//glm::vec3 g_cameraXYZ = glm::vec3(0.0f, 0.2f, 30.0f); // Side "horizontal"  view
glm::vec3 g_cameraTarget_XYZ = glm::vec3( 0.0f, 0.0f, 0.0f );

// TODO include camera new code

cVAOMeshManager*	g_pVAOManager = 0;		// or NULL or nullptr

cShaderManager*		g_pShaderManager;		// Heap, new (and delete)
cLightManager*		g_pLightManager;

// Other uniforms:
GLint uniLoc_materialDiffuse = -1;
GLint uniLoc_materialAmbient = -1;
GLint uniLoc_ambientToDiffuseRatio = -1; 	// Maybe	// 0.2 or 0.3
GLint uniLoc_materialSpecular = -1;  // rgb = colour of HIGHLIGHT only
									 // w = shininess of the 
GLint uniLoc_bIsDebugWireFrameObject = -1;

GLint uniLoc_eyePosition = -1;	// Camera position
GLint uniLoc_mModel = -1;
GLint uniLoc_mView = -1;
GLint uniLoc_mProjection = -1;

struct sWindowConfig
{
public:
	int height = 480;
	int width = 640;
	std::string title = "Physics 101 is Awesome!";
};

// Forward declare the Functions
void loadConfigFile( std::string fileName, sWindowConfig& wConfig );
void loadLightObjects();

void PhysicsStep( double deltaTime );

bool LoadModelsIntoScene(std::string &error);
bool Load3DModelsIntoMeshManager(int shaderID, cVAOMeshManager* pVAOManager, std::string &error);
float generateRandomNumber( float min, float max );


void DrawObject( cGameObject* pTheGO );
void DrawDebugSphere( glm::vec3 location, glm::vec4 colour, float scale );
void findClosestPointsOnTable( glm::vec3 thePoint );

glm::vec3 returnNormal( glm::vec3 vertex[3] );


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

	// "Shoot" the white ball (at random speed for now)
	// TODO change this to be controlled by user
	if( key == GLFW_KEY_SPACE && action == GLFW_PRESS )
	{
		//::g_vecGameObjects[2]->vel.x = generateRandomNumber( 6.0f, 12.0f );
		//::g_vecGameObjects[2]->vel.z = generateRandomNumber( 6.0f, 12.0f );

		float speedX, speedZ;

		//speedX = speed * sin( angle ); 
		//speedZ = speed * cos( angle );

		speedX = speed * cos( ( angle / 180 )* M_PI ); 
		speedZ = speed * sin( ( angle / 180 )* M_PI );

		::g_vecGameObjects[2]->vel.x = speedX;
		::g_vecGameObjects[2]->vel.z = speedZ;

	}

	// Change "target" selected game object
	if( key == GLFW_KEY_TAB && action == GLFW_PRESS )
	{
		if( g_GameObjNumber << ( ::g_vecGameObjects.size() - 1 ) ) {
			g_GameObjNumber++;
		}
		else
		{
			g_GameObjNumber = 0;
		}
	}

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

	// Change Game Object's position
	switch( key )
	{
	case GLFW_KEY_UP:		// Up arrow
		speed = decreaseSpeed( speed );
		std::cout << "Speed : " << speed << std::endl;
		break;
	case GLFW_KEY_DOWN:		// Down arrow
		speed = increaseSpeed( speed );
		std::cout << "Speed : " << speed << std::endl;
		break;
	case GLFW_KEY_LEFT:		// Left arrow
		angle = increaseAngle( angle );
		std::cout << "Angle : " << angle << std::endl;

		if( g_pTheCueGO != NULL )
		{
			g_pTheCueGO->orientation2.y = glm::radians( (float)angle );
		}
		break;
	case GLFW_KEY_RIGHT:	// Right arrow
		angle = decreaseAngle( angle );
		std::cout << "Angle : " << angle << std::endl;
		if( g_pTheCueGO != NULL )
		{
			g_pTheCueGO->orientation2.y = glm::radians( ( float ) angle );
		}
		break;
	//case GLFW_KEY_LEFT_BRACKET:		// [{ key
	//	::g_vecGameObjects[g_GameObjNumber]->position.z += 0.10f;
	//	break;
	//case GLFW_KEY_RIGHT_BRACKET:		// ]} key
	//	::g_vecGameObjects[g_GameObjNumber]->position.z -= 0.10f;
	//	break;
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
	}

	return;
}

int main( void )
{
	GLFWwindow* window;
	GLint mvp_location;
	glfwSetErrorCallback( error_callback );

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

	std::string error;
	if (Load3DModelsIntoMeshManager(sexyShaderID, ::g_pVAOManager, error))
	{
		std::cout << "Not all meshes were loaded..." << std::endl;
		std::cout << error << std::endl;
	}

	if (LoadModelsIntoScene(error))
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
	
	uniLoc_bIsDebugWireFrameObject = glGetUniformLocation( currentProgID, "bIsDebugWireFrameObject" );

	uniLoc_eyePosition = glGetUniformLocation( currentProgID, "eyePosition" );


	uniLoc_mModel = glGetUniformLocation( currentProgID, "mModel" );
	uniLoc_mView = glGetUniformLocation( currentProgID, "mView" );
	uniLoc_mProjection = glGetUniformLocation( currentProgID, "mProjection" );

	::g_pLightManager = new cLightManager();

	::g_pLightManager->CreateLights( g_NUMBER_OF_LIGHTS );	// There are 10 lights in the shader
	::g_pLightManager->LoadShaderUniformLocations( currentProgID );

	// Change ZERO light position "Main Top light"
	::g_pLightManager->vecLights[0].position = glm::vec3(0.0f, 45.0f, 0.0f);
	::g_pLightManager->vecLights[0].attenuation.y = 0.06f;		// Change the linear attenuation
	::g_pLightManager->vecLights[0].diffuse = glm::vec3(1.0f, 1.0f, 1.0f);

	// ADD 4 MORE LIGHTS========================================
	// One at each "corner"
	//{

	//	::g_pLightManager->vecLights[1].position = glm::vec3( -30.0f, 20.0f, 18.0f );
	//	::g_pLightManager->vecLights[1].diffuse = glm::vec3( 1.0f, 1.0f, 1.0f );
	//	::g_pLightManager->vecLights[1].attenuation.y = 0.06f;
	//	
	//	::g_pLightManager->vecLights[2].position = glm::vec3( -30.0f, 20.0f, -18.0f );
	//	::g_pLightManager->vecLights[2].diffuse = glm::vec3( 1.0f, 1.0f, 1.0f );
	//	::g_pLightManager->vecLights[2].attenuation.y = 0.0222676f;
	//	::g_pLightManager->vecLights[2].attenuation.z = 0.0f;


	//	::g_pLightManager->vecLights[3].position = glm::vec3( 30.0f, 20.0f, 18.0f );
	//	::g_pLightManager->vecLights[3].diffuse = glm::vec3( 1.0f, 1.0f, 1.0f );

	//	::g_pLightManager->vecLights[4].position = glm::vec3( 30.0f, 20.0f, -18.0f );
	//	::g_pLightManager->vecLights[4].diffuse = glm::vec3( 1.0f, 1.0f, 1.0f );

	//}
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
		//glm::mat4x4 m, p, mvp;			//  mat4x4 m, p, mvp;
		//glm::mat4x4 p, mvp;			//  mat4x4 m, p, mvp;
		glm::mat4x4 p;			//  mat4x4 m, p, mvp;

		glfwGetFramebufferSize( window, &width, &height );
		ratio = width / ( float )height;
		glViewport( 0, 0, width, height );

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		//        glUseProgram(program);
		::g_pShaderManager->useShaderProgram( "mySexyShader" );
		GLint shaderID = ::g_pShaderManager->getIDFromFriendlyName( "mySexyShader" );

		// Update all the light uniforms...
		// (for the whole scene)
		::g_pLightManager->CopyLightInformationToCurrentShader(); 

		// Projection and view don't change per scene (maybe)
		p = glm::perspective( 0.6f,			// FOV
								ratio,		// Aspect ratio
								0.1f,		// Near (as big as possible)
								1000.0f );	// Far (as small as possible)

		// View or "camera" matrix
		glm::mat4 v = glm::mat4( 1.0f );	// identity

		v = glm::lookAt( g_cameraXYZ,							// "eye" or "camera" position
							g_cameraTarget_XYZ,					// "At" or "target" 
							glm::vec3( 0.0f, 1.0f, 0.0f ) );	// "up" vector

		glUniformMatrix4fv( uniLoc_mView, 1, GL_FALSE,
							( const GLfloat* ) glm::value_ptr( v ) );
		glUniformMatrix4fv( uniLoc_mProjection, 1, GL_FALSE,
							( const GLfloat* ) glm::value_ptr( p ) );

		// "Draw scene" loop
		unsigned int sizeOfVector = ::g_vecGameObjects.size();
		for ( int index = 0; index != sizeOfVector; index++ )
		{
			cGameObject* pTheGO = ::g_vecGameObjects[index];

			DrawObject( pTheGO );

		}//for ( int index = 0...

		std::stringstream ssTitle;
		ssTitle << "Camera (xyz): "
			<< g_cameraXYZ.x << ", "
			<< g_cameraXYZ.y << ", "
			<< g_cameraXYZ.z;
		glfwSetWindowTitle( window, ssTitle.str().c_str() );

		// **************************************************
		// Physics being updated BEFORE the present 
		// Essentially the "frame time"
		// How many seconds that have elapsed since we last checked
		double curTime = glfwGetTime();
		double deltaTime = curTime - lastTimeStep;

		// Physics Calculation
		PhysicsStep( deltaTime );

		lastTimeStep = curTime;
		// **************************************************
		
		// "Presents" what we've drawn
		// Done once per scene 
		glfwSwapBuffers( window );
		glfwPollEvents();


	}// while ( ! glfwWindowShouldClose(window) )


	glfwDestroyWindow( window );
	glfwTerminate();

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

void findClosestPointsOnTable( glm::vec3 thePoint )
{
	int numberOfTriangles = ::g_MeshPoolTable.vecPhysTris.size();

	//std::fill( g_vecPoints.begin(), g_vecPoints.end(), glm::vec3( NULL ) );
	g_vecPoints.clear();

	for( int triIndex = 0; triIndex != numberOfTriangles; triIndex++ )
	{
		// Get reference object for current triangle
		// (so the line below isn't huge long...)
		cPhysTriangle& curTriangle = ::g_MeshPoolTable.vecPhysTris[triIndex];

		pointTriangles theClosestPoint;
		theClosestPoint.point = curTriangle.ClosestPtPointTriangle( thePoint );
		theClosestPoint.triangle = curTriangle;

		::g_vecPoints.push_back( theClosestPoint );

		//DrawDebugSphere( theClosestPoint.point, glm::vec4( 1, 1, 1, 1 ), 0.2f );
	}

		//collisionThingy:
		//	- Which objects collided (sphere - triangle/sphere - sphere)
		//	- Where, the speed, etc.


		//	vecMyColisions.push_Back( collisionThingy );
		// Calc response...
		//		glm::reflect( 

	return;
}


// Update the world 1 "step" in time
void PhysicsStep( double deltaTime )
{
	// Distance                          m
	// Velocity = distance / time		 m/s
	// Accleration = velocity / time     m/s/s

	// Distance = time * velocity
	// velocity = time * acceleration

	//// Use the white ball location to draw the closest points in the table
	//glm::vec3 thePointToTest = ::g_vecGameObjects[2]->position;
	//DrawClosestPointsOnTable( thePointToTest );

	// Boundaries
	const float SURFACEOFGROUND = 0.0f;
	const float RIGHTSIDEWALL = 18.0f;
	const float LEFTSIDEWALL = -18.0f;
	const float BACKSIDEWALL = -9.0f;
	const float FRONTSIDEWALL = 9.0f;

	glm::vec3 triangleNormal = glm::vec3( 0.0f );

	//const glm::vec3 GRAVITY = glm::vec3( 0.0f, -2.0f, 0.0f );
	const glm::vec3 GRAVITY = glm::vec3(0.0f, 0.0f, 0.0f);		//NO gravity (for now)
	
	// Always set this to false before checking the movement in the loop
	g_isThereMovement = false;

	// Identical to the 'render' (drawing) loop
	for ( int index = 0; index != ::g_vecGameObjects.size(); index++ )
	{
		// Look for the cue object
		if( g_vecGameObjects[index]->typeOfObject == eTypeOfObject::CUE )
		{	// We found the CUE!
			g_pTheCueGO = ::g_vecGameObjects[index];
		}

		cGameObject* pCurGO = ::g_vecGameObjects[index];

		// Stores the closests points in a Vector to be used bellow
		glm::vec3 thePointToTest = pCurGO->position;
		findClosestPointsOnTable( thePointToTest );

		// Is this object to be updated?
		if ( !pCurGO->bIsUpdatedInPhysics )
		{	// DON'T update this
			continue;		// Skip everything else in the for
		}

		glm::vec3 olderPosition = pCurGO->prevPosition;

		pCurGO->prevPosition = pCurGO->position;

		// Explicity Euler integration (RK4)
		// New position is based on velocity over time
		glm::vec3 deltaPosition = ( float )deltaTime * pCurGO->vel;
		//pCurGO->position += deltaPosition;

		//Check future position to see if it's beyond the borders of the table
		glm::vec3 futurePosition = pCurGO->position += deltaPosition;
		if( futurePosition.x > RIGHTSIDEWALL ||
			futurePosition.x < LEFTSIDEWALL ||
			futurePosition.z > FRONTSIDEWALL ||
			futurePosition.z < BACKSIDEWALL )
		{ // The position is beyond the borders of the table
		  // Go back to the first position of the object
			pCurGO->position = pCurGO->initialPosition;
			pCurGO->vel = glm::vec3( 0.0f );
		}
		else
		{	// Move as normal
			pCurGO->position += deltaPosition;
		}

		// New velocity is based on acceleration over time
		glm::vec3 deltaVelocity = ( ( float )deltaTime * pCurGO->accel )
			+ ( ( float )deltaTime * GRAVITY );

		pCurGO->vel += deltaVelocity;

		// Move the cue to the white ball when there's no movement
		// Check to see if the object is moving
		if( pCurGO->vel != glm::vec3( 0.0f, 0.0f, 0.0f ) )
		{	// is in movement
			g_isThereMovement = true;
		}
		
		// HACK: Collision step
		switch ( pCurGO->typeOfObject )
		{
		case eTypeOfObject::SPHERE:
			//	// Compare this to EVERY OTHER object in the scene
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
					if ( PenetrationTestSphereSphere( pCurGO, pOtherObject, deltaTime ) )
					{
						// Return both spheres to previous position before the impact
						if ( pCurGO->prevPosition != glm::vec3( NULL ) )
							pCurGO->position = pCurGO->prevPosition;

						if( pOtherObject->prevPosition != glm::vec3( NULL ) )
							pOtherObject->position = pOtherObject->prevPosition;

						// Call the reaction on the Spheres
						bounceSpheres( pCurGO, pOtherObject );
					}

					break;

				case eTypeOfObject::PLANE:
					
					// HACK TO CHECK ONLY AGAINST THE TABLE SIDES OBJECT
					if( pOtherObject->physicsMeshName == "physics_poolsides" )
					{ // It's the sides
						for( int i_point = 0; i_point != ::g_vecPoints.size(); i_point++ )
						{	// Check if any point is in contact with the pCurGO

							// TODO Implement the collision with Y axis as well, this would be the logic
							// The Pythagorean distance 
							//float distance = glm::distance( pCurGO->position, ::g_vecPoints[i_point] );
							//if( distance <= pCurGO->radius )
							
							// RIGHT NOW we're only testing X and Z
							float distanceX = 0.0f;
							float distanceZ = 0.0f;

							if( pCurGO->position.x > g_vecPoints[i_point].point.x )
							{
								distanceX = pCurGO->position.x - g_vecPoints[i_point].point.x;
							}
							else
							{
								distanceX = g_vecPoints[i_point].point.x - pCurGO->position.x;
							}
							
							if( pCurGO->position.z > g_vecPoints[i_point].point.z )
							{
								distanceZ = pCurGO->position.z - g_vecPoints[i_point].point.z;
							}
							else
							{
								distanceZ = g_vecPoints[i_point].point.z - pCurGO->position.z;
							}

							float tempRadius = pCurGO->radius; // *1.2;

							if ( ( tempRadius >= distanceX ) && ( tempRadius >= distanceZ ) )
							{	// COLLISION!!

								triangleNormal = returnNormal( g_vecPoints[i_point].triangle.vertex );

								// Return sphere to previous position before the impact
								if( pCurGO->prevPosition != glm::vec3( NULL ) )
									pCurGO->position = pCurGO->prevPosition;

								bounceSphereAgainstPlane( pCurGO, pOtherObject, triangleNormal );

								break;
							}
						}
					}

					break;
				}
			}

			// HACK to stop the balls, simulate the frictional force
			if (pCurGO->vel.x < 0.01f && pCurGO->vel.x > -0.01f)
			{   // Check to see if the ball is (almost) stopped and stop it (otherwise it will never stop)
				pCurGO->vel.x = 0.0f;
			}

			// If it's moving, apply the frictional force
			if (pCurGO->vel.x < 0.0f)
			{
				pCurGO->vel.x += g_FRICTION_FORCE;
			}
			else if (pCurGO->vel.x > 0.0f)
			{
				pCurGO->vel.x -= g_FRICTION_FORCE;
			}

			if (pCurGO->vel.z < 0.01f && pCurGO->vel.z > -0.01f)
			{
				pCurGO->vel.z = 0.0f;
			}

			if (pCurGO->vel.z < 0.0f)
			{
				pCurGO->vel.z += g_FRICTION_FORCE;
			}
			else if (pCurGO->vel.z > 0.0f)
			{
				pCurGO->vel.z -= g_FRICTION_FORCE;
			}
						

			// HACK
			const float SURFACEOFGROUND = 0.0f;
			const float RIGHTSIDEWALL   = 18.0f;
			const float LEFTSIDEWALL    = -18.0f;
			const float BACKSIDEWALL    = -9.0f;
			const float FRONTSIDEWALL   = 9.0f;
			 
			//Sphere-Plane detection

			// HACK (Ground is table (game object 0)
			if ( ( pCurGO->position.y - pCurGO->radius ) <= ::g_vecGameObjects[0]->position.z)
			{	// Object has "hit" the ground 
				pCurGO->vel.y = +( fabs( pCurGO->vel.y ) );
			}

			//if ( ( pCurGO->position.x + pCurGO->radius ) >= RIGHTSIDEWALL )
			//{	// Object too far to the right
			//	// Object has penetrated the right plane
			//	pCurGO->vel.x = -( fabs( pCurGO->vel.x ) );
			//}
			//if ( ( pCurGO->position.x - pCurGO->radius ) <= LEFTSIDEWALL )
			//{	// Object too far to the left
			//	// Object has penetrated the left plane
			//	pCurGO->vel.x = +( fabs( pCurGO->vel.x ) );
			//}
			//if ((pCurGO->position.z - pCurGO->radius) <= BACKSIDEWALL)
			//{	// Object too far to the back
			//	// Object has penetrated the back plane
			//	pCurGO->vel.z = +(fabs(pCurGO->vel.z));
			//}
			//if( ( pCurGO->position.z + pCurGO->radius ) >= FRONTSIDEWALL )
			//{	// Object too far to the front
			//	// Object has penetrated the front plane
			//	pCurGO->vel.z = -( fabs( pCurGO->vel.z ) );
			//}
			break;
		};
	}//for ( int index...
	
	// If there's a Cue object, AND there's no movement on objects
	// set the cue position the same as the white ball
	if( g_pTheCueGO != NULL )
	{
		if( !g_isThereMovement )
		{ // All balls are static, so we draw the cue next to the white ball
		  // HACK, set position of cue depending on White ball position
			g_pTheCueGO->position = ::g_vecGameObjects[2]->position;	// [2] ITS THE WHITE BALL! (Hardcoded, for now)
		}
		else
		{ // At least one ball is in movement so we "hide away" the cue
			g_pTheCueGO->position = glm::vec3( 0.0f, 100.0f, 0.0f );
		}
	}

	return;
}

// Will draw a ball at this location with this colour
// Use the same object as the white ball (or draw a copy of it)
void DrawDebugSphere( glm::vec3 location, glm::vec4 colour,
	float scale )
{
	// TODO: the magic
	glm::vec3 oldPosition = ::g_pTheDebugSphere->position;
	glm::vec4 oldDiffuse = ::g_pTheDebugSphere->diffuseColour;
	bool bOldIsWireFrame = ::g_pTheDebugSphere->bIsWireFrame;

	::g_pTheDebugSphere->position = location;
	::g_pTheDebugSphere->diffuseColour = colour;
	::g_pTheDebugSphere->bIsWireFrame = true;
	::g_pTheDebugSphere->scale = scale;

	DrawObject( ::g_pTheDebugSphere );

	::g_pTheDebugSphere->position = oldPosition;
	::g_pTheDebugSphere->diffuseColour = oldDiffuse;
	::g_pTheDebugSphere->bIsWireFrame = bOldIsWireFrame;

	return;
}

// Draw a single object
void DrawObject( cGameObject* pTheGO )
{
	// Is there a game object? 
	if( pTheGO == 0 )	//if ( ::g_GameObjects[index] == 0 )
	{	// Nothing to draw
		return;		// Skip all for loop code and go to next
	}

	// Was near the draw call, but we need the mesh name
	std::string meshToDraw = pTheGO->meshName;		//::g_GameObjects[index]->meshName;

	sVAOInfo VAODrawInfo;
	if( ::g_pVAOManager->lookupVAOFromName( meshToDraw, VAODrawInfo ) == false )
	{	// Didn't find mesh
		return;
	}

	// Change "Light Objects" position based on light position
	// The game object sphere that "contains" the light follows the light
	if( pTheGO->bIsLight == true )
	{
		int lightIndex = pTheGO->myLight;
		pTheGO->position = ::g_pLightManager->vecLights[lightIndex].position;
		pTheGO->diffuseColour = glm::vec4( ::g_pLightManager->vecLights[lightIndex].diffuse, 1.0f );
	}

	// There IS something to draw

	glm::mat4x4 mModel = glm::mat4x4( 1.0f );	//		mat4x4_identity(m);

	glm::mat4 matRreRotZ = glm::mat4x4( 1.0f );
	matRreRotZ = glm::rotate( matRreRotZ, pTheGO->orientation.z,
								glm::vec3( 0.0f, 0.0f, 1.0f ) );
	mModel = mModel * matRreRotZ;

	glm::mat4 trans = glm::mat4x4( 1.0f );
	trans = glm::translate( trans, pTheGO->position );
	mModel = mModel * trans;

	glm::mat4 matPostRotZ = glm::mat4x4( 1.0f );
	matPostRotZ = glm::rotate( matPostRotZ, pTheGO->orientation2.z,
								glm::vec3( 0.0f, 0.0f, 1.0f ) );
	mModel = mModel * matPostRotZ;

	glm::mat4 matPostRotY = glm::mat4x4( 1.0f );
	matPostRotY = glm::rotate( matPostRotY, pTheGO->orientation2.y,
								glm::vec3( 0.0f, 1.0f, 0.0f ) );
	mModel = mModel * matPostRotY;

	glm::mat4 matPostRotX = glm::mat4x4( 1.0f );
	matPostRotX = glm::rotate( matPostRotX, pTheGO->orientation2.x,
								glm::vec3( 1.0f, 0.0f, 0.0f ) );
	mModel = mModel * matPostRotX;
	// TODO: add the other rotation matrix (i.e. duplicate code above)

	// assume that scale to unit bounding box
	// ************* BEWARE *****************
	//			float finalScale = VAODrawInfo.scaleForUnitBBox * ::g_vecGameObjects[index]->scale;
	// We have taken out the scale adjustment so the scale is AS IT IS FROM THE MODEL
	//float finalScale = pTheGO->scale;
	float finalScale = VAODrawInfo.scaleForUnitBBox * pTheGO->scale;

	glm::mat4 matScale = glm::mat4x4( 1.0f );
	matScale = glm::scale( matScale,
							glm::vec3( finalScale,
										finalScale,
										finalScale ) );
	mModel = mModel * matScale;

	glUniformMatrix4fv( uniLoc_mModel, 1, GL_FALSE,
						( const GLfloat* ) glm::value_ptr( mModel ) );
	
	glm::mat4 mWorldInTranpose = glm::inverse( glm::transpose( mModel ) );

	glUniform4f( uniLoc_materialDiffuse,
					pTheGO->diffuseColour.r,
					pTheGO->diffuseColour.g,
					pTheGO->diffuseColour.b,
					pTheGO->diffuseColour.a );
	//...and all the other object material colours

	if( pTheGO->bIsWireFrame )
	{
		glUniform1f( uniLoc_bIsDebugWireFrameObject, 1.0f );	// TRUE
	}
	else
	{
		glUniform1f( uniLoc_bIsDebugWireFrameObject, 0.0f );	// FALSE
	}


	if( pTheGO->bIsWireFrame )
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );	// Default
		//glEnable(GL_DEPTH_TEST);		// Test for z and store in z buffer
		glDisable( GL_CULL_FACE );
	}
	else
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );	// Default
		glEnable( GL_DEPTH_TEST );		// Test for z and store in z buffer
		glEnable( GL_CULL_FACE );
	}

	glCullFace( GL_BACK );


	glBindVertexArray( VAODrawInfo.VAO_ID );

	glDrawElements( GL_TRIANGLES,
		VAODrawInfo.numberOfIndices,		// testMesh.numberOfTriangles * 3,	// How many vertex indices
		GL_UNSIGNED_INT,					// 32 bit int 
		0 );
	// Unbind that VAO
	glBindVertexArray( 0 );

	return;
}

glm::vec3 returnNormal( glm::vec3 vertex[3] ){
	glm::vec3 vertA = vertex[0];
	glm::vec3 vertB = vertex[1];
	glm::vec3 vertC = vertex[2];
	
	glm::vec3 theNormal = glm::vec3( 0.0f );

	glm::vec3 vectorU = ( vertB - vertA );
	glm::vec3 vectorV = ( vertC - vertA );

	theNormal.x = ( vectorU.y * vectorV.z ) - ( vectorU.z * vectorV.y );
	theNormal.y = ( vectorU.z * vectorV.x ) - ( vectorU.x * vectorV.z );
	theNormal.z = ( vectorU.x * vectorV.y ) - ( vectorU.y * vectorV.x );

	return theNormal;
}