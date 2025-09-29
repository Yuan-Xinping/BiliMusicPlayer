#pragma once
#include <QObject>

class ServiceRegistry : public QObject {
    Q_OBJECT
public:
    explicit ServiceRegistry(QObject* parent = nullptr);

    static ServiceRegistry& instance();

    void registerServices();
};
