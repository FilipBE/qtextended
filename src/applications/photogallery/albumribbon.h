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
#ifndef ALBUMRIBBON_H
#define ALBUMRIBBON_H

#include <QAbstractSlider>

class SmoothImageMover;

class AlbumRibbon : public QAbstractSlider
{
    Q_OBJECT
public:
    enum Type
    {
        CharacterRibbon,
        YearRibbon
    };

    AlbumRibbon(Type type = YearRibbon, QWidget *parent = 0);
    ~AlbumRibbon();

    Type type() const;
    void setType(Type type);

    int minimumSpacing() const;
    void setMinimumSpacing(int spacing);

    QSize sizeHint() const;

protected:
    void sliderChange(SliderChange change);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void timerEvent(QTimerEvent *event);

private:
    enum Control
    {
        None,
        LeftButton,
        RightButton,
        List
    };

    void incrementRibbon();
    void decrementRibbon();
    void ensureVisible(int value);
    void mouseOver(Control control, const QPoint &point);
    Control hitTest(const QPoint &point);
    QRect rectForValue(int year) const;
    int valueForPos(int position, bool *exactMatch = 0) const;

    void doLayout();

    Type m_type;
    mutable QSize m_sizeHint;
    QRect m_leftButtonRect;
    QRect m_rightButtonRect;
    QRect m_listRect;
    QSize m_textSize;
    int m_minimumSpacing;
    int m_spacing;
    int m_visibleValues;
    int m_scrollPosition;
    int m_listWidth;
    int m_decrementTimerId;
    int m_incrementTimerId;
};

#endif
