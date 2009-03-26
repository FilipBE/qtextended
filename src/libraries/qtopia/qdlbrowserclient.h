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

#ifndef QDLBROWSERCLIENT_H
#define QDLBROWSERCLIENT_H

// Local includes
#include "qdlclient.h"

// Qt includes
#include <QTextBrowser>

// Forward class declarations
class QDLBrowserClientPrivate;

// ============================================================================
//
// QDLBrowserClient
//
// ============================================================================

class QTOPIA_EXPORT QDLBrowserClient : public QTextBrowser
{
    Q_OBJECT

public:
    QDLBrowserClient( QWidget* parent, const QString& name );
    virtual ~QDLBrowserClient();

    // QTextBrowser overloads
    virtual void setSource( const QUrl & name );

    // Modification
    void loadLinks( const QString &str );

public slots:
    void activateLink( const QUrl& link );
    void verifyLinks();

private slots:
    void browserTextChanged();

private:
    QDLBrowserClientPrivate *d;
};

#endif
