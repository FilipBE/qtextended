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

#include "qsdpxmlparser_p.h"

#include <qbluetoothsdprecord.h>
#include <qbluetoothsdpuuid.h>

#include <QUrl>
#include <QByteArray>

#include <QXmlSimpleReader>
#include <QXmlInputSource>


QSdpXmlHandler::QSdpXmlHandler()
{
    reset();
}

// Resets the Handler object
// Should be called before every new sdp xml record parse step
void QSdpXmlHandler::reset()
{
    m_isRecord = false;
}

const QBluetoothSdpRecord &QSdpXmlHandler::record() const
{
    return m_record;
}

bool QSdpXmlHandler::parseBoolElement(const QString &value)
{
    bool r;
    if (value == "true")
        r = true;
    else if (value == "false")
        r = false;
    else {
        m_errorString = "Trouble parsing boolean value, should be 'true' or 'false'";
        return false;
    }

    QVariant var = QVariant::fromValue(r);
    m_stack.push(var);

    return true;
}

bool QSdpXmlHandler::parseUInt8Element(const QString &value)
{
    bool ok;
    QVariant var = QVariant::fromValue<quint8>(value.toULong(&ok, 0));

    if (!ok) {
        m_errorString = "Trouble parsing uint8 value";
        return false;
    }

    m_stack.push(var);

    return true;
}

bool QSdpXmlHandler::parseUInt16Element(const QString &value)
{
    bool ok;
    QVariant var = QVariant::fromValue<quint16>(value.toULong(&ok, 0));

    if (!ok) {
        m_errorString = "Trouble parsing uint16 value";
        return false;
    }

    m_stack.push(var);

    return true;
}

bool QSdpXmlHandler::parseUInt32Element(const QString &value)
{
    bool ok;
    QVariant var = QVariant::fromValue<quint32>(value.toULong(&ok, 0));

    if (!ok) {
        m_errorString = "Trouble parsing uint32 value";
        return false;
    }

    m_stack.push(var);

    return true;
}

bool QSdpXmlHandler::parseUInt64Element(const QString &value)
{
    bool ok;
    QVariant var = QVariant::fromValue<quint64>(value.toULongLong(&ok, 0));

    if (!ok) {
        m_errorString = "Trouble parsing uint64 value";
        return false;
    }

    m_stack.push(var);

    return true;
}

bool QSdpXmlHandler::parseUInt128Element(const QString &value)
{
    quint128 val;
    char buf[3];

    buf[2] = '\0';

    if (value.size() != 32) {
        m_errorString = "Trouble parsing uint128";
        return false;
    }

    for (int i = 0; i < 32; i += 2) {
        buf[0] = value[i].toLatin1();
        buf[1] = value[i + 1].toLatin1();

        val.data[i >> 1] = strtoul(buf, 0, 16);
    }

    QVariant var = QVariant::fromValue<quint128>(val);
    m_stack.push(var);

    return true;
}

bool QSdpXmlHandler::parseInt8Element(const QString &value)
{
    bool ok;
    QVariant var = QVariant::fromValue<qint8>(value.toLong(&ok, 0));

    if (!ok) {
        m_errorString = "Trouble parsing int8 value";
        return false;
    }

    m_stack.push(var);

    return true;
}

bool QSdpXmlHandler::parseInt16Element(const QString &value)
{
    bool ok;
    QVariant var = QVariant::fromValue<qint16>(value.toLong(&ok, 0));

    if (!ok) {
        m_errorString = "Trouble parsing int16 value";
        return false;
    }

    m_stack.push(var);

    return true;
}

bool QSdpXmlHandler::parseInt32Element(const QString &value)
{
    bool ok;
    QVariant var = QVariant::fromValue<qint32>(value.toLong(&ok, 0));

    if (!ok) {
        m_errorString = "Trouble parsing int32 value";
        return false;
    }

    m_stack.push(var);

    return true;
}

bool QSdpXmlHandler::parseInt64Element(const QString &value)
{
    bool ok;
    QVariant var = QVariant::fromValue<qint64>(value.toLongLong(&ok, 0));

    if (!ok) {
        m_errorString = "Trouble parsing int64 value";
        return false;
    }

    m_stack.push(var);

    return true;
}

