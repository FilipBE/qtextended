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

#ifndef WHEELBROWSER_H
#define WHEELBROWSER_H

#include "qabstractbrowserscreen.h"
#include "qpixmapwheel.h"
class QSettings;
class QPixmapWheel;
class QPixmapWheelData;
class QTimeLine;
class QContent;

class WheelBrowserScreen : public QAbstractBrowserScreen
{
Q_OBJECT
public:
    WheelBrowserScreen(QWidget *parent = 0, Qt::WFlags flags = 0);

    virtual QString currentView() const;
    virtual bool viewAvailable(const QString &) const;

    virtual void resetToView(const QString &);
    virtual void moveToView(const QString &);

protected:
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void paintEvent(QPaintEvent *);

private slots:
    void moveToCompleted();
    void timelineStep(qreal);
    void clicked(const QString &);

private:
    QPixmapWheelData getData(const QString &entry);
    QContent *readLauncherMenuItem(const QString &entry);
    void doHide();

    unsigned char m_fillAlpha;
    bool m_hiding;
    QPixmapWheel *m_wheel;
    QPixmapWheelData *m_data;
    QTimeLine *m_fillTimeline;
    QStringList m_views;
};

#endif
