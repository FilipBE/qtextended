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
#ifndef QDOCUMENTSELECTORSOCKETSERVER_P_H
#define QDOCUMENTSELECTORSOCKETSERVER_P_H

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

#include "qdocumentserverchannel_p.h"
#include <private/qunixsocketserver_p.h>
#include <QContent>

class QDocumentSelector;
class QDialog;
class NewDocumentDialog;
class SaveDocumentDialog;

class QDocumentSelectorServer : public QDocumentServerHost
{
    Q_OBJECT
public:
    QDocumentSelectorServer( QObject *parent = 0 );
    virtual ~QDocumentSelectorServer();

protected:
    virtual QDocumentServerMessage invokeMethod( const QDocumentServerMessage &message );
    virtual void invokeSlot( const QDocumentServerMessage &message );

private slots:
    void documentSelected( const QContent &document );
    void newDocumentAccepted();
    void saveDocumentAccepted();
    void rejected();

private:
    QDocumentSelector *m_selector;
    QDialog *m_selectorDialog;

    NewDocumentDialog *m_newDocumentDialog;
    SaveDocumentDialog *m_saveDocumentDialog;

    QIODevice::OpenMode m_openMode;
    QContent m_selectedDocument;
};

class QDocumentSelectorSocketServer : public QUnixSocketServer
{
public:
    QDocumentSelectorSocketServer( QObject *parent = 0 );

protected:
    virtual void incomingConnection( int socketDescriptor );
};

Q_DECLARE_USER_METATYPE_ENUM(QIODevice::OpenMode);

#endif
