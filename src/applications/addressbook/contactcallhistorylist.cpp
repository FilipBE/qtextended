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

#include "contactcallhistorylist.h"

#include "qsoftmenubar.h"
#include "qcontactmodel.h"
#include "qcontactview.h"

#include "qcalllist.h"
#include "qtimestring.h"
#include "qphonenumber.h"

#include "contactdetails.h"

#include <QKeyEvent>
#include <QVBoxLayout>
#include <QPixmap>
#include <QtopiaIpcEnvelope>
#ifdef QTOPIA_HOMEUI
#include <QSmoothList>
#endif

// -------------------------------------------------------------
// ContactCallHistoryItem
// -------------------------------------------------------------

class ContactCallHistoryItem : public QStandardItem
{
    public:
        ContactCallHistoryItem( const QIcon &icon, const QString& label);
        virtual ~ContactCallHistoryItem();

        virtual int type() const {return QStandardItem::UserType;}

        QCallListItem clItem;
};

ContactCallHistoryItem::ContactCallHistoryItem( const QIcon& icon, const QString& label)
    :  QStandardItem(icon, label)
{
}

ContactCallHistoryItem::~ContactCallHistoryItem()
{

}


// -------------------------------------------------------------
// ContactCallHistoryModel
// -------------------------------------------------------------
class ContactCallHistoryModel : public QStandardItemModel
{
    Q_OBJECT
    public:

        ContactCallHistoryModel( QObject *parent = 0);
        virtual ~ContactCallHistoryModel();

        void setContact( const QContact& contact )      {mContact = contact;refresh();}
        QContact contact() const                        {return mContact;}

        QContactModel::Field contactNumberToFieldType(const QString& number) const;

    public slots:
        void refresh();

    protected:
        void addRecord(const QCallListItem& cl);

        QCallList mCallList;
        QContact mContact;
};

ContactCallHistoryModel::ContactCallHistoryModel( QObject* parent )
    : QStandardItemModel(parent)
{
    connect( &mCallList, SIGNAL(updated()), this, SLOT(refresh()) );
}

ContactCallHistoryModel::~ContactCallHistoryModel()
{
}

QContactModel::Field ContactCallHistoryModel::contactNumberToFieldType(const QString& number) const
{
    QList<QContactModel::Field> list = QContactModel::phoneFields();
    list.append(QContactModel::Emails);

    int bestMatch = 0;
    QContactModel::Field bestField = QContactModel::Invalid;
    foreach(QContactModel::Field f, list) {
        QString candidate = QContactModel::contactField(mContact, f).toString();
        if ( candidate.isEmpty() )
            continue;
        int match = QPhoneNumber::matchNumbers(number, candidate);
        if (match > bestMatch) {
            bestField = f;
            bestMatch = match;
        }
    }
    return bestField;
}

void ContactCallHistoryModel::addRecord(const QCallListItem& cl)
{
    QIcon icon;
    QString desc;
    QString subText;
    QString mainText;

    switch( cl.type() )
    {
        case QCallListItem::Dialed:
            {
                static QIcon i(":icon/phone/outgoingcall");
                desc = tr("Dialed %1 %2", "Dialed 4th July 17:42");
                icon = i;
            }
            break;
        case QCallListItem::Received:
            {
                static QIcon i(":icon/phone/incomingcall");
                desc = tr("Received %1 %2", "Received 4th July 17:42");
                icon = i;
            }
            break;
        case QCallListItem::Missed:
            {
                static QIcon i(":icon/phone/missedcall");
                desc = tr("Missed %1 %2", "Missed 4th July 17:42");
                icon = i;
            }
            break;
    }

#ifndef QTOPIA_HOMEUI
    mainText = cl.number();
    subText = desc.arg(QTimeString::localMD(cl.start().date())).arg(QTimeString::localHM(cl.start().time(), QTimeString::Medium));
#else
    QDate today = QDate::currentDate();
    if (cl.start().date() == today)
        desc = tr("Today, %1", "Today, 15:30");
    else if(cl.start().date().addDays(1) == today)
        desc = tr("Yesterday, %1", "Yesterday, 15:30");
    else
        desc = tr("%1, %2", "<4th July>, <17:42>").arg(QTimeString::localMD(cl.start().date(), QTimeString::Long));

    mainText = desc.arg(QTimeString::localHM(cl.start().time(), QTimeString::Medium));
    if (cl.type() == QCallListItem::Missed)
        subText = tr("missed", "missed call");
    else {
        int secs = 0;
        if (cl.end().isValid())
            secs = cl.start().secsTo(cl.end());

        // Now format this somehow
        if (secs > 3600)
            subText = tr("%1:%2 hrs", "1:05 hrs").arg(secs / 3600, 2, 10, QLatin1Char('0')).arg((secs / 60) % 60, 2, 10, QLatin1Char('0'));
        else
            subText = tr("%1:%2 min", "4:02 min").arg(secs / 60, 2, 10, QLatin1Char('0')).arg(secs % 60, 2, 10, QLatin1Char('0'));
    }
#endif

    ContactCallHistoryItem *newItem = new ContactCallHistoryItem(icon, mainText);
    QIcon subicon = QContactModel::fieldIcon(contactNumberToFieldType(cl.number()));

    newItem->setData(subicon, ContactHistoryDelegate::SecondaryDecorationRole);
    newItem->setData(subText, ContactHistoryDelegate::SubLabelRole);
    newItem->setData(cl.start(), ContactHistoryDelegate::UserRole);

    newItem->clItem = cl;

    appendRow(newItem);
}

