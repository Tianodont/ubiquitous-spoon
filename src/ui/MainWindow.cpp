#include "MainWindow.h"

#include "domain/SkillNode.h"
#include "infrastructure/JsonRepository.h"
#include "ui/panels/RollLogPanel.h"
#include "ui/panels/SkillDetailPanel.h"
#include "ui/panels/SkillEditorPanel.h"
#include "ui/widgets/SkillTreeView.h"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStatusBar>
#include <QToolTip>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_model(new SkillTreeModel(this)) {
    setupUi();
    setupMenu();
    setupConnections();

    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_dataPath = baseDir + "/skill_tree.json";

    loadProject();
    m_model->reload();
    m_treeView->expandAll();
}

MainWindow::~MainWindow() {
    saveProject();
}

void MainWindow::setupUi() {
    auto* splitter = new QSplitter(this);
    auto* leftContainer = new QWidget(splitter);
    auto* leftLayout = new QVBoxLayout(leftContainer);
    leftLayout->setContentsMargins(4, 4, 4, 4);

    m_treeView = new SkillTreeView(leftContainer);
    m_treeView->setModel(m_model);
    leftLayout->addWidget(m_treeView, 3);

    m_rollLogPanel = new RollLogPanel(leftContainer);
    leftLayout->addWidget(m_rollLogPanel, 1);

    m_sideStack = new QStackedWidget(splitter);
    m_detailPanel = new SkillDetailPanel(m_sideStack);
    m_editorPanel = new SkillEditorPanel(m_sideStack);
    m_sideStack->addWidget(m_detailPanel);
    m_sideStack->addWidget(m_editorPanel);

    splitter->addWidget(leftContainer);
    splitter->addWidget(m_sideStack);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);
    setWindowTitle("Skill Tree App");
    resize(1200, 760);
    statusBar()->showMessage("Ready");
}

void MainWindow::setupMenu() {
    auto* settingsMenu = menuBar()->addMenu("Settings");
    m_editorModeAction = settingsMenu->addAction("Editor Mode");
    m_editorModeAction->setCheckable(true);
}

void MainWindow::setupConnections() {
    connect(m_editorModeAction, &QAction::toggled, this, [this](bool checked) {
        m_editorMode = checked;
        statusBar()->showMessage(checked ? "Editor mode enabled" : "Editor mode disabled", 2000);
    });

    connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
        this, [this](const QModelIndex& current, const QModelIndex&) {
            const QString id = m_model->nodeIdForIndex(current);
            if (!id.isEmpty()) {
                selectSkill(id);
            }
        });

    connect(m_treeView, &SkillTreeView::skillRightClicked, this,
        [this](const QModelIndex& index, const QPoint& globalPos) {
            if (!m_editorMode) {
                return;
            }

            const QString id = m_model->nodeIdForIndex(index);
            if (id.isEmpty()) {
                return;
            }

            QMenu menu(this);
            auto* editAction = menu.addAction("Edit skill");
            auto* deleteAction = menu.addAction("Delete skill");
            auto* addChildAction = menu.addAction("Add child skill");

            QAction* chosen = menu.exec(globalPos);
            if (chosen == editAction) {
                openEditorForExisting(id);
            } else if (chosen == deleteAction) {
                deleteSkill(id);
            } else if (chosen == addChildAction) {
                openEditorForNewChild(id);
            }
        });

    connect(m_detailPanel, &SkillDetailPanel::diceExpressionClicked, this,
        [this](const QString& expression) {
            const auto result = m_dice.roll(expression);
            if (!result.ok) {
                statusBar()->showMessage(QString("Invalid dice expression: %1").arg(expression), 2500);
                return;
            }
            m_rollLogPanel->appendRoll(expression, result.total, result.breakdown);
        });

    connect(m_detailPanel, &SkillDetailPanel::helpTermClicked, this,
        [this](const QString& term, const QPoint& pos) {
            const QString explanation = m_model->tree().helpEntries().value(term);
            if (!explanation.isEmpty()) {
                QToolTip::showText(pos, explanation, m_detailPanel);
            } else {
                QToolTip::showText(pos, QString("No help entry for '%1'").arg(term), m_detailPanel);
            }
        });

    connect(m_editorPanel, &SkillEditorPanel::saveRequested, this, &MainWindow::saveEditorChanges);
    connect(m_editorPanel, &SkillEditorPanel::cancelRequested, this, [this] {
        setPanelMode(PanelMode::Detail);
        showDetailPanel(m_selectedId);
    });
}

