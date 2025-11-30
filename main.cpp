/*
 * Pixar Luxo Lamp Animation
 * 
 * Features:
 * - Articulated lamp with multiple joints (base, lower arm, upper arm, lampshade)
 * - Interactive joint rotation using arrow keys
 * - Spotlight simulation with directional lighting
 * - Material properties for realistic rendering
 * 
 * Controls:
 * - Number keys (1-4): Select joint to rotate
 *   1: Base rotation
 *   2: Lower arm joint
 *   3: Upper arm joint
 *   4: Lampshade joint
 * - Arrow keys: Rotate selected joint
 *   Left/Right: Rotate around Y-axis
 *   Up/Down: Rotate around X-axis
 * - F key: Toggle spotlight on/off
 * - R key: Reset all joints to default position
 * - ESC: Exit program
 */

#include <GL/glut.h>
#include <cmath>
#include <iostream>

// Window dimensions
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

// Joint selection
enum JointSelection {
    BASE = 0,
    LOWER_ARM = 1,
    UPPER_ARM = 2,
    LAMPSHADE = 3
};

// Lamp joint angles (in degrees)
struct LampJoints {
    float baseRotation;      // Rotation around Y-axis
    float lowerArmAngle;     // Angle at base joint
    float upperArmAngle;     // Angle at middle joint
    float lampshadeAngle;    // Angle at lampshade joint
    float lampshadeRotation; // Rotation of lampshade around its axis
};

// Global variables
LampJoints lampJoints = {0.0f, 45.0f, -60.0f, -30.0f, 0.0f};
JointSelection selectedJoint = BASE;
bool spotlightEnabled = true;
float cameraAngleX = 20.0f;
float cameraAngleY = 30.0f;
float cameraDistance = 15.0f;

// Lamp dimensions
const float BASE_RADIUS = 1.0f;
const float BASE_HEIGHT = 0.3f;
const float ARM_RADIUS = 0.15f;
const float LOWER_ARM_LENGTH = 3.0f;
const float UPPER_ARM_LENGTH = 2.5f;
const float LAMPSHADE_RADIUS = 0.8f;
const float LAMPSHADE_HEIGHT = 1.2f;

// Function prototypes
void init();
void display();
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void specialKeys(int key, int x, int y);
void drawBase();
void drawArm(float length);
void drawJoint();
void drawLampshade();
void drawTable();
void setupLighting();
void setupMaterials();
void drawCylinder(float radius, float height, int slices);
void drawSphere(float radius, int slices, int stacks);

// Initialize OpenGL settings
void init() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);  // Ambient light
    glEnable(GL_LIGHT1);  // Spotlight from lamp
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    
    // Enable blending for better visual effects
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    std::cout << "Pixar Luxo Lamp Animation" << std::endl;
    std::cout << "=========================" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  1-4: Select joint (Base, Lower Arm, Upper Arm, Lampshade)" << std::endl;
    std::cout << "  Arrow Keys: Rotate selected joint" << std::endl;
    std::cout << "  F: Toggle spotlight" << std::endl;
    std::cout << "  R: Reset to default position" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
}

