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
#include <qlist.h>

#include "qgb18030codec.h"

#ifndef QT_NO_TEXTCODECPLUGIN

QT_BEGIN_NAMESPACE

class CNTextCodecs : public QTextCodecPlugin
{
public:
    CNTextCodecs() {}

    QList<QByteArray> names() const;
    QList<QByteArray> aliases() const;
    QList<int> mibEnums() const;

    QTextCodec *createForMib(int);
    QTextCodec *createForName(const QByteArray &);
};

QList<QByteArray> CNTextCodecs::names() const
{
    QList<QByteArray> list;
    list += QGb18030Codec::_name();
    list += QGbkCodec::_name();
    list += QGb2312Codec::_name();
#ifdef Q_WS_X11
    list += QFontGb2312Codec::_name();
    list += QFontGbkCodec::_name();
#endif
    return list;
}

QList<QByteArray> CNTextCodecs::aliases() const
{
    QList<QByteArray> list;
    list += QGb18030Codec::_aliases();
    list += QGbkCodec::_aliases();
    list += QGb2312Codec::_aliases();
#ifdef Q_WS_X11
    list += QFontGb2312Codec::_aliases();
    list += QFontGbkCodec::_aliases();
#endif
    return list;
}

QList<int> CNTextCodecs::mibEnums() const
{
    QList<int> list;
    list += QGb18030Codec::_mibEnum();
    list += QGbkCodec::_mibEnum();
    list += QGb2312Codec::_mibEnum();
#ifdef Q_WS_X11
    list += QFontGb2312Codec::_mibEnum();
    list += QFontGbkCodec::_mibEnum();
#endif
    return list;
}

QTextCodec *CNTextCodecs::createForMib(int mib)
{
    if (mib == QGb18030Codec::_mibEnum())
        return new QGb18030Codec;
    if (mib == QGbkCodec::_mibEnum())
        return new QGbkCodec;
    if (mib == QGb2312Codec::_mibEnum())
        return new QGb2312Codec;
#ifdef Q_WS_X11
    if (mib == QFontGbkCodec::_mibEnum())
        return new QFontGbkCodec;
    if (mib == QFontGb2312Codec::_mibEnum())
        return new QFontGb2312Codec;
#endif
    return 0;
}


QTextCodec *CNTextCodecs::createForName(const QByteArray &name)
{
    if (name == QGb18030Codec::_name() || QGb18030Codec::_aliases().contains(name))
        return new QGb18030Codec;
    if (name == QGbkCodec::_name() || QGbkCodec::_aliases().contains(name))
        return new QGbkCodec;
    if (name == QGb2312Codec::_name() || QGb2312Codec::_aliases().contains(name))
        return new QGb2312Codec;
#ifdef Q_WS_X11
    if (name == QFontGbkCodec::_name() || QFontGbkCodec::_aliases().contains(name))
        return new QFontGbkCodec;
    if (name == QFontGb2312Codec::_name() || QFontGb2312Codec::_aliases().contains(name))
        return new QFontGb2312Codec;
#endif
    return 0;
}


Q_EXPORT_STATIC_PLUGIN(CNTextCodecs)
Q_EXPORT_PLUGIN2(qcncodecs, CNTextCodecs)

QT_END_NAMESPACE

#endif // QT_NO_TEXTCODECPLUGIN
