/*
 * Pixar Luxo Lamp Animation
 *
 * This program simulates a Pixar-style articulated desk lamp with:
 * - Multiple rotatable joints (base, lower arm, upper arm, lampshade)
 * - Realistic spotlight that follows the lampshade direction
 * - Material properties for metallic appearance
 * - Interactive controls for manipulating each joint
 *
 * Controls:
 * - 1-4: Select joint (Base, Lower Arm, Upper Arm, Lampshade)
 * - Arrow keys: Rotate selected joint
 * - F: Toggle spotlight on/off
 * - R: Reset to default position
 * - ESC: Exit
 */

#include <GL/glut.h>
#include <cmath>
#include <iostream>

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

// Enum for selecting which joint to rotate
enum JointSelection
{
    BASE = 0,      // Base rotation (Y-axis)
    LOWER_ARM = 1, // Lower arm joint (X-axis)
    UPPER_ARM = 2, // Upper arm joint (X-axis)
    LAMPSHADE = 3  // Lampshade joint (X and Y-axis)
};

// Structure to hold all lamp joint angles (in degrees)
struct LampJoints
{
    float baseRotation;      // Rotation of entire lamp around Y-axis
    float lowerArmAngle;     // Angle of lower arm from base
    float upperArmAngle;     // Angle of upper arm from lower arm
    float lampshadeAngle;    // Tilt angle of lampshade
    float lampshadeRotation; // Rotation of lampshade around its own axis
};

// Initial lamp configuration
LampJoints lampJoints = {0.0f, 45.0f, -60.0f, -30.0f, 0.0f};
JointSelection selectedJoint = BASE;
bool spotlightEnabled = true;

// Camera settings
float cameraAngleX = 20.0f;
float cameraAngleY = 30.0f;
float cameraDistance = 15.0f;

// Lamp physical dimensions
const float BASE_RADIUS = 1.0f;
const float BASE_HEIGHT = 0.3f;
const float ARM_RADIUS = 0.15f;
const float LOWER_ARM_LENGTH = 3.0f;
const float UPPER_ARM_LENGTH = 2.5f;
const float LAMPSHADE_RADIUS = 0.8f;
const float LAMPSHADE_HEIGHT = 1.2f;

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

/**
 * Initialize OpenGL settings and display control instructions
 */
void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
    glEnable(GL_DEPTH_TEST);              // Enable depth testing for 3D
    glEnable(GL_LIGHTING);                // Enable lighting calculations
    glEnable(GL_LIGHT0);                  // Ambient light source
    glEnable(GL_LIGHT1);                  // Spotlight from lamp
    glEnable(GL_COLOR_MATERIAL);          // Allow materials to be set by glColor
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);  // Normalize normals after transformations
    glShadeModel(GL_SMOOTH); // Smooth shading for better appearance
    glEnable(GL_BLEND);      // Enable transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Print control instructions to console
    std::cout << "Pixar Luxo Lamp Animation" << std::endl;
    std::cout << "=========================" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  1-4: Select joint (Base, Lower Arm, Upper Arm, Lampshade)" << std::endl;
    std::cout << "  Arrow Keys: Rotate selected joint" << std::endl;
    std::cout << "  F: Toggle spotlight" << std::endl;
    std::cout << "  R: Reset to default position" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
}

/**
 * Configure two light sources:
 * - GL_LIGHT0: Weak ambient light to create dark scene
 * - GL_LIGHT1: Bright spotlight emanating from lampshade
 */
