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
LampJoints lampJoints = {0.0f, 30.0f, -60.0f, -90.0f, 0.0f};
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
void drawDisk(float innerRadius, float outerRadius, int slices);
void drawCone(float baseRadius, float topRadius, float height, int slices);
void drawWireCube(float size);
void drawWireSphere(float radius, int slices, int stacks);

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
 * Draw a disk (circle) using custom implementation
 * Algorithm: Parametric Circle Drawing
 * @param innerRadius - Inner radius (0 for filled circle)
 * @param outerRadius - Outer radius
 * @param slices - Number of subdivisions around the circle
 */
void drawDisk(float innerRadius, float outerRadius, int slices)
{
    glBegin(GL_TRIANGLE_STRIP);
    
    for (int i = 0; i <= slices; i++)
    {
        // Parametric circle equation: x = r*cos(θ), y = r*sin(θ)
        float theta = 2.0f * M_PI * i / slices;
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);
        
        // Normal points in +Z direction (perpendicular to disk)
        glNormal3f(0.0f, 0.0f, 1.0f);
        
        // Inner vertex
        glVertex3f(innerRadius * cosTheta, innerRadius * sinTheta, 0.0f);
        
        // Outer vertex
        glVertex3f(outerRadius * cosTheta, outerRadius * sinTheta, 0.0f);
    }
    
    glEnd();
}

/**
 * Draw a cylinder using custom implementation
 * Algorithm: Surface of Revolution - rotating a line segment around an axis
 * @param radius - Cylinder radius
 * @param height - Cylinder height (along Z-axis)
 * @param slices - Number of subdivisions around the circumference
 */
void drawCylinder(float radius, float height, int slices)
{
    // Draw the side surface using quad strips
    glBegin(GL_QUAD_STRIP);
    
    for (int i = 0; i <= slices; i++)
    {
        // Parametric circle equation for revolution
        float theta = 2.0f * M_PI * i / slices;
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);
        
        // Normal vector points radially outward (perpendicular to cylinder axis)
        glNormal3f(cosTheta, sinTheta, 0.0f);
        
        // Bottom vertex at z=0
        glVertex3f(radius * cosTheta, radius * sinTheta, 0.0f);
        
        // Top vertex at z=height
        glVertex3f(radius * cosTheta, radius * sinTheta, height);
    }
    
    glEnd();
}

/**
 * Draw a cone (truncated or full) using custom implementation
 * Algorithm: Surface of Revolution - rotating a line segment with varying radius
 * @param baseRadius - Radius at base (z=0)
 * @param topRadius - Radius at top (z=height)
 * @param height - Cone height (along Z-axis)
 * @param slices - Number of subdivisions around the circumference
 */
void drawCone(float baseRadius, float topRadius, float height, int slices)
{
    // Calculate the slope for normal computation
    float radiusDiff = baseRadius - topRadius;
    float normalZ = radiusDiff / sqrt(radiusDiff * radiusDiff + height * height);
    float normalXY = height / sqrt(radiusDiff * radiusDiff + height * height);
    
    glBegin(GL_QUAD_STRIP);
    
    for (int i = 0; i <= slices; i++)
    {
        float theta = 2.0f * M_PI * i / slices;
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);
        
        // Normal vector for a cone surface
        glNormal3f(normalXY * cosTheta, normalXY * sinTheta, normalZ);
        
        // Bottom vertex
        glVertex3f(baseRadius * cosTheta, baseRadius * sinTheta, 0.0f);
        
        // Top vertex
        glVertex3f(topRadius * cosTheta, topRadius * sinTheta, height);
    }
    
    glEnd();
}

/**
 * Draw a sphere using custom implementation
 * Algorithm: UV Sphere (Latitude-Longitude parameterization)
 * Parametric equations:
 *   x = r * cos(φ) * sin(θ)
 *   y = r * sin(φ) * sin(θ)
 *   z = r * cos(θ)
 * where θ ∈ [0, π] (latitude), φ ∈ [0, 2π] (longitude)
 * 
 * @param radius - Sphere radius
 * @param slices - Number of subdivisions around the Z-axis (longitude)
 * @param stacks - Number of subdivisions along the Z-axis (latitude)
 */
