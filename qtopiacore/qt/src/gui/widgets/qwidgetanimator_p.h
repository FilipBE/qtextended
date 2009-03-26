/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QWIDGET_ANIMATOR_P_H
#define QWIDGET_ANIMATOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <qobject.h>
#include <qrect.h>
#include <qmap.h>

QT_BEGIN_NAMESPACE

class QWidget;
class QTimer;
class QTime;

class QWidgetAnimator : public QObject
{
    Q_OBJECT
public:
    QWidgetAnimator(QObject *parent = 0);
    ~QWidgetAnimator();
    void animate(QWidget *widget, const QRect &final_geometry, bool animate);
    bool animating() const;
    bool animating(QWidget *widget);

    void abort(QWidget *widget);

signals:
    void finished(QWidget *widget);
    void finishedAll();

private slots:
    void animationStep();

private:
    struct AnimationItem {
        AnimationItem(QWidget *_widget = 0, const QRect &_r1 = QRect(),
                        const QRect &_r2 = QRect())
            : widget(_widget), r1(_r1), r2(_r2), step(0) {}
        QWidget *widget;
        QRect r1, r2;
        int step;
    };
    typedef QMap<QWidget*, AnimationItem> AnimationMap;
    AnimationMap m_animation_map;
    QTimer *m_timer;
    QTime *m_time;
};

QT_END_NAMESPACE

#endif // QWIDGET_ANIMATOR_P_H
