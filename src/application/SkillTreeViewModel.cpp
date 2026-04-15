#include "SkillTreeViewModel.h"

#include "infrastructure/JsonRepository.h"

#include <QMap>
#include <QQueue>
#include <QSet>
#include <QStandardPaths>
#include <QUrl>
#include <QUuid>
#include <QtGlobal>

SkillTreeViewModel::SkillTreeViewModel(QObject* parent)
    : QObject(parent)
    , m_themeManager(this)
    , m_settings(QStringLiteral("SkillTreeApp"), QStringLiteral("SkillTreeApp")) {
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_dataPath = baseDir + "/skill_tree.json";

    JsonRepository repo;
    QString error;
    repo.load(&m_tree, m_dataPath, &error);
    if (!error.isEmpty()) {
        m_operationMessage = error;
    }

    ensureSeedData();
    if (!m_tree.root()->childrenIds.isEmpty()) {
        m_selectedId = m_tree.root()->childrenIds.first();
    }
    m_editorPanelWidth = qBound(320, m_settings.value(QStringLiteral("ui/editorPanelWidth"), 440).toInt(), 800);
}

QVariantList SkillTreeViewModel::skillNodes() const {
    return buildLayoutNodes();
}

QVariantList SkillTreeViewModel::skillEdges() const {
    return buildLayoutEdges(buildLayoutNodes());
}

QVariantMap SkillTreeViewModel::nodeToMap(const SkillNode& node, double x, double y, int depth) const {
    QVariantMap map;
    QVector<SkillNode::Resource> resources = node.resources;
    if (resources.isEmpty()) {
        resources.append(SkillNode::Resource{QStringLiteral("Resource"), 0, -1});
    }
    QVariantList resourcesList;
    for (const SkillNode::Resource& resource : resources) {
        QVariantMap resourceMap;
        resourceMap.insert("name", resource.name);
        resourceMap.insert("current", resource.current);
        resourceMap.insert("max", resource.max);
        resourcesList.append(resourceMap);
    }
    const SkillNode::Resource primary = resources.first();

    map.insert("id", node.id);
    map.insert("parentId", node.parentId);
    map.insert("name", node.name);
    map.insert("iconPath", node.iconPath);
    map.insert("shortDescription", node.shortDescription);
    map.insert("longDescription", node.longDescription);
    map.insert("isLocked", node.isLocked);
    map.insert("isCollapsed", node.isCollapsed);
    map.insert("resourceCurrent", primary.current);
    map.insert("resourceMax", primary.max);
    map.insert("resources", resourcesList);
    QVariantMap custom;
    for (auto it = node.customFields.begin(); it != node.customFields.end(); ++it) {
        custom.insert(it.key(), it.value());
    }
    map.insert("customFields", custom);
    map.insert("x", x);
    map.insert("y", y);
    map.insert("depth", depth);
    map.insert("selected", node.id == m_selectedId);
    return map;
}

QVariantList SkillTreeViewModel::buildLayoutNodes() const {
    QVariantList output;
    const SkillNode* root = m_tree.root();
    if (!root) {
        return output;
    }

    QMap<int, QList<QString>> levels;
    QSet<QString> hiddenIds;
    QQueue<QPair<QString, int>> queue;

    for (const QString& id : root->childrenIds) {
        queue.enqueue({id, 0});
    }

    while (!queue.isEmpty()) {
        const auto pair = queue.dequeue();
        const SkillNode* node = m_tree.nodeById(pair.first);
        if (!node) {
            continue;
        }

        levels[pair.second].append(node->id);
        for (const QString& childId : node->childrenIds) {
            queue.enqueue({childId, pair.second + 1});
        }
    }

    QQueue<QString> hiddenQueue;
    for (const auto& levelIds : levels) {
        for (const QString& id : levelIds) {
            const SkillNode* node = m_tree.nodeById(id);
            if (node && node->isCollapsed) {
                for (const QString& childId : node->childrenIds) {
                    hiddenQueue.enqueue(childId);
                }
            }
        }
    }

    while (!hiddenQueue.isEmpty()) {
        const QString id = hiddenQueue.dequeue();
        if (hiddenIds.contains(id)) {
            continue;
        }
        hiddenIds.insert(id);
        const SkillNode* node = m_tree.nodeById(id);
        if (!node) {
            continue;
        }
        for (const QString& childId : node->childrenIds) {
            hiddenQueue.enqueue(childId);
        }
    }

    const double xSpacing = 220.0;
    const double ySpacing = 180.0;

    for (auto it = levels.begin(); it != levels.end(); ++it) {
        const int depth = it.key();
        const QList<QString>& levelIds = it.value();
        const double width = (levelIds.size() - 1) * xSpacing;
        for (int i = 0; i < levelIds.size(); ++i) {
            const SkillNode* node = m_tree.nodeById(levelIds.at(i));
            if (!node || hiddenIds.contains(node->id)) {
                continue;
            }
            const double x = i * xSpacing - (width / 2.0);
            const double y = depth * ySpacing;
            output.append(nodeToMap(*node, x, y, depth));
        }
    }

    return output;
}

