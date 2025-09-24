#pragma once
#include <QObject>
#include <QString>

struct ConcurrentDownloadConfig {
    int maxConcurrentDownloads = 3;     // 最大并发下载数
    int maxRetryCount = 2;              // 最大重试次数
    int retryDelayMs = 1000;            // 重试延迟（毫秒）
    int taskTimeoutMs = 300000;         // 单个任务超时时间（5分钟）
    bool enableAutoRetry = true;        // 启用自动重试
    bool enableDownloadSpeedLimit = false; // 启用速度限制
    QString speedLimitPerTask = "500K";  // 每个任务的速度限制

    // 验证配置的有效性
    bool isValid() const {
        return maxConcurrentDownloads > 0 &&
            maxConcurrentDownloads <= 10 &&  // 限制最大并发数
            maxRetryCount >= 0 &&
            retryDelayMs > 0 &&
            taskTimeoutMs > 0;
    }

    // 获取默认配置
    static ConcurrentDownloadConfig getDefault() {
        return ConcurrentDownloadConfig();
    }

    // 获取保守配置（适用于网络较慢的情况）
    static ConcurrentDownloadConfig getConservative() {
        ConcurrentDownloadConfig config;
        config.maxConcurrentDownloads = 2;
        config.maxRetryCount = 3;
        config.retryDelayMs = 2000;
        config.enableDownloadSpeedLimit = true;
        config.speedLimitPerTask = "300K";
        return config;
    }

    // 获取激进配置（适用于网络较好的情况）
    static ConcurrentDownloadConfig getAggressive() {
        ConcurrentDownloadConfig config;
        config.maxConcurrentDownloads = 5;
        config.maxRetryCount = 1;
        config.retryDelayMs = 500;
        config.taskTimeoutMs = 180000; // 3分钟
        return config;
    }
};
