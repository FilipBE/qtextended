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

#include <QApplication>
#include <gfximage.h>
#include <QMutex>
#include <routines.h>
#include <QTimer>
#include <QKeyEvent>
#include <QFontMetrics>
#include <unistd.h>
#include <QPaintEngine>
#include <QDebug>
#include <QList>
#include <QPainter>
#include <QWidget>
#include <QThread>
#include <QPoint>
#include <gfx.h>
#include <QLinearGradient>
#include <QTime>
#include <QWaitCondition>
#include <QMutex>
#include <gfxpainter.h>
#include <gfxparticles.h>

extern "C" {
void tint(unsigned short *img, int width, int height, int stepwidth,
          unsigned char *color, int bpl, unsigned int blendcolor,
          unsigned int blendoffset);
};

static int stars = 1000;

class MyWidget;
class MyThread : public QThread
{
public:
    MyThread(MyWidget *w);

    virtual void run();
    MyWidget *_w;
};

class MyWidget : public QWidget
{
public:
	MyWidget()
            : QWidget(0, Qt::FramelessWindowHint), frames(0), _animOffsetColor(0), menuItem(0), menuPos(0, 0), menuDestPos(0, 0)
	{
            _img = QImage("testimage.png");
            _back = QImage(240, 320, QImage::Format_RGB16);
            _backImg = QImage("backimage.png");
            _backImg = _backImg.scaled(240, 320);
            _backImg = _backImg.convertToFormat(QImage::Format_RGB16);

            notSupported = QImage(240, 160, QImage::Format_ARGB32_Premultiplied);
            notSupported.fill(0);
            {
                QPainter np(&notSupported);
                np.setBrush(QColor(0, 0, 0, 0x30));
                np.setPen(Qt::white);
                np.setRenderHint(QPainter::Antialiasing);
                np.drawRoundRect(0, 0, notSupported.width() - 1,
                                notSupported.height() - 1,
                                800/notSupported.width(),800/notSupported.height());
#if 0
                QFont f;
                f.setBold(true);
                f.setPointSize(18);
                np.setFont(f);
                np.drawText(notSupported.rect(), Qt::AlignVCenter | Qt::AlignCenter, "Context Menu");
#endif
            }
            aNot = new GfxImage(notSupported);

            {
                static const char *items[] = {
                    "1. View",
                    "2. Edit",
                    "3. Delete",
                    "4. Mail to...",
                };
                QFont f;
                f.setBold(true);
                f.setPointSize(18);
                QFontMetrics fm(f);
                for(int ii = 0; ii < sizeof(items) / sizeof(items[0]); ++ii) {
                    QRect r = fm.boundingRect(items[ii]);
                    r.setWidth(r.width() + 10);
                    QImage pix(r.size(), QImage::Format_ARGB32_Premultiplied);
                    pix.fill(0);
                    QPainter p(&pix);
                    p.setPen(Qt::white);
                    p.setFont(f);
                    p.drawText(pix.rect(), Qt::AlignCenter | Qt::AlignVCenter, items[ii]);

                    menuItems << new GfxImage(pix);
                }

                f = QFont();
                fm = QFontMetrics(f);
                QRect fpsRect = fm.boundingRect("000");
                fpsImage = QImage(fpsRect.size(), QImage::Format_ARGB32_Premultiplied);
                fpsImage.fill(0);

                QImage high(236, 36, QImage::Format_ARGB32_Premultiplied);
                high.fill(0);
                QPainter p(&high);
                p.setBrush(QColor(0, 0, 127, 127));
                p.setRenderHint(QPainter::Antialiasing);
                p.drawRoundRect(0, 0, high.width() - 1,
                                high.height() - 1,
                                800/high.width(),800/high.height());
                menuHighlight = new GfxImage(high);
            }

            _color = 0xFF800000;
            _offset = 0x00200020;
            _selection = 0;
            _animOffset = 0;

            _blurState = UnBlurred;
            _imgState = Tint;
            _blurOffset = 0;
            _imgOffset = 0;

                QPalette pal;
                pal.setColor(backgroundRole(), Qt::black);
                setPalette(pal);


                static const char *strs[] = {
                    "Robert Menzies",
                    "John Gorton",
                    "William McMahon",
                    "Gough Whitlam",
                    "Bob Hawke",
                    "Paul Keating",
                    "John Howard",
                    "Kevin Rudd"
                };
                QFont f;
                f.setBold(true);
                f.setPointSize(16);
                QFontMetrics fm(f);
                for(int ii = 0; ii < 8; ++ii) {
                    QRect r = fm.boundingRect(strs[ii]);
                    r.setWidth(r.width() + 10);
                    QImage pix(r.size(), QImage::Format_ARGB32_Premultiplied);
                    qWarning() << r.size();

                    pix.fill(0);

                    QPainter p(&pix);
                    //p.fillRect(pix.rect(), Qt::black);
                    p.setBrush(QColor(0, 0, 0, 0x80));
                    p.setPen(Qt::NoPen);
                    p.setRenderHint(QPainter::Antialiasing);
                    p.drawRoundRect(0, 0, pix.width() - 1, pix.height() - 1,
                            800/pix.width(),800/pix.height());
                    p.setFont(f);
                    p.setPen(Qt::white);
                    p.drawText(pix.rect(), Qt::AlignCenter | Qt::AlignVCenter, strs[ii]);

                    names << pix;
                    gnames << new GfxImage(pix);
                }
                part.loadSimpleStars(10);

                time.start();

                part.setParticles(100);
                part.setRect(QRect(0, -5, 240, 50));

                new MyThread(this);
	}

