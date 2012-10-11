#include <cstdio>

#define DEBUG 1

#include <GL/glut.h>
#include <GL/glui.h>
#include <math.h>
#include "matrix.h"
#include "2dcube.h"

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
GLfloat Theta = 40.0f;
GLfloat CubeSize = 3.0f;
GLfloat EyePosX, EyePosY, EyePosZ;
GLfloat LookAtX, LookAtY, LookAtZ;
GLfloat Wl, Wr, Wt, Wb, Vl, Vr, Vt, Vb;
GLfloat Ax, Bx, Cx, Ay, By, Cy, Az, Bz, Cz;

/* matrix transforms */
Matrix TranslateToViewer(4,4);
Matrix Rotate1(4,4);
Matrix Rotate2(4,4);
Matrix Rotate3(4,4);
Matrix FlipHandedness(4,4);
Matrix V(4,4); /* viewer coordinate transformation */
Matrix P(4,4); /* perspective transformation */
Matrix W(4,4); /* viewplane-window transformation */

/* constants */
int SIDES[] = { 8, 16, 32, 64, 128 };
float SPEEDS[] = { 0.0, 0.01, 0.1, 1.0, 6.0 };
GLfloat x=5.0, y=5.0;
GLfloat side = 3.0;
#define VIEW_PLANE_DIST 10.0f
#define HITHER_PLANE_DIST 5.0f
#define YON_PLANE_DIST 15.0f
#define VIEWPORT_MARGIN 20.0f
#define CubeX 0
#define CubeY 0
#define CubeZ 0

/* setupViewport(width, height)
 * 
 * Set up the viewport.
 */
void setupViewport(int w, int h) {
    // w -= 2*VIEWPORT_MARGIN;
    // h -= 2*VIEWPORT_MARGIN;
    glViewport(0, 0, w, h); 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Wr = XSCALE*w/INITIAL_WIDTH;
    // Wb = h*YSCALE/INITIAL_HEIGHT;
    Wr = w;
    Wb = h;
    Wt = 0;
    Wl = 0;

    if ( DEBUG )
	printf("Wl=%f, Wr=%f, Wb=%f, Wt=%f\n", Wl, Wr, Wb, Wt);
    
    gluOrtho2D(Wl, Wr, Wt, Wb);

    /* Recompute W matrix */
    computeWindowMatrix(0);
}

void myReshape(int w, int h) {
    setupViewport(w, h);
    glutPostWindowRedisplay(main_window);  
}

GLfloat radians(float alpha) {
    return alpha*PI/180.0;
}

/* init()
 *
 * Set up background, clearcolor, call setupViewport(width, height).
 */
void init() {
    glClearColor(1.0, 1.0, 0.4, 1.0);
    glColor3f(0.5, 0.5, 0.5);
    setupViewport(INITIAL_WIDTH, INITIAL_HEIGHT);

    LookAtX = LookAtY = LookAtZ = 0.0f;
    EyePosX = EyePosY = EyePosZ = 10.0f;

    /***** initialize matrix transforms *****/
 
    // Initial entries for translation matrix
    float ttv_entries[16] = { 1, 0, 0, 0,
			      0, 1, 0, 0,
			      0, 0, 1, 0,
			      0, 0, 0, 1 };
    TranslateToViewer << ttv_entries;
    // Initial entries for first rotation matrix
    float rot1[16] = { 0, 0, 0, 0,
		       0, 0, 0, 0,
		       0, 0, 1, 0,
		       0, 0, 0, 1 };
    Rotate1 << rot1;
    // Initial entries for second rotation matrix
    float rot2[16] = { 0, 0, 0, 0,
		       0, 1, 0, 0,
		       0, 0, 0, 0,
		       0, 0, 0, 1 };
    Rotate2 << rot2;
    // Initial entries for third rotation matrix
    float rot3[16] = { 1, 0, 0, 0,
		       0, 0, 0, 0,
		       0, 0, 0, 0,
		       0, 0, 0, 1 };
    Rotate3 << rot3;
    // Matrix to flip handedness
    float fh[16] = { 1, 0, 0, 0,
		     0, -1, 0, 0,
		     0, 0, 1, 0,
		     0, 0, 0, 1 };
    FlipHandedness << fh;
    // Initial entries for P
    float pentries[] = { VIEW_PLANE_DIST, 0, 0, 0,
			 0, VIEW_PLANE_DIST, 0, 0,
			 0, 0, 0, 1,
			 0, 0, 0, 0 };
    P << pentries;
    // Initial entries for W
    float ws[] = { 0, 0, 0, 0,
		   0, 0, 0, 0,
		   0, 0, 1, 0,
		   0, 0, 0, 1 };
    W << ws;
 
    // Calculate the view pipeline for first time
    computeViewerAngle(0);
    computePerspectiveMatrix(0);
    computeWindowMatrix(0);

    V = TranslateToViewer * Rotate1 * Rotate2 * Rotate3 * FlipHandedness;

}

