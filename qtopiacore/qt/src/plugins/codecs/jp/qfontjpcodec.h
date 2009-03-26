/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QFONTJPCODEC_H
#define QFONTJPCODEC_H

#include <QtCore/qtextcodec.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class QJpUnicodeConv;

#ifdef Q_WS_X11
class QFontJis0201Codec : public QTextCodec
{
public:
    QFontJis0201Codec();

    static QByteArray _name();
    static QList<QByteArray> _aliases() { return QList<QByteArray>(); }
    static int _mibEnum();

    QByteArray name() const { return _name(); }
    QList<QByteArray> aliases() const { return _aliases(); }
    int mibEnum() const { return _mibEnum(); }

    QByteArray convertFromUnicode(const QChar *uc, int len,  ConverterState *) const;
    QString convertToUnicode(const char*, int,  ConverterState *) const;
};

class QFontJis0208Codec : public QTextCodec
{
public:
    QFontJis0208Codec();
    ~QFontJis0208Codec();

    static QByteArray _name();
    static QList<QByteArray> _aliases() { return QList<QByteArray>(); }
    static int _mibEnum();

    QByteArray name() const { return _name(); }
    QList<QByteArray> aliases() const { return _aliases(); }
    int mibEnum() const { return _mibEnum(); }

    QString convertToUnicode(const char* /*chars*/, int /*len*/, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *uc, int len, ConverterState *) const;
private:
    QJpUnicodeConv *convJP;
};
#endif

QT_END_NAMESPACE

#endif // QFONTJPCODEC_H
