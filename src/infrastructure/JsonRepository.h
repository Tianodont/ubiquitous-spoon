#pragma once

#include "domain/SkillTree.h"

#include <QString>

class JsonRepository {
public:
    bool save(const SkillTree& tree, const QString& path, QString* errorMessage = nullptr) const;
    bool load(SkillTree* tree, const QString& path, QString* errorMessage = nullptr) const;
};