/**
 * Recomputes the viewer-parallel vectors.
 * */
void computeViewerAngle(int id) {
    // Calculate parallel Z-vector by subtracting viewer pos. from lookat point
    Az = LookAtX - EyePosX;
    Bz = LookAtY - EyePosY;
    Cz = LookAtZ - EyePosZ;

    // Since the UP vector doesn't chance, we'll use that as the parallel y-vector
    Ay = 0;
    By = 0;
    Cy = 1;	// why is the z-component 1???

    // x-vector is cross-product of Z and UP vectors
    float zvector[] = { Az, Bz, Cz };
    float upvector[] = { Ay, By, Cy };
    float *xvector = crossProduct(zvector, upvector);

    if ( DEBUG ) {
	puts("Printing parallel x-vector to viewer:");
	for ( int i=0; i<3; i++ ) {
	    printf("xvector[%d] = %f\n",i,xvector[i]);
	}
    }

    Ax = xvector[0];
    Bx = xvector[1];
    Cx = xvector[2];

    computeViewerMatrix(0);
}

void computeViewerMatrix(int id) {
    /* transform matrix V */
    float r = sqrt(Ax*Ax + Bx*Bx);
    float R = sqrt(Ax*Ax + Bx*Bx + Cx*Cx);
    float h = r*sqrt(Az*Az + Bz*Bz + Cz*Cz);

    if ( DEBUG ) {
	printf("Az=%f, Bz=%f, Cz=%f\n",Az,Bz,Cz);
	printf("r=%f, R=%f, h=%f\n",r,R,h);
    }

    TranslateToViewer(3,0) = -EyePosX;
    TranslateToViewer(3,1) = -EyePosY;
    TranslateToViewer(3,2) = -EyePosZ;

    Rotate1(0,0) = Ax/r;
    Rotate1(0,1) = -Bx/r;
    Rotate1(1,0) = Bx/r;
    Rotate1(1,1) = Ax/r;
    
    Rotate2(0,0) = r/R;
    Rotate2(0,2) = -Cx/R;
    Rotate2(2,0) = Cx/R;
    Rotate2(2,2) = r/R;

    Rotate3(1,1) = Cz*R/h;
    Rotate3(1,2) = (Bz*Ax - Az*Bx)/h;
    Rotate3(2,1) = (Az*Bx - Bz*Ax)/h;
    Rotate3(2,2) = Cz*R/h;

    V = TranslateToViewer * Rotate1 * Rotate2 * Rotate3 * FlipHandedness;
}

/**
 * Recomputes the Matrix W based on current parameters.
 * */
void computeWindowMatrix(int id) {
    // Find viewplane variables
    Vb = Vl = -VIEW_PLANE_DIST * tan(radians(Theta));
    Vr = Vt = -Vb;
    W(0,0) = (Wr - Wl)/(Vr - Vl);
    W(1,1) = (Wt - Wb)/(Vt - Vb);
    W(3,0) = (Wl*Vr - Vl*Wr)/(Vr - Vl);
    W(3,1) = (Wb*Vt - Vb*Wt)/(Vt - Vb);

    if ( DEBUG ) {
	puts("Inside of computeWindowMatrix!");
	printf("Vb=%f, Vr=%f\n",Vb,Vr);
    }
}