void setupLighting()
{
    // --------------------------------------------------------------------
    // GL_LIGHT0: Low ambient light to enhance spotlight effect
    // --------------------------------------------------------------------
    GLfloat ambientLight[] = {0.05f, 0.05f, 0.05f, 1.0f};
    GLfloat lightPos0[] = {5.0f, 10.0f, 5.0f, 0.0f}; // Directional light
    GLfloat diffuseLight[] = {0.1f, 0.1f, 0.1f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

    // --------------------------------------------------------------------
    // GL_LIGHT1: Dynamic spotlight from lampshade
    // --------------------------------------------------------------------
    if (spotlightEnabled)
    {
        glEnable(GL_LIGHT1);

        // Save current matrix and start fresh
        glPushMatrix();
        glLoadIdentity();

        // Apply the same transformation hierarchy as the lamp geometry
        // This ensures the light position and direction match the lampshade
        glRotatef(lampJoints.baseRotation, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, BASE_HEIGHT, 0.0f);
        glRotatef(lampJoints.lowerArmAngle, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, LOWER_ARM_LENGTH, 0.0f);
        glRotatef(lampJoints.upperArmAngle, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, UPPER_ARM_LENGTH, 0.0f);
        glRotatef(lampJoints.lampshadeAngle, 1.0f, 0.0f, 0.0f);
        glRotatef(lampJoints.lampshadeRotation, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, ARM_RADIUS * 1.5f, 0.0f); // Move past joint sphere

        // Align coordinate system with lampshade geometry
        // The lampshade is drawn with -90° rotation, so we match that here
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

        // Position light source inside lampshade at 60% depth
        // This creates the effect of light emanating from within
        glTranslatef(0.0f, 0.0f, LAMPSHADE_HEIGHT * 0.6f);

        // Extract the final world-space position and direction from the matrix
        GLfloat modelview[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

        // Position is the translation component (4th column)
        GLfloat spotPosition[] = {modelview[12], modelview[13], modelview[14], 1.0f};

        // Direction is the transformed Z-axis (3rd column)
        // After our transformations, +Z points in the direction the lampshade opens
        GLfloat spotDirection[] = {modelview[8], modelview[9], modelview[10]};

        glPopMatrix(); // Restore previous matrix

        // Spotlight properties: Warm, bright yellow-white light
        GLfloat spotDiffuse[] = {3.0f, 2.5f, 1.5f, 1.0f};  // Warm yellow-white
        GLfloat spotSpecular[] = {2.0f, 2.0f, 2.0f, 1.0f}; // White highlights

        // Apply all spotlight properties
        glLightfv(GL_LIGHT1, GL_POSITION, spotPosition);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, spotDiffuse);
        glLightfv(GL_LIGHT1, GL_SPECULAR, spotSpecular);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDirection);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 60.0f);   // 60° cone angle
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 15.0f); // Moderate falloff
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.5f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.02f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.005f);
    }
    else
    {
        glDisable(GL_LIGHT1);
    }
}

/**
 * Draw a cylinder using GLU quadrics
 * @param radius - Cylinder radius
 * @param height - Cylinder height
 * @param slices - Number of subdivisions around the Z-axis
 */
void drawCylinder(float radius, float height, int slices)
{
    GLUquadric *quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH); // Generate smooth normals
    gluCylinder(quad, radius, radius, height, slices, 1);
    gluDeleteQuadric(quad);
}

/**
 * Draw a sphere using GLU quadrics
 * @param radius - Sphere radius
 * @param slices - Number of subdivisions around the Z-axis
 * @param stacks - Number of subdivisions along the Z-axis
 */
void drawSphere(float radius, int slices, int stacks)
{
    GLUquadric *quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH); // Generate smooth normals
    gluSphere(quad, radius, slices, stacks);
    gluDeleteQuadric(quad);
}

/**
 * Draw the circular base of the lamp
 * Material: Dark metallic gray with high specular for metal appearance
 */
void drawBase()
{
    // Set material properties for dark metal
    GLfloat baseMaterial[] = {0.2f, 0.2f, 0.22f, 1.0f};
    GLfloat baseSpecular[] = {0.9f, 0.9f, 0.95f, 1.0f}; // High shine
    GLfloat baseShininess[] = {80.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, baseMaterial);
    glMaterialfv(GL_FRONT, GL_SPECULAR, baseSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, baseShininess);

    glPushMatrix();
    // Rotate -90° so cylinder points upward (Y-axis)
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    drawCylinder(BASE_RADIUS, BASE_HEIGHT, 32);

    // Draw top cap to close the cylinder
    GLUquadric *quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    glTranslatef(0.0f, 0.0f, BASE_HEIGHT);
    gluDisk(quad, 0.0f, BASE_RADIUS, 32, 1);
    gluDeleteQuadric(quad);
    glPopMatrix();
}

