/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
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

#ifndef QVFBX11VIEW_H
#define QVFBX11VIEW_H

#include "qvfbview.h"

QT_BEGIN_NAMESPACE

class X11KeyFaker;
class QProcess;
class QTemporaryFile;

class QVFbX11View : public QVFbAbstractView
{
    Q_OBJECT
public:
    QVFbX11View( int id, int w, int h, int d, Rotation r, QWidget *parent = 0);
    virtual ~QVFbX11View();

    QString xServerPath() const { return xserver; }
    void setXServerPath(const QString& path) { xserver = path; }

    int displayId() const;
    int displayWidth() const;
    int displayHeight() const;
    int displayDepth() const;
    Rotation displayRotation() const;

    void skinKeyPressEvent( int code, const QString& text, bool autorep=FALSE );
    void skinKeyReleaseEvent( int code, const QString& text, bool autorep=FALSE );

    void setGamma(double gr, double gg, double gb);
    double gammaRed() const;
    double gammaGreen() const;
    double gammaBlue() const;
    void getGamma(int i, QRgb& rgb);

    bool touchScreenEmulation() const;
    bool lcdScreenEmulation() const;
    int rate();
    bool animating() const;
    QImage image() const;
    void setRate(int);

    double zoomH() const;
    double zoomV() const;

    QSize sizeHint() const;

public slots:
    void setTouchscreenEmulation( bool );
    void setLcdScreenEmulation( bool );
    void setZoom( double, double );
    void setRotation(Rotation);
    void startAnimation( const QString& );
    void stopAnimation();

protected:
    void showEvent(QShowEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

private slots:
    void startXnest();
    void xnestStopped();
    void startKeyFaker();

private:
    int id, w, h, d;
    Rotation rotation;
    double gr, gg, gb;
    bool touchscreen, lcd;
    X11KeyFaker *keyFaker;
    QProcess *xnest;
    QTemporaryFile *serverAuthFile;
    bool shutdown;
    QString xserver;
};

QT_END_NAMESPACE

#endif
