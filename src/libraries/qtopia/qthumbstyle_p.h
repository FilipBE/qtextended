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

#ifndef QTHUMBSTYLE_P_H
#define QTHUMBSTYLE_P_H

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

#include <QPhoneStyle>

class QThumbStylePrivate;

class QTOPIA_EXPORT QThumbStyle : public QPhoneStyle
{
    Q_OBJECT
public:
    QThumbStyle();
    virtual ~QThumbStyle();

    using QPhoneStyle::polish; // Don't hide these symbols!
    void polish(QWidget *);

    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                        const QWidget *w = 0) const;
    QSize sizeFromContents(ContentsType type, const QStyleOption* opt,
                            const QSize &csz, const QWidget *widget ) const;
    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;

private:
    QThumbStylePrivate *d;
};

#endif
