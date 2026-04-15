#pragma once

#include "domain/SkillNode.h"

#include <QMap>
#include <QWidget>

class QTextBrowser;

class SkillDetailPanel : public QWidget {
    Q_OBJECT

public:
    explicit SkillDetailPanel(QWidget* parent = nullptr);

    void setSkill(const SkillNode& node, const QMap<QString, QString>& helpEntries);
    void clear();

signals:
    void diceExpressionClicked(const QString& expression);
    void helpTermClicked(const QString& term, const QPoint& globalPos);

private:
    QString decorateLongDescription(const QString& text) const;

    QTextBrowser* m_text = nullptr;
};
