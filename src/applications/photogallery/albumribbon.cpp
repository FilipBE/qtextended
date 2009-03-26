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
#include "albumribbon.h"
#include "smoothimagemover.h"

#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

AlbumRibbon::AlbumRibbon(Type type, QWidget *parent)
    : QAbstractSlider(parent)
    , m_type(type)
    , m_minimumSpacing(8)
    , m_spacing(-1)
    , m_visibleValues(0)
    , m_scrollPosition(0)
    , m_listWidth(0)
    , m_decrementTimerId(-1)
    , m_incrementTimerId(-1)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

AlbumRibbon::~AlbumRibbon()
{
}

AlbumRibbon::Type AlbumRibbon::type() const
{
    return m_type;
}

void AlbumRibbon::setType(Type type)
{
    if (type != m_type) {
        m_type = type;

        m_spacing = -1;

        update();
    }
}

int AlbumRibbon::minimumSpacing() const
{
    return m_minimumSpacing;
}

void AlbumRibbon::setMinimumSpacing(int spacing)
{
    if (spacing != m_minimumSpacing) {
        m_minimumSpacing = spacing;

        m_spacing = -1;

        update();
    }
}

QSize AlbumRibbon::sizeHint() const
{
    if (!m_sizeHint.isValid()) {
        QSize textSize = fontMetrics().size(0, QLatin1String("0000"));

        int left, top, right, bottom;

        getContentsMargins(&left, &top, &right, &bottom);

        int buttonSize
            = style()->pixelMetric(QStyle::PM_ButtonIconSize)
            + style()->pixelMetric(QStyle::PM_ButtonMargin) * 2;

        int height
            = buttonSize
            + top
            + bottom;

        int width 
            = buttonSize * 2
            + textSize.width()
            + left
            + right;

        m_sizeHint = QSize(width, height);
    }
    return m_sizeHint;
}

void AlbumRibbon::sliderChange(SliderChange change)
{
    switch (change) {
    case SliderRangeChange:
        m_spacing = -1;
        update();
        break;
    case SliderOrientationChange:
        break;
    case SliderStepsChange:
        break;
    case SliderValueChange:
        ensureVisible(value());
        break;
    }
}

void AlbumRibbon::paintEvent(QPaintEvent *event)
{
    QAbstractSlider::paintEvent(event);

    if (m_spacing == -1)
        doLayout();

    QStyleOption option;

    option.initFrom(this);

    QPainter painter(this);

    const int buttonMargin = style()->pixelMetric(QStyle::PM_ButtonMargin);
    option.palette.setColor(QPalette::ButtonText, option.palette.color(QPalette::WindowText));

    if (m_scrollPosition < 0) {
        option.rect = m_leftButtonRect;
        option.rect.adjust(buttonMargin, buttonMargin, -buttonMargin, -buttonMargin);
        style()->drawPrimitive(QStyle::PE_IndicatorArrowLeft, &option, &painter);
    }

    if (m_scrollPosition > -m_listWidth + m_listRect.width()) {
        option.rect = m_rightButtonRect;
        option.rect.adjust(buttonMargin, buttonMargin, -buttonMargin, -buttonMargin);
        style()->drawPrimitive(QStyle::PE_IndicatorArrowRight, &option, &painter);
    }

    painter.setClipRect(m_listRect);

    for (int value = valueForPos(m_listRect.x()); ; ++value) {
        QRect textRect = rectForValue(value);

        if (textRect.left() > m_listRect.right()) {
            break;
        } else if (value >= minimum() && value <= maximum() && textRect.right() > m_listRect.left()) {
            textRect.translate(0, option.fontMetrics.descent() / 2);

            painter.setPen(value == sliderPosition()
                    ? option.palette.color(QPalette::BrightText)
                    : option.palette.color(QPalette::WindowText));

            if (m_type == YearRibbon)
                painter.drawText(textRect, Qt::AlignCenter, QString::number(value));
            else if (m_type == CharacterRibbon)
                painter.drawText(textRect, Qt::AlignCenter, QChar(value));
        }
    }
}

void AlbumRibbon::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mouseOver(hitTest(event->pos()), event->pos());

        event->accept();
    } else {
        QAbstractSlider::mousePressEvent(event);
    }
}

void AlbumRibbon::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        Control control = hitTest(event->pos());

        if (m_decrementTimerId != -1 && control != LeftButton) {
            killTimer(m_decrementTimerId);

            m_decrementTimerId = -1;
        }

        if (m_incrementTimerId != -1 && control != RightButton) {
            killTimer(m_incrementTimerId);

            m_incrementTimerId = -1;
        }

        mouseOver(control, event->pos());

        event->accept();
    } else {
        QAbstractSlider::mouseMoveEvent(event);
    }
}

void AlbumRibbon::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_decrementTimerId != -1) {
            killTimer(m_decrementTimerId);

            m_decrementTimerId = -1;
        }

        if (m_incrementTimerId != -1) {
            killTimer(m_incrementTimerId);

            m_incrementTimerId = -1;
        }
        event->accept();
    } else {
        QAbstractSlider::mouseReleaseEvent(event);
    }

}

