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
#ifndef QMAILCODEC_H
#define QMAILCODEC_H

#include <qtopiaglobal.h>

#include <QByteArray>
#include <QDataStream>
#include <QString>
#include <QTextStream>

class QTOPIAMAIL_EXPORT QMailCodec
{
public:
    enum { ChunkCharacters = 8192 };

    virtual ~QMailCodec();

    // Stream conversion interface including character translation
    virtual void encode(QDataStream& out, QTextStream& in, const QString& charset = "UTF-8");
    virtual void decode(QTextStream& out, QDataStream& in, const QString& charset);

    // Stream conversion interface
    virtual void encode(QDataStream& out, QDataStream& in);
    virtual void decode(QDataStream& out, QDataStream& in);

    // Convenience functions to encapsulate stream processing
    QByteArray encode(const QString& in, const QString& charset = "UTF-8");
    QString decode(const QByteArray& in, const QString& charset);

    QByteArray encode(const QByteArray& in);
    QByteArray decode(const QByteArray& in);

    // Static helper functions
    static void copy(QDataStream& out, QDataStream& in);
    static void copy(QTextStream& out, QTextStream& in);

protected:
    // Helper functions to convert stream chunks
    virtual void encodeChunk(QDataStream& out, const unsigned char* in, int length, bool finalChunk) = 0;
    virtual void decodeChunk(QDataStream& out, const char* in, int length, bool finalChunk) = 0;
};

class QTOPIAMAIL_EXPORT QMailBase64Codec : public QMailCodec
{
public:
    enum ContentType { Text, Binary };

    QMailBase64Codec(ContentType content, int maximumLineLength = -1);

protected:
    virtual void encodeChunk(QDataStream& out, const unsigned char* in, int length, bool finalChunk);
    virtual void decodeChunk(QDataStream& out, const char* in, int length, bool finalChunk);

private:
    ContentType _content;
    int _maximumLineLength;

    unsigned char _encodeBuffer[3];
    unsigned char* _encodeBufferOut;
    int _encodeLineCharsRemaining;

    unsigned char _decodeBuffer[4];
    unsigned char* _decodeBufferOut;
    int _decodePaddingCount;

    unsigned char _lastChar;
};

class QTOPIAMAIL_EXPORT QMailQuotedPrintableCodec : public QMailCodec
{
public:
    enum ContentType { Text, Binary };
    enum ConformanceType { Rfc2045, Rfc2047 };

    QMailQuotedPrintableCodec(ContentType content, ConformanceType conformance, int maximumLineLength = -1);

protected:
    virtual void encodeChunk(QDataStream& out, const unsigned char* in, int length, bool finalChunk);
    virtual void decodeChunk(QDataStream& out, const char* in, int length, bool finalChunk);

private:
    ContentType _content;
    ConformanceType _conformance;
    int _maximumLineLength;

    int _encodeLineCharsRemaining;
    unsigned char _encodeLastChar;

    char _decodePrecedingInput;
    unsigned char _decodeLastChar;
};

class QTOPIAMAIL_EXPORT QMailPassThroughCodec : public QMailCodec
{
protected:
    virtual void encodeChunk(QDataStream& out, const unsigned char* in, int length, bool finalChunk);
    virtual void decodeChunk(QDataStream& out, const char* in, int length, bool finalChunk);
};

class QTOPIAMAIL_EXPORT QMailLineEndingCodec : public QMailCodec
{
public:
    QMailLineEndingCodec();

protected:
    virtual void encodeChunk(QDataStream& out, const unsigned char* in, int length, bool finalChunk);
    virtual void decodeChunk(QDataStream& out, const char* in, int length, bool finalChunk);

private:
    unsigned char _lastChar;
};

#endif