bool QSdpXmlHandler::parseInt128Element(const QString &value)
{
    qint128 val;
    char buf[3];

    buf[2] = '\0';

    if (value.size() != 32) {
        m_errorString = "Trouble parsing int128";
        return false;
    }

    for (int i = 0; i < 32; i += 2) {
        buf[0] = value[i].toLatin1();
        buf[1] = value[i + 1].toLatin1();

        val.data[i >> 1] = strtoul(buf, 0, 16);
    }

    QVariant var = QVariant::fromValue<qint128>(val);
    m_stack.push(var);

    return true;
}

bool QSdpXmlHandler::parseUuidElement(const QString &value)
{
    QBluetoothSdpUuid val(value);
    if (!val.isValid()) {
        m_errorString = "Trouble parsing uuid element";
        return false;
    }

    QVariant var = QVariant::fromValue(val);
    m_stack.push(var);

    return true;
}

bool QSdpXmlHandler::parseUrlElement(const QString &value)
{
    QUrl val(value);

    if (!val.isValid()) {
        m_errorString = "Trouble parsing url element";
        return false;
    }

    m_stack.push(QVariant::fromValue(val));

    return true;
}

bool QSdpXmlHandler::parseTextElement(const QString &value, QSdpXmlHandler::Encoding encoding)
{
    QVariant var;

    if (encoding == QSdpXmlHandler::Default) {
        var = QVariant::fromValue(value);
    } else {
        int len = value.size();
        char buf[3];

        QByteArray decoded;

        /* Ensure the string is a power of 2 */
        len = (len >> 1) << 1;

        buf[2] = '\0';

        for (int i = 0; i < len; i += 2) {
            buf[0] = value[i].toLatin1();
            buf[1] = value[i + 1].toLatin1();

            decoded.append(strtoul(buf, 0, 16));
        }

        var = QVariant::fromValue(decoded);
    }

    m_stack.push(var);
    return true;
}

bool QSdpXmlHandler::parseNilElement()
{
    m_stack.push(QVariant());

    return true;
}

bool QSdpXmlHandler::parseElement(const QString &tag,
                                  const QString &,
                                  const QString &value,
                                  QSdpXmlHandler::Encoding encoding)
{
    if (tag == "boolean")
        return parseBoolElement(value);
    else if (tag == "uint8")
        return parseUInt8Element(value);
    else if (tag == "uint16")
        return parseUInt16Element(value);
    else if (tag == "uint32")
        return parseUInt32Element(value);
    else if (tag == "uint64")
        return parseUInt64Element(value);
    else if (tag == "uint128")
        return parseUInt128Element(value);
    else if (tag == "int8")
        return parseInt8Element(value);
    else if (tag == "int16")
        return parseInt16Element(value);
    else if (tag == "int32")
        return parseInt32Element(value);
    else if (tag == "int64")
        return parseInt64Element(value);
    else if (tag == "int128")
        return parseInt128Element(value);
    else if (tag == "uuid")
        return parseUuidElement(value);
    else if (tag == "url")
        return parseUrlElement(value);
    else if (tag == "text")
        return parseTextElement(value, encoding);
    else if (tag == "nil")
        return parseNilElement();

    return false;
}

bool QSdpXmlHandler::startElement(const QString &,
                                  const QString &,
                                  const QString &qName,
                                  const QXmlAttributes &attributes)
{
    if (!m_isRecord && qName != "record") {
        m_errorString = QObject::tr("The file is not an SDP XML file.");
        return false;
    }

    if (qName == "record") {
        m_isRecord = true;
        return true;
    }

    if (qName == "attribute") {
        QString id = attributes.value("id");
        if (id.isEmpty()) {
            m_errorString = QObject::tr("<attribute> tag must have an id attribute");
            return false;
        }

        m_currentId = id.toInt(0, 0);
        return true;
    }

    if (qName == "sequence") {
        QVariant var = QVariant::fromValue(QBluetoothSdpSequence());
        m_stack.push(var);
        return true;
    }

    if (qName == "alternate") {
        QVariant var = QVariant::fromValue(QBluetoothSdpAlternative());
        m_stack.push(var);
        return true;
    }

    QSdpXmlHandler::Encoding encoding = QSdpXmlHandler::Default;
    QString value = attributes.value("encoding");
    if (value == "hex") {
        encoding = QSdpXmlHandler::Hex;
    }

    int index = attributes.index("value");
    if (index == -1) {
        value = QString();
    }
    else {
        value = attributes.value(index);
    }

    // We also have the name attribute, but we ignore it for now

    QString name;
    index = attributes.index("name");
    if (index != -1)
        name = attributes.value(index);

    return parseElement(qName, name, value, encoding);
}

