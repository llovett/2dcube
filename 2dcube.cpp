#include <GL/glut.h>
#include <GL/glui.h>
#include <math.h>

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

/* constants */
int SIDES[] = { 8, 16, 32, 64, 128 };
float SPEEDS[] = { 0.0, 0.01, 0.1, 1.0, 6.0 };
GLfloat x=5.0, y=5.0;
GLfloat side = 3.0;

void setupViewport(int w, int h) {
    glViewport(0, 0, w, h); 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluOrtho2D(0.0, XSCALE*w/INITIAL_WIDTH, 0.0, h*YSCALE/INITIAL_HEIGHT );
}

void init() {
    glClearColor(1.0, 1.0, 0.4, 1.0);
    glColor3f(0.5, 0.5, 0.5);
    setupViewport(INITIAL_WIDTH, INITIAL_HEIGHT);
}

void myReshape(int w, int h) {
    setupViewport(w, h);
    glutPostWindowRedisplay(main_window);  
}

GLfloat radians(float alpha) {
    return alpha*PI/180.0;
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
	glVertex2f(x+side*cos(radians(baseAngle)), y+side*sin(radians(baseAngle)));
	glVertex2f(x+side*cos(radians(baseAngle+inc)), y+side*sin(radians(baseAngle + inc)));

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
    
    // GLUI_Rollout *speedRollout = new GLUI_Rollout(control_panel, "Speeds", false);
    // GLUI_RadioGroup *speedsGroup = new GLUI_RadioGroup(speedRollout, &WhichSpeed, 0, speedsCallback);
    // new GLUI_RadioButton(speedsGroup, "tree" );
    // new GLUI_RadioButton(speedsGroup, "tortoise" );
    // new GLUI_RadioButton(speedsGroup, "raccoon" );
    // new GLUI_RadioButton(speedsGroup, "kangaroo" );
    // new GLUI_RadioButton(speedsGroup, "cheetah" );
    // speedsGroup->set_int_val(2);	// racoon by default
    // GLUI_Rollout *sidesRollout = new GLUI_Rollout(control_panel, "Sides", false);
    // GLUI_RadioGroup *sidesGroup = new GLUI_RadioGroup(sidesRollout, &WhichSides, 0, sidesCallback);
    // new GLUI_RadioButton(sidesGroup, "8" );
    // new GLUI_RadioButton(sidesGroup, "16" );
    // new GLUI_RadioButton(sidesGroup, "32" );
    // new GLUI_RadioButton(sidesGroup, "64" );
    // new GLUI_RadioButton(sidesGroup, "128" );
    // sidesGroup->set_int_val(2);		// 32 by defaultx
    
    control_panel->set_main_gfx_window(main_window);

    GLUI_Master.set_glutIdleFunc(spinDisplay);
    glutMainLoop();
    return EXIT_SUCCESS;
} 
