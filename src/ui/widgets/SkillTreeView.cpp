#include "SkillTreeView.h"

#include <QContextMenuEvent>

SkillTreeView::SkillTreeView(QWidget* parent)
    : QTreeView(parent) {
    setAlternatingRowColors(true);
    setUniformRowHeights(true);
    setExpandsOnDoubleClick(true);
}

void SkillTreeView::contextMenuEvent(QContextMenuEvent* event) {
    const QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        emit skillRightClicked(index, event->globalPos());
    }
}