        void runThread()
        {
            m_lock.lock();
            p.setRect(QRect(0, 0, 240, 320));
            while(true) {
                go(p);
                m_lock.unlock();
                m_lock.lock();
            }
        }

        virtual void keyPressEvent(QKeyEvent *e)
        {
            if(e->key() == Qt::Key_Up) {
                if(Blurred == _blurState) {
                    if(menuItem) {
                        --menuItem;
                        menuDestPos = QPoint(0, 40 * menuItem);
                    }
                } else {
                    if(_selection) {
                        --_selection;
                        part.moveToRect(QRect(0, _selection * 40 - 5, 240, 50));
                    }
                }
            } else if(e->key() == Qt::Key_Down) {
                if(Blurred == _blurState) {
                    if(menuItem < 3) {
                        ++menuItem;
                        menuDestPos = QPoint(0, 40 * menuItem);
                    }
                } else {
                    if(_selection < 7) {
                        ++_selection;
                        part.moveToRect(QRect(0, _selection * 40 - 5, 240, 50));
                    }
                }
            } else if(e->key() == Qt::Key_Right) {
//                _offset -= 0x00010001;
 //               printf("%x\n", _offset);
                if(_imgState != Img) {
                    _imgState = Img;
                    _imgOffset = _animOffset;
                }
            } else if(e->key() == Qt::Key_Left) {
  //              _offset += 0x00010001;
   //             printf("%x\n", _offset);
                if(_imgState != Tint) {
                    _imgState = Tint;
                    _imgOffset = _animOffset;
                }
            } else if(e->key() == Qt::Key_2) {
                _color += 0x00010000;
                printf("%x\n", _color);
            } else if(e->key() == Qt::Key_0) {
                _color -= 0x00010000;
                printf("%x\n", _color);
            } else if(e->key() == Qt::Key_Context1 ||
                      (Blurred == _blurState && e->key() == Qt::Key_Back)) {
                if(_blurState == Blurred) {
                    _blurState = UnBlurred;
                    part.setParticles(100);
                } else {
                    m_lock.lock();
                    tint(_animOffsetColor % (_img.width() - 240));
                    ::memcpy(_back.bits(), p.backBuffer(), 240 * 320 * 2);
                    _blurState = Blurred;
                    part.setParticles(0);
                    m_lock.unlock();
                }
                _blurOffset = _animOffset;
            } else if(e->key() == Qt::Key_Back) {
                qApp->quit();
            }
        }

