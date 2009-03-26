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

#ifndef GUI_H
#define GUI_H

#include <QWidget>
#include <QMutex>
#include <stdio.h>

class RuleEngine;
class GuiBase
{
public:
    GuiBase();
    virtual ~GuiBase();

    void setRuleEngine(RuleEngine *);
    void setRuleProgress(int completed, int total, bool certain);
    static GuiBase *instance();
    virtual QObject *object() = 0;
    static void write( int fd, const char *buffer, int length );

    QMutex m_lock;
    bool m_postInProgress;
    bool m_certain;
    int m_completed;
    int m_total;
    RuleEngine *m_engine;
};

class QProgressBar;
class QLabel;
class QTextEdit;
class Gui : public QWidget, public GuiBase
{
    Q_OBJECT
public:
    Gui();
    ~Gui();
    bool event(QEvent *);
    QObject *object() { return this; }

    QProgressBar *m_bar;
    QLabel *m_label;
    QTextEdit *m_text;

private slots:
    void updateData();
};

class ConsoleGui : public QObject, public GuiBase
{
public:
    ConsoleGui();
    ~ConsoleGui();
    static ConsoleGui *instance();
    bool event(QEvent *);
    QObject *object() { return this; }
    void lock();
    void release(bool = true);
    void draw();
    void write( int fd, const char *buffer, int length );
    void dumpLeftovers();
    void disable();

    int columns;
    char *buffer;
    QByteArray leftover_out;
    QByteArray leftover_err;
    bool enabled;
};

#endif
