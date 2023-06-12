/******************************************************************************
 *
 * Computer Graphics Programming 2020 Project Template v1.0 (11/04/2021)
 *
 * Based on: Animation Controller v1.0 (11/04/2021)
 *
 * This template provides a basic FPS-limited render loop for an animated scene,
 * plus keyboard handling for smooth game-like control of an object such as a
 * character or vehicle.
 *
 * A simple static lighting setup is provided via initLights(), which is not
 * included in the animationalcontrol.c template. There are no other changes.
 *
 ******************************************************************************/
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <freeglut.h>
#include <math.h>
#include <stdio.h>

 /******************************************************************************
  * Animation & Timing Setup
  ******************************************************************************/

  // Target frame rate (number of Frames Per Second).
#define TARGET_FPS 60				

// Ideal time each frame should be displayed for (in milliseconds).
const unsigned int FRAME_TIME = 1000 / TARGET_FPS;

// Frame time in fractional seconds.
// Note: This is calculated to accurately reflect the truncated integer value of
// FRAME_TIME, which is used for timing, rather than the more accurate fractional
// value we'd get if we simply calculated "FRAME_TIME_SEC = 1.0f / TARGET_FPS".
const float FRAME_TIME_SEC = (1000 / TARGET_FPS) / 1000.0f;

// Time we started preparing the current frame (in milliseconds since GLUT was initialized).
unsigned int frameStartTime = 0;

/******************************************************************************
 * Some Simple Definitions of Motion
 ******************************************************************************/

#define MOTION_NONE 0				// No motion.
#define MOTION_CLOCKWISE -1			// Clockwise rotation.
#define MOTION_ANTICLOCKWISE 1		// Anticlockwise rotation.
#define MOTION_BACKWARD -1			// Backward motion.
#define MOTION_FORWARD 1			// Forward motion.
#define MOTION_LEFT -1				// Leftward motion.
#define MOTION_RIGHT 1				// Rightward motion.
#define MOTION_DOWN -1				// Downward motion.
#define MOTION_UP 1					// Upward motion.

 // Represents the motion of an object on four axes (Yaw, Surge, Sway, and Heave).
 // 
 // You can use any numeric values, as specified in the comments for each axis. However,
 // the MOTION_ definitions offer an easy way to define a "unit" movement without using
 // magic numbers (e.g. instead of setting Surge = 1, you can set Surge = MOTION_FORWARD).
 //
typedef struct {
	int Yaw;		// Turn about the Z axis	[<0 = Clockwise, 0 = Stop, >0 = Anticlockwise]
	int Surge;		// Move forward or back		[<0 = Backward,	0 = Stop, >0 = Forward]
	int Sway;		// Move sideways (strafe)	[<0 = Left, 0 = Stop, >0 = Right]
	int Heave;		// Move vertically			[<0 = Down, 0 = Stop, >0 = Up]
} motionstate4_t;

/******************************************************************************
 * Keyboard Input Handling Setup
 ******************************************************************************/

 // Represents the state of a single keyboard key.Represents the state of a single keyboard key.
typedef enum {
	KEYSTATE_UP = 0,	// Key is not pressed.
	KEYSTATE_DOWN		// Key is pressed down.
} keystate_t;

// Represents the states of a set of keys used to control an object's motion.
typedef struct {
	keystate_t MoveForward;
	keystate_t MoveBackward;
	keystate_t MoveLeft;
	keystate_t MoveRight;
	keystate_t MoveUp;
	keystate_t MoveDown;
	keystate_t TurnLeft;
	keystate_t TurnRight;
} motionkeys_t;

// Current state of all keys used to control our "player-controlled" object's motion.
motionkeys_t motionKeyStates = {
	KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP,
	KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP };

// How our "player-controlled" object should currently be moving, solely based on keyboard input.
//
// Note: this may not represent the actual motion of our object, which could be subject to
// other controls (e.g. mouse input) or other simulated forces (e.g. gravity).
motionstate4_t keyboardMotion = { MOTION_NONE, MOTION_NONE, MOTION_NONE, MOTION_NONE };

// Define all character keys used for input (add any new key definitions here).
// Note: USE ONLY LOWERCASE CHARACTERS HERE. The keyboard handler provided converts all
// characters typed by the user to lowercase, so the SHIFT key is ignored.

#define KEY_MOVE_FORWARD				'w'
#define KEY_MOVE_BACKWARD				's'
#define KEY_MOVE_LEFT					'a'
#define KEY_MOVE_RIGHT					'd'
#define KEY_RENDER_FILL					'l'
#define KEY_EXIT						27 // Escape key.
#define DEBUG_CAMERA					'='
#define DEBUG_CAMERA_DEFAULT			'1'
#define DEBUG_CAMERA_FRONT				'2'
#define DEBUG_CAMERA_TOP				'3'
#define DEBUG_CAMERA_LOW				'4'
#define DEBUG_CAMERA_DEFAULT_ZOOM_OUT	'5'
#define SPOTLIGHT_TOGGLE				't'

// Define all GLUT special keys used for input (add any new key definitions here).

#define SP_KEY_MOVE_UP		GLUT_KEY_UP
#define SP_KEY_MOVE_DOWN	GLUT_KEY_DOWN
#define SP_KEY_TURN_LEFT	GLUT_KEY_LEFT
#define SP_KEY_TURN_RIGHT	GLUT_KEY_RIGHT

/******************************************************************************
 * GLUT Callback Prototypes
 ******************************************************************************/

void display(void);
void reshape(int width, int h);
void keyPressed(unsigned char key, int x, int y);
void specialKeyPressed(int key, int x, int y);
void keyReleased(unsigned char key, int x, int y);
void specialKeyReleased(int key, int x, int y);
void idle(void);

/******************************************************************************
 * Mesh Object Loader Setup and Prototypes
 ******************************************************************************/

typedef struct {
	GLfloat x;
	GLfloat y;
	GLfloat z;
} vec3d;

typedef struct {
	GLfloat x;
	GLfloat y;
} vec2d;

typedef struct {
	int vertexIndex;	// Index of this vertex in the object's vertices array
	int texCoordIndex; // Index of the texture coordinate for this vertex in the object's texCoords array
	int normalIndex;	// Index of the normal for this vertex in the object's normals array
} meshObjectFacePoint;

typedef struct {
	int pointCount;
	meshObjectFacePoint* points;
} meshObjectFace;

typedef struct {
	int vertexCount;
	vec3d* vertices;
	int texCoordCount;
	vec2d* texCoords;
	int normalCount;
	vec3d* normals;
	int faceCount;
	meshObjectFace* faces;
} meshObject;

meshObject* loadMeshObject(char* fileName);
void renderMeshObject(meshObject* object);
void initMeshObjectFace(meshObjectFace* face, char* faceData, int faceDataLength);
void freeMeshObject(meshObject* object);

/******************************************************************************
 * Animation-Specific Function Prototypes (add your own here)
 ******************************************************************************/

void main(int argc, char** argv);
void init(void);
void think(void);
void initLights(void);

// grid function
void drawGrid(void);

// border
void drawSkyBorder(void);
void borderCollision(void);

// helipad
void drawHelipad(void);

// hierarchical model functions to position and scale parts for helicopter
void drawHelicopter(void);
void drawSkidConnector(enum Side side);
void drawSkid(enum Side side);
void drawSkidEnding(enum Side xSide, enum Side zSide);
void drawWindshield(void);
void drawTopRotors(void);
void drawBlade(int num);
void drawTail(void);
void drawTailRotors(void);
void drawTailFin(void);
void drawSpotLight(void);

// hierarchical model functions to position and scale parts for boat
void drawBoat(void);
void drawBoatBase(void);
void drawBoatCabin(void);

// move boat
void moveBoat(void);

// dock
void drawDock(void);
void drawPlank(int num);
void drawLamp(void);

// camera
void updateCameraPos(void);

// spotlight
void setupSpotlight(void);



/******************************************************************************
 * Animation-Specific Setup (Add your own definitions, constants, and globals here)
 ******************************************************************************/

 // Render objects as filled polygons (1) or wireframes (0). Default filled.
int renderFillEnabled = 1;

//is the object to be drawn on the left (-x) or right (-y)
enum Side {
	leftSide = -1,
	rightSide = 1,
	frontSide = 1,
	backSide = -1,
};

// down
const float downForward[] = { 0.0f, -1.0f, -1.0f };
const float down[] = { 0.0f, -1.0f, 0.0f };

// window dimensions
GLint windowWidth = 800;
GLint windowHeight = 600;

