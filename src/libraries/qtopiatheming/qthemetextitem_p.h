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

#ifndef QTHEMETEXTITEM_P_H
#define QTHEMETEXTITEM_P_H

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

#include <QThemeItemAttribute>
#include <QExpressionEvaluator>
#include <QString>
#include <QStringList>

class QThemeTextItemPrivate
{

public:
    QThemeTextItemPrivate()
            : textExpression(0),
            richText(false),
            format(Qt::AutoText) {
    }

    QExpressionEvaluator *textExpression;
    QString text;
    QString trText;
    QStringList trArgs;
    QString displayText;
    bool richText;
    Qt::TextFormat format;

    QThemeItemAttribute alignment;
    QThemeItemAttribute font;
    QThemeItemAttribute color;
    QThemeItemAttribute outline;
};

#endif
