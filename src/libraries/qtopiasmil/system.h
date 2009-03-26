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

#ifndef SMILSYSTEM_H
#define SMILSYSTEM_H

#include <qobject.h>
#include <qstring.h>
#include <qmap.h>
#include <qpixmap.h>
#include <QRegion>

class SmilTransferServer;
class SmilModule;
class SmilElement;
class QTimer;

class SmilSystem : public QObject
{
    Q_OBJECT
public:
    SmilSystem();
    ~SmilSystem();

    void play();
    void reset();

    QString systemBitRate() const { return bitRate; }
    QString systemCaptions() const { return captions; }
    QString systemLanguage() const { return language; }
    QString systemOverdubOrSubtitle() const { return overdub; }
    QString systemRequired() const { return required; }
    QString systemScreenDepth() const { return screenDepth; }
    QString systemScreenSize() const { return screenSize; }

    SmilTransferServer *transferServer() { return xferServer; }
    const QMap<QString,SmilModule *> &modules() const { return mods; }

    void addModule(const QString &n, SmilModule *m) { mods[n] = m; }
    SmilModule *module(const QString &n) const { return mods[n]; }

    void setRootElement(SmilElement *e);
    SmilElement *rootElement() const { return root; }
    void setTarget(QWidget *w) { m_targetWidget = w; }
    QWidget* targetWidget() const;

    SmilElement *findElement(SmilElement *e, const QString &id) const;
    QColor rootColor() const;

    void setDirty(const QRect &r);
    const QRegion &dirtyRegion() const { return updRgn; }

    void update(const QRect &r);
    void bodyFinished();

signals:
    void finished();

protected:
    void paint(QPainter *p);
    void paint(SmilElement *e, QPainter *p);

private:
    SmilTransferServer *xferServer;
    QString bitRate;
    QString captions;
    QString language;
    QString overdub;
    QString required;
    QString screenDepth;
    QString screenSize;
    QMap<QString,SmilModule *> mods;
    SmilElement *root;
    QPixmap buffer;
    QWidget *m_targetWidget;
    QTimer *timer;
    QRegion updRgn;
    QRegion m_implicitPaintRegion;
    friend class SmilView;
};

#endif
