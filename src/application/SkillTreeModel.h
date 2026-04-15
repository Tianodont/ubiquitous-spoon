#pragma once

#include "domain/SkillTree.h"

#include <QAbstractItemModel>

class SkillTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit SkillTreeModel(QObject* parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    SkillTree& tree();
    const SkillTree& tree() const;

    QString nodeIdForIndex(const QModelIndex& index) const;

    void reload();

private:
    SkillTree m_tree;
};
