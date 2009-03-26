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
#ifndef CONTACTCALLHISTORYLIST_H
#define CONTACTCALLHISTORYLIST_H

#include <QWidget>
#include <qcontact.h>

class ContactCallHistoryModel;
class QCallList;
class QListView;
class QSmoothList;
class QContactModel;
class QModelIndex;

class ContactCallHistoryList : public QWidget
{
    Q_OBJECT

public:
    ContactCallHistoryList( QWidget *parent );
    virtual ~ContactCallHistoryList();

    QContact entry() const {return ent;}

public slots:
    void init( const QContact &entry );

signals:
    void externalLinkActivated();
    void closeView();

protected:
    void keyPressEvent( QKeyEvent *e );
    bool eventFilter(QObject*, QEvent*);

protected slots:
    void updateItemUI(const QModelIndex& idx);
    void showCall(const QModelIndex &idx);

private:
    QContact ent;
    bool mInitedGui;
    ContactCallHistoryModel *mModel;
    QCallList *mCallList;
#ifdef QTOPIA_HOMEUI
    QSmoothList*mListView;
#else
    QListView *mListView;
#endif
};

#endif
