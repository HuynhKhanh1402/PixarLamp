# Pixar Luxo Lamp Animation

A classic computer graphics project that simulates the iconic Pixar desk lamp with articulated joints and dynamic spotlight illumination using OpenGL and GLUT.

![Pixar Lamp](https://img.shields.io/badge/OpenGL-GLUT-blue)
![C++](https://img.shields.io/badge/C++-11-green)
![License](https://img.shields.io/badge/license-MIT-orange)

## 🎯 Features

- **Articulated Lamp Structure**: Multi-joint lamp with realistic movement
  - Base rotation (360°)
  - Lower arm joint
  - Upper arm joint
  - Lampshade joint with independent rotation

- **Dynamic Spotlight**: Realistic directional lighting (GL_LIGHT1)
  - Spotlight follows lampshade orientation
  - Adjustable spot cutoff and exponent
  - Attenuation effects for realistic light falloff
  - Toggle on/off functionality

- **Advanced Rendering**:
  - Material properties with specular highlights
  - Smooth shading (GL_SMOOTH)
  - Multiple light sources (ambient + spotlight)
  - Visual joint selection indicators

- **Interactive Controls**: Real-time joint manipulation with keyboard input

## 📚 Concepts Demonstrated

This project demonstrates key computer graphics concepts from the course materials:

- **3D Transformations** (CG.4): Hierarchical modeling with nested transformations
- **Lighting & Shading** (CG.6): 
  - Spotlight with `GL_SPOT_CUTOFF` and `GL_SPOT_DIRECTION`
  - Material properties: `GL_AMBIENT`, `GL_DIFFUSE`, `GL_SPECULAR`
  - Multiple light sources
- **Articulated Structures**: Forward kinematics with joint constraints

## 🎮 Controls

### Joint Selection
- `1` - Select base rotation
- `2` - Select lower arm joint
- `3` - Select upper arm joint
- `4` - Select lampshade joint

### Joint Rotation
- `Arrow Keys`:
  - **Left/Right**: Rotate base and lampshade around Y-axis
  - **Up/Down**: Rotate arm joints (with angle limits)

### Other Controls
- `F` - Toggle spotlight on/off
- `R` - Reset lamp to default position
- `ESC` - Exit application

## 🛠️ Requirements

### Linux
- GCC/G++ compiler (build-essential)
- FreeGLUT library
- OpenGL libraries

```bash
sudo apt-get install build-essential freeglut3-dev libglu1-mesa-dev
```

## 📦 Installation & Building

### 1. Clone or Download
```bash
git clone <repository-url>
cd PixarLamp
```

### 2. Build with Make
```bash
make
```

### 3. Build manually (if Make unavailable)
```bash
g++ -Wall -Wextra -std=c++11 -O2 main.cpp -o PixarLamp -lGL -lGLU -lglut -lm
```

### 4. Run
```bash
./PixarLamp
```

Or use:
```bash
make run
```

## 🎨 Usage Examples

### Basic Animation Sequence
1. Press `2` to select lower arm
2. Use `Up` arrow to lift the arm
3. Press `3` to select upper arm
4. Use `Down` arrow to bend the upper segment
5. Press `4` to select lampshade
6. Use arrow keys to orient the light direction
7. Press `F` to see the spotlight effect on the table

### Creating a "Looking Around" Animation
1. Press `1` to select base
2. Use `Left/Right` arrows to rotate the entire lamp
3. The spotlight will sweep across the table surface

## 🔧 Technical Details

### Lamp Dimensions
- Base radius: 1.0 units
- Lower arm length: 3.0 units
- Upper arm length: 2.5 units
- Lampshade radius: 0.8 units

### Lighting Configuration
- **GL_LIGHT0**: Ambient scene lighting
- **GL_LIGHT1**: Spotlight from lamp
  - Cutoff angle: 30°
  - Spot exponent: 15
  - Color: Warm white (1.0, 0.95, 0.8)

### Material Properties
- **Base/Arms**: Metallic gray with medium specularity
- **Lampshade**: High specularity (shininess: 128) for glossy appearance
- **Table**: Wooden texture with low specularity

## 📖 Code Structure

```
main.cpp
├── Lamp Structure
│   ├── drawBase()        - Cylindrical base with circular platform
│   ├── drawArm()         - Articulated arm segments
│   ├── drawJoint()       - Spherical joints connecting segments
│   └── drawLampshade()   - Conical lampshade with inner glow
├── Lighting
│   └── setupLighting()   - Configure spotlight and ambient light
├── Interaction
│   ├── keyboard()        - Joint selection and commands
│   └── specialKeys()     - Arrow key rotation controls
└── Rendering
    └── display()         - Main render loop with hierarchical transforms
```

## 🎓 Learning Objectives

This project helps understand:
1. **Hierarchical Modeling**: Building complex objects from simple primitives
2. **Forward Kinematics**: Parent-child transformation relationships
3. **Spotlight Implementation**: Directional lighting with falloff
4. **Material Systems**: How surface properties affect appearance
5. **Interactive 3D Applications**: Real-time user input handling

## 🐛 Troubleshooting

### "Cannot find -lglut" or "Cannot find -lGL" error
Install required libraries:
```bash
# Ubuntu/Debian
sudo apt-get install build-essential freeglut3-dev libglu1-mesa-dev

# Fedora/RHEL
sudo dnf install gcc-c++ freeglut-devel mesa-libGLU-devel

# Arch Linux
sudo pacman -S base-devel freeglut glu
```

### Black screen or no lighting
- Ensure `GL_LIGHTING` is enabled
- Check that normals are properly set
- Verify light positions are reasonable

### Lamp disappears when rotating
- Check joint angle limits in `specialKeys()` function
- Verify transformation order in `display()` function

## 🎯 Future Enhancements

Possible extensions:
- [ ] Add animation playback system
- [ ] Implement inverse kinematics for point-at behavior
- [ ] Add shadows using shadow mapping
- [ ] Include texture mapping for the table
- [ ] Create multiple lamps with different colors
- [ ] Add mouse camera control

## 📄 License

This project is created for educational purposes as part of a Computer Graphics course.

## 🙏 Acknowledgments

- Inspired by Pixar's iconic Luxo Jr. character
- Course materials: CG.4 (3D Graphics), CG.5 (Curves and Surfaces), CG.6 (Advanced Graphics Techniques)
- OpenGL and GLUT documentation

## 👨‍💻 Author

Created as a Computer Graphics course project demonstrating lighting and articulated structures.

---

**Note**: This is a learning project demonstrating fundamental computer graphics concepts. The implementation focuses on clarity and educational value rather than production-level optimization.
