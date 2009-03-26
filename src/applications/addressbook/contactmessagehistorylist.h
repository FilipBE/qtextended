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
#ifndef CONTACTMESSAGEHISTORYLIST_H
#define CONTACTMESSAGEHISTORYLIST_H

#include <QWidget>
#include <qcontact.h>

class ContactMessageHistoryListView;
class ContactMessageHistoryModel;
class QListView;
class QSmoothList;
class QContactModel;
class QModelIndex;
class QMailMessageId;

class ContactMessageHistoryList : public QWidget
{
    Q_OBJECT

public:
    ContactMessageHistoryList( QWidget *parent );
    virtual ~ContactMessageHistoryList();

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
    void showMessage(const QMailMessageId &id);
#ifdef QTOPIA_HOMEUI
    void replyToMessage(const QMailMessageId &id);
#endif

private:
    QContact ent;
    bool mInitedGui;
    ContactMessageHistoryModel *mModel;
    ContactMessageHistoryListView *mListView;
};

#endif
