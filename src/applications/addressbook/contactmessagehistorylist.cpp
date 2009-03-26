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

#include "contactmessagehistorylist.h"

#include "qsoftmenubar.h"
#include "qcontactmodel.h"
#include "qcontactview.h"

#include "contactdetails.h"

#include "qtimestring.h"

#include "qtopiaservices.h"

#include "qmailmessage.h"
#include "qmailstore.h"
#include "qmailmessagekey.h"

#include "qcollectivenamespace.h"

#include <QValueSpaceItem>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPhoneNumber>
#include <QContactFieldDefinition>

#ifdef QTOPIA_HOMEUI
#include <QMailMessageListModel>
#include <QtopiaHomeMailMessageDelegate>
#include <QSmoothList>
#endif

// -------------------------------------------------------------
// ContactMessageHistoryItem
// -------------------------------------------------------------

// Just to store the mailbox and message id
class ContactMessageHistoryItem : public QStandardItem
{
    public:
        ContactMessageHistoryItem( const QIcon& icon, const QString& str);
        virtual ~ContactMessageHistoryItem();

        int type() const {return QStandardItem::UserType;}

        QString mMessageMailbox;
        QMailMessageId mMessageId;
};

ContactMessageHistoryItem::ContactMessageHistoryItem( const QIcon& icon, const QString& str)
    :  QStandardItem(icon, str)
{
}

ContactMessageHistoryItem::~ContactMessageHistoryItem()
{
}

// -------------------------------------------------------------
// ContactMessageHistoryModel
// -------------------------------------------------------------
#ifdef QTOPIA_HOMEUI
typedef QMailMessageListModel ContactMessageHistoryModelBase;
#else
typedef QStandardItemModel ContactMessageHistoryModelBase;
#endif

class ContactMessageHistoryModel : public ContactMessageHistoryModelBase
{
    Q_OBJECT
    public:

        ContactMessageHistoryModel( QObject *parent = 0);
        virtual ~ContactMessageHistoryModel();

        virtual void refresh();

        void setContact( const QContact& contact )      {mContact = contact;refresh();}
        QContact contact() const                        {return mContact;}

    protected slots:
        void newMessageCountChanged();

    protected:
#ifndef QTOPIA_HOMEUI
        void addRecord(bool sent, const QMailMessageMetaData& msg);
#endif

        QContact mContact;
        QValueSpaceItem *mVSItem;
};

ContactMessageHistoryModel::ContactMessageHistoryModel( QObject* parent )
    : ContactMessageHistoryModelBase(parent)
{
    mVSItem = new QValueSpaceItem("Communications/Messages/NewMessages");
    connect(mVSItem, SIGNAL(contentsChanged()), this, SLOT(newMessageCountChanged()));
}

ContactMessageHistoryModel::~ContactMessageHistoryModel()
{
    delete mVSItem;
}

void ContactMessageHistoryModel::newMessageCountChanged()
{
    refresh();
}

#ifndef QTOPIA_HOMEUI
void ContactMessageHistoryModel::addRecord(bool sent, const QMailMessageMetaData &m)
{
    static QIcon sentMessageIcon(":icon/qtmail/sendmail");
    static QIcon receivedMessageIcon(":icon/qtmail/getmail");

    QIcon* itemIcon = 0;
    switch(m.messageType()) {
        case QMailMessage::Mms: { static QIcon mmsIcon(":icon/multimedia"); itemIcon = &mmsIcon; }
            break;

        case QMailMessage::Email: { static QIcon emailIcon(":icon/email"); itemIcon = &emailIcon; }
            break;

        case QMailMessage::Instant: { static QIcon imIcon(":icon/im"); itemIcon = &imIcon; }
            break;

        // Default case - SMS and unspecialized types (System)
        default: { static QIcon icon(":icon/txt"); itemIcon = &icon; }
            break;
    }

    ContactMessageHistoryItem * newItem = new ContactMessageHistoryItem(sent ? sentMessageIcon : receivedMessageIcon, m.subject());
    newItem->setData(*itemIcon, ContactHistoryDelegate::SecondaryDecorationRole);

    QDateTime messageTime = m.date().toLocalTime();
    newItem->setData(messageTime, ContactHistoryDelegate::UserRole);

    QString desc = sent ? tr("Sent %1 %2", "Sent 4th July 17:42") : tr("Received %1 %2", "Received 4th July 17:42");
    QString subtext = desc.arg(QTimeString::localMD(messageTime.date())).arg(QTimeString::localHM(messageTime.time(), QTimeString::Short));

    newItem->setData(subtext, ContactHistoryDelegate::SubLabelRole);

    newItem->mMessageMailbox = m.fromMailbox();
    newItem->mMessageId = m.id();
    appendRow(newItem);
}
#endif

