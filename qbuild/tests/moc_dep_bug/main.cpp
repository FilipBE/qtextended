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

#include <stdio.h>
#include <QObject>
#include "foo.h"

class Foo : public QObject
{
#ifdef USE_FOO
    Q_OBJECT
#endif
public:
    Foo()
        : QObject()
    {
    }
};

int main( int /*argc*/, char ** /*argv*/ )
{
    Foo foo;
    printf("hello world\n");
    return 0;
}

#include "main.moc"
