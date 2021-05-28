/*
 *  SPDX-License-Identifier: BSD-2-Clause
 *  SPDX-License-File: LICENSE
 *
 * Copyright 2019 Adriaan de Groot <groot@kde.org>
 */   

#ifndef QUATBOT_LOGGER_H
#define QUATBOT_LOGGER_H

#include "watcher.h"

#include <QString>
#include <QStringList>

namespace QuatBot
{
    
class Logger : public Watcher
{
    class Private;

public:
    Logger(Bot* parent);
    virtual ~Logger() override;

    const QString& moduleName() const override;
    const QStringList& moduleCommands() const override;
    
    virtual void handleMessage(const QString&) override;
    virtual void handleMessage(const Quotient::RoomMessageEvent*) override;
    virtual void handleCommand(const CommandArgs&) override;

private:
    Private* d;
};
    
}  // namespace
#endif
