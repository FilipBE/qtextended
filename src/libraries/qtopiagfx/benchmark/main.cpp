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
#include <QDebug>
#include <gfximage.h>
#include <routines.h>
#include <QTimer>
#include <QTime>
#include <QObject>
#include <QDebug>
#include <gfxpainter.h>
#include <gfx.h>

#define BENCHMARK_TIME 3000

GfxPainter *gfxPainter = 0;
QString benchmark;
bool includeSmall = false;
QImage::Format srcFormat = QImage::Format_RGB16;

class GfxBenchmarks : public QObject
{
Q_OBJECT
public:
    GfxBenchmarks()
    {
    }

public slots:
    void runBenchmarks()
    {
        if(benchmark.isEmpty() || "blit" == benchmark) blitBenchmark();
        if(benchmark.isEmpty() || "blur" == benchmark) blurBenchmark();
        if(benchmark.isEmpty() || "memcpy" == benchmark) memcpyBenchmark();
        if(benchmark.isEmpty() || "transformedBlit" == benchmark) rotateBenchmark();
        if(benchmark.isEmpty() || "fill" == benchmark) fillBenchmark();
        if(benchmark.isEmpty() || "transformedFill" == benchmark) fillTransformedBenchmark();
    }

private:

    void rotateBenchmark()
    {
        struct {
            int size;
            float rotation;
            const char *name;
        } images[] = {
            { 10, 5, "10x10, 5 degrees" },
            { 10, 30, "10x10, 30 degrees" },
            { 10, 45, "10x10, 45 degrees" },
            { 10, 120, "10x10, 120 degrees" },

            { 150, 5, "150x150, 5 degrees" },
            { 150, 30, "150x150, 30 degrees" },
            { 150, 45, "150x150, 45 degrees" },
            { 150, 120, "150x150, 120 degrees" },

            { 250, 5, "250x250, 5 degrees" },
            { 250, 30, "250x250, 30 degrees" },
            { 250, 45, "250x250, 45 degrees" },
            { 250, 120, "250x250, 120 degrees" }
        };

        for(unsigned int ii = 0; ii < sizeof(images) / sizeof(images[0]); ++ii) {
            if(!includeSmall && images[ii].size <= 10)
                continue;

            QImage img(images[ii].size, images[ii].size, srcFormat);

            QMatrix m;
            m.translate(images[ii].size / 2, images[ii].size / 2);
            m.rotate(images[ii].rotation);
            m.translate(-images[ii].size / 2, -images[ii].size / 2);

            transform(img, m, images[ii].name);
            transform_smooth(img, m, QString(images[ii].name) + " (smooth)");
        }

    }

    void fillTransformedBenchmark()
    {
        struct {
            int size;
            float rotation;
            uchar alpha;
            const char *name;
        } images[] = {
            { 10, 5, 0x70, "10x10, 0x70, 5 degrees" },
            { 10, 5, 0xFF, "10x10, 0xFF, 5 degrees" },
            { 10, 30, 0x70, "10x10, 0x70, 30 degrees" },
            { 10, 30, 0xFF, "10x10, 0xFF, 30 degrees" },
            { 10, 45, 0x70, "10x10, 0x70, 45 degrees" },
            { 10, 45, 0xFF, "10x10, 0xFF, 45 degrees" },
            { 10, 120, 0x70, "10x10, 0x70, 120 degrees" },
            { 10, 120, 0xFF, "10x10, 0xFF, 120 degrees" },

            { 150, 5, 0x70, "150x150, 0x70, 5 degrees" },
            { 150, 5, 0xFF, "150x150, 0xFF, 5 degrees" },
            { 150, 30, 0x70, "150x150, 0x70, 30 degrees" },
            { 150, 30, 0xFF, "150x150, 0xFF, 30 degrees" },
            { 150, 45, 0x70, "150x150, 0x70, 45 degrees" },
            { 150, 45, 0xFF, "150x150, 0xFF, 45 degrees" },
            { 150, 120, 0x70, "150x150, 0x70, 120 degrees" },
            { 150, 120, 0xFF, "150x150, 0xFF, 120 degrees" },

            { 250, 5, 0x70, "250x250, 0x70, 5 degrees" },
            { 250, 5, 0xFF, "250x250, 0xFF, 5 degrees" },
            { 250, 30, 0x70, "250x250, 0x70, 30 degrees" },
            { 250, 30, 0xFF, "250x250, 0xFF, 30 degrees" },
            { 250, 45, 0x70, "250x250, 0x70, 45 degrees" },
            { 250, 45, 0xFF, "250x250, 0xFF, 45 degrees" },
            { 250, 120, 0x70, "250x250, 0x70, 120 degrees" },
            { 250, 120, 0xFF, "250x250, 0xFF, 120 degrees" }
        };

        for(unsigned int ii = 0; ii < sizeof(images) / sizeof(images[0]); ++ii) {
            if(!includeSmall && images[ii].size <= 10)
                continue;

            int size = images[ii].size;

            QMatrix m;
            m.translate(size / 2, size / 2);
            m.rotate(images[ii].rotation);
            m.translate(-size / 2, -size / 2);

            QColor c = QColor::fromRgba(images[ii].alpha << 24);
            fillTransformed(QSize(size, size), m, c, images[ii].name);
            fillTransformed_smooth(QSize(size, size), m, c, QString(images[ii].name) + " (smooth)");
        }

    }

