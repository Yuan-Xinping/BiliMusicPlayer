// infra/ProcessRunner.cpp
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

    // 设置进程环境，确保 UTF-8 输出
#ifdef Q_OS_WIN
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    m_process->setProcessEnvironment(env);
#endif
}

ProcessRunner::~ProcessRunner() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
}

bool ProcessRunner::start(const QString& program, const QStringList& arguments) {
    if (m_process->state() != QProcess::NotRunning) {
        qWarning() << "ProcessRunner: 进程已在运行中";
        return false;
    }

    qDebug() << "ProcessRunner: 启动进程:" << program;
    qDebug() << "ProcessRunner: 参数:" << arguments;

    // 设置进程工作目录和环境
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    m_process->setProcessEnvironment(env);

    // 直接启动程序
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
    if (m_process) {
        m_process->terminate();
        if (!m_process->waitForFinished(5000)) {
            m_process->kill();
        }
    }
}

bool ProcessRunner::isRunning() const {
    return m_process && m_process->state() == QProcess::Running;
}

int ProcessRunner::exitCode() const {
    return m_process ? m_process->exitCode() : -1;
}

bool ProcessRunner::waitForFinished(int msecs) {
    return m_process ? m_process->waitForFinished(msecs) : false;
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
    qDebug() << "ProcessRunner: 进程完成，退出码:" << exitCode;

    // 处理剩余的缓冲区内容
    if (!m_outputBuffer.trimmed().isEmpty()) {
        processOutputLine(m_outputBuffer.trimmed());
    }
    if (!m_errorBuffer.trimmed().isEmpty()) {
        processOutputLine(m_errorBuffer.trimmed());
    }

    emit finished(exitCode);
}

void ProcessRunner::onProcessError(QProcess::ProcessError error) {
    QString errorString = m_process->errorString();
    qWarning() << "ProcessRunner 错误:" << errorString;
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

    // 调用输出回调
    if (m_outputCallback) {
        m_outputCallback(line);
    }

    emit outputReady(line);
}
