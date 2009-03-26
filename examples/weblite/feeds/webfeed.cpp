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

#include "webfeed.h"
#include "webliteclient.h"
#include <QXmlStreamReader>
#include <QTextDocument>
#include <QTimer>
#include <QDebug>

class WebFeedLoaderPrivate
{
    public:
        WebFeedElement* root;
        QMap<QString,WebFeedExtensionParser*> extensions;

        WebFeedLoaderPrivate() : root(0) {}
};

WebFeedLoader::WebFeedLoader (QObject* parent) : WebLiteClient(parent)
{
    d = new WebFeedLoaderPrivate;
    connect (this, SIGNAL(loaded()), this, SLOT(startParse()));
}
WebFeedLoader::~WebFeedLoader()
{
    delete d;
}
void WebFeedLoader::installExtension (WebFeedExtensionParser* p)
{
    QStringList uris = p->namespaceUris();
    for (QStringList::iterator it = uris.begin(); it != uris.end(); ++it)
    {
        d->extensions[*it] = p;
    }
}

WebFeedElement* WebFeedLoader::root() const
{
    return d->root;
}

void WebFeedLoader::startParse()
{
    QFile f (this->filename());
    f.open(QIODevice::ReadOnly);
    parse (&f);
    this->abort ();
}
#define RSS_DATE_FORMAT "ddd, dd MMM yyyy hh:mm:ss"
void WebFeedLoader::parse (QIODevice* iodev)
{
    QXmlStreamReader r(iodev);
    WebFeedElement* curElem = 0;
    WebFeedItem* curItem = 0;
    WebFeedImage* curImg = 0;
    while (!r.atEnd())
    {
        bool inItem = curItem && (curItem == curElem);
        bool inImg = curImg && (curImg == curElem);
        r.readNext ();
        QString name = r.name().toString();

        if (r.hasError())
        {
            qWarning() << "WebFeedLoader::parse: XML error" << r.errorString();
        }
        else
        {
            if (r.isStartElement())
            {
                if (r.namespaceUri().toString() == "" || r.namespaceUri().toString() == "http://www.w3.org/2005/Atom")
                {

                    if (name == "channel" || name == "feed")
                    {
                        d->root = new WebFeedChannel;
                        curElem = d->root;
                    }
                    else if (name == "item" || name == "entry")
                    {
                        curElem = curItem = new WebFeedItem;
                        if (d->root)
                            d->root->add(curElem);
                        else
                            d->root = curElem;
                    }
                    else if (name == "img")
                    {
                        curImg = new WebFeedImage;
                        curElem->add(curImg);
                        curElem = curImg;
                    }
                    else if (name == "link")
                    {
                        QXmlStreamAttributes attr = r.attributes();
                        WebFeedLink* lnk = new WebFeedLink;
                        lnk->rel = attr.value("rel").toString();
                        QString href = attr.value("href").toString();
                        if (!href.length())
                            href = r.readElementText();
                        lnk->href = QUrl(href);
                        lnk->type = attr.value("type").toString();
                        lnk->length = attr.value("length").toString().toInt();
                        curElem->add(lnk);
                    }
                    else if (inImg)
                    {
                        if (name == "title")
                        {
                            curImg->title = r.readElementText();
                        }
                        else if (name == "url")
                        {
                            curImg->url = QUrl(r.readElementText());
                        }

                    }
                    else if (inItem)
                    {
                        if (name == "title")
                        {
                            curItem->title = r.readElementText();
                        }
                        else if (name == "pubDate" || name == "published")
                        {
                            QString str = r.readElementText();
                            curItem->pubDate = QDateTime::fromString(str.left(strlen(RSS_DATE_FORMAT)),RSS_DATE_FORMAT);
                        }
                        else if (name == "description" || name == "summary")
                        {
                            curItem->description = r.readElementText();
                        }
                        else if (name == "subtitle")
                        {
                            curItem->subtitle = r.readElementText();
                        }
                        else if (name == "guid" || name == "id")
                        {
                            curItem->uid = r.readElementText();
                        }
                        else if (name == "category")
                        {
                            curItem->categories.push_back(r.readElementText());
                        }
                        else if (name == "enclosure")
                        {
                            QXmlStreamAttributes attr = r.attributes();
                            WebFeedLink* enc = new WebFeedLink;
                            enc->rel = "enclosure";
                            enc->href = QUrl(attr.value("url").toString());
                            enc->type = attr.value("type").toString();
                            enc->length = attr.value("length").toString().toInt();
                            curItem->add(enc);
                        }
                        else if (name == "source")
                        {
                            QXmlStreamAttributes attr = r.attributes();
                            curItem->sourceUrl = QUrl(attr.value("url").toString());
                        }
                    }
                    else // in channel
                    {
                        WebFeedChannel* chan = (WebFeedChannel*)curElem;

                        if (name == "title")
                        {
                            chan->title = r.readElementText();
                        }
                        else if (name == "description" || name == "summary")
                        {
                            chan->description = r.readElementText();
                        }
                        else if (name == "language")
                            chan->lang = r.readElementText();
                        else if (name == "ttl")
                        {
                            QString ttlTxt = r.readElementText();
                            chan->ttlMin = 60;
                            if (ttlTxt != "")
                            {
                                chan->ttlMin = ttlTxt.toInt();
                            }

                            int timeout = chan->ttlMin*60*60*1000;
                            QTimer::singleShot(timeout, this, SLOT(load()));
                            chan->ttl = QDateTime::currentDateTime().addSecs(chan->ttlMin * 60);
                        }
                    }
                }
                else
                {
                    WebFeedExtensionParser* p = d->extensions[r.namespaceUri().toString()];
                    if (p)
                        p->parse (curElem,r);
                }
            }
            else if (r.isEndElement())
            {
                if (name == "img")
                {
                    curImg = NULL;
                    curElem = curItem ? curItem : d->root;
                }
                else if (name == "item" || name == "entry")
                {
                    curItem = NULL;
                    curElem = d->root;
                }
            }
        }
    }

    emit updated ();

}

WebFeedElement::~WebFeedElement()
{
    for (QList<WebFeedElement*>::iterator it = children.begin(); it != children.end(); ++it)
    {
        delete *it;
    }
}
