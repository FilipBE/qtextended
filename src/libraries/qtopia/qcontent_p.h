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

#ifndef QCONTENT_P_H
#define QCONTENT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
#include <QObject>
#include <QTimer>
#include <QMutex>
#include <qatomic.h>
#include <qcontent.h>

class QValueSpaceObject;

class QValueSpaceProxyObject : public QObject
{
    Q_OBJECT
    public:
        explicit QValueSpaceProxyObject( const QString & objectPath, QObject * parent = 0 );
        virtual ~QValueSpaceProxyObject();
        QString objectPath () const;
        static void sync ();
        void setAttribute ( const QByteArray & attribute, const QVariant & data );
        void setAttribute ( const char * attribute, const QVariant & data );
        void setAttribute ( const QString & attribute, const QVariant & data );
        void removeAttribute ( const QString & attribute );

    signals:
        void itemRemove ( const QByteArray & attribute );
        void itemSetValue ( const QByteArray & attribute, const QVariant & value );
        void doInit(const QString & objectPath);
        void doInternalSetAttribute(const QString& attribute, const QVariant &data);
        void doInternalremoveAttribute ( const QString & attribute );

    private slots:
        void init( const QString & objectPath );

    private:
        QValueSpaceObject *d;
        QString path;
};

class QContentUpdateManager : public QObject
{
    Q_OBJECT
    public:
        QContentUpdateManager(QObject *parent=NULL);
        void addUpdated(QContentId id, QContent::ChangeType c);
        void requestRefresh();

        static QContentUpdateManager *instance();

    signals:
        void refreshRequested();

    public slots:
        void beginInstall();
        void endInstall();
        void beginSendingUpdates();
        void endSendingUpdates();
        void sendUpdate();

    private:

        QList<QPair<QContentId,QContent::ChangeType> > updateList;
        QTimer updateTimer;
        QMutex mutex;
        QAtomicInt installAtom;
        QAtomicInt updateAtom;
        QValueSpaceProxyObject *vsoDocuments;
};

#endif
