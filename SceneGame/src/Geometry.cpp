#include "Geometry.h"
#include <vector>
#include <cmath>
#include <map>
Mesh Geometry::createCube() {
    float vertices[] = {
        // positions          // normals           // texture coords
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // top-right
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // bottom-right         
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // top-right
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // bottom-left
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, // top-left
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
        // Left face
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        // Right face
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        // Top face
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
    };

    Mesh m;
    m.vertexCount = 36;
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // TexCoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    return m;
}

Mesh Geometry::createCylinder(int segments) {
    std::vector<float> vertices;
    float pi = 3.14159265359f;
    float step = 2.0f * pi / segments;
    
    // Y runs from -0.5 to 0.5
    for(int i = 0; i < segments; i++) {
        float a0 = i * step;
        float a1 = (i+1) * step;
        
        float x0 = std::cos(a0)*0.5f; float z0 = std::sin(a0)*0.5f;
        float x1 = std::cos(a1)*0.5f; float z1 = std::sin(a1)*0.5f;
        
        float v0 = (float)i / segments;
        float v1 = (float)(i+1) / segments;
        
        // Face normal (approx)
        float nx0 = std::cos(a0); float nz0 = std::sin(a0);
        float nx1 = std::cos(a1); float nz1 = std::sin(a1);
        
        // Triangle 1
        vertices.insert(vertices.end(), {x0,-0.5f,z0, nx0,0,nz0, v0,0});
        vertices.insert(vertices.end(), {x1,-0.5f,z1, nx1,0,nz1, v1,0});
        vertices.insert(vertices.end(), {x0, 0.5f,z0, nx0,0,nz0, v0,1});
        // Triangle 2
        vertices.insert(vertices.end(), {x1,-0.5f,z1, nx1,0,nz1, v1,0});
        vertices.insert(vertices.end(), {x1, 0.5f,z1, nx1,0,nz1, v1,1});
        vertices.insert(vertices.end(), {x0, 0.5f,z0, nx0,0,nz0, v0,1});
        
        // Top and bottom caps
        // top
        vertices.insert(vertices.end(), {0,0.5f,0, 0,1,0, 0.5f,0.5f});
        vertices.insert(vertices.end(), {x0,0.5f,z0, 0,1,0, 0.5f+x0,0.5f+z0});
        vertices.insert(vertices.end(), {x1,0.5f,z1, 0,1,0, 0.5f+x1,0.5f+z1});
        // bot
        vertices.insert(vertices.end(), {0,-0.5f,0, 0,-1,0, 0.5f,0.5f});
        vertices.insert(vertices.end(), {x1,-0.5f,z1, 0,-1,0, 0.5f+x1,0.5f+z1});
        vertices.insert(vertices.end(), {x0,-0.5f,z0, 0,-1,0, 0.5f+x0,0.5f+z0});
    }
    
    Mesh m;
    m.vertexCount = vertices.size() / 8;
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    return m;
}

Mesh Geometry::createOvalTrack(int segments, float outerA, float outerB, float innerA, float innerB) {
    std::vector<float> vertices;
    float pi = 3.14159265359f;
    float step = 2.0f * pi / segments;

    for(int i = 0; i < segments; i++) {
        float a0 = i * step;
        float a1 = (i+1) * step;

        float c0 = std::cos(a0), s0 = std::sin(a0);
        float c1 = std::cos(a1), s1 = std::sin(a1);

        // Outer points (ellipse with semi-axes outerA, outerB)
        float xo0 = c0 * outerA, zo0 = s0 * outerB;
        float xo1 = c1 * outerA, zo1 = s1 * outerB;
        // Inner points (ellipse with semi-axes innerA, innerB)
        float xi0 = c0 * innerA, zi0 = s0 * innerB;
        float xi1 = c1 * innerA, zi1 = s1 * innerB;

        // UVs: u = parametric position along track, v = 0 inner, 1 outer
        float u0 = (float)i / segments;
        float u1 = (float)(i+1) / segments;

        // Normal is up
        // Triangle 1: inner0, outer0, outer1
        vertices.insert(vertices.end(), {xi0, 0.0f, zi0,  0,1,0,  u0*4.0f, 0.0f});
        vertices.insert(vertices.end(), {xo0, 0.0f, zo0,  0,1,0,  u0*4.0f, 1.0f});
        vertices.insert(vertices.end(), {xo1, 0.0f, zo1,  0,1,0,  u1*4.0f, 1.0f});
        // Triangle 2: inner0, outer1, inner1
        vertices.insert(vertices.end(), {xi0, 0.0f, zi0,  0,1,0,  u0*4.0f, 0.0f});
        vertices.insert(vertices.end(), {xo1, 0.0f, zo1,  0,1,0,  u1*4.0f, 1.0f});
        vertices.insert(vertices.end(), {xi1, 0.0f, zi1,  0,1,0,  u1*4.0f, 0.0f});
    }

    Mesh m;
    m.vertexCount = vertices.size() / 8;
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    return m;
}

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <string>

