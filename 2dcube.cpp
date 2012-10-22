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
GLfloat EyePosX = 5.0f;
GLfloat EyePosY = 5.0f;
GLfloat EyePosZ = 2.5f;
GLfloat LookAtX, LookAtY, LookAtZ;
GLfloat Wl, Wr, Wt, Wb, Vl, Vr, Vt, Vb;
GLfloat Ax, Bx, Cx, Ay, By, Cy, Az, Bz, Cz;
GLfloat ViewPlaneDist = 10.0f;
GLfloat HitherPlaneDist = 2.0f;
GLfloat YonPlaneDist = 15.0f;

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
#define VIEWPORT_MARGIN 20.0f
#define CubeX 0
#define CubeY 0
#define CubeZ 0

/* setupViewport(width, height)
 * 
 * Set up the viewport.
 */
void setupViewport(int w, int h) {
    glViewport(0, 0, w, h); 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Wr = w;
    Wb = h;
    Wt = 0;
    Wl = 0;

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
    // glClearColor(8.0, 0.4, 0.0, 1.0);
    glClearColor(0, 0, 0, 1);
    glColor3f(0.5, 0.5, 0.5);
    setupViewport(INITIAL_WIDTH, INITIAL_HEIGHT);

    LookAtX = LookAtY = LookAtZ = 0.0f;

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
    float pentries[] = { ViewPlaneDist, 0, 0, 0,
			 0, ViewPlaneDist, 0, 0, 
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

    // The up-vector
    GLfloat UPA = 0;
    GLfloat UPB = 0;
    GLfloat UPC = 1;

    // x-vector is cross-product of Z and UP vectors
    float zvector[] = { Az, Bz, Cz };
    float upvector[] = { UPA, UPB, UPC };
    float *xvector = crossProduct(zvector, upvector);
 
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

    display();
}

/**
 * Recomputes the Matrix W based on current parameters.
 * */
void computeWindowMatrix(int id) {
    // Find viewplane variables
    Vb = Vl = -ViewPlaneDist * tan(radians(Theta));
    Vr = Vt = -Vb;
    W(0,0) = (Wr - Wl)/(Vr - Vl);
    W(1,1) = (Wt - Wb)/(Vt - Vb);
    W(3,0) = (Wl*Vr - Vl*Wr)/(Vr - Vl);
    W(3,1) = (Wb*Vt - Vb*Wt)/(Vt - Vb);

    display();
}

void computePerspectiveMatrix(int id) {
    /* transform matrix P */
    float pentries[] = { ViewPlaneDist, 0, 0, 0,
			 0, ViewPlaneDist, 0, 0,
			 0, 0, YonPlaneDist/(YonPlaneDist-HitherPlaneDist), 1,
			 0, 0, -HitherPlaneDist*YonPlaneDist/(YonPlaneDist-HitherPlaneDist), 0 };
    P << pentries;

    display();
}

void refreshWindowAndPerspective(int id) {
    computeWindowMatrix(id);
    computePerspectiveMatrix(id);
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
    glVertex2f(VIEWPORT_MARGIN, VIEWPORT_MARGIN);
    glVertex2f(VIEWPORT_MARGIN, Wb - VIEWPORT_MARGIN);
    glVertex2f(Wr - VIEWPORT_MARGIN, Wb - VIEWPORT_MARGIN);
    glVertex2f(Wr - VIEWPORT_MARGIN, VIEWPORT_MARGIN);
    glEnd();

    /* draw the cube */
    glColor3f(1.0,0.0,0.0);
    drawLine(0, 0, 0,
    	     CubeSize, 0, 0);
    drawLine(CubeSize, 0, 0,
    	     CubeSize, CubeSize, 0);
    drawLine(CubeSize, CubeSize, 0,
    	     0, CubeSize, 0);
    drawLine(0, CubeSize, 0,
    	     0, 0, 0);
    glColor3f(1.0,0.3,0.0);
    drawLine(CubeSize,0,CubeSize,
    	     CubeSize,CubeSize,0);

    glColor3f(0.0,1.0,0.0);
    drawLine(0, 0, CubeSize,
    	     CubeSize, 0, CubeSize);
    drawLine(CubeSize, 0, CubeSize,
    	     CubeSize, CubeSize, CubeSize);
    drawLine(CubeSize, CubeSize, CubeSize,
    	     0, CubeSize, CubeSize);
    drawLine(0, CubeSize, CubeSize,
    	     0, 0, CubeSize);

    glColor3f(0.0,1.0,1.0);
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

    // Draw the line, if it doesn't clip off the screen
    float p1[3] = { m1(0,0), m1(0,1), m1(0,2) };
    float p2[3] = { m2(0,0), m2(0,1), m2(0,2) };

    if ( clip(p1, p2, TOP) &&
    	 clip(p1, p2, BOTTOM) &&
    	 clip(p1, p2, LEFT) &&
    	 clip(p1, p2, RIGHT) &&
    	 clip(p1, p2, HITHER) &&
    	 clip(p1, p2, YON) ) {

    	glBegin(GL_LINES);
    	glVertex2f( p1[0], p1[1] );
	glVertex2f( p2[0], p2[1] );
    	glEnd();
    }
}

float len(float p1[3], float p2[3]) {
    float x = p1[0] - p2[0];
    float y = p1[1] - p2[1];
    float z = p1[2] - p2[2];
    return (float)sqrt(x*x + y*y + z*z);
}

/*
 * clip
 *
 * Takes two 3-arrays by reference. clip will change these values to be ones that
 * are clipped within the viewport, hither, and yon planes. Returns 0 if the line
 * is clipped entirely, 1 otherwise.
 * */
int clip(float p1[3], float p2[3], direction d) {
    Matrix normal(3,1);
    Matrix m0(1,3,p1);
    Matrix v(1,3);
    float v_entries[] = { p2[0] - p1[0],
			  p2[1] - p1[1],
			  p2[2] - p1[2] };
    v << v_entries;
    float normal_entries[3] = { 0, 0, 0 };
    float D;

    // Set normal vector, D, and check if the entire line is clipped
    switch ( d ) {
    case TOP:
    {
	if ( p1[1] > Wb - VIEWPORT_MARGIN && p2[1] > Wb - VIEWPORT_MARGIN ) {
	    return 0;
	}
	normal_entries[1] = 1;
	D = Wb - VIEWPORT_MARGIN;
    }
	break;
    case BOTTOM:
    {
	if ( p1[1] < VIEWPORT_MARGIN && p2[1] < VIEWPORT_MARGIN ) {
	    return 0;
	}
	normal_entries[1] = 1;
	D = VIEWPORT_MARGIN;
    }
	break;
    case LEFT:
    {
	if ( p1[0] < VIEWPORT_MARGIN && p2[0] < VIEWPORT_MARGIN ) {
	    return 0;
	}
	normal_entries[0] = 1;
	D = VIEWPORT_MARGIN;
    }
	break;
    case RIGHT:
    {
	if ( p1[0] > Wr - VIEWPORT_MARGIN && p2[0] > Wr - VIEWPORT_MARGIN ) {
	    return 0;
	}
	normal_entries[0] = 1;
	D = Wr - VIEWPORT_MARGIN;
    }
	break;
    case HITHER:
    {
	if ( p1[2] < 0 && p2[2] < 0 ) {
	    return 0;
	}
	normal_entries[2] = 1;
	D = 0;
    }
        break;
    case YON:
    {
	if ( p1[2] > 1 && p2[2] > 1) {
	    return 0;
	}
	normal_entries[2] = 1;
	D = 1;
    }
	break;
    }
    normal << normal_entries;
    
    // Check if there is nothing to be clipped. If so, return.
    float normalDotV = (v*normal)(0,0);
    if ( normalDotV == 0.0f ) return 1;

    // Get the clipped point
    float t = (D - (m0*normal)(0,0))/normalDotV;
    Matrix clippedPointVector = v*t + m0;
    float clippedPoint[3] = { clippedPointVector(0,0),
			      clippedPointVector(0,1),
			      clippedPointVector(0,2) };
 
    // We assume that at least part of the line is visible here.
    switch ( d ) {
    case TOP:
    {
	if ( p1[1] > Wb - VIEWPORT_MARGIN ) {
	    std::copy(clippedPoint, clippedPoint+3, p1);
	} else if ( p2[1] > Wb - VIEWPORT_MARGIN ) {
	    std::copy(clippedPoint, clippedPoint+3, p2);
	}
    }
	break;
    case BOTTOM:
    {
	if ( p1[1] < VIEWPORT_MARGIN ) {
	    std::copy(clippedPoint, clippedPoint+3, p1);
	} else if ( p2[1] < VIEWPORT_MARGIN ) {
	    std::copy(clippedPoint, clippedPoint+3, p2);
	}
    }
	break;
    case LEFT:
    {
	if ( p1[0] < VIEWPORT_MARGIN ) {
	    std::copy(clippedPoint, clippedPoint+3, p1);
	} else if ( p2[0] < VIEWPORT_MARGIN ) {
	    std::copy(clippedPoint, clippedPoint+3, p2);
	}
    }
	break;
    case RIGHT:
    {
	if ( p1[0] > Wr - VIEWPORT_MARGIN ) {
	    std::copy(clippedPoint, clippedPoint+3, p1);
	} else if ( p2[0] > Wr - VIEWPORT_MARGIN) {
	    std::copy(clippedPoint, clippedPoint+3, p2);
	}
    }
	break;
    case HITHER:
    {
	if ( p1[2] < 0 ) {
	    std::copy(clippedPoint, clippedPoint+3, p1);
	} else if ( p2[2] < 0 ) {
	    std::copy(clippedPoint, clippedPoint+3, p2);
	}

    }
	break;
    case YON:
    {
	if ( p1[2] > 1 ) {
	    std::copy(clippedPoint, clippedPoint+3, p1);
	} else if ( p2[2] > 1 ) {
	    std::copy(clippedPoint, clippedPoint+3, p2);
	}

    }
	break;
    }

    return 1;
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
    GLUI_Spinner *hitherDistRollout = new GLUI_Spinner(clippingRollout, "Hither", GLUI_SPINNER_FLOAT, &HitherPlaneDist, 0, computePerspectiveMatrix);
    GLUI_Spinner *yonDistRollout = new GLUI_Spinner(clippingRollout, "Yon", GLUI_SPINNER_FLOAT, &YonPlaneDist, 0, computePerspectiveMatrix);
    GLUI_Spinner *viewDistRollout = new GLUI_Spinner(clippingRollout, "View", GLUI_SPINNER_FLOAT, &ViewPlaneDist, 0, refreshWindowAndPerspective);
    hitherDistRollout->set_float_limits(0.0f, 5.0f, GLUI_LIMIT_CLAMP);
    viewDistRollout->set_float_limits(5.0f, 10.0f, GLUI_LIMIT_CLAMP);
    yonDistRollout->set_float_limits(10.0f, 100.0f, GLUI_LIMIT_CLAMP);
    
    GLUI_Spinner *thetaSpinner = new GLUI_Spinner(control_panel, "Theta", GLUI_SPINNER_FLOAT, &Theta, 0, computeWindowMatrix);
    thetaSpinner->set_float_limits(0.0f, 360.0f, GLUI_LIMIT_WRAP);
    thetaSpinner->set_float_val(Theta);
 
    control_panel->set_main_gfx_window(main_window);

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

void testClip() {
    float p1[] = { 30, 30, 10 };
    float p2[] = { 60, 0, 5 };

    if ( clip(p1, p2, BOTTOM) ) {
	puts("Line does display:");
	printf("(%f,%f,%f) - (%f,%f,%f)\n",
	       p1[0], p1[1], p1[2],
	       p2[0], p2[1], p2[2]);
    } else {
	puts("The line is entirely clipped.");
    }
}