void ContactCallHistoryModel::refresh()
{
    clear();
    QCallList::SearchOptions so;
    so.contactId = mContact.uid();
    QList<QCallListItem> cl = mCallList.searchCalls( so );

    foreach(QCallListItem  clItem, cl) {
        addRecord(clItem);
    }

    setSortRole(ContactHistoryDelegate::UserRole);

    sort(0, Qt::DescendingOrder);
}

// -------------------------------------------------------------
// ContactCallHistoryList
// -------------------------------------------------------------
ContactCallHistoryList::ContactCallHistoryList( QWidget *parent )
    : QWidget( parent ), mInitedGui(false), mModel(0), mCallList(0), mListView(0)
{
    setObjectName("chl");

    QSoftMenuBar::setLabel(this, Qt::Key_Back,
                           QSoftMenuBar::Back, QSoftMenuBar::AnyFocus);
    QSoftMenuBar::setLabel(this, Qt::Key_Select,
                           QSoftMenuBar::NoLabel, QSoftMenuBar::AnyFocus);
}

ContactCallHistoryList::~ContactCallHistoryList()
{
    delete mCallList;
}

void ContactCallHistoryList::init( const QContact &entry )
{
    ent = entry;
    if (!mModel)
        mModel = new ContactCallHistoryModel(this);
    mModel->setContact(ent);

    /* Create our UI, if we haven't */
    if (!mInitedGui) {
        mInitedGui = true;

        QVBoxLayout *main = new QVBoxLayout();
#ifdef QTOPIA_HOMEUI
        mListView = new QSmoothList();
        mListView->setEmptyText(tr("No history of calls with this contact"));
        mListView->setModel(mModel);
#else
        mListView = new QListView();
        mListView->setResizeMode(QListView::Adjust);
        mListView->setLayoutMode(QListView::Batched);
        mListView->setFrameStyle(QFrame::NoFrame);
        mListView->setSelectionMode(QAbstractItemView::SingleSelection);
        mListView->setModel(mModel); // this creates the selectionModel
        connect(mListView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(updateItemUI(QModelIndex)));
#endif
        mListView->setItemDelegate(new ContactHistoryDelegate(mListView));
        mListView->installEventFilter(this);

        main->addWidget(mListView);
        main->setMargin(0);
        setLayout(main);
        connect(mListView, SIGNAL(activated(QModelIndex)), this, SLOT(showCall(QModelIndex)));
    }
}

void ContactCallHistoryList::updateItemUI(const QModelIndex& idx)
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

void ContactCallHistoryList::showCall(const QModelIndex &idx)
{
    if (idx.isValid()) {
        ContactCallHistoryItem * cchi = static_cast<ContactCallHistoryItem*>(mModel->itemFromIndex(idx));
        if (cchi) {
            QtopiaServiceRequest req( "CallHistory", "viewDetails(QCallListItem,QContact,int)" );
            req << cchi->clItem << mModel->contact() << (int)mModel->contactNumberToFieldType(cchi->clItem.number());
            req.send();
        }
    }
}

bool ContactCallHistoryList::eventFilter( QObject *o, QEvent *e )
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

void ContactCallHistoryList::keyPressEvent( QKeyEvent *e )
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

#include "contactcallhistorylist.moc"