    void fillBenchmark()
    {
        struct {
            int size;
            int alpha;
            const char *name;
        } images[] = {
            { 2, 0, "2x2, 0x00 fill" },
            { 2, 0x77, "2x2, 0x77 fill" },
            { 2, 0xFF, "2x2, 0xFF fill" },
            { 10, 0, "10x10, 0x00 fill" },
            { 10, 0x77, "10x10, 0x77 fill" },
            { 10, 0xFF, "10x10, 0xFF fill" },
            { 150, 0, "150x150, 0x00 fill" },
            { 150, 0x77, "150x150, 0x77 fill" },
            { 150, 0xFF, "150x150, 0xFF fill" },
            { 320, 0, "320x320, 0x00 fill" },
            { 320, 0x77, "320x320, 0x77 fill" },
            { 320, 0xFF, "320x320, 0xFF fill" },
        };

        for(unsigned int ii = 0; ii < sizeof(images) / sizeof(images[0]); ++ii) {
            if(!includeSmall && images[ii].size <= 10)
                continue;

            fill(images[ii].size, images[ii].alpha << 24, images[ii].name);
        }
    }

    void blitBenchmark()
    {
        struct {
            int size;
            int alpha;
            const char *name;
        } images[] = {
            { 2, 0, "2x2, 0x00 blit" },
            { 2, 0x77, "2x2, 0x77 blit" },
            { 2, 0xFF, "2x2, 0xFF blit" },
            { 10, 0, "10x10, 0x00 blit" },
            { 10, 0x77, "10x10, 0x77 blit" },
            { 10, 0xFF, "10x10, 0xFF blit" },
            { 150, 0, "150x150, 0x00 blit" },
            { 150, 0x77, "150x150, 0x77 blit" },
            { 150, 0xFF, "150x150, 0xFF blit" },
            { 320, 0, "320x320, 0x00 blit" },
            { 320, 0x77, "320x320, 0x77 blit" },
            { 320, 0xFF, "320x320, 0xFF blit" },
        };

        for(unsigned int ii = 0; ii < sizeof(images) / sizeof(images[0]); ++ii) {
            if(!includeSmall && images[ii].size <= 10)
                continue;
            if(srcFormat == QImage::Format_RGB16 && images[ii].alpha != 0xFF)
                continue;

            QImage img(images[ii].size, images[ii].size, srcFormat);
            img.fill(images[ii].alpha << 24);

            blit(img, images[ii].name);
            unalignedBlit(img, QString(images[ii].name) + " (unaligned)");
            subPixelBlit(img, QString(images[ii].name) + " (sub-pixel)");
        }
    }