void ContactMessageHistoryModel::refresh()
{
#ifndef QTOPIA_HOMEUI
    clear();
#endif

    // We do two queries - one for from, one for to.
    QMailMessageKey msgsTo;
    QMailMessageKey msgsFrom;

    // Phone numbers
    foreach(const QString &num, mContact.phoneNumbers().values()) {
        msgsTo |= QMailMessageKey(QMailMessageKey::Recipients, num, QMailDataComparator::Includes);
        msgsFrom |= QMailMessageKey(QMailMessageKey::Sender, num, QMailDataComparator::Equal);
    }

    // Email addresses
    foreach(const QString &email, mContact.emailList()) {
        msgsTo |= QMailMessageKey(QMailMessageKey::Recipients, email, QMailDataComparator::Includes);
        msgsFrom |= QMailMessageKey(QMailMessageKey::Sender, email, QMailDataComparator::Equal);
    }

    // Chat addresses
    foreach(const QString &field, QContactFieldDefinition::fields("chat")) {
        QContactFieldDefinition def(field);
        QString address = def.value(mContact).toString();
        if (!address.isEmpty()) {
            QString qualified = QCollective::encodeUri(def.provider(), address);
            msgsTo |= QMailMessageKey(QMailMessageKey::Recipients, qualified, QMailDataComparator::Includes);
            msgsFrom |= QMailMessageKey(QMailMessageKey::Sender, qualified, QMailDataComparator::Equal);
        }
    }

#ifndef QTOPIA_HOMEUI
    // Now get the messages sent to this contact
    if (!msgsTo.isEmpty())
        foreach (const QMailMessageId &id, QMailStore::instance()->queryMessages(msgsTo))
            addRecord(true, QMailMessageMetaData(id));

    // And messages from
    if (!msgsFrom.isEmpty())
        foreach (const QMailMessageId &id, QMailStore::instance()->queryMessages(msgsFrom))
            addRecord(false, QMailMessageMetaData(id));

    // Make sure we sort on the right role
    setSortRole(ContactHistoryDelegate::UserRole);

    // and sort it
    sort(0, Qt::DescendingOrder);
#else
    if (msgsTo.isEmpty() && msgsFrom.isEmpty()) {
        // No addresses to select with; force an empty selection filter (match null ID)
        setKey(QMailMessageKey::nonMatchingKey());
    } else {
        setKey(msgsTo | msgsFrom);
    }

    setSortKey(QMailMessageSortKey(QMailMessageSortKey::TimeStamp, Qt::DescendingOrder));
#endif
}


// -------------------------------------------------------------
// ContactMessageHistoryListView
// -------------------------------------------------------------
#ifdef QTOPIA_HOMEUI
typedef QSmoothList ContactMessageHistoryListViewBase;
#else
typedef QListView ContactMessageHistoryListViewBase;
#endif

class ContactMessageHistoryListView : public ContactMessageHistoryListViewBase
{
    Q_OBJECT

public:
    ContactMessageHistoryListView(QWidget* parent = 0);
    virtual ~ContactMessageHistoryListView();

#ifndef QTOPIA_HOMEUI
    using ContactMessageHistoryListViewBase::setModel;
    void setModel(ContactMessageHistoryModel *m);
#endif

signals:
    void messageActivated(const QMailMessageId &id); 
#ifdef QTOPIA_HOMEUI
    void messageReplyActivated(const QMailMessageId &id); 
#endif

protected slots:
    void messageActivated(const QModelIndex &idx);

protected:
#ifdef QTOPIA_HOMEUI
    void mouseReleaseEvent(QMouseEvent* e);

    QPoint pos;
    QtopiaHomeMailMessageDelegate *delegate;
#else
    ContactMessageHistoryModel *model;
#endif
};

ContactMessageHistoryListView::ContactMessageHistoryListView(QWidget* parent)
    : ContactMessageHistoryListViewBase(parent)
{
#ifdef QTOPIA_HOMEUI
    delegate = new QtopiaHomeMailMessageDelegate(QtopiaHomeMailMessageDelegate::AddressbookMode, this);

    setEmptyText(tr("No history of messages with this contact"));
    setItemDelegate(delegate);
#else
    setResizeMode(QListView::Adjust);
    setLayoutMode(QListView::Batched);
    setFrameStyle(QFrame::NoFrame);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setItemDelegate(new ContactHistoryDelegate(this));
#endif

    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(messageActivated(QModelIndex)));
}

ContactMessageHistoryListView::~ContactMessageHistoryListView()
{
};

