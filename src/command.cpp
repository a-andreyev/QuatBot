/*
 *  SPDX-License-Identifier: BSD-2-Clause
 *  SPDX-License-File: LICENSE
 *
 * Copyright 2019 Adriaan de Groot <groot@kde.org>
 */

#include "command.h"

#include "quatbot.h"

#include <room.h>

#include <QProcess>
#include <QTimer>

namespace QuatBot
{
static QString runProcess(const QString& executable, const QStringList& args, const QString& failure)
{
    QProcess f;
    f.start(executable, args);
    f.waitForFinished();
    if (f.exitCode()==0)
    {
        return QString::fromLatin1(f.readAllStandardOutput());
    }
    else
        return failure;
}

static QString fortune()
{
    return runProcess(QStringLiteral("/usr/bin/fortune"), {"freebsd-tips"}, QStringLiteral("No fortune for you!"));
}

#ifdef ENABLE_COWSAY
// Copy because we modify the string
static QString cowsay(QString message)
{
    message = message.simplified();
    message.truncate(40);
    if (message.isEmpty())
        return QStringLiteral("ix-nay on the oo-may");

    static int instance = 0;
    static const char* const specials[16]={
        nullptr, nullptr, "-d", nullptr,
        nullptr, nullptr, "-s", "-p",
        nullptr, "-y", nullptr, "-g",
        "-w", "-t", "-b", nullptr};

    QStringList arg;
    // Go around and around mod 16
    if (specials[instance])
        arg << specials[instance];
    instance = (instance+1) & 0xf;
    // The message
    arg << message;

    return runProcess(QStringLiteral("/usr/local/bin/cowsay"), arg, "Moo!");
}
#endif

BasicCommands::BasicCommands(Bot* parent) :
    Watcher(parent)
{
}

BasicCommands::~BasicCommands()
{
}

const QString& BasicCommands::moduleName() const
{
    static const QString name(QStringLiteral("quatbot"));
    return name;
}

const QStringList& BasicCommands::moduleCommands() const
{
    static const QStringList commands{"echo","fortune",
#ifdef ENABLE_COWSAY
        "cowsay",
#endif
        "ops",
        "help", "status","quit"};
    return commands;
}


static QString munge(const QTime& t)
{
    return t.toString();
}

void BasicCommands::handleCommand(const CommandArgs& l)
{
    if (l.command == QStringLiteral("echo"))
    {
        message(l.args);
    }
    else if (l.command == QStringLiteral("fortune"))
    {
        message(fortune());
    }
#ifdef ENABLE_COWSAY
    else if (l.command == QStringLiteral("cowsay"))
    {
        message(cowsay(l.args.join(' ')));
    }
#endif
    else if (l.command == QStringLiteral("ops"))
    {
        if (l.args.count() < 1)
        {
            message(OpsUsage{});
        }
        else if ((l.args[0] == "?") || (l.args[0] == "status"))
        {
            message(QStringList{QString("There are %1 operators.").arg(m_bot->m_operators.count())} << m_bot->m_operators.values());
        }
        else if ((l.args[0] == "+") || (l.args[0] == "add") || (l.args[0] == "op"))
        {
            opsChange(l, true);
        }
        else if ((l.args[0] == "-") || (l.args[0] == "remove") || (l.args[0] == "deop"))
        {
            opsChange(l, false);
        }
        else
        {
            message(OpsUsage{});
        }
    }
    else if (l.command == QStringLiteral("quit"))
    {
        if (m_bot->checkOps(l))
        {
            QTimer::singleShot(1000, m_bot, &QObject::deleteLater);
            message(QString("Goodbye (bot operation terminated)!"));
        }
    }
    else if (l.command == QStringLiteral("status"))
    {
        message(QString(
            "(quatbot) It is %1. Your message was sent at %2. (Time UTC) "
            "I can see %3 people in the room. I have processed %4 messages and %5 commands.")
            .arg(QDateTime::currentDateTimeUtc().time().toString(), m_lastMessageTime.toString())
            .arg(m_bot->userIds().count())
            .arg(m_messageCount)
            .arg(m_commandCount)
        );
        for (const auto& w : m_bot->watcherNames())
        {
            auto* watcher = m_bot->getWatcher(w);
            if (watcher && (watcher->moduleName() != moduleName()) && watcher->moduleCommands().contains("status"))
            {
                watcher->handleCommand(l);
            }
        }
    }
    else if (l.command == QStringLiteral("help"))
    {
        if (l.args.isEmpty())
        {
            message(QStringList{"The following modules are available:"} << m_bot->watcherNames());
            message("Use help <modulename..> to see what commands are available.");
        }
        else
        {
            for (const auto& w : l.args)
            {
                auto* watcher = m_bot->getWatcher(w);
                if (watcher)
                {
                    message(QStringList{QString("Module %1 understands:").arg(watcher->moduleName())} << watcher->moduleCommands());
                }
            }
        }
    }
    else
    {
        message(Usage{});
    }
    m_commandCount++;
}

void QuatBot::BasicCommands::handleMessage(const Quotient::RoomMessageEvent* event)
{
    m_lastMessageTime = event->originTimestamp().time();
    m_messageCount++;
}

void BasicCommands::opsChange(const CommandArgs& cmd, bool enable)
{
    if (m_bot->checkOps(cmd))
    {
        if (cmd.args.count() > 1)
        {
            CommandArgs a(cmd);
            a.pop();

            for (const auto& user : m_bot->userLookup(a.args))
            {
                if (user.isEmpty())
                {
                    message(QString("Unrecognized when changing operators.").arg(user));
                    continue;
                }
                if (m_bot->setOps(user, enable))
                {
                    if (!enable)
                    {
                        // Successfully disabled ops mode
                        message(QString("%1 is no longer an operator").arg(user));
                    }
                    else
                    {
                        message(QString("%1 is now an operator").arg(user));
                    }

                }
                else
                {
                    message(QString("Changing operator status of %1 failed.").arg(user));
                }
            }
        }
        else
        {
            message(OpsUsage{});
        }
    }
}

void BasicCommands::message(OpsUsage)
{
    message(QString("Usage: %1 ops status").arg(displayCommand()));
    message(QString("Usage: %1 ops <add|op|+|remove|deop|-> <name..>").arg(displayCommand()));
}

}
