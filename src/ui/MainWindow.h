#pragma once

#include "application/DiceEngine.h"
#include "application/SkillTreeModel.h"

#include <QMainWindow>

class QAction;
class RollLogPanel;
class SkillDetailPanel;
class SkillEditorPanel;
class SkillTreeView;
class QStackedWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    enum class PanelMode {
        Detail,
        Editor
    };

    void setupUi();
    void setupMenu();
    void setupConnections();

    void selectSkill(const QString& nodeId);
    void openEditorForExisting(const QString& nodeId);
    void openEditorForNewChild(const QString& parentId);
    void saveEditorChanges();
    void deleteSkill(const QString& nodeId);
    void showDetailPanel(const QString& nodeId);
    void setPanelMode(PanelMode mode);

    void saveProject();
    void loadProject();

    QString m_selectedId;
    QString m_editTargetId;
    QString m_createParentId;

    SkillTreeModel* m_model = nullptr;
    DiceEngine m_dice;

    SkillTreeView* m_treeView = nullptr;
    QStackedWidget* m_sideStack = nullptr;
    SkillDetailPanel* m_detailPanel = nullptr;
    SkillEditorPanel* m_editorPanel = nullptr;
    RollLogPanel* m_rollLogPanel = nullptr;

    QAction* m_editorModeAction = nullptr;
    bool m_editorMode = false;

    QString m_dataPath;
};
