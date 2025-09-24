#include "BiliMusicPlayerApp.h"
#include "../common/AppConfig.h"
#include "../data/DatabaseManager.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

BiliMusicPlayerApp::BiliMusicPlayerApp(QObject* parent)
    : QObject(parent) {

    // ����Ӧ�ó�����Ϣ
    QCoreApplication::setApplicationName("BiliMusicPlayer");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("BiliMusicPlayer");

    setupLogging();
}

bool BiliMusicPlayerApp::initialize() {
    qDebug() << "���ڳ�ʼ�� BiliMusicPlayer...";

    if (!initializeDatabase()) {
        qCritical() << "���ݿ��ʼ��ʧ��";
        return false;
    }

    if (!initializeServices()) {
        qCritical() << "�����ʼ��ʧ��";
        return false;
    }

    qDebug() << "BiliMusicPlayer ��ʼ���ɹ�";
    return true;
}

bool BiliMusicPlayerApp::initializeDatabase() {
    qDebug() << "��ʼ�����ݿ�...";

    DatabaseManager& dbManager = DatabaseManager::instance();
    if (!dbManager.initialize()) {
        qCritical() << "���ݿ��ʼ��ʧ��";
        return false;
    }

    qDebug() << "���ݿ��ʼ���ɹ�";
    return true;
}

bool BiliMusicPlayerApp::initializeServices() {
    qDebug() << "��ʼ������...";

    // ��������
    AppConfig& config = AppConfig::instance();
    if (!config.loadConfig()) {
        qWarning() << "���ü���ʧ�ܣ���ʹ��Ĭ��ֵ";
    }

    // ȷ������Ŀ¼����
    QDir downloadDir(config.getDownloadPath());
    if (!downloadDir.exists()) {
        if (downloadDir.mkpath(".")) {
            qDebug() << "��������Ŀ¼:" << downloadDir.absolutePath();
        }
        else {
            qWarning() << "�޷���������Ŀ¼:" << downloadDir.absolutePath();
        }
    }

    qDebug() << "�����ʼ���ɹ�";
    return true;
}

void BiliMusicPlayerApp::setupLogging() {
    // ������־��ʽ
    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss}] %{type}: %{message}");
}
