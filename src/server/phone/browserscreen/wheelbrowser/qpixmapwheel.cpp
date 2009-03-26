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

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPalette>
#include <QLineF>
#include <QGraphicsView>
#include <QTimeLine>
#include <QGraphicsItemAnimation>
#include <QGraphicsLineItem>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QKeyEvent>
#include <math.h>
#include "qpixmapwheel.h"

#define PI 3.14159265
#define MIN_SIZE 0.5
#define MAX_SIZE 2.0

/*!
  \class QPixmapWheel
    \inpublicgroup QtUiModule
  \brief The QPixmapWheel class displays a navigatable 3d "wheel" of pixmaps.

  The QPixmapWheel class displays a set of icons and names in a psuedo-3d
  circular layout.  The wheel can be rotated using the arrow keys, which will
  change the selected (forward-most) icon.

  The set of icons to be displayed are specified by the QPixmapWheelData class.
  This class is separated from QPixmapWheel itself to allow multiple icon sets
  to be created.  The current icon set can be controlled by the setWheel()
  method.  Alternatively, using the moveToWheel() method will change the current
  icon set in a smooth animated fashion.
 */
// define QPixmapWheel
/*!
  Construct a new QPixmapWheel with the specified \a parent.
  */
QPixmapWheel::QPixmapWheel(QWidget *parent)
: QWidget(parent), m_pos(0.0), m_startPos(0.0),  m_moveArc(0.0),
  m_direction(Forward), m_changingWheel(false), m_selected(0), m_timeLine(0),
  m_maximumIcons(-1)
{
    m_timeLine = new QTimeLine(750, this);
    m_timeLine->setLoopCount(1);
    QObject::connect(m_timeLine, SIGNAL(valueChanged(qreal)),
                     this, SLOT(timelinePos(qreal)));

    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0,0,0,0));
    setPalette(pal);
}

qreal QPixmapWheel::normalizeDegrees(qreal orig, qreal arcSize)
{
    Q_ASSERT(arcSize);

    while(orig >= arcSize)
        orig -= arcSize;
    while(orig < 0.0)
        orig += arcSize;

    return orig;
}

qreal QPixmapWheel::arcSize(qreal orig, qreal dest, qreal sweep, Direction dir)
{
    qreal rv;
    if(dir == Forward) {
        rv = dest - orig;
        if(dest <= orig)
            rv += sweep;
    } else {
        rv = orig - dest;
        if(orig <= dest)
            rv += sweep;
    }

    return rv;
}

/*!
  Change the visible \a wheel.  The change will take place immediately.
 */
void QPixmapWheel::setWheel(const QPixmapWheelData &wheel)
{
    if(m_changingWheel) {
        m_changingWheel = false;
        m_incomingWheel = QPixmapWheelData();
        m_timeLine->stop();
        emit moveToCompleted();
    }

    m_wheel = wheel;
    m_selected = 0;

    update();
}

/*!
  Return the maximum number of icons that will be visible on the screen or -1 if
  there is no limit.  The default is -1.

  If the visible wheel contains more than this number of icons, the excess will
  be hidden at the "invisibility" point at the back of the screen.  If -1, the
  icons will be evenly spaced around the 360 degree span of the wheel.  For
  large numbers this can lead to overlap.
  */
int QPixmapWheel::maximumVisibleIcons() const
{
    return m_maximumIcons;
}

/*!
  Set the \a maximum number of icons that will be visible on the screen, or -1
  if there is no limit.
 */
void QPixmapWheel::setMaximumVisibleIcons(int maximum)
{
    m_maximumIcons = maximum;
    update();
}

/*!
  \fn void QPixmapWheel::itemSelected(const QString &name)

  Emitted whenever an item is selected.  \a name will be set to the items name
  as set by the QPixmapWheelData::appendItem() call.
 */

/*!
  \fn void QPixmapWheel::moveToCompleted()

  Emitted whenever a transition caused by moveToWheel() completes.
 */

