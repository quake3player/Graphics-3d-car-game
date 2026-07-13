#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram;

class SceneNode {
public:
    SceneNode();
    virtual ~SceneNode();

    void addChild(SceneNode* child);
    
    void setLocalTransform(const glm::mat4& transform);
    glm::mat4 getLocalTransform() const;
    glm::mat4 getGlobalTransform() const;
    
    virtual void draw(ShaderProgram& shader, const glm::mat4& parentTransform = glm::mat4(1.0f));

protected:
    virtual void drawSelf(ShaderProgram& shader, const glm::mat4& globalTransform) {}

    glm::mat4 m_localTransform;
    glm::mat4 m_globalTransform;

    SceneNode* m_parent;
    std::vector<SceneNode*> m_children;
};
