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

#include "threegppcontentplugin.h"
#include <qmimetype.h>
#include <QTextStream>
#include <QFileInfo>

void skip(QFile *file, quint32 jump)
{
    file->seek(file->pos() + jump);
}

/*!
    Reads data using correct codec: UTF-8 or UTF-16
    as per 3gpp spec
    \internal
*/
QString readUTF(QByteArray data)
{
    QTextStream stream(data);
    stream.setAutoDetectUnicode(true);
    return stream.readAll();
}

void getHeaderBox(QFile *file, qint32 &size, QString &type,bool eat)
{
    QByteArray h;
    if (eat)
        h = file->read(8);
    else
        h = file->peek(8);

    unsigned char sz[4] = { h[3], h[2], h[1], h[0] };
    size = *reinterpret_cast< const quint32 * >(&sz[0]);
    QString t( h.constData() + 4 );
    type = t;
}

/*!
    \class ThreeGPPContentPlugin
    \internal
*/
ThreeGPPContentPlugin::ThreeGPPContentPlugin()
{
    m_isAudioOnly = true;
}

ThreeGPPContentPlugin::~ThreeGPPContentPlugin()
{
}

QStringList ThreeGPPContentPlugin::keys() const
{
    return  QMimeType( QLatin1String( "audio/3gpp" )).extensions();
}

bool ThreeGPPContentPlugin::installContent( const QString &filePath, QContent *content)
{
    QFile file(filePath);

    bool success = false;

    if (file.open(QIODevice::ReadOnly))
    {
        qint32 ftypSize = 0;
        QString sig;

        getHeaderBox(&file, ftypSize, sig, false);
        if ( sig == "ftyp")
        {
            content->setName( QFileInfo(filePath).baseName());
            content->setFile( filePath);
            content->setRole( QContent::Document);

            skip(&file, ftypSize);
            findUserData(&file, content);
            if(m_isAudioOnly)
            {
                content->setType( QString("audio/3gpp"));
                success = true;
            }
            else
            {
                content->setType( QString("video/3gpp"));
                success = false;
            }
        }

        file.close();
    }

    return success;
}

bool ThreeGPPContentPlugin::updateContent( QContent *content)
{
    return installContent(content->fileName(), content);
}


void ThreeGPPContentPlugin::findUserData(QFile *file, QContent *content)
{
    QString sig;
    bool valid = true;
    bool found_udta = false;
    qint32 jump = 0;

    while(!file->atEnd() && valid)
    {
        getHeaderBox(file, jump, sig, false);
        if (sig == "moov")                      //look in moov container
        {
            qint32 jump2 = 0;
            qint32 moovSize = 0;

            getHeaderBox(file, moovSize, sig, true);
            moovSize -= 8;
            while  ((moovSize -= jump2)  > 0)
            {
                getHeaderBox(file, jump2, sig, false);
                if (sig == "udta")
                {
                    found_udta = true;
                    break;
                }
                else if (sig == "trak")         //look in trak container
                {
                    qint32 trakSize = 0;
                    qint32 jump3 = 0;

                    getHeaderBox(file, trakSize, sig, true);
                    trakSize -= 8;
                    while ((trakSize -= jump3) > 0)
                    {
                        getHeaderBox(file, jump3, sig, false);
                        if (sig == "udta")
                        {
                            found_udta = true;
                        }
                        if (sig == "mdia")      //look in mdia container for audio only
                        {
                            qint32 mdiaSize = 0;
                            qint32 jump4 = 0;

                            getHeaderBox(file, mdiaSize, sig, true);
                            mdiaSize -= 8;
                            while ((mdiaSize -= jump4) > 0)
                            {
                                getHeaderBox(file, jump4, sig, false);

                                if( sig == "minf")  //look in minf container
                                {
                                    qint32 minfSize = 0;
                                    qint32 jump5 = 0;

                                    getHeaderBox(file, minfSize, sig, true);
                                    minfSize -= 8;
                                    while ((minfSize -= jump5) > 0)
                                    {
                                        getHeaderBox(file, jump5, sig, false);
                                        if(sig == "vmhd")       // has video
                                        {
                                            m_isAudioOnly = false;
                                            return;         //call off the search
                                        }
                                        skip(file, jump5);
                                    }
                                }
                                else
                                {
                                    skip(file, jump4);
                                }
                            }

                        }
                        else
                        {
                            skip(file, jump3);
                        }
                        if(found_udta)
                        {
                            readUserData(file, content);
                            return;
                        }
                    }
                }
                else
                {
                    skip(file, jump2);
                }

            }

            if(found_udta)
            {
               readUserData(file, content);
               return;
            }
            else
            {
                valid = false;
                break;
            }
        }
        else
        {
            skip(file, jump);
        }
     }
}

void ThreeGPPContentPlugin::readUserData(QFile* file, QContent *content)
{
    qint32 box_len = 0;
    qint32 len;

    QString sig;

    const quint32 box_header_size = 8,
                  fullbox_header_size = 4 + box_header_size,
                  header_size = fullbox_header_size + 2;

    getHeaderBox(file, len, sig, true);
    len -= 8;
    while((len-=box_len) > 0)
    {
        getHeaderBox(file, box_len, sig,true);
        skip(file, 6);

        if (sig == "dscp")
        {
            content->setProperty( QContent::Description, readUTF( file->read(box_len - header_size)));
        }
        else if (sig == "cprt")
        {
            content->setProperty( QContent::Copyright, readUTF( file->read(box_len - header_size)));
        }
        else if (sig == "perf")
        {
            content->setProperty( QContent::Artist, readUTF( file->read(box_len - header_size)));
        }
        else if (sig == "auth")
        {
            content->setProperty( QContent::Author, readUTF( file->read(box_len - header_size)));
        }
        else if (sig == "gnre")
        {
            content->setProperty( QContent::Genre, readUTF( file->read(box_len - header_size)));
        }
        else if (sig == "albm")
        {
            content->setProperty( QContent::Album, readUTF( file->read(box_len - header_size - 1)));
            content->setProperty( QContent::Track, QString(file->read(1)));
        }
        else
        {
            skip(file,box_len-header_size);
        }
    }
}

QTOPIA_EXPORT_PLUGIN(ThreeGPPContentPlugin);

