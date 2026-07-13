#pragma once
#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram {
public:
    ShaderProgram();
    ~ShaderProgram();

    bool loadFiles(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;

    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setVec3(const std::string& name, const glm::vec3& vec) const;
    void setFloat(const std::string& name, float value) const;
    void setInt(const std::string& name, int value) const;
    void setBool(const std::string& name, bool value) const;
    
    GLuint getID() const { return m_id; }

private:
    GLuint m_id;
    GLuint compileShader(GLenum type, const std::string& source);
};
