#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

namespace mpu {
namespace gph {
namespace Geometry {
namespace Cube {

    const float size = 1.0;

    std::vector<GLfloat> position(float size = 1.0)
    {
        return {
                // Front face
                -size, -size, size, size, -size, size, size, size, size,
                size, size, size, -size, size, size, -size, -size, size,
                // Right face
                size, -size, size, size, -size, -size, size, size, -size,
                size, size, -size, size, size, size, size, -size, size,
                // Back face
                -size, -size, -size, size, -size, -size, size, size, -size,
                size, size, -size, -size, size, -size, -size, -size, -size,
                // Left face
                -size, -size, size, -size, -size, -size, -size, size, -size,
                -size, size, -size, -size, size, size, -size, -size, size,
                // Bottom face
                -size, -size, size, size, -size, size, size, -size, -size,
                size, -size, -size, -size, -size, -size, -size, -size, size,
                // Top Face
                -size, size, size, size, size, size, size, size, -size,
                size, size, -size, -size, size, -size, -size, size, size,
        };
    }

    std::vector<GLfloat> normals = {
        // Front face
        0.0,  0.0,  1.0,    0.0,  0.0,  1.0,    0.0,  0.0,  1.0,
        0.0,  0.0,  1.0,    0.0,  0.0,  1.0,    0.0,  0.0,  1.0,
        // Right face
        1.0,  0.0, 0.0,    1.0,  0.0, 0.0,    1.0,  0.0, 0.0,
        1.0,  0.0, 0.0,    1.0,  0.0, 0.0,    1.0,  0.0, 0.0,
        // Back face
        0.0,  0.0,  -1.0,    0.0,  0.0,  -1.0,    0.0,  0.0,  -1.0,
        0.0,  0.0,  -1.0,    0.0,  0.0,  -1.0,    0.0,  0.0,  -1.0,
        // Left face
        -1.0, 0.0,  0.0,    -1.0, 0.0,  0.0,    -1.0, 0.0,  0.0,
        -1.0, 0.0,  0.0,    -1.0, 0.0,  0.0,    -1.0, 0.0,  0.0,
        // Bottom face
        0.0,  -1.0,  0.0,    0.0,  -1.0,  0.0,    0.0,  -1.0,  0.0,
        0.0,  -1.0,  0.0,    0.0,  -1.0,  0.0,    0.0,  -1.0,  0.0,
        // Top face
        0.0,  1.0,  0.0,   0.0,  1.0,  0.0,   0.0,  1.0,  0.0,
        0.0,  1.0,  0.0,   0.0,  1.0,  0.0,   0.0,  1.0,  0.0
    };

    std::vector<GLfloat> tangents = {
        // Front face
        0.0, -1.0,  0.0,    0.0, -1.0,  0.0,    0.0, -1.0,  0.0,
        0.0, -1.0,  0.0,    0.0, -1.0,  0.0,    0.0, -1.0,  0.0,
        // Right face
        0.0,  0.0, -1.0,    0.0,  0.0, -1.0,    0.0,  0.0, -1.0,
        0.0,  0.0, -1.0,    0.0,  0.0, -1.0,    0.0,  0.0, -1.0,
        // Back face
        1.0,  0.0,  0.0,    1.0,  0.0,  0.0,    1.0,  0.0,  0.0,
        1.0,  0.0,  0.0,    1.0,  0.0,  0.0,    1.0,  0.0,  0.0,
        // Left face
        0.0,  1.0,  0.0,    0.0,  1.0,  0.0,    0.0,  1.0,  0.0,
        0.0,  1.0,  0.0,    0.0,  1.0,  0.0,    0.0,  1.0,  0.0,
        // Bottom face
        0.0,  0.0,  1.0,    0.0,  0.0,  1.0,    0.0,  0.0,  1.0,
        0.0,  0.0,  1.0,    0.0,  0.0,  1.0,    0.0,  0.0,  1.0
         // Top face
       -1.0,  0.0,  0.0,   -1.0,  0.0,  0.0,   -1.0,  0.0,  0.0,
        -1.0,  0.0,  0.0,   -1.0,  0.0,  0.0,   -1.0,  0.0,  0.0

    };

    std::vector<GLfloat> uvCoordinates = {
        // Front face
        0,0, 1,0, 1,1,
        1,1, 0,1, 0,0,
        // Right face
        0,0, 1,0, 1,1,
        1,1, 0,1, 0,0,
        // Back face
        0,0, 1,0, 1,1,
        1,1, 0,1, 0,0,
        // Left face
        0,0, 1,0, 1,1,
        1,1, 0,1, 0,0,
        // Bottom face
        0,0, 1,0, 1,1,
        1,1, 0,1, 0,0,
        // Top face
        0,0, 1,0, 1,1,
        1,1, 0,1, 0,0,
    };
}}}}