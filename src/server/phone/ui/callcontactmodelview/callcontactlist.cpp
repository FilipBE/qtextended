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

#include "callcontactlist.h"

#include <qphonenumber.h>

#include <qtimestring.h>
#include <qsoftmenubar.h>
#include <qtopiaservices.h>
#include <quniqueid.h>
#include <qtopiaipcenvelope.h>
#include <qthumbnail.h>
#include <qtopiaapplication.h>

#include <QKeyEvent>
#include <QPainter>
#include <QDebug>
#include <QSettings>
#include <QMenu>

#include "savetocontacts.h"


/*!
    \class CallContactItem
    \inpublicgroup QtTelephonyModule
    \brief The CallContactItem class provides a wrapper for call entries.
    \ingroup QtopiaServer::PhoneUI

    It simplifies the interaction between CallContactModel and CallContactListView and connects
    the actual call data such as the phone number with a contact.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa CallContactListView, CallContactDelegate, CallContactModel
*/

/*!
    Creates a new CallContactItem with the given \a parent.
    The item is based on the call information provided by \a cli.

    \sa QCallListItem
*/
CallContactItem::CallContactItem( QCallListItem cli, QObject *parent)
: QObject(parent), mFieldType(QContactModel::Invalid), mModel(0), clItem(cli)
{
}

/*!
    Returns the call data associated with this item.
*/
QCallListItem CallContactItem::callListItem() const
{
    return clItem;
}

/*!
    Returns the type of the associated contact field.

    \sa QContactModel::Field
*/
QContactModel::Field CallContactItem::fieldType() const
{
    if (mFieldType == QContactModel::Invalid && mModel) {
        QString number = contact().defaultPhoneNumber();
        mFieldType = contactNumberToFieldType(number);
    }
    return mFieldType;
}

/*!
    Returns the id of the associated contact.
*/
QUniqueId CallContactItem::contactID() const
{
    return mId;
}


/*!
    The given \a number becomes associated to \a contact.
*/
void CallContactItem::setContact( const QContact& contact, const QString& number)
{
    mContact = contact;
    mId = contact.uid();
    mModel = 0;
    mFieldType = contactNumberToFieldType(number.simplified());
}

/*!
    The contact item becomes associated with \a id and \a model is used to
    to lookup the details of \a id.
*/
void CallContactItem::setContact( const QContactModel *model, const QUniqueId &id)
{
    mModel = model;
    mId = id;
    mFieldType = QContactModel::Invalid;
    mContact = QContact();
}

/*!
    Returns the contact associated to the call entry.
*/
QContact CallContactItem::contact() const
{
    if (mContact.uid() != mId && mModel)
        mContact = mModel->contact(mId);
    return mContact;
}

/*!
    Returns the pixmap for this call entry.

    The most useful pixmap is the contacts image. If the contact doesn't have a protrait
    the phone numbers type (dialed/received/missed) pixmap is returned.
*/
QPixmap CallContactItem::decoration() const
{
    if (!mId.isNull())
        return contact().thumbnail();

    return typePixmap(clItem.type());
}

/*!
    Returns an icon representing the contact model field of the call entry.
    Returns a null icon if no icon is available.

    \sa QContactModel::fieldIcon()
*/
QPixmap CallContactItem::extraDecoration() const
{
    QIcon icon = QContactModel::fieldIcon(fieldType());
    return icon.isNull() ? QPixmap() : QPixmap(icon.pixmap(QSize(16,16)));
}

/*!
    Returns a suitable display string for the contact or the phone number of this entry if
    no contact information is avilable.
*/
QString CallContactItem::text() const
{
    if (!mId.isNull())
        return contact().label();
    return clItem.number();
}

