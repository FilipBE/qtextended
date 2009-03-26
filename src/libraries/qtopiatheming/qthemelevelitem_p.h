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

#ifndef QTHEMELEVELITEM_P_H
#define QTHEMELEVELITEM_P_H

#include <QExpressionEvaluator>
#include <QObject>

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
class QThemeLevelItem;

class QThemeAnimationFrameInfo : public QObject
{
public:
    QThemeAnimationFrameInfo(QThemeLevelItem *a, int p);
    ~QThemeAnimationFrameInfo();
    void setPeriod(int p);
    int getPeriod();
    void start();
    void stop();

protected:
    void timerEvent(QTimerEvent *);

private:
    QThemeLevelItem *anim;
    int tid;
    int period;
    QPixmap pm;
};

/***************************************************************************/

class QThemeLevelItemPrivate
{
public:
    int currentFrame;
    int value;
    QString text;
    int count;
    int min;
    int max;
    int inc;
    int delay;
    bool playing;
    int looprev;
    int loop;

    QExpressionEvaluator* playExpression;
    QExpressionEvaluator* frameExpression;
    QThemeAnimationFrameInfo* animInfo;

    QThemeLevelItemPrivate()
            : currentFrame(0), value(0), count(0), min(0), max(0), delay(0), playing(false), looprev(0),
            loop(0), playExpression(0), frameExpression(0), animInfo(0) {
    }

    ~QThemeLevelItemPrivate() {
        if (animInfo)
            delete animInfo;
    }
};

#endif

