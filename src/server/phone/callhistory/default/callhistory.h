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

#ifndef CALLHISTORY_H
#define CALLHISTORY_H

#include "callcontactlist.h"
#include "qabstractcallhistory.h"
#include <qcontact.h>
#include <QCallList>
#include <QtopiaAbstractService>
#include <QDialog>

class QLineEdit;
class QTextEntryProxy;
class QToolButton;
class QListWidget;
class QListWidgetItem;

class QAction;
class QAbstractMessageBox;
class QListBox;
class QTabWidget;
class QStackedWidget;
class QLabel;
class QKeyEvent;
class QItemDelegate;

class CallHistoryModel : public CallContactModel
{
    Q_OBJECT
public:
    CallHistoryModel(QCallList& callList, QObject* parent = 0);
    ~CallHistoryModel();

    QCallList::ListType type() const;
    void setType( QCallList::ListType type );

    void refresh();
    int rowCount(const QModelIndex& parent = QModelIndex()) const;

signals:
    void contactsAboutToBeUpdated();
    void contactsUpdated();

protected slots:
    void updateContacts();
    void reallyUpdateContacts();

private:
    QCallList::ListType mType;
    bool mDirty;
};

class CallHistoryListView : public CallContactListView
{
    friend class PhoneCallHistory;
    Q_OBJECT
public:
    CallHistoryListView( QWidget *parent, Qt::WFlags fl = 0 );
    void setModel( QAbstractItemModel* model );
#ifdef QTOPIA_HOMEUI
    QPoint clickPos() const { return cpos; }

protected:
    void mousePressEvent(QMouseEvent *e);
#endif

public slots:
    void refreshModel();
    void modelChanged();

protected slots:
    void updateMenu();
    void contactsAboutToChange();
    void contactsChanged();

private:
    QAction *mClearList;
    int prevRow;
    QString prevNumber;
    int prevCount;
    bool contactsChanging;
    QPoint cpos;
};

class CallHistoryView : public QWidget
{
    Q_OBJECT
public:
    CallHistoryView( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~CallHistoryView();

    QContact contact( QContactModel::Field &phoneType ) const;
    void setContact( const QContact &cnt, QContactModel::Field phoneType );
    void setCallListItem( QCallListItem item );

public slots:
    void update();
    void clear();

signals:
    void externalLinkActivated();
    void deleteCurrentItem();

protected slots:
    void openContact();
    void addContact();
    void sendMessage();
    void updateMenu();

    void deleteItem();
    void deleteItem(int);
    void contactsChanged();
    void dialNumber();

protected:
    void keyPressEvent( QKeyEvent *e );

private:
    bool mHaveFocus;
    bool mHaveContact;
    bool mHaveDialer;
    QLabel *mPortrait, *mName, *mContactTypePic, *mPhoneTypePic, *mNumber, *mStartDate, *mStartTime, *mDuration, *mTimeZone, *mTimeZoneLabel;
#ifdef QTOPIA_HOMEUI
    QToolButton *mCallTypeHeader;
#else
    QLabel *mCallTypePic, *mCallType;
#endif
    QCallListItem mCallListItem;
    QContact mContact;
    QContactModel::Field mPhoneType;
    QAbstractMessageBox *deleteMsg, *addContactMsg;
    QPushButton *mContactBtn;

    QMenu *mMenu;
    QAction *mDeleteAction, *mOpenContact, *mSendMessage, *mAddContact;
};

#ifndef QTOPIA_HOMEUI
class CallHistoryClearList : public QDialog
{
    Q_OBJECT
public:
    CallHistoryClearList( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~CallHistoryClearList();

    void setSelected(QCallList::ListType);

protected slots:
    void userSelected(QListWidgetItem *);

signals:
    void selected(QCallList::ListType);

private:
    QListWidget *mList;
};
#endif

class PhoneCallHistory : public QAbstractCallHistory
{
    Q_OBJECT
    friend class CallHistoryService;
public:
    PhoneCallHistory( QWidget *parent = 0, Qt::WFlags fl = 0 );
    bool eventFilter( QObject *o, QEvent *e );
    void reset();
    void showMissedCalls();
    void refresh();
    void setFilter( const QString &f );
signals:
    void requestedDial(const QString&, const QUniqueId&);
    void viewedMissedCalls();

protected slots:
    void refreshOnFirstShow(int);
    void focusFindLE(int);
    void showEvent( QShowEvent *e );
    void viewDetails( const QModelIndex& idx );
    void pageChanged(int);
    void clearList();
    void clearList( QCallList::ListType );
    void setFilterCur(const QString &f);
    void deleteCurrentItem();
    void deleteViewedItem();
    void showList( QCallList::ListType type );
    void viewDetails( QCallListItem, QContact, int );
    void updateTabText( const QString &filterStr );
    void deleteItems(int);

protected:
    void constructTab( QCallList::ListType type, QAction *clearAction, QAbstractItemDelegate *delegate );
    void cleanup();

private:
    QMap<QObject *, QString> mFilters;
    QTabWidget *mTabs;
    CallHistoryListView *mAllList, *mDialedList, *mReceivedList, *mMissedList;
    CallHistoryView *mView;
    QCallList &mCallList;
    bool mShowMissedCalls;
    bool mAllListShown, mDialedListShown, mReceivedListShown, mMissedListShown;
#ifndef QTOPIA_HOMEUI
    CallHistoryClearList *mClearList;
#endif
    QLineEdit *mAllFindLE, *mDialedFindLE, *mReceivedFindLE, *mMissedFindLE;
    QTextEntryProxy *mAllFindProxy, *mDialedFindProxy, *mReceivedFindProxy, *mMissedFindProxy;
    QLabel *mAllFindIcon, *mDialedFindIcon, *mReceivedFindIcon, *mMissedFindIcon;
    QCallListItem mViewedItem;
    QAbstractMessageBox *mDeleteMsg;
    QCallList::ListType mDeleteType;
};

class CallHistoryService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class PhoneCallHistory;
public:
    CallHistoryService( PhoneCallHistory *parent )
        : QtopiaAbstractService( "CallHistory", parent )
        { this->parent = parent; publishAll(); }

public:
    ~CallHistoryService();

public slots:
    void showCallHistory( QCallList::ListType type, const QString& filterHint );
    void showCallHistory();
    void showMissedCalls();
    void viewDetails( QCallListItem, QContact, int );

private:
    PhoneCallHistory *parent;
};


#endif
