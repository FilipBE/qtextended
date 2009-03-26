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

#ifndef GSMKEYFILTER_H
#define GSMKEYFILTER_H

#include <qtopiaipcmarshal.h>
#include <QObject>
#include <QFlags>
#include <QRegExp>

class GsmKeyFilterPrivate;

class GsmKeyFilter : public QObject
{
    Q_OBJECT
public:
    explicit GsmKeyFilter( QObject *parent = 0 );
    ~GsmKeyFilter();

    enum Flag
    {
        Send        = (1<<0),
        Immediate   = (1<<1),
        OnCall      = (1<<2),
        Incoming    = (1<<3),
        BeforeDial  = (1<<4),
        TestOnly    = (1<<5)
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    enum ServiceAction
    {
        Activate,
        Deactivate,
        Interrogate,
        Registration,
        Erasure
    };

    bool filter( const QString& digits, Flags flags );

    void addAction
        ( const QString& digits, QObject *target, const char *slot,
          GsmKeyFilter::Flags flags = GsmKeyFilter::Immediate );
    void addAction
        ( const QRegExp& regex, QObject *target, const char *slot,
          GsmKeyFilter::Flags flags = GsmKeyFilter::Immediate );

    void addService
        ( const QString& code, QObject *target, const char *slot,
          GsmKeyFilter::Flags flags = GsmKeyFilter::Send );

signals:
    void setBusy();
    void releaseHeld();
    void releaseActive();
    void releaseAllAcceptIncoming();
    void release( int call );
    void activate( int call );
    void swap();
    void join();
    void transfer();
    void deflect( const QString& number );

private slots:
    void releaseId( const QString& id );
    void activateId( const QString& id );

private:
    GsmKeyFilterPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(GsmKeyFilter::Flags);
Q_DECLARE_USER_METATYPE_ENUM(GsmKeyFilter::ServiceAction);

#endif