// current camera position
GLfloat cameraPosition[] = { 0.0f, 5.0f, 12.0f };
float cameraOffset[] = { 0.0f, 7.5f, 0.0f };
// camera debug
int debug = 0;

// pointer to quadric objects
GLUquadricObj* sphereQuadric;
GLUquadricObj* cylinderQuadric;

// textures
typedef struct {
	int width;
	int height;
	GLubyte* data;
} PPMImage;

PPMImage loadPPM(char* fileName);
GLuint loadOBJPPM(char* filename);

PPMImage water;
PPMImage grass;

GLuint waterId;
GLuint grassId;

// rotor blade speed management
float rotorSpeed = 750.0f;
float rotorAngle = 1.0f;

// border values
#define WORLD_RADIUS 50.0f
#define SKY_HEIGHT 70.0f

// hierachical model setup values

// helicopter
// body
#define HELICOPTER_BODY_RADIUS 2.0f

// skid connectors
#define SKID_CONNECTOR_RADIUS HELICOPTER_BODY_RADIUS / 10.0f
#define SKID_CONNECTOR_LENGTH HELICOPTER_BODY_RADIUS * 0.8f

// skids
#define SKID_RADIUS HELICOPTER_BODY_RADIUS / 10.0f
#define SKID_LENGTH HELICOPTER_BODY_RADIUS * 3.0f

// skid endings
#define SKID_ENDING_RADIUS SKID_RADIUS

// wind shield
#define WINDSHIELD_RADIUS 0.75f
#define WINDSHIELD_LENGTH 1.5f

// rotor speed variables
#define ROTOR_MAX_SPEED 750.0f
#define ROTOR_ACCELRATION 100.0f

// top rotors
#define ROTOR_CUBE_SIZE 0.8f
#define ROTOR_BLADE_SIZE 10.0f
#define ROTOR_NUMBER_OF_BLADES 4

// tail
#define TAIL_BASE 1.0f
#define TAIL_LENGTH 6.5f
#define TAIL_TIP_RADIUS 0.25f
#define TAIL_ROTOR_SCALE_FACTOR 0.25f

// grid
#define GRID_SQUARE_SIZE 1.0f
#define GRID_SIZE 100.0f

// boat
// base
#define BOAT_BASE_SIZE 7.5f
#define BOAT_CABIN_SIZE 3.0f

// dock
#define DOCK_PLANK_SIZE 12.5f
// lamp
#define LAMP_POST_SIZE 6.0f
#define LAMP_CONNECTOR_SIZE 1.0f
#define LAMP_BULB_SIZE 0.25f

#define PI 3.1415f

// camera distance
#define CAMERA_DISTANCE 15.0f

// initial y value for the helicopter centre 
#define START_HEIGHT HELICOPTER_BODY_RADIUS + SKID_CONNECTOR_LENGTH + SKID_RADIUS

// colours
const GLfloat PALE_GREEN[4] = { 0.596f, 0.984f, 0.596f };
const GLfloat GREY[4] = { 0.3f, 0.3f, 0.3f };
const GLfloat BROWN[4] = { 0.545f, 0.27f, 0.0745f };
const GLfloat POLICE_BLUE[4] = { 0.0f, 0.0f, 0.40f };
const GLfloat LIGHT_CYAN[4] = { 0.58f, 1.0f, 1.0f };
const GLfloat WHITE[4] = { 1.0f, 1.0f, 1.0f };
const GLfloat RED[4] = { 1.0f, 0.0f, 0.0f };
const GLfloat BLUE[4] = { 0.0f, 0.0f, 1.0f };
const GLfloat YELLOW[4] = { 1.0f, 1.0f, 0.0f };
const GLfloat BLACK[4] = { 0.0f, 0.0f, 0.0f };

// materials
const GLfloat zeroMaterial[4] = { 0.0, 0.0, 0.0, 1.0 };

