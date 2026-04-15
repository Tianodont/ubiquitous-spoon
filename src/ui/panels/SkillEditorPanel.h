#pragma once

#include "domain/SkillNode.h"

#include <QMap>
#include <QWidget>

class QLineEdit;
class QTableWidget;
class QTextEdit;

class SkillEditorPanel : public QWidget {
    Q_OBJECT

public:
    explicit SkillEditorPanel(QWidget* parent = nullptr);

    void setEditMode(const SkillNode* node, const QMap<QString, QString>& helpEntries);
    SkillNode buildNode(const QString& originalId, const QString& parentId) const;
    QMap<QString, QString> buildHelpEntries() const;

signals:
    void saveRequested();
    void cancelRequested();

private:
    void setTableData(QTableWidget* table, const QMap<QString, QString>& map);
    QMap<QString, QString> tableData(QTableWidget* table) const;

    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_iconEdit = nullptr;
    QTextEdit* m_shortEdit = nullptr;
    QTextEdit* m_longEdit = nullptr;
    QTableWidget* m_fieldsTable = nullptr;
    QTableWidget* m_helpTable = nullptr;
};