QVariantList SkillTreeViewModel::buildLayoutEdges(const QVariantList& nodes) const {
    QVariantList edges;
    QMap<QString, QVariantMap> byId;
    QSet<QString> selectedPathIds;
    for (const QVariant& nodeVar : nodes) {
        const QVariantMap node = nodeVar.toMap();
        byId.insert(node.value("id").toString(), node);
    }
    QString walkId = m_selectedId;
    while (!walkId.isEmpty()) {
        const SkillNode* node = m_tree.nodeById(walkId);
        if (!node) {
            break;
        }
        selectedPathIds.insert(node->id);
        walkId = node->parentId;
    }

    for (auto it = byId.begin(); it != byId.end(); ++it) {
        const QVariantMap node = it.value();
        const QString parentId = node.value("parentId").toString();
        if (parentId.isEmpty() || !byId.contains(parentId)) {
            continue;
        }

        const QVariantMap parent = byId.value(parentId);
        QVariantMap edge;
        edge.insert("x1", parent.value("x").toDouble());
        edge.insert("y1", parent.value("y").toDouble());
        edge.insert("x2", node.value("x").toDouble());
        edge.insert("y2", node.value("y").toDouble());
        edge.insert("active", selectedPathIds.contains(node.value("id").toString()) && selectedPathIds.contains(parentId));
        edges.append(edge);
    }

    return edges;
}

QVariantMap SkillTreeViewModel::selectedSkill() const {
    const SkillNode* node = m_tree.nodeById(m_selectedId);
    if (!node) {
        return {};
    }
    return nodeToMap(*node);
}

QVariantMap SkillTreeViewModel::helpEntries() const {
    QVariantMap map;
    for (auto it = m_tree.helpEntries().begin(); it != m_tree.helpEntries().end(); ++it) {
        map.insert(it.key(), it.value());
    }
    return map;
}

bool SkillTreeViewModel::editorMode() const {
    return m_editorMode;
}

void SkillTreeViewModel::setEditorMode(bool value) {
    if (m_editorMode == value) {
        return;
    }
    m_editorMode = value;
    emit editorModeChanged();
}

QString SkillTreeViewModel::lastRollExpression() const {
    return m_lastRollExpression;
}

int SkillTreeViewModel::lastRollTotal() const {
    return m_lastRollTotal;
}

QString SkillTreeViewModel::lastRollBreakdown() const {
    return m_lastRollBreakdown;
}

QString SkillTreeViewModel::saveError() const {
    return m_saveError;
}

QString SkillTreeViewModel::operationMessage() const {
    return m_operationMessage;
}

int SkillTreeViewModel::editorPanelWidth() const {
    return m_editorPanelWidth;
}

void SkillTreeViewModel::setEditorPanelWidth(int value) {
    const int clamped = qBound(320, value, 800);
    if (m_editorPanelWidth == clamped) {
        return;
    }
    m_editorPanelWidth = clamped;
    m_settings.setValue(QStringLiteral("ui/editorPanelWidth"), clamped);
    emit editorPanelWidthChanged();
}

ThemeManager* SkillTreeViewModel::themeManager() {
    return &m_themeManager;
}

void SkillTreeViewModel::selectSkill(const QString& id) {
    if (id.isEmpty() || !m_tree.nodeById(id)) {
        return;
    }

    if (m_selectedId == id) {
        return;
    }

    m_selectedId = id;
    emit selectedSkillChanged();
    emit skillNodesChanged();
}

void SkillTreeViewModel::createChildSkill(const QString& parentId) {
    m_editingId.clear();
    m_creatingParentId = parentId;
}

