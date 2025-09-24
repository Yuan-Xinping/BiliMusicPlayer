#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QElapsedTimer>
#include "../service/ConcurrentDownloadManager.h"
#include "../common/AppConfig.h"
#include "../data/DatabaseManager.h"

class ConcurrentDownloadTester : public QObject {
    Q_OBJECT

public:
    explicit ConcurrentDownloadTester(QObject* parent = nullptr) : QObject(parent), m_completedTasks(0) {}

    void testConcurrentDownload() {
        qDebug() << "\n=== 测试并发下载管理器 ===";

        if (!AppConfig::instance().isYtDlpAvailable()) {
            qDebug() << "⚠️ yt-dlp 不可用，跳过测试";
            QCoreApplication::quit();
            return;
        }

        m_downloadManager = new ConcurrentDownloadManager(this);

        // 设置配置
        ConcurrentDownloadConfig config = ConcurrentDownloadConfig::getDefault();
        config.maxConcurrentDownloads = 3; // 同时下载3个任务
        config.maxRetryCount = 1;          // 最多重试1次
        config.taskTimeoutMs = 120000;     // 2分钟超时

        m_downloadManager->setConfig(config);

        // 连接所有信号
        connectSignals();

        // 添加测试任务
        qDebug() << "\n添加并发下载任务...";

        QStringList testIdentifiers = {
            "BV1mK4y1C7Bz",     // 音乐相关视频1
            "BV15Y411q7RY",     // 音乐相关视频2  
            "BV1BX4y1g7rX",     // 音乐相关视频3
            "BV1aL4y1h7JF",     // 音乐相关视频4
            "BV1Tb411f7cL"      // 音乐相关视频5
        };

        // 使用不同的预设测试
        QStringList presets = { "high_quality_mp3", "medium_quality_mp3", "small_size_opus" };

        for (int i = 0; i < testIdentifiers.size(); ++i) {
            QString identifier = testIdentifiers[i];
            QString preset = presets[i % presets.size()];
            DownloadOptions options = DownloadOptions::createPreset(preset);

            QString taskId = m_downloadManager->addTask(identifier, options);
            if (!taskId.isEmpty()) {
                m_expectedTaskIds.append(taskId);
                qDebug() << QString("  [%1] %2 (预设: %3)").arg(i + 1).arg(identifier).arg(preset);
            }
        }

        qDebug() << "\n预期任务数:" << m_expectedTaskIds.size();
        qDebug() << "最大并发数:" << config.maxConcurrentDownloads;

        m_startTime.start();

        // 显示初始统计
        showStatistics();

        // 启动定期统计显示
        m_statisticsTimer = new QTimer(this);
        connect(m_statisticsTimer, &QTimer::timeout, this, &ConcurrentDownloadTester::showStatistics);
        m_statisticsTimer->start(5000); // 每5秒显示一次统计
    }

private slots:
    void onTaskAdded(const QString& taskId, const DownloadTaskState& task) {
        qDebug() << "✓ 任务已添加:" << task.getIdentifier() << "(" << taskId.left(8) << "...)";
    }

    void onTaskStarted(const QString& taskId) {
        DownloadTaskState task = m_downloadManager->getTask(taskId);
        qDebug() << "▶ 开始下载:" << task.getIdentifier() << "(" << taskId.left(8) << "...)";
    }

    void onTaskProgress(const QString& taskId, double progress, const QString& message) {
        DownloadTaskState task = m_downloadManager->getTask(taskId);
        qDebug() << QString("📥 [%1] %2% - %3")
            .arg(task.getIdentifier())
            .arg(progress * 100, 0, 'f', 1)
            .arg(message);
    }

    void onTaskCompleted(const QString& taskId, const Song& song) {
        m_completedTasks++;
        DownloadTaskState task = m_downloadManager->getTask(taskId);

        qDebug() << "✅ 下载完成:" << task.getIdentifier();
        qDebug() << "   标题:" << song.getTitle();
        qDebug() << "   艺术家:" << song.getArtist();
        qDebug() << "   文件:" << song.getLocalFilePath();
        qDebug() << "   耗时:" << task.getElapsedMs() << "毫秒";
        qDebug() << "   完成进度:" << m_completedTasks << "/" << m_expectedTaskIds.size();

        checkCompletion();
    }

    void onTaskFailed(const QString& taskId, const QString& error) {
        DownloadTaskState task = m_downloadManager->getTask(taskId);

        qDebug() << "❌ 下载失败:" << task.getIdentifier();
        qDebug() << "   错误:" << error;
        qDebug() << "   重试次数:" << task.getRetryCount();

        checkCompletion();
    }

    void onTaskRetrying(const QString& taskId, int retryCount) {
        DownloadTaskState task = m_downloadManager->getTask(taskId);
        qDebug() << "🔄 任务重试:" << task.getIdentifier() << "，第" << retryCount << "次重试";
    }