/**
 * Draw an arm segment (cylinder)
 * Material: Dark metallic with blue-gray tint
 * @param length - Length of the arm segment
 */
void drawArm(float length)
{
    // Set material properties for metallic arm
    GLfloat armMaterial[] = {0.25f, 0.25f, 0.28f, 1.0f};
    GLfloat armSpecular[] = {0.95f, 0.95f, 1.0f, 1.0f}; // Very shiny
    GLfloat armShininess[] = {100.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, armMaterial);
    glMaterialfv(GL_FRONT, GL_SPECULAR, armSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, armShininess);

    glPushMatrix();
    // Rotate so cylinder extends along Y-axis
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    drawCylinder(ARM_RADIUS, length, 16);
    glPopMatrix();
}

/**
 * Draw a joint sphere that connects arm segments
 * Material: Polished dark metal with chrome-like finish
 */
void drawJoint()
{
    // Set material properties for chrome-like joint
    GLfloat jointMaterial[] = {0.22f, 0.22f, 0.25f, 1.0f};
    GLfloat jointSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Maximum shine
    GLfloat jointShininess[] = {120.0f};                // Very sharp highlights

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, jointMaterial);
    glMaterialfv(GL_FRONT, GL_SPECULAR, jointSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, jointShininess);

    // Joint sphere is slightly larger than arm radius
    drawSphere(ARM_RADIUS * 1.5f, 16, 16);
}

/**
 * Draw the lampshade (cone shape)
 * Material: Dark gray with slight blue tint
 * Shape: Narrow at top (joint), wide at bottom (opening)
 */
void drawLampshade()
{
    // Set material properties for lampshade
    GLfloat shadeMaterial[] = {0.3f, 0.3f, 0.35f, 1.0f};
    GLfloat shadeSpecular[] = {0.8f, 0.8f, 0.85f, 1.0f};
    GLfloat shadeShininess[] = {90.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, shadeMaterial);
    glMaterialfv(GL_FRONT, GL_SPECULAR, shadeSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shadeShininess);

    glPushMatrix();
    // Move past the joint sphere
    glTranslatef(0.0f, ARM_RADIUS * 1.5f, 0.0f);
    // Rotate -90° so the cone points downward
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    GLUquadric *quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);

    // Draw cone: narrow at top (0.4 * radius), wide at bottom (radius)
    gluCylinder(quad, LAMPSHADE_RADIUS * 0.4f, LAMPSHADE_RADIUS, LAMPSHADE_HEIGHT, 32, 1);

    gluDisk(quad, 0.0f, LAMPSHADE_RADIUS * 0.4f, 32, 1);

    // Draw inner glow at bottom opening when spotlight is on
    if (spotlightEnabled)
    {
        glDisable(GL_LIGHTING);                     // Draw unlit for glowing effect
        glColor4f(1.0f, 0.9f, 0.2f, 0.9f);          // Bright warm yellow
        glTranslatef(0.0f, 0.0f, LAMPSHADE_HEIGHT); // Move to bottom opening
        gluDisk(quad, 0.0f, LAMPSHADE_RADIUS * 0.5f, 32, 1);
        glEnable(GL_LIGHTING);
    }

    gluDeleteQuadric(quad);
    glPopMatrix();
}

/**
 * Draw the table surface as a subdivided grid
 * Subdivision improves per-vertex lighting calculation, making the spotlight
 * appear as a smooth gradient instead of interpolated across 4 corners
 */
