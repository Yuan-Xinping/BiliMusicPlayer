#include "ServiceRegistry.h"

ServiceRegistry::ServiceRegistry(QObject* parent) : QObject(parent) {
}

ServiceRegistry& ServiceRegistry::instance() {
    static ServiceRegistry instance;
    return instance;
}

void ServiceRegistry::registerServices() {
    // TODO: �ں����׶�ʵ�ַ���ע��
}
