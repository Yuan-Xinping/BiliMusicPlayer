#pragma once
#include <QString>
#include <QUuid>

class Playlist {
public:
    Playlist() = default;
    Playlist(const QString& id, const QString& name, const QString& description);
    explicit Playlist(const QString& name, const QString& description = QString());

    // Getters
    QString getId() const { return m_id; }
    QString getName() const { return m_name; }
    QString getDescription() const { return m_description; }

    // Setters
    void setId(const QString& id) { m_id = id; }
    void setName(const QString& name) { m_name = name; }
    void setDescription(const QString& description) { m_description = description; }

    QString toString() const { return m_name; }
    bool operator==(const Playlist& other) const;

private:
    QString m_id;
    QString m_name;
    QString m_description;
};
