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

#ifndef HEXKEYVALIDATOR_H
#define HEXKEYVALIDATOR_H

#include <QValidator>
#include <QWidget>
#include <qtopiaglobal.h>

class QTOPIACOMM_EXPORT HexKeyValidator : public QValidator {
public:
    explicit HexKeyValidator( QWidget* parent = 0, int numDigits = 0);
    ~HexKeyValidator() {};

    QValidator::State validate( QString& key, int& curs ) const;
private:
    const int neededNumDigits;
};

#endif
