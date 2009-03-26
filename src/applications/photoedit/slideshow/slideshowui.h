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

#ifndef SLIDESHOWUI_H
#define SLIDESHOWUI_H

#include <qcontent.h>

#include <qwidget.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qstring.h>

class ImageScaler;

class SlideShowUI : public QWidget {
    Q_OBJECT
public:
    SlideShowUI(ImageScaler *scaler, QWidget* parent = 0, Qt::WFlags f = 0 );

signals:
    // Stylus pressed
    void pressed();

public slots:
    // If true, display name of image
    void setDisplayName( bool b ) { display_name = b; }

protected:
    // Draw scaled image onto widget
    void paintEvent( QPaintEvent* );

    void keyPressEvent( QKeyEvent* );

    // Transform stylus presses into signals
    void mousePressEvent( QMouseEvent* );

private slots:
    void imageChanged();

private:
    ImageScaler *m_scaler;
    QContent image;

    bool display_name;
};

#endif