void drawTable()
{
    // Set material properties for matte table surface
    GLfloat tableMaterial[] = {0.4f, 0.4f, 0.4f, 1.0f}; // Medium gray
    GLfloat tableSpecular[] = {0.2f, 0.2f, 0.2f, 1.0f}; // Low specular
    GLfloat tableShininess[] = {10.0f};                 // Dull surface

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, tableMaterial);
    glMaterialfv(GL_FRONT, GL_SPECULAR, tableSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, tableShininess);

    glPushMatrix();
    glTranslatef(0.0f, -0.1f, 0.0f); // Slightly below origin

    // Grid parameters for table subdivision
    float start = -10.0f;
    float end = 10.0f;
    float step = 0.5f; // Smaller step = finer grid = better lighting quality

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f); // All vertices have upward normal

    // Draw grid of small quads instead of one large quad
    // This allows OpenGL to calculate lighting at more vertices
    for (float x = start; x < end; x += step)
    {
        for (float z = start; z < end; z += step)
        {
            glVertex3f(x, 0.0f, z);
            glVertex3f(x, 0.0f, z + step);
            glVertex3f(x + step, 0.0f, z + step);
            glVertex3f(x + step, 0.0f, z);
        }
    }
    glEnd();

    glPopMatrix();
}

/**
 * Main display callback - renders the entire scene
 * Hierarchy: Table -> Lamp (Base -> LowerArm -> UpperArm -> Lampshade)
 */
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Position camera using spherical coordinates
    gluLookAt(
        cameraDistance * cos(cameraAngleY * M_PI / 180.0f) * sin(cameraAngleX * M_PI / 180.0f),
        cameraDistance * sin(cameraAngleY * M_PI / 180.0f),
        cameraDistance * cos(cameraAngleY * M_PI / 180.0f) * cos(cameraAngleX * M_PI / 180.0f),
        0.0f, 3.0f, 0.0f, // Look at point slightly above origin
        0.0f, 1.0f, 0.0f  // Up vector
    );

    setupLighting();
    drawTable();

    glPushMatrix();

    // Level 1: Base rotation (Y-axis)
    glRotatef(lampJoints.baseRotation, 0.0f, 1.0f, 0.0f);

    // Draw selection highlight for base
    if (selectedJoint == BASE)
    {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow wireframe
        glPushMatrix();
        glTranslatef(0.0f, BASE_HEIGHT * 0.5f, 0.0f);
        glutWireCube(BASE_RADIUS * 2.2f);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }

    drawBase();
    glTranslatef(0.0f, BASE_HEIGHT, 0.0f); // Move up to top of base

    // Level 2: Lower arm joint
    if (selectedJoint == LOWER_ARM)
    {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow wireframe
        glutWireSphere(ARM_RADIUS * 2.5f, 16, 16);
        glEnable(GL_LIGHTING);
    }

    drawJoint();
    glRotatef(lampJoints.lowerArmAngle, 1.0f, 0.0f, 0.0f); // Rotate lower arm
    drawArm(LOWER_ARM_LENGTH);
    glTranslatef(0.0f, LOWER_ARM_LENGTH, 0.0f); // Move to end of lower arm

    // Level 3: Upper arm joint
    if (selectedJoint == UPPER_ARM)
    {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow wireframe
        glutWireSphere(ARM_RADIUS * 2.5f, 16, 16);
        glEnable(GL_LIGHTING);
    }

    drawJoint();
    glRotatef(lampJoints.upperArmAngle, 1.0f, 0.0f, 0.0f); // Rotate upper arm
    drawArm(UPPER_ARM_LENGTH);
    glTranslatef(0.0f, UPPER_ARM_LENGTH, 0.0f); // Move to end of upper arm

    // Level 4: Lampshade joint
    if (selectedJoint == LAMPSHADE)
    {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow wireframe
        glutWireSphere(ARM_RADIUS * 2.5f, 16, 16);
        glEnable(GL_LIGHTING);
    }

    drawJoint();
    glRotatef(lampJoints.lampshadeAngle, 1.0f, 0.0f, 0.0f);    // Tilt
    glRotatef(lampJoints.lampshadeRotation, 0.0f, 1.0f, 0.0f); // Spin
    drawLampshade();

    glPopMatrix();

    // Draw 2D UI text overlay
    glDisable(GL_LIGHTING);

    // Switch to orthographic projection for 2D text
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Display selected joint name
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(10, WINDOW_HEIGHT - 20);
    const char *jointNames[] = {"Base", "Lower Arm", "Upper Arm", "Lampshade"};
    std::string info = "Selected Joint: " + std::string(jointNames[selectedJoint]);
    for (char c : info)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Display spotlight status
    glRasterPos2f(10, WINDOW_HEIGHT - 45);
    std::string lightInfo = spotlightEnabled ? "Spotlight: ON" : "Spotlight: OFF";
    for (char c : lightInfo)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Restore previous projection
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);

    glutSwapBuffers(); // Swap front and back buffers
}