const GLfloat paleGreenDiffuse[4] = { 0.596f, 0.984f, 0.596f, 1.0f };
const GLfloat greyDiffuse[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
const GLfloat brownDiffuse[4] = { 0.545f, 0.27f, 0.0745f, 1.0f };
const GLfloat policeBlueDiffuse[4] = { 0.0f, 0.0f, 0.40f, 1.0f };
const GLfloat lightCyanDiffuse[4] = { 0.58f, 1.0f, 1.0f, 1.0f };
const GLfloat whiteDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat redDiffuse[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
const GLfloat blueDiffuse[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
const GLfloat yellowDiffuse[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
const GLfloat blackDiffuse[4] = { 0.0f, 0.0f, 0.0f, 1.0f };


// the degrees of shinnines
GLfloat noShininess = 0.0;
GLfloat highShininess = 100.0;

// model animation variables (position, heading, speed (metres per second)) for the helicopter
float helicopterLocation[] = { GRID_SIZE / 2 * 0.5, START_HEIGHT, -GRID_SIZE / 2 * 0.5 }; // X, Y, Z
float helicopterFacing = 0.0f;
const float helicopterMoveSpeed = 10.0f;

// model animation variables (position, heading, speed (metres per second)) for the boat
float boatLocation[] = { GRID_SIZE / 2 * 0.4, -0.25f, GRID_SIZE / 2 * 0.4 };
float boatFacing = 0.0f;
const float boatMoveSpeed = 5.0f;

// lamp
GLfloat lampLightPosition[] = { 0.0, 4.0, 0.0, 1.0 };

// Test mesh object
meshObject* treeMesh;
GLuint tree;

/******************************************************************************
 * Entry Point (don't put anything except the main function here)
 ******************************************************************************/

void main(int argc, char** argv)
{
	// Initialize the OpenGL window.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutCreateWindow("Animation");

	// Set up the scene.
	init();

	// Disable key repeat (keyPressed or specialKeyPressed will only be called once when a key is first pressed).
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	// Register GLUT callbacks.
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyPressed);
	glutSpecialFunc(specialKeyPressed);
	glutKeyboardUpFunc(keyReleased);
	glutSpecialUpFunc(specialKeyReleased);
	glutIdleFunc(idle);

	// Record when we started rendering the very first frame (which should happen after we call glutMainLoop).
	frameStartTime = (unsigned int)glutGet(GLUT_ELAPSED_TIME);

	// Enter the main drawing loop (this will never return).
	glutMainLoop();
}

/******************************************************************************
 * GLUT Callbacks (don't add any other functions here)
 ******************************************************************************/

 /*
	 Called when GLUT wants us to (re)draw the current animation frame.

	 Note: This function must not do anything to update the state of our simulated
	 world. Animation (moving or rotating things, responding to keyboard input,
	 etc.) should only be performed within the think() function provided below.
 */
void display(void)
{
	/*
		TEMPLATE: REPLACE THIS COMMENT WITH YOUR DRAWING CODE

		Separate reusable pieces of drawing code into functions, which you can add
		to the "Animation-Specific Functions" section below.

		Remember to add prototypes for any new functions to the "Animation-Specific
		Function Prototypes" section near the top of this template.
	*/

	// clear the screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// load the identity matrix into the model view matrix
	glLoadIdentity();

	//set up our camera - slightly up in the y so we can see the ground plane
	gluLookAt(cameraPosition[0], cameraPosition[1], cameraPosition[2],
		helicopterLocation[0], helicopterLocation[1], helicopterLocation[2],
		0, 1, 0);

	// draw the ground
	drawGrid();

	// draw the border
	drawSkyBorder();

	// draw helipad
	drawHelipad();

	// draw helicopter
	drawHelicopter();

	// draw the boat
	drawBoat();

	// draw the dock and lamp();
	drawDock();

	//textured object

	glMaterialfv(GL_FRONT, GL_DIFFUSE, whiteDiffuse);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tree);
	renderMeshObject(treeMesh);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);

	// swap the drawing buffers
	glutSwapBuffers();

}

/*
	Called when the OpenGL window has been resized.
*/
void reshape(int width, int h)
{
	windowHeight = h;
	windowWidth = width;

	glViewport(0, 0, windowWidth, windowHeight);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(60, (float)windowWidth / (float)windowHeight, 1.0, 500.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/*
	Called each time a character key (e.g. a letter, number, or symbol) is pressed.
*/
void keyPressed(unsigned char key, int x, int y)
{
	switch (tolower(key)) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			Whenever one of our movement keys is pressed, we do two things:
			(1) Update motionKeyStates to record that the key is held down. We use
				this later in the keyReleased callback.
			(2) Update the relevant axis in keyboardMotion to set the new direction
				we should be moving in. The most recent key always "wins" (e.g. if
				you're holding down KEY_MOVE_LEFT then also pressed KEY_MOVE_RIGHT,
				our object will immediately start moving right).
		*/
	case KEY_MOVE_FORWARD:
		motionKeyStates.MoveForward = KEYSTATE_DOWN;
		keyboardMotion.Surge = MOTION_FORWARD;
		break;
	case KEY_MOVE_BACKWARD:
		motionKeyStates.MoveBackward = KEYSTATE_DOWN;
		keyboardMotion.Surge = MOTION_BACKWARD;
		break;
	case KEY_MOVE_LEFT:
		motionKeyStates.MoveLeft = KEYSTATE_DOWN;
		keyboardMotion.Sway = MOTION_LEFT;
		break;
	case KEY_MOVE_RIGHT:
		motionKeyStates.MoveRight = KEYSTATE_DOWN;
		keyboardMotion.Sway = MOTION_RIGHT;
		break;

		/*
			Other Keyboard Functions (add any new character key controls here)

			Rather than using literals (e.g. "t" for spotlight), create a new KEY_
			definition in the "Keyboard Input Handling Setup" section of this file.
			For example, refer to the existing keys used here (KEY_MOVE_FORWARD,
			KEY_MOVE_LEFT, KEY_EXIT, etc).
		*/
	case KEY_RENDER_FILL:
		renderFillEnabled = !renderFillEnabled;
		break;
	case KEY_EXIT:
		gluDeleteQuadric(sphereQuadric);
		gluDeleteQuadric(cylinderQuadric);
		exit(0);
		break;

		// debug camera options used primarily for inspecting the helicopter from various angles
	case DEBUG_CAMERA:
		debug = debug ? 0 : 1;
		if (debug)
		{
			cameraOffset[0] = 0.0f;
			cameraOffset[1] = 5.0f;
			cameraOffset[2] = 0.0f;
			cameraPosition[0] = 0.0f + cameraOffset[0];
			cameraPosition[1] = helicopterLocation[1] + cameraOffset[1]; //track bird on heave only
			cameraPosition[2] = 12.0f + cameraOffset[2];
		}
		break;
	case DEBUG_CAMERA_DEFAULT:
		cameraOffset[1] = 5.0f;
		cameraOffset[2] = 0.0f;
		break;
	case DEBUG_CAMERA_FRONT:
		cameraOffset[1] = 0.0f;
		cameraOffset[2] = 0.0f;
		break;
	case DEBUG_CAMERA_TOP:
		cameraOffset[1] = 20.0f;
		cameraOffset[2] = -11.99f;
		break;
	case DEBUG_CAMERA_LOW:
		cameraOffset[1] = -4.0f;
		cameraOffset[2] = 0.0f;
		break;
	case DEBUG_CAMERA_DEFAULT_ZOOM_OUT:
		cameraOffset[1] = 5.0f;
		cameraOffset[2] = 5.0f;
		break;
	case SPOTLIGHT_TOGGLE:
		glIsEnabled(GL_LIGHT1) ? glDisable(GL_LIGHT1) : glEnable(GL_LIGHT1);
	}
}

/*
	Called each time a "special" key (e.g. an arrow key) is pressed.
*/
void specialKeyPressed(int key, int x, int y)
{
	switch (key) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			This works as per the motion keys in keyPressed.
		*/
	case SP_KEY_MOVE_UP:
		motionKeyStates.MoveUp = KEYSTATE_DOWN;
		keyboardMotion.Heave = MOTION_UP;
		break;
	case SP_KEY_MOVE_DOWN:
		motionKeyStates.MoveDown = KEYSTATE_DOWN;
		keyboardMotion.Heave = MOTION_DOWN;
		break;
	case SP_KEY_TURN_LEFT:
		motionKeyStates.TurnLeft = KEYSTATE_DOWN;
		keyboardMotion.Yaw = MOTION_ANTICLOCKWISE;
		break;
	case SP_KEY_TURN_RIGHT:
		motionKeyStates.TurnRight = KEYSTATE_DOWN;
		keyboardMotion.Yaw = MOTION_CLOCKWISE;
		break;

		/*
			Other Keyboard Functions (add any new special key controls here)

			Rather than directly using the GLUT constants (e.g. GLUT_KEY_F1), create
			a new SP_KEY_ definition in the "Keyboard Input Handling Setup" section of
			this file. For example, refer to the existing keys used here (SP_KEY_MOVE_UP,
			SP_KEY_TURN_LEFT, etc).
		*/
	}
}

/*
	Called each time a character key (e.g. a letter, number, or symbol) is released.
*/
void keyReleased(unsigned char key, int x, int y)
{
	switch (tolower(key)) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			Whenever one of our movement keys is released, we do two things:
			(1) Update motionKeyStates to record that the key is no longer held down;
				we need to know when we get to step (2) below.
			(2) Update the relevant axis in keyboardMotion to set the new direction
				we should be moving in. This gets a little complicated to ensure
				the controls work smoothly. When the user releases a key that moves
				in one direction (e.g. KEY_MOVE_RIGHT), we check if its "opposite"
				key (e.g. KEY_MOVE_LEFT) is pressed down. If it is, we begin moving
				in that direction instead. Otherwise, we just stop moving.
		*/
	case KEY_MOVE_FORWARD:
		motionKeyStates.MoveForward = KEYSTATE_UP;
		keyboardMotion.Surge = (motionKeyStates.MoveBackward == KEYSTATE_DOWN) ? MOTION_BACKWARD : MOTION_NONE;
		break;
	case KEY_MOVE_BACKWARD:
		motionKeyStates.MoveBackward = KEYSTATE_UP;
		keyboardMotion.Surge = (motionKeyStates.MoveForward == KEYSTATE_DOWN) ? MOTION_FORWARD : MOTION_NONE;
		break;
	case KEY_MOVE_LEFT:
		motionKeyStates.MoveLeft = KEYSTATE_UP;
		keyboardMotion.Sway = (motionKeyStates.MoveRight == KEYSTATE_DOWN) ? MOTION_RIGHT : MOTION_NONE;
		break;
	case KEY_MOVE_RIGHT:
		motionKeyStates.MoveRight = KEYSTATE_UP;
		keyboardMotion.Sway = (motionKeyStates.MoveLeft == KEYSTATE_DOWN) ? MOTION_LEFT : MOTION_NONE;
		break;

		/*
			Other Keyboard Functions (add any new character key controls here)

			Note: If you only care when your key is first pressed down, you don't have to
			add anything here. You only need to put something in keyReleased if you care
			what happens when the user lets go, like we do with our movement keys above.
			For example: if you wanted a spotlight to come on while you held down "t", you
			would need to set a flag to turn the spotlight on in keyPressed, and update the
			flag to turn it off in keyReleased.
		*/
	}
}

/*
	Called each time a "special" key (e.g. an arrow key) is released.
*/
void specialKeyReleased(int key, int x, int y)
{
	switch (key) {
		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			This works as per the motion keys in keyReleased.
		*/
	case SP_KEY_MOVE_UP:
		motionKeyStates.MoveUp = KEYSTATE_UP;
		keyboardMotion.Heave = (motionKeyStates.MoveDown == KEYSTATE_DOWN) ? MOTION_DOWN : MOTION_NONE;
		break;
	case SP_KEY_MOVE_DOWN:
		motionKeyStates.MoveDown = KEYSTATE_UP;
		keyboardMotion.Heave = (motionKeyStates.MoveUp == KEYSTATE_DOWN) ? MOTION_UP : MOTION_NONE;
		break;
	case SP_KEY_TURN_LEFT:
		motionKeyStates.TurnLeft = KEYSTATE_UP;
		keyboardMotion.Yaw = (motionKeyStates.TurnRight == KEYSTATE_DOWN) ? MOTION_CLOCKWISE : MOTION_NONE;
		break;
	case SP_KEY_TURN_RIGHT:
		motionKeyStates.TurnRight = KEYSTATE_UP;
		keyboardMotion.Yaw = (motionKeyStates.TurnLeft == KEYSTATE_DOWN) ? MOTION_ANTICLOCKWISE : MOTION_NONE;
		break;

		/*
			Other Keyboard Functions (add any new special key controls here)

			As per keyReleased, you only need to handle the key here if you want something
			to happen when the user lets go. If you just want something to happen when the
			key is first pressed, add you code to specialKeyPressed instead.
		*/
	}
}

