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

#include <qtextcodecplugin.h>
#include <qtextcodec.h>
#include <qstringlist.h>

#ifndef QT_NO_TEXTCODECPLUGIN

#include "qeucjpcodec.h"
#include "qjiscodec.h"
#include "qsjiscodec.h"
#ifdef Q_WS_X11
#include "qfontjpcodec.h"
#endif

QT_BEGIN_NAMESPACE

class JPTextCodecs : public QTextCodecPlugin
{
public:
    JPTextCodecs() {}

    QList<QByteArray> names() const;
    QList<QByteArray> aliases() const;
    QList<int> mibEnums() const;

    QTextCodec *createForMib(int);
    QTextCodec *createForName(const QByteArray &);
};

QList<QByteArray> JPTextCodecs::names() const
{
    QList<QByteArray> list;
    list += QEucJpCodec::_name();
    list += QJisCodec::_name();
    list += QSjisCodec::_name();
#ifdef Q_WS_X11
    list += QFontJis0201Codec::_name();
    list += QFontJis0208Codec::_name();
#endif
    return list;
}

QList<QByteArray> JPTextCodecs::aliases() const
{
    QList<QByteArray> list;
    list += QEucJpCodec::_aliases();
    list += QJisCodec::_aliases();
    list += QSjisCodec::_aliases();
#ifdef Q_WS_X11
    list += QFontJis0201Codec::_aliases();
    list += QFontJis0208Codec::_aliases();
#endif
    return list;
}

QList<int> JPTextCodecs::mibEnums() const
{
    QList<int> list;
    list += QEucJpCodec::_mibEnum();
    list += QJisCodec::_mibEnum();
    list += QSjisCodec::_mibEnum();
#ifdef Q_WS_X11
    list += QFontJis0201Codec::_mibEnum();
    list += QFontJis0208Codec::_mibEnum();
#endif
    return list;
}

QTextCodec *JPTextCodecs::createForMib(int mib)
{
    if (mib == QEucJpCodec::_mibEnum())
        return new QEucJpCodec;
    if (mib == QJisCodec::_mibEnum())
        return new QJisCodec;
    if (mib == QSjisCodec::_mibEnum())
        return new QSjisCodec;
#ifdef Q_WS_X11
    if (mib == QFontJis0208Codec::_mibEnum())
        return new QFontJis0208Codec;
    if (mib == QFontJis0201Codec::_mibEnum())
        return new QFontJis0201Codec;
#endif
    return 0;
}


QTextCodec *JPTextCodecs::createForName(const QByteArray &name)
{
    if (name == QEucJpCodec::_name() || QEucJpCodec::_aliases().contains(name))
        return new QEucJpCodec;
    if (name == QJisCodec::_name() || QJisCodec::_aliases().contains(name))
        return new QJisCodec;
    if (name == QSjisCodec::_name() || QSjisCodec::_aliases().contains(name))
        return new QSjisCodec;
#ifdef Q_WS_X11
    if (name == QFontJis0208Codec::_name() || QFontJis0208Codec::_aliases().contains(name))
        return new QFontJis0208Codec;
    if (name == QFontJis0201Codec::_name() || QFontJis0201Codec::_aliases().contains(name))
        return new QFontJis0201Codec;
#endif
    return 0;
}

Q_EXPORT_STATIC_PLUGIN(JPTextCodecs);
Q_EXPORT_PLUGIN2(qjpcodecs, JPTextCodecs);

QT_END_NAMESPACE

#endif // QT_NO_TEXTCODECPLUGIN
