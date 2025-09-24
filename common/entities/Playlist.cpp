#include "Playlist.h"

Playlist::Playlist(const QString& id, const QString& name, const QString& description)
    : m_id(id), m_name(name), m_description(description)
{
}

Playlist::Playlist(const QString& name, const QString& description)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_name(name), m_description(description)
{
}

bool Playlist::operator==(const Playlist& other) const {
    return m_id == other.m_id;
}
