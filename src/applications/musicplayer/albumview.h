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

#ifndef ALBUMVIEW_H
#define ALBUMVIEW_H

#include <QtGui>

class PictureFlowView;
class QMarqueeLabel;

class AlbumView : public QWidget
{
    Q_OBJECT
public:
    AlbumView(QWidget *parent);
    virtual ~AlbumView();
    void setModel(QAbstractItemModel *model, int displayRole);
    void setIndex(int index);
protected:
    virtual void resizeEvent(QResizeEvent *event);
private:
    PictureFlowView *pfView;
    QLabel *artistLabel;
    QLabel *albumLabel;
signals:
    void clicked(const QModelIndex &index);
private slots:
    void updateLabels(const QModelIndex &index);
};

#endif