void drawSphere(float radius, int slices, int stacks)
{
    for (int i = 0; i < stacks; i++)
    {
        // Latitude angles (from north pole to south pole)
        float theta1 = M_PI * i / stacks;
        float theta2 = M_PI * (i + 1) / stacks;
        
        float sinTheta1 = sin(theta1);
        float cosTheta1 = cos(theta1);
        float sinTheta2 = sin(theta2);
        float cosTheta2 = cos(theta2);
        
        glBegin(GL_QUAD_STRIP);
        
        for (int j = 0; j <= slices; j++)
        {
            // Longitude angle (around the Z-axis)
            float phi = 2.0f * M_PI * j / slices;
            float cosPhi = cos(phi);
            float sinPhi = sin(phi);
            
            // First vertex (at theta1)
            float x1 = radius * cosPhi * sinTheta1;
            float y1 = radius * sinPhi * sinTheta1;
            float z1 = radius * cosTheta1;
            
            // Normal is the normalized position vector for a sphere
            glNormal3f(cosPhi * sinTheta1, sinPhi * sinTheta1, cosTheta1);
            glVertex3f(x1, y1, z1);
            
            // Second vertex (at theta2)
            float x2 = radius * cosPhi * sinTheta2;
            float y2 = radius * sinPhi * sinTheta2;
            float z2 = radius * cosTheta2;
            
            glNormal3f(cosPhi * sinTheta2, sinPhi * sinTheta2, cosTheta2);
            glVertex3f(x2, y2, z2);
        }
        
        glEnd();
    }
}

/**
 * Draw a wireframe cube using custom implementation
 * Algorithm: Direct vertex specification for 12 edges of a cube
 * The cube is centered at origin with edges parallel to coordinate axes
 * 
 * @param size - Edge length of the cube
 */
void drawWireCube(float size)
{
    float half = size / 2.0f;
    
    // Define 8 vertices of the cube
    // Front face: z = +half
    // Back face:  z = -half
    float vertices[8][3] = {
        {-half, -half,  half},  // 0: Front bottom left
        { half, -half,  half},  // 1: Front bottom right
        { half,  half,  half},  // 2: Front top right
        {-half,  half,  half},  // 3: Front top left
        {-half, -half, -half},  // 4: Back bottom left
        { half, -half, -half},  // 5: Back bottom right
        { half,  half, -half},  // 6: Back top right
        {-half,  half, -half}   // 7: Back top left
    };
    
    // Draw 12 edges using GL_LINES
    glBegin(GL_LINES);
    
    // Front face edges (4 edges)
    glVertex3fv(vertices[0]); glVertex3fv(vertices[1]); // Bottom edge
    glVertex3fv(vertices[1]); glVertex3fv(vertices[2]); // Right edge
    glVertex3fv(vertices[2]); glVertex3fv(vertices[3]); // Top edge
    glVertex3fv(vertices[3]); glVertex3fv(vertices[0]); // Left edge
    
    // Back face edges (4 edges)
    glVertex3fv(vertices[4]); glVertex3fv(vertices[5]); // Bottom edge
    glVertex3fv(vertices[5]); glVertex3fv(vertices[6]); // Right edge
    glVertex3fv(vertices[6]); glVertex3fv(vertices[7]); // Top edge
    glVertex3fv(vertices[7]); glVertex3fv(vertices[4]); // Left edge
    
    // Connecting edges between front and back faces (4 edges)
    glVertex3fv(vertices[0]); glVertex3fv(vertices[4]); // Bottom left
    glVertex3fv(vertices[1]); glVertex3fv(vertices[5]); // Bottom right
    glVertex3fv(vertices[2]); glVertex3fv(vertices[6]); // Top right
    glVertex3fv(vertices[3]); glVertex3fv(vertices[7]); // Top left
    
    glEnd();
}

/**
 * Draw a wireframe sphere using custom implementation
 * Algorithm: UV Sphere with latitude/longitude lines
 * Draws horizontal circles (latitude lines) and vertical circles (longitude lines)
 * 
 * @param radius - Sphere radius
 * @param slices - Number of vertical segments (longitude lines)
 * @param stacks - Number of horizontal segments (latitude lines)
 */
