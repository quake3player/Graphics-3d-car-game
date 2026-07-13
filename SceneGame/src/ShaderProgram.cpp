#include "ShaderProgram.h"
#include <fstream>
#include <sstream>
#include <iostream>

ShaderProgram::ShaderProgram() : m_id(0) {}

ShaderProgram::~ShaderProgram() {
    if (m_id != 0) {
        glDeleteProgram(m_id);
    }
}

bool ShaderProgram::loadFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCode, fragmentCode;
    std::ifstream vShaderFile, fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        return false;
    }

    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexCode);
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentCode);

    if (vertex == 0 || fragment == 0) return false;

    m_id = glCreateProgram();
    glAttachShader(m_id, vertex);
    glAttachShader(m_id, fragment);
    glLinkProgram(m_id);

    int success;
    char infoLog[512];
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return true;
}

GLuint ShaderProgram::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        return 0;
    }
    return shader;
}

void ShaderProgram::use() const {
    glUseProgram(m_id);
}

void ShaderProgram::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::setMat3(const std::string& name, const glm::mat3& mat) const {
     glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::setVec3(const std::string& name, const glm::vec3& vec) const {
    glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(vec));
}
void ShaderProgram::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}
void ShaderProgram::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}
void ShaderProgram::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), (int)value);
}
