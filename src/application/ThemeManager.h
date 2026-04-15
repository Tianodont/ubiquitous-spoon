#pragma once

#include <QObject>
#include <QVariantMap>

class ThemeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentThemeChanged)
    Q_PROPERTY(QVariantMap palette READ palette NOTIFY paletteChanged)

public:
    explicit ThemeManager(QObject* parent = nullptr);

    QString currentTheme() const;
    QVariantMap palette() const;

public slots:
    void setCurrentTheme(const QString& themeName);

signals:
    void currentThemeChanged();
    void paletteChanged();

private:
    QVariantMap darkRpgPalette() const;
    QVariantMap lightPalette() const;
    QVariantMap cyberPalette() const;

    QString m_currentTheme = "dark";
};