/*!
    If this call item is associated to a contact this function returns the called number.
    Otherwise the extra information contains the call details such as "Dialed 2. March 14:30".
*/
QString CallContactItem::extraInfoText( ) const
{
    if (!mId.isNull() && clItem.isNull())
    {
        return fieldTypeToContactDetail();
    }
    else if (!clItem.isNull())
    {
        QString desc;
        QCallListItem::CallType st = clItem.type();
        if ( st == QCallListItem::Dialed )
            desc = tr("Dialed");
        else if ( st == QCallListItem::Received )
            desc = tr("Received");
        else if ( st == QCallListItem::Missed )
            desc = tr("Missed");
        desc += QChar(' ');
        QDateTime dt = clItem.start();
        QDate callDate = dt.date();
        QTime callTime = dt.time();
        QString when("%1 %2");
        when = when.arg(QTimeString::localMD(callDate, QTimeString::Short))
            .arg(QTimeString::localHM(callTime, QTimeString::Medium));
        return desc + when;
    }
    else
    {
        qWarning("BUG: item is not contact and not in call list index");
        return QString();
    }
}

/*!
    Returns the phone number associated to this call item.
*/
QString CallContactItem::number() const
{
    if (!mId.isNull() && clItem.isNull())
        return fieldTypeToContactDetail();
    else if (!clItem.isNull())
        return clItem.number();
    else
        return QString("");
}

/*!
    Returns the pixmap for \a type.
*/
QPixmap CallContactItem::typePixmap( QCallListItem::CallType type )
{
    QString typePixFileName;
    switch( type )
    {
        case QCallListItem::Dialed:
            typePixFileName = "phone/outgoingcall";
            break;
        case QCallListItem::Received:
            typePixFileName = "phone/incomingcall";
            break;
        case QCallListItem::Missed:
            typePixFileName = "phone/missedcall";
            break;
    }

    QIcon icon(":icon/"+typePixFileName);

    return icon.pixmap(QContact::thumbnailSize());
}

QContactModel::Field CallContactItem::contactNumberToFieldType(const QString& number) const
{
    QContact cnt = contact();
    static QList<QContactModel::Field> list;
    if ( list.count() == 0 ) {
        list = QContactModel::phoneFields();
        list.append(QContactModel::Emails);
    }

    int bestMatch = 0;
    QContactModel::Field bestField = QContactModel::Invalid;
    foreach(QContactModel::Field f, list) {
        QString candidate = QContactModel::contactField(cnt, f).toString();
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

QString CallContactItem::fieldTypeToContactDetail() const
{
    QContact cnt = contact();
    return QContactModel::contactField(cnt, fieldType()).toString();
}


/*!
    Translates the QCallListItem::CallType enum value \a st into the appropriate QCallList::ListType field.
*/
QCallList::ListType CallContactItem::stateToType( QCallListItem::CallType st )
{
    if ( st == QCallListItem::Dialed )
        return QCallList::Dialed;
    else if ( st == QCallListItem::Missed )
        return QCallList::Missed;
    else if ( st == QCallListItem::Received )
        return QCallList::Received;
    else {
        qWarning("BUG: Invalid state passed to CallContactItem::stateToType");
        return QCallList::Dialed;
    }
}

//===================================================================

/*!
    \class CallContactModel
    \inpublicgroup QtTelephonyModule
    \brief The CallContactModel class provides underlying model for the call contact list view.
    \ingroup QtopiaServer::PhoneUI

    This model is based on a QAbstractListModel and maintains CallContactItems.
    Together with CallContactListView, CallContactItem and CallContactDelegate it
    provides a model-view representation of call contact information.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
    \fn QString CallContactModel::filter () const

    Returns the currently applied filter for this model.
*/

/*!
    \fn void CallContactModel::filtered(const QString& filter)

    Emitted when the current filter changes. The new filter value is \a filter.
*/

/*!
    \fn int CallContactModel::rowCount(const QModelIndex &idx) const

    \reimp
*/
/*!
    Creates a new CallContactModel based on the given \a callList. \a parent
    is the standard QObject parent parameter.
*/
CallContactModel::CallContactModel( QCallList &callList, QObject *parent)
    :QAbstractListModel(parent), mCallList(callList), pk_matcher("text")
{
    mRawCallList = mCallList.allCalls();
}

/*!
    Destroys the model.
*/
CallContactModel::~CallContactModel() {}


/*!
    Returns the CallContactItem at position \a index in the model.
*/
CallContactItem * CallContactModel::itemAt( const QModelIndex & index ) const
{
    if (!index.isValid())
        return 0;

    return callContactItems.at(index.row());
}

int CallContactModel::findPattern(const QString &content) const
{
    const QString ctext = content.toLower();
    int idx = ctext.indexOf(mFilter);
    if (idx == -1)
        idx = pk_matcher.collate(ctext).indexOf(mFilter);
    return idx;
}

/*!
    \reimp
*/
QVariant CallContactModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < rowCount())
    {
        CallContactItem * item = callContactItems.at(index.row());
        if (!item)
            return QVariant();
        QString text = item->text();
        switch(role)
        {
            case Qt::DecorationRole:
            case QContactModel::PortraitRole:
                return item->decoration();
            case Qt::DisplayRole:
                return text.isEmpty() ? item->extraInfoText() : text;
            case QContactModel::LabelRole:
                {
                    QString result = text.isEmpty() ? item->extraInfoText() : text;
                    return result.prepend("<b>").append("</b>");
                }
            case QContactModel::SubLabelRole:
                return text.isEmpty() ? QString() : item->extraInfoText();
            case QContactModel::StatusIconRole:
                return item->extraDecoration();
            default:
                break;
        }
    }
    return QVariant();
}

/*!
    Reinitializes the call items in the model.
*/
void CallContactModel::refresh()
{
    mRawCallList = mCallList.allCalls();
}

/*!
    Resets the model to its original in any attached CallContactListView.
    All contact items will be removed in the process.
*/
void CallContactModel::resetModel()
{
    while (!callContactItems.isEmpty())
        delete callContactItems.takeFirst();
}

/*!
    \internal

    Not currently used.
*/
void CallContactModel::setFilter(const QString& filter)
{
    bool ok = false;
    filter.toInt( &ok );
    if ( ok )
        mFilter = filter;
    else
        mFilter = pk_matcher.collate( filter );
    refresh();

    emit filtered( filter );
}

//==================================================================

/*!
    \class CallContactDelegate
    \inpublicgroup QtTelephonyModule
    \brief The CallContactDelegate class provides the delegate for CallContactListView.
    \ingroup QtopiaServer::PhoneUI

    Together with CallContactListView, CallContactModel it
    provides a model-view representation for call contact information.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
    Creates a new delegate with the given \a parent.
*/
CallContactDelegate::CallContactDelegate( QObject * parent)
    : QContactDelegate(parent)
{
}

/*!
    Destroys the delegate.
*/
CallContactDelegate::~CallContactDelegate() {}

/*!
    \reimp
*/
QFont CallContactDelegate::secondaryFont(const QStyleOptionViewItem& o, const QModelIndex& idx) const
{
    QFont font = QContactDelegate::secondaryFont(o, idx);
    QFont f(font);
    f.setItalic(true);
    return f;
}


//==============================================================


/*!
    \class CallContactListView
    \inpublicgroup QtTelephonyModule
    \brief The CallContactListView class provides a view for call entries.
    \ingroup QtopiaServer::PhoneUI

    The view is based on a QSmoothList and provides a visual representation of call records.
    It uses CallContactModel to manage the underlying data and uses CallContactDelegate to
    render the content.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
    \fn void CallContactListView::hangupActivated()

    Emitted upon reception of a hangup request by the user.
*/

/*!
    \fn void CallContactListView::requestedDial(const QString& number, const QUniqueId& contactId)

    Emitted when the has requested to dial a \a number associated with \a contactId.
*/

