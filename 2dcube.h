typedef enum {
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
    HITHER,
    YON
} direction;

void computeViewerAngle(int id);
void computeViewerMatrix(int id);
void computePerspectiveMatrix(int id);
void computeWindowMatrix(int id);
void drawLine(float, float, float, float, float, float);
int clip(float p1[3], float p2[3], direction d);
/* void clipBounds(float p1[2], float p2[3]); */
/* void clipBounds(float p1[2], float p2[3], direction d); */
void display();






void testClip();
