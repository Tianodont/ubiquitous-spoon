#pragma once

#include "application/DiceEngine.h"
#include "application/ThemeManager.h"
#include "domain/SkillTree.h"

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QSettings>

class SkillTreeViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList skillNodes READ skillNodes NOTIFY skillNodesChanged)
    Q_PROPERTY(QVariantList skillEdges READ skillEdges NOTIFY skillNodesChanged)
    Q_PROPERTY(QVariantMap selectedSkill READ selectedSkill NOTIFY selectedSkillChanged)
    Q_PROPERTY(QVariantMap helpEntries READ helpEntries NOTIFY helpEntriesChanged)
    Q_PROPERTY(bool editorMode READ editorMode WRITE setEditorMode NOTIFY editorModeChanged)
    Q_PROPERTY(QString lastRollExpression READ lastRollExpression NOTIFY lastRollChanged)
    Q_PROPERTY(int lastRollTotal READ lastRollTotal NOTIFY lastRollChanged)
    Q_PROPERTY(QString lastRollBreakdown READ lastRollBreakdown NOTIFY lastRollChanged)
    Q_PROPERTY(QString saveError READ saveError NOTIFY saveErrorChanged)
    Q_PROPERTY(QString operationMessage READ operationMessage NOTIFY operationMessageChanged)
    Q_PROPERTY(int editorPanelWidth READ editorPanelWidth WRITE setEditorPanelWidth NOTIFY editorPanelWidthChanged)
    Q_PROPERTY(ThemeManager* themeManager READ themeManager CONSTANT)

public:
    explicit SkillTreeViewModel(QObject* parent = nullptr);

    QVariantList skillNodes() const;
    QVariantList skillEdges() const;
    QVariantMap selectedSkill() const;
    QVariantMap helpEntries() const;

    bool editorMode() const;
    void setEditorMode(bool value);

    QString lastRollExpression() const;
    int lastRollTotal() const;
    QString lastRollBreakdown() const;
    QString saveError() const;
    QString operationMessage() const;
    int editorPanelWidth() const;
    void setEditorPanelWidth(int value);

    ThemeManager* themeManager();

    Q_INVOKABLE void selectSkill(const QString& id);
    Q_INVOKABLE void createChildSkill(const QString& parentId);
    Q_INVOKABLE void updateOrCreateSkill(const QVariantMap& payload);
    Q_INVOKABLE void deleteSkill(const QString& id);
    Q_INVOKABLE bool rollDice(const QString& expression);
    Q_INVOKABLE bool rollDiceForSkill(const QString& skillId, const QString& expression);
    Q_INVOKABLE QString helpText(const QString& term) const;
    Q_INVOKABLE void setHelpEntry(const QString& term, const QString& text);
    Q_INVOKABLE void removeHelpEntry(const QString& term);
    Q_INVOKABLE QVariantMap emptySkillForParent(const QString& parentId) const;
    Q_INVOKABLE void toggleSkillLock(const QString& id);
    Q_INVOKABLE void adjustResource(const QString& skillId, int delta);
    Q_INVOKABLE void resetResource(const QString& skillId);
    Q_INVOKABLE void setResourceCurrent(const QString& skillId, int value);
    Q_INVOKABLE void setResourceMax(const QString& skillId, int value);
    Q_INVOKABLE void addResource(const QString& skillId);
    Q_INVOKABLE void removeResource(const QString& skillId, int index);
    Q_INVOKABLE void renameResource(const QString& skillId, int index, const QString& name);
    Q_INVOKABLE void setResourceAtCurrent(const QString& skillId, int index, int value);
    Q_INVOKABLE void setResourceAtMax(const QString& skillId, int index, int value);
    Q_INVOKABLE void adjustResourceAt(const QString& skillId, int index, int delta);
    Q_INVOKABLE void toggleCollapse(const QString& id);
    Q_INVOKABLE bool exportTree(const QString& fileUrl);
    Q_INVOKABLE bool importTree(const QString& fileUrl);

signals:
    void skillNodesChanged();
    void selectedSkillChanged();
    void helpEntriesChanged();
    void editorModeChanged();
    void lastRollChanged();
    void saveErrorChanged();
    void operationMessageChanged();
    void editorPanelWidthChanged();

private:
    QVariantMap nodeToMap(const SkillNode& node, double x = 0.0, double y = 0.0, int depth = 0) const;
    QVariantList buildLayoutNodes() const;
    QVariantList buildLayoutEdges(const QVariantList& nodes) const;
    void ensureSeedData();
    void persist();
    void emitTreeChanged();
    SkillNode::Resource sanitizeResource(const SkillNode::Resource& resource) const;
    QVector<SkillNode::Resource> parseResources(const QVariant& value) const;
    QString normalizeFileUrl(const QString& fileUrl) const;
    void setSaveError(const QString& message);
    void setOperationMessage(const QString& message);

    SkillTree m_tree;
    DiceEngine m_dice;
    ThemeManager m_themeManager;

    QString m_selectedId;
    bool m_editorMode = false;

    QString m_lastRollExpression;
    int m_lastRollTotal = 0;
    QString m_lastRollBreakdown;

    QString m_editingId;
    QString m_creatingParentId;

    QString m_dataPath;
    QString m_saveError;
    QString m_operationMessage;
    int m_editorPanelWidth = 440;
    QSettings m_settings;
};
