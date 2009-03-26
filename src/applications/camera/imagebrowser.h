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

#ifndef IMAGEBROWSER_H
#define IMAGEBROWSER_H

#include <QObject>
#include <QEvent>

#include <QContentSet>
#include <QContentSetModel>
#include <QCategoryManager>

#include <pictureflowview.h>

// Coverflow style camera image browser

class QContentSetModel;

class ImageBrowserPrivate;
class ImageBrowser : public QObject
{
    Q_OBJECT;
public:
    ImageBrowser(QContentSetModel* model, QObject *parent =0);

    ~ImageBrowser();

    void show();

    QWidget *pictureflowWidget();

private slots:
    void  currentChanged(const QModelIndex& index);

    void deleteCurrentSlide();

    void editCurrentSlide();

    void addCurrentSlideToContact();

    void sendCurrentSlideToContact();
private:
    ImageBrowserPrivate* d;
};
#endif