bool QSdpXmlHandler::endElement(const QString &,
                                const QString &,
                                const QString &qName)
{
    if (qName == "record") {
        return true;
    }

    if (qName == "attribute") {
        // Handle the case of an empty attribute, bizarre

        if (m_stack.size() == 0) {
            m_record.addAttribute(m_currentId, QVariant());
            return true;
        }

        if (m_stack.size() != 1) {
            m_errorString = "Adding attribute results in stack size != 1";
            return false;
        }

        m_record.addAttribute(m_currentId, m_stack.pop());
        return true;
    }

    // We're ending an element, check to see if we're inside a seq/alt
    // If so, pop it off the stack and add it to the seq/alt
    // If we're not inside seq/alt, make sure stack size is no greater than 1

    if (m_stack.size() > 1) {
        QVariant var = m_stack.pop();

        if (m_stack.top().canConvert<QBluetoothSdpSequence>()) {
            QBluetoothSdpSequence *seq =
                    static_cast<QBluetoothSdpSequence *>(m_stack.top().data());
            seq->push_back(var);
        }
        else if (m_stack.top().canConvert<QBluetoothSdpAlternative>()) {
            QBluetoothSdpAlternative *alt =
                    static_cast<QBluetoothSdpAlternative *>(m_stack.top().data());
            alt->push_back(var);
        }
        else {
            qWarning("Unknown type in the QVariant, should be either a sequence or an alternative");
            m_errorString = "Stack size more than 1 and we're not inside an alt or seq";
            return false;
        }
    }

    return true;
}

bool QSdpXmlHandler::characters(const QString &)
{
    return true;
}

bool QSdpXmlHandler::fatalError(const QXmlParseException &)
{
    return false;
}

QString QSdpXmlHandler::errorString() const
{
    return m_errorString;
}

class QSdpXmlParser_Private
{
public:
    QSdpXmlParser_Private();

    QSdpXmlParser::Error m_error;
    QXmlSimpleReader m_reader;
    QSdpXmlHandler m_handler;
};

QSdpXmlParser_Private::QSdpXmlParser_Private()
{
    // Nothing to do here
}

QSdpXmlParser::QSdpXmlParser()
{
    m_data = new QSdpXmlParser_Private();
    m_data->m_error = QSdpXmlParser::NoError;
    m_data->m_reader.setContentHandler(&m_data->m_handler);
    m_data->m_reader.setErrorHandler(&m_data->m_handler);
}

QSdpXmlParser::~QSdpXmlParser()
{
    delete m_data;
}

bool QSdpXmlParser::parseRecord(const QByteArray &data)
{
    m_data->m_handler.reset();
    m_data->m_error = QSdpXmlParser::NoError;

    QXmlInputSource xmlInputSource;
    xmlInputSource.setData(data);

    if (m_data->m_reader.parse(xmlInputSource))
        return true;

    m_data->m_error = QSdpXmlParser::ParseError;

    return false;
}

bool QSdpXmlParser::parseRecord(QIODevice *device)
{
    m_data->m_handler.reset();
    m_data->m_error = QSdpXmlParser::NoError;

    QXmlInputSource xmlInputSource(device);

    if (m_data->m_reader.parse(xmlInputSource))
        return true;

    m_data->m_error = QSdpXmlParser::ParseError;

    return false;
}

QSdpXmlParser::Error QSdpXmlParser::lastError() const
{
    return m_data->m_error;
}

const QBluetoothSdpRecord &QSdpXmlParser::record() const
{
    return m_data->m_handler.record();
}

QString QSdpXmlParser::errorString() const
{
    return m_data->m_handler.errorString();
}