/*
	Called by GLUT when it's not rendering a frame.

	Note: We use this to handle animation and timing. You shouldn't need to modify
	this callback at all. Instead, place your animation logic (e.g. moving or rotating
	things) within the think() method provided with this template.
*/
void idle(void)
{
	// Wait until it's time to render the next frame.

	unsigned int frameTimeElapsed = (unsigned int)glutGet(GLUT_ELAPSED_TIME) - frameStartTime;
	if (frameTimeElapsed < FRAME_TIME)
	{
		// This frame took less time to render than the ideal FRAME_TIME: we'll suspend this thread for the remaining time,
		// so we're not taking up the CPU until we need to render another frame.
		unsigned int timeLeft = FRAME_TIME - frameTimeElapsed;
		Sleep(timeLeft);
	}

	// Begin processing the next frame.

	frameStartTime = glutGet(GLUT_ELAPSED_TIME); // Record when we started work on the new frame.

	think(); // Update our simulated world before the next call to display().

	glutPostRedisplay(); // Tell OpenGL there's a new frame ready to be drawn.
}

/******************************************************************************
 * Animation-Specific Functions (Add your own functions at the end of this section)
 ******************************************************************************/

 /*
	 Initialise OpenGL and set up our scene before we begin the render loop.
 */
void init(void)
{
	// enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Anything that relies on lighting or specifies normals must be initialised after initLights.
	initLights();
	
	//Enable use of fog
	glEnable(GL_FOG);

	// define the color and density of the fog
	GLfloat fogColor[4] = { 0.2f, 0.2f, 0.2f, 0.2f };
	GLfloat fogDensity = 0.05f;
	// set the color of the fog
	glFogfv(GL_FOG_COLOR, fogColor);
	//set the fog mode to be exponential
	glFogf(GL_FOG_MODE, GL_EXP);
	//set the fog density
	glFogf(GL_FOG_DENSITY, fogDensity);

	//create the quadric for drawing the sphere
	sphereQuadric = gluNewQuadric();

	//create the quadric for drawing the cylinder
	cylinderQuadric = gluNewQuadric();

	//load assets
	treeMesh = loadMeshObject("tree.obj");

	tree = loadOBJPPM("P3grass.ppm");

	grass = loadPPM("P3grass.ppm");
	glGenTextures(1, &grassId);
	glBindTexture(GL_TEXTURE_2D, grassId);

	water = loadPPM("P3water.ppm");
	glGenTextures(1, &waterId);
	glBindTexture(GL_TEXTURE_2D, waterId);
}

/*
	Advance our animation by FRAME_TIME milliseconds.

	Note: Our template's GLUT idle() callback calls this once before each new
	frame is drawn, EXCEPT the very first frame drawn after our application
	starts. Any setup required before the first frame is drawn should be placed
	in init().
*/
void think(void)
{
	/*
		TEMPLATE: REPLACE THIS COMMENT WITH YOUR ANIMATION/SIMULATION CODE

		In this function, we update all the variables that control the animated
		parts of our simulated world. For example: if you have a moving box, this is
		where you update its coordinates to make it move. If you have something that
		spins around, here's where you update its angle.

		NOTHING CAN BE DRAWN IN HERE: you can only update the variables that control
		how everything will be drawn later in display().

		How much do we move or rotate things? Because we use a fixed frame rate, we
		assume there's always FRAME_TIME milliseconds between drawing each frame. So,
		every time think() is called, we need to work out how far things should have
		moved, rotated, or otherwise changed in that period of time.

		Movement example:
		* Let's assume a distance of 1.0 GL units is 1 metre.
		* Let's assume we want something to move 2 metres per second on the x axis
		* Each frame, we'd need to update its position like this:
			x += 2 * (FRAME_TIME / 1000.0f)
		* Note that we have to convert FRAME_TIME to seconds. We can skip this by
		  using a constant defined earlier in this template:
			x += 2 * FRAME_TIME_SEC;

		Rotation example:
		* Let's assume we want something to do one complete 360-degree rotation every
		  second (i.e. 60 Revolutions Per Minute, or RPM).
		* Each frame, we'd need to update our object's angle like this (we'll use the
		  FRAME_TIME_SEC constant as per the example above):
			a += 360 * FRAME_TIME_SEC;

		This works for any type of "per second" change: just multiply the amount you'd
		want to move in a full second by FRAME_TIME_SEC, and add or subtract that
		from whatever variable you're updating.

		You can use this same approach to animate other things like color, opacity,
		brightness of lights, etc.
	*/

	/*
		Keyboard motion handler: complete this section to make your "player-controlled"
		object respond to keyboard input.
	*/
	// checks that the rotors are at the appropriate speed
	if (rotorSpeed >= ROTOR_MAX_SPEED) {
		if (keyboardMotion.Yaw != MOTION_NONE) {
			/* TEMPLATE: Turn your object right (clockwise) if .Yaw < 0, or left (anticlockwise) if .Yaw > 0 */
			helicopterFacing += 90.0f * FRAME_TIME_SEC * keyboardMotion.Yaw; //90 RPM
		}
		if (keyboardMotion.Surge != MOTION_NONE) {
			/* TEMPLATE: Move your object backward if .Surge < 0, or forward if .Surge > 0 */
			float xMove = sinf(helicopterFacing * (PI / 180)) * helicopterMoveSpeed;
			float zMove = cosf(helicopterFacing * (PI / 180)) * helicopterMoveSpeed;

			helicopterLocation[0] += xMove * FRAME_TIME_SEC * keyboardMotion.Surge;
			helicopterLocation[2] += zMove * FRAME_TIME_SEC * keyboardMotion.Surge;
		}
		if (keyboardMotion.Sway != MOTION_NONE) {
			/* TEMPLATE: Move (strafe) your object left if .Sway < 0, or right if .Sway > 0 */
			float xMove = sinf((helicopterFacing + 90.0f) * (PI / 180)) * helicopterMoveSpeed;
			float zMove = cosf((helicopterFacing + 90.0f) * (PI / 180)) * helicopterMoveSpeed;

			helicopterLocation[0] -= xMove * FRAME_TIME_SEC * keyboardMotion.Sway;
			helicopterLocation[2] -= zMove * FRAME_TIME_SEC * keyboardMotion.Sway;
		}
		if (keyboardMotion.Heave != MOTION_NONE) {
			/* TEMPLATE: Move your object down if .Heave < 0, or up if .Heave > 0 */
			// stops the helicopter from moving below the grid
			if (helicopterLocation[1] > START_HEIGHT) {
				if (helicopterLocation[1] < SKY_HEIGHT)
					helicopterLocation[1] += keyboardMotion.Heave * helicopterMoveSpeed / 2 * FRAME_TIME_SEC;
				else if (keyboardMotion.Heave < 0)
					helicopterLocation[1] += keyboardMotion.Heave * helicopterMoveSpeed / 2 * FRAME_TIME_SEC;
			}
			else if (keyboardMotion.Heave > 0)
				helicopterLocation[1] += keyboardMotion.Heave * helicopterMoveSpeed / 2 * FRAME_TIME_SEC;
		}
	}
	else {
		rotorSpeed += ROTOR_ACCELRATION * FRAME_TIME_SEC;
	}

	GLfloat spotLightPosition[] = { helicopterLocation[0], helicopterLocation[1] - HELICOPTER_BODY_RADIUS, helicopterLocation[2], 1.0f };
	glLightfv(GL_LIGHT1, GL_POSITION, spotLightPosition);
	glLightfv(GL_LIGHT2, GL_POSITION, lampLightPosition);
	

	// I didn't like the idea of this number getting stupidly huge so I wanted to reset it to avoid bugs
	if (rotorAngle > 360.0f)
		rotorAngle = 0.0f;

	// rotor spin
	rotorAngle += rotorSpeed * FRAME_TIME_SEC;

	moveBoat();

	// make sure that the helicopter does not leave the world border
	borderCollision();

	// update the camera position to follow the helicopter
	updateCameraPos();
}