void AlbumRibbon::resizeEvent(QResizeEvent *event)
{
    QAbstractSlider::resizeEvent(event);

    ensureVisible(value());
}

void AlbumRibbon::showEvent(QShowEvent *event)
{
    QAbstractSlider::showEvent(event);

    ensureVisible(value());
}

void AlbumRibbon::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_decrementTimerId) {
        decrementRibbon();

        event->accept();
    } else if (event->timerId() == m_incrementTimerId) {
        incrementRibbon();

        event->accept();
    } else {
        QAbstractSlider::timerEvent(event);
    }
}

void AlbumRibbon::incrementRibbon()
{
    const int scrollLimit = m_listWidth - m_listRect.width();

    if (m_scrollPosition > -scrollLimit) {
        const int newValue = value() + 1;

        m_scrollPosition = qMax(m_scrollPosition - m_textSize.width() - m_spacing, -scrollLimit);

        setValue(newValue);

        update();
    }
}

void AlbumRibbon::decrementRibbon()
{
    if (m_scrollPosition < 0) {
        const int newValue = value() - 1;

        m_scrollPosition = qMin(m_scrollPosition + m_textSize.width() + m_spacing, 0);

        setValue(newValue);

        update();
    }
}

void AlbumRibbon::ensureVisible(int value)
{
    if (m_spacing != -1) {
        QRect rect = rectForValue(value);

        if (rect.left() < m_listRect.left())
            m_scrollPosition += m_listRect.left() - rect.left();
        else if (rect.right() > m_listRect.right())
            m_scrollPosition += m_listRect.right() - rect.right();
    }
    update();
}

void AlbumRibbon::mouseOver(Control control, const QPoint &point)
{
    switch (control) {
    case LeftButton:
        if (m_decrementTimerId == -1) {
            decrementRibbon();

            m_decrementTimerId = startTimer(100);
        }
        break;
    case RightButton:
        if (m_incrementTimerId == -1) {
            incrementRibbon();

            m_incrementTimerId = startTimer(100);
        }
        break;
    case List:
        {
            bool exactMatch = false;

            int pressedValue = valueForPos(point.x(), &exactMatch);

            if (pressedValue != value())
                setValue(pressedValue);
        }
        break;
    default:
        break;
    }
}

AlbumRibbon::Control AlbumRibbon::hitTest(const QPoint &point)
{
    if (m_spacing == -1)
        doLayout();

    if (m_leftButtonRect.contains(point))
        return LeftButton;
    else if (m_rightButtonRect.contains(point))
        return RightButton;
    else if (m_leftButtonRect.united(m_rightButtonRect).contains(point))
        return List;
    else
        return None;
}

QRect AlbumRibbon::rectForValue(int value) const
{
    const int offset = m_listRect.x() + m_scrollPosition;

    int x   = (value - minimum()) * (m_textSize.width() + m_spacing) + offset;
    int y = m_listRect.top();

    return QRect(x, y, m_textSize.width(), m_listRect.height());
}

int AlbumRibbon::valueForPos(int position, bool *exactMatch) const
{
    const int offset = m_listRect.x() + m_scrollPosition;

    int value = minimum() + (position - offset - m_spacing / 2) / (m_textSize.width() + m_spacing);

    if (exactMatch) {
        *exactMatch
                =  value >= minimum()
                && value <= maximum()
                && rectForValue(value).contains(QPoint(position, m_listRect.top()));
    }

    return value;
}

void AlbumRibbon::doLayout()
{
    QRect r = rect();

    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);

    r.adjust(left, top, -right, -bottom);

    const int buttonSize
        = style()->pixelMetric(QStyle::PM_ButtonIconSize)
        + style()->pixelMetric(QStyle::PM_ButtonMargin) * 2;

    m_textSize = fontMetrics().size(0, m_type == YearRibbon
            ? QLatin1String("0000")
            : QLatin1String("W"));

    const int values = maximum() - minimum() + 1;

    const int minimumWidth
            = values * m_textSize.width()
            + (values - 1) * m_minimumSpacing;

    if (minimumWidth > r.width()) {

        m_leftButtonRect = QRect(
            r.left(),
            r.top(),
            buttonSize,
            r.height());

        m_rightButtonRect = QRect(
            r.right() - buttonSize,
            r.top(),
            buttonSize,
            r.height());

        m_listRect = QRect(
            m_leftButtonRect.right(),
            r.top(),
            m_rightButtonRect.left() - m_leftButtonRect.right(),
            r.height());
    } else {
        m_leftButtonRect = QRect();
        m_rightButtonRect = QRect();
        m_listRect = r;
    }

    m_visibleValues
        = (m_listRect.width() - m_minimumSpacing)
        / (m_textSize.width() + m_minimumSpacing);

    if (m_visibleValues > 1) {
        m_spacing
                = (m_listRect.width() - m_textSize.width() * m_visibleValues)
                / (m_visibleValues - 1);
    } else {
        m_spacing = 0;
    }

    m_listWidth
            = values * m_textSize.width()
            + (values - 1) * m_spacing;

    if (m_listWidth < m_listRect.width()) {
        m_listRect.translate((m_listRect.width() - m_listWidth) / 2, 0);
        m_listRect.setWidth(m_listWidth);
    }
}
