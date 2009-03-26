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

#include "mediafeed.h"
#include <QXmlStreamReader>
#include <QMimeType>
#include <QDebug>
#include <QTextDocument>


void WebMediaFeedParser::parse (WebFeedElement* parent,
                        QXmlStreamReader & r)
{
    /*
    struct WebFeedMediaContent : public WebFeedElement
    {
        QUrl url;
        int fileSize;
        QString contentType;
        QString medium;
        bool isDefault;
        int bitrate;
        int framerate;
        double samplingRate;
        int channels;
        int duration;
        int hpeeight;
        int w =;
        QString lan;
        QString expression;
        QString rating;
    };
*/
    if (r.name() == "content")
    {
        WebFeedMediaContent* c = new WebFeedMediaContent();
        QXmlStreamAttributes attr = r.attributes();
        c->url = QUrl (attr.value("url").toString());
        QString fs = attr.value("fileSize").toString();
        c->fileSize = fs.length() ? fs.toInt() : -1;

        c->contentType = attr.value("type").toString();
        if (c->contentType == "")
            c->contentType = QMimeType::fromFileName(c->url.toLocalFile()).id();

        QString medium = attr.value("medium").toString();
        if (medium == "")
        {
            if (c->contentType.startsWith("image"))
                c->medium = WebFeedMediaContent::Image;
            else if (c->contentType.startsWith("audio"))
                c->medium = WebFeedMediaContent::Audio;
            else if (c->contentType.startsWith("video"))
                c->medium = WebFeedMediaContent::Video;
            else if (c->contentType.startsWith("application"))
                c->medium = WebFeedMediaContent::Executable;
            else
                c->medium = WebFeedMediaContent::Document;
        }
        else {
            if (medium == "image")
                c->medium = WebFeedMediaContent::Image;
            else if (medium == "audio")
                c->medium = WebFeedMediaContent::Audio;
            else if (medium == "video")
                c->medium = WebFeedMediaContent::Video;
            else if (medium == "executable")
                c->medium = WebFeedMediaContent::Executable;
            else if (medium == "document")
                c->medium = WebFeedMediaContent::Document;
        }
        QString expr = attr.value("expression").toString();
        if (expr == "" || expr == "full")
            c->expression = WebFeedMediaContent::Full;
        else if (expr == "sample")
            c->expression = WebFeedMediaContent::Sample;
        else if (expr == "nonstop")
            c->expression = WebFeedMediaContent::Nonstop;

        QString br = attr.value("bitrate").toString();
        c->bitrate = br.length() ? br.toInt() : 0;
        QString fr = attr.value("framerate").toString();
        c->frameRate = fr.length() ? fr.toInt() : 0;
        QString sr = attr.value("samplingrate").toString();
        c->samplingRate = sr.length() ? sr.toDouble() : 0;
        QString chans = attr.value("channels").toString();
        c->channels = chans.length() ? chans.toInt() : 0;
        QString dur = attr.value("duration").toString();
        c->duration = dur.length() ? dur.toInt() : 0;
        QString height = attr.value("height").toString();
        QString width = attr.value("width").toString();
        c->size = QSize(width.length()?width.toInt():-1,height.length()?height.toInt():-1);
        c->language = attr.value("lang").toString();
        parent->add(c);
    }
    else if (r.name() == "thumbnail")
    {
        WebFeedMediaThumbnail* thumb = new WebFeedMediaThumbnail;
        parent->add(thumb);

        QXmlStreamAttributes attr = r.attributes();
        thumb->url = QUrl(attr.value("url").toString());
        thumb->size = QSize (attr.value("width").toString().toInt(), attr.value("height").toString().toInt());
        thumb->time = attr.value("time").toString();
    }

}

QStringList WebMediaFeedParser::namespaceUris () const
{
    return QStringList() << "http://search.yahoo.com/mrss" << "http://search.yahoo.com/mrss/";
}
QUrl bestImageUrlForFeedItem (WebFeedItem* item, const QSize & sz)
{
    QMap<int,QUrl> byWidth;
    QList<WebFeedMediaThumbnail*> thumbs = item->elementsByType<WebFeedMediaThumbnail>();
    QList<WebFeedMediaContent*> contents = item->elementsByType<WebFeedMediaContent>();
    QList<WebFeedLink*> links = item->elementsByType<WebFeedLink>();
    for (QList<WebFeedLink*>::iterator link_it = links.begin(); link_it != links.end(); ++link_it)
    {
        if ((*link_it)->rel == "enclosure" && ((*link_it)->type == "" || (*link_it)->type.startsWith("image")))
        {
            byWidth[1600] = (*link_it)->href;
        }
    }
    for (QList<WebFeedMediaThumbnail*>::iterator thumb_it = thumbs.begin(); thumb_it != thumbs.end(); ++thumb_it)
    {
        WebFeedMediaThumbnail* thumb = *thumb_it;
        byWidth[thumb->size.width()] = thumb->url;
    }
    for (QList<WebFeedMediaContent*>::iterator content_it = contents.begin(); content_it != contents.end(); ++content_it)
    {
        WebFeedMediaContent* cnt = *content_it;
        if (cnt->contentType.startsWith("image/"))
        {
            byWidth[cnt->size.width()] = cnt->url;
        }
    }

    QUrl def;
    if (!byWidth.empty())
    {

        QMap<int,QUrl>::iterator it= byWidth.end();
        --it;
        def = *it;
    }
    for (QMap<int,QUrl>::iterator it = byWidth.begin(); it != byWidth.end(); ++it)
        if (it.key() >= sz.width())
            return *it;

    return def;
}
void WebPodcastFeedParser::parse (WebFeedElement* parent,
                        QXmlStreamReader & r)
{
    if (!r.isStartElement())
        return;
    WebFeedPodcastInfo* inf = NULL;
    QList<WebFeedPodcastInfo*> l = parent->elementsByType<WebFeedPodcastInfo>();
    if (l.empty())
    {
        inf = new WebFeedPodcastInfo();
        parent->add(inf);
    }
    else
        inf = l[0];
    
    QString n = r.name().toString();
    if (n == "subtitle")
        inf->subtitle = r.readElementText();      
    else if (n == "summary")
            inf->summary = r.readElementText();      
    else if (n == "author")
            inf->subtitle = r.readElementText();      
    else if (n == "keyword")
        inf->keywords << r.readElementText();      
    else if (n == "category")
        inf->categories << r.attributes().value("text").toString();      
    else if (n == "explicit")
        inf->isExplicit = ( r.readElementText() == "yes");      
    else if (n == "duration")
    {
        QString str = r.readElementText();
        inf->durationSecs = 0;
        QStringList l = str.split(":");
        int secs = 1;
        for (int i=l.count()-1; i >=0; --i,secs*=60)
        {
            inf->durationSecs += l[i].toInt()*secs;
        }
        
    }
        
}

QStringList WebPodcastFeedParser::namespaceUris () const
{
    return QStringList() << "http://www.itunes.com/dtds/podcast-1.0.dtd";
}
