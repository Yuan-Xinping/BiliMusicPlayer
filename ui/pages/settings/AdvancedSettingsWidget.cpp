#include "AdvancedSettingsWidget.h"
#include "../../../common/AppConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkProxy>       
#include <QNetworkRequest>       
#include <QNetworkReply>       
#include <QTimer>          
#include <QUrl>     
#include <QDebug> 

AdvancedSettingsWidget::AdvancedSettingsWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();
}

void AdvancedSettingsWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    QGroupBox* advancedGroup = new QGroupBox("🔬 高级设置");
    advancedGroup->setObjectName("settingsGroup");
    QVBoxLayout* advancedLayout = new QVBoxLayout(advancedGroup);
    advancedLayout->setSpacing(12);

    // 代理设置
    m_proxyEnabledCheck = new QCheckBox("启用代理");
    m_proxyEnabledCheck->setObjectName("settingsCheckbox");

    QHBoxLayout* proxyLayout = new QHBoxLayout();
    QLabel* proxyLabel = new QLabel("代理地址:");
    proxyLabel->setFixedWidth(100);
    proxyLabel->setObjectName("settingsLabel");

    m_proxyUrlInput = new QLineEdit();
    m_proxyUrlInput->setObjectName("settingsInput");
    m_proxyUrlInput->setPlaceholderText("例如：http://127.0.0.1:7890");
    m_proxyUrlInput->setEnabled(false);

    m_testProxyBtn = new QPushButton("🔍 测试");
    m_testProxyBtn->setObjectName("testBtn");
    m_testProxyBtn->setFixedWidth(80);
    m_testProxyBtn->setEnabled(false);

    proxyLayout->addWidget(proxyLabel);
    proxyLayout->addWidget(m_proxyUrlInput);
    proxyLayout->addWidget(m_testProxyBtn);

    // 说明文本
    QLabel* infoLabel = new QLabel(
        "💡 提示：\n"
        "• 启用代理后，下载将通过代理服务器进行\n"
        "• 代理地址格式：http://ip:port 或 socks5://ip:port\n"
        "• 如果代理设置错误可能导致下载失败"
    );
    infoLabel->setObjectName("infoLabel");
    infoLabel->setWordWrap(true);

    advancedLayout->addWidget(m_proxyEnabledCheck);
    advancedLayout->addLayout(proxyLayout);
    advancedLayout->addSpacing(10);
    advancedLayout->addWidget(infoLabel);

    mainLayout->addWidget(advancedGroup);
    mainLayout->addStretch();

    connect(m_proxyEnabledCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_proxyUrlInput->setEnabled(checked);
        m_testProxyBtn->setEnabled(checked);
        });

    connect(m_testProxyBtn, &QPushButton::clicked,
        this, &AdvancedSettingsWidget::onTestProxyClicked);
}

void AdvancedSettingsWidget::setupStyles()
{
    
}

void AdvancedSettingsWidget::loadSettings()
{
    AppConfig& config = AppConfig::instance();

    m_proxyEnabledCheck->setChecked(config.getProxyEnabled());
    m_proxyUrlInput->setText(config.getProxyUrl());
}

bool AdvancedSettingsWidget::validate()
{
    if (m_proxyEnabledCheck->isChecked()) {
        QString proxyUrl = m_proxyUrlInput->text().trimmed();
        if (proxyUrl.isEmpty()) {
            QMessageBox::warning(this, "输入错误", "代理已启用但未填写地址！");
            return false;
        }
    }
    return true;
}

void AdvancedSettingsWidget::applySettings()
{
    AppConfig& config = AppConfig::instance();

    config.setProxyEnabled(m_proxyEnabledCheck->isChecked());
    config.setProxyUrl(m_proxyUrlInput->text());
}

void AdvancedSettingsWidget::onTestProxyClicked()
{
    if (!m_proxyEnabledCheck->isChecked()) {
        QMessageBox::information(this, "提示", "请先启用代理！");
        return;
    }

    QString proxyUrl = m_proxyUrlInput->text().trimmed();
    if (proxyUrl.isEmpty()) {
        QMessageBox::warning(this, "错误", "代理地址不能为空！");
        return;
    }

    // 解析代理URL
    QUrl url(proxyUrl);
    if (!url.isValid()) {
        QMessageBox::warning(this, "错误", "代理地址格式错误！\n示例：http://127.0.0.1:7890");
        return;
    }

    // 设置代理
    QNetworkProxy proxy;
    QString scheme = url.scheme().toLower();

    if (scheme == "http" || scheme == "https") {
        proxy.setType(QNetworkProxy::HttpProxy);
    }
    else if (scheme == "socks5") {
        proxy.setType(QNetworkProxy::Socks5Proxy);
    }
    else {
        QMessageBox::warning(this, "错误",
            "不支持的代理类型！\n仅支持：http、https、socks5");
        return;
    }

    proxy.setHostName(url.host());
    proxy.setPort(url.port(1080));

    qDebug() << "测试代理:" << url.host() << ":" << url.port(1080) << "类型:" << scheme;

    // 禁用测试按钮，显示测试中
    m_testProxyBtn->setEnabled(false);
    m_testProxyBtn->setText("🔄 测试中...");

    // 测试代理连接
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    manager->setProxy(proxy);

    // 使用多个测试URL（以防某个被墙）
    QStringList testUrls = {
        "https://www.google.com",
        "https://www.youtube.com",
        "https://httpbin.org/ip"
    };

    QNetworkRequest request(QUrl(testUrls.first()));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply* reply = manager->get(request);

    // 设置超时
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, reply, timer]() {
        if (reply->isRunning()) {
            reply->abort();
            m_testProxyBtn->setEnabled(true);
            m_testProxyBtn->setText("🔍 测试");
            QMessageBox::warning(this, "超时", "❌ 代理连接超时（5秒）！\n请检查代理地址和端口是否正确。");
        }
        timer->deleteLater();
        });
    timer->start(5000);  // 5秒超时

    // 处理响应
    connect(reply, &QNetworkReply::finished, this,
        [this, reply, timer, manager]() {
            timer->stop();
            timer->deleteLater();

            m_testProxyBtn->setEnabled(true);
            m_testProxyBtn->setText("🔍 测试");

            if (reply->error() == QNetworkReply::NoError) {
                int statusCode = reply->attribute(
                    QNetworkRequest::HttpStatusCodeAttribute).toInt();

                qDebug() << "✅ 代理测试成功，HTTP状态码:" << statusCode;

                QMessageBox::information(this, "成功",
                    QString("✅ 代理连接成功！\n\nHTTP 状态码：%1\n代理工作正常。")
                    .arg(statusCode));
            }
            else {
                QString errorMsg = reply->errorString();
                qWarning() << "❌ 代理测试失败:" << errorMsg;

                QMessageBox::warning(this, "失败",
                    QString("❌ 代理连接失败！\n\n错误信息：\n%1\n\n"
                        "可能的原因：\n"
                        "• 代理服务器未运行\n"
                        "• 地址或端口错误\n"
                        "• 代理需要认证但未提供")
                    .arg(errorMsg));
            }

            reply->deleteLater();
            manager->deleteLater();
        });
}