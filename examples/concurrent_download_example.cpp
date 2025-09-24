#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "../service/ConcurrentDownloadManager.h"
#include "../common/AppConfig.h"
#include "../data/DatabaseManager.h"

class ConcurrentDownloadExample : public QObject {
    Q_OBJECT

public:
    explicit ConcurrentDownloadExample(QObject* parent = nullptr) : QObject(parent) {}

    void demonstrateConcurrentDownload() {
        qDebug() << "=== 并发下载示例 ===\n";

        // 创建并发下载管理器
        m_manager = new ConcurrentDownloadManager(this);

        // 配置并发下载参数
        ConcurrentDownloadConfig config = ConcurrentDownloadConfig::getAggressive(); // 使用激进配置
        m_manager->setConfig(config);

        qDebug() << "配置信息:";
        qDebug() << "  最大并发数:" << config.maxConcurrentDownloads;
        qDebug() << "  最大重试次数:" << config.maxRetryCount;
        qDebug() << "  任务超时时间:" << config.taskTimeoutMs << "毫秒";

        // 连接关键信号
        connect(m_manager, &ConcurrentDownloadManager::taskCompleted,
            this, &ConcurrentDownloadExample::onTaskCompleted);
        connect(m_manager, &ConcurrentDownloadManager::taskFailed,
            this, &ConcurrentDownloadExample::onTaskFailed);
        connect(m_manager, &ConcurrentDownloadManager::allTasksCompleted,
            this, &ConcurrentDownloadExample::onAllCompleted);

        // 模拟用户导入播放列表的场景
        QStringList playlistItems = {
            "BV1xx411c7mD",
            "BV1GJ411x7h7",
            "BV1ef4y1H7h5",
            "BV1Lx411G7kP",
            "BV1mx41137Xx"
        };

        qDebug() << "\n模拟导入播放列表，包含" << playlistItems.size() << "首歌曲：";

        // 批量添加任务（模拟导入时检测并下载不存在的歌曲）
        DownloadOptions options = DownloadOptions::createPreset("high_quality_mp3");
        QStringList taskIds = m_manager->addBatchTasks(playlistItems, options);

        qDebug() << "实际需要下载的歌曲数量:" << taskIds.size();
        qDebug() << "\n开始并发下载...\n";

        // 启动定时统计显示
        QTimer* statsTimer = new QTimer(this);
        connect(statsTimer, &QTimer::timeout, this, &ConcurrentDownloadExample::showProgress);
        statsTimer->start(3000); // 每3秒显示一次进度

        m_totalTasks = taskIds.size();
    }

private slots:
    void onTaskCompleted(const QString& taskId, const Song& song) {
        m_completedCount++;
        qDebug() << QString("✅ [%1/%2] 完成下载: %3")
            .arg(m_completedCount)
            .arg(m_totalTasks)
            .arg(song.getTitle());
    }

    void onTaskFailed(const QString& taskId, const QString& error) {
        m_failedCount++;
        DownloadTaskState task = m_manager->getTask(taskId);
        qDebug() << QString("❌ [%1/%2] 下载失败: %3 - %4")
            .arg(m_completedCount + m_failedCount)
            .arg(m_totalTasks)
            .arg(task.getIdentifier())
            .arg(error);
    }

    void onAllCompleted() {
        qDebug() << "\n🎉 播放列表导入完成！";

        auto stats = m_manager->getStatistics();
        qDebug() << "\n📊 导入结果统计:";
        qDebug() << "  成功:" << stats.completedTasks << "首";
        qDebug() << "  失败:" << stats.failedTasks << "首";
        qDebug() << "  成功率:" << QString::number((double)stats.completedTasks / stats.totalTasks * 100, 'f', 1) + "%";

        if (stats.averageDownloadTimeMs > 0) {
            qDebug() << "  平均下载时间:" << QString::number(stats.averageDownloadTimeMs / 1000.0, 'f', 1) << "秒";
        }

        QTimer::singleShot(2000, []() { QCoreApplication::quit(); });
    }

    void showProgress() {
        auto stats = m_manager->getStatistics();

        if (stats.totalTasks > 0) {
            qDebug() << QString("📈 进度更新: %1% (%2/%3 完成, %4 进行中, %5 等待中)")
                .arg(QString::number(stats.overallProgress * 100, 'f', 1))
                .arg(stats.completedTasks)
                .arg(stats.totalTasks)
                .arg(stats.activeTasks)
                .arg(stats.pendingTasks);
        }
    }

private:
    ConcurrentDownloadManager* m_manager;
    int m_totalTasks = 0;
    int m_completedCount = 0;
    int m_failedCount = 0;
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // 初始化
    AppConfig::instance().loadConfig();
    DatabaseManager::instance().initialize();

    if (!AppConfig::instance().isYtDlpAvailable()) {
        qDebug() << "❌ yt-dlp 不可用，请先配置 yt-dlp 路径";
        return 1;
    }

    ConcurrentDownloadExample example;
    QTimer::singleShot(500, &example, &ConcurrentDownloadExample::demonstrateConcurrentDownload);

    return app.exec();
}

#include "concurrent_download_example.moc"