Mesh Geometry::loadOBJ(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
    
    if (!warn.empty()) std::cerr << "OBJ WARN: " << warn << std::endl;
    if (!err.empty()) std::cerr << "OBJ ERR: " << err << std::endl;
    
    Mesh m;
    m.vao = 0; m.vbo = 0; m.vertexCount = 0;
    if (!ret) {
        std::cerr << "Failed to load OBJ: " << path << std::endl;
        return m;
    }
    
    std::vector<float> vertices;
    
    // First pass: find bounding box for normalization
    float minX = std::numeric_limits<float>::max(), maxX = -std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max(), maxY = -std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max(), maxZ = -std::numeric_limits<float>::max();
    
    for (size_t i = 0; i < attrib.vertices.size() / 3; i++) {
        float x = attrib.vertices[3*i+0];
        float y = attrib.vertices[3*i+1];
        float z = attrib.vertices[3*i+2];
        minX = std::min(minX, x); maxX = std::max(maxX, x);
        minY = std::min(minY, y); maxY = std::max(maxY, y);
        minZ = std::min(minZ, z); maxZ = std::max(maxZ, z);
    }
    
    float cx = (minX + maxX) * 0.5f;
    float cy = (minY + maxY) * 0.5f;
    float cz = (minZ + maxZ) * 0.5f;
    float extent = std::max({maxX - minX, maxY - minY, maxZ - minZ});
    float scale = (extent > 0.0001f) ? (1.0f / extent) : 1.0f;
    
    // Second pass: extract triangulated faces
    for (const auto& shape : shapes) {
        size_t indexOffset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            int fv = shape.mesh.num_face_vertices[f];
            for (int v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];
                
                float vx = (attrib.vertices[3*idx.vertex_index+0] - cx) * scale;
                float vy = (attrib.vertices[3*idx.vertex_index+1] - cy) * scale;
                float vz = (attrib.vertices[3*idx.vertex_index+2] - cz) * scale;
                
                float nx = 0, ny = 1, nz = 0;
                if (idx.normal_index >= 0) {
                    nx = attrib.normals[3*idx.normal_index+0];
                    ny = attrib.normals[3*idx.normal_index+1];
                    nz = attrib.normals[3*idx.normal_index+2];
                }
                
                float tx = 0, ty = 0;
                if (idx.texcoord_index >= 0) {
                    tx = attrib.texcoords[2*idx.texcoord_index+0];
                    ty = attrib.texcoords[2*idx.texcoord_index+1];
                }
                
                vertices.insert(vertices.end(), {vx, vy, vz, nx, ny, nz, tx, ty});
            }
            indexOffset += fv;
        }
    }
    
    m.vertexCount = vertices.size() / 8;
    std::cout << "Loaded OBJ: " << path << " (" << m.vertexCount << " vertices)" << std::endl;
    
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    
    return m;
}

