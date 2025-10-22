#include "ThemeManager.h"
#include <QFile>
#include <QTextStream>
#include <QWidget>
#include <QApplication>
#include <QDebug>
#include <QRegularExpression>
#include <QDir>

ThemeManager& ThemeManager::instance() {
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager() {
    loadTheme(Theme::Dark);
}

bool ThemeManager::loadTheme(Theme theme) {
    QString themeName = (theme == Theme::Dark) ? "dark" : "light";
    QString qss = loadThemeFile(themeName);

    if (qss.isEmpty()) {
        qWarning() << "❌ 无法加载主题：" << themeName;
        return false;
    }

    m_currentTheme = theme;
    m_currentStyleSheet = qss;
    parseColorVariables(qss);

    qDebug() << "✅ 主题已加载：" << themeName;
    emit themeChanged(theme);
    return true;
}

bool ThemeManager::loadTheme(const QString& themeName) {
    Theme theme = stringToTheme(themeName);
    return loadTheme(theme);
}

bool ThemeManager::reloadCurrentTheme() {
    return loadTheme(m_currentTheme);
}

QString ThemeManager::loadThemeFile(const QString& themeName) {
    // 优先从资源文件加载
    QString resourcePath = QString(":/themes/%1.qss").arg(themeName);
    QFile file(resourcePath);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "❌ 无法打开主题文件：" << resourcePath;

        QString externalPath = QString("ui/themes/%1.qss").arg(themeName);
        QFile externalFile(externalPath);
        if (externalFile.open(QFile::ReadOnly | QFile::Text)) {
            qDebug() << "✅ 从外部文件加载主题：" << externalPath;
            QTextStream in(&externalFile);
            QString content = in.readAll();
            externalFile.close();
            return content;
        }

        return QString();
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    qDebug() << "✅ 主题文件加载成功：" << resourcePath;
    return content;
}

void ThemeManager::parseColorVariables(const QString& qss) {
    m_colorMap.clear();

    // 解析颜色变量（格式: /* @varName: #RRGGBB */）
    QRegularExpression re(R"(/\*\s*@(\w+):\s*(#[0-9A-Fa-f]{6})\s*\*/)");
    QRegularExpressionMatchIterator it = re.globalMatch(qss);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString varName = match.captured(1);
        QString colorHex = match.captured(2);
        m_colorMap[varName] = QColor(colorHex);
    }

    qDebug() << "  - 解析到" << m_colorMap.size() << "个颜色变量";

    for (auto it = m_colorMap.constBegin(); it != m_colorMap.constEnd(); ++it) {
        qDebug() << "    " << it.key() << ":" << it.value().name();
    }
}

QString ThemeManager::currentThemeName() const {
    return (m_currentTheme == Theme::Dark) ? "dark" : "light";
}

void ThemeManager::applyToWidget(QWidget* widget) {
    if (!widget) {
        qWarning() << "⚠️ applyToWidget: widget 为空";
        return;
    }

    widget->setStyleSheet(m_currentStyleSheet);
    qDebug() << "✅ 主题已应用到 widget：" << widget->objectName();
}

void ThemeManager::applyToApplication(QApplication* app) {
    if (!app) {
        qWarning() << "⚠️ applyToApplication: app 为空";
        return;
    }

    app->setStyleSheet(m_currentStyleSheet);
    qDebug() << "✅ 主题已应用到 QApplication";
}

QColor ThemeManager::getColor(const QString& colorKey) const {
    if (!m_colorMap.contains(colorKey)) {
        qWarning() << "⚠️ 未找到颜色变量：" << colorKey;
        return QColor("#FF00FF");  // 洋红色表示未找到
    }
    return m_colorMap.value(colorKey);
}

QColor ThemeManager::getPrimaryColor() const {
    return getColor("primary");
}

QColor ThemeManager::getBackgroundColor() const {
    return getColor("background");
}

QColor ThemeManager::getSurfaceColor() const {
    return getColor("surface");
}

QColor ThemeManager::getTextColor() const {
    return getColor("text");
}

QColor ThemeManager::getTextSecondaryColor() const {
    return getColor("text-secondary");
}

QString ThemeManager::getThemeFilePath(Theme theme) const {
    QString themeName = (theme == Theme::Dark) ? "dark" : "light";
    return QString(":/themes/%1.qss").arg(themeName);
}

ThemeManager::Theme ThemeManager::stringToTheme(const QString& themeName) const {
    if (themeName.toLower() == "light") {
        return Theme::Light;
    }
    return Theme::Dark;  // 默认深色
}
