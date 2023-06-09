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
 * Animation-Specific Function Prototypes (add your own here)
 ******************************************************************************/

void main(int argc, char** argv);
void init(void);
void think(void);
void initLights(void);

// origin and grid functions
void drawOrigin(void);
void drawGrid(void);


// hierarchical model functions to position and scale parts
void drawHelicopter();
void drawSkidConnector(enum Side side);
void drawSkid(enum Side side);
void drawSkidEnding(enum Side xSide, enum Side zSide);
void drawWindshield(void);
void drawWindow(enum Side side);
void drawTopRotors(void);
void drawBlade(int num);
void drawTail(void);
void drawTailRotors(void);
void drawTailFin(void);
void drawSpotLight(void);

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
const float down[] = { 0.0f, -1.0f, -1.0f };

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

PPMImage water;
PPMImage grass;

GLuint waterId;
GLuint grassId;

// rotor blade speed management
float rotorSpeed = 750.0f;
float rotorAngle = 1.0f;

// hierachical model setup values

// helicopter
// body
#define BODY_RADIUS 2.0f

// skid connectors
#define SKID_CONNECTOR_RADIUS BODY_RADIUS / 10.0f
#define SKID_CONNECTOR_LENGTH BODY_RADIUS * 0.8f

// skids
#define SKID_RADIUS BODY_RADIUS / 10.0f
#define SKID_LENGTH BODY_RADIUS * 3.0f

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
#define NUMBER_OF_BLADES 4

// tail
#define TAIL_BASE 1.0f
#define TAIL_LENGTH 6.5f
#define TAIL_TIP_RADIUS 0.25f
#define TAIL_ROTORS_SCALE_FACTOR 0.25f

// grid
#define GRID_SQUARE_SIZE 1.0f
#define GRID_SIZE 100.0f

#define PI 3.1415f

// camera distance
#define CAMERA_DISTANCE 15.0f

// initial y value for the helicopter centre 
#define START_HEIGHT BODY_RADIUS + SKID_CONNECTOR_LENGTH + SKID_RADIUS

const GLfloat PALE_GREEN[3] = { 0.596f, 0.984f, 0.596f };
const GLfloat BATMAN_GREY[3] = { 0.3f, 0.3f, 0.3f };
const GLfloat BROWN[3] = { 0.545f, 0.27f, 0.0745f };
const GLfloat POLICE_BLUE[3] = { 0.0f, 0.0f, 0.40f };
const GLfloat LIGHT_CYAN[3] = { 0.58f, 1.0f, 1.0f };
const GLfloat WHITE[3] = { 1.0f, 1.0f, 1.0f };

