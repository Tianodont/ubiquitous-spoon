#pragma once

#include <QWidget>

class QListWidget;

class RollLogPanel : public QWidget {
    Q_OBJECT

public:
    explicit RollLogPanel(QWidget* parent = nullptr);

public slots:
    void appendRoll(const QString& expression, int total, const QString& breakdown);

private:
    QListWidget* m_listWidget = nullptr;
};