#ifndef QTOPIA_HOMEUI
void ContactMessageHistoryListView::setModel(ContactMessageHistoryModel *m)
{
    model = m;
    ContactMessageHistoryListViewBase::setModel(m);
}
#endif

#ifdef QTOPIA_HOMEUI
void ContactMessageHistoryListView::mouseReleaseEvent(QMouseEvent* e)
{
    pos = e->pos();
    ContactMessageHistoryListViewBase::mouseReleaseEvent(e);
}
#endif

void ContactMessageHistoryListView::messageActivated(const QModelIndex &idx)
{
    if (idx.isValid()) {
        QMailMessageId msgId;

#ifdef QTOPIA_HOMEUI
        msgId = qvariant_cast<QMailMessageId>(idx.data(QMailMessageListModel::MessageIdRole));
#else
        if (ContactMessageHistoryItem * cmhi = static_cast<ContactMessageHistoryItem*>(model->itemFromIndex(idx)))
            msgId = cmhi->mMessageId;
#endif

        if (msgId.isValid()) {
#ifdef QTOPIA_HOMEUI
            if (delegate->replyButtonRect(visualRect(idx)).contains(pos)) {
                emit messageReplyActivated(msgId);
                return;
            }
#endif
            emit messageActivated(msgId);
        }
    }
}


// -------------------------------------------------------------
// ContactMessageHistoryList
// -------------------------------------------------------------
ContactMessageHistoryList::ContactMessageHistoryList( QWidget *parent )
    : QWidget( parent ), mInitedGui(false), mModel(0), mListView(0)
{
    setObjectName("cmhl");

    QSoftMenuBar::setLabel(this, Qt::Key_Back,
                           QSoftMenuBar::Back, QSoftMenuBar::AnyFocus);
    QSoftMenuBar::setLabel(this, Qt::Key_Select,
                           QSoftMenuBar::NoLabel, QSoftMenuBar::AnyFocus);
}

ContactMessageHistoryList::~ContactMessageHistoryList()
{

}

void ContactMessageHistoryList::init( const QContact &entry )
{
    ent = entry;
    if (!mModel)
        mModel = new ContactMessageHistoryModel(this);
    mModel->setContact(ent);

    /* Create our UI, if we haven't */
    if (!mInitedGui) {
        mInitedGui = true;

        QVBoxLayout *main = new QVBoxLayout();
        mListView = new ContactMessageHistoryListView();
        mListView->setModel(mModel);
        mListView->installEventFilter(this);

        connect(mListView, SIGNAL(messageActivated(QMailMessageId)), this, SLOT(showMessage(QMailMessageId)));
#ifdef QTOPIA_HOMEUI
        connect(mListView, SIGNAL(messageReplyActivated(QMailMessageId)), this, SLOT(replyToMessage(QMailMessageId)));
#else
        connect(mListView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(updateItemUI(QModelIndex)));
#endif

        main->addWidget(mListView);
        main->setMargin(0);
        setLayout(main);
    }
}

void ContactMessageHistoryList::updateItemUI(const QModelIndex& idx)
{
    if (idx.isValid()) {
        QSoftMenuBar::setLabel(this, Qt::Key_Select,
                           QSoftMenuBar::View, QSoftMenuBar::AnyFocus);
    } else {
        QSoftMenuBar::setLabel(this, Qt::Key_Select,
                               QSoftMenuBar::NoLabel, QSoftMenuBar::AnyFocus);
    }

#ifndef QTOPIA_HOMEUI
    if (idx.isValid())
        mListView->selectionModel()->select(idx, QItemSelectionModel::Select);
#endif
}

void ContactMessageHistoryList::showMessage(const QMailMessageId &id)
{
    QtopiaServiceRequest req( "Messages", "viewMessage(QMailMessageId)" );
    req << id;
    req.send();
}

#ifdef QTOPIA_HOMEUI
void ContactMessageHistoryList::replyToMessage(const QMailMessageId &id)
{
    QtopiaServiceRequest req( "Messages", "replyToMessage(QMailMessageId)" );
    req << id;
    req.send();
}
#endif

bool ContactMessageHistoryList::eventFilter( QObject *o, QEvent *e )
{
    if(o == mListView && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent *)e;
        if (ke->key() == Qt::Key_Back ) {
            emit closeView();
            return true;
        }
    }
    return false;
}

void ContactMessageHistoryList::keyPressEvent( QKeyEvent *e )
{
    switch(e->key())
    {
        case Qt::Key_Back:
            emit closeView();
            return;
        case Qt::Key_Call:
        // TODO handleCall();
            return;
    }

    QWidget::keyPressEvent(e);
}

#include "contactmessagehistorylist.moc"