//model animation variables (position, heading, speed (metres per second))
float helicopterLocation[] = { 0.0f, START_HEIGHT, 0.0f }; // X, Y, Z
float helicopterFacing = 0.0f;
const float moveSpeed = 10.0f;

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

	// origin point
	drawOrigin();

	// draw the ground
	drawGrid();

	// draw helicopter
	drawHelicopter();

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

	// set background color to be black
	glClearColor(0, 0, 0, 1.0);

	// Anything that relies on lighting or specifies normals must be initialised after initLights.
	initLights();

	//Enable use of fog
	glEnable(GL_FOG);

	// define the color and density of the fog
	GLfloat fogColor[4] = { 0.25f, 0.25f, 0.25f, 0.25f };
	GLfloat fogDensity = 0.025f;
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

	// Load PPM image
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
			float xMove = sinf(helicopterFacing * (PI / 180)) * moveSpeed;
			float zMove = cosf(helicopterFacing * (PI / 180)) * moveSpeed;

			helicopterLocation[0] += xMove * FRAME_TIME_SEC * keyboardMotion.Surge;
			helicopterLocation[2] += zMove * FRAME_TIME_SEC * keyboardMotion.Surge;
		}
		if (keyboardMotion.Sway != MOTION_NONE) {
			/* TEMPLATE: Move (strafe) your object left if .Sway < 0, or right if .Sway > 0 */
			float xMove = sinf((helicopterFacing + 90.0f) * (PI / 180)) * moveSpeed;
			float zMove = cosf((helicopterFacing + 90.0f) * (PI / 180)) * moveSpeed;

			helicopterLocation[0] -= xMove * FRAME_TIME_SEC * keyboardMotion.Sway;
			helicopterLocation[2] -= zMove * FRAME_TIME_SEC * keyboardMotion.Sway;
		}
		if (keyboardMotion.Heave != MOTION_NONE) {
			/* TEMPLATE: Move your object down if .Heave < 0, or up if .Heave > 0 */
			// stops the helicopter from moving below the grid
			if (helicopterLocation[1] > START_HEIGHT)
				helicopterLocation[1] += keyboardMotion.Heave * moveSpeed / 2 * FRAME_TIME_SEC;
			else if (keyboardMotion.Heave > 0)
				helicopterLocation[1] += keyboardMotion.Heave * moveSpeed / 2 * FRAME_TIME_SEC;
		}
	}
	else {
		rotorSpeed += ROTOR_ACCELRATION * FRAME_TIME_SEC;
	}

	GLfloat spotLightPosition[] = { helicopterLocation[0], helicopterLocation[1] - BODY_RADIUS, helicopterLocation[2], 1.0f };

	glLightfv(GL_LIGHT1, GL_POSITION, spotLightPosition);

	// I didn't like the idea of this number getting stupidly huge so I wanted to reset it to avoid bugs
	if (rotorAngle > 360.0f)
		rotorAngle = 0.0f;

	// rotor spin
	rotorAngle += rotorSpeed * FRAME_TIME_SEC;

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

	float spotLightExponent = 5.0f;
	float spotLightCutoff = 60.0f;

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

	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, down);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, spotLightExponent);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, spotLightCutoff);


	// Enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	// Make GL normalize the normal vectors we supply.
	glEnable(GL_NORMALIZE);

	// Enable use of simple GL colours as materials.
	glEnable(GL_COLOR_MATERIAL);
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

void drawOrigin(void)
{
	glColor3f(0.0f, 1.0f, 1.0f);
	glutWireSphere(0.1, 10, 10);

	glBegin(GL_LINES);

	//x axis -red
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(2.0f, 0.0f, 0.0f);

	//y axis -green
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 2.0f, 0.0f);

	//z axis - blue
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 2.0f);

	glEnd();
}

/*
  A simple ground plane in the XZ plane with vertex normals specified for lighting
  the top face of the ground. The bottom face is not lit.
*/

void drawGrid(void)
{
	renderFillEnabled ? glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) : glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glColor3fv(WHITE); //pale green -- better to have a const

	glEnable(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Specify the texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, grass.width, grass.height, 0, GL_RGB, GL_UNSIGNED_BYTE, grass.data);

	float origin = -GRID_SIZE / 2.0f;

	for (float z = origin; z < (GRID_SIZE / 2.0f) * 0.5f; z += GRID_SQUARE_SIZE)
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

	origin = -GRID_SIZE / 2.0f;

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

	glDeleteTextures(1, &waterId);
}


void drawHelicopter()
{
	renderFillEnabled ? gluQuadricDrawStyle(sphereQuadric, GLU_FILL) : gluQuadricDrawStyle(sphereQuadric, GLU_LINE);

	glPushMatrix();

	// translate helictoper
	glTranslated(helicopterLocation[0], helicopterLocation[1], helicopterLocation[2]);
	// rotate helicopter
	glRotated(helicopterFacing, 0.0, 1.0, 0.0);

	glColor3fv(POLICE_BLUE);
	gluSphere(sphereQuadric, BODY_RADIUS, 50, 50);

	// front windshield
	drawWindshield();

	// left and right skid connectors
	drawSkidConnector(leftSide);
	drawSkidConnector(rightSide);

	// left and right skids
	drawSkid(rightSide);
	drawSkid(leftSide);

	/*drawWindow(rightSide);
	drawWindow(leftSide);*/

	// Top rotors
	drawTopRotors();

	// Tail
	drawTail();

	glPopMatrix();
}

