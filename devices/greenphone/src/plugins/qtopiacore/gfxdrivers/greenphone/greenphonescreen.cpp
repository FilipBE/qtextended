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

#include "greenphonescreen.h"
#include <QRect>
#include <QRegion>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QtGui/qscreen_qws.h>
#include <QWSServer>
#include <sys/time.h>
#include <qcopchannel_qws.h>
#include <sched.h>
#include <private/qwindowsurface_qws_p.h>
#include <private/qunixsocketserver_p.h>
#include <QtNetwork/qtcpsocket.h>
#include "blend.h"

class ScreenAnimator;
class GreenphoneScreenPrivate : public QUnixSocketServer
{
    Q_OBJECT
public:
    GreenphoneScreenPrivate();

    ScreenAnimator *animator;

private slots:
    void readyRead();
    void disconnected();

protected:
    virtual void incomingConnection(int fd);
};

class AWindow
{
public:
    AWindow()
        : winId(-1), isReserved(false)
    {
    }
    AWindow(const AWindow &other)
        : winId(other.winId), image(other.image), screen(other.screen),
          isReserved(other.isReserved)
    {
    }
    AWindow &operator=(const AWindow &other)
    {
        winId = other.winId;
        image = other.image;
        screen = other.screen;
        isReserved = other.isReserved;
        return *this;
    }

    int winId;
    QImage image; // RGB16 - unscaled
    QRect screen; // Rect on the screen - may include scaling
    bool isReserved;
};

class ScreenAnimator : public QThread
{
    Q_OBJECT
public:
    ScreenAnimator(int w, int h, int lstep, uchar *buffer, QObject *parent = 0);

    void updateWindow(const AWindow &window);

    virtual void run()
    {
        m_lock.lock();
        m_wait.wakeAll();

        while(true) {
            if(-1 == animationTicker) {
                struct sched_param p;
                ::bzero(&p, sizeof(sched_param));
                p.sched_priority = 0;
                int rv = sched_setscheduler(0, SCHED_OTHER, &p);
                m_wait.wait(&m_lock);
                if(-1 != animationTicker) {
                    struct sched_param p;
                    ::bzero(&p, sizeof(sched_param));
                    p.sched_priority = 1;
                    int rv = sched_setscheduler(0, SCHED_RR, &p);
                }
            } else {
                updateAnimation();
            }

            expose();
        }
        m_lock.unlock();
    }

    void removeReserved(int);
    void addReserved(int, const QRect &);

private slots:
    void receivedQCop(const QString &, const QByteArray &);

private slots:
    void windowEvent(QWSWindow *w, QWSServer::WindowEvent e);

private:
    void expose();
    void updateAnimation();
    void layoutWindows();

    void moveLeft();
    void moveRight();

    int width;
    int height;
    int lineStep;
    uchar *frameBuffer;

    struct AnimData
    {
        QRect imageRect;
        QRect windowRect;
        QRect fromWindowRect;
        QRect destWindowRect;
        bool decoration;
    };
    QList<int> layoutOrder;
    int layoutFocus;

    QHash<int, QRect> reserved;

    QHash<int, AWindow> windows;
    QHash<int, AnimData> anim;

    QHash<int, QString> names;

    QList<int> windowOrder;

    int animationTicker;
    bool shrunk;
    int active;
    QMutex m_lock;
    QWaitCondition m_wait;
};

GreenphoneScreenPrivate::GreenphoneScreenPrivate()
: animator(0)
{
    if(qwsServer) {
        ::unlink("/tmp/region_reserve");
        listen("/tmp/region_reserve");
    }
}

class LinuxFbSocket : public QTcpSocket
{
public:
    LinuxFbSocket() : datacount(0) {}

    bool readReserved(QRect &);
    void sendAck();

private:
    uint data[4];
    uint datacount;
};

bool LinuxFbSocket::readReserved(QRect &r)
{
    char *data_ptr = (char *)data;

    qint64 read_count = read(data_ptr + datacount, sizeof(uint) * 4 - datacount);
    datacount += read_count;
    if(datacount == sizeof(uint) * 4) {
        r = QRect(data[0], data[1], data[2], data[3]);
        datacount = 0;
        return true;
    }

    return false;
}

void LinuxFbSocket::sendAck()
{
    write("a", 1);
}

