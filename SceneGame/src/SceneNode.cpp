#include "SceneNode.h"

SceneNode::SceneNode() 
    : m_localTransform(1.0f), m_globalTransform(1.0f), m_parent(nullptr) 
{
}

SceneNode::~SceneNode() {
    for (auto child : m_children) {
        delete child;
    }
}

void SceneNode::addChild(SceneNode* child) {
    child->m_parent = this;
    m_children.push_back(child);
}

void SceneNode::setLocalTransform(const glm::mat4& transform) {
    m_localTransform = transform;
}

glm::mat4 SceneNode::getLocalTransform() const {
    return m_localTransform;
}

glm::mat4 SceneNode::getGlobalTransform() const {
    return m_globalTransform;
}

void SceneNode::draw(ShaderProgram& shader, const glm::mat4& parentTransform) {
    m_globalTransform = parentTransform * m_localTransform;
    drawSelf(shader, m_globalTransform);
    for (auto child : m_children) {
        child->draw(shader, m_globalTransform);
    }
}