void drawWireSphere(float radius, int slices, int stacks)
{
    // Draw latitude lines (horizontal circles)
    for (int i = 0; i <= stacks; i++)
    {
        float theta = M_PI * i / stacks; // Latitude angle from 0 (north pole) to π (south pole)
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
        float currentRadius = radius * sinTheta; // Radius of current latitude circle
        float z = radius * cosTheta;             // Z-coordinate at this latitude
        
        glBegin(GL_LINE_LOOP);
        for (int j = 0; j < slices; j++)
        {
            float phi = 2.0f * M_PI * j / slices; // Longitude angle
            float x = currentRadius * cos(phi);
            float y = currentRadius * sin(phi);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
    
    // Draw longitude lines (vertical circles from pole to pole)
    for (int j = 0; j < slices; j++)
    {
        float phi = 2.0f * M_PI * j / slices; // Longitude angle
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= stacks; i++)
        {
            float theta = M_PI * i / stacks; // Latitude angle
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);
            
            float x = radius * cosPhi * sinTheta;
            float y = radius * sinPhi * sinTheta;
            float z = radius * cosTheta;
            glVertex3f(x, y, z);
        }
        glEnd();
    }
}

/**
 * Draw the circular base of the lamp
 * Material: Dark metallic gray with high specular for metal appearance
 * Using custom primitives: cylinder + disk
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
    glTranslatef(0.0f, 0.0f, BASE_HEIGHT);
    drawDisk(0.0f, BASE_RADIUS, 32);
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
 * Using custom primitives: truncated cone + disks
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

    // Draw cone: narrow at top (0.4 * radius), wide at bottom (radius)
    drawCone(LAMPSHADE_RADIUS * 0.4f, LAMPSHADE_RADIUS, LAMPSHADE_HEIGHT, 32);

    // Draw top cap (narrow end)
    drawDisk(0.0f, LAMPSHADE_RADIUS * 0.4f, 32);

    // Draw inner glow at bottom opening when spotlight is on
    if (spotlightEnabled)
    {
        glDisable(GL_LIGHTING);                     // Draw unlit for glowing effect
        glColor4f(1.0f, 0.9f, 0.2f, 0.9f);          // Bright warm yellow
        glTranslatef(0.0f, 0.0f, LAMPSHADE_HEIGHT); // Move to bottom opening
        drawDisk(0.0f, LAMPSHADE_RADIUS * 0.5f, 32);
        glEnable(GL_LIGHTING);
    }

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

    // Draw selection highlight for base using custom wireframe cube
    if (selectedJoint == BASE)
    {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow wireframe
        glPushMatrix();
        glTranslatef(0.0f, BASE_HEIGHT * 0.5f, 0.0f);
        drawWireCube(BASE_RADIUS * 2.2f);
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
        drawWireSphere(ARM_RADIUS * 2.5f, 16, 16);
        glEnable(GL_LIGHTING);
    }

    drawJoint();
    glRotatef(lampJoints.lowerArmAngle, 1.0f, 0.0f, 0.0f); // Rotate lower arm
    drawArm(LOWER_ARM_LENGTH);
    glTranslatef(0.0f, LOWER_ARM_LENGTH, 0.0f); // Move to end of lower arm

    // Level 3: Upper arm joint using custom wireframe sphere
    if (selectedJoint == UPPER_ARM)
    {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow wireframe
        drawWireSphere(ARM_RADIUS * 2.5f, 16, 16);
        glEnable(GL_LIGHTING);
    }

    drawJoint();
    glRotatef(lampJoints.upperArmAngle, 1.0f, 0.0f, 0.0f); // Rotate upper arm
    drawArm(UPPER_ARM_LENGTH);
    glTranslatef(0.0f, UPPER_ARM_LENGTH, 0.0f); // Move to end of upper arm

    // Level 4: Lampshade joint using custom wireframe sphere
    if (selectedJoint == LAMPSHADE)
    {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow wireframe
        drawWireSphere(ARM_RADIUS * 2.5f, 16, 16);
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
        lampJoints.lowerArmAngle = 30.0f;
        lampJoints.upperArmAngle = -60.0f;
        lampJoints.lampshadeAngle = -90.0f;
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