/*!
  Transition to \a wheel.  The current wheel (if any) will spin out, while the
  new wheel will spin in.

  It is legal to both move to and move from an empty QPixmapWheelData instance.
  */
void QPixmapWheel::moveToWheel(const QPixmapWheelData &wheel)
{
    m_incomingWheel = wheel;

    m_selected = 0;
    m_changingWheel = true;
    moveOverArc(360.0, Forward);
}

int QPixmapWheel::circleWidth() const
{
    return width() - 40;
}

int QPixmapWheel::circleHeight() const
{
    return height() - 48;
}

// Degrees map as follows:
//
//       180
//  90          270
//        0
QPixmapWheel::PixState QPixmapWheel::itemPosition(qreal degrees)
{
    PixState rv;

    qreal drawdegrees = normalizeDegrees(degrees + 90.0, 360.0);
    qreal rad = drawdegrees * PI / 180.0;

    qreal x = (circleWidth() / 2.0) * ::cos(rad);
    qreal y_sin = ::sin(rad);
    qreal y = (circleHeight() / 2.0) * y_sin;

    rv.size = (y_sin + 1.0) / 2.0;

    rv.x = x + (circleWidth() / 2.0) + (width() - circleWidth()) / 2.0;
    rv.y = y + (circleHeight() / 2.0);

    rv.trans = 0.0;

    if(m_wheel.count() || m_incomingWheel.count()) {

        qreal segment = 0.0;
        if(m_wheel.count() && m_incomingWheel.count()) {
            qreal segment1 = segment = 360.0 / segments(m_wheel);
            qreal segment2 = segment = 360.0 / segments(m_incomingWheel);

            segment = m_timeLine->currentValue() * segment2 +
                      (1.0 - m_timeLine->currentValue()) * segment1;

        } else if(m_wheel.count()) segment = 360.0 / segments(m_wheel);
        else segment = 360.0 / segments(m_incomingWheel);

        if(degrees <= segment) {
            rv.trans = 1.0 - (degrees / segment);
        } else if(degrees >= 360.0 - segment) {
            rv.trans = 1.0 - (360.0 - degrees) / segment;
        }
    }

    return rv;
}

void QPixmapWheel::draw(QPainter *p,
                        qreal degrees,
                        const QPixmapWheelData &data,
                        int dataId)
{
    QFontMetrics fm(font());
    QColor baseColor = palette().text().color();

    const QPixmap  &pix = data.pixmap(dataId);

    PixState pos = itemPosition(degrees);
    if(pos.size == 0.0)
        return;

    qreal scaledWidth = pix.width() * pos.size;
    qreal scaledHeight = pix.width() * pos.size;
    qreal scaledX = pos.x - scaledWidth / 2.0;
    qreal scaledY = pos.y - scaledHeight / 2.0;

    if(scaledWidth > 3 && scaledHeight > 3) {
        p->drawPixmap((int)scaledX, (int)scaledY, (int)scaledWidth,
                      (int)scaledHeight, pix, 0, 0, pix.width(), pix.height());
    }

    const QString &text = data.text(dataId);
    int textWidth = fm.width(text);
    int textHeight = fm.height();

    unsigned char alpha = (unsigned char)(255.0 * pos.trans);

    if(alpha != 0) {
        QColor color = baseColor;
        color.setAlpha(alpha);
        p->setPen(color);
        p->drawText((int)(pos.x - textWidth / 2),
                    (int)(scaledY + scaledHeight + textHeight), text);
    }
}

