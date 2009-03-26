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

#ifndef WEBLITECLIENT_H
#define WEBLITECLIENT_H
class WebLiteClientPrivate;
#include "weblitecore.h"
#include <Qtopia>
                 
class WebLiteClient : public QObject
{
    /* Enable background downloading, a feature allowing to pre-download files that are probably going to be needed later */
    Q_PROPERTY (bool backgroundDownload READ isBackgroundDownload WRITE setBackgroundDownload)
    Q_PROPERTY (bool direct READ isDirect WRITE setDirect)

    /* The URL to download. can be file:// or http:// */
    Q_PROPERTY (QUrl url READ url WRITE setUrl)
    /* Forces the loader to download the content to a specific filename */
    Q_PROPERTY (QString filename READ filename)
            
    Q_OBJECT
            
    public:
        
        WebLiteClient (QObject* parent = NULL);
        virtual ~WebLiteClient ();
        
        bool isDone() const;
        
        bool isBackgroundDownload () const;
        void setBackgroundDownload (bool);
        
        bool isDirect () const;
        void setDirect (bool);
        
        QUrl url () const;
        void setUrl (const QUrl &);

        void setTimeout(int millis);
        int timeout () const;

        int loadedBytes () const;
        int totalBytes () const;
        QHttp::Error httpError () const;
        WebLiteLoadResponse::Status status () const;

        QString filename() const;

        static void loadInBackground (const QUrl &);

    public slots:
        void load ();
        void abort ();

    private slots:
        void response (const QByteArray &);
        void processTime ();
    signals:
        void progress ();
        void loaded();
        void error ();
        void done (bool err);

        void request    (const QByteArray &);
        void aborted    (const QUuid & clientId);

    private:
        WebLiteClientPrivate *d;
};


#endif
