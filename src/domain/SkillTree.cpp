#include "SkillTree.h"

#include <QUuid>

SkillTree::SkillTree() {
    auto rootNode = std::make_unique<SkillNode>();
    rootNode->id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    rootNode->name = "Skills";
    rootNode->shortDescription = "Root of the skill tree";
    m_rootId = rootNode->id;
    m_nodes.emplace(rootNode->id, std::move(rootNode));
}

const SkillNode* SkillTree::root() const {
    return nodeById(m_rootId);
}

SkillNode* SkillTree::root() {
    return nodeById(m_rootId);
}

const SkillNode* SkillTree::nodeById(const QString& id) const {
    auto it = m_nodes.find(id);
    if (it == m_nodes.end()) {
        return nullptr;
    }
    return it->second.get();
}

SkillNode* SkillTree::nodeById(const QString& id) {
    auto it = m_nodes.find(id);
    if (it == m_nodes.end()) {
        return nullptr;
    }
    return it->second.get();
}

QString SkillTree::createNode(const QString& parentId, const SkillNode& source) {
    SkillNode* parent = nodeById(parentId);
    if (!parent) {
        return {};
    }

    auto node = std::make_unique<SkillNode>(source);
    node->id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    node->parentId = parentId;
    node->childrenIds.clear();
    if (node->resources.isEmpty()) {
        node->resources.append(SkillNode::Resource{QStringLiteral("Resource"), 0, -1});
    }

    const QString id = node->id;
    parent->childrenIds.append(id);
    m_nodes.emplace(id, std::move(node));
    return id;
}

bool SkillTree::updateNode(const SkillNode& node) {
    SkillNode* existing = nodeById(node.id);
    if (!existing) {
        return false;
    }
    if (!canReparent(node.id, existing->parentId)) {
        return false;
    }

    const QStringList keepChildren = existing->childrenIds;
    const QString keepParent = existing->parentId;

    *existing = node;
    existing->childrenIds = keepChildren;
    existing->parentId = keepParent;
    if (existing->resources.isEmpty()) {
        existing->resources.append(SkillNode::Resource{QStringLiteral("Resource"), 0, -1});
    }
    return true;
}

bool SkillTree::removeNode(const QString& id) {
    if (id == m_rootId || !nodeById(id)) {
        return false;
    }

    SkillNode* target = nodeById(id);
    SkillNode* parent = nodeById(target->parentId);
    if (!parent) {
        return false;
    }

    parent->childrenIds.removeAll(id);
    removeRecursive(id);
    return true;
}

void SkillTree::removeRecursive(const QString& id) {
    SkillNode* node = nodeById(id);
    if (!node) {
        return;
    }

    const QStringList children = node->childrenIds;
    for (const QString& childId : children) {
        removeRecursive(childId);
    }
    m_nodes.erase(id);
}

const QMap<QString, QString>& SkillTree::helpEntries() const {
    return m_helpEntries;
}

void SkillTree::setHelpEntries(const QMap<QString, QString>& entries) {
    m_helpEntries = entries;
}

const std::map<QString, std::unique_ptr<SkillNode>>& SkillTree::allNodes() const {
    return m_nodes;
}

bool SkillTree::containsId(const QString& id) const {
    return nodeById(id) != nullptr;
}

bool SkillTree::isDescendantOf(const QString& id, const QString& potentialAncestorId) const {
    const SkillNode* node = nodeById(id);
    while (node && !node->parentId.isEmpty()) {
        if (node->parentId == potentialAncestorId) {
            return true;
        }
        node = nodeById(node->parentId);
    }
    return false;
}

bool SkillTree::canReparent(const QString& nodeId, const QString& newParentId) const {
    if (nodeId.isEmpty() || newParentId.isEmpty()) {
        return false;
    }
    if (nodeId == newParentId) {
        return false;
    }
    if (!containsId(nodeId) || !containsId(newParentId)) {
        return false;
    }
    return !isDescendantOf(newParentId, nodeId);
}
