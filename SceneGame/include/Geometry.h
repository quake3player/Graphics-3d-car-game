#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct Mesh {
    GLuint vao;
    GLuint vbo;
    int vertexCount;
};

struct OBJPart {
    Mesh mesh;
    glm::vec3 color;
    float metallic;
};

class Geometry {
public:
    static Mesh createCube();
    static Mesh createCylinder(int segments = 16);

    static Mesh createOvalTrack(int segments = 64,
                                float outerA = 40.0f, float outerB = 32.0f,
                                float innerA = 30.0f, float innerB = 22.0f);

    static Mesh loadOBJ(const std::string& path);
    static std::vector<OBJPart> loadOBJParts(const std::string& path);
};
