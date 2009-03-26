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

#ifndef PINYINIMPL_H
#define PINYINIMPL_H

#include <inputmethodinterface.h>
class PinyinIM;

class PinyinImpl : public QtopiaInputMethod
{

public:
    PinyinImpl(QObject *parent = 0);
    ~PinyinImpl();

    QString name() const;
    QString identifier() const;

    QIcon icon() const;
    QString version() const;

    int properties() const;

    State state() const;

    void reset();

    QWSInputMethod *inputModifier();

    void setHint(const QString &, bool);
private:
    PinyinIM *input;
    QIcon icn;
    ulong ref;
};

#endif