/*! \internal */
void QPixmapWheel::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if(!m_changingWheel) {

        for(int ii = 0; ii < m_wheel.count(); ++ii) {
            qreal itemPos = degrees(m_wheel, ii);
            qreal arc = wheelArc(m_wheel);
            qreal deg = squashDegrees(itemPos, m_pos, arc);
            if(deg >= 0.0)
                draw(&p, deg, m_wheel, ii);
        }

    } else {

        qreal arcSoFar = arcSize(m_startPos, m_pos, wheelArc(m_wheel), Forward);
        if(wheelArc(m_wheel) == arcSoFar) arcSoFar = 0.0;

        for(int ii = 0; ii < m_wheel.count(); ++ii) {
            qreal itemPos = degrees(m_wheel, ii);
            qreal arc = wheelArc(m_wheel);
            qreal deg = squashDegrees(itemPos, m_pos, arc);
            if(deg < 0)
                continue;

            qreal adjDeg;
            if(deg >= 0 && deg <= 180.0)
                adjDeg = 180.0 - deg;
            else
                adjDeg = 360.0 - (deg - 180.0);

            if(adjDeg >= arcSoFar)
                draw(&p, deg, m_wheel, ii);
        }
        for(int ii = 0; ii < m_incomingWheel.count(); ++ii) {
            qreal itemPos = degrees(m_incomingWheel, ii);
            qreal arc = wheelArc(m_incomingWheel);
            qreal deg = squashDegrees(itemPos, 0.0, arc);

            if(deg < 0)
                continue;

            deg -= arcSoFar;
            if(deg < 0.0)
                deg += 360.0;

            qreal adjDeg;
            if(deg >= 0 && deg <= 180.0)
                adjDeg = 180.0 - deg;
            else
                adjDeg = 360.0 - (deg - 180.0);

            if(adjDeg < arcSoFar)
                draw(&p, deg, m_incomingWheel, ii);
        }
    }
}

void QPixmapWheel::timelinePos(qreal value)
{
    if(Forward == m_direction) {
        m_pos = normalizeDegrees(m_startPos + value * m_moveArc,
                                 wheelArc(m_wheel));
    } else {
        m_pos = normalizeDegrees(m_startPos + -1.0 * value * m_moveArc,
                                 wheelArc(m_wheel));
    }

    if(1.0 == value && m_changingWheel) {
        m_changingWheel = false;
        m_wheel = m_incomingWheel;
        m_incomingWheel = QPixmapWheelData();
        m_pos = 0;
        m_selected = 0;
        emit moveToCompleted();
    }

    update();
}

void QPixmapWheel::moveOverArc(qreal arc, Direction dir)
{
    m_startPos = m_pos;
    m_timeLine->stop();
    m_timeLine->setCurrentTime(0);
    m_timeLine->setDuration(2000);

    m_direction = dir;
    m_moveArc = arc;

    m_timeLine->start();
}

void QPixmapWheel::moveToRotation(qreal pos, Direction dir)
{
    m_startPos = m_pos;
    m_timeLine->stop();
    m_timeLine->setCurrentTime(0);
    m_timeLine->setDuration(750);

    m_direction = dir;
    m_moveArc = arcSize(m_startPos, pos, wheelArc(m_wheel), m_direction);

    m_timeLine->start();
}

void QPixmapWheel::moveToPosition(int pos, Direction dir)
{
    Q_ASSERT(pos < m_wheel.count());
    m_selected = pos;

    moveToRotation(degrees(m_wheel, pos), dir);
}

/*! \internal */
void QPixmapWheel::keyPressEvent(QKeyEvent *e)
{
    if(m_changingWheel) {
        QPixmapWheelData data = m_incomingWheel;
        setWheel(data);
        e->accept();
        return;
    }

    int newSelected = m_selected;
    Direction moveDir = Forward;

    if(e->key() == Qt::Key_Right) {

        newSelected = newSelected - 1;
        moveDir = Backward;
        if(newSelected < 0) newSelected = m_wheel.count() + newSelected;

    } else if(e->key() == Qt::Key_Left) {

        newSelected = (newSelected + 1) % m_wheel.count();
        moveDir = Forward;

    } else if(e->key() == Qt::Key_Enter ||
              e->key() == Qt::Key_Return ||
              e->key() == Qt::Key_Select) {

        e->accept();
        if(m_wheel.count() <= m_selected) return;
        emit itemSelected(m_wheel.name(m_selected));
        return;

    } else {
        QWidget::keyPressEvent(e);
        return;
    }
    e->accept();

    moveToPosition(newSelected, moveDir);
}

