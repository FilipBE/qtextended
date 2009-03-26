/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QORDEREDMUTEXLOCKER_P_H
#define QORDEREDMUTEXLOCKER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QMutex;

/*
  Locks 2 mutexes in a defined order, avoiding a recursive lock if
  we're trying to lock the same mutex twice.
*/
class QOrderedMutexLocker
{
public:
    QOrderedMutexLocker(QMutex *m1, QMutex *m2)
        : mtx1((m1 == m2) ? m1 : (m1 < m2 ? m1 : m2)),
          mtx2((m1 == m2) ?  0 : (m1 < m2 ? m2 : m1)),
          locked(false)
    {
        relock();
    }
    ~QOrderedMutexLocker()
    {
        unlock();
    }

    void relock()
    {
        if (!locked) {
            if (mtx1) mtx1->lock();
            if (mtx2) mtx2->lock();
            locked = true;
        }
    }

    void unlock()
    {
        if (locked) {
            if (mtx1) mtx1->unlock();
            if (mtx2) mtx2->unlock();
            locked = false;
        }
    }

    static bool relock(QMutex *mtx1, QMutex *mtx2)
    {
        // mtx1 is already locked, mtx2 not... do we need to unlock and relock?
        if (mtx1 == mtx2)
            return false;
        if (mtx1 < mtx2) {
            mtx2->lock();
            return true;
        }
        mtx1->unlock();
        mtx2->lock();
        mtx1->lock();
        return true;
    }

private:
    QMutex *mtx1, *mtx2;
    bool locked;
};

QT_END_NAMESPACE

#endif
