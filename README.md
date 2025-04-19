# BallQuest 720

A 3D ball-catching game implemented in OpenGL where players need to catch falling balls through a ring or directly.

## Game Features

### Difficulty Levels
- **Easy**: 5 lives, normal ball speed (1.0x)
- **Medium**: 3 lives, faster ball speed (1.5x)
- **Hard**: 1 life, fastest ball speed (2.0x)

### Gameplay Elements
- Regular colored balls (positive points)
- Black balls (negative points)
- Wall collision detection
- Black screen effect when hitting black balls

### Controls
- **WASD**: Move the player
- **Mouse**: Look around
- **Spacebar**: Sprint (2x movement speed)
- **Q**: Quit game immediately
- **Z**: End current game and show score
- **+/-**: Adjust game speed
- **[/]**: Adjust mouse sensitivity

### Scoring System
- Catch balls through the ring or by direct contact
- Regular balls give positive points
- Black balls give negative points and reduce lives
- Game ends when:
  - Time runs out (120 seconds)
  - Lives reach zero
  - Player presses Z

### Visual Effects
- Textured walls
- Semi-transparent ring for catching
- 2-second explosion/black screen effect when hitting black balls
- Score and life display
- Remaining time counter
- Menu system with buttons

## Technical Requirements
- OpenGL
- GLUT
- C++ compiler (C++17 standard)

## Building and Running
1. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```

2. Build the project:
   ```bash
   cmake ..
   make
   ```

3. Run the game:
   ```bash
   ./BallCatcherGame
   ```

## Project Structure

```
BallQuest720/
├── include/                  # Header files
│   ├── Camera.h              # Camera viewpoint and movement
│   ├── Fruit.h               # Ball objects and behavior
│   ├── Text.h                # Text rendering
│   ├── Texture.h             # Texture handling
│   ├── Vector3.h             # 3D vector mathematics
│   ├── shaders.h             # OpenGL shader programs
│   └── sphere.h              # Sphere rendering
│
├── src/                      # Source files
│   ├── Camera.cpp            # Camera implementation
│   ├── Fruit.cpp             # Ball physics and rendering
│   ├── Text.cpp              # Text display implementation
│   ├── Texture.cpp           # Texture loading and management
│   ├── Vector3.cpp           # Vector operations
│   └── main.cpp              # Main game loop and core logic
│
├── textures/                 # Texture assets
│   └── wall.bmp              # Wall texture
│
├── build/                    # Build directory (created during compilation)
│
└── CMakeLists.txt            # CMake build configuration
```

### Key Components

- **Camera System**: First-person camera with mouse look and keyboard movement
- **Fruit/Ball System**: Different types of falling balls with physics and scoring
- **Rendering**: 
  - 3D objects (sphere, ring, walls)
  - Texture mapping
  - Text display for score and UI
- **Game Logic**:
  - Menu system with difficulty selection
  - Collision detection
  - Scoring mechanism
  - Game state management

## Game Flow
1. Start at menu screen
2. Select difficulty level
3. Play the game:
   - Catch regular balls for points
   - Avoid or be careful with black balls
   - Watch your remaining lives and time
4. Game ends when:
   - Time runs out
   - Lives reach zero
   - Z key is pressed
5. View final score