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

#ifndef QDLEDITCLIENT_H
#define QDLEDITCLIENT_H

// Local includes
#include "qdlclient.h"

// Qt includes
#include <QMenu>

// Forward class declarations
class QDLLink;
class QDLWidgetClientPrivate;
class QTextEdit;

// ============================================================================
//
// QDLEditClient
//
// ============================================================================

class QTOPIA_EXPORT QDLEditClient : public QDLClient
{
    Q_OBJECT

public:
    QDLEditClient( QTextEdit *edit, const QString& name );
    virtual ~QDLEditClient();

    QMenu *setupStandardContextMenu( QMenu *context = 0 );

    // Access
    bool isValid() const;
    QString hint() const;

    // Modification
    virtual int addLink( QDSData& link );
    virtual void setLink( const int linkId, const QDLLink &link );
    virtual void removeLink( const int linkId );

public slots:
    void requestLinks();
    void verifyLinks();

private:

    QDLWidgetClientPrivate *d;
};

#endif
