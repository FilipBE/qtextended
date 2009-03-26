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

#include <qbluetoothsdprecord.h>
#include <qbluetoothsdpuuid.h>
#include <QUrl>
#include <QString>
#include <QByteArray>

#include <QIODevice>
#include <stdio.h>
#include <stdlib.h>

#include "qsdpxmlgenerator_p.h"

#define STRBUFSIZE 1024
#define MAXINDENT 64

/*!
    \class QSdpXmlGenerator
    \inpublicgroup QtBluetoothModule
    \internal
    \brief The QSdpXmlGenerator class converts the QBluetoothSdpRecord to an XML format

    This class is internal.  It converts a QBluetoothSdpRecord to
    an XML format useable by BlueZ.

    \sa QBluetoothSdpRecord
*/


static void generate_value(const QVariant &val, int indent_level, QIODevice *device)
{
    char buf[STRBUFSIZE];
    char indent[MAXINDENT];
    char next_indent[MAXINDENT];

    if (indent_level >= MAXINDENT)
        indent_level = MAXINDENT - 2;

    for (int i = 0; i < indent_level; i++) {
        indent[i] = '\t';
        next_indent[i] = '\t';
    }

    indent[indent_level] = '\0';
    next_indent[indent_level] = '\t';
    next_indent[indent_level + 1] = '\0';

    buf[STRBUFSIZE - 1] = '\0';

    if (!val.isValid()) {
        device->write(indent);
        device->write("<nil/>\n");
        return;
    }

    if (val.userType() == qMetaTypeId<bool>()) {
        device->write(indent);
        device->write("<boolean value=\"");
        device->write(val.value<bool>() ? "true" : "false");
        device->write("\" />\n");
        return;
    }

    if (val.userType() == qMetaTypeId<quint8>()) {
        device->write(indent);
        device->write("<uint8 value=\"");
        snprintf(buf, STRBUFSIZE - 1, "0x%02x", val.value<quint8>());
        device->write(buf);
        device->write("\" />\n");
        return;
    }

    if (val.userType() == qMetaTypeId<quint16>()) {
        device->write(indent);
        device->write("<uint16 value=\"");
        snprintf(buf, STRBUFSIZE - 1, "0x%04x", val.value<quint16>());
        device->write(buf);
        device->write("\" />\n");
        return;
    }

    if (val.userType() == qMetaTypeId<quint32>()) {
        device->write(indent);
        device->write("<uint32 value=\"");
        snprintf(buf, STRBUFSIZE - 1, "0x%08x", val.value<quint32>());
        device->write(buf);
        device->write("\" />\n");
        return;
    }

    if (val.userType() == qMetaTypeId<quint64>()) {
        device->write(indent);
        device->write("<uint64 value=\"");
        snprintf(buf, STRBUFSIZE - 1, "0x%016jx", val.value<quint64>());
        device->write(buf);
        device->write("\" />\n");
        return;
    }

    if (val.userType() == qMetaTypeId<quint128>()) {
        device->write(indent);
        device->write("<uint128 value=\"");

        quint128 t = val.value<quint128>();

        for (int i = 0; i < 16; i++) {
            sprintf(&buf[i * 2], "%02x",
                        (unsigned char) t.data[i]);
        }

        device->write(buf);
        device->write("\" />\n");
        return;
    }

    if (val.userType() == qMetaTypeId<qint8>()) {
        device->write(indent);
        device->write("<int8 value=\"");
        snprintf(buf, STRBUFSIZE - 1, "%d", val.value<qint8>());
        device->write(buf);
        device->write("\" />\n");
        return;
    }

    if (val.userType() == qMetaTypeId<qint16>()) {
        device->write(indent);
        device->write("<int16 value=\"");
        snprintf(buf, STRBUFSIZE - 1, "%d", val.value<qint16>());
        device->write(buf);
        device->write("\" />\n");
        return;
    }

    if (val.userType() == qMetaTypeId<qint32>()) {
        device->write(indent);
        device->write("<int32 value=\"");
        snprintf(buf, STRBUFSIZE - 1, "%d", val.value<qint32>());
        device->write(buf);
        device->write("\" />\n");
        return;
    }

    if (val.userType() == qMetaTypeId<qint64>()) {
        device->write(indent);
        device->write("<int64 value=\"");
        snprintf(buf, STRBUFSIZE - 1, "%jd", val.value<qint64>());
        device->write(buf);
        device->write("\" />\n");
        return;
    }

    if (val.userType() == qMetaTypeId<qint128>()) {
        device->write(indent);
        device->write("<int128 value=\"");

        qint128 t = val.value<qint128>();

        for (int i = 0; i < 16; i++) {
            sprintf(&buf[i * 2], "%02x",
                     (unsigned char) t.data[i]);
        }

        device->write(buf);
        device->write("\" />\n");

        return;
    }

    if (val.userType() == qMetaTypeId<QBluetoothSdpUuid>()) {
        QBluetoothSdpUuid uuid = val.value<QBluetoothSdpUuid>();

        device->write(indent);
        device->write("<uuid value=\"");
        device->write(uuid.toString().toLatin1());
        device->write("\" />\n");
        return;
    }

    if (val.userType() == qMetaTypeId<QUrl>()) {
        device->write(indent);
        device->write("<url value=\"");
        device->write(val.value<QUrl>().toString().toLatin1());
        device->write("\" />\n");

        return;
    }

    if (val.userType() == qMetaTypeId<QBluetoothSdpSequence>()) {
        device->write(indent);
        device->write("<sequence>\n");

        // Avoid making copies
        const QBluetoothSdpSequence *seq =
                static_cast<const QBluetoothSdpSequence *>(val.data());

        foreach (QVariant var, *seq) {
            generate_value(var, indent_level + 1, device);
        }

        device->write(indent);
        device->write("</sequence>\n");
        return;
    }

    if (val.userType() == qMetaTypeId<QBluetoothSdpAlternative>()) {
        device->write(indent);
        device->write("<alternate>\n");

        // Avoid making copies
        const QBluetoothSdpAlternative *seq =
                static_cast<const QBluetoothSdpAlternative *>(val.data());

        foreach (QVariant var, *seq) {
            generate_value(var, indent_level + 1, device);
        }

        device->write(indent);
        device->write("</alternate>\n");
        return;
    }

    if (val.userType() == qMetaTypeId<QByteArray>()) {
        QByteArray b = val.value<QByteArray>();
        device->write(indent);
        device->write("<text encoding=\"hex\" ");

        char *strBuf = (char *) malloc(sizeof(char) * ( b.size() * 2 + 1));

        for (int i = 0; i < b.size(); i++)
            sprintf(&strBuf[i*sizeof(char)*2], "%02x", (unsigned char) b[i]);

        strBuf[b.size() * 2] = '\0';

        device->write("value=\"");
        device->write(strBuf);
        device->write("\" />\n");

        free(strBuf);

        return;
    }

    if (val.userType() == qMetaTypeId<QString>()) {
        QString s = val.value<QString>();
        int num_chars_to_escape = 0;

        for (int i = 0; i < s.size(); i++) {
            if (!s[i].isPrint()) {
                qWarning("Non-printable characters found in a QString!");
                return;
            }

            /* XML is evil, must do this... */
            if ((s[i] == QChar('<')) ||
                (s[i] == QChar('>')) ||
                (s[i] == QChar('"')) ||
                (s[i] == QChar('&')))
                num_chars_to_escape++;
        }

        device->write(indent);
        device->write("<text ");

        /* Encode our strings in UTF8 */
        QByteArray b = s.toUtf8();
        char *strBuf = (char *) malloc(sizeof(char) * (b.size() + 1 + num_chars_to_escape * 4));

        int j = 0;
        for (int i = 0; i < b.size(); i++) {
            if (b[i] == '&') {
                strBuf[j++] = '&';
                strBuf[j++] = 'a';
                strBuf[j++] = 'm';
                strBuf[j++] = 'p';
            }
            else if (b[i] == '<') {
                strBuf[j++] = '&';
                strBuf[j++] = 'l';
                strBuf[j++] = 't';
            }
            else if (b[i] == '>') {
                strBuf[j++] = '&';
                strBuf[j++] = 'g';
                strBuf[j++] = 't';
            }
            else if (b[i] == '"') {
                strBuf[j++] = '&';
                strBuf[j++] = 'q';
                strBuf[j++] = 'u';
                strBuf[j++] = 'o';
                strBuf[j++] = 't';
            }
            else {
                strBuf[j++] = b[i];
            }
        }

        strBuf[j] = '\0';
        device->write("value=\"");
        device->write(strBuf);
        device->write("\" />\n");

        free(strBuf);

        return;
    }

    qWarning("Unknown SDP Value type!");
}

static void generate_attribute(const QVariant &attr, quint16 id, QIODevice *device)
{
    char buf[STRBUFSIZE];

    buf[STRBUFSIZE - 1] = '\0';
    snprintf(buf, STRBUFSIZE - 1, "\t<attribute id=\"0x%04x\">\n", id);
    device->write(buf);

    generate_value(attr, 2, device);

    device->write("\t</attribute>\n");
}

/*!
    \internal

    Convert a QBluetoothSdpRecord into an XML format useable by BlueZ hcid
    daemon.  The \a device parameter specifies the device to use for
    writing the output.  This could be a QFile or a QBuffer for instance.
*/
void QSdpXmlGenerator::generate(const QBluetoothSdpRecord &record, QIODevice *device)
{
    QList<quint16> attrList = record.attributeIds();

    device->write("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n");
    device->write("<record>\n");

    foreach (quint16 attr, attrList) {
        generate_attribute(record.attribute(attr), attr, device);
    }

    device->write("</record>\n");
}
