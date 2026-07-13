#include "MeshNode.h"
#include "ShaderProgram.h"

MeshNode::MeshNode(const Mesh& mesh, const glm::vec3& color) 
    : m_mesh(mesh), m_color(color) 
{
}

void MeshNode::drawSelf(ShaderProgram& shader, const glm::mat4& globalTransform) {
    shader.setMat4("model", globalTransform);
    
    // Compute normal matrix: transpose of inverse of top-left 3x3 of model
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(globalTransform)));
    shader.setMat3("normalMatrix", normalMat);
    
    shader.setVec3("objectColor", m_color);
    shader.setBool("useTexture", m_useTexture);
    shader.setFloat("metallic", m_metallic);
    
    if (m_useTexture && m_textureID != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textureID);
        shader.setInt("texture1", 0);
    }
    
    glBindVertexArray(m_mesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, m_mesh.vertexCount);
    glBindVertexArray(0);
}
