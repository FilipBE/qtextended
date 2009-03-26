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

#ifndef QKEYGENERATOR_P_H
#define QKEYGENERATOR_P_H

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

#include <QtGlobal>
#include <QObject>

class QPoint;
class QInputGeneratorPrivate;

class QInputGenerator : public QObject
{
Q_OBJECT
public:
    explicit QInputGenerator(QObject* =0);
    virtual ~QInputGenerator();

    void keyPress  (Qt::Key, Qt::KeyboardModifiers, bool=false);
    void keyRelease(Qt::Key, Qt::KeyboardModifiers);
    void keyClick  (Qt::Key, Qt::KeyboardModifiers);

    void mousePress  (QPoint const&, Qt::MouseButtons);
    void mouseRelease(QPoint const&, Qt::MouseButtons);
    void mouseClick  (QPoint const&, Qt::MouseButtons);

    QPoint mapFromActiveWindow(QPoint const&) const;

private:
    QInputGeneratorPrivate* d;
};

#endif

