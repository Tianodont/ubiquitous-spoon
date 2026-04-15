#include "SkillEditorPanel.h"

#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>

SkillEditorPanel::SkillEditorPanel(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    auto* form = new QFormLayout();
    m_nameEdit = new QLineEdit(this);

    auto* iconContainer = new QWidget(this);
    auto* iconLayout = new QHBoxLayout(iconContainer);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    m_iconEdit = new QLineEdit(iconContainer);
    auto* browseIcon = new QPushButton("Browse", iconContainer);
    iconLayout->addWidget(m_iconEdit);
    iconLayout->addWidget(browseIcon);

    m_shortEdit = new QTextEdit(this);
    m_shortEdit->setMaximumHeight(80);
    m_longEdit = new QTextEdit(this);

    form->addRow("Name", m_nameEdit);
    form->addRow("Icon", iconContainer);
    form->addRow("Short Description", m_shortEdit);
    form->addRow("Long Description", m_longEdit);

    layout->addLayout(form);

    layout->addWidget(new QLabel("Custom Fields", this));
    m_fieldsTable = new QTableWidget(this);
    m_fieldsTable->setColumnCount(2);
    m_fieldsTable->setHorizontalHeaderLabels({"Key", "Value"});
    m_fieldsTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_fieldsTable);

    auto* fieldsButtons = new QHBoxLayout();
    auto* addField = new QPushButton("Add Field", this);
    auto* removeField = new QPushButton("Remove Field", this);
    fieldsButtons->addWidget(addField);
    fieldsButtons->addWidget(removeField);
    layout->addLayout(fieldsButtons);

    layout->addWidget(new QLabel("Inline Help Entries", this));
    m_helpTable = new QTableWidget(this);
    m_helpTable->setColumnCount(2);
    m_helpTable->setHorizontalHeaderLabels({"Term", "Explanation"});
    m_helpTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_helpTable);

    auto* helpButtons = new QHBoxLayout();
    auto* addHelp = new QPushButton("Add Help", this);
    auto* removeHelp = new QPushButton("Remove Help", this);
    helpButtons->addWidget(addHelp);
    helpButtons->addWidget(removeHelp);
    layout->addLayout(helpButtons);

    auto* actions = new QHBoxLayout();
    auto* save = new QPushButton("Save", this);
    auto* cancel = new QPushButton("Cancel", this);
    actions->addStretch();
    actions->addWidget(save);
    actions->addWidget(cancel);
    layout->addLayout(actions);

    connect(save, &QPushButton::clicked, this, &SkillEditorPanel::saveRequested);
    connect(cancel, &QPushButton::clicked, this, &SkillEditorPanel::cancelRequested);
    connect(addField, &QPushButton::clicked, this, [this] {
        m_fieldsTable->insertRow(m_fieldsTable->rowCount());
    });
    connect(removeField, &QPushButton::clicked, this, [this] {
        if (m_fieldsTable->currentRow() >= 0) {
            m_fieldsTable->removeRow(m_fieldsTable->currentRow());
        }
    });
    connect(addHelp, &QPushButton::clicked, this, [this] {
        m_helpTable->insertRow(m_helpTable->rowCount());
    });
    connect(removeHelp, &QPushButton::clicked, this, [this] {
        if (m_helpTable->currentRow() >= 0) {
            m_helpTable->removeRow(m_helpTable->currentRow());
        }
    });
    connect(browseIcon, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getOpenFileName(this, "Select icon", QString(), "Images (*.png *.jpg *.svg)");
        if (!path.isEmpty()) {
            m_iconEdit->setText(path);
        }
    });
}

void SkillEditorPanel::setTableData(QTableWidget* table, const QMap<QString, QString>& map) {
    table->setRowCount(0);
    for (auto it = map.begin(); it != map.end(); ++it) {
        const int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(it.key()));
        table->setItem(row, 1, new QTableWidgetItem(it.value()));
    }
}

QMap<QString, QString> SkillEditorPanel::tableData(QTableWidget* table) const {
    QMap<QString, QString> data;
    for (int row = 0; row < table->rowCount(); ++row) {
        auto* keyItem = table->item(row, 0);
        auto* valueItem = table->item(row, 1);
        if (!keyItem || keyItem->text().trimmed().isEmpty()) {
            continue;
        }
        data.insert(keyItem->text().trimmed(), valueItem ? valueItem->text().trimmed() : QString());
    }
    return data;
}

void SkillEditorPanel::setEditMode(const SkillNode* node, const QMap<QString, QString>& helpEntries) {
    m_nameEdit->clear();
    m_iconEdit->clear();
    m_shortEdit->clear();
    m_longEdit->clear();
    setTableData(m_fieldsTable, {});

    if (node) {
        m_nameEdit->setText(node->name);
        m_iconEdit->setText(node->iconPath);
        m_shortEdit->setPlainText(node->shortDescription);
        m_longEdit->setPlainText(node->longDescription);
        setTableData(m_fieldsTable, node->customFields);
    }

    setTableData(m_helpTable, helpEntries);
}

SkillNode SkillEditorPanel::buildNode(const QString& originalId, const QString& parentId) const {
    SkillNode node;
    node.id = originalId;
    node.parentId = parentId;
    node.name = m_nameEdit->text().trimmed();
    node.iconPath = m_iconEdit->text().trimmed();
    node.shortDescription = m_shortEdit->toPlainText().trimmed();
    node.longDescription = m_longEdit->toPlainText().trimmed();
    node.customFields = tableData(m_fieldsTable);
    return node;
}

QMap<QString, QString> SkillEditorPanel::buildHelpEntries() const {
    return tableData(m_helpTable);
}
