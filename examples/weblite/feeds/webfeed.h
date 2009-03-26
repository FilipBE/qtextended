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

#ifndef WEBFEED_H
#define WEBFEED_H
#include "webliteclient.h"

#define ELEM_FINAL_TYPE virtual int _typeId() const { return qMetaTypeId<T>()

class QXmlStreamReader;
struct WebFeedElement
{
    QList<WebFeedElement*> children;
    template <class T> 
    QList<T*> elementsByType() const
    {
        
        int id = qMetaTypeId<T>();
        QList<T*> l;
        for (QList<WebFeedElement*>::const_iterator it = children.begin(); it != children.end(); ++it)
        {
            if ((*it)->typeId() == id)
                l.push_back((T*)*it);
        }
        return l;
    }
    virtual int typeId() const { return qMetaTypeId<WebFeedElement>(); };
    
    void add(WebFeedElement* e) { children.push_back(e); }
    
    virtual ~WebFeedElement();
};
Q_DECLARE_METATYPE(WebFeedElement)

struct WebFeedImage : WebFeedElement
{
    QString title;
    QString description;
    QUrl url;
    QSize size;
    QUrl link;
    virtual int typeId() const { return qMetaTypeId<WebFeedImage>(); };
};
Q_DECLARE_METATYPE(WebFeedImage)

struct WebFeedLink : WebFeedElement
{
    QString rel;
    QUrl href;
    QString type;
    int length;
    virtual int typeId() const { return qMetaTypeId<WebFeedLink>(); };
};
Q_DECLARE_METATYPE(WebFeedLink)

struct WebFeedItem : public WebFeedElement
{
    QString title;
    QString subtitle;
    QUrl link;
    QUrl sourceUrl;
    QString description;
    QString author;
    QStringList categories;
    QDateTime pubDate;
    QString uid;
    virtual int typeId() const { return qMetaTypeId<WebFeedItem>(); }
};
Q_DECLARE_METATYPE(WebFeedItem)

struct WebFeedChannel : public WebFeedElement
{
    QUrl url;
    QDateTime ttl;
    int ttlMin;
    QString title;
    QString subtitle;
    QUrl link;
    QString lang;
    QString description;
    QString uid;
    
    QList<WebFeedItem*> items() const { return elementsByType<WebFeedItem>(); }
    virtual int typeId() const { return qMetaTypeId<WebFeedChannel>(); };
};
Q_DECLARE_METATYPE(WebFeedChannel)


class WebFeedExtensionParser 
{
    public:
        virtual ~WebFeedExtensionParser(){}
    virtual void parse (WebFeedElement* elem,
                        QXmlStreamReader & reader) = 0;
    
    virtual QStringList namespaceUris () const = 0;
};



class WebFeedLoaderPrivate;
class WebFeedLoader : public WebLiteClient
{
    Q_OBJECT
            
    public:
        WebFeedLoader (QObject* parent = NULL);
        virtual ~WebFeedLoader();
        
        void installExtension (WebFeedExtensionParser*);
        
        WebFeedElement* root() const;
    public slots:
        void parse (QIODevice* iodev);
        
    private slots:  
        void startParse ();
                
    signals:
        void updated    ();
        
    private:
        WebFeedLoaderPrivate* d;
};
#endif