void computePerspectiveMatrix(int id) {
    /* transform matrix P */
    float pentries[] = { VIEW_PLANE_DIST, 0, 0, 0,
			 0, VIEW_PLANE_DIST, 0, 0,
			 0, 0, YON_PLANE_DIST/(YON_PLANE_DIST-HITHER_PLANE_DIST), 1,
			 0, 0, -HITHER_PLANE_DIST*YON_PLANE_DIST/(YON_PLANE_DIST-HITHER_PLANE_DIST), 0 };
    P << pentries;

    if ( DEBUG ) {
	puts("New perspective matrix:");
	P.print();
    }
}

void display(){
    int i;
    int sides = SIDES[WhichSides];

    glutSetWindow(main_window);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.3f, 0.0f, 0.4f);

    /* drawing the viewport */
    glBegin(GL_POLYGON);
    /* this is not the real viewport. We will clip to this
     * area later on. */
    glVertex2f(Wl, Wt);
    glVertex2f(Wr, Wt);
    glVertex2f(Wr, Wb);
    glVertex2f(Wl, Wb);
    glEnd();

    glColor3f(1.0,0.0,0.0);
    glBegin(GL_POLYGON);
    glVertex2f(0,0);
    glVertex2f(0,10);
    glVertex2f(10,10);
    glVertex2f(10,0);
    glEnd();

    /* draw the cube */
    glColor3f(1.0f, 1.0f, 1.0f);

    drawLine(0, 0, 0,
    	     CubeSize, 0, 0);
    drawLine(CubeSize, 0, 0,
    	     CubeSize, CubeSize, 0);
    drawLine(CubeSize, CubeSize, 0,
    	     0, CubeSize, 0);
    drawLine(0, CubeSize, 0,
    	     0, 0, 0);

    drawLine(0, 0, CubeSize,
    	     CubeSize, 0, CubeSize);
    drawLine(CubeSize, 0, CubeSize,
    	     CubeSize, CubeSize, CubeSize);
    drawLine(CubeSize, CubeSize, CubeSize,
    	     0, CubeSize, CubeSize);
    drawLine(0, CubeSize, CubeSize,
    	     0, 0, CubeSize);

    drawLine(0, 0, 0,
	     0, 0, CubeSize);
    drawLine(CubeSize, 0, 0,
	     CubeSize, 0, CubeSize);
    drawLine(CubeSize, CubeSize, 0,
	     CubeSize, CubeSize, CubeSize);
    drawLine(0, CubeSize, 0,
	     0, CubeSize, CubeSize);

    glutSwapBuffers();

}

