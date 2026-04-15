#include "SkillTreeModel.h"

#include <QIcon>

SkillTreeModel::SkillTreeModel(QObject* parent)
    : QAbstractItemModel(parent) {
}

QModelIndex SkillTreeModel::index(int row, int column, const QModelIndex& parentIndex) const {
    if (!hasIndex(row, column, parentIndex)) {
        return {};
    }

    const SkillNode* parentNode = parentIndex.isValid()
        ? m_tree.nodeById(nodeIdForIndex(parentIndex))
        : m_tree.root();

    if (!parentNode || row < 0 || row >= parentNode->childrenIds.size()) {
        return {};
    }

    const QString childId = parentNode->childrenIds.at(row);
    return createIndex(row, column, m_tree.nodeById(childId));
}

QModelIndex SkillTreeModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) {
        return {};
    }

    const SkillNode* childNode = static_cast<const SkillNode*>(child.internalPointer());
    if (!childNode || childNode->parentId.isEmpty()) {
        return {};
    }

    const SkillNode* parentNode = m_tree.nodeById(childNode->parentId);
    if (!parentNode || parentNode->parentId.isEmpty()) {
        return {};
    }

    const SkillNode* grandParent = m_tree.nodeById(parentNode->parentId);
    if (!grandParent) {
        return {};
    }

    const int row = grandParent->childrenIds.indexOf(parentNode->id);
    if (row < 0) {
        return {};
    }

    return createIndex(row, 0, const_cast<SkillNode*>(parentNode));
}

int SkillTreeModel::rowCount(const QModelIndex& parentIndex) const {
    const SkillNode* parentNode = parentIndex.isValid()
        ? m_tree.nodeById(nodeIdForIndex(parentIndex))
        : m_tree.root();

    return parentNode ? parentNode->childrenIds.size() : 0;
}

int SkillTreeModel::columnCount(const QModelIndex&) const {
    return 1;
}

QVariant SkillTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }

    const SkillNode* node = m_tree.nodeById(nodeIdForIndex(index));
    if (!node) {
        return {};
    }

    if (role == Qt::DisplayRole) {
        return node->name;
    }
    if (role == Qt::ToolTipRole) {
        return node->shortDescription;
    }
    if (role == Qt::DecorationRole) {
        if (!node->iconPath.isEmpty()) {
            return QIcon(node->iconPath);
        }
        return QIcon(":/icons/default_skill.svg");
    }

    return {};
}

QVariant SkillTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0) {
        return "Skill Tree";
    }
    return {};
}

SkillTree& SkillTreeModel::tree() {
    return m_tree;
}

const SkillTree& SkillTreeModel::tree() const {
    return m_tree;
}

QString SkillTreeModel::nodeIdForIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        return {};
    }
    const auto* node = static_cast<const SkillNode*>(index.internalPointer());
    return node ? node->id : QString();
}

void SkillTreeModel::reload() {
    beginResetModel();
    endResetModel();
}
