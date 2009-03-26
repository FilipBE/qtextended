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

#ifndef GSMSPEC_H
#define GSMSPEC_H

#include <QString>
#include <QMap>
#include <QStringList>
#include "gsmitem.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDir>

class GsmXmlNode
{
public:
    GsmXmlNode( const QString& tag );
    ~GsmXmlNode();

    GsmXmlNode *parent, *next, *children, *attributes;
    QString tag;
    QString contents;

    void addChild( GsmXmlNode *child );
    void addAttribute( GsmXmlNode *child );
    QString getAttribute( const QString& name );
};


class GsmXmlHandler
{
public:
    GsmXmlHandler();
    ~GsmXmlHandler();

    bool startElement( const QString& name, const QXmlStreamAttributes& atts );
    bool endElement();
    bool characters( const QString& ch );

    GsmXmlNode *documentElement() const;

private:
    GsmXmlNode *tree;
    GsmXmlNode *current;
};

class GSMSpec
{

public:
    GSMSpec(const QString& specFile);
    bool commandExists(const QString&);
    QString getDescription(const QString&);
    QString getProfile(const QString&);
    QStringList getParameterFormat(const QString&);
    QStringList getResponseFormat(const QString&);

    bool validateCommand(QString format, QString pars);
    bool validateResponse(QString format, QString pars);
    void resetDictionary( const QString& );

private:
    QMap<QString, GSMItem> commandMap;
    GSMItem getGSMItem( const QString& );
    void setupDictionary( const QString& );
    //GSMItem createGSMItem( QString command, QString profile, QStringList pars, QStringList resps );
};

#endif
