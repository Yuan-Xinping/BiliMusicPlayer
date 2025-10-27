#include "ProcessRunner.h"
#include <QDebug>
#include <QRegularExpression>

ProcessRunner::ProcessRunner(QObject* parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
    connect(m_process, &QProcess::readyReadStandardOutput,
        this, &ProcessRunner::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError,
        this, &ProcessRunner::onReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &ProcessRunner::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred,
        this, &ProcessRunner::onProcessError);

#ifdef Q_OS_WIN
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    m_process->setProcessEnvironment(env);
#endif
}

ProcessRunner::~ProcessRunner() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        qDebug() << "ProcessRunner: 析构时检测到进程仍在运行，正在清理...";

        disconnect(m_process, nullptr, this, nullptr);

        m_process->terminate();

        if (!m_process->waitForFinished(3000)) {
            qWarning() << "ProcessRunner: 进程未在3秒内终止，强制杀死";
            m_process->kill();
            m_process->waitForFinished(1000);
        }

        qDebug() << "ProcessRunner: 进程已清理";
    }
}

bool ProcessRunner::start(const QString& program, const QStringList& arguments) {
    if (m_process->state() != QProcess::NotRunning) {
        qWarning() << "ProcessRunner: 进程已在运行中";
        return false;
    }

    qDebug() << "ProcessRunner: 启动进程:" << program;
    qDebug() << "ProcessRunner: 参数:" << arguments;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    m_process->setProcessEnvironment(env);

    m_process->start(program, arguments);

    if (!m_process->waitForStarted()) {
        qWarning() << "ProcessRunner: 无法启动进程:" << m_process->errorString();
        return false;
    }

    qDebug() << "ProcessRunner: 进程启动成功，PID:" << m_process->processId();
    return true;
}

void ProcessRunner::setProgressCallback(const ProgressCallback& callback) {
    m_progressCallback = callback;
}

void ProcessRunner::setOutputCallback(const OutputCallback& callback) {
    m_outputCallback = callback;
}

void ProcessRunner::terminate() {
    if (m_process && m_process->state() == QProcess::Running) {
        qDebug() << "ProcessRunner: 发送 SIGTERM 信号...";
        m_process->terminate();
    }
    else {
        qDebug() << "ProcessRunner: 进程未运行，无需终止";
    }
}

void ProcessRunner::kill() {
    if (m_process && m_process->state() == QProcess::Running) {
        qDebug() << "ProcessRunner: 强制杀死进程 (SIGKILL)...";
        m_process->kill();
    }
    else {
        qDebug() << "ProcessRunner: 进程未运行，无需杀死";
    }
}

bool ProcessRunner::isRunning() const {
    return m_process && m_process->state() == QProcess::Running;
}

int ProcessRunner::exitCode() const {
    return m_process ? m_process->exitCode() : -1;
}

bool ProcessRunner::waitForFinished(int msecs) {
    if (!m_process) {
        return true;
    }

    if (m_process->state() == QProcess::NotRunning) {
        return true; 
    }

    bool finished = m_process->waitForFinished(msecs);

    if (!finished) {
        qWarning() << "ProcessRunner: waitForFinished 超时 (" << msecs << "ms)";
    }

    return finished;
}

void ProcessRunner::onReadyReadStandardOutput() {
    QByteArray data = m_process->readAllStandardOutput();

    QString text = QString::fromUtf8(data);

    if (text.contains(QChar::ReplacementCharacter)) {
        text = QString::fromLocal8Bit(data);
    }

    m_outputBuffer += text;

    QStringList lines = m_outputBuffer.split(QRegularExpression("[\\r\\n]+"), Qt::SkipEmptyParts);

    if (m_outputBuffer.endsWith('\n') || m_outputBuffer.endsWith('\r')) {
        m_outputBuffer.clear();
    }
    else {
        if (!lines.isEmpty()) {
            m_outputBuffer = lines.takeLast();
        }
    }

    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (!trimmed.isEmpty()) {
            processOutputLine(trimmed);
        }
    }
}

void ProcessRunner::onReadyReadStandardError() {
    QByteArray data = m_process->readAllStandardError();

    QString text = QString::fromUtf8(data);

    if (text.contains(QChar::ReplacementCharacter)) {
        text = QString::fromLocal8Bit(data);
    }

    m_errorBuffer += text;

    QStringList lines = m_errorBuffer.split(QRegularExpression("[\\r\\n]+"), Qt::SkipEmptyParts);

    if (m_errorBuffer.endsWith('\n') || m_errorBuffer.endsWith('\r')) {
        m_errorBuffer.clear();
    }
    else {
        if (!lines.isEmpty()) {
            m_errorBuffer = lines.takeLast();
        }
    }

    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (!trimmed.isEmpty()) {
            qDebug() << "ProcessRunner 错误输出:" << trimmed;
            processOutputLine(trimmed);
        }
    }
}

void ProcessRunner::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    qDebug() << "ProcessRunner: 进程完成，退出码:" << exitCode
        << "退出状态:" << (exitStatus == QProcess::NormalExit ? "正常" : "崩溃");

    if (!m_outputBuffer.trimmed().isEmpty()) {
        processOutputLine(m_outputBuffer.trimmed());
        m_outputBuffer.clear();
    }
    if (!m_errorBuffer.trimmed().isEmpty()) {
        processOutputLine(m_errorBuffer.trimmed());
        m_errorBuffer.clear();
    }

    emit finished(exitCode);
}

void ProcessRunner::onProcessError(QProcess::ProcessError error) {
    QString errorString = m_process->errorString();
    QString errorType;

    switch (error) {
    case QProcess::FailedToStart:
        errorType = "启动失败";
        break;
    case QProcess::Crashed:
        errorType = "进程崩溃";
        break;
    case QProcess::Timedout:
        errorType = "超时";
        break;
    case QProcess::WriteError:
        errorType = "写入错误";
        break;
    case QProcess::ReadError:
        errorType = "读取错误";
        break;
    default:
        errorType = "未知错误";
        break;
    }

    qWarning() << "ProcessRunner 错误 [" << errorType << "]:" << errorString;
    emit this->error(errorString);
}

void ProcessRunner::processOutputLine(const QString& line) {
    qDebug() << "ProcessRunner 输出:" << line;

    static QRegularExpression progressRegex(R"(\[download\]\s+(\d+(?:\.\d+)?)%\s+of)");
    QRegularExpressionMatch match = progressRegex.match(line);

    if (match.hasMatch()) {
        bool ok;
        double progress = match.captured(1).toDouble(&ok);
        if (ok && m_progressCallback) {
            qDebug() << "📊 识别到进度:" << progress << "%";
            m_progressCallback(progress / 100.0, QString("下载中: %1%").arg(progress, 0, 'f', 1));
        }
    }

    if (m_outputCallback) {
        m_outputCallback(line);
    }

    emit outputReady(line);
}