std::vector<OBJPart> Geometry::loadOBJParts(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    
    // Extract directory for mtl search path
    std::string mtlDir = "";
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        mtlDir = path.substr(0, lastSlash + 1);
    }
    
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), mtlDir.c_str());
    
    if (!warn.empty()) std::cerr << "OBJ WARN: " << warn << std::endl;
    if (!err.empty()) std::cerr << "OBJ ERR: " << err << std::endl;
    
    std::vector<OBJPart> result;
    if (!ret) {
        std::cerr << "Failed to load OBJ: " << path << std::endl;
        return result;
    }
    
    // Compute bounding box for normalization
    float minX = std::numeric_limits<float>::max(), maxX = -std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max(), maxY = -std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max(), maxZ = -std::numeric_limits<float>::max();
    
    for (size_t i = 0; i < attrib.vertices.size() / 3; i++) {
        float x = attrib.vertices[3*i+0], y = attrib.vertices[3*i+1], z = attrib.vertices[3*i+2];
        minX = std::min(minX, x); maxX = std::max(maxX, x);
        minY = std::min(minY, y); maxY = std::max(maxY, y);
        minZ = std::min(minZ, z); maxZ = std::max(maxZ, z);
    }
    float cx = (minX+maxX)*0.5f, cy = (minY+maxY)*0.5f, cz = (minZ+maxZ)*0.5f;
    float extent = std::max({maxX-minX, maxY-minY, maxZ-minZ});
    float scale = (extent > 0.0001f) ? (1.0f / extent) : 1.0f;
    
    // Group faces by material ID
    // material_id -> vector of vertex floats
    std::map<int, std::vector<float>> matGroups;
    
    for (const auto& shape : shapes) {
        size_t indexOffset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            int fv = shape.mesh.num_face_vertices[f];
            int matId = shape.mesh.material_ids[f];
            
            auto& verts = matGroups[matId];
            for (int v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];
                
                float vx = (attrib.vertices[3*idx.vertex_index+0] - cx) * scale;
                float vy = (attrib.vertices[3*idx.vertex_index+1] - cy) * scale;
                float vz = (attrib.vertices[3*idx.vertex_index+2] - cz) * scale;
                
                float nx = 0, ny = 1, nz = 0;
                if (idx.normal_index >= 0) {
                    nx = attrib.normals[3*idx.normal_index+0];
                    ny = attrib.normals[3*idx.normal_index+1];
                    nz = attrib.normals[3*idx.normal_index+2];
                }
                
                float tx = 0, ty = 0;
                if (idx.texcoord_index >= 0) {
                    tx = attrib.texcoords[2*idx.texcoord_index+0];
                    ty = attrib.texcoords[2*idx.texcoord_index+1];
                }
                
                verts.insert(verts.end(), {vx, vy, vz, nx, ny, nz, tx, ty});
            }
            indexOffset += fv;
        }
    }
    
    // Create a Mesh + color for each material group
    int totalVerts = 0;
    for (auto& [matId, verts] : matGroups) {
        if (verts.empty()) continue;
        
        OBJPart part;
        // Get color from material
        if (matId >= 0 && matId < (int)materials.size()) {
            auto& mat = materials[matId];
            glm::vec3 kd(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
            std::string name = mat.name;
            
            // Check if this is the default Blender gray (0.64, 0.64, 0.64)
            bool isDefaultGray = (std::abs(kd.r - 0.64f) < 0.02f && 
                                  std::abs(kd.g - 0.64f) < 0.02f && 
                                  std::abs(kd.b - 0.64f) < 0.02f);
            
            if (isDefaultGray) {
                // Assign smart colors based on material name
                if (name.find("paint") != std::string::npos || 
                    name.find("Carroserie") != std::string::npos ||
                    name.find("body") != std::string::npos) {
                    kd = glm::vec3(0.75f, 0.12f, 0.12f); // Red body
                } else if (name.find("tire") != std::string::npos || 
                           name.find("Pneu") != std::string::npos) {
                    kd = glm::vec3(0.08f, 0.08f, 0.08f); // Black rubber
                } else if (name.find("window") != std::string::npos || 
                           name.find("glass") != std::string::npos ||
                           name.find("Verre") != std::string::npos) {
                    kd = glm::vec3(0.15f, 0.18f, 0.22f); // Dark tinted
                } else if (name.find("Chrome") != std::string::npos ||
                           name.find("chrome") != std::string::npos ||
                           name.find("rim") != std::string::npos ||
                           name.find("inner_rim") != std::string::npos ||
                           name.find("Mirror") != std::string::npos) {
                    kd = glm::vec3(0.75f, 0.75f, 0.78f); // Chrome silver
                } else if (name.find("headlight") != std::string::npos ||
                           name.find("Headlight") != std::string::npos ||
                           name.find("Lamp") != std::string::npos) {
                    kd = glm::vec3(1.0f, 1.0f, 0.9f); // Bright white-yellow
                } else if (name.find("brake") != std::string::npos ||
                           name.find("tail") != std::string::npos) {
                    kd = glm::vec3(0.8f, 0.05f, 0.05f); // Red brake light
                } else if (name.find("turn") != std::string::npos ||
                           name.find("indicator") != std::string::npos) {
                    kd = glm::vec3(1.0f, 0.6f, 0.0f); // Orange indicator
                } else if (name.find("grill") != std::string::npos) {
                    kd = glm::vec3(0.12f, 0.12f, 0.12f); // Dark grille
                } else if (name.find("wing_mirror") != std::string::npos) {
                    kd = glm::vec3(0.75f, 0.12f, 0.12f); // Match body
                } else if (name.find("reverse") != std::string::npos) {
                    kd = glm::vec3(0.9f, 0.9f, 0.9f); // White reverse light
                } else if (name.find("black") != std::string::npos) {
                    kd = glm::vec3(0.05f, 0.05f, 0.05f);
                } else if (name.find("box") != std::string::npos) {
                    kd = glm::vec3(0.3f, 0.3f, 0.35f); // Generic dark
                } else {
                    kd = glm::vec3(0.45f, 0.45f, 0.48f); // Neutral mid-gray
                }
            }
            
            part.color = kd;
            // Detect metallic materials by name or specular
            if (name.find("Chrome") != std::string::npos || 
                name.find("Mirror") != std::string::npos ||
                name.find("rim") != std::string::npos ||
                mat.shininess > 80.0f) {
                part.metallic = 0.9f;
            } else if (name.find("window") != std::string::npos ||
                       name.find("glass") != std::string::npos ||
                       name.find("Verre") != std::string::npos) {
                part.metallic = 0.85f;
            } else {
                part.metallic = 0.3f;
            }
        } else {
            part.color = glm::vec3(0.5f, 0.5f, 0.5f);
            part.metallic = 0.3f;
        }
        
        Mesh m;
        m.vertexCount = verts.size() / 8;
        totalVerts += m.vertexCount;
        
        glGenVertexArrays(1, &m.vao);
        glGenBuffers(1, &m.vbo);
        glBindVertexArray(m.vao);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
        
        part.mesh = m;
        result.push_back(part);
    }
    
    std::cout << "Loaded OBJ (multi-material): " << path 
              << " (" << totalVerts << " verts, " << result.size() << " parts)" << std::endl;
    return result;
}
