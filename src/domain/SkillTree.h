#pragma once

#include "domain/SkillNode.h"

#include <QString>
#include <map>
#include <memory>

class SkillTree {
public:
    SkillTree();

    const SkillNode* root() const;
    SkillNode* root();

    const SkillNode* nodeById(const QString& id) const;
    SkillNode* nodeById(const QString& id);

    QString createNode(const QString& parentId, const SkillNode& source);
    bool updateNode(const SkillNode& node);
    bool removeNode(const QString& id);
    bool containsId(const QString& id) const;
    bool isDescendantOf(const QString& id, const QString& potentialAncestorId) const;
    bool canReparent(const QString& nodeId, const QString& newParentId) const;

    const QMap<QString, QString>& helpEntries() const;
    void setHelpEntries(const QMap<QString, QString>& entries);

    const std::map<QString, std::unique_ptr<SkillNode>>& allNodes() const;

private:
    void removeRecursive(const QString& id);

    std::map<QString, std::unique_ptr<SkillNode>> m_nodes;
    QString m_rootId;
    QMap<QString, QString> m_helpEntries;
};