/*!
    Creates a new view with the given \a parent.
*/
CallContactListView::CallContactListView(QWidget * parent)
    :QSmoothList(parent)
{
    mMenu = QSoftMenuBar::menuFor( this );

    QIcon addressbookIcon(":image/addressbook/AddressBook");

    mAddContact = mMenu->addAction(addressbookIcon, tr("Save to Contacts"), this, SLOT(addItemToContact()));
    mAddContact->setVisible(false);
    mOpenContact = mMenu->addAction(addressbookIcon, tr("Open Contact"), this, SLOT(openContact()));
    mOpenContact->setVisible(false);
    mSendMessage = mMenu->addAction(QIcon(":icon/txt"), tr("Send Message"), this, SLOT(sendMessageToItem()));
    mSendMessage->setVisible(false);
    mRelatedCalls = mMenu->addAction(QIcon(":icon/view"), tr("Related Calls"), this, SLOT(showRelatedCalls()));
    mRelatedCalls->setVisible(false);
    mAllCalls = mMenu->addAction(QIcon(":icon/view"), tr("All Calls"), this, SLOT(showAllCalls()));
    mAllCalls->setVisible(false);

    m_noResultMessage = tr("No matches.");

    connect(this, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(updateMenu()));
}

/*!
    Destroys the view.
*/
CallContactListView::~CallContactListView() {}

/*!
    \reimp
*/
void CallContactListView::paintEvent( QPaintEvent *pe )
{
    if (cclm && !cclm->rowCount())
        setEmptyText(cclm->filter().isEmpty() ? tr("No Items"): m_noResultMessage);
    QSmoothList::paintEvent(pe);
    /*if (cclm && !cclm->rowCount())
    {
        QWidget *vp = this;
        QPainter p( vp );
        QFont f = p.font();
        f.setBold(true);
        f.setItalic(true);
        p.setFont(f);
        p.drawText( 0, 0, vp->width(), vp->height(), Qt::AlignCenter,
                (cclm->filter().isEmpty() ? tr("No Items"): m_noResultMessage) );
    }*/
}

/*!
    Sets the \a model for the view.
*/
void CallContactListView::setModel(QAbstractItemModel* model)
{
    cclm  = qobject_cast<CallContactModel*>(model);
    if (!cclm)
    {
        qWarning("CallContactListView::setModel(): expecting model of type CallContactModel");
    }
    QSmoothList::setModel(model);
    updateMenu(); // cclm changed
}

/*!
    Adds the currently selected call number to a contact.
*/
void CallContactListView::addItemToContact()
{
    QModelIndex idx = currentIndex();
    CallContactItem * cci = cclm->itemAt(idx);
    if (!cci)
        return;

    QString number = cci->text();
    if (cci->fieldType() == QContactModel::Invalid && !number.isEmpty())
        SavePhoneNumberDialog::savePhoneNumber(number);
}

/*!
    Shows the contact information for the currently selected entry. If the current
    entry doesn't have any contact information this function does nothing.
*/
void CallContactListView::openContact()
{
    QModelIndex idx = currentIndex();
    CallContactItem* cci = cclm->itemAt(idx);
    if (!cci)
        return;

    QUniqueId cntID = cci->contactID();
    if (!cntID.isNull())
    {
        QtopiaServiceRequest req( "Contacts", "showContact(QUniqueId)" );
        req << cntID;
        req.send();
    }
}

/*!
    Sends a message to the currently selected entry.
*/
void CallContactListView::sendMessageToItem()
{
    QModelIndex idx = currentIndex();
    CallContactItem* cci = cclm->itemAt(idx);
    if (!cci)
        return;

    QString name = cci->text();
    QString number = cci->number();

    if (!number.isEmpty()) {
        QtopiaServiceRequest req( "SMS", "writeSms(QString,QString)");
        req << name << number;
        req.send();
        // XXX what about atachments
    }
}

/*!
    The view ionly shows call entries which are related to the currently selected entry
    (e.g. have the same number).
*/
void CallContactListView::showRelatedCalls()
{
    QModelIndex idx = currentIndex();
    CallContactItem * cci = cclm->itemAt(idx);
    if ( !cci )
        return;
    if ( cci->contactID().isNull() )
        cclm->setFilter( cci->number() );
    else
        cclm->setFilter( cci->contact().label() );

    updateMenu();
}

