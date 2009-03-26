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

#ifndef QSDPXMLPARSER_P_H
#define QSDPXMLPARSER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QXmlDefaultHandler>

#include <QStack>
#include <QVariant>
#include <QString>

#include <qbluetoothsdprecord.h>

class QSdpXmlHandler : public QXmlDefaultHandler
{
public:
    QSdpXmlHandler();

    bool startElement(const QString &namespaceURI,
                      const QString &localName,
                      const QString &qName,
                      const QXmlAttributes &attributes);

    bool endElement(const QString &namespaceURI,
                    const QString &localName,
                    const QString &qName);

    bool characters(const QString &str);
    bool fatalError(const QXmlParseException &exception);
    QString errorString() const;

    const QBluetoothSdpRecord &record() const;
    void reset();

private:
    enum Encoding { Default, Hex };

    bool parseElement(const QString &tag,
                      const QString &name,
                      const QString &value,
                      QSdpXmlHandler::Encoding encoding);

    bool parseBoolElement(const QString &value);
    bool parseUInt8Element(const QString &value);
    bool parseUInt16Element(const QString &value);
    bool parseUInt32Element(const QString &value);
    bool parseUInt64Element(const QString &value);
    bool parseUInt128Element(const QString &value);
    bool parseInt8Element(const QString &value);
    bool parseInt16Element(const QString &value);
    bool parseInt32Element(const QString &value);
    bool parseInt64Element(const QString &value);
    bool parseInt128Element(const QString &value);
    bool parseUuidElement(const QString &value);
    bool parseUrlElement(const QString &value);
    bool parseTextElement(const QString &value, QSdpXmlHandler::Encoding encoding);
    bool parseNilElement();

    QString m_errorString;
    bool m_isRecord;
    QStack<QVariant> m_stack;
    int m_currentId;
    QBluetoothSdpRecord m_record;

    Q_DISABLE_COPY(QSdpXmlHandler)
};

class QSdpXmlParser_Private;

class QSdpXmlParser
{
public:
    enum Error { NoError, ParseError };

    QSdpXmlParser();
    ~QSdpXmlParser();

    bool parseRecord(const QByteArray &data);
    bool parseRecord(QIODevice *device);
    QSdpXmlParser::Error lastError() const;
    const QBluetoothSdpRecord &record() const;
    QString errorString() const;

private:
    QSdpXmlParser_Private *m_data;
    Q_DISABLE_COPY(QSdpXmlParser)
};

#endif