void SkillTreeViewModel::updateOrCreateSkill(const QVariantMap& payload) {
    setSaveError(QString());
    SkillNode node;
    node.id = payload.value("id").toString().trimmed();
    node.parentId = payload.value("parentId").toString();
    node.name = payload.value("name").toString();
    node.iconPath = payload.value("iconPath").toString();
    node.shortDescription = payload.value("shortDescription").toString();
    node.longDescription = payload.value("longDescription").toString();
    node.isLocked = payload.contains("isLocked") ? payload.value("isLocked").toBool() : true;
    node.isCollapsed = payload.contains("isCollapsed") ? payload.value("isCollapsed").toBool() : false;
    node.resources = parseResources(payload.value("resources"));
    if (node.resources.isEmpty()) {
        SkillNode::Resource fallback;
        fallback.name = QStringLiteral("Resource");
        fallback.current = payload.contains("resourceCurrent") ? payload.value("resourceCurrent").toInt() : 0;
        fallback.max = payload.contains("resourceMax") ? payload.value("resourceMax").toInt() : -1;
        node.resources.append(sanitizeResource(fallback));
    }

    const QVariantMap fields = payload.value("customFields").toMap();
    for (auto it = fields.begin(); it != fields.end(); ++it) {
        node.customFields.insert(it.key(), it.value().toString());
    }

    if (node.id.isEmpty()) {
        node.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    const bool isExisting = m_tree.nodeById(node.id) != nullptr;
    if (!isExisting && m_tree.containsId(node.id)) {
        setSaveError(QStringLiteral("Skill ID already exists: %1").arg(node.id));
        return;
    }

    if (!isExisting) {
        const QString parent = node.parentId.isEmpty() ? m_tree.root()->id : node.parentId;
        const QString newId = m_tree.createNode(parent, node);
        if (!newId.isEmpty()) {
            m_selectedId = newId;
            setOperationMessage(QStringLiteral("Skill created"));
        }
    } else {
        const SkillNode* existing = m_tree.nodeById(node.id);
        node.childrenIds = existing->childrenIds;
        if (!m_tree.updateNode(node)) {
            setSaveError(QStringLiteral("Failed to update skill due to integrity rules."));
            return;
        }
        m_selectedId = node.id;
        setOperationMessage(QStringLiteral("Skill updated"));
    }

    persist();
    emitTreeChanged();
}

void SkillTreeViewModel::deleteSkill(const QString& id) {
    const SkillNode* node = m_tree.nodeById(id);
    if (!node || node->parentId.isEmpty()) {
        return;
    }

    const QString parentId = node->parentId;
    if (m_tree.removeNode(id)) {
        m_selectedId = parentId;
        persist();
        emitTreeChanged();
    }
}

bool SkillTreeViewModel::rollDice(const QString& expression) {
    if (m_selectedId.isEmpty()) {
        return false;
    }
    return rollDiceForSkill(m_selectedId, expression);
}

bool SkillTreeViewModel::rollDiceForSkill(const QString& skillId, const QString& expression) {
    const SkillNode* node = m_tree.nodeById(skillId);
    if (!node || node->isLocked) {
        return false;
    }

    const auto result = m_dice.roll(expression);
    if (!result.ok) {
        return false;
    }

    m_lastRollExpression = expression;
    m_lastRollTotal = result.total;
    m_lastRollBreakdown = result.breakdown;
    emit lastRollChanged();
    return true;
}

QString SkillTreeViewModel::helpText(const QString& term) const {
    return m_tree.helpEntries().value(term);
}

void SkillTreeViewModel::setHelpEntry(const QString& term, const QString& text) {
    if (term.trimmed().isEmpty()) {
        return;
    }

    QMap<QString, QString> map = m_tree.helpEntries();
    map.insert(term.trimmed(), text.trimmed());
    m_tree.setHelpEntries(map);
    persist();
    emit helpEntriesChanged();
}

void SkillTreeViewModel::removeHelpEntry(const QString& term) {
    QMap<QString, QString> map = m_tree.helpEntries();
    map.remove(term);
    m_tree.setHelpEntries(map);
    persist();
    emit helpEntriesChanged();
}

QVariantMap SkillTreeViewModel::emptySkillForParent(const QString& parentId) const {
    QVariantMap defaultResource;
    defaultResource.insert("name", "Resource");
    defaultResource.insert("current", 0);
    defaultResource.insert("max", -1);
    return {
        {"id", ""},
        {"parentId", parentId},
        {"name", ""},
        {"iconPath", ""},
        {"shortDescription", ""},
        {"longDescription", ""},
        {"isLocked", true},
        {"isCollapsed", false},
        {"resourceCurrent", 0},
        {"resourceMax", -1},
        {"resources", QVariantList{defaultResource}},
        {"customFields", QVariantMap{}}
    };
}

void SkillTreeViewModel::toggleSkillLock(const QString& id) {
    SkillNode* node = m_tree.nodeById(id);
    if (!node || node->parentId.isEmpty()) {
        return;
    }
    node->isLocked = !node->isLocked;
    persist();
    emitTreeChanged();
}

void SkillTreeViewModel::adjustResource(const QString& skillId, int delta) {
    adjustResourceAt(skillId, 0, delta);
}

void SkillTreeViewModel::resetResource(const QString& skillId) {
    setResourceAtCurrent(skillId, 0, 0);
}

void SkillTreeViewModel::setResourceCurrent(const QString& skillId, int value) {
    setResourceAtCurrent(skillId, 0, value);
}

void SkillTreeViewModel::setResourceMax(const QString& skillId, int value) {
    setResourceAtMax(skillId, 0, value);
}

bool SkillTreeViewModel::exportTree(const QString& fileUrl) {
    const QString path = normalizeFileUrl(fileUrl);
    if (path.isEmpty()) {
        setOperationMessage(QStringLiteral("Export cancelled: invalid path."));
        return false;
    }
    JsonRepository repo;
    QString error;
    const bool ok = repo.save(m_tree, path, &error);
    setOperationMessage(ok ? QStringLiteral("Tree exported successfully.") : QStringLiteral("Export failed: %1").arg(error));
    return ok;
}

bool SkillTreeViewModel::importTree(const QString& fileUrl) {
    const QString path = normalizeFileUrl(fileUrl);
    if (path.isEmpty()) {
        setOperationMessage(QStringLiteral("Import cancelled: invalid path."));
        return false;
    }

    SkillTree importedTree;
    JsonRepository repo;
    QString error;
    const bool ok = repo.load(&importedTree, path, &error);
    if (!ok) {
        setOperationMessage(QStringLiteral("Import failed: %1").arg(error));
        return false;
    }

    m_tree = std::move(importedTree);
    const SkillNode* root = m_tree.root();
    m_selectedId = (root && !root->childrenIds.isEmpty()) ? root->childrenIds.first() : QString();
    persist();
    emitTreeChanged();
    emit helpEntriesChanged();
    setOperationMessage(QStringLiteral("Tree imported successfully."));
    return true;
}

void SkillTreeViewModel::addResource(const QString& skillId) {
    SkillNode* node = m_tree.nodeById(skillId);
    if (!node || node->isLocked) {
        return;
    }
    SkillNode::Resource resource;
    resource.name = QStringLiteral("Resource %1").arg(node->resources.size() + 1);
    node->resources.append(resource);
    persist();
    emit selectedSkillChanged();
    emit skillNodesChanged();
}

void SkillTreeViewModel::removeResource(const QString& skillId, int index) {
    SkillNode* node = m_tree.nodeById(skillId);
    if (!node || node->isLocked) {
        return;
    }
    if (index < 0 || index >= node->resources.size()) {
        return;
    }
    if (node->resources.size() == 1) {
        node->resources[0].current = 0;
        node->resources[0].max = -1;
        node->resources[0].name = QStringLiteral("Resource");
    } else {
        node->resources.removeAt(index);
    }
    persist();
    emit selectedSkillChanged();
    emit skillNodesChanged();
}

void SkillTreeViewModel::renameResource(const QString& skillId, int index, const QString& name) {
    SkillNode* node = m_tree.nodeById(skillId);
    if (!node || node->isLocked) {
        return;
    }
    if (index < 0 || index >= node->resources.size()) {
        return;
    }
    node->resources[index].name = name.trimmed().isEmpty() ? QStringLiteral("Resource") : name.trimmed();
    persist();
    emit selectedSkillChanged();
    emit skillNodesChanged();
}

void SkillTreeViewModel::setResourceAtCurrent(const QString& skillId, int index, int value) {
    SkillNode* node = m_tree.nodeById(skillId);
    if (!node || node->isLocked) {
        return;
    }
    if (index < 0 || index >= node->resources.size()) {
        return;
    }
    SkillNode::Resource& resource = node->resources[index];
    resource.current = qMax(0, value);
    if (resource.max >= 0) {
        resource.current = qMin(resource.current, resource.max);
    }
    persist();
    emit selectedSkillChanged();
    emit skillNodesChanged();
}

void SkillTreeViewModel::setResourceAtMax(const QString& skillId, int index, int value) {
    SkillNode* node = m_tree.nodeById(skillId);
    if (!node || node->isLocked) {
        return;
    }
    if (index < 0 || index >= node->resources.size()) {
        return;
    }
    SkillNode::Resource& resource = node->resources[index];
    resource.max = value < 0 ? -1 : value;
    if (resource.max >= 0) {
        resource.current = qMin(resource.current, resource.max);
    }
    persist();
    emit selectedSkillChanged();
    emit skillNodesChanged();
}

void SkillTreeViewModel::adjustResourceAt(const QString& skillId, int index, int delta) {
    SkillNode* node = m_tree.nodeById(skillId);
    if (!node || node->isLocked) {
        return;
    }
    if (index < 0 || index >= node->resources.size()) {
        return;
    }
    SkillNode::Resource& resource = node->resources[index];
    int next = resource.current + delta;
    if (resource.max >= 0) {
        next = qBound(0, next, resource.max);
    } else {
        next = qMax(0, next);
    }
    resource.current = next;
    persist();
    emit selectedSkillChanged();
    emit skillNodesChanged();
}

void SkillTreeViewModel::toggleCollapse(const QString& id) {
    SkillNode* node = m_tree.nodeById(id);
    if (!node || node->parentId.isEmpty()) {
        return;
    }
    node->isCollapsed = !node->isCollapsed;
    persist();
    emitTreeChanged();
}

void SkillTreeViewModel::ensureSeedData() {
    if (!m_tree.root()->childrenIds.isEmpty()) {
        return;
    }

    SkillNode starter;
    starter.name = "Melee Mastery";
    starter.shortDescription = "Basic combat proficiency.";
    starter.longDescription = "Use [Strength] for heavy weapons. Attack roll: 1d20+2.";
    starter.customFields.insert("Cost", "1 point");
    starter.isLocked = true;
    starter.resources.append(SkillNode::Resource{QStringLiteral("Energy"), 0, 5});
    m_tree.createNode(m_tree.root()->id, starter);

    QMap<QString, QString> help;
    help.insert("Strength", "Primary attribute used for physical attacks.");
    m_tree.setHelpEntries(help);

    persist();
}

void SkillTreeViewModel::persist() {
    JsonRepository repo;
    QString error;
    repo.save(m_tree, m_dataPath, &error);
}

void SkillTreeViewModel::emitTreeChanged() {
    emit skillNodesChanged();
    emit selectedSkillChanged();
}

SkillNode::Resource SkillTreeViewModel::sanitizeResource(const SkillNode::Resource& resource) const {
    SkillNode::Resource sanitized = resource;
    sanitized.name = sanitized.name.trimmed().isEmpty() ? QStringLiteral("Resource") : sanitized.name.trimmed();
    sanitized.current = qMax(0, sanitized.current);
    sanitized.max = sanitized.max < 0 ? -1 : sanitized.max;
    if (sanitized.max >= 0) {
        sanitized.current = qMin(sanitized.current, sanitized.max);
    }
    return sanitized;
}

QVector<SkillNode::Resource> SkillTreeViewModel::parseResources(const QVariant& value) const {
    QVector<SkillNode::Resource> parsed;
    const QVariantList list = value.toList();
    for (const QVariant& item : list) {
        const QVariantMap map = item.toMap();
        SkillNode::Resource resource;
        resource.name = map.value("name").toString();
        resource.current = map.value("current").toInt();
        resource.max = map.contains("max") ? map.value("max").toInt() : -1;
        parsed.append(sanitizeResource(resource));
    }
    return parsed;
}

QString SkillTreeViewModel::normalizeFileUrl(const QString& fileUrl) const {
    const QUrl url(fileUrl);
    if (url.isLocalFile()) {
        return url.toLocalFile();
    }
    return fileUrl;
}

void SkillTreeViewModel::setSaveError(const QString& message) {
    if (m_saveError == message) {
        return;
    }
    m_saveError = message;
    emit saveErrorChanged();
}

void SkillTreeViewModel::setOperationMessage(const QString& message) {
    if (m_operationMessage == message) {
        return;
    }
    m_operationMessage = message;
    emit operationMessageChanged();
}
