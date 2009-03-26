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

#include "longstring_p.h"

#include <qtopialog.h>

#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <limits.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifndef USE_FANCY_MATCH_ALGORITHM
#include <ctype.h>
#endif

// LongString: A string/array class for processing IETF documents such as
// RFC(2)822 messages in a memory efficient method.

// The LongString string/array class implemented in this file provides 2
// primary benefits over the QString/QByteArray string/array classes namely:
//
// 1) Inbuilt support for mmap'ing a file so that a string can parsed without
//    requiring all bytes of that string to be loaded into physical memory.
//
// 2) left, mid and right methods that don't create deep copies of the
//    string data, this is achieved by using ref counting and
//    QByteArray::fromRawData
//
// Normal QByteArray methods can be used on LongStrings by utilizing the
// LongString::toQByteArray method.
//
// Known Limitations:
//
// 1) The internal representation is 8bit ascii, which is fine for 7bit ascii
//    email messages.
//
// 2) The underlying data is treated as read only.
//
// 3) mmap may not be supported on non *nix platforms.
//
// Additionally LongString provides a case insensitive indexOf method, this
// is useful as email fields/tokens (From, Date, Subject etc) are case
// insensitive.

#ifdef USE_FANCY_MATCH_ALGORITHM
#define REHASH(a) \
    if (ol_minus_1 < sizeof(uint) * CHAR_BIT) \
        hashHaystack -= (a) << ol_minus_1; \
    hashHaystack <<= 1
#endif

static int insensitiveIndexOf(const QByteArray& target, const QByteArray &source, int from, int off, int len) 
{
#ifndef USE_FANCY_MATCH_ALGORITHM
    const char* const matchBegin = target.constData();
    const char* const matchEnd = matchBegin + target.length();

    const char* const begin = source.constData() + off;
    const char* const end = begin + len - (target.length() - 1);

    const char* it = begin + from;
    while (it < end)
    {
        if (toupper(*it++) == toupper(*matchBegin))
        {
            const char* restart = it;

            // See if the remainder matches
            const char* searchIt = it;
            const char* matchIt = matchBegin + 1;

            do 
            {
                if (matchIt == matchEnd)
                    return ((it - 1) - begin);

                // We may find the next place to search in our scan
                if ((restart == it) && (*searchIt == *(it - 1)))
                    restart = searchIt;
            }
            while (toupper(*searchIt++) == toupper(*matchIt++));

            // No match
            it = restart;
        }
    }

    return -1;
#else
    // Based on QByteArray::indexOf, except use strncasecmp for
    // case-insensitive string comparison
    const int ol = target.length();
    if (from > len || ol + from > len)
        return -1;
    if (ol == 0)
        return from;

    const char *needle = target.data();
    const char *haystack = source.data() + off + from;
    const char *end = source.data() + off + (len - ol);
    const uint ol_minus_1 = ol - 1;
    uint hashNeedle = 0, hashHaystack = 0;
    int idx;
    for (idx = 0; idx < ol; ++idx) {
        hashNeedle = ((hashNeedle<<1) + needle[idx]);
        hashHaystack = ((hashHaystack<<1) + haystack[idx]);
    }
    hashHaystack -= *(haystack + ol_minus_1);

    while (haystack <= end) {
        hashHaystack += *(haystack + ol_minus_1);
        if (hashHaystack == hashNeedle  && *needle == *haystack
             && strncasecmp(needle, haystack, ol) == 0)
        {
            return haystack - source.data();
        }
        REHASH(*haystack);
        ++haystack;
    }
    return -1;
#endif
}


static const int PageSize = ::getpagesize();

class LongStringFileMapping : public QSharedData
{
public:
    LongStringFileMapping();
    LongStringFileMapping(const QString& name);
    ~LongStringFileMapping();

    const QByteArray toQByteArray() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    void init();

    QString filename;
    const char* buffer;
    int len;
};

template <typename Stream> 
Stream& operator<<(Stream &stream, const LongStringFileMapping& mapping) { mapping.serialize(stream); return stream; }

template <typename Stream> 
Stream& operator>>(Stream &stream, LongStringFileMapping& mapping) { mapping.deserialize(stream); return stream; }

LongStringFileMapping::LongStringFileMapping(const QString& name)
    : QSharedData(),
      filename(name),
      buffer(0),
      len(0)
{
    init();
}

LongStringFileMapping::LongStringFileMapping()
    : QSharedData(),
      buffer(0),
      len(0)
{
}

LongStringFileMapping::~LongStringFileMapping()
{
    if (buffer)
    {
        int mapLength = ((len + (PageSize - 1)) / PageSize) * PageSize;
        if (munmap(const_cast<char*>(buffer), mapLength) == -1)
        {
            qWarning() << "Unable to munmap" << mapLength << "bytes from buffer at" << reinterpret_cast<const void*>(buffer) << ':' << errno;
        }
    }
}

void LongStringFileMapping::init()
{
    QFileInfo fi(filename);
    if (fi.exists() && fi.isFile() && fi.isReadable() && fi.size())
    {
        int fd = ::open(QFile::encodeName(fi.absoluteFilePath()), O_RDONLY);
        if (fd != -1)
        {
            int mapLength = ((fi.size() + (PageSize - 1)) / PageSize) * PageSize;
            void* address = ::mmap(0, mapLength, PROT_READ, MAP_SHARED, fd, 0);
            if (address == MAP_FAILED)
            {
                qWarning() << "Unable to mmap" << mapLength << "bytes from" << fi.absoluteFilePath() << ':' << errno;
            }
            else
            {
                buffer = reinterpret_cast<char*>(address);
                len = fi.size();
            }

            ::close(fd);
        }
    }

    if (!buffer)
        qWarning() << "Unable to mmap:" << fi.absoluteFilePath();
}