/*
	Initialise OpenGL lighting before we begin the render loop.

	Note (advanced): If you're using dynamic lighting (e.g. lights that move around, turn on or
	off, or change colour) you may want to replace this with a drawLights function that gets called
	at the beginning of display() instead of init().
*/
void initLights(void)
{
	// Simple lighting setup
	GLfloat globalAmbient[] = { 0.4f, 0.4f, 0.4f, 1 };
	GLfloat lightPosition[] = { 100.0f, 100.0f, 100.0f, 1.0f };
	GLfloat ambientLight[] = { 0, 0, 0, 1 };
	GLfloat diffuseLight[] = { 0.5f, 0.5f, 0.5f, 1 };
	GLfloat specularLight[] = { 1, 1, 1, 1 };

	// Set up light parameters
	GLfloat spotLight[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat lampLight[] = { 1.0, 1.0, 1.0, 1.0 };

	float spotLightExponent = 5.0f;
	float spotLightCutoff = 60.0f;

	float lampLightExponent = 5.0f;
	float lampLightCutoff = 20.0f;

	// Configure global ambient lighting.
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

	// Configure Light 0.
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

	// Configure Light 1 (spotlight).
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, spotLight);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specularLight);

	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, downForward);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, spotLightExponent);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, spotLightCutoff);

	// Configure Light 2 (dock lamp).
	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT2, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, lampLight);
	glLightfv(GL_LIGHT2, GL_SPECULAR, specularLight);

	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, down);
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, lampLightExponent);
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, lampLightCutoff);

	// Enable lighting (GL_LIGHT1 can be toggled with the 't' key)
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);	

	// Make GL normalize the normal vectors we supply.
	glEnable(GL_NORMALIZE);
}

void initMeshObjectFace(meshObjectFace* face, char* faceData, int maxFaceDataLength) {
	int maxPoints = 0;
	int inWhitespace = 0;
	const char* delimiter = " ";
	char* token = NULL;
	char* context = NULL;

	// Do a quick scan of the input string to determine the maximum number of points in this face by counting
	// blocks of whitespace (each point must be preceded by at least one space). Note that we may end up with
	// fewer points than this if one or more prove to be invalid.
	for (int i = 0; i < maxFaceDataLength; i++)
	{
		char c = faceData[i];
		if (c == '\0') {
			break;
		}
		else if ((c == ' ') || (c == '\t')) {
			if (!inWhitespace) {
				inWhitespace = 1;
				maxPoints++;
			}
		}
		else {
			inWhitespace = 0;
		}
	}

	// Parse the input string to extract actual face points (if we're expecting any).
	face->pointCount = 0;
	if (maxPoints > 0) {
		face->points = malloc(sizeof(meshObjectFacePoint) * maxPoints);

		token = strtok_s(faceData, delimiter, &context);
		while ((token != NULL) && (face->pointCount < maxPoints)) {
			meshObjectFacePoint parsedPoint = { 0, 0, 0 }; // At this point we're working with 1-based indices from the OBJ file.

			if (strcmp(token, "f") != 0) {
				// Attempt to parse this face point in the format "v/t[/n]" (vertex, texture, and optional normal).
				if (sscanf_s(token, "%d/%d/%d", &parsedPoint.vertexIndex, &parsedPoint.texCoordIndex, &parsedPoint.normalIndex) < 2) {
					// That didn't work out: try parsing in the format "v[//n]" instead (vertex, no texture, and optional normal).
					sscanf_s(token, "%d//%d", &parsedPoint.vertexIndex, &parsedPoint.normalIndex);
				}

				// If we parsed a valid face point (i.e. one that at least contains the index of a vertex), add it.
				if (parsedPoint.vertexIndex > 0) {
					// Adjust all indices down by one: Wavefront OBJ uses 1-based indices, but our arrays are 0-based.
					parsedPoint.vertexIndex--;

					// Discard any negative texture coordinate or normal indices while adjusting them down by one.
					parsedPoint.texCoordIndex = (parsedPoint.texCoordIndex > 0) ? parsedPoint.texCoordIndex - 1 : -1;
					parsedPoint.normalIndex = (parsedPoint.normalIndex > 0) ? parsedPoint.normalIndex - 1 : -1;

					memcpy_s(&face->points[face->pointCount], sizeof(meshObjectFacePoint), &parsedPoint, sizeof(meshObjectFacePoint));
					face->pointCount++;
				}
			}

			token = strtok_s(NULL, delimiter, &context);
		}

		// If we have fewer points than expected, free the unused memory.
		if (face->pointCount == 0) {
			free(face->points);
			face->points = NULL;
		}
		else if (face->pointCount < maxPoints) {
			realloc(face->points, sizeof(meshObjectFacePoint) * face->pointCount);
		}
	}
	else
	{
		face->points = NULL;
	}
}

void updateCameraPos(void)
{
	// camera position if in debug mode
	if (debug) {
		cameraPosition[0] = 0.0f + cameraOffset[0];
		cameraPosition[1] = helicopterLocation[1] + cameraOffset[1]; //track bird on heave only
		cameraPosition[2] = 12.0f + cameraOffset[2];
	}
	// normal tracking camera 
	else
	{
		cameraPosition[0] = helicopterLocation[0] - sinf(helicopterFacing * (PI / 180)) * CAMERA_DISTANCE;
		cameraPosition[1] = helicopterLocation[1] + cameraOffset[1]; //track bird on heave only
		cameraPosition[2] = helicopterLocation[2] - cosf(helicopterFacing * (PI / 180)) * CAMERA_DISTANCE;
	}
}

void moveBoat(void)
{
	boatFacing += 50.0f * FRAME_TIME_SEC; // 50 RPM

	float xMove = sinf((boatFacing) * (PI / 180)) * boatMoveSpeed;
	float zMove = cosf((boatFacing) * (PI / 180)) * boatMoveSpeed;

	boatLocation[0] -= xMove * FRAME_TIME_SEC;
	boatLocation[2] -= zMove * FRAME_TIME_SEC;
}

PPMImage loadPPM(char* filename) // loads a PPM image
{
	// declare ppmimage
	PPMImage image;

	// the ID of the image file
	FILE* fileID;

	GLubyte* imageData;

	// width and height
	int width, height;

	// maxValue
	int maxValue;

	// total number of pixels in the image
	int totalPixels;

	// temporary character
	char tempChar;

	// counter variable for the current pixel in the image
	int i;

	// array for reading in header information
	char headerLine[100];

	// if the original values are larger than 255
	float RGBScaling;

	// temporary variables for reading in the red, green and blue data of each pixel
	int red, green, blue;

	// open the image file for reading - note this is hardcoded would be better to provide a parameter which
	// is the file name. There are 3 PPM files you can try out mount03, sky08 and sea02.
	if (fopen_s(&fileID, filename, "r") != 0) {
		printf("Failed to open the file.\n");
		exit(0);
	}

	// read in the first header line
	// - "%[^\n]"  matches a string of all characters not equal to the new line character ('\n')
	// - so we are just reading everything up to the first line break
	fscanf_s(fileID, "%[^\n] ", headerLine, sizeof(headerLine));

	// make sure that the image begins with 'P3', which signifies a PPM file
	if ((headerLine[0] != 'P') || (headerLine[1] != '3')) {
		printf("This is not a PPM file!\n %c %c", headerLine[0], headerLine[1]);
		exit(0);
	}

	// read in the first character of the next line
	fscanf_s(fileID, "%c", &tempChar, sizeof(tempChar));

	// while we still have comment lines (which begin with #)
	while (tempChar == '#') {
		// read in the comment
		fscanf_s(fileID, "%[^\n] ", headerLine, sizeof(headerLine));

		// read in the first character of the next line
		fscanf_s(fileID, "%c", &tempChar, sizeof(tempChar));
	}

	// the last one was not a comment character '#', so we need to put it back into the file stream (undo)
	ungetc(tempChar, fileID);

	// read in the image hieght, width and the maximum value
	fscanf_s(fileID, "%d %d %d", &width, &height, &maxValue);

	// compute the total number of pixels in the image
	totalPixels = width * height;

	// allocate enough memory for the image (3*) because of the RGB data
	imageData = (GLubyte*)malloc(3 * sizeof(GLuint) * totalPixels);

	// determine the scaling for RGB values
	RGBScaling = 255.0f / maxValue;

	// if the maxValue is 255 then we do not need to scale the
	// image data values to be in the range or 0 to 255
	if (maxValue == 255) {
		for (i = 0; i < totalPixels; i++) {
			// read in the current pixel from the file
			fscanf_s(fileID, "%d %d %d", &red, &green, &blue);

			// store the red, green and blue data of the current pixel in the data array
			imageData[3 * totalPixels - 3 * i - 3] = red;
			imageData[3 * totalPixels - 3 * i - 2] = green;
			imageData[3 * totalPixels - 3 * i - 1] = blue;
		}
	}
	else { // need to scale up the data values
		for (i = 0; i < totalPixels; i++) {
			// read in the current pixel from the file
			fscanf_s(fileID, "%d %d %d", &red, &green, &blue);

			// store the red, green and blue data of the current pixel in the data array
			imageData[3 * totalPixels - 3 * i - 3] = (GLubyte)red * (GLubyte)RGBScaling;
			imageData[3 * totalPixels - 3 * i - 2] = (GLubyte)green * (GLubyte)RGBScaling;
			imageData[3 * totalPixels - 3 * i - 1] = (GLubyte)blue * (GLubyte)RGBScaling;
		}
	}



	// close the image file
	fclose(fileID);

	// construct ppmimage
	image.width = width;
	image.height = height;
	image.data = imageData;

	return image;
}

