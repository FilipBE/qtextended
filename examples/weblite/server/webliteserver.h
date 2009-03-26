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

#include "weblitecore.h"
#include <QtopiaIpcAdaptor>

class WebLiteServerPrivate;
class WebLiteServer : public QtopiaIpcAdaptor
{
    Q_OBJECT
            
    public:
        WebLiteServer (QObject* parent = NULL);
        virtual ~WebLiteServer ();
        
    public slots:
        void request    (const QByteArray &);
        void request    (const WebLiteLoadRequest &);
        void sendResponse    (WebLiteLoadResponse);
        
    private slots:  
        void useCache (const WebLiteLoadRequest &);
        
    signals:
        void response   (const QByteArray &);
        void abort      (const QUuid & clientId);
        
    private:
        WebLiteServerPrivate* d;
};
