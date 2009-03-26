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

#ifndef MEDIAFEED_H
#define MEDIAFEED_H
#include "webfeed.h"

struct WebFeedMediaThumbnail : public WebFeedElement
{
    QUrl url;
    QSize size;
    QString time;
    virtual int typeId() const { return qMetaTypeId<WebFeedMediaThumbnail>(); };
};
Q_DECLARE_METATYPE(WebFeedMediaThumbnail)

struct WebFeedMediaContent : public WebFeedElement
{
    QUrl url;
    int fileSize;
    QString contentType;
    enum Medium {
        Image, Audio, Video, Document, Executable
    } medium;
    bool isDefault;

    enum Expression {
        Full, Sample, Nonstop
    } expression;

    int bitrate;
    int frameRate;
    double samplingRate;
    int channels;
    int duration;
    QSize size;
    QString language;
    virtual int typeId() const { return qMetaTypeId<WebFeedMediaContent>(); };
};
Q_DECLARE_METATYPE(WebFeedMediaContent)
        
struct WebFeedPodcastInfo : public WebFeedElement
{
    QString subtitle;
    QString summary;
    QStringList categories;
    QStringList keywords;
    QString author;
    bool isExplicit;
    int durationSecs;
    
    virtual int typeId() const { return qMetaTypeId<WebFeedPodcastInfo>(); };
};
Q_DECLARE_METATYPE(WebFeedPodcastInfo)

QUrl bestImageUrlForFeedItem (WebFeedItem*, const QSize & );
class WebMediaFeedParser : public WebFeedExtensionParser 
{
    virtual void parse (WebFeedElement* parent,
                        QXmlStreamReader & reader);

    virtual QStringList namespaceUris () const;
};

class WebPodcastFeedParser : public WebFeedExtensionParser 
{
    virtual void parse (WebFeedElement* parent,
                        QXmlStreamReader & reader);

    virtual QStringList namespaceUris () const;
};


#endif