// Setup lighting
void setupLighting() {
    // Ambient light
    GLfloat ambientLight[] = {0.2f, 0.2f, 0.25f, 1.0f};
    GLfloat lightPos0[] = {5.0f, 10.0f, 5.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
    
    // Spotlight from lamp (GL_LIGHT1)
    if (spotlightEnabled) {
        glEnable(GL_LIGHT1);
        
        // Calculate spotlight position and direction based on lamp joints
        glPushMatrix();
        glRotatef(lampJoints.baseRotation, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, BASE_HEIGHT, 0.0f);
        glRotatef(lampJoints.lowerArmAngle, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, LOWER_ARM_LENGTH, 0.0f);
        glRotatef(lampJoints.upperArmAngle, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, UPPER_ARM_LENGTH, 0.0f);
        glRotatef(lampJoints.lampshadeAngle, 1.0f, 0.0f, 0.0f);
        
        // Get current position for spotlight
        GLfloat modelview[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
        GLfloat spotPosition[] = {modelview[12], modelview[13], modelview[14], 1.0f};
        
        glPopMatrix();
        
        // Spotlight properties
        GLfloat spotDiffuse[] = {1.0f, 0.95f, 0.8f, 1.0f};
        GLfloat spotSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
        
        // Calculate spotlight direction
        float totalAngleX = lampJoints.lowerArmAngle + lampJoints.upperArmAngle + lampJoints.lampshadeAngle;
        float radAngleY = lampJoints.baseRotation * M_PI / 180.0f;
        float radAngleX = totalAngleX * M_PI / 180.0f;
        
        GLfloat spotDirection[] = {
            sin(radAngleY) * cos(radAngleX),
            -sin(radAngleX),
            cos(radAngleY) * cos(radAngleX)
        };
        
        glLightfv(GL_LIGHT1, GL_POSITION, spotPosition);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, spotDiffuse);
        glLightfv(GL_LIGHT1, GL_SPECULAR, spotSpecular);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDirection);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0f);
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 15.0f);
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.05f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.01f);
    } else {
        glDisable(GL_LIGHT1);
    }
}

// Draw a cylinder
void drawCylinder(float radius, float height, int slices) {
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluCylinder(quad, radius, radius, height, slices, 1);
    gluDeleteQuadric(quad);
}

// Draw a sphere
void drawSphere(float radius, int slices, int stacks) {
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, radius, slices, stacks);
    gluDeleteQuadric(quad);
}

// Draw lamp base
void drawBase() {
    GLfloat baseMaterial[] = {0.3f, 0.3f, 0.35f, 1.0f};
    GLfloat baseSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat baseShininess[] = {32.0f};
    
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, baseMaterial);
    glMaterialfv(GL_FRONT, GL_SPECULAR, baseSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, baseShininess);
    
    // Draw circular base
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    drawCylinder(BASE_RADIUS, BASE_HEIGHT, 32);
    
    // Base top cap
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    glTranslatef(0.0f, 0.0f, BASE_HEIGHT);
    gluDisk(quad, 0.0f, BASE_RADIUS, 32, 1);
    gluDeleteQuadric(quad);
    glPopMatrix();
}

// Draw arm segment
void drawArm(float length) {
    GLfloat armMaterial[] = {0.4f, 0.4f, 0.45f, 1.0f};
    GLfloat armSpecular[] = {0.6f, 0.6f, 0.6f, 1.0f};
    GLfloat armShininess[] = {64.0f};
    
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, armMaterial);
    glMaterialfv(GL_FRONT, GL_SPECULAR, armSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, armShininess);
    
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    drawCylinder(ARM_RADIUS, length, 16);
    glPopMatrix();
}

// Draw joint sphere
void drawJoint() {
    GLfloat jointMaterial[] = {0.35f, 0.35f, 0.4f, 1.0f};
    GLfloat jointSpecular[] = {0.7f, 0.7f, 0.7f, 1.0f};
    GLfloat jointShininess[] = {96.0f};
    
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, jointMaterial);
    glMaterialfv(GL_FRONT, GL_SPECULAR, jointSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, jointShininess);
    
    drawSphere(ARM_RADIUS * 1.5f, 16, 16);
}

// Draw lampshade
void drawLampshade() {
    GLfloat shadeMaterial[] = {0.9f, 0.9f, 0.95f, 1.0f};
    GLfloat shadeSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat shadeShininess[] = {128.0f};
    
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, shadeMaterial);
    glMaterialfv(GL_FRONT, GL_SPECULAR, shadeSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shadeShininess);
    
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    
    // Draw cone for lampshade
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluCylinder(quad, LAMPSHADE_RADIUS, LAMPSHADE_RADIUS * 0.4f, LAMPSHADE_HEIGHT, 32, 1);
    
    // Draw rim
    gluDisk(quad, LAMPSHADE_RADIUS * 0.4f, LAMPSHADE_RADIUS, 32, 1);
    
    // Inner glow when light is on
    if (spotlightEnabled) {
        glDisable(GL_LIGHTING);
        glColor4f(1.0f, 0.95f, 0.7f, 0.8f);
        glTranslatef(0.0f, 0.0f, LAMPSHADE_HEIGHT * 0.5f);
        gluDisk(quad, 0.0f, LAMPSHADE_RADIUS * 0.6f, 32, 1);
        glEnable(GL_LIGHTING);
    }
    
    gluDeleteQuadric(quad);
    glPopMatrix();
}