GLuint loadOBJPPM(char* filename)
{
	FILE* inFile; //File pointer
	int width, height, maxVal; //image metadata from PPM file format
	int totalPixels; // total number of pixels in the image

	// temporary character
	char tempChar;
	// counter variable for the current pixel in the image
	int i;

	char header[100]; //input buffer for reading in the file header information

	// if the original values are larger than 255
	float RGBScaling;

	// temporary variables for reading in the red, green and blue data of each pixel
	int red, green, blue;

	GLubyte* texture; //the texture buffer pointer

	//create one texture with the next available index
	GLuint textureID;
	glGenTextures(1, &textureID);

	inFile = fopen(filename, "r");

	// read in the first header line
	//    - "%[^\n]"  matches a string of all characters not equal to the new line character ('\n')
	//    - so we are just reading everything up to the first line break
	fscanf(inFile, "%[^\n] ", header);

	// make sure that the image begins with 'P3', which signifies a PPM file
	if ((header[0] != 'P') || (header[1] != '3'))
	{
		printf("This is not a PPM file!\n");
		exit(0);
	}

	// we have a PPM file
	printf("This is a PPM file\n");

	// read in the first character of the next line
	fscanf(inFile, "%c", &tempChar);

	// while we still have comment lines (which begin with #)
	while (tempChar == '#')
	{
		// read in the comment
		fscanf(inFile, "%[^\n] ", header);

		// print the comment
		printf("%s\n", header);

		// read in the first character of the next line
		fscanf(inFile, "%c", &tempChar);
	}

	// the last one was not a comment character '#', so we need to put it back into the file stream (undo)
	ungetc(tempChar, inFile);

	// read in the image hieght, width and the maximum value
	fscanf(inFile, "%d %d %d", &width, &height, &maxVal);
	// print out the information about the image file
	printf("%d rows  %d columns  max value= %d\n", height, width, maxVal);

	// compute the total number of pixels in the image
	totalPixels = width * height;

	// allocate enough memory for the image  (3*) because of the RGB data
	texture = malloc(3 * sizeof(GLuint) * totalPixels);

	// determine the scaling for RGB values
	RGBScaling = 255.0f / maxVal;

	// if the maxValue is 255 then we do not need to scale the 
	//    image data values to be in the range or 0 to 255
	if (maxVal == 255)
	{
		for (i = 0; i < totalPixels; i++)
		{
			// read in the current pixel from the file
			fscanf(inFile, "%d %d %d", &red, &green, &blue);

			// store the red, green and blue data of the current pixel in the data array
			texture[3 * totalPixels - 3 * i - 3] = red;
			texture[3 * totalPixels - 3 * i - 2] = green;
			texture[3 * totalPixels - 3 * i - 1] = blue;
		}
	}
	else  // need to scale up the data values
	{
		for (i = 0; i < totalPixels; i++)
		{
			// read in the current pixel from the file
			fscanf(inFile, "%d %d %d", &red, &green, &blue);

			// store the red, green and blue data of the current pixel in the data array
			texture[3 * totalPixels - 3 * i - 3] = (GLubyte)(red * RGBScaling);
			texture[3 * totalPixels - 3 * i - 2] = (GLubyte)(green * RGBScaling);
			texture[3 * totalPixels - 3 * i - 1] = (GLubyte)(blue * RGBScaling);
		}
	}


	fclose(inFile);

	glBindTexture(GL_TEXTURE_2D, textureID);


	//Set the texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);


	//Create mipmaps
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, (GLuint)width, (GLuint)height, GL_RGB, GL_UNSIGNED_BYTE, texture);

	//openGL guarantees to have the texture data stored so we no longer need it
	free(texture);

	//return the current texture id
	return(textureID);
}

meshObject* loadMeshObject(char* fileName)
{
	FILE* inFile;
	meshObject* object;
	char line[512];					// Line currently being parsed 
	char keyword[10];				// Keyword currently being parsed
	int currentVertexIndex = 0;		// 0-based index of the vertex currently being parsed
	int currentTexCoordIndex = 0;	// 0-based index of the texure coordinate currently being parsed
	int currentNormalIndex = 0;		// 0-based index of the normal currently being parsed
	int currentFaceIndex = 0;		// 0-based index of the face currently being parsed

	inFile = fopen(fileName, "r");

	if (inFile == NULL) {
		return NULL;
	}

	// Allocate and initialize a new Mesh Object.
	object = malloc(sizeof(meshObject));
	object->vertexCount = 0;
	object->vertices = NULL;
	object->texCoordCount = 0;
	object->texCoords = NULL;
	object->normalCount = 0;
	object->normals = NULL;
	object->faceCount = 0;
	object->faces = NULL;

	// Pre-parse the file to determine how many vertices, texture coordinates, normals, and faces we have.
	while (fgets(line, (unsigned)_countof(line), inFile))
	{
		if (sscanf_s(line, "%9s", keyword, (unsigned)_countof(keyword)) == 1) {
			if (strcmp(keyword, "v") == 0) {
				object->vertexCount++;
			}
			else if (strcmp(keyword, "vt") == 0) {
				object->texCoordCount++;
			}
			else if (strcmp(keyword, "vn") == 0) {
				object->normalCount++;
			}
			else if (strcmp(keyword, "f") == 0) {
				object->faceCount++;
			}
		}
	}

	if (object->vertexCount > 0)object->vertices = malloc(sizeof(vec3d) * object->vertexCount);
	if (object->texCoordCount > 0) object->texCoords = malloc(sizeof(vec2d) * object->texCoordCount);
	if (object->normalCount > 0) object->normals = malloc(sizeof(vec3d) * object->normalCount);
	if (object->faceCount > 0) object->faces = malloc(sizeof(meshObjectFace) * object->faceCount);

	// Parse the file again, reading the actual vertices, texture coordinates, normals, and faces.
	rewind(inFile);

	while (fgets(line, (unsigned)_countof(line), inFile))
	{
		if (sscanf_s(line, "%9s", keyword, (unsigned)_countof(keyword)) == 1) {
			if (strcmp(keyword, "v") == 0) {
				vec3d vertex = { 0, 0, 0 };
				sscanf_s(line, "%*s %f %f %f", &vertex.x, &vertex.y, &vertex.z);
				memcpy_s(&object->vertices[currentVertexIndex], sizeof(vec3d), &vertex, sizeof(vec3d));
				currentVertexIndex++;
			}
			else if (strcmp(keyword, "vt") == 0) {
				vec2d texCoord = { 0, 0 };
				sscanf_s(line, "%*s %f %f", &texCoord.x, &texCoord.y);
				memcpy_s(&object->texCoords[currentTexCoordIndex], sizeof(vec2d), &texCoord, sizeof(vec2d));
				currentTexCoordIndex++;
			}
			else if (strcmp(keyword, "vn") == 0) {
				vec3d normal = { 0, 0, 0 };
				sscanf_s(line, "%*s %f %f %f", &normal.x, &normal.y, &normal.z);
				memcpy_s(&object->normals[currentNormalIndex], sizeof(vec3d), &normal, sizeof(vec3d));
				currentNormalIndex++;
			}
			else if (strcmp(keyword, "f") == 0) {
				initMeshObjectFace(&(object->faces[currentFaceIndex]), line, _countof(line));
				currentFaceIndex++;
			}
		}
	}

	fclose(inFile);

	return object;
}

void renderMeshObject(meshObject* object) {
	for (int faceNo = 0; faceNo < object->faceCount; faceNo++) {
		meshObjectFace face = object->faces[faceNo];
		if (face.pointCount >= 3) {
			glBegin(GL_POLYGON);

			for (int pointNo = 0; pointNo < face.pointCount; pointNo++) {
				meshObjectFacePoint point = face.points[pointNo];

				if (point.normalIndex >= 0) {
					vec3d normal = object->normals[point.normalIndex];
					glNormal3d(normal.x, normal.y, normal.z);
				}

				if (point.texCoordIndex >= 0) {
					vec2d texCoord = object->texCoords[point.texCoordIndex];
					glTexCoord2d(texCoord.x, texCoord.y);
				}

				vec3d vertex = object->vertices[point.vertexIndex];
				glVertex3f(vertex.x, vertex.y, vertex.z);
			}

			glEnd();
		}
	}
}

