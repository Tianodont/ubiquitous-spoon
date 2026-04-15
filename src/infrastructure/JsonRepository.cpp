#include "JsonRepository.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QSet>
#include <QUrl>
#include <QtGlobal>
#include <functional>

namespace {
QString resolveIconPathForRead(const QString& sourcePath) {
    if (sourcePath.startsWith("qrc:/")) {
        return QStringLiteral(":/") + sourcePath.mid(5);
    }
    const QUrl url(sourcePath);
    if (url.isLocalFile()) {
        return url.toLocalFile();
    }
    return sourcePath;
}

QString mimeTypeForIcon(const QString& path) {
    QMimeDatabase db;
    const QMimeType mime = db.mimeTypeForFile(path);
    if (mime.isValid() && mime.name().startsWith("image/")) {
        return mime.name();
    }
    const QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix == "jpg" || suffix == "jpeg") return "image/jpeg";
    if (suffix == "svg") return "image/svg+xml";
    if (suffix == "webp") return "image/webp";
    if (suffix == "gif") return "image/gif";
    return "image/png";
}

QString buildIconDataUri(const QString& iconPath) {
    if (iconPath.isEmpty()) {
        return {};
    }
    if (iconPath.startsWith("data:image/")) {
        return iconPath;
    }
    QFile iconFile(resolveIconPathForRead(iconPath));
    if (!iconFile.open(QIODevice::ReadOnly)) {
        return {};
    }
    const QByteArray bytes = iconFile.readAll();
    if (bytes.isEmpty()) {
        return {};
    }
    const QString mime = mimeTypeForIcon(iconFile.fileName());
    return QStringLiteral("data:%1;base64,%2")
        .arg(mime, QString::fromLatin1(bytes.toBase64()));
}

QString normalizedIconSource(const QString& iconPath, const QString& iconBase64) {
    if (iconBase64.startsWith("data:image/")) {
        const int comma = iconBase64.indexOf(',');
        if (comma > 0) {
            const QByteArray payload = QByteArray::fromBase64(iconBase64.mid(comma + 1).toLatin1());
            const QImage image = QImage::fromData(payload);
            if (!image.isNull()) {
                return iconBase64;
            }
        }
    }
    return iconPath;
}
} // namespace