void MainWindow::setPanelMode(PanelMode mode) {
    m_sideStack->setCurrentWidget(mode == PanelMode::Detail
            ? static_cast<QWidget*>(m_detailPanel)
            : static_cast<QWidget*>(m_editorPanel));
}

void MainWindow::selectSkill(const QString& nodeId) {
    m_selectedId = nodeId;
    showDetailPanel(nodeId);
}

void MainWindow::showDetailPanel(const QString& nodeId) {
    const SkillNode* node = m_model->tree().nodeById(nodeId);
    if (!node) {
        m_detailPanel->clear();
        return;
    }

    m_detailPanel->setSkill(*node, m_model->tree().helpEntries());
    setPanelMode(PanelMode::Detail);
}

void MainWindow::openEditorForExisting(const QString& nodeId) {
    const SkillNode* node = m_model->tree().nodeById(nodeId);
    if (!node) {
        return;
    }

    m_editTargetId = nodeId;
    m_createParentId.clear();
    m_editorPanel->setEditMode(node, m_model->tree().helpEntries());
    setPanelMode(PanelMode::Editor);
}

void MainWindow::openEditorForNewChild(const QString& parentId) {
    const SkillNode* parentNode = m_model->tree().nodeById(parentId);
    if (!parentNode) {
        return;
    }

    m_createParentId = parentId;
    m_editTargetId.clear();

    SkillNode empty;
    empty.parentId = parentId;
    m_editorPanel->setEditMode(&empty, m_model->tree().helpEntries());
    setPanelMode(PanelMode::Editor);
}

void MainWindow::saveEditorChanges() {
    m_model->tree().setHelpEntries(m_editorPanel->buildHelpEntries());

    if (!m_editTargetId.isEmpty()) {
        const SkillNode* existing = m_model->tree().nodeById(m_editTargetId);
        if (!existing) {
            return;
        }

        SkillNode updated = m_editorPanel->buildNode(existing->id, existing->parentId);
        updated.childrenIds = existing->childrenIds;
        m_model->tree().updateNode(updated);
        m_selectedId = updated.id;
    } else if (!m_createParentId.isEmpty()) {
        SkillNode created = m_editorPanel->buildNode({}, m_createParentId);
        const QString id = m_model->tree().createNode(m_createParentId, created);
        if (!id.isEmpty()) {
            m_selectedId = id;
        }
    }

    m_model->reload();
    m_treeView->expandAll();
    setPanelMode(PanelMode::Detail);
    showDetailPanel(m_selectedId);
    saveProject();
}

void MainWindow::deleteSkill(const QString& nodeId) {
    const SkillNode* node = m_model->tree().nodeById(nodeId);
    if (!node || node->parentId.isEmpty()) {
        return;
    }

    const auto response = QMessageBox::question(
        this,
        "Delete Skill",
        QString("Delete '%1' and all children?").arg(node->name));

    if (response != QMessageBox::Yes) {
        return;
    }

    const QString parentId = node->parentId;
    if (m_model->tree().removeNode(nodeId)) {
        m_selectedId = parentId;
        m_model->reload();
        m_treeView->expandAll();
        showDetailPanel(parentId);
        saveProject();
    }
}

void MainWindow::saveProject() {
    JsonRepository repository;
    QString error;
    if (!repository.save(m_model->tree(), m_dataPath, &error)) {
        statusBar()->showMessage("Save failed: " + error, 4000);
    }
}

void MainWindow::loadProject() {
    JsonRepository repository;
    QString error;
    if (!repository.load(&m_model->tree(), m_dataPath, &error)) {
        statusBar()->showMessage("Load failed: " + error, 4000);
    }

    if (m_model->tree().root()->childrenIds.isEmpty()) {
        SkillNode starter;
        starter.name = "Melee Mastery";
        starter.shortDescription = "Basic combat proficiency.";
        starter.longDescription = "Use [Strength] for heavy weapons. Attack roll: 1d20+2.";
        starter.customFields.insert("Cost", "1 point");
        m_model->tree().createNode(m_model->tree().root()->id, starter);

        QMap<QString, QString> help;
        help.insert("Strength", "Primary attribute used for physical attacks.");
        m_model->tree().setHelpEntries(help);
    }
}