const QByteArray LongStringFileMapping::toQByteArray() const
{
    // Does not create a copy:
    return QByteArray::fromRawData(buffer, len);
}

template <typename Stream> 
void LongStringFileMapping::serialize(Stream &stream) const
{
    stream << filename;
}

template <typename Stream> 
void LongStringFileMapping::deserialize(Stream &stream)
{
    stream >> filename;
    init();
}


class LongStringPrivate : public QSharedData
{
public:
    LongStringPrivate();
    LongStringPrivate(const QByteArray& ba);
    LongStringPrivate(const QString& filename);

    int length() const;
    bool isEmpty() const;

    int indexOf(const QByteArray &target, int from) const;

    void mid(int i, int len);
    void left(int i);
    void right(int i);

    const QByteArray toQByteArray() const;

    QDataStream* dataStream() const;
    QTextStream* textStream() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QSharedDataPointer<LongStringFileMapping> mapping;
    QByteArray data;
    int offset;
    int len;
};

template <typename Stream> 
Stream& operator<<(Stream &stream, const LongStringPrivate& ls) { ls.serialize(stream); return stream; }

template <typename Stream> 
Stream& operator>>(Stream &stream, LongStringPrivate& ls) { ls.deserialize(stream); return stream; }

LongStringPrivate::LongStringPrivate()
    : QSharedData(),
      mapping(0),
      offset(0), 
      len(0) 
{
}

LongStringPrivate::LongStringPrivate(const QByteArray& ba)
    : QSharedData(),
      mapping(0),
      data(ba),
      offset(0), 
      len(data.length()) 
{
}

LongStringPrivate::LongStringPrivate(const QString& filename)
    : QSharedData(),
      mapping(new LongStringFileMapping(filename)),
      data(mapping->toQByteArray()),
      offset(0), 
      len(data.length()) 
{
}

int LongStringPrivate::length() const
{
    return len;
}

bool LongStringPrivate::isEmpty() const
{
    return (len == 0);
}

int LongStringPrivate::indexOf(const QByteArray &target, int from) const
{
    return insensitiveIndexOf(target, data, from, offset, len);
}

void LongStringPrivate::mid(int i, int size)
{
    i = qMax(i, 0);
    if (i > len)
    {
        len = 0;
    }
    else
    {
        int remainder = len - i;
        if (size < 0 || size > remainder)
            size = remainder;

        offset += i;
        len = size;
    }
}

void LongStringPrivate::left(int size)
{
    if (size < 0 || size > len)
        size = len;

    len = size;
}

void LongStringPrivate::right(int size)
{
    if (size < 0 || size > len)
        size = len;

    offset = (len - size) + offset;
    len = size;
}

const QByteArray LongStringPrivate::toQByteArray() const
{
    // Does not copy:
    return QByteArray::fromRawData(data.constData() + offset, len);
}

QDataStream* LongStringPrivate::dataStream() const
{
    const QByteArray input = toQByteArray();
    return new QDataStream(input);
}

QTextStream* LongStringPrivate::textStream() const
{
    const QByteArray input = toQByteArray();
    return new QTextStream(input);
}

template <typename Stream> 
void LongStringPrivate::serialize(Stream &stream) const
{
    bool usesMapping(mapping != 0);

    stream << usesMapping;
    if (usesMapping) {
        stream << *mapping;
    } else {
        stream << data;
    }
    stream << offset;
    stream << len;
}

template <typename Stream> 
void LongStringPrivate::deserialize(Stream &stream)
{
    bool usesMapping;

    stream >> usesMapping;
    if (usesMapping) {
        mapping = new LongStringFileMapping();
        stream >> *mapping;
        data = mapping->toQByteArray();
    } else {
        stream >> data;
    }
    stream >> offset;
    stream >> len;
}


LongString::LongString()
{
    d = new LongStringPrivate();
}

LongString::LongString(const LongString &other)
{
    this->operator=(other);
}

LongString::LongString(const QByteArray &ba)
{
    d = new LongStringPrivate(ba);
}

LongString::LongString(const QString &fileName)
{
    d = new LongStringPrivate(fileName);
}

LongString::~LongString()
{
}

LongString &LongString::operator=(const LongString &other)
{
    d = other.d;
    return *this;
}

int LongString::length() const
{
    return d->length();
}

bool LongString::isEmpty() const
{
    return d->isEmpty();
}

int LongString::indexOf(const QByteArray &target, int from) const
{
    return d->indexOf(target, from);
}

LongString LongString::mid(int i, int len) const
{
    LongString copy(*this);
    copy.d->mid(i, len);
    return copy;
}

LongString LongString::left(int len) const
{
    LongString copy(*this);
    copy.d->left(len);
    return copy;
}

LongString LongString::right(int len) const
{
    LongString copy(*this);
    copy.d->right(len);
    return copy;
}

const QByteArray LongString::toQByteArray() const
{
    return d->toQByteArray();
}

QDataStream* LongString::dataStream() const
{
    return d->dataStream();
}

QTextStream* LongString::textStream() const
{
    return d->textStream();
}

template <typename Stream> 
void LongString::serialize(Stream &stream) const
{
    d->serialize(stream);
}

template <typename Stream> 
void LongString::deserialize(Stream &stream)
{
    d->deserialize(stream);
}


// We need to instantiate serialization functions for QDataStream
template void LongString::serialize<QDataStream>(QDataStream&) const;
template void LongString::deserialize<QDataStream>(QDataStream&);