bool JsonRepository::save(const SkillTree& tree, const QString& path, QString* errorMessage) const {
    QJsonObject root;

    QJsonArray nodes;
    for (const auto& nodePair : tree.allNodes()) {
        const SkillNode& node = *nodePair.second;
        QJsonObject jsonNode;
        jsonNode["id"] = node.id;
        jsonNode["parentId"] = node.parentId;
        jsonNode["name"] = node.name;
        jsonNode["iconPath"] = node.iconPath;
        jsonNode["iconBase64"] = buildIconDataUri(node.iconPath);
        jsonNode["shortDescription"] = node.shortDescription;
        jsonNode["longDescription"] = node.longDescription;
        jsonNode["isLocked"] = node.isLocked;
        jsonNode["isCollapsed"] = node.isCollapsed;

        QJsonObject customFields;
        for (auto it = node.customFields.begin(); it != node.customFields.end(); ++it) {
            customFields[it.key()] = it.value();
        }
        jsonNode["customFields"] = customFields;

        QJsonArray children;
        for (const QString& childId : node.childrenIds) {
            children.append(childId);
        }
        jsonNode["children"] = children;

        QJsonArray resources;
        for (const SkillNode::Resource& resource : node.resources) {
            QJsonObject item;
            item["name"] = resource.name;
            item["current"] = resource.current;
            item["max"] = resource.max;
            resources.append(item);
        }
        if (resources.isEmpty()) {
            QJsonObject fallback;
            fallback["name"] = "Resource";
            fallback["current"] = 0;
            fallback["max"] = -1;
            resources.append(fallback);
        }
        jsonNode["resources"] = resources;

        nodes.append(jsonNode);
    }

    root["nodes"] = nodes;

    QJsonObject helpEntries;
    for (auto it = tree.helpEntries().begin(); it != tree.helpEntries().end(); ++it) {
        helpEntries[it.key()] = it.value();
    }
    root["helpEntries"] = helpEntries;

    QFile file(path);
    QFileInfo fileInfo(path);
    QDir().mkpath(fileInfo.absolutePath());

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool JsonRepository::load(SkillTree* tree, const QString& path, QString* errorMessage) const {
    if (!tree) {
        return false;
    }

    QFile file(path);
    if (!file.exists()) {
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = parseError.errorString();
        }
        return false;
    }

    SkillTree rebuilt;

    const QJsonObject root = doc.object();
    const QJsonArray nodes = root["nodes"].toArray();

    QMap<QString, SkillNode> imported;
    QString oldRootId;
    for (const QJsonValue& value : nodes) {
        const QJsonObject obj = value.toObject();
        SkillNode node;
        node.id = obj["id"].toString();
        node.parentId = obj["parentId"].toString();
        node.name = obj["name"].toString();
        node.iconPath = normalizedIconSource(obj["iconPath"].toString(), obj["iconBase64"].toString());
        node.shortDescription = obj["shortDescription"].toString();
        node.longDescription = obj["longDescription"].toString();
        node.isLocked = obj.contains("isLocked") ? obj["isLocked"].toBool() : true;
        node.isCollapsed = obj.contains("isCollapsed") ? obj["isCollapsed"].toBool() : false;

        const QJsonObject fields = obj["customFields"].toObject();
        for (auto it = fields.begin(); it != fields.end(); ++it) {
            node.customFields.insert(it.key(), it.value().toString());
        }

        const QJsonArray children = obj["children"].toArray();
        for (const QJsonValue& child : children) {
            node.childrenIds.append(child.toString());
        }

        const QJsonArray resources = obj["resources"].toArray();
        if (!resources.isEmpty()) {
            for (const QJsonValue& resourceValue : resources) {
                const QJsonObject resourceObj = resourceValue.toObject();
                SkillNode::Resource resource;
                resource.name = resourceObj["name"].toString().trimmed();
                resource.current = qMax(0, resourceObj["current"].toInt());
                resource.max = resourceObj.contains("max") ? resourceObj["max"].toInt() : -1;
                if (resource.max >= 0) {
                    resource.current = qMin(resource.current, resource.max);
                }
                if (resource.name.isEmpty()) {
                    resource.name = "Resource";
                }
                node.resources.append(resource);
            }
        } else {
            SkillNode::Resource fallback;
            fallback.name = "Resource";
            fallback.current = qMax(0, obj.contains("resourceCurrent") ? obj["resourceCurrent"].toInt() : 0);
            fallback.max = obj.contains("resourceMax") ? obj["resourceMax"].toInt() : -1;
            if (fallback.max >= 0) {
                fallback.current = qMin(fallback.current, fallback.max);
            }
            node.resources.append(fallback);
        }

        if (node.parentId.isEmpty()) {
            oldRootId = node.id;
        }
        imported.insert(node.id, node);
    }

    QSet<QString> knownIds;
    for (auto it = imported.begin(); it != imported.end(); ++it) {
        if (it.key().isEmpty() || knownIds.contains(it.key())) {
            if (errorMessage) {
                *errorMessage = "Invalid data: duplicate or empty IDs.";
            }
            return false;
        }
        knownIds.insert(it.key());
    }

    for (auto it = imported.begin(); it != imported.end(); ++it) {
        const SkillNode& node = it.value();
        if (node.parentId.isEmpty()) {
            continue;
        }
        if (!imported.contains(node.parentId)) {
            if (errorMessage) {
                *errorMessage = "Invalid data: orphan node detected.";
            }
            return false;
        }
    }

    if (!oldRootId.isEmpty() && imported.contains(oldRootId)) {
        SkillNode rootData = imported[oldRootId];
        SkillNode mutableRoot = *rebuilt.root();
        mutableRoot.name = rootData.name;
        mutableRoot.iconPath = rootData.iconPath;
        mutableRoot.shortDescription = rootData.shortDescription;
        mutableRoot.longDescription = rootData.longDescription;
        mutableRoot.customFields = rootData.customFields;
        rebuilt.updateNode(mutableRoot);

        QSet<QString> recursionGuard;
        std::function<void(const QString&, const QString&)> copyChildren;
        copyChildren = [&](const QString& newParentId, const QString& oldParentId) {
            if (recursionGuard.contains(oldParentId)) {
                return;
            }
            recursionGuard.insert(oldParentId);
            const SkillNode oldParent = imported.value(oldParentId);
            for (const QString& oldChildId : oldParent.childrenIds) {
                if (!imported.contains(oldChildId) || oldChildId == oldParentId) {
                    continue;
                }
                const SkillNode childData = imported.value(oldChildId);
                QString newChildId = rebuilt.createNode(newParentId, childData);
                if (!newChildId.isEmpty()) {
                    copyChildren(newChildId, oldChildId);
                }
            }
            recursionGuard.remove(oldParentId);
        };

        copyChildren(rebuilt.root()->id, oldRootId);

        const QJsonObject helpEntries = root["helpEntries"].toObject();
        QMap<QString, QString> helpMap;
        for (auto it = helpEntries.begin(); it != helpEntries.end(); ++it) {
            helpMap.insert(it.key(), it.value().toString());
        }
        rebuilt.setHelpEntries(helpMap);
    }

    *tree = std::move(rebuilt);
    return true;
}
