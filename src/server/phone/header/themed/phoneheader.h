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

#ifndef PHONEHEADER_H
#define PHONEHEADER_H

#include <QThemedView>
#include <QString>
#include "serverthemeview.h"
#include "qabstractheader.h"

class QRect;

class PhoneHeader : public QAbstractHeader
{
    Q_OBJECT

public:
    PhoneHeader(QWidget *parent = 0, Qt::WFlags fl = 0);
    ~PhoneHeader();
    QSize reservedSize() const;

private:
    QString title;
    QThemedView* themedView;
    double titleSize;
};

#endif
