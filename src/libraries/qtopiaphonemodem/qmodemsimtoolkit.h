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
#ifndef QMODEMSIMTOOLKIT_H
#define QMODEMSIMTOOLKIT_H


#include <qsimtoolkit.h>
#include <qatresult.h>

class QModemService;
class QModemSimToolkitPrivate;

class QTOPIAPHONEMODEM_EXPORT QModemSimToolkit : public QSimToolkit
{
    Q_OBJECT
public:
    explicit QModemSimToolkit( QModemService *service );
    ~QModemSimToolkit();

    QModemService *service() const;

public slots:
    virtual void initialize();
    virtual void begin();
    virtual void end();
    virtual void sendResponse( const QSimTerminalResponse& resp );
    virtual void sendEnvelope( const QSimEnvelope& env );

protected:
    void initializationStarted();
    void initializationDone();
    void fetchCommand( int size );

private slots:
    void fetch( bool ok, const QAtResult& result );

private:
    QModemSimToolkitPrivate *d;
};

#endif