// Draw table surface
void drawTable() {
    GLfloat tableMaterial[] = {0.6f, 0.4f, 0.2f, 1.0f};
    GLfloat tableSpecular[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat tableShininess[] = {16.0f};
    
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, tableMaterial);
    glMaterialfv(GL_FRONT, GL_SPECULAR, tableSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, tableShininess);
    
    glPushMatrix();
    glTranslatef(0.0f, -0.1f, 0.0f);
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-10.0f, 0.0f, -10.0f);
        glVertex3f(-10.0f, 0.0f, 10.0f);
        glVertex3f(10.0f, 0.0f, 10.0f);
        glVertex3f(10.0f, 0.0f, -10.0f);
    glEnd();
    glPopMatrix();
}

// Display callback
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Set up camera
    gluLookAt(
        cameraDistance * cos(cameraAngleY * M_PI / 180.0f) * sin(cameraAngleX * M_PI / 180.0f),
        cameraDistance * sin(cameraAngleY * M_PI / 180.0f),
        cameraDistance * cos(cameraAngleY * M_PI / 180.0f) * cos(cameraAngleX * M_PI / 180.0f),
        0.0f, 3.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    );
    
    setupLighting();
    
    // Draw table
    drawTable();
    
    // Draw lamp
    glPushMatrix();
    
    // Base rotation
    glRotatef(lampJoints.baseRotation, 0.0f, 1.0f, 0.0f);
    
    // Highlight selected joint
    if (selectedJoint == BASE) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f);
        glPushMatrix();
        glTranslatef(0.0f, BASE_HEIGHT * 0.5f, 0.0f);
        glutWireCube(BASE_RADIUS * 2.2f);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }
    
    drawBase();
    
    // Lower arm joint
    glTranslatef(0.0f, BASE_HEIGHT, 0.0f);
    
    if (selectedJoint == LOWER_ARM) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f);
        glutWireSphere(ARM_RADIUS * 2.5f, 16, 16);
        glEnable(GL_LIGHTING);
    }
    
    drawJoint();
    glRotatef(lampJoints.lowerArmAngle, 1.0f, 0.0f, 0.0f);
    drawArm(LOWER_ARM_LENGTH);
    
    // Upper arm joint
    glTranslatef(0.0f, LOWER_ARM_LENGTH, 0.0f);
    
    if (selectedJoint == UPPER_ARM) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f);
        glutWireSphere(ARM_RADIUS * 2.5f, 16, 16);
        glEnable(GL_LIGHTING);
    }
    
    drawJoint();
    glRotatef(lampJoints.upperArmAngle, 1.0f, 0.0f, 0.0f);
    drawArm(UPPER_ARM_LENGTH);
    
    // Lampshade joint
    glTranslatef(0.0f, UPPER_ARM_LENGTH, 0.0f);
    
    if (selectedJoint == LAMPSHADE) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f);
        glutWireSphere(ARM_RADIUS * 2.5f, 16, 16);
        glEnable(GL_LIGHTING);
    }
    
    drawJoint();
    glRotatef(lampJoints.lampshadeAngle, 1.0f, 0.0f, 0.0f);
    glRotatef(lampJoints.lampshadeRotation, 0.0f, 1.0f, 0.0f);
    drawLampshade();
    
    glPopMatrix();
    
    // Display current selection
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(10, WINDOW_HEIGHT - 20);
    const char* jointNames[] = {"Base", "Lower Arm", "Upper Arm", "Lampshade"};
    std::string info = "Selected Joint: " + std::string(jointNames[selectedJoint]);
    for (char c : info) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    
    glRasterPos2f(10, WINDOW_HEIGHT - 45);
    std::string lightInfo = spotlightEnabled ? "Spotlight: ON" : "Spotlight: OFF";
    for (char c : lightInfo) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
    
    glutSwapBuffers();
}

