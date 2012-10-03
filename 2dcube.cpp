#include <GL/glut.h>
#include <GL/glui.h>
#include <math.h>
#include "matrix.h"

#define true 1
#define false 0
#define INITIAL_WIDTH 500
#define INITIAL_HEIGHT 500
#define XSCALE 10.0
#define YSCALE 10.0
#define PI 3.1415926535

int main_window;

/* globals */
int WhichSides;
int WhichSpeed;
GLfloat Theta = 0.0;
GLfloat CubeSize = 3.0f;
GLfloat EyePosX, EyePosY, EyePosZ;
GLfloat LookAtX, LookAtY, LookAtZ;
GLfloat CubeX, CubeY, CubeZ;
/* matrix transforms */
Matrix TranslateToViewer(4,4);
Matrix Rotate1(4,4);
Matrix Rotate2(4,4);
Matrix Rotate3(4,4);
Matrix FlipHandedness(4,4);
Matrix V(4,4); /* viewer coordinate transformation */
Matrix P(4,4); /* perspective transformation */

/* constants */
int SIDES[] = { 8, 16, 32, 64, 128 };
float SPEEDS[] = { 0.0, 0.01, 0.1, 1.0, 6.0 };
GLfloat x=5.0, y=5.0;
GLfloat side = 3.0;

/* setupViewport(width, height)
 * 
 * Set up the viewport.
 */
void setupViewport(int w, int h) {
    glViewport(0, 0, w, h); 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluOrtho2D(0.0, XSCALE*w/INITIAL_WIDTH, 0.0, h*YSCALE/INITIAL_HEIGHT );
}

/* init()
 *
 * Set up background, clearcolor, call setupViewport(width, height).
 */
void init() {
    glClearColor(1.0, 1.0, 0.4, 1.0);
    glColor3f(0.5, 0.5, 0.5);
    setupViewport(INITIAL_WIDTH, INITIAL_HEIGHT);

    /* initialize matrix transforms */

    /* I'm assuming the following vectors are parallel with the viewer's axes:
     *     
     *     a, b, c
     *     -------
     * X: <1, 0, 0>
     * Y: <0, 1, 0>
     * Z: <0, 0, 1>
     */ 
    float r = sqrt(EyePosX*EyePosX + EyePosY*EyePosY);
    float R = sqrt(EyePosX*EyePosX + EyePosY*EyePosY + EyePosZ*EyePosZ);
    float h = r*R;
    float ttv_entries[16] = { 1, 0, 0, 0,
			      0, 1, 0, 0,
			      0, 0, 1, 0,
			      -EyePosX, -EyePosY, -EyePosZ, 1 };
    TranslateToViewer << ttv_entries;
    float rot1[16] = { 1.0f/r, 0, 0, 0,
		       1.0f/r, 1.0f/r, 0, 0,
		       0, 0, 1, 0,
		       0, 0, 0, 1 };
    Rotate1 << rot1;
    float rot2[16] = { r/R, 0, 0, 0,
		       0, 1, 0, 0,
		       0, 0, r/R, 0,
		       0, 0, 0, 1 };
    Rotate2 << rot2;
    float rot3[16] = { 1, 0, 0, 0,
		       0, R/h, 0, 0,
		       0, 0, R/h, 0,
		       0, 0, 0, 1 };
    Rotate3 << rot3;
    float fh[16] = { 1, 0, 0, 0,
		     0, -1, 0, 0,
		     0, 0, 1, 0,
		     0, 0, 0, 1 };
    FlipHandedness << fh;

    V = TranslateToViewer * Rotate1 * Rotate2 * Rotate3 * FlipHandedness;
}

void myReshape(int w, int h) {
    setupViewport(w, h);
    glutPostWindowRedisplay(main_window);  
}

GLfloat radians(float alpha) {
    return alpha*PI/180.0;
}

void computeViewerMatrix() {
    /* transform matrix V */
    float r = sqrt(EyePosX*EyePosX + EyePosY*EyePosY);
    float R = sqrt(EyePosX*EyePosX + EyePosY*EyePosY + EyePosZ*EyePosZ);
    float h = r*R;
    TranslateToViewer(3,0) = -EyePosX;
    TranslateToViewer(3,1) = -EyePosY;
    TranslateToViewer(3,2) = -EyePosZ;

    Rotate1(0,0) = 1.0f/r;
    Rotate1(1,0) = 1.0f/r;
    Rotate1(1,1) = 1.0f/r;

    Rotate2(0,0) = r/R;
    Rotate2(2,2) = r/R;

    Rotate3(1,1) = R/h;
    Rotate3(2,2) = R/h;

    V = TranslateToViewer * Rotate1 * Rotate2 * Rotate3 * FlipHandedness;
}