void GreenphoneScreenPrivate::incomingConnection(int fd)
{
    LinuxFbSocket *sock = new LinuxFbSocket();
    sock->setSocketDescriptor(fd);
    QObject::connect(sock, SIGNAL(readyRead()), this, SLOT(readyRead()));
    QObject::connect(sock, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void GreenphoneScreenPrivate::readyRead()
{
    LinuxFbSocket *sock = static_cast<LinuxFbSocket *>(sender());

    QRect rect;
    if(sock->readReserved(rect)) {
        animator->addReserved((unsigned int)sock, rect);

        sock->sendAck();
    }
}

void GreenphoneScreenPrivate::disconnected()
{
    LinuxFbSocket *sock = static_cast<LinuxFbSocket *>(sender());

    if(animator)
        animator->removeReserved((unsigned int)sock);

    sock->disconnect();
    sock->deleteLater();
}

void stripe(ushort *dest, int dest_width,
            ushort *src, int src_native_width,
            int src_x1, int src_y1, int src_x2, int src_y2)
{
    int deltax = src_x2 - src_x1;
    int deltay = src_y2 - src_y1;

    int error = -deltax / 2;

    int src_yy = src_y1;
    int src_xx = src_x1;

    int src_width = src_x2 - src_x1;

    uint *dest_int = (uint *)dest;
    int src_x1_t_dest_width_tally = src_xx * dest_width - src_x1 * dest_width;

    ushort *src_addr = src + src_xx + src_yy * src_native_width;

    if (deltay < 0)
    {
        deltay *= -1;
        src_native_width *= -1;
    }

    for(int xx = 0; xx < dest_width; ++xx)
    {
        dest[xx] = *src_addr;

        while(src_x1_t_dest_width_tally < 0)
        {
            src_addr++;

            error += deltay;
            src_x1_t_dest_width_tally += dest_width;

            if(error > 0) {
                src_addr += src_native_width;
                error -= deltax;
            }
        }

        src_x1_t_dest_width_tally -= src_width;
    }
}

static ScreenAnimator *g_animator = 0;

ScreenAnimator::ScreenAnimator(int w, int h, int lstep, uchar *buffer, QObject *parent)
: QThread(parent), width(w), height(h), lineStep(lstep), frameBuffer(buffer),
  active(-1), animationTicker(-1), shrunk(false)
{
    Q_ASSERT(g_animator == 0);

    g_animator = this;
    QObject::connect(qwsServer,
                     SIGNAL(windowEvent(QWSWindow*,QWSServer::WindowEvent)),
                     this,
                     SLOT(windowEvent(QWSWindow*,QWSServer::WindowEvent)),
                     Qt::DirectConnection);

    QCopChannel *channel = new QCopChannel("WS/Animation", this);
    QObject::connect(channel, SIGNAL(received(QString,QByteArray)),
                     this, SLOT(receivedQCop(QString,QByteArray)));

    m_lock.lock();
    start();
    // Wait for startup of thread before returning from constructor
    m_wait.wait(&m_lock);
    m_lock.unlock();
}

void ScreenAnimator::updateWindow(const AWindow &window)
{
    m_lock.lock();

    windows[window.winId] = window;

    m_wait.wakeAll(); // trigger redraw
    m_lock.unlock();
}

void ScreenAnimator::removeReserved(int id)
{
    m_lock.lock();
    reserved.remove(id);
    m_wait.wakeAll(); // trigger redraw
    m_lock.unlock();
}

void ScreenAnimator::addReserved(int id, const QRect &rect)
{
    m_lock.lock();
    reserved.insert(id, rect);
    m_wait.wakeAll(); // trigger redraw
    m_lock.unlock();
}

void GreenphoneScreen::addReserved(int id, const QRect &rect)
{
    d->animator->addReserved(id, rect);
}

void GreenphoneScreen::removeReserved(int id)
{
    d->animator->removeReserved(id);
}

void ScreenAnimator::receivedQCop(const QString &message, const QByteArray &)
{
    m_lock.lock();
    if (message == "go()")
    {
        if (shrunk)
        {
            int winId = layoutOrder.at(layoutFocus);
            QList<QWSWindow *> windows = qwsServer->clientWindows();
            QWSWindow *raise = 0;
            for (int ii = 0; !raise && ii < windows.count(); ++ii)
                if (windows.at(ii)->winId() == winId)
                    raise = windows.at(ii);

            if (raise)
            {
                m_lock.unlock();
                raise->setActiveWindow();
                raise->raise();
                m_lock.lock();
            }
        }

        layoutWindows();
        animationTicker = 0;
    } else if(message == "right()") {
        moveRight();
    } else if(message == "left()") {
        moveLeft();
    } else {
        animationTicker = -1;
        anim.clear();
    }
    m_wait.wakeAll();
    m_lock.unlock();
}

void ScreenAnimator::windowEvent(QWSWindow *w, QWSServer::WindowEvent e)
{
    m_lock.lock();

    if(e == QWSServer::Destroy || e == QWSServer::Hide || e == QWSServer::Geometry) {
        if(e == QWSServer::Geometry) {
            QHash<int, AWindow>::Iterator iter = windows.find(w->winId());
            if(iter != windows.end() && !iter->isReserved) {
                windows.erase(iter);
                anim.remove(w->winId());
            }
        } else {
            windows.remove(w->winId());
            anim.remove(w->winId());
        }
    }
    if(e == QWSServer::Destroy) {
        names.remove(w->winId());
    }

    if(e == QWSServer::Active) {
        active = w->winId();
    }

    QList<QWSWindow *> clientWindows = qwsServer->clientWindows();
    windowOrder.clear();
    for(int ii = 0; ii < clientWindows.count(); ++ii) {
        QWSWindow *cw = clientWindows.at(ii);
        windowOrder.append(clientWindows.at(ii)->winId());
        names[cw->winId()] = cw->caption();
    }

    if(animationTicker != -1)
        layoutWindows();

    m_lock.unlock();
}

#define ANIM_STEPS 40
void ScreenAnimator::updateAnimation()
{
    // XXX currently do nothing...
    Q_ASSERT(animationTicker != -1);

    static struct timeval start_tv;
    static struct timeval end_tv;

    if(animationTicker == 0)
        ::gettimeofday(&start_tv, 0);

    animationTicker++;
    int togo = ANIM_STEPS - animationTicker;

    for(QHash<int, AnimData>::Iterator iter = anim.begin();
            iter != anim.end();
            ++iter)
    {
        if (togo) {
            const AnimData &data = *iter;
            int newx = data.fromWindowRect.x() + ((data.destWindowRect.x() - data.fromWindowRect.x()) * animationTicker) / ANIM_STEPS;
            int newy = data.fromWindowRect.y() + ((data.destWindowRect.y() - data.fromWindowRect.y()) * animationTicker) / ANIM_STEPS;
            int newwidth = data.fromWindowRect.width() + ((data.destWindowRect.width() - data.fromWindowRect.width()) * animationTicker) / ANIM_STEPS;
            int newheight = data.fromWindowRect.height() + ((data.destWindowRect.height() - data.fromWindowRect.height()) * animationTicker) / ANIM_STEPS;

            iter->windowRect = QRect(newx, newy, newwidth, newheight);
        } else {
            iter->windowRect = iter->destWindowRect;
            iter->fromWindowRect = iter->destWindowRect;
        }
    }

    if (animationTicker == ANIM_STEPS)
    {
        animationTicker = -1;
        ::gettimeofday(&end_tv, 0);
        int timems = (end_tv.tv_sec - start_tv.tv_sec) * 1000 + (end_tv.tv_usec - start_tv.tv_usec) / 1000;
        qWarning() << "XXXXXXXXXXX Animation time" << timems;
    }
}

/* Call locked */
#define LAYOUT_SPACING 10
void ScreenAnimator::layoutWindows()
{
    int leftPoint = 0;
    int rightPoint = 0;
    int layouts = 0;
    layoutOrder.clear();
    layoutFocus = -1;

    QRegion decorationRegion;

    for(int ii = 0; ii < windowOrder.count(); ++ii)
    {
        QHash<int, AWindow>::Iterator iter = windows.find(windowOrder.at(ii));
        if(iter == windows.end())
            continue;
        const AWindow &window = *iter;

        QString name = names[window.winId];
        if(name == "_decoration_") {
            qWarning() << "Decoration found";
            decorationRegion = decorationRegion.united(window.screen);
        }
    }

    for (int ii = 0; ii < windowOrder.count(); ++ii)
    {
        QHash<int, AWindow>::Iterator iter = windows.find(windowOrder.at(ii));
        if(iter == windows.end())
            continue;
        const AWindow &window = *iter;
        if(window.isReserved)
            continue;

        QRect destRect;
        QRect windowRect;
        QRect imageRect;

        QString name = names[window.winId];

        if (name == "_decoration_")
        {
            windowRect = window.screen;
            imageRect = QRect(QPoint(0,0), window.image.size());

            if(shrunk) {
                destRect = windowRect;
            } else {
                if(window.screen.y() <= height / 2) {
                    // Moves to the top of the screen
                    destRect = window.screen;
                    destRect.moveBottom(-1);
                } else {
                    // Moves to the bottom of the screen
                    destRect = window.screen;
                    destRect.moveTop(height);
                }
            }
        } else {
            QRect winRect =
                QRegion(QRect(0,0,width,height)).intersected(window.screen).subtracted(decorationRegion).boundingRect();

            if(winRect.size() != window.image.size()) {
                    imageRect = QRect(QPoint(winRect.x() - window.screen.x(),
                                             winRect.y() - window.screen.y()),
                                      winRect.size());
            } else {
                imageRect = QRect(QPoint(0,0), window.image.size());
            }

            windowRect = winRect;
            destRect = windowRect;
            if(!shrunk) {
                destRect.setWidth((destRect.width() * 9) / 12);
                destRect.setHeight((destRect.height() * 9) / 12);

                destRect.moveBottom(240);

                if(0 == layouts) {
                    destRect.moveLeft((width - destRect.width()) / 2);
                    leftPoint = destRect.left();
                    rightPoint = destRect.right();

                    layoutOrder.append(window.winId);
                    layoutFocus = 0;

                } else if(layouts & 0x00000001) {
                    // odd - left
                    destRect.moveRight(leftPoint - LAYOUT_SPACING);
                    leftPoint = destRect.left();
                    layoutOrder.prepend(window.winId);
                    layoutFocus++;
                } else {
                    // even - right
                    destRect.moveLeft(rightPoint + LAYOUT_SPACING);
                    rightPoint = destRect.right();
                    layoutOrder.append(window.winId);
                }
                ++layouts;
            }
        }

        QHash<int, AnimData>::Iterator aiter = anim.find(window.winId);
        if(aiter == anim.end()) {
            AnimData data;

            data.imageRect = imageRect;
            data.windowRect = windowRect;
            data.fromWindowRect = windowRect;
            data.destWindowRect = destRect;
            data.decoration = (name == "_decoration_");
            anim[window.winId] = data;
        } else {
            aiter->fromWindowRect = aiter->windowRect;
            aiter->destWindowRect = destRect;
        }
    }

    shrunk = !shrunk;
}

void ScreenAnimator::moveLeft()
{
    if(!shrunk || -1 != animationTicker) {
        return;
    }

    if(layoutFocus == 0) {
        return;
    }

    layoutFocus--;
    int delta = anim[layoutOrder.at(layoutFocus + 1)].destWindowRect.x() - anim[layoutOrder.at(layoutFocus)].destWindowRect.x();

   for(QHash<int,  AnimData>::Iterator iter = anim.begin();
       iter != anim.end();
       ++iter)
       if(layoutOrder.contains(iter.key()))
           iter->destWindowRect.translate(delta, 0);

   animationTicker = 0;
}

void ScreenAnimator::moveRight()
{
    if(!shrunk || -1 != animationTicker) {
        return;
    }

    if(layoutFocus == (layoutOrder.count() - 1)) {
        return;
    }

    layoutFocus++;

    int delta = anim[layoutOrder.at(layoutFocus)].destWindowRect.x() - anim[layoutOrder.at(layoutFocus - 1)].destWindowRect.x();

    for(QHash<int,  AnimData>::Iterator iter = anim.begin();
            iter != anim.end();
            ++iter)
       if(layoutOrder.contains(iter.key()))
           iter->destWindowRect.translate(-delta, 0);

    animationTicker = 0;
}

void ScreenAnimator::expose()
{
    QRegion exposedRegion;
    QRegion totalRegion;

    ushort backbuffer[248];

    int reflectionLine = 240;
    int reflectionHeight = 54;
    QRect reflection(0, reflectionLine + 2, width, reflectionHeight);

    for(QHash<int, QRect>::ConstIterator iter = reserved.begin();
            iter != reserved.end();
            ++iter)
        exposedRegion = exposedRegion.united(*iter);

    for(int ii = 0; ii < windowOrder.count(); ++ii)
    {
        QHash<int, AWindow>::Iterator iter = windows.find(windowOrder.at(ii));
        if(iter == windows.end())
            continue;

        const AWindow &window = *iter;
        QRect window_screen_rect = window.screen;

        QHash<int, AnimData>::Iterator aiter = anim.find(window.winId);
        if(aiter != anim.end())
            window_screen_rect = aiter->windowRect;

        totalRegion = totalRegion.united(window_screen_rect);

        if(window.isReserved)
            exposedRegion = exposedRegion.united(window_screen_rect);
    }

    QRect reflectionBounds = reflection;
    reflectionBounds.moveBottom(reflectionLine);

    for(int ii = 0; ii < windowOrder.count(); ++ii)
    {
        QHash<int, AWindow>::Iterator iter = windows.find(windowOrder.at(ii));
        if(iter == windows.end()) {
            continue;
        }

        const AWindow &window = *iter;

        QRect window_screen_rect = window.screen;
        QRect window_image_rect = QRect(QPoint(0,0), window.image.size());
        QHash<int, AnimData>::Iterator aiter = anim.find(window.winId);
        if(aiter != anim.end()) {
            window_screen_rect = aiter->windowRect;
            window_image_rect = aiter->imageRect;
        }

        const QImage &window_image = window.image;
        QSize window_image_size = window_image_rect.size();
        bool scaled = window_screen_rect.size() != window_image_size;


        QRegion toDraw = QRegion(window_screen_rect.intersected(QRect(0,0,width,height))).subtracted(exposedRegion);
        exposedRegion = exposedRegion.united(toDraw);

        if(window.isReserved) {
            continue;
        }

        // Reflection stuff
        QRect reflectRect = window_screen_rect.intersected(reflectionBounds);
        if(!reflectRect.isEmpty()) {
            reflectRect.moveTop(reflectionLine +
                                (reflectionLine - reflectRect.bottom()) + 2);
        }

        QRegion reflectRegion =
            QRegion(reflectRect).subtracted(totalRegion).subtracted(exposedRegion).intersected(QRect(0,0,width,height));
        exposedRegion = exposedRegion.united(reflectRegion);

        QVector<QRect> reflectRects = reflectRegion.rects();
        for(int ii = 0; ii < reflectRects.count(); ++ii) {
            QRect &rect = reflectRects[ii];
            rect.moveBottom(reflectionLine - (rect.top() - reflectionLine - 2));
        }

        foreach(QRect screen_rect, toDraw.rects()) {
            QRect surface_rect;
            if(!scaled) {
                surface_rect =
                    screen_rect.translated(-window_screen_rect.topLeft());
            } else {
                int x = (window_image_size.width() * (screen_rect.x() - window_screen_rect.x())) / window_screen_rect.width();
                int y = (window_image_size.height() * (screen_rect.y() - window_screen_rect.y())) / window_screen_rect.height();
                int width = (window_image_size.width() * screen_rect.width()) / window_screen_rect.width();
                int height = (window_image_size.height() * screen_rect.height()) / window_screen_rect.height();

                surface_rect = QRect(x, y, width, height);
            }

            ushort *screen_buffer = (ushort *)(frameBuffer + screen_rect.y() * lineStep) + screen_rect.x();
            int screen_step = lineStep;

            int window_step = window_image.bytesPerLine();
            ushort * window_buffer =
                (ushort *)(window_image.bits() + (window_image_rect.y() + surface_rect.y()) * window_step) + window_image_rect.x() + surface_rect.x();
            ushort * window_bits = (ushort *)window_image.bits();


            if(screen_rect.size() == surface_rect.size()) {
                for(int ii = 0; ii < screen_rect.height(); ++ii) {
                    ::memcpy(screen_buffer,
                             window_buffer,
                             screen_rect.width() * sizeof(ushort));

                    window_buffer = (ushort *)((uchar *)window_buffer + window_step);
                    screen_buffer = (ushort *)((uchar *)screen_buffer + screen_step);
                }
            } else {
                // XXX - only supports down scaling
                Q_ASSERT(screen_rect.height() <= surface_rect.height());
                Q_ASSERT(screen_rect.width() <= surface_rect.width());

                bool scaled_width = screen_rect.width() != surface_rect.width();

                int image_yy = 0;
                for(int ii = 0; ii < screen_rect.height(); ++ii) {
                    ushort *true_screen_buffer = 0;
                    int true_yy = screen_rect.y() + ii;
                    if(true_yy >= reflectionBounds.top() &&
                       true_yy <= reflectionBounds.bottom()) {

                        true_screen_buffer = screen_buffer;
                        screen_buffer = backbuffer;
                    }

                    if(scaled_width) {
                        stripe(screen_buffer, screen_rect.width(),
                               window_bits, window_image.width(),
                               surface_rect.x() + window_image_rect.x(),
                               image_yy + surface_rect.y() + window_image_rect.y(),
                               surface_rect.x() + window_image_rect.x() + surface_rect.width(), image_yy + surface_rect.y() + window_image_rect.y());
                    } else {
                        if(true_screen_buffer)
                            screen_buffer = window_buffer;
                        else
                            ::memcpy(screen_buffer, window_buffer, screen_rect.width() * sizeof(ushort));
                    }

                    if(true_screen_buffer) {
                        ::memcpy(true_screen_buffer, screen_buffer, screen_rect.width() * sizeof(ushort));
                        for(int ii = 0; ii < reflectRects.count(); ++ii) {
                            const QRect &rect = reflectRects.at(ii);

                            if(rect.top() <= true_yy &&
                               rect.bottom() >= true_yy &&
                               screen_rect.left() < rect.right() &&
                               screen_rect.right() > rect.left()) {

                                int left = 0;
                                if(screen_rect.left() < rect.left())
                                    left = rect.left() - screen_rect.left();

                                int width = rect.width();
                                if(screen_rect.right() < rect.right())
                                    width = screen_rect.right() - rect.left();


                                int reflect_yy = (reflectionLine - true_yy) + reflectionLine + 2;

                                uchar alpha = (((reflect_yy - reflectionLine - 1) * 0xE0) / reflectionHeight);

//                                memcpy((ushort *)(frameBuffer + reflect_yy * lineStep + rect.x() * 2), screen_buffer + left, width * 2);
                                blend_rgb16_color(screen_buffer + left, alpha << 24, width, (ushort *)(frameBuffer + reflect_yy * lineStep + rect.x() * 2));

                            }
                        }
                        screen_buffer = true_screen_buffer;
                    }

                    while(image_yy * screen_rect.height() <=
                            ii * surface_rect.height()) {
                        image_yy++;
                        window_buffer = (ushort *)((uchar *)window_buffer + window_step);
                    }

                    screen_buffer = (ushort *)((uchar *)screen_buffer + screen_step);
                }
            }
        }
    }

    QRegion toDraw;

    // Fill the rest with black
    toDraw = QRegion(QRect(0,0,width,height)).subtracted(exposedRegion);
    foreach(QRect screen_rect, toDraw.rects()) {
        uchar *screen_buffer = frameBuffer + screen_rect.y() * lineStep + screen_rect.x() * 2;
        int screen_step = lineStep;
        for(int ii = 0; ii < screen_rect.height(); ++ii) {
            ::memset(screen_buffer, 0, screen_rect.width() * 2);
            screen_buffer += lineStep;
        }
    }
}

GreenphoneScreen::GreenphoneScreen(int displayId)
: QLinuxFbScreen(displayId), d(0)
{
    cookie = 0x91822282;
    d = new GreenphoneScreenPrivate();
}

GreenphoneScreen::~GreenphoneScreen()
{
    delete d;
}

// #define COMPOSE_DEBUGGING

void GreenphoneScreen::exposeRegion(QRegion region, int changing)
{
    struct timeval tv;
    ::gettimeofday(&tv, 0);

    if(!d->animator)
        d->animator = new ScreenAnimator(w, h, lstep, data);

    QWSWindow *changed = qwsServer->clientWindows().at(changing);
    QWSWindowSurface *surface = changed->windowSurface();

    if(!surface) {
        return;
    }

    if(surface->isRegionReserved()) {
        AWindow achanged;
        achanged.isReserved = true;
        achanged.winId = changed->winId();
        achanged.screen =  changed->requestedRegion().boundingRect();
        qWarning() << "Updating reserved" << achanged.winId << achanged.screen;

        d->animator->updateWindow(achanged);
        return;
    }

    QImage window_image = surface->image();
    if(window_image.format() != QImage::Format_RGB16) {
        AWindow achanged;
        achanged.isReserved = true;
        achanged.winId = changed->winId();
        achanged.screen =  changed->requestedRegion().boundingRect();
        qWarning() << "Updating reserved" << achanged.winId << achanged.screen;

        d->animator->updateWindow(achanged);
        return;
    }

    AWindow achanged;
    achanged.isReserved = false;
    achanged.winId = changed->winId();
    achanged.image = window_image.copy();

    achanged.screen =  changed->requestedRegion().boundingRect();

    d->animator->updateWindow(achanged);
}

#include "greenphonescreen.moc"
