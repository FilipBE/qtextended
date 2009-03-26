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

#ifndef SMILMEDIA_H
#define SMILMEDIA_H

#include <element.h>
#include <module.h>
#include <qpixmap.h>

class SmilSystem;
class AudioPlayer;
class VideoPlayer;

class SmilMediaParam : public SmilElement
{
public:
    SmilMediaParam(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts);

    enum ValueType { Data, Ref, Object };

    QString name;
    QString value;
    ValueType valueType;
    QString type;
};

class SmilMedia : public SmilElement
{

public:
    SmilMedia(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts);

    void setState(State s);
    void reset();

    SmilMediaParam *findParameter(const QString &name);

protected:
    QString source;
};

class SmilText : public SmilMedia
{
public:
    SmilText(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts);

    void addCharacters(const QString &ch);
    virtual void setData(const QByteArray &, const QString &);
    virtual void process();
    Duration implicitDuration();
    void paint(QPainter *p);

protected:
    QString text;
    QColor textColor;
    bool waiting;
};

class ImgPrivate;

class SmilImg : public SmilMedia
{
public:
    SmilImg(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts);
    ~SmilImg();

    virtual void setData(const QByteArray &, const QString &);
    virtual void process();
    virtual void setState(State s);
    Duration implicitDuration();
    void paint(QPainter *p);

protected:
    ImgPrivate *d;
    QPixmap pix;
    bool waiting;
};

class SmilAudio : public SmilMedia
{
public:
    SmilAudio(SmilSystem *sys, SmilElement *p, const QString &n, const QXmlStreamAttributes &atts);
    ~SmilAudio();

    virtual void setData(const QByteArray &, const QString &);
    virtual void process();
    virtual void setState(State s);
    Duration implicitDuration();
    void paint(QPainter *p);

protected:
    static AudioPlayer *player;
    QByteArray audioData;
    QString audioType;
    bool waiting;
};

#ifdef MEDIA_SERVER

class SmilVideo : public QObject, public SmilMedia
{
    Q_OBJECT

public:
    SmilVideo(SmilSystem* sys, SmilElement* p, const QString& name, const QXmlStreamAttributes& atts);
    ~SmilVideo();

    virtual void setData(const QByteArray& data, const QString& type);
    virtual void process();
    virtual void setState(State s);
    Duration implicitDuration();
    void paint(QPainter* p);
    void reset();

private slots:
    void widgetAvailable();

protected:
    VideoPlayer* m_videoPlayer;
    QWidget* m_videoWidget;
    bool m_waiting;
};

#endif

class SmilMediaModule : public SmilModule
{
public:
    SmilMediaModule();
    virtual ~SmilMediaModule();

    virtual SmilElement *beginParseElement(SmilSystem *, SmilElement *, const QString &qName, const QXmlStreamAttributes &atts);
    virtual bool parseAttributes(SmilSystem *sys, SmilElement *e, const QXmlStreamAttributes &atts);
    virtual void endParseElement(SmilElement *, const QString &qName);
    virtual QStringList elementNames() const;
    virtual QStringList attributeNames() const;
};

#endif
