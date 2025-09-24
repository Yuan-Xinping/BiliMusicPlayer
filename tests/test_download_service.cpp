#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "../service/DownloadService.h"
#include "../common/AppConfig.h"
#include "../data/DatabaseManager.h"

class DownloadServiceTester : public QObject {
    Q_OBJECT

public:
    explicit DownloadServiceTester(QObject* parent = nullptr) : QObject(parent) {}

    void testDownloadService() {
        qDebug() << "\n=== 测试完整下载服务 ===";

        if (!AppConfig::instance().isYtDlpAvailable()) {
            qDebug() << "⚠️ yt-dlp 不可用，跳过测试";
            QCoreApplication::quit();
            return;
        }

        DownloadService* service = new DownloadService(this);

        // 连接所有信号
        connect(service, &DownloadService::taskAdded, this, &DownloadServiceTester::onTaskAdded);
        connect(service, &DownloadService::taskStarted, this, &DownloadServiceTester::onTaskStarted);
        connect(service, &DownloadService::taskProgress, this, &DownloadServiceTester::onTaskProgress);
        connect(service, &DownloadService::taskCompleted, this, &DownloadServiceTester::onTaskCompleted);
        connect(service, &DownloadService::taskFailed, this, &DownloadServiceTester::onTaskFailed);
        connect(service, &DownloadService::allTasksCompleted, this, &DownloadServiceTester::onAllTasksCompleted);

        // 添加测试任务
        qDebug() << "添加测试下载任务...";

        // 测试不同预设
        service->addDownloadTask("BV1xx411c7mD", "small_size_opus");
        service->addDownloadTask("BV1GJ411x7h7", "medium_quality_mp3");

        // 测试自定义配置
        DownloadOptions customOptions = DownloadOptions::createPreset("high_quality_mp3");
        customOptions.embedThumbnail = true;
        customOptions.writeDescription = true;
        service->addDownloadTask("BV1xx411c7mD", customOptions);

        qDebug() << "总任务数:" << service->getQueueSize();

        // 开始下载
        service->startDownload();
    }

private slots:
    void onTaskAdded(const DownloadService::DownloadTask& task) {
        qDebug() << "✓ 任务已添加:" << task.identifier;
    }

    void onTaskStarted(const DownloadService::DownloadTask& task) {
        qDebug() << "▶ 开始下载:" << task.identifier;
    }

    void onTaskProgress(const DownloadService::DownloadTask& task, double progress, const QString& message) {
        qDebug() << QString("📥 [%1] %2% - %3")
            .arg(task.identifier)
            .arg(progress * 100, 0, 'f', 1)
            .arg(message);
    }

    void onTaskCompleted(const DownloadService::DownloadTask& task, const Song& song) {
        qDebug() << "✅ 下载完成:" << task.identifier;
        qDebug() << "   标题:" << song.getTitle();
        qDebug() << "   艺术家:" << song.getArtist();
        qDebug() << "   文件:" << song.getLocalFilePath();
        qDebug() << "   时长:" << song.getDurationSeconds() << "秒";
    }

    void onTaskFailed(const DownloadService::DownloadTask& task, const QString& error) {
        qDebug() << "❌ 下载失败:" << task.identifier;
        qDebug() << "   错误:" << error;
    }

    void onAllTasksCompleted() {
        qDebug() << "\n🎉 所有下载任务完成！";
        QCoreApplication::quit();
    }
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // 初始化
    AppConfig::instance().loadConfig();
    DatabaseManager::instance().initialize();

    DownloadServiceTester tester;
    QTimer::singleShot(100, &tester, &DownloadServiceTester::testDownloadService);

    return app.exec();
}

#include "test_download_service.moc"
