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

#ifndef QOUTPUT_H
#define QOUTPUT_H

#include "qdebug.h"

template <typename T>
class QOutputQuoter;

class QOutput
{
    struct Stream {
        Stream(QIODevice *device) : ts(device), ref(1), type(QtDebugMsg), space(true), message_output(false), eol(false) {}
        Stream(QString *string) : ts(string, QIODevice::WriteOnly), ref(1), type(QtDebugMsg), space(true), message_output(false), eol(false) {}
        Stream(QtMsgType t) : ts(&buffer, QIODevice::WriteOnly), ref(1), type(t), space(true), message_output(true), eol(false) {}
        QTextStream ts;
        QString buffer;
        int ref;
        QtMsgType type;
        bool space;
        bool message_output;
        bool eol;
    } *stream;
public:
    inline QOutput(QIODevice *device) : stream(new Stream(device)) {}
    inline QOutput(QString *string) : stream(new Stream(string)) {}
    inline QOutput(QtMsgType t) : stream(new Stream(t)) {}
    inline QOutput(const QOutput &o):stream(o.stream) { ++stream->ref; }
    inline QOutput &operator=(const QOutput &other);
    inline ~QOutput() {
        if (!--stream->ref) {
            if(stream->message_output)
                qt_message_output(stream->type, stream->buffer.toLocal8Bit().data());
            else if(stream->eol)
                stream->ts << endl;
            delete stream;
        }
    }
    inline QOutput &space() { stream->space = true; stream->ts << " "; return *this; }
    inline QOutput &nospace() { stream->space = false; return *this; }
    inline QOutput &maybeSpace() { if (stream->space) stream->ts << " "; return *this; }
    inline QOutput &eol() { stream->eol = true; return *this; }

    inline QOutput &operator<<(QChar t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(bool t) { stream->ts << (t ? "true" : "false"); return maybeSpace(); }
    inline QOutput &operator<<(char t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(signed short t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(unsigned short t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(signed int t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(unsigned int t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(signed long t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(unsigned long t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(qint64 t)
        { stream->ts << QString::number(t); return maybeSpace(); }
    inline QOutput &operator<<(quint64 t)
        { stream->ts << QString::number(t); return maybeSpace(); }
    inline QOutput &operator<<(float t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(double t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(const char* t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(const QString & t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(const QLatin1String &t) { stream->ts << t.latin1(); return maybeSpace(); }
    inline QOutput &operator<<(const QByteArray & t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(const void * t) { stream->ts << t; return maybeSpace(); }
    inline QOutput &operator<<(QTextStreamFunction f) {
        stream->ts << f;
        return *this;
    }
    inline QOutput &operator<<(QTextStreamManipulator m)
    { stream->ts << m; return *this; }
    template <typename T>
    inline QOutput &operator<<(QOutputQuoter<T> m);
};

inline QOutput &QOutput::operator=(const QOutput &other)
{
    if (this != &other) {
        QOutput copy(other);
        qSwap(stream, copy.stream);
    }
    return *this;
}

template <class T>
inline QOutput &operator<<(QOutput debug, const QList<T> &list)
{
    debug.nospace() << "(";
    for (Q_TYPENAME QList<T>::size_type i = 0; i < list.count(); ++i) {
        if (i)
            debug << ", ";
        debug << list.at(i);
    }
    debug << ")";
    return debug.space();
}

template <typename T>
inline QOutput &operator<<(QOutput debug, const QVector<T> &vec)
{
    debug << "QVector";
    return operator<<(debug, vec.toList());
}

template <class aKey, class aT>
inline QOutput &operator<<(QOutput debug, const QMap<aKey, aT> &map)
{
    debug.nospace() << "QMap(";
    for (typename QMap<aKey, aT>::const_iterator it = map.constBegin();
         it != map.constEnd(); ++it) {
        debug << "(" << it.key() << ", " << it.value() << ")";
    }
    debug << ")";
    return debug.space();
}

template <class aKey, class aT>
inline QOutput &operator<<(QOutput debug, const QHash<aKey, aT> &hash)
{
    debug.nospace() << "QHash(";
    for (typename QHash<aKey, aT>::const_iterator it = hash.constBegin();
            it != hash.constEnd(); ++it)
        debug << "(" << it.key() << ", " << it.value() << ")";
    debug << ")";
    return debug.space();
}

template <class T1, class T2>
inline QOutput &operator<<(QOutput debug, const QPair<T1, T2> &pair)
{
    debug.nospace() << "QPair(" << pair.first << "," << pair.second << ")";
    return debug.space();
}

template <typename T>
inline QOutput &operator<<(QOutput debug, const QSet<T> &set)
{
    debug.nospace() << "QSet";
    return operator<<(debug, set.toList());
}

inline QOutput qOutput() { return QOutput(QtWarningMsg); }

// quote(foo) helper class
template <typename T>
class QOutputQuoter
{
public:
    QOutputQuoter(const T &t, char quote) : m_t(t), m_quote(quote) {}
    QOutputQuoter(const QOutputQuoter &other) : m_t(other.m_t), m_quote(other.m_quote) {}
    QOutput &exec( QOutput &debug )
    {
        debug.nospace() << m_quote << m_t << m_quote;
        return debug.space();
    }
private:
    T m_t;
    char m_quote;
};

template <typename T>
inline QOutput &QOutput::operator<<(QOutputQuoter<T> m)
{
    return m.exec(*this);
}

// quote(foo) supports QChar, char (printing like 'f')
inline QOutputQuoter<QChar> quote(const QChar &t) { return QOutputQuoter<QChar>(t, '\''); }
inline QOutputQuoter<char> quote(char t) { return QOutputQuoter<char>(t, '\''); }
// Anything else is printed like a string ("foo")
template <typename T>
inline QOutputQuoter<T> quote(T t) { return QOutputQuoter<T>(t, '"'); }

#endif