void drawLine(float x1, float y1, float z1, float x2, float y2, float z2) {
    // Encode points as vectors (matrix)
    Matrix m1 = Matrix(1,4);
    float m1_entries[] = { x1, y1, z1, 1 };
    m1 << m1_entries;
    Matrix m2 = Matrix(1,4);
    float m2_entries[] = { x2, y2, z2, 1 };
    m2 << m2_entries;
    
    Matrix pipeline = V * P * W;
    m1 = Matrix::Homogenize(m1*pipeline);
    m2 = Matrix::Homogenize(m2*pipeline);

    if ( DEBUG ) {
	static int pcounter = 0;
	pcounter = (pcounter+1)%50000;
	if ( !pcounter ) {
	    if ( DEBUG ) {
		puts("-------------------- VIEW PIPELINE --------------------");
		puts("Here is V:");
		V.print();
		puts("Here is P:");
		P.print();
		puts("And this is W:");
		W.print();
		puts("The pipeline, altogether:");
		pipeline.print();

		puts("I AM DRAWING THIS LINE:");
		m1.print();
		m2.print();
	    }
	}
    }

    // Draw the line
    glBegin(GL_LINES);
    glVertex3f( m1(0,0), m1(0,1), m1(0,2) );
    glVertex3f( m2(0,0), m2(0,1), m2(0,2) );
    glEnd();
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
    main_window = glutCreateWindow( "2DCube" );

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
    spinner->set_float_limits(2.0f, 8.0f, GLUI_LIMIT_CLAMP);

    new GLUI_Column(control_panel, true);
    GLUI_Rollout *eyePosRollout = new GLUI_Rollout(control_panel, "Eye Position", false);
    GLUI_Spinner *epX = new GLUI_Spinner(eyePosRollout, "X", GLUI_SPINNER_FLOAT, &EyePosX, 0, computeViewerAngle);
    GLUI_Spinner *epY = new GLUI_Spinner(eyePosRollout, "Y", GLUI_SPINNER_FLOAT, &EyePosY, 0, computeViewerAngle);
    GLUI_Spinner *epZ = new GLUI_Spinner(eyePosRollout, "Z", GLUI_SPINNER_FLOAT, &EyePosZ, 0, computeViewerAngle);
    epX->set_float_limits(-100.0f, 100.0f, GLUI_LIMIT_WRAP);
    epY->set_float_limits(-100.0f, 100.0f, GLUI_LIMIT_WRAP);
    epZ->set_float_limits(-100.0f, 100.0f, GLUI_LIMIT_WRAP);
    epX->set_float_val(EyePosX);
    epY->set_float_val(EyePosY);
    epZ->set_float_val(EyePosZ);
    
    GLUI_Rollout *lookAtRollout = new GLUI_Rollout(control_panel, "Looking At", false);
    GLUI_Spinner *laX = new GLUI_Spinner(lookAtRollout, "X", GLUI_SPINNER_FLOAT, &LookAtX, 0, computeViewerAngle);
    GLUI_Spinner *laY = new GLUI_Spinner(lookAtRollout, "Y", GLUI_SPINNER_FLOAT, &LookAtY, 0, computeViewerAngle);
    GLUI_Spinner *laZ = new GLUI_Spinner(lookAtRollout, "Z", GLUI_SPINNER_FLOAT, &LookAtZ, 0, computeViewerAngle);
    laX->set_float_limits(-100.0f, 100.0f, GLUI_LIMIT_WRAP);
    laY->set_float_limits(-100.0f, 100.0f, GLUI_LIMIT_WRAP);
    laZ->set_float_limits(-100.0f, 100.0f, GLUI_LIMIT_WRAP);
    laX->set_float_val(LookAtX);
    laY->set_float_val(LookAtY);
    laZ->set_float_val(LookAtZ);

    new GLUI_Column(control_panel, true);
    GLUI_Rollout *clippingRollout = new GLUI_Rollout(control_panel, "Clipping Parameters", false);
    GLUI_Spinner *thetaSpinner = new GLUI_Spinner(clippingRollout, "Theta", GLUI_SPINNER_FLOAT, &Theta, 0, computeWindowMatrix);
    thetaSpinner->set_float_limits(0.0f, 360.0f, GLUI_LIMIT_WRAP);
    thetaSpinner->set_float_val(Theta);
 
    control_panel->set_main_gfx_window(main_window);

    GLUI_Master.set_glutIdleFunc(spinDisplay);
    glutMainLoop();
    return EXIT_SUCCESS;
} 

/**
 * Test a cross-product and see viewer-parallel axes.
 * */
void testCP() {
    float x, y, z;
    printf("Lookat X: ");
    scanf("%f", &x);
    printf("Lookat Y: ");
    scanf("%f", &y);
    printf("Lookat Z: ");
    scanf("%f", &z);
    
    EyePosX = EyePosY = EyePosZ = 10.0f;

    computeViewerAngle(0);

    printf("Parallel z-vector: <%f,%f,%f>\n", Az, Bz, Cz);
    printf("Parallel x-vector: <%f,%f,%f>\n", Ax, Bx, Cx);
}
