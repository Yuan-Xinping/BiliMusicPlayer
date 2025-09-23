#pragma once
#include <QObject>

class LibraryViewModel : public QObject {
    Q_OBJECT
public:
    explicit LibraryViewModel(QObject* parent = nullptr);
    static LibraryViewModel& instance();
};