void computePerspectiveMatrix() {
    /* transform matrix P */
}

void display(){
    int i;
    int sides = SIDES[WhichSides];

    glutSetWindow(main_window);
    glClear(GL_COLOR_BUFFER_BIT);

    float inc = 360.0 / sides;
    for (i=0; i<sides; i++) {
	float baseAngle = Theta + i*inc;
	/* color function uses trig to smooth values, scaled arbitrarily to look nice. */
	float color = fabs(cos(PI*(float)i/sides))/1.3f + 0.08f;
	glColor3f(color, color, color);

	glBegin(GL_POLYGON);

	glVertex2f(x, y);
	glVertex2f(x+CubeSize*cos(radians(baseAngle)), y+CubeSize*sin(radians(baseAngle)));
	glVertex2f(x+CubeSize*cos(radians(baseAngle+inc)), y+CubeSize*sin(radians(baseAngle + inc)));

	glEnd();
    }
  
    glutSwapBuffers();
}

/* callbacks... nothing needed here. */
void speedsCallback(int ID) { }
void sidesCallback(int ID) { }

void spinDisplay() {
    Theta += SPEEDS[WhichSpeed];
    display();
}

int main(int argc, char **argv) {
  
    /* setup OpenGL */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(INITIAL_WIDTH, INITIAL_HEIGHT);
    glutInitWindowPosition(50, 50);
    main_window = glutCreateWindow( "Umbrella" );

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(myReshape);

    /* setup user controls */
    GLUI *control_panel = GLUI_Master.create_glui( "Controls");
    new GLUI_StaticText( control_panel, "2DCube Controls" );
    new GLUI_Separator(control_panel);
    new GLUI_Button(control_panel, "Quit", 0, (GLUI_Update_CB)exit);

    new GLUI_Column(control_panel, true);
    GLUI_Spinner *spinner = new GLUI_Spinner(control_panel, "Size:", GLUI_SPINNER_FLOAT, &CubeSize);
    spinner->set_float_limits(0.1f, 8.0f, GLUI_LIMIT_CLAMP);

    new GLUI_Column(control_panel, true);
    GLUI_Rollout *eyePosRollout = new GLUI_Rollout(control_panel, "Eye Position", false);
    GLUI_Spinner *epX = new GLUI_Spinner(eyePosRollout, "X", GLUI_SPINNER_FLOAT, &EyePosX);
    GLUI_Spinner *epY = new GLUI_Spinner(eyePosRollout, "Y", GLUI_SPINNER_FLOAT, &EyePosY);
    GLUI_Spinner *epZ = new GLUI_Spinner(eyePosRollout, "Z", GLUI_SPINNER_FLOAT, &EyePosZ);
    epX->set_float_limits(-10.0f, 10.0f, GLUI_LIMIT_WRAP);
    epY->set_float_limits(-10.0f, 10.0f, GLUI_LIMIT_WRAP);
    epZ->set_float_limits(-10.0f, 10.0f, GLUI_LIMIT_WRAP);
    
    GLUI_Rollout *lookAtRollout = new GLUI_Rollout(control_panel, "Looking At", false);
    GLUI_Spinner *laX = new GLUI_Spinner(lookAtRollout, "X", GLUI_SPINNER_FLOAT, &LookAtX);
    GLUI_Spinner *laY = new GLUI_Spinner(lookAtRollout, "Y", GLUI_SPINNER_FLOAT, &LookAtY);
    GLUI_Spinner *laZ = new GLUI_Spinner(lookAtRollout, "Z", GLUI_SPINNER_FLOAT, &LookAtZ);
    laX->set_float_limits(-10.0f, 10.0f, GLUI_LIMIT_WRAP);
    laY->set_float_limits(-10.0f, 10.0f, GLUI_LIMIT_WRAP);
    laZ->set_float_limits(-10.0f, 10.0f, GLUI_LIMIT_WRAP);

    new GLUI_Column(control_panel, true);
    GLUI_Rollout *clippingRollout = new GLUI_Rollout(control_panel, "Clipping Parameters", false);
    GLUI_Spinner *thetaSpinner = new GLUI_Spinner(clippingRollout, "Theta", GLUI_SPINNER_FLOAT, &Theta);
    thetaSpinner->set_float_limits(0.0f, 360.0f, GLUI_LIMIT_WRAP);
 
    control_panel->set_main_gfx_window(main_window);

    GLUI_Master.set_glutIdleFunc(spinDisplay);
    glutMainLoop();
    return EXIT_SUCCESS;
} 