    void blurBenchmark()
    {
        int sizes[] = { 2, 10, 150, 320 };
        for(unsigned int ii = 0; ii < sizeof(sizes) / sizeof(sizes[0]); ++ii) {
            if(!includeSmall && sizes[ii] <= 10)
                continue;
            QImage img32(sizes[ii], sizes[ii], QImage::Format_RGB32);
            QImage img16(sizes[ii], sizes[ii], QImage::Format_RGB16);

            QString size = QString::number(sizes[ii]);

            blur(img32, size + "x" + size + ", 32-bit");
            blur(img16, size + "x" + size + ", 16-bit");
        }
    }

    void memcpyBenchmark()
    {
        int sizes[] = { 2, 10, 150, 320, 10000, 100000, 1000000 };

        for(unsigned int ii = 0; ii < sizeof(sizes) / sizeof(sizes[0]); ++ii) {
            if(!includeSmall && sizes[ii] <= 10)
                continue;
            char *dest = (char *)malloc(sizes[ii] + 1);
            char *src = (char *)malloc(sizes[ii] + 1);

            QString size = QString::number(sizes[ii]);

            memcpy(size + " bytes memcpy", dest, src, sizes[ii]);
            memcpy(size + " bytes unaligned memcpy", dest, src + 1, sizes[ii]);

            free(dest);
            free(src);
        }
    }


    void memcpy(const QString &name, char *dest, char *src, int len)
    {
        QTime t;
        t.start();
        unsigned int frames = 0;
        while(true) {
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            q_memoryroutines.memcpy((uchar *)dest, (uchar *)src, len);
            frames += 16;

            if(t.elapsed() >= BENCHMARK_TIME) {
               int e = t.elapsed(); 

               qWarning().nospace() << name << ": " << 1000. * (qreal)frames / (qreal)e << " copy/sec, " << 1000. * (((qreal)(len * frames)) / (qreal)(1024 * 1024)) / (qreal)e << " MB/sec";
               return;
            }
        }
    }

    void blur(QImage &img, const QString &name)
    {
        QTime t;
        t.start();
        unsigned int frames = 0;
        while(true) {
            ++frames;
            Gfx::blur(img, 10.);

            if(t.elapsed() >= BENCHMARK_TIME) {
               int e = t.elapsed(); 

               qWarning().nospace() << name << ": " << 1000. * (qreal)frames / (qreal)e << " fps, " << (qreal)e / (qreal)frames << " ms/frame";
               return;
            }
        }
    }

    void fill(int size, unsigned int color, const QString &name)
    {
        QTime t;
        t.start();
        unsigned int frames = 0;
        QColor c = QColor::fromRgba(color);
        while(true) {
            ++frames;
            gfxPainter->fillRect(QRect(0, 0, size, size), c);

            if(t.elapsed() >= BENCHMARK_TIME) {
               int e = t.elapsed(); 

               qWarning().nospace() << name << ": " << 1000. * (qreal)frames / (qreal)e << " fps, " << (qreal)e / (qreal)frames << " ms/frame";
               return;
            }
        }
    }

    void subPixelBlit(const QImage &img, const QString &name)
    {
        QTime t;
        t.start();
        unsigned int frames = 0;
        while(true) {
            ++frames;
            gfxPainter->drawImage(0.3f, 0, img);

            if(t.elapsed() >= BENCHMARK_TIME) {
               int e = t.elapsed(); 

               qWarning().nospace() << name << ": " << 1000. * (qreal)frames / (qreal)e << " fps, " << (qreal)e / (qreal)frames << " ms/frame";
               return;
            }
        }
    }


    void unalignedBlit(const QImage &img, const QString &name)
    {
        QTime t;
        t.start();
        unsigned int frames = 0;
        while(true) {
            ++frames;
            gfxPainter->drawImage(1, 0, img);

            if(t.elapsed() >= BENCHMARK_TIME) {
               int e = t.elapsed(); 

               qWarning().nospace() << name << ": " << 1000. * (qreal)frames / (qreal)e << " fps, " << (qreal)e / (qreal)frames << " ms/frame";
               return;
            }
        }
    }