// Reshape callback
void reshape(int width, int height) {
    if (height == 0) height = 1;
    float aspect = (float)width / (float)height;
    
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, aspect, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

// Keyboard callback
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '1':
            selectedJoint = BASE;
            std::cout << "Selected: Base" << std::endl;
            break;
        case '2':
            selectedJoint = LOWER_ARM;
            std::cout << "Selected: Lower Arm" << std::endl;
            break;
        case '3':
            selectedJoint = UPPER_ARM;
            std::cout << "Selected: Upper Arm" << std::endl;
            break;
        case '4':
            selectedJoint = LAMPSHADE;
            std::cout << "Selected: Lampshade" << std::endl;
            break;
        case 'f':
        case 'F':
            spotlightEnabled = !spotlightEnabled;
            std::cout << "Spotlight: " << (spotlightEnabled ? "ON" : "OFF") << std::endl;
            break;
        case 'r':
        case 'R':
            lampJoints.baseRotation = 0.0f;
            lampJoints.lowerArmAngle = 45.0f;
            lampJoints.upperArmAngle = -60.0f;
            lampJoints.lampshadeAngle = -30.0f;
            lampJoints.lampshadeRotation = 0.0f;
            std::cout << "Reset to default position" << std::endl;
            break;
        case 27: // ESC
            exit(0);
            break;
    }
    glutPostRedisplay();
}

// Special keys callback
void specialKeys(int key, int x, int y) {
    const float rotationStep = 3.0f;
    
    switch (key) {
        case GLUT_KEY_LEFT:
            if (selectedJoint == BASE) {
                lampJoints.baseRotation -= rotationStep;
            } else if (selectedJoint == LAMPSHADE) {
                lampJoints.lampshadeRotation -= rotationStep;
            }
            break;
        case GLUT_KEY_RIGHT:
            if (selectedJoint == BASE) {
                lampJoints.baseRotation += rotationStep;
            } else if (selectedJoint == LAMPSHADE) {
                lampJoints.lampshadeRotation += rotationStep;
            }
            break;
        case GLUT_KEY_UP:
            if (selectedJoint == LOWER_ARM) {
                lampJoints.lowerArmAngle += rotationStep;
                lampJoints.lowerArmAngle = fmin(lampJoints.lowerArmAngle, 90.0f);
            } else if (selectedJoint == UPPER_ARM) {
                lampJoints.upperArmAngle += rotationStep;
                lampJoints.upperArmAngle = fmin(lampJoints.upperArmAngle, 90.0f);
            } else if (selectedJoint == LAMPSHADE) {
                lampJoints.lampshadeAngle += rotationStep;
                lampJoints.lampshadeAngle = fmin(lampJoints.lampshadeAngle, 45.0f);
            }
            break;
        case GLUT_KEY_DOWN:
            if (selectedJoint == LOWER_ARM) {
                lampJoints.lowerArmAngle -= rotationStep;
                lampJoints.lowerArmAngle = fmax(lampJoints.lowerArmAngle, -10.0f);
            } else if (selectedJoint == UPPER_ARM) {
                lampJoints.upperArmAngle -= rotationStep;
                lampJoints.upperArmAngle = fmax(lampJoints.upperArmAngle, -120.0f);
            } else if (selectedJoint == LAMPSHADE) {
                lampJoints.lampshadeAngle -= rotationStep;
                lampJoints.lampshadeAngle = fmax(lampJoints.lampshadeAngle, -90.0f);
            }
            break;
    }
    
    glutPostRedisplay();
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Pixar Luxo Lamp Animation");
    
    init();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    
    glutMainLoop();
    return 0;
}
