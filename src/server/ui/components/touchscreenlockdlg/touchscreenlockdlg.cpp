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

#include "touchscreenlockdlg.h"
#include "uifactory.h"
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QPalette>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QTimeLine>
#include <QFileInfo>
#include <QSvgRenderer>
#include <QPicture>

class KeyItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

public:
    KeyItem(const QPixmap &pixmap, QGraphicsItem *parent = 0)
      : QObject(0), QGraphicsPixmapItem(pixmap, parent)
    {
        setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
        b_unlocked = false;
        timeLine = new QTimeLine(500, this);
        timeLine->setFrameRange(0, 100);
        connect(timeLine, SIGNAL(frameChanged(int)), this, SLOT(move(int)));
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        qreal newPos = event->scenePos().y() - event->lastPos().y();

        qreal y1 = scene()->sceneRect().height() * 0.18;
        qreal y2 = scene()->sceneRect().height() * 0.2;

        if (newPos > y1 && newPos < initialY) {
            setPos(pos().x(), newPos);
            b_unlocked = false;
        }
        if (newPos <= y2) {
            setPos(pos().x(), y1);
            b_unlocked = true;
        }
    }

    void setInitialPos(qreal x, qreal y)
    {
        initialY = y;
        setPos(x, y);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *)
    {
        timeLine->stop();
        timeLine->setStartFrame(0);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *)
    {
        if (b_unlocked) {
            emit unlocked();
            return;
        }
        d = (initialY - scenePos().y()) / 100.0;
        ini = scenePos().y();
        timeLine->start();
    }

protected Q_SLOTS:
    void move(int i)
    {
        setPos(pos().x(), ini + (i * d));
    }

Q_SIGNALS:
    void unlocked();

private:
    bool b_unlocked;
    QTimeLine *timeLine;
    qreal d;
    qreal ini;
    qreal initialY;
};

/*!
  \class TouchScreenLockDialog
    \inpublicgroup QtUiModule
  \brief The TouchScreenLockDialog class provides a dialog that is shown while a touchscreen-only 
  device is locked.
  \ingroup QtopiaServer::GeneralUI
 
  The dialog allows the user to disengage the touchscreen lock via a specific move operation 
  on the screen.

  This class is part of the Qt Extended server and cannot be used by other applications. Any server 
  component that uses this dialog should create an instance via UIFactory::createDialog().
*/

/*!
  Constructs a TouchScreenLockDialog instance with the given \a parent and \a flags.
*/
TouchScreenLockDialog::TouchScreenLockDialog(QWidget *parent, Qt::WFlags flags)
  : QDialog(parent, flags), scene(0)
{
    //set palette before setting window state so we get correct transparency
    QPalette p = palette();
    p.setBrush(QPalette::Window, QBrush(QColor(0,0,0,0)));
    setPalette(p);
    
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    setWindowState(Qt::WindowFullScreen);
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);

    QVBoxLayout *vb = new QVBoxLayout;
    vb->setMargin(0);
    QGraphicsView *v = new QGraphicsView(this);
    v->setFrameStyle(QFrame::NoFrame);
    v->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    v->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scene = new QGraphicsScene(this);
    QDesktopWidget *desktop = QApplication::desktop();
    scene->setSceneRect(desktop->screenGeometry(desktop->primaryScreen()));
    v->setScene(scene);
    vb->addWidget(v);
    setLayout(vb);

    int kw = (int)(scene->sceneRect().width() * 0.14);
    int kh = (int)(scene->sceneRect().height() * 0.21);
    KeyItem *key = new KeyItem(generatePixmap(":image/qpe/Key", kw, kh), 0);
    QObject::connect (key, SIGNAL(unlocked()), this, SLOT(accept()));

    int lw = (int)(scene->sceneRect().width() * 0.27);
    int lh = (int)(scene->sceneRect().height() * 0.29);
    QGraphicsPixmapItem *lock = new QGraphicsPixmapItem(generatePixmap(":image/qpe/Lock", lw, lh), 0);
    scene->addItem(key);
    scene->addItem(lock);

    int dw = desktop->screenGeometry(desktop->primaryScreen()).width();
    int dh = desktop->screenGeometry(desktop->primaryScreen()).height();

    lock->setPos(dw/2 - lock->boundingRect().width()/2, 0);
    lock->setZValue(2);

    key->setInitialPos(dw/2 - key->boundingRect().width()/2, dh - key->boundingRect().height());
    key->setFlag(QGraphicsItem::ItemIsMovable);
    lock->setZValue(1);

    activateWindow();
    raise();
}

QPixmap TouchScreenLockDialog::generatePixmap(const QString &filename, int width, int height) const
{
    QFileInfo fileInfo(filename);

    if (!fileInfo.exists())
        return QPixmap();

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    QPainter painter(&image);

#ifndef QT_NO_PICTURE
    QPicture picture;
    picture.load(fileInfo.filePath());
    QRect br = picture.boundingRect();
    painter.scale(qreal(width) / br.width(), qreal(height) / br.height());
    painter.drawPicture(0, 0, picture);
#else
    QSvgRenderer renderer(fileInfo.filePath());
    renderer.render(&painter);
#endif
    painter.end();
    return QPixmap::fromImage(image);
}
UIFACTORY_REGISTER_WIDGET( TouchScreenLockDialog );
#include "touchscreenlockdlg.moc"