    void onTaskCancelled(const QString& taskId) {
        DownloadTaskState task = m_downloadManager->getTask(taskId);
        qDebug() << "⏹ 任务已取消:" << task.getIdentifier();
    }

    void onAllTasksCompleted() {
        qDebug() << "\n🎉 所有并发下载任务完成！";
        showFinalStatistics();

        m_statisticsTimer->stop();
        QTimer::singleShot(2000, this, []() { QCoreApplication::quit(); });
    }

    void onStatisticsUpdated(const ConcurrentDownloadManager::Statistics& stats) {
        // 这里可以处理实时统计更新，目前我们使用定时显示
    }

    void showStatistics() {
        auto stats = m_downloadManager->getStatistics();

        qDebug() << "\n📊 当前统计:";
        qDebug() << "  总任务数:" << stats.totalTasks;
        qDebug() << "  运行中:" << stats.activeTasks;
        qDebug() << "  等待中:" << stats.pendingTasks;
        qDebug() << "  已完成:" << stats.completedTasks;
        qDebug() << "  失败:" << stats.failedTasks;
        qDebug() << "  总进度:" << QString::number(stats.overallProgress * 100, 'f', 1) + "%";

        if (stats.completedTasks > 0) {
            qDebug() << "  平均下载时间:" << stats.averageDownloadTimeMs << "毫秒";
            qDebug() << "  总下载时间:" << stats.totalDownloadTimeMs << "毫秒";
        }

        if (m_startTime.isValid()) {
            qDebug() << "  总耗时:" << m_startTime.elapsed() << "毫秒";
        }
    }

private:
    void connectSignals() {
        connect(m_downloadManager, &ConcurrentDownloadManager::taskAdded,
            this, &ConcurrentDownloadTester::onTaskAdded);
        connect(m_downloadManager, &ConcurrentDownloadManager::taskStarted,
            this, &ConcurrentDownloadTester::onTaskStarted);
        connect(m_downloadManager, &ConcurrentDownloadManager::taskProgress,
            this, &ConcurrentDownloadTester::onTaskProgress);
        connect(m_downloadManager, &ConcurrentDownloadManager::taskCompleted,
            this, &ConcurrentDownloadTester::onTaskCompleted);
        connect(m_downloadManager, &ConcurrentDownloadManager::taskFailed,
            this, &ConcurrentDownloadTester::onTaskFailed);
        connect(m_downloadManager, &ConcurrentDownloadManager::taskRetrying,
            this, &ConcurrentDownloadTester::onTaskRetrying);
        connect(m_downloadManager, &ConcurrentDownloadManager::taskCancelled,
            this, &ConcurrentDownloadTester::onTaskCancelled);
        connect(m_downloadManager, &ConcurrentDownloadManager::allTasksCompleted,
            this, &ConcurrentDownloadTester::onAllTasksCompleted);
        connect(m_downloadManager, &ConcurrentDownloadManager::statisticsUpdated,
            this, &ConcurrentDownloadTester::onStatisticsUpdated);
    }

    void checkCompletion() {
        auto stats = m_downloadManager->getStatistics();
        int finishedTasks = stats.completedTasks + stats.failedTasks;

        if (finishedTasks >= m_expectedTaskIds.size()) {
            if (stats.activeTasks == 0 && stats.pendingTasks == 0) {
                onAllTasksCompleted();
            }
        }
    }

    void showFinalStatistics() {
        auto stats = m_downloadManager->getStatistics();

        qDebug() << "\n📈 最终统计报告:";
        qDebug() << "==========================================";
        qDebug() << "  总任务数:" << stats.totalTasks;
        qDebug() << "  成功完成:" << stats.completedTasks;
        qDebug() << "  失败:" << stats.failedTasks;
        qDebug() << "  成功率:" << QString::number((double)stats.completedTasks / stats.totalTasks * 100, 'f', 1) + "%";
        qDebug() << "  平均下载时间:" << stats.averageDownloadTimeMs << "毫秒";
        qDebug() << "  总下载时间:" << stats.totalDownloadTimeMs << "毫秒";
        qDebug() << "  总测试时间:" << m_startTime.elapsed() << "毫秒";

        if (stats.totalTasks > 0) {
            double efficiency = (double)stats.totalDownloadTimeMs / m_startTime.elapsed();
            qDebug() << "  并发效率:" << QString::number(efficiency * 100, 'f', 1) + "%";
        }

        qDebug() << "==========================================";
    }

    ConcurrentDownloadManager* m_downloadManager;
    QStringList m_expectedTaskIds;
    int m_completedTasks;
    QElapsedTimer m_startTime;
    QTimer* m_statisticsTimer;
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // 初始化
    AppConfig::instance().loadConfig();
    DatabaseManager::instance().initialize();

    ConcurrentDownloadTester tester;
    QTimer::singleShot(100, &tester, &ConcurrentDownloadTester::testConcurrentDownload);

    return app.exec();
}

#include "test_concurrent_download.moc"

        

