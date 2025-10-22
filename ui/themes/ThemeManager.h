#pragma once
#include <QObject>
#include <QColor>
#include <QMap>
#include <QString>

class QWidget;
class QApplication;

class ThemeManager : public QObject {
    Q_OBJECT

public:
    enum class Theme {
        Dark = 0,
        Light = 1
    };
    Q_ENUM(Theme)

        static ThemeManager& instance();

    // 主题加载
    bool loadTheme(Theme theme);
    bool loadTheme(const QString& themeName);
    bool reloadCurrentTheme();

    // 应用主题
    void applyToWidget(QWidget* widget);
    void applyToApplication(QApplication* app);

    // 主题信息
    Theme currentTheme() const { return m_currentTheme; }
    QString currentThemeName() const;
    QString getStyleSheet() const { return m_currentStyleSheet; }

    // 颜色访问（用于动态绘制）
    QColor getColor(const QString& colorKey) const;
    QColor getPrimaryColor() const;
    QColor getBackgroundColor() const;
    QColor getSurfaceColor() const;
    QColor getTextColor() const;
    QColor getTextSecondaryColor() const;

signals:
    void themeChanged(Theme theme);

private:
    ThemeManager();
    ~ThemeManager() = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    QString loadThemeFile(const QString& themeName);
    void parseColorVariables(const QString& qss);
    QString getThemeFilePath(Theme theme) const;
    Theme stringToTheme(const QString& themeName) const;

    Theme m_currentTheme = Theme::Dark;
    QString m_currentStyleSheet;
    QMap<QString, QColor> m_colorMap;
};