/*!
    Reverts any currently applied filter settings such as those applied by
    calling showRelatedCalls().
*/
void CallContactListView::showAllCalls()
{
    cclm->setFilter( QString() );
    updateMenu();
}

/*!
    Updates the actions in the context menu based on the current selection.
*/
void CallContactListView::updateMenu()
{
    mSendMessage->setEnabled(false);
    mSendMessage->setVisible(false);
    mAddContact->setEnabled(false);
    mAddContact->setVisible(false);
    mOpenContact->setEnabled(false);
    mOpenContact->setVisible(false);
    mRelatedCalls->setEnabled(false);
    mRelatedCalls->setVisible(false);
    mAllCalls->setEnabled(false);
    mAllCalls->setVisible(false);

    if ( !cclm->filter().isEmpty() ) {
        mAllCalls->setEnabled(true);
        mAllCalls->setVisible(true);
    }

    if (Qtopia::mousePreferred()) // No way to select item without activating
        return;

    CallContactItem* cci = cclm->itemAt(currentIndex());
    if (!cci)
        return;

    QContactModel::Field fieldType = cci->fieldType();
    if ( fieldType == QContactModel::HomeMobile ||
         fieldType == QContactModel::BusinessMobile )
    {
        mSendMessage->setEnabled(true);
        mSendMessage->setVisible(true);
    }

    if ( !cci->contactID().isNull() ) {
        mOpenContact->setEnabled(true);
        mOpenContact->setVisible(true);
    }

    if ( fieldType == QContactModel::Invalid
        && !cci->number().trimmed().isEmpty() ) {
        mAddContact->setVisible(true);
        mAddContact->setEnabled(true);
    }

    if ( cci->number().trimmed().isEmpty() )
        return;

    if ( cclm->filter().isEmpty() ) {
        mRelatedCalls->setEnabled(true);
        mRelatedCalls->setVisible(true);
    }
}

/*!
    Returns the phone number at position \a idx.
*/
QString CallContactListView::numberForIndex(const QModelIndex & idx) const
{
    QString number;
    CallContactItem* cci = cclm->itemAt(idx);
    if (!cci)
        return number;
    number = cci->number();
    return number;
}

/*!
    Returns the contact at position \a idx.
*/
QContact CallContactListView::contactForIndex(const QModelIndex & idx) const
{
    CallContactItem* cci = cclm->itemAt(idx);
    if (!cci)
        return QContact();
    return cci->contact();
}

/*!
    Reset the internal state of the list.
*/
void CallContactListView::reset()
{
    QSmoothList::reset();
}

/*!
    \reimp
*/
void CallContactListView::keyPressEvent( QKeyEvent *e )
{
    int key = e->key();
    if (key == Qt::Key_Call || key == Qt::Key_Yes) {
        QModelIndex idx = currentIndex();
        if( idx.isValid() )
            emit requestedDial( numberForIndex(idx), contactForIndex(idx).uid() );
        else
            emit hangupActivated();
        e->accept();
    } else if (key == Qt::Key_Hangup  || key == Qt::Key_Back || key == Qt::Key_No) {
        emit hangupActivated();
        e->accept();
    } else if (key == Qt::Key_Flip) {
        QSettings cfg("Trolltech","Phone");
        cfg.beginGroup("FlipFunction");
        if (cfg.value("hangup").toBool()) {
            emit hangupActivated();
            e->accept();
        }
    } else {
        QSmoothList::keyPressEvent( e );
    }
}

/*!
    \reimp
*/
void CallContactListView::focusInEvent( QFocusEvent *focusEvent)
{
    QSmoothList::focusInEvent( focusEvent );
    setEditFocus( true );
}

/*!
    \reimp
*/
void CallContactListView::focusOutEvent( QFocusEvent *focusEvent)
{
    QSmoothList::focusOutEvent( focusEvent );
    if (Qtopia::mousePreferred())
        clearSelection();
}

/*!
    Sets the \a newMessage to be shown while the list is empty.
*/
void CallContactListView::setEmptyMessage(const QString& newMessage)
{
    m_noResultMessage = newMessage;
}

