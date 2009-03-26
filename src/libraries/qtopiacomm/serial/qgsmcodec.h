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

#ifndef QGSMCODEC_H
#define QGSMCODEC_H

#include <qtopiaglobal.h>

#include <qstring.h>
#include <qtextcodec.h>

class QTOPIACOMM_EXPORT QGsmCodec : public QTextCodec
{
public:
    explicit QGsmCodec( bool noLoss=false );
    ~QGsmCodec();

    QByteArray name() const;
    int mibEnum() const;

    static char singleFromUnicode(QChar ch);
    static QChar singleToUnicode(char ch);

    static unsigned short twoByteFromUnicode(QChar ch);
    static QChar twoByteToUnicode(unsigned short ch);

protected:
    QString convertToUnicode(const char *in, int length, ConverterState *state) const;
    QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const;

private:
    bool noLoss;
};

#endif