        void go(GfxPainter &p)
        {
            frames++;

            static int inc = 1;
            _animOffset++;

            if((_color & 0xFF) == 0x00)
                inc = 1;
            else if((_color & 0xFF) == 0x80)
                inc = -1;

#if 1
#if 0
            if(UnBlurred == _blurState && _animOffset > (_blurOffset + 12)) {
                _color += inc;
                tint(_animOffsetColor % (_img.width() - 240));
                ++_animOffsetColor;
            } else {
                QRect r = _backImg.rect().translated(-(_animOffset % 240), 0);
                p.copy(r, _backImg);
                r = r.translated(240, 0);
                p.copy(r, _backImg);
            }
#endif

            if(_imgState == Tint && _animOffset < (_imgOffset + 12)) {
                _color += inc;

                QRect r((240 * (_animOffset - _imgOffset)) / 12, 0, 240, 320);

                tint(_animOffsetColor % (_img.width() - 240), r.x() - 240);
                p.blit(r.topLeft(), _backImg);
                ++_animOffsetColor;

            } else if(_imgState == Img && _animOffset < (_imgOffset + 12)) {
                _color += inc;
                QRect r(240 - (240 * (_animOffset - _imgOffset)) / 12, 0, 240, 320);
                tint(_animOffsetColor % (_img.width() - 240), r.x() - 240);
                p.blit(r.topLeft(), _backImg);
                ++_animOffsetColor;
            } else if(_imgState == Img) {
                p.blit(0, 0, _backImg);
            }  else if(Tint == _imgState) {
                _color += inc;
                tint(_animOffsetColor % (_img.width() - 240));
                ++_animOffsetColor;
            }
#else
            //p.clear();
//            p.copy(_backImg.rect(), _backImg);
#endif

#if 1
            part.paint(p);

            p.setOpacity(1.);
            for(int ii = 0; ii < names.count(); ++ii) {
                const QImage &img = names.at(ii);

                QRect imgRect((240 - img.width()) / 2, ii * 40 + (40 - img.height()) / 2, img.width(), img.height());

                // p.blit(imgRect, img);
                p.blit(imgRect.topLeft(), *gnames.at(ii));
            }

            qreal blur = -1.0f;
            QRect blurrect;
            qreal fade = 0.0f;

            int nsh = notSupported.height();
            int nsw = notSupported.width();

            if(UnBlurred == _blurState && _animOffset > (_blurOffset + 24)) {
                // Do nothing
            } else if(UnBlurred == _blurState) {
                blur = 24.0f - 24.0f * ((_animOffset - _blurOffset) / 6.0f);
                fade = blur / 24.0f;
                qreal pos = 320. - (qreal)nsh * blur / 24.0f;
                blur -= 1.0f;
                blurrect = QRect(0, pos, nsw, nsh);
            } else if(Blurred == _blurState && _animOffset > (_blurOffset + 6)) {
                blur = 24.0f;
                fade = 1.0f;
                qreal pos = 320. - (qreal)nsh;
                blur -= 1.0f;
                blurrect = QRect(0, pos, nsw, nsh);

            } else {
                blur = 24.0f * ((_animOffset - _blurOffset) / 6.0f);
                fade = blur / 24.0f;
                qreal pos = 320. - (qreal)nsh * blur / 24.0f;
                blur -= 1.0f;
                blurrect = QRect(0, pos, nsw, nsh);
            }

            if(0.0f != fade) {

                //QImage back(p.backBuffer(), 240, 320, QImage::Format_RGB16);
                QImage back(p.backBuffer() + blurrect.y() * 240 * 2, 240, 320 - blurrect.y(), QImage::Format_RGB16);

                p.setOpacity(fade);
                Gfx::blur(back, blur);
                // p.blit(blurrect, notSupported);
                p.blit(blurrect.topLeft(), *aNot);

                if(menuDestPos != menuPos) {
                    if(menuDestPos.y() > menuPos.y())
                        menuPos += QPoint(0, 4);
                    else
                        menuPos -= QPoint(0, 4);
                }
                p.blit(2, blurrect.y() + 2 + menuPos.y(), *menuHighlight);
                for(int ii = 0; ii < menuItems.count(); ++ii) {
                    GfxImage *img = menuItems.at(ii);
                    QRect r = img->rect().translated(0, blurrect.y() + 40 * ii + (40 - img->height()) / 2);
                    p.blit(r.topLeft(), *img);
                }
            }

            part.advance();

            p.setOpacity(1.);
            p.blit(240 - fpsImage.width() - 10, 320 - fpsImage.height() - 10, fpsImage);

#endif
            p.flip();

            if(time.elapsed() > 1000) {
                int fps = (int)(1000.0f * (qreal)frames / (qreal)time.elapsed());
                qreal frametime = (qreal)time.elapsed() / (qreal)frames;
                qWarning() << stars << fps << frametime << "(direct)";
                time.restart();
                frames = 0;

                fpsImage.fill(0);
                QPainter p(&fpsImage);
                p.drawText(fpsImage.rect(), Qt::AlignVCenter | Qt::AlignCenter, QString::number(fps));
            }

        }

