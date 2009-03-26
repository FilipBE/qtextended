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

#ifndef QTOPIAABSTRACTSERVICE_H
#define QTOPIAABSTRACTSERVICE_H

#include <qtopiaglobal.h>

#include <qobject.h>
#include <qstring.h>

class QtopiaAbstractService_Private;
class QTOPIABASE_EXPORT QtopiaAbstractService : public QObject
{
    Q_OBJECT
public:
    explicit QtopiaAbstractService( const QString& service, QObject *parent = 0 );
    ~QtopiaAbstractService();

protected:
    void publishAll();

private:
    QtopiaAbstractService_Private *m_data;
};

#endif
