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
- Semi-transparent yellow ring
- 2-second black screen fade effect when hitting black balls
- Score and life display
- Remaining time counter

## Technical Requirements
- OpenGL
- GLUT
- C++ compiler

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
- `src/`: Source files
  - `main.cpp`: Main game loop, OpenGL initialization, and core game logic
  - `Fruit.cpp`: Implementation of ball objects and their behaviors
  - `Camera.cpp`: First-person camera implementation
  - `Vector3.cpp`: 3D vector mathematics implementation
  - `Texture.cpp`: Texture loading and management
  - `Text.cpp`: Text rendering utilities

- `include/`: Header files
  - `Fruit.h`: Ball class definition, including:
    - Ball types (regular and black)
    - Movement and collision detection
    - Points system
  - `Camera.h`: Camera class definition, including:
    - View matrix calculations
    - Movement and rotation handling
    - First-person controls
  - `Vector3.h`: 3D vector class with:
    - Basic vector operations
    - Normalization and dot product
    - Vector mathematics utilities
  - `Texture.h`: Texture management class with:
    - Texture loading from BMP files
    - OpenGL texture binding
    - Resource management
  - `Text.h`: Text rendering class for:
    - Score display
    - Menu text
    - Game messages

- `textures/`: Texture files
  - `wall.bmp`: Wall texture for the game environment

- `build/`: Build directory (created during compilation)
  - Contains compiled objects
  - Executable file
  - CMake generated files

- `CMakeLists.txt`: CMake configuration file
  - Project settings
  - Compiler flags
  - Library dependencies
  - Build targets

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