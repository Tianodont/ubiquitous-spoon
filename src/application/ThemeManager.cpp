#include "ThemeManager.h"

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent) {
}

QString ThemeManager::currentTheme() const {
    return m_currentTheme;
}

QVariantMap ThemeManager::palette() const {
    if (m_currentTheme == "light") {
        return lightPalette();
    }
    if (m_currentTheme == "cyber") {
        return cyberPalette();
    }
    return darkRpgPalette();
}

void ThemeManager::setCurrentTheme(const QString& themeName) {
    const QString normalized =
        (themeName == "light") ? "light" :
        (themeName == "cyber") ? "cyber" :
        "dark";
    if (normalized == m_currentTheme) {
        return;
    }

    m_currentTheme = normalized;
    emit currentThemeChanged();
    emit paletteChanged();
}

QVariantMap ThemeManager::darkRpgPalette() const {
    return {
        {"windowBg", "#14171f"},
        {"panelBg", "#1b1f2a"},
        {"panelAltBg", "#222839"},
        {"textPrimary", "#e8ecf6"},
        {"textSecondary", "#9ca7bb"},
        {"accent", "#62a2ff"},
        {"accentSoft", "#3f7fcf"},
        {"danger", "#cd5c6a"},
        {"nodeFill", "#2a3145"},
        {"nodeHoverFill", "#323b54"},
        {"nodeLockedFill", "#202531"},
        {"nodeGlow", "#4f7fc8"},
        {"nodeSelectedGlow", "#62a2ff"},
        {"nodeLockedOverlay", "#66000000"},
        {"nodeBorder", "#4a5574"},
        {"nodeSelectedBorder", "#62a2ff"},
        {"edgeColor", "#4d5b7f"},
        {"edgeActiveColor", "#7cb0ff"},
        {"edgeGlowColor", "#2f4f84"},
        {"edgeWidth", 2.8},
        {"edgeActiveWidth", 3.6},
        {"treeBg", "#14171f"},
        {"treeGridColor", "#223049"},
        {"menuShadow", "#30000000"},
        {"menuShadowOffsetY", 2},
        {"tooltipBg", "#212838"},
        {"tooltipText", "#f0f3fb"},
        {"menuBg", "#202636"},
        {"menuHoverBg", "#2a3348"},
        {"inputBg", "#1f2533"},
        {"inputBorder", "#3b4661"}
    };
}

QVariantMap ThemeManager::lightPalette() const {
    return {
        {"windowBg", "#eceff5"},
        {"panelBg", "#f8f9fc"},
        {"panelAltBg", "#ffffff"},
        {"textPrimary", "#2a3445"},
        {"textSecondary", "#657286"},
        {"accent", "#376fd6"},
        {"accentSoft", "#5d8adf"},
        {"danger", "#b2505d"},
        {"nodeFill", "#d8e2f5"},
        {"nodeHoverFill", "#cedbf2"},
        {"nodeLockedFill", "#d2d8e3"},
        {"nodeGlow", "#80a4db"},
        {"nodeSelectedGlow", "#376fd6"},
        {"nodeLockedOverlay", "#55ffffff"},
        {"nodeBorder", "#95a5c5"},
        {"nodeSelectedBorder", "#376fd6"},
        {"edgeColor", "#9daac3"},
        {"edgeActiveColor", "#4a79cf"},
        {"edgeGlowColor", "#bdd0ef"},
        {"edgeWidth", 2.6},
        {"edgeActiveWidth", 3.4},
        {"treeBg", "#edf1f8"},
        {"treeGridColor", "#d2deef"},
        {"menuShadow", "#29000000"},
        {"menuShadowOffsetY", 2},
        {"tooltipBg", "#ffffff"},
        {"tooltipText", "#243146"},
        {"menuBg", "#ffffff"},
        {"menuHoverBg", "#edf2fc"},
        {"inputBg", "#ffffff"},
        {"inputBorder", "#b9c5db"}
    };
}

QVariantMap ThemeManager::cyberPalette() const {
    return {
        {"windowBg", "#0d1020"},
        {"panelBg", "#12162b"},
        {"panelAltBg", "#171d36"},
        {"textPrimary", "#d6f7ff"},
        {"textSecondary", "#7cb4c3"},
        {"accent", "#00d6ff"},
        {"accentSoft", "#3eaec9"},
        {"danger", "#ff5f7a"},
        {"nodeFill", "#182342"},
        {"nodeHoverFill", "#20315a"},
        {"nodeLockedFill", "#111a31"},
        {"nodeGlow", "#1ca1be"},
        {"nodeSelectedGlow", "#00d6ff"},
        {"nodeLockedOverlay", "#66000000"},
        {"nodeBorder", "#2f4b73"},
        {"nodeSelectedBorder", "#00d6ff"},
        {"edgeColor", "#2b6d86"},
        {"edgeActiveColor", "#21c6eb"},
        {"edgeGlowColor", "#174e67"},
        {"edgeWidth", 2.8},
        {"edgeActiveWidth", 3.8},
        {"treeBg", "#0d1020"},
        {"treeGridColor", "#1b3553"},
        {"menuShadow", "#3a00131a"},
        {"menuShadowOffsetY", 2},
        {"tooltipBg", "#141f36"},
        {"tooltipText", "#dffbff"},
        {"menuBg", "#17223f"},
        {"menuHoverBg", "#203056"},
        {"inputBg", "#121a31"},
        {"inputBorder", "#2f4668"}
    };
}
