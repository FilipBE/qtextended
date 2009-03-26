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

#ifndef QABSTRACTSLIDER_P_H
#define QABSTRACTSLIDER_P_H

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

#include "QtCore/qbasictimer.h"
#include "private/qwidget_p.h"
#include "qstyle.h"

QT_BEGIN_NAMESPACE

class QAbstractSliderPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QAbstractSlider)
public:
    QAbstractSliderPrivate();
    ~QAbstractSliderPrivate();

    void setSteps(int single, int page);

    int minimum, maximum, singleStep, pageStep, value, position, pressValue;
    uint tracking : 1;
    uint blocktracking :1;
    uint pressed : 1;
    uint invertedAppearance : 1;
    uint invertedControls : 1;
    Qt::Orientation orientation;

    QBasicTimer repeatActionTimer;
    int repeatActionTime;
    QAbstractSlider::SliderAction repeatAction;
    
#ifdef QT_KEYPAD_NAVIGATION
    int origValue;
#endif

    inline int bound(int val) const { return qMax(minimum, qMin(maximum, val)); }
    inline void setAdjustedSliderPosition(int position)
    {
        Q_Q(QAbstractSlider);
        if (q->style()->styleHint(QStyle::SH_Slider_StopMouseOverSlider, 0, q)) {
            if ((position > pressValue - 2 * pageStep) && (position < pressValue + 2 * pageStep)) {
                repeatAction = QAbstractSlider::SliderNoAction;
                q->setSliderPosition(pressValue);
                return;
            }
        }
        q->triggerAction(repeatAction);
    }
};

QT_END_NAMESPACE

#endif // QABSTRACTSLIDER_P_H
