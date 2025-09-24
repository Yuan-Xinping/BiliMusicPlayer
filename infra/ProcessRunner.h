#pragma once
#include <QObject>
#include <QProcess>
#include <QStringList>
#include <functional>

class ProcessRunner : public QObject {
    Q_OBJECT

public:
    // 进度回调函数类型
    using ProgressCallback = std::function<void(double progress, const QString& message)>;
    using OutputCallback = std::function<void(const QString& line)>;

    explicit ProcessRunner(QObject* parent = nullptr);
    ~ProcessRunner();

    // 启动进程
    bool start(const QString& program, const QStringList& arguments);

    // 设置回调函数
    void setProgressCallback(const ProgressCallback& callback);
    void setOutputCallback(const OutputCallback& callback);

    // 进程控制
    void terminate();
    bool isRunning() const;
    int exitCode() const;

    // 等待完成
    bool waitForFinished(int msecs = 30000);

signals:
    void finished(int exitCode);
    void error(const QString& errorString);
    void outputReady(const QString& line);

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    QProcess* m_process;
    ProgressCallback m_progressCallback;
    OutputCallback m_outputCallback;
    QString m_outputBuffer;
    QString m_errorBuffer;

    void processOutputLine(const QString& line);
};