    void fillTransformed(const QSize &size, const QMatrix &m, const QColor &c, const QString &name)
    {
        QTime t;
        t.start();
        unsigned int frames = 0;
        while(true) {
            ++frames;
            gfxPainter->fillRectTransformed(m, size, c, false);

            if(t.elapsed() >= BENCHMARK_TIME) {
               int e = t.elapsed(); 

               qWarning().nospace() << name << ": " << 1000. * (qreal)frames / (qreal)e << " fps, " << (qreal)e / (qreal)frames << " ms/frame";
               return;
            }
        }
    }
    
    void fillTransformed_smooth(const QSize &size, const QMatrix &m, const QColor &c, const QString &name)
    {
        QTime t;
        t.start();
        unsigned int frames = 0;
        while(true) {
            ++frames;
            gfxPainter->fillRectTransformed(m, size, c, true);

            if(t.elapsed() >= BENCHMARK_TIME) {
               int e = t.elapsed(); 

               qWarning().nospace() << name << ": " << 1000. * (qreal)frames / (qreal)e << " fps, " << (qreal)e / (qreal)frames << " ms/frame";
               return;
            }
        }
    }

    void transform(const QImage &img, const QMatrix &m, const QString &name)
    {
        QTime t;
        t.start();
        unsigned int frames = 0;
        while(true) {
            ++frames;
            gfxPainter->drawImageTransformed(m, img, false);

            if(t.elapsed() >= BENCHMARK_TIME) {
               int e = t.elapsed(); 

               qWarning().nospace() << name << ": " << 1000. * (qreal)frames / (qreal)e << " fps, " << (qreal)e / (qreal)frames << " ms/frame";
               return;
            }
        }
    }

    void transform_smooth(const QImage &img, const QMatrix &m, 
                          const QString &name)
    {
        QTime t;
        t.start();
        unsigned int frames = 0;
        while(true) {
            ++frames;
            gfxPainter->drawImageTransformed(m, img, true);

            if(t.elapsed() >= BENCHMARK_TIME) {
               int e = t.elapsed(); 

               qWarning().nospace() << name << ": " << 1000. * (qreal)frames / (qreal)e << " fps, " << (qreal)e / (qreal)frames << " ms/frame";
               return;
            }
        }
    }

    void blit(const QImage &img, const QString &name)
    {
        QTime t;
        t.start();
        unsigned int frames = 0;
        while(true) {
            ++frames;
            gfxPainter->drawImage(0, 0, img);

            if(t.elapsed() >= BENCHMARK_TIME) {
               int e = t.elapsed(); 

               qWarning().nospace() << name << ": " << 1000. * (qreal)frames / (qreal)e << " fps, " << (qreal)e / (qreal)frames << " ms/frame";
               return;
            }
        }
    }
};


void help(char *name)
{
    qWarning() << name << ": [-32] [-16] [-src32] [-src16] [-src32p] [-small] [test]";
    qWarning() << "     blit";
    qWarning() << "     blur";
    qWarning() << "     memcpy";
    qWarning() << "     transformedBlit";
    qWarning() << "     fill";
    qWarning() << "     transformedFill";
    exit(-1);
}

int main(int argc, char ** argv)
{
    QApplication app(argc, argv, QApplication::GuiServer);

    Gfx::init();

    bool depth32 = false;
    for(int ii = 1; ii < argc; ++ii) {
        if(0 == ::strcmp(argv[ii], "-help")) 
            help(argv[0]);
        else if(0 == ::strcmp(argv[ii], "-32")) 
            depth32 = true;
        else if(0 == ::strcmp(argv[ii], "-16")) 
            depth32 = false;
        else if(0 == ::strcmp(argv[ii], "-src16")) 
            srcFormat = QImage::Format_RGB16;
        else if(0 == ::strcmp(argv[ii], "-src32")) 
            srcFormat = QImage::Format_RGB32;
        else if(0 == ::strcmp(argv[ii], "-src32p")) 
            srcFormat = QImage::Format_ARGB32_Premultiplied;
        else if(0 == ::strcmp(argv[ii], "-small")) 
            includeSmall = true;
        else
            benchmark = argv[ii];
    }

    QImage img(240, 320, depth32?QImage::Format_RGB32:QImage::Format_RGB16);
    gfxPainter = new GfxPainter(img);;

    GfxBenchmarks benchmarks;
    benchmarks.runBenchmarks();
}


#include "main.moc"