/**
 * Reshape callback - called when window is resized
 * @param width - New window width
 * @param height - New window height
 */
void reshape(int width, int height)
{
    if (height == 0)
        height = 1; // Prevent division by zero
    float aspect = (float)width / (float)height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, aspect, 0.1f, 100.0f); // 45° FOV
    glMatrixMode(GL_MODELVIEW);
}

/**
 * Keyboard callback - handles number keys and special commands
 * @param key - ASCII character code
 * @param x - Mouse X position (unused)
 * @param y - Mouse Y position (unused)
 */
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
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
        // Reset all joints to default configuration
        lampJoints.baseRotation = 0.0f;
        lampJoints.lowerArmAngle = 45.0f;
        lampJoints.upperArmAngle = -60.0f;
        lampJoints.lampshadeAngle = -30.0f;
        lampJoints.lampshadeRotation = 0.0f;
        std::cout << "Reset to default position" << std::endl;
        break;
    case 27: // ESC key
        exit(0);
        break;
    }
    glutPostRedisplay(); // Request scene redraw
}

/**
 * Special keys callback - handles arrow keys for joint rotation
 * @param key - GLUT special key code
 * @param x - Mouse X position (unused)
 * @param y - Mouse Y position (unused)
 */
void specialKeys(int key, int x, int y)
{
    const float rotationStep = 3.0f; // Degrees per keypress

    switch (key)
    {
    case GLUT_KEY_LEFT:
        // Base and lampshade use left/right for Y-axis rotation
        if (selectedJoint == BASE)
        {
            lampJoints.baseRotation -= rotationStep;
        }
        else if (selectedJoint == LAMPSHADE)
        {
            lampJoints.lampshadeRotation -= rotationStep;
        }
        break;
    case GLUT_KEY_RIGHT:
        if (selectedJoint == BASE)
        {
            lampJoints.baseRotation += rotationStep;
        }
        else if (selectedJoint == LAMPSHADE)
        {
            lampJoints.lampshadeRotation += rotationStep;
        }
        break;
    case GLUT_KEY_UP:
        // Arms and lampshade use up/down for X-axis rotation with limits
        if (selectedJoint == LOWER_ARM)
        {
            lampJoints.lowerArmAngle += rotationStep;
            lampJoints.lowerArmAngle = fmin(lampJoints.lowerArmAngle, 90.0f);
        }
        else if (selectedJoint == UPPER_ARM)
        {
            lampJoints.upperArmAngle += rotationStep;
            lampJoints.upperArmAngle = fmin(lampJoints.upperArmAngle, 90.0f);
        }
        else if (selectedJoint == LAMPSHADE)
        {
            lampJoints.lampshadeAngle += rotationStep;
            lampJoints.lampshadeAngle = fmin(lampJoints.lampshadeAngle, 45.0f);
        }
        break;
    case GLUT_KEY_DOWN:
        if (selectedJoint == LOWER_ARM)
        {
            lampJoints.lowerArmAngle -= rotationStep;
            lampJoints.lowerArmAngle = fmax(lampJoints.lowerArmAngle, -10.0f);
        }
        else if (selectedJoint == UPPER_ARM)
        {
            lampJoints.upperArmAngle -= rotationStep;
            lampJoints.upperArmAngle = fmax(lampJoints.upperArmAngle, -120.0f);
        }
        else if (selectedJoint == LAMPSHADE)
        {
            lampJoints.lampshadeAngle -= rotationStep;
            lampJoints.lampshadeAngle = fmax(lampJoints.lampshadeAngle, -90.0f);
        }
        break;
    }

    glutPostRedisplay(); // Request scene redraw
}

int main(int argc, char **argv)
{
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Pixar Luxo Lamp Animation");

    // Initialize OpenGL settings
    init();

    // Register callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    // Enter main event loop (never returns)
    glutMainLoop();
    return 0;
}
