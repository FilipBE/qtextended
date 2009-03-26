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

#ifndef CALLCONTACTLIST_H
#define CALLCONTACTLIST_H

#include <qcontactmodel.h>
#include <qcontactview.h>
#include <qcalllist.h>
#include <pkimmatcher.h>
#include <QSmoothList>

class QMenu;
class QAction;
class QUniqueId;

class CallContactItem : public QObject
{
Q_OBJECT
public:
    static QCallList::ListType stateToType( QCallListItem::CallType st );
    static QPixmap typePixmap( QCallListItem::CallType type );

    CallContactItem( QCallListItem clItem, QObject * parent = 0 );

    QContactModel::Field fieldType() const;

    void setContact( const QContact& cnt, const QString& phoneNumber);
    void setContact( const QContactModel *m, const QUniqueId &);

    QContact contact() const;

    QPixmap decoration() const;
    QPixmap extraDecoration() const;
    QString text() const;
    QString extraInfoText() const;
    QCallListItem callListItem() const;

    QString number() const;
    QUniqueId contactID() const;

private:
    QContactModel::Field contactNumberToFieldType(const QString& number) const;
    QString fieldTypeToContactDetail() const;

    mutable QContactModel::Field mFieldType;
    mutable QContact mContact;

    QContactModel const *mModel;
    QUniqueId mId;

    QCallListItem clItem;
};

class CallContactModel : public QAbstractListModel
{
    Q_OBJECT
public:

    CallContactModel( QCallList &callList, QObject *parent = 0);
    virtual ~CallContactModel();

    CallContactItem * itemAt( const QModelIndex & index ) const;

    int rowCount(const QModelIndex & = QModelIndex()) const
    { return callContactItems.count(); }
    QVariant data(const QModelIndex &index, int role) const;

    void resetModel();
    virtual void refresh();

    QString filter () const { return mFilter;};

public slots:
    void setFilter(const QString& filter);

signals:
    void filtered(const QString& filter);

protected:
    QList<CallContactItem*> callContactItems;
    QCallList &mCallList;
    QList<QCallListItem> mRawCallList;
    InputMatcher pk_matcher;
private:
    int findPattern(const QString &content) const;

    QString mFilter;
};

class CallContactDelegate : public QContactDelegate
{
public:
    CallContactDelegate( QObject * parent = 0 );
    virtual ~CallContactDelegate();

    QFont secondaryFont(const QStyleOptionViewItem& o, const QModelIndex& idx) const;
};

class CallContactListView : public QSmoothList
{
    Q_OBJECT
public:
    CallContactListView(QWidget * parent = 0);
    ~CallContactListView();

    void setModel(QAbstractItemModel* model);
    QString numberForIndex(const QModelIndex & idx) const;
    QContact contactForIndex(const QModelIndex & idx) const;

    void setEmptyMessage(const QString& newMessage);

public slots:
    void addItemToContact();
    void openContact();
    void sendMessageToItem();
    void showRelatedCalls();
    void showAllCalls();
    virtual void updateMenu();

    virtual void reset();

signals:
    void requestedDial(const QString&, const QUniqueId&);
    void hangupActivated();

protected:
    void keyPressEvent( QKeyEvent *e );
    void paintEvent( QPaintEvent *pe );
    void focusInEvent( QFocusEvent *focusEvent);
    void focusOutEvent( QFocusEvent *focusEvent);

    QMenu *mMenu;

    QAction *mOpenContact;
    QAction *mAddContact;
    QAction *mSendMessage;
    QAction *mRelatedCalls, *mAllCalls;

    QString m_noResultMessage;

    CallContactModel *cclm;
};

#endif