void freeMeshObject(meshObject* object)
{
	if (object != NULL) {
		free(object->vertices);
		free(object->texCoords);
		free(object->normals);

		if (object->faces != NULL) {
			for (int i = 0; i < object->faceCount; i++) {
				free(object->faces[i].points);
			}

			free(object->faces);
		}

		free(object);
	}
}

void borderCollision(void)
{
	float border = WORLD_RADIUS - HELICOPTER_BODY_RADIUS - TAIL_LENGTH;
	// distance of the helicopter from the origin
	float distance = sqrtf(helicopterLocation[0] * helicopterLocation[0] + helicopterLocation[2] * helicopterLocation[2]);

	if (distance >= border) {
		
		// calculate the new helicopter location
		float angle = atan2f(helicopterLocation[2], helicopterLocation[0]);
		helicopterLocation[0] = border * cosf(angle);
		helicopterLocation[2] = border * sinf(angle);
	}
}

/*
  A simple ground plane in the XZ plane with vertex normals specified for lighting
  the top face of the ground. The bottom face is not lit.
*/

void drawGrid(void)
{
	renderFillEnabled ? glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) : glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glMaterialfv(GL_FRONT, GL_DIFFUSE, whiteDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	glEnable(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Specify the texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, grass.width, grass.height, 0, GL_RGB, GL_UNSIGNED_BYTE, grass.data);

	float origin = -GRID_SIZE / 2.0f;

	for (float z = origin; z < (GRID_SIZE / 2.0f) * 0.15f; z += GRID_SQUARE_SIZE)
	{
		for (float x = origin; x < GRID_SIZE / 2.0f; x += GRID_SQUARE_SIZE)
		{
			glBegin(GL_QUADS);
			glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
			glTexCoord2d(0.0f, 1.0f); // coord for texture
			glVertex3f(x, 0.0f, z);
			glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
			glTexCoord2d(1.0f, 1.0f); // coord for texture
			glVertex3f(x, 0.0f, z + GRID_SQUARE_SIZE);
			glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
			glTexCoord2d(1.0f, 0.0f); // coord for texture
			glVertex3f(x + GRID_SQUARE_SIZE, 0.0f, z + GRID_SQUARE_SIZE);
			glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
			glTexCoord2d(0.0f, 0.0f); // coord for texture
			glVertex3f(x + GRID_SQUARE_SIZE, 0.0f, z);
			glEnd();
		}
	}

	glDeleteTextures(1, &grassId);

	// Specify the texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, water.width, water.height, 0, GL_RGB, GL_UNSIGNED_BYTE, water.data);

	for (float z = origin; z < GRID_SIZE / 2.0f; z += GRID_SQUARE_SIZE)
	{
		for (float x = origin; x < GRID_SIZE / 2.0f; x += GRID_SQUARE_SIZE)
		{
			// chagne the 'origin' in vertexes to be based on x & y
			glBegin(GL_QUADS);
			glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
			glTexCoord2d(0.0f, 1.0f); // coord for texture
			glVertex3f(x, 0.0f, z);
			glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
			glTexCoord2d(1.0f, 1.0f); // coord for texture
			glVertex3f(x, 0.0f, z + GRID_SQUARE_SIZE);
			glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
			glTexCoord2d(1.0f, 0.0f); // coord for texture
			glVertex3f(x + GRID_SQUARE_SIZE, 0.0f, z + GRID_SQUARE_SIZE);
			glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
			glTexCoord2d(0.0f, 0.0f); // coord for texture
			glVertex3f(x + GRID_SQUARE_SIZE, 0.0f, z);
			glEnd();
		}
	}

	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &waterId);
}

void drawSkyBorder(void)
{
	// calculate heli distance form origin

	glMaterialfv(GL_FRONT, GL_DIFFUSE,greyDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	glPushMatrix();

	// move upwards
	glTranslated(0.0, SKY_HEIGHT * 1.5, 0.0);

	// rotate to verticle
	glRotated(90, 1.0, 0.0, 0.0);

	gluCylinder(cylinderQuadric, WORLD_RADIUS, WORLD_RADIUS, SKY_HEIGHT * 1.5, 50, 50);

	glPopMatrix();
}

void drawHelipad(void)
{
	renderFillEnabled ? glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) : glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glMaterialfv(GL_FRONT, GL_DIFFUSE,whiteDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	// base
	glBegin(GL_QUADS);

	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.6f, 0.1f, -GRID_SIZE / 2 * 0.6f);
	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.4f, 0.1f, -GRID_SIZE / 2 * 0.6f);
	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.4f, 0.1f, -GRID_SIZE / 2 * 0.4f);
	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.6f, 0.1f, -GRID_SIZE / 2 * 0.4f);

	glEnd();

	glMaterialfv(GL_FRONT, GL_DIFFUSE,blackDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	// "left" vert line
	glBegin(GL_QUADS);

	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.55f, 0.11f, -GRID_SIZE / 2 * 0.55f);
	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.53f, 0.11f, -GRID_SIZE / 2 * 0.55f);
	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.53f, 0.11f, -GRID_SIZE / 2 * 0.45f);
	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.55f, 0.11f, -GRID_SIZE / 2 * 0.45f);

	glEnd();

	// horizontal line
	glBegin(GL_QUADS);

	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.53f, 0.11f, -GRID_SIZE / 2 * 0.51f);
	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.47f, 0.11f, -GRID_SIZE / 2 * 0.51f);
	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.47f, 0.11f, -GRID_SIZE / 2 * 0.49f);
	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.53f, 0.11f, -GRID_SIZE / 2 * 0.49f);

	glEnd();
	
	// "right" vert line
	glBegin(GL_QUADS);

	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.47f, 0.11f, -GRID_SIZE / 2 * 0.55f);
	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.45f, 0.11f, -GRID_SIZE / 2 * 0.55f);
	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.45f, 0.11f, -GRID_SIZE / 2 * 0.45f);
	glNormal3d(0.0, 1.0, 0.0); //set normal to enable by-vertex lighting on ground
	glVertex3f(GRID_SIZE / 2 * 0.47f, 0.11f, -GRID_SIZE / 2 * 0.45f);

	glEnd();
}

void drawHelicopter(void)
{
	renderFillEnabled ? gluQuadricDrawStyle(sphereQuadric, GLU_FILL) : gluQuadricDrawStyle(sphereQuadric, GLU_LINE);

	glPushMatrix();

	// translate helictoper
	glTranslated(helicopterLocation[0], helicopterLocation[1], helicopterLocation[2]);
	// rotate helicopter
	glRotated(helicopterFacing, 0.0, 1.0, 0.0);

	glMaterialfv(GL_FRONT, GL_DIFFUSE,policeBlueDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, policeBlueDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);
	gluSphere(sphereQuadric, HELICOPTER_BODY_RADIUS, 50, 50);

	// front windshield
	drawWindshield();

	// left and right skid connectors
	drawSkidConnector(leftSide);
	drawSkidConnector(rightSide);

	// left and right skids
	drawSkid(rightSide);
	drawSkid(leftSide);

	// Top rotors
	drawTopRotors();

	// Tail
	drawTail();

	glPopMatrix();
}

