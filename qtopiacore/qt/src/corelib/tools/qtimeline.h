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

#ifndef QTIMELINE_H
#define QTIMELINE_H

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Core)

class QTimeLinePrivate;
class Q_CORE_EXPORT QTimeLine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int duration READ duration WRITE setDuration)
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval)
    Q_PROPERTY(int currentTime READ currentTime WRITE setCurrentTime)
    Q_PROPERTY(Direction direction READ direction WRITE setDirection)
    Q_PROPERTY(int loopCount READ loopCount WRITE setLoopCount)
    Q_PROPERTY(CurveShape curveShape READ curveShape WRITE setCurveShape)
public:
    enum State {
        NotRunning,
        Paused,
        Running
    };
    enum Direction {
        Forward,
        Backward
    };
    enum CurveShape {
        EaseInCurve,
        EaseOutCurve,
        EaseInOutCurve,
        LinearCurve,
        SineCurve,
        CosineCurve
    };

    explicit QTimeLine(int duration = 1000, QObject *parent = 0);
    virtual ~QTimeLine();

    State state() const;

    int loopCount() const;
    void setLoopCount(int count);

    Direction direction() const;
    void setDirection(Direction direction);

    int duration() const;
    void setDuration(int duration);

    int startFrame() const;
    void setStartFrame(int frame);
    int endFrame() const;
    void setEndFrame(int frame);
    void setFrameRange(int startFrame, int endFrame);

    int updateInterval() const;
    void setUpdateInterval(int interval);

    CurveShape curveShape() const;
    void setCurveShape(CurveShape shape);

    int currentTime() const;
    int currentFrame() const;
    qreal currentValue() const;

    int frameForTime(int msec) const;
    virtual qreal valueForTime(int msec) const;

public Q_SLOTS:
    void start();
    void resume();
    void stop();
    void setPaused(bool paused);
    void setCurrentTime(int msec);
    void toggleDirection();

Q_SIGNALS:
    void valueChanged(qreal x);
    void frameChanged(int);
    void stateChanged(QTimeLine::State newState);
    void finished();

protected:
    void timerEvent(QTimerEvent *event);

private:
    Q_DISABLE_COPY(QTimeLine)
    Q_DECLARE_PRIVATE(QTimeLine)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif

