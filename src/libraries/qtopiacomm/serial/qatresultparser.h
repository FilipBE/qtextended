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

#ifndef QATRESULTPARSER_H
#define QATRESULTPARSER_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qtopiaglobal.h>

class QAtResultParserPrivate;
class QAtResult;

class QTOPIACOMM_EXPORT QAtResultParser
{
public:
    QAtResultParser( const QAtResult& result );
    QAtResultParser( const QString& notification );
    ~QAtResultParser();

    class QTOPIACOMM_EXPORT Node
    {
        friend class QAtResultParser;
    private:
        enum Kind { Number, Range, String, List };

        Node( uint number );
        Node( uint first, uint last );
        Node( const QString& str );
        Node( QList<Node> *list );
    public:
        Node( const Node& other );
        ~Node();

        bool isNumber() const { return ( _kind == Number ); }
        bool isRange() const { return ( _kind == Range ); }
        bool isString() const { return ( _kind == String ); }
        bool isList() const { return ( _kind == List ); }

        uint asNumber() const { return ( _kind == Number ? _number : 0 ); }
        uint asFirst() const { return ( _kind == Range ? _number : 0 ); }
        uint asLast() const { return ( _kind == Range ? _last : 0 ); }
        QString asString() const { return _str; }
        QList<QAtResultParser::Node> asList() const
            { return ( _list ? *_list : QList<QAtResultParser::Node>() ); }

    private:
        Kind _kind;
        uint _number;
        uint _last;
        QString _str;
        QList<QAtResultParser::Node> *_list;
    };

    void reset();
    bool next( const QString& prefix );
    QString line();
    uint readNumeric();
    QString readString();
    void skip();
    QString readNextLine();
    QStringList lines( const QString& prefix );
    QList<QAtResultParser::Node> readList();

private:
    QAtResultParserPrivate *d;
};

#endif
