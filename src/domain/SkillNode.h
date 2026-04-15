#pragma once

#include <QMap>
#include <QVector>
#include <QString>
#include <QStringList>

struct SkillNode {
    struct Resource {
        QString name;
        int current = 0;
        int max = -1; // -1 means no cap
    };

    QString id;
    QString parentId;
    QString name;
    QString iconPath;
    QString shortDescription;
    QString longDescription;
    bool isLocked = true;
    bool isCollapsed = false;
    QVector<Resource> resources;
    QMap<QString, QString> customFields;
    QStringList childrenIds;
};
