/*
 *  SPDX-License-Identifier: BSD-2-Clause
 *  SPDX-License-File: LICENSE
 *
 * Copyright 2019 Adriaan de Groot <groot@kde.org>
 */   

#ifndef QUATBOT_WATCHER_H
#define QUATBOT_WATCHER_H

#include <QString>
#include <QStringList>

namespace QMatrixClient
{
    class Room;
    class RoomMessageEvent;
}

namespace QuatBot
{
class Bot;

struct CommandArgs
{
    explicit CommandArgs(QString);   // Copied because it's modified in the method
    explicit CommandArgs(const QMatrixClient::RoomMessageEvent*);
    
    static bool isCommand(const QString& s);
    static bool isCommand(const QMatrixClient::RoomMessageEvent*);
    
    bool isValid() const { return !command.isEmpty(); }
    
    QString id;  // event Id, if available
    QString command;
    QStringList args;
};
    
class Watcher
{
public:
    explicit Watcher(Bot* parent);
    virtual ~Watcher();
    
    virtual void handleMessage(QMatrixClient::Room*, const QMatrixClient::RoomMessageEvent*) = 0;
    virtual void handleCommand(QMatrixClient::Room*, const CommandArgs&) = 0;

    // Duplicated for convenience
    static bool isCommand(const QString& s) { return CommandArgs::isCommand(s); }
    static bool isCommand(const QMatrixClient::RoomMessageEvent* e) { return CommandArgs::isCommand(e); }
    
    static void message(QMatrixClient::Room* room, const QStringList& l);
    static void message(QMatrixClient::Room* room, const QString& s);
    
protected:
    /// @brief human-readable version of the-command-for @p s with command-prefix
    QString displayCommand(const QString& s);
    
    Bot* m_bot;
};
    
}
#endif