        void tint(int offset, int placement = 0)
        {
            ushort *newbits = (ushort *)(p.backBuffer());
            int height = 320;
            int width = 240;
            uchar *origbits = _img.bits() + offset;
            int bpl = _img.bytesPerLine();
            if(placement < -239)
                return;
            width += placement;
            origbits -= placement;

            ::tint(newbits, width, height, 240, origbits, bpl, _color, _offset);
        }

private:
        QList<QImage> names;
        QList<GfxImage *> gnames;
        QList<GfxImage *> menuItems;
        int menuItem;
        QPoint menuPos;
        QPoint menuDestPos;
        QImage fpsImage;

        GfxImage *menuHighlight;
        QImage notSupported;
        QImage _backImg;
        GfxImage *aNot;

        QImage _img;
        QImage _back;
        unsigned int _animOffsetColor;
        unsigned int _animOffset;
        unsigned int _color;
        unsigned int _offset;
        unsigned int _selection;

        enum { Blurred, UnBlurred } _blurState;
        unsigned int _blurOffset;

        enum { Img, Tint } _imgState;
        unsigned int _imgOffset;

        GfxPainter p;
        GfxParticles part;

        QTime time;
        int frames;

        QMutex m_lock;
        QWaitCondition m_wait;
};


MyThread::MyThread(MyWidget *w)
    : _w(w)
{
#ifndef NO_EMBEDDED
    start();
#endif
}

void MyThread::run()
{
    _w->runThread();
}

int main(int argc, char ** argv)
{
    Gfx::init();
    if(argc > 1) {
        uint *src = new uint[120 * 320 + 1];
        uint *src32 = new uint[240 * 320];
        uint *dest = new uint[120 * 320 + 1];
        uint *alpha = new uint[240 * 80];
#define VALUE 0x77
        ::memset(alpha, VALUE, 240 * 320);
        ::memset(src32, VALUE, 240 * 320 * 4);

        QTime t;
        t.start();
        for(int ii = 0; ii < 1000000; ++ii) {
            // XXX - Insert timing function
            q_memoryroutines.memcpy((uchar *)dest,(uchar *)src, 500);
        }
        unsigned int e = t.elapsed();
        qWarning() << e;

        return 0;
    }


    QApplication app(argc, argv, QApplication::GuiServer);
    MyWidget wid;
    wid.setFixedSize(240, 320);
#ifdef NO_EMBEDDED
        wid.show();
#else
        wid.showMaximized();
#endif

	return app.exec();
}
