#pragma once

#include <QTreeView>

class SkillTreeView : public QTreeView {
    Q_OBJECT

public:
    explicit SkillTreeView(QWidget* parent = nullptr);

signals:
    void skillRightClicked(const QModelIndex& index, const QPoint& globalPos);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
};
