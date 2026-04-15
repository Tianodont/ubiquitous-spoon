#include "RollLogPanel.h"

#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

RollLogPanel::RollLogPanel(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* title = new QLabel("Dice Results", this);
    m_listWidget = new QListWidget(this);

    layout->addWidget(title);
    layout->addWidget(m_listWidget);
}

void RollLogPanel::appendRoll(const QString& expression, int total, const QString& breakdown) {
    auto* item = new QListWidgetItem(QString("%1 = %2").arg(expression, QString::number(total)));
    item->setToolTip(breakdown);
    m_listWidget->insertItem(0, item);
}