void drawWindshield(void)
{
	renderFillEnabled ? gluQuadricDrawStyle(cylinderQuadric, GLU_FILL) : gluQuadricDrawStyle(cylinderQuadric, GLU_LINE);

	glMaterialfv(GL_FRONT, GL_DIFFUSE,lightCyanDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	glPushMatrix();

	// move forwards and up
	glTranslated(-WINDSHIELD_LENGTH / 2, WINDSHIELD_LENGTH / 3.75, WINDSHIELD_LENGTH);

	// rotate
	glRotated(90, 0.0, 1.0, 0.0);

	// cylinder acting as windshield
	gluCylinder(cylinderQuadric, WINDSHIELD_RADIUS, WINDSHIELD_RADIUS, WINDSHIELD_LENGTH, 50, 50);

	// ball to cap windshield
	gluSphere(sphereQuadric, WINDSHIELD_RADIUS, 50, 50);

	// move to the other end of the winshield
	glTranslated(0.0, 0.0, WINDSHIELD_LENGTH);

	// ball to cap windshield
	gluSphere(sphereQuadric, WINDSHIELD_RADIUS, 50, 50);

	glPopMatrix();
}

void drawSkidConnector(enum Side side)
{
	renderFillEnabled ? gluQuadricDrawStyle(cylinderQuadric, GLU_FILL) : gluQuadricDrawStyle(cylinderQuadric, GLU_LINE);

	glMaterialfv(GL_FRONT, GL_DIFFUSE,brownDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	glPushMatrix();

	// move to the left or right, into position 
	glTranslated(-HELICOPTER_BODY_RADIUS / 2 * side, 0, 0);

	// move down and to the side
	glTranslated(HELICOPTER_BODY_RADIUS * side, -HELICOPTER_BODY_RADIUS * 0.75, 0.0);

	// rotate to verticle
	glRotated(90, 1.0, 0.0, 0.0);

	// connecter cylinder
	gluCylinder(cylinderQuadric, SKID_CONNECTOR_RADIUS, SKID_CONNECTOR_RADIUS, SKID_CONNECTOR_LENGTH, 50, 50);

	glPopMatrix();
}

void drawSkid(enum Side side)
{
	renderFillEnabled ? gluQuadricDrawStyle(cylinderQuadric, GLU_FILL) : gluQuadricDrawStyle(cylinderQuadric, GLU_LINE);

	glMaterialfv(GL_FRONT, GL_DIFFUSE,brownDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	glPushMatrix();

	// move to correct position for middle of skid
	glTranslated(-HELICOPTER_BODY_RADIUS / 2 * side, -HELICOPTER_BODY_RADIUS * 1.5, -HELICOPTER_BODY_RADIUS * 1.5);

	// skid
	gluCylinder(cylinderQuadric, SKID_RADIUS, SKID_RADIUS, SKID_LENGTH, 50, 50);

	// skid endings
	drawSkidEnding(side, frontSide);
	drawSkidEnding(side, backSide);

	glPopMatrix();
}

void drawSkidEnding(enum Side xSide, enum Side zSide)
{
	glPushMatrix();

	// stay or move to the front
	glTranslated(0, 0, zSide == frontSide ? SKID_LENGTH : 0);
	// ball
	gluSphere(sphereQuadric, SKID_ENDING_RADIUS, 50, 50);

	glPopMatrix();
}

void drawTopRotors(void)
{
	glPushMatrix();

	// stay or move to the front
	glTranslated(0.0, HELICOPTER_BODY_RADIUS + 0.2, 0.0);

	// blades
	for (int i = 1; i < ROTOR_NUMBER_OF_BLADES + 1; i++)
	{
		drawBlade(i);
	}

	// scale the cube 
	glScaled(0.2, 1.0, 0.2);

	// cube in the middle of rotors
	glutSolidCube(ROTOR_CUBE_SIZE);

	glPopMatrix();
}

void drawBlade(int num)
{
	glPushMatrix();

	// stay or move to the front
	glTranslated(0.0, ROTOR_CUBE_SIZE / 2 - 0.2, 0.0);

	// rotate based on which blade
	glRotated(360 / ROTOR_NUMBER_OF_BLADES * num + rotorAngle, 0.0, 1.0, 0.0);

	// flatten cube to make it look like a blade
	glScaled(1.0, 0.02, 0.05);

	// blade
	glutSolidCube(ROTOR_BLADE_SIZE);

	glPopMatrix();
}

void drawTail(void)
{
	glPushMatrix();

	glMaterialfv(GL_FRONT, GL_DIFFUSE,policeBlueDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, policeBlueDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	// rotate to the back
	glRotated(180, 1.0, 0.0, 0.0);

	// draw the tail cylinder, getting smaller at the end
	gluCylinder(cylinderQuadric, TAIL_BASE, TAIL_TIP_RADIUS, TAIL_LENGTH, 20, 20);

	// move to the end of the tail
	glTranslated(0.0, 0.0, TAIL_LENGTH);

	// cap the tail
	gluSphere(sphereQuadric, TAIL_TIP_RADIUS, 50, 50);


	// tail rotors
	drawTailRotors();

	glPopMatrix();
}

void drawTailRotors(void)
{
	glMaterialfv(GL_FRONT, GL_DIFFUSE,brownDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	glPushMatrix();

	// turn to the side
	glRotated(90, 0.0, 0.0, 1.0);

	// move out to the side of the tail
	glTranslated(0.0, TAIL_TIP_RADIUS * 1.35, 0.0);

	// scale the rotor
	glScaled(1.0 * TAIL_ROTOR_SCALE_FACTOR, 1.0 * TAIL_ROTOR_SCALE_FACTOR, 1.0 * TAIL_ROTOR_SCALE_FACTOR);

	// blades
	for (int i = 1; i < ROTOR_NUMBER_OF_BLADES + 1; i++)
	{
		drawBlade(i);
	}

	// scale rotor cube
	glScaled(0.2, 1.0, 0.2);

	// cube
	glutSolidCube(ROTOR_CUBE_SIZE);

	glPopMatrix();
}

void drawBoat(void)
{
	glPushMatrix();

	// translate boat
	glTranslated(boatLocation[0], boatLocation[1], boatLocation[2]);

	// rotate about the y for spin
	glRotated(boatFacing, 0.0, 1.0, 0.0);

	// draw base
	drawBoatBase();


	glPopMatrix();
}

void drawBoatBase(void)
{
	glMaterialfv(GL_FRONT, GL_DIFFUSE,blueDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	glPushMatrix();

	// scale base cube
	glScaled(0.4, 0.5, 0.7);

	// cube
	glutSolidCube(BOAT_BASE_SIZE);


	// draw cabin
	drawBoatCabin();

	glPopMatrix();
}

void drawBoatCabin(void)
{
	glMaterialfv(GL_FRONT, GL_DIFFUSE,redDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	glPushMatrix();

	// translate upwards
	glTranslated(0.0, BOAT_CABIN_SIZE * 1.5, BOAT_CABIN_SIZE * 0.333);

	// scale base cube
	glScaled(0.9, 0.8, 0.9);

	// cube
	glutSolidCube(BOAT_CABIN_SIZE);

	glPopMatrix();
}

void drawDock(void)
{
	glMaterialfv(GL_FRONT, GL_DIFFUSE,brownDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	glPushMatrix();

	// translate to side 
	glTranslated(0.0, -0.1, GRID_SIZE / 2 * 0.2);

	for (int i = 0; i < 8; i++)
	{
		drawPlank(i);
	}

	drawLamp();

	glPopMatrix();
}

void drawPlank(int num)
{
	glPushMatrix();

	// translate to side 
	glTranslated(DOCK_PLANK_SIZE * num * 0.055, 0.0, 0.0);

	// rotate about the x so it is is horizontal
	glRotated(90, 1.0, 0.0, 0.0);

	// scale the cube 
	glScaled(0.05, 1.0, 0.05);

	// cube
	glutSolidCube(DOCK_PLANK_SIZE);

	glPopMatrix();
}

void drawLamp(void)
{
	renderFillEnabled ? gluQuadricDrawStyle(sphereQuadric, GLU_FILL) : gluQuadricDrawStyle(sphereQuadric, GLU_LINE);
	glPushMatrix();

	// translate to the top of the dock
	glTranslated(0.0, 1.0, 0.0);

	// draw the street light post
	glMaterialfv(GL_FRONT, GL_DIFFUSE,paleGreenDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);
	glScaled(0.05, 1.0, 0.05);
	glutSolidCube(LAMP_POST_SIZE);

	glPopMatrix();

	glPushMatrix();

	// translate to the top of the street light post
	glTranslated(LAMP_CONNECTOR_SIZE / 4, LAMP_POST_SIZE * 0.7, 0.0);

	// draw the street light lamp
	glMaterialfv(GL_FRONT, GL_DIFFUSE,blueDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);
	// rotate about the x so it is is horizontal
	glRotated(90, 1.0, 0.0, 0.0);
	glScaled(1.0, 0.3, 0.3);
	glutSolidCube(LAMP_CONNECTOR_SIZE);

	glPopMatrix();

	glPushMatrix();

	// translate to the top of the street light lamp
	glTranslated(LAMP_CONNECTOR_SIZE / 2, LAMP_POST_SIZE * 0.65, 0.0);

	// draw the light bulb
	glMaterialfv(GL_FRONT, GL_EMISSION, yellowDiffuse);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,yellowDiffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glMaterialf(GL_FRONT, GL_SHININESS, noShininess);

	glutSolidSphere(LAMP_BULB_SIZE, 50, 50);

	// turn off the emission
	glMaterialfv(GL_FRONT, GL_EMISSION, zeroMaterial);

	glPopMatrix();
}
/******************************************************************************/