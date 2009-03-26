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

#ifndef TESTSLAVEINTERFACE_P_H
#define TESTSLAVEINTERFACE_P_H

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

#include <QVariantMap>
class QString;
class QWidget;

class TestSlaveInterface {
public:
    virtual ~TestSlaveInterface() {}

    virtual void postMessage(QString const &name, QVariantMap const &data) = 0;
    virtual bool isConnected() const = 0;
    virtual void showMessageBox(QWidget*,QString const&,QString const&) = 0;
    virtual void showDialog(QWidget*,QString const&) = 0;
};

Q_DECLARE_INTERFACE(TestSlaveInterface,"com.trolltech.Qtopia.TestSlaveInterface")

#endif
