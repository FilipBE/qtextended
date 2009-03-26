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

#ifndef SMIL_H
#define SMIL_H

#include <qwidget.h>
#include <qtopiaglobal.h>

class QTextStream;
class SmilDataSource;
class SmilViewPrivate;
class SmilElement;

class QTOPIASMIL_EXPORT SmilView : public QWidget
{
    Q_OBJECT
public:
    explicit SmilView(QWidget *parent=0, Qt::WFlags f=0);
    ~SmilView();

    bool setSource(const QString &str);
    SmilElement *rootElement() const;

public slots:
    void play();
    void reset();

signals:
    void finished();
    void transferRequested(SmilDataSource *, const QString &src);
    void transferCancelled(SmilDataSource *, const QString &src);

protected:
    void paintEvent(QPaintEvent *p);

private:
    void clear();

private:
    SmilViewPrivate *d;
};

#endif