qreal QPixmapWheel::degrees(const QPixmapWheelData &data, int idx)
{
    Q_ASSERT(idx < data.count());

    if(m_maximumIcons == -1 || data.count() < m_maximumIcons) {
        return (360.0 * idx) / data.count();
    } else {
        return (360.0 * idx) / m_maximumIcons;
    }
}

qreal QPixmapWheel::wheelArc(const QPixmapWheelData &data)
{
    if(m_maximumIcons == -1 || data.count() < m_maximumIcons)
        return 360.0;
    else
        return (360.0 * data.count()) / m_maximumIcons;
}

int QPixmapWheel::segments(const QPixmapWheelData &data)
{
    if(m_maximumIcons == -1 || data.count() < m_maximumIcons)
        return data.count();
    else
        return m_maximumIcons;
}

qreal QPixmapWheel::squashDegrees(qreal itemPos, qreal wheelPos, qreal wheelArc)
{
    Q_ASSERT(wheelArc >= 360.0);
    Q_ASSERT(itemPos < wheelArc && wheelPos < wheelArc);
    Q_ASSERT(itemPos >= 0.0);

    // Check if itemPos should squash to 0.0 to 180.0
    {
        qreal pos = itemPos;
        if(pos < wheelPos)
            pos += wheelArc;

        if(pos >= wheelPos && (wheelPos + 180.0) >= pos)
            return pos - wheelPos;
    }

    // Check if itemPos should squash to 180.+ to 360.-
    {
        qreal pos = itemPos;
        qreal wpos = wheelPos;

        if(pos > wpos)
            wpos += wheelArc;

        if(pos < wpos && pos > wpos - 180.0)
            return 360.0 - (wpos - pos);
    }

    return -1.0;
}

/*!
  \class QPixmapWheelData
    \inpublicgroup QtUiModule
  \brief The QPixmapWheelData class represents a set of icons used by the QPixmapWheel class.
  */
// define QPixmapWheelData
/*!
  Construct an empty QPixmapWheelData instance.
  */
QPixmapWheelData::QPixmapWheelData()
{
}

/*!
  Construct a copy of \a other.
  */
QPixmapWheelData::QPixmapWheelData(const QPixmapWheelData &other)
: m_pics(other.m_pics), m_names(other.m_names), m_texts(other.m_texts)
{
}

/*!
  Assign the contents of \a other to this instance.
  */
QPixmapWheelData &QPixmapWheelData::operator=(const QPixmapWheelData &other)
{
    m_pics = other.m_pics;
    m_names = other.m_names;
    m_texts = other.m_texts;
    return *this;
}

/*!
  Append a new icon.  The icon \a name represents the value emitted from the
  QPixmapWheel::itemSelected() signal when the icon is selected.  The \a pix
  and \a text parameters specify the pixmap and user visible text to be used
  for the icon.
 */
void QPixmapWheelData::appendItem(const QString &name, const QPixmap &pix,
                                  const QString &text)
{
    m_pics.append(pix);
    m_names.append(name);
    m_texts.append(text);
}

/*!
  Returns the number of icons in the data set.
  */
int QPixmapWheelData::count() const
{
    return m_pics.count();
}

/*!
  Return the name of the \a idx icon in the set.  \a idx must be less than
  count().
  */
QString QPixmapWheelData::name(int idx) const
{
    Q_ASSERT(idx < count());

    return m_names.at(idx);
}

/*!
  Return the text for the \a idx icon in the set.  \a idx must be less than
  count().
  */
QString QPixmapWheelData::text(int idx) const
{
    Q_ASSERT(idx < count());

    return m_texts.at(idx);
}

/*!
  Return the pixmap for the \a idx icon in the set.  \a idx must be less than
  count().
  */
QPixmap QPixmapWheelData::pixmap(int idx) const
{
    Q_ASSERT(idx < count());

    return m_pics.at(idx);
}

