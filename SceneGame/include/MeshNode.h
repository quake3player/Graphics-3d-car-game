#pragma once
#include "SceneNode.h"
#include "Geometry.h"

class MeshNode : public SceneNode {
public:
    MeshNode(const Mesh& mesh, const glm::vec3& color = glm::vec3(1.0f));
    
    void setColor(const glm::vec3& color) { m_color = color; }
    void setUseTexture(bool use) { m_useTexture = use; }
    void setTextureID(GLuint texID) { m_textureID = texID; }
    void setMetallic(float m) { m_metallic = m; }

protected:
    void drawSelf(ShaderProgram& shader, const glm::mat4& globalTransform) override;

private:
    Mesh m_mesh;
    glm::vec3 m_color;
    bool m_useTexture = false;
    GLuint m_textureID = 0;
    float m_metallic = 0.0f;
};
