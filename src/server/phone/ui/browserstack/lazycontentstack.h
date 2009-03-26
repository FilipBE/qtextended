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

#ifndef LAZYCONTENTSTACK_H
#define LAZYCONTENTSTACK_H

#include <QContent>

#include <QStack>
#include <QWidget>

#include "applicationmonitor.h"

class LazyContentStack : public QWidget
{
Q_OBJECT
public:
    enum Flags { NoFlags = 0x00000000, NoStack = 0x00000001 };
    Q_FLAGS(Flags);

    LazyContentStack(Flags lcsFlags = NoFlags, QWidget *parent = 0,
                    Qt::WFlags wflags = 0);

    virtual void reset();
    virtual void resetToView(const QString &);
    virtual void showView(const QString &);
    virtual QString currentView() const;

    bool isDone() const;

public slots:
    // Stack manipulation
    virtual void back();

protected:
    // Lazy creation methods
    virtual QObject* createView(const QString &) = 0;
    virtual void raiseView(const QString &, bool reset) = 0;
    virtual QObject* currentViewObject() = 0;

    // UI methods
    virtual void noView(const QString &);
    virtual void busy(const QContent &);
    virtual void notBusy();

signals:
    void done();
    void viewContentSet( const QContentSet &set );

private slots:
    void appStateChanged(const QString &);
    void execContent(QContent);

private:
    void addView(const QString &, bool);

    QString busyApp;
    QContent busyContent;
    UIApplicationMonitor monitor;
    Flags m_flags;
    QSet<QString> m_views;
    QStack<QString> m_viewStack;
};

#endif
