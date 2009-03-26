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

#ifndef THEMEBACKGROUND_P_H
#define THEMEBACKGROUND_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <themedview.h>
#include <qdrmcontent.h>
#include <QPixmap>
// This class must be visible to homescreen.cpp and secondarythemeddisplay.cpp
#include <private/themedviewinterface_p.h>

class QPainter;
class ThemedView;

class ThemeBackground : public ThemedItemPlugin
{
    Q_OBJECT
public:
    ThemeBackground(QObject *object = 0);
    ThemeBackground(ThemedView *themedView = 0);
    virtual ~ThemeBackground();

    void updateBackground();
    void resize(int w, int h);
    void paint(QPainter *p, const QRect &r);

private:
    QPixmap bg;
    ThemedView *themedView;
};

class HomeScreenImagePlugin : public ThemedItemPlugin
{
    Q_OBJECT
public:
    HomeScreenImagePlugin(ThemedView *themedView = 0);
    virtual ~HomeScreenImagePlugin() {}

    void resize(int w, int h);
    void paint(QPainter *p, const QRect &r);

    enum DisplayMode { ScaleAndCrop, Stretch, Tile, Center, Scale };

private:
    void renderSvg(int width, int height, Qt::AspectRatioMode mode);

private slots:
    void rightsExpired( const QDrmContent &content );

private:
    QDrmContent imgContent;
    QString imgName;
    QPixmap bg;
    int width;
    int height;
    DisplayMode dpMode;
    ulong ref;
    ThemedView *themedView;
};

#endif
