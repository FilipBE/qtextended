/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef PROCESS_H
#define PROCESS_H

#include <QString>
#include <QByteArray>

class ShellProcess
{
public:
    // Blocks
    enum ProcessType { Normal, TTy };
    ShellProcess(const QString &command,
                 const QString &cwd,
                 ProcessType = Normal,
                 bool echoCommandBeforeOutput = false);

    int exitCode() const;
    QByteArray output() const;

private:
    ShellProcess(const ShellProcess &);
    ShellProcess &operator=(const ShellProcess &);

    void runCommand(int out_fd, int err_fd, const QString &);

    bool openTTy(int *fds);


    int m_exitCode;
    QByteArray m_output;
};

#endif
