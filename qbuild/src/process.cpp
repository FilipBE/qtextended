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

#include "process.h"
#include "gui.h"
#include "qbuild.h"
#include "qoutput.h"
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pty.h>
#include <QMutex>
#include <sys/stat.h>
#include <fcntl.h>

/*!
  \class ShellProcess
  \brief The ShellProcess class runs a process.

  The ShellProcess class runs a process via the shell.
*/

/*!
  \enum ShellProcess::ProcessType
  This enum specifies the type of communications to use for stdout/stderr.
  \value Normal Use pipes for stdout/stderr.
  \value TTy Use a virtual TTY for stdout/stderr. Note that if the current process'
         stdout or stderr is not a TTY a pipe will be used for that stream instead.
*/

/*!
  Construct a shell process that will run \a command in \a cwd.
  The \a type value specifies if pipes or a TTY should be used.
  The \a echoCommandBeforeOutput value indicates that the command
  should be echoed if any output is produced.
  The command is run immediately and synchronously.
*/
ShellProcess::ShellProcess(const QString &command,
                           const QString &cwd,
                           ProcessType type,
                           bool echoCommandBeforeOutput)
: m_exitCode(-1)
{
    pid_t pid;
    int err_fds[2] = { -1, -1 };
    int out_fds[2] = { -1, -1 };

    if (type != TTy || !isatty(STDERR_FILENO) || !openTTy(err_fds)) {
        if (::pipe(err_fds)) {
            m_output = ::strerror(errno);
            goto cleanup;
        }
    }

    if (type != TTy || !isatty(STDOUT_FILENO) || !openTTy(out_fds)) {
        if (::pipe(out_fds)) {
            m_output = ::strerror(errno);
            goto cleanup;
        }
    }

    if (-1 == (pid = ::fork())) {
        m_output = ::strerror(errno);
        goto cleanup;
    }

    if (0 == pid) {
        ::close(err_fds[0]);
        ::close(out_fds[0]);

        if (!cwd.isEmpty())
            ::chdir(cwd.toAscii().constData());

        runCommand(out_fds[1], err_fds[1], command);
        Q_ASSERT(!"runCommand should not return");
    } else {
        ::close(err_fds[1]);
        ::close(out_fds[1]);
        out_fds[1] = -1;
        err_fds[1] = -1;

        int out_fd = out_fds[0];
        int err_fd = err_fds[0];

        char buffer[1024];

        while (out_fd != -1 || err_fd != -1) {

            fd_set set;
            FD_ZERO(&set);

            int max_fd = -1;
            if (out_fd != -1) {
                FD_SET(out_fd, &set);
                if (out_fd > max_fd) max_fd = out_fd;
            }
            if (err_fd != -1) {
                FD_SET(err_fd, &set);
                if (err_fd > max_fd) max_fd = err_fd;
            }

            int select_rv = ::select(max_fd + 1, &set, 0, 0, 0);
            if (-1 == select_rv)
                continue;

            if (out_fd != -1 && FD_ISSET(out_fd, &set)) {

                int readrv = ::read(out_fd, buffer, 1024);
                if (0 == readrv) {
                    out_fd = -1;
                } else if (-1 == readrv) {
                    if (EINTR == errno)
                        continue;
                    else
                        out_fd = -1;
                } else {
                    if (echoCommandBeforeOutput) {
                        echoCommandBeforeOutput = false;
                        GuiBase::write(fileno(stdout),
                                command.toLatin1().constData(),
                                command.length());
                        GuiBase::write(fileno(stdout), "\n", 1);
                    }

                    m_output.append(QByteArray(buffer, readrv));
                    GuiBase::write(fileno(stdout), buffer, readrv);
                }

            } else if (err_fd != -1 && FD_ISSET(err_fd, &set)) {
                int readrv = ::read(err_fd, buffer, 1024);
                if (0 == readrv) {
                    err_fd = -1;
                } else if (-1 == readrv) {
                    if (EINTR == errno)
                        continue;
                    else
                        err_fd = -1;
                } else {
                    if (echoCommandBeforeOutput) {
                        echoCommandBeforeOutput = false;
                        GuiBase::write(fileno(stdout),
                                command.toLatin1().constData(),
                                command.length());
                        GuiBase::write(fileno(stdout), "\n", 1);
                    }

                    m_output.append(QByteArray(buffer, readrv));
                    GuiBase::write(fileno(stderr), buffer, readrv);
                }

            }

        }

        int status;
        while (true) {
            if (-1 != ::waitpid(pid, &status, 0)) {
                m_exitCode = WEXITSTATUS(status);
                break;
            } else if (errno == EINTR) {
            } else {
                m_output = ::strerror(errno);
                break;
            }
        }
    }


cleanup:
    if (-1 != err_fds[0]) ::close(err_fds[0]);
    if (-1 != err_fds[1]) ::close(err_fds[1]);
    if (-1 != out_fds[0]) ::close(out_fds[0]);
    if (-1 != out_fds[1]) ::close(out_fds[1]);
}

/*!
  Returns the exit code of the shell.
*/
int ShellProcess::exitCode() const
{
    return m_exitCode;
}

/*!
  Returns the output from the process.
*/
QByteArray ShellProcess::output() const
{
    return m_output;
}

/*!
  \internal
*/
void ShellProcess::runCommand(int out_fd, int err_fd, const QString &command)
{
    char *shell = "/bin/sh";
    char *argv[4];
    char *commandStr = 0;

    if (-1 == ::dup2(err_fd, ::fileno(stderr)))
        goto fail;
    if (-1 == ::dup2(out_fd, ::fileno(stdout)))
        goto fail;

    commandStr = new char[command.length() + 1];
    ::memcpy(commandStr, command.toAscii().constData(), command.length() + 1);
    argv[0] = shell;
    argv[1] = "-c";
    argv[2] = commandStr;
    argv[3] = 0;

    if (-1 == ::execve(shell, argv, environ)) {
        goto fail;
    }

    Q_ASSERT(!"execve should not return");

fail:
    exit(-1);
}

struct TTyTracking {
    QSet<ushort> used;
    QMutex lock;
};
Q_GLOBAL_STATIC(TTyTracking, ttyTracking)

/*!
  \internal
*/
bool ShellProcess::openTTy(int *fds)
{
    TTyTracking *tracking = ttyTracking();
    if ( !tracking )
        return false;
    LOCK(Process);
    tracking->lock.lock();

    int ptyfd = -1;
    int ttyfd = -1;
    for (const char* c0 = "pqrstuvwxyzabcde"; ptyfd == -1 && *c0 != 0; c0++) {
        for (const char* c1 = "0123456789abcdef"; ptyfd == -1 && *c1 != 0; c1++) {

            ushort s = *c0 << 8 | *c1;
            if (tracking->used.contains(s))
                continue;

            char name[11];
            sprintf(name, "/dev/pty%c%c", *c0, *c1);

            ptyfd = ::open(name, O_RDONLY);
            if (ptyfd == -1)
                continue;

            if (ptyfd != -1) {
                sprintf(name, "/dev/tty%c%c", *c0, *c1);
                ttyfd = ::open(name, O_WRONLY);
            }

            if (ttyfd == -1) {
                ::close(ptyfd);
                ptyfd = -1;
            }
        }
    }

    Q_ASSERT((ptyfd == -1 && ttyfd == -1) || (ptyfd != -1 && ttyfd != -1));

    bool ret;
    if (ptyfd == -1) {
        ret = false;
    } else {
        fds[0] = ptyfd;
        fds[1] = ttyfd;
        ret = true;
    }
    tracking->lock.unlock();
    return ret;
}