void drawWindshield(void)
{
	renderFillEnabled ? gluQuadricDrawStyle(cylinderQuadric, GLU_FILL) : gluQuadricDrawStyle(cylinderQuadric, GLU_LINE);

	glColor3fv(LIGHT_CYAN);

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

	glColor3fv(BROWN);

	glPushMatrix();

	// move to the left or right, into position 
	glTranslated(-BODY_RADIUS / 2 * side, 0, 0);

	// move down and to the side
	glTranslated(BODY_RADIUS * side, -BODY_RADIUS * 0.75, 0.0);

	// rotate to verticle
	glRotated(90, 1.0, 0.0, 0.0);

	// connecter cylinder
	gluCylinder(cylinderQuadric, SKID_CONNECTOR_RADIUS, SKID_CONNECTOR_RADIUS, SKID_CONNECTOR_LENGTH, 50, 50);

	glPopMatrix();
}

void drawSkid(enum Side side)
{
	renderFillEnabled ? gluQuadricDrawStyle(cylinderQuadric, GLU_FILL) : gluQuadricDrawStyle(cylinderQuadric, GLU_LINE);

	glColor3fv(BROWN);

	glPushMatrix();

	// move to correct position for middle of skid
	glTranslated(-BODY_RADIUS / 2 * side, -BODY_RADIUS * 1.5, -BODY_RADIUS * 1.5);

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

void drawWindow(enum Side side) {

	glColor3fv(BATMAN_GREY);

	glPushMatrix();


	//	glutSolidCube(WINDSCREEN_);

	glPopMatrix();

}

void drawTopRotors(void)
{
	glPushMatrix();

	// stay or move to the front
	glTranslated(0.0, BODY_RADIUS + 0.2, 0.0);

	// blades
	for (int i = 1; i < NUMBER_OF_BLADES + 1; i++)
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
	glRotated(360 / NUMBER_OF_BLADES * num + rotorAngle, 0.0, 1.0, 0.0);

	// flatten cube to make it look like a blade
	glScaled(1.0, 0.02, 0.05);

	// blade
	glutSolidCube(ROTOR_BLADE_SIZE);

	glPopMatrix();
}

void drawTail(void)
{
	glPushMatrix();

	glColor3fv(POLICE_BLUE);

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
	glColor3fv(BROWN);

	glPushMatrix();

	// turn to the side
	glRotated(90, 0.0, 0.0, 1.0);

	// move out to the side of the tail
	glTranslated(0.0, TAIL_TIP_RADIUS * 1.35, 0.0);

	// scale the rotor
	glScaled(1.0 * TAIL_ROTORS_SCALE_FACTOR, 1.0 * TAIL_ROTORS_SCALE_FACTOR, 1.0 * TAIL_ROTORS_SCALE_FACTOR);

	// blades
	for (int i = 1; i < NUMBER_OF_BLADES + 1; i++)
	{
		drawBlade(i);
	}

	// scale rotor cube
	glScaled(0.2, 1.0, 0.2);

	// cube
	glutSolidCube(ROTOR_CUBE_SIZE);

	glPopMatrix();
}

void makeWater(void)
{
	// Set texture parameters
	glEnable(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Specify the texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, water.width, water.height, 0, GL_RGB, GL_UNSIGNED_BYTE, water.data);

	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &waterId);
}

void makeGrass(void)
{
	// Set texture parameters
	glEnable(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Specify the texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, grass.width, grass.height, 0, GL_RGB, GL_UNSIGNED_BYTE, grass.data);

	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &grassId);
}


/******************************************************************************/