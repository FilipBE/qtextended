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

#ifndef THEMEDVIEWINTERFACE_P_H
#define THEMEDVIEWINTERFACE_P_H

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

#include <qfactoryinterface.h>
#include <qtopiaglobal.h>

class QPainter;
class QRect;

struct QTOPIA_EXPORT ThemedItemInterface
{
    virtual void resize(int w, int h) = 0;
    virtual void paint(QPainter *p, const QRect &r) = 0;
    virtual ~ThemedItemInterface();
};

#define ThemedItemInterface_iid "com.trolltech.Qtopia.ThemedItemInterface"
Q_DECLARE_INTERFACE(ThemedItemInterface, ThemedItemInterface_iid)

class QTOPIA_EXPORT ThemedItemPlugin : public QObject, public ThemedItemInterface
{
    Q_OBJECT
    Q_INTERFACES(ThemedItemInterface)
public:
    explicit ThemedItemPlugin( QObject* parent = 0 );
    virtual ~ThemedItemPlugin();
};

#endif
