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

#include <QTabWidget>
#include <QLayout>
#include <QAction>
#include <QListWidget>
#include <QDebug>
#include <QLineEdit>
#include <QTextEntryProxy>
#include <QMenu>
#include <QKeyEvent>
#include <QLabel>
#include <QFormLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QTimer>
#include <QToolButton>

#include <qtopianamespace.h>
#include <qphonenumber.h>

#include <qsoftmenubar.h>
#include <qtopianamespace.h>
#include <qtopiaservices.h>
#include <qtopiaipcenvelope.h>
#include <QtopiaItemDelegate>

#include "dialercontrol.h"
#include "callhistory.h"
#include "savetocontacts.h"
#include "qabstractmessagebox.h"
#include "servercontactmodel.h"

// -------------------------------------------------------------
// CallHistoryModel

CallHistoryModel::CallHistoryModel( QCallList & callList, QObject* parent )
    : CallContactModel(callList, parent), mType(QCallList::All), mDirty(true)
{
    //alt method would be to listen to QPE/PIM for addedContact
    connect(ServerContactModel::instance(), SIGNAL(modelReset()),
             this, SLOT(updateContacts()));
}

CallHistoryModel::~CallHistoryModel()
{
}

void CallHistoryModel::setType( QCallList::ListType type )
{
    if (mType != type)
        mDirty = true;
    mType = type;
}

QCallList::ListType CallHistoryModel::type() const
{
    return mType;
}

int CallHistoryModel::rowCount(const QModelIndex& index) const
{
    if (mDirty)
        const_cast<CallHistoryModel*>(this)->refresh();
    return CallContactModel::rowCount(index);
}

void CallHistoryModel::updateContacts()
{
    if (!mDirty) {
        mDirty = true;
        QTimer::singleShot(0, this, SLOT(reallyUpdateContacts()));
    }
}

void CallHistoryModel::reallyUpdateContacts()
{
    emit contactsAboutToBeUpdated();
    refresh();
    emit contactsUpdated();
}

void CallHistoryModel::refresh()
{
    mDirty = false;
    CallContactModel::resetModel(); //delete existing items
    CallContactModel::refresh(); //reread CallListItems

    QList<QCallListItem> cl = mRawCallList;
    const QString filStr = filter();
    QContactModel *clm = ServerContactModel::instance();

    //go through all CallListItems
    foreach(QCallListItem  clItem, cl) {

        if (clItem.isNull())
            continue;

        QCallListItem::CallType st = clItem.type();

        if ( mType == QCallList::All )
            ;
        else if ( mType != QCallList::All && (int)CallContactItem::stateToType(st) == (int)mType )
            ;
        else
            continue; //skip items that aren't part of this list type

        QString number = clItem.number();
        const QUniqueId contactId = clItem.contact();

        if (number.startsWith(QLatin1String("sip:")))
            number = number.mid(4);

        //find all contacts that match the user-specified filter and this call item number
        bool hasContact = false;
        QContact cnt;

        if (!contactId.isNull())
            cnt = clm->contact( contactId );
        else if (!number.isEmpty())
            cnt = clm->matchPhoneNumber( number );

        if (!cnt.uid().isNull())  { // QContactModel::search does not match the exact number
            //can there be more than a single entry for one given number???
            if (filStr.isEmpty() || pk_matcher.collate(cnt.label()).startsWith(filStr)) {
                hasContact = true;
                CallContactItem * newItem = new CallContactItem( clItem, this);
                newItem->setContact(cnt, number);
                callContactItems.append(newItem);
            }
        }

        if ( !hasContact && (filStr.isEmpty() || QPhoneNumber::matchPrefix( number, filStr ))) {
            //no contact associated to this QCallListItem
            CallContactItem * newItem = new CallContactItem(clItem, this);
            callContactItems.append(newItem);
        }
    }
    reset(); //update views
}

#ifdef QTOPIA_HOMEUI
// -------------------------------------------------------------
// DeskphoneCallItemDelegate

class DeskphoneCallItemDelegate : public QtopiaItemDelegate
{
    Q_OBJECT
public:
    DeskphoneCallItemDelegate( QObject * parent = 0 )
        : QtopiaItemDelegate(parent)
    {}

    virtual ~DeskphoneCallItemDelegate() {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
    {
        QWidget *w = qobject_cast<QWidget*>(parent());
        QFontMetrics fm(titleFont(option));
        QSize hint(w ? w->width() : 200, fm.height() * 7/4);
        return hint;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItem opt = option;
        bool selected = opt.state & QStyle::State_Selected;
        QFont titleFont = this->titleFont(opt);

        // prepare
        painter->save();
        if (hasClipping())
            painter->setClipRect(opt.rect);

        drawBackground(painter, option, index);

        painter->setRenderHint(QPainter::Antialiasing);

        int iconSize = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);

        QString title, subtitle, duration;
        QIcon icon;
        CallContactItem *item = ((CallContactModel*)index.model())->itemAt(index);
        if (item) {
            QContact c = item->contact();
            QCallListItem clItem = item->callListItem();

            QCallListItem::CallType st = clItem.type();
            if ( st == QCallListItem::Dialed )
                icon = QIcon(":icon/phone/outgoingcall");
            else if ( st == QCallListItem::Received )
                icon = QIcon(":icon/phone/incomingcall");
            else if ( st == QCallListItem::Missed )
                icon = QIcon(":icon/phone/missedcall");

            QDateTime dt = clItem.start();
            QDate callDate = dt.date();
            QTime callTime = dt.time();
            QString when("%1, %2 ");
            when = when.arg(QTimeString::localMD(callDate, QTimeString::Short))
            .arg(QTimeString::localHM(callTime, QTimeString::Medium));
            subtitle = when;

            if (clItem.type() == QCallListItem::Missed) {
                subtitle += tr("(missed)");
            } else if(!clItem.end().isNull()) {
                QString buf;
                const int SECS_PER_HOUR= 3600;
                const int SECS_PER_MIN  = 60;
                int duration = clItem.start().secsTo( clItem.end() );
                int hour = duration/SECS_PER_HOUR;
                int minute = (duration % SECS_PER_HOUR)/SECS_PER_MIN;
                int second = duration % SECS_PER_MIN;
                buf.sprintf( " (%.1d:%.2d:%.2d)", hour, minute, second );
                subtitle += buf;
            }
        }

        //draw call button
	QRect cbrect (callButtonRect(opt.rect));
        if (cbrect.isValid()) {
            QIcon callicon(":icon/deskphone/call_green");
            painter->drawPixmap(cbrect.x() + (cbrect.width()-iconSize)/2,
                                cbrect.y() + (cbrect.height()-iconSize)/2,
                                callicon.pixmap(iconSize, iconSize));
        }

        if (selected)
            painter->setPen(opt.palette.color(QPalette::HighlightedText));
        else
            painter->setPen(opt.palette.color(QPalette::Text));

        //draw call details
        QFontMetrics fm(opt.font);
        int detailsWidth = fm.width(subtitle);
        QRect detailsRect = QRect(0, 0, detailsWidth, opt.rect.height());
        detailsRect.moveLeft(opt.rect.width()-detailsWidth-cbrect.width()-2);
        painter->drawText(detailsRect, Qt::AlignRight | Qt::AlignVCenter, fm.elidedText(subtitle, opt.textElideMode, opt.rect.width()));

        //draw name/number
        QFontMetrics titlefm(titleFont);
        QRect titleRect = opt.rect;
        titleRect.setX(iconSize+8);
        titleRect.setRight(detailsRect.left()-4);
        QString text;
        QVariant value = index.data(Qt::DisplayRole);
        if (value.isValid()) {
            text = value.toString();
        }
        title += text;
        title = titlefm.elidedText(title, Qt::ElideRight, titleRect.width());
        painter->setFont(titleFont);
        painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, title);
        painter->setFont(opt.font);

        // draw call type icon.
        painter->drawPixmap(4, (opt.rect.height()-iconSize)/2, icon.pixmap(QSize(iconSize, iconSize)));

        painter->restore();
    }

    static QRect callButtonRect(const QRect &rect) {
        // FIXME: Uncomment to enable call button to right
//        return QRect(rect.right()-32, rect.y(), 32, rect.height()-1);
        Q_UNUSED(rect);
        return QRect();
    }

private:
    QFont titleFont(const QStyleOptionViewItem &option) const
    {
        QFont fmain = option.font;
        fmain.setWeight(QFont::Bold);
        return fmain;
    }

    QPoint mousePressPos;
};

#include "callhistory.moc"
#endif

// -------------------------------------------------------------
// CallHistoryListView


CallHistoryListView::CallHistoryListView( QWidget *parent, Qt::WFlags /*fl*/ )
: CallContactListView( parent ), mClearList( 0 ), prevRow( -1 ), prevCount( -1 ), contactsChanging( false )
{
}

void CallHistoryListView::setModel(QAbstractItemModel *model)
{
    CallContactListView::setModel(model);
    if(cclm) {
        connect( cclm, SIGNAL(modelReset()), this, SLOT(modelChanged()));
        connect( cclm, SIGNAL(contactsAboutToBeUpdated()), this, SLOT(contactsAboutToChange()));
        connect( cclm, SIGNAL(contactsUpdated()), this, SLOT(contactsChanged()));
        connect( cclm, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(modelChanged()));
        connect( cclm, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(modelChanged()));
    }
}

void CallHistoryListView::refreshModel()
{
    if (cclm)
        cclm->refresh();
}

void CallHistoryListView::updateMenu()
{
    CallContactListView::updateMenu();
    if( mClearList ) {
        if ( cclm->rowCount() )
            mMenu->addAction(mClearList);
        else
            mMenu->removeAction(mClearList);
    }
}

void CallHistoryListView::contactsAboutToChange()
{
    contactsChanging = true;
    CallContactItem *item = cclm->itemAt(currentIndex());
    if (item && isVisible()) {
        prevRow = currentIndex().row();
        prevNumber = item->number();
        prevCount = cclm->rowCount();
    }
}

void CallHistoryListView::contactsChanged()
{
    if (cclm && cclm->rowCount() > 0) {
        QModelIndex idx = cclm->index( 0 );
        if (isVisible()) {   //matches isVisible condition in contactsAboutToChange
            //do a couple sanity checks to see if we can select the previously selected item
            QModelIndex tempidx = cclm->index( prevRow );
            CallContactItem *item = cclm->itemAt(tempidx);
            if (item && prevRow != -1
                     && prevRow == tempidx.row()
                     && prevNumber == item->number()
                     && prevCount == cclm->rowCount()) {
                idx = tempidx;
            }
        }
        if (!Qtopia::mousePreferred())
            setCurrentIndex( idx );
    }
    contactsChanging = false;
}

void CallHistoryListView::modelChanged()
{
    /* Hide the select button if we're empty - cclm should be valid to raise this signal*/
    if (cclm && cclm->rowCount() > 0) {
        QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::View);
        if (!contactsChanging && !Qtopia::mousePreferred()) {
            // select the first item
            QModelIndex idx = cclm->index( 0 );
            setCurrentIndex( idx );
        }
    } else {
        QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    }
}

#ifdef QTOPIA_HOMEUI
void CallHistoryListView::mousePressEvent(QMouseEvent *e)
{
    cpos = e->pos();
    QSmoothList::mousePressEvent(e);
}
#endif

// -------------------------------------------------------------
// CallHistoryView

CallHistoryView::CallHistoryView( QWidget *parent, Qt::WFlags fl )
    : QWidget( parent, fl ), mHaveFocus( false ), mHaveContact( false ),
      mPhoneType( QContactModel::Invalid ), deleteMsg(0), addContactMsg(0),
      mContactBtn(0)
{
    setObjectName( "callhistory-view" );
    mMenu = QSoftMenuBar::menuFor( this );

    QIcon addressbookIcon( ":icon/addressbook/AddressBook" );
    mOpenContact = new QAction( addressbookIcon, tr( "Open Contact" ), this );
    connect( mOpenContact, SIGNAL(triggered()), this, SLOT(openContact()) );
    mAddContact = new QAction( addressbookIcon, tr( "Save to Contacts" ), this );
    connect( mAddContact, SIGNAL(triggered()), this, SLOT(addContact()) );
    mSendMessage = new QAction( QIcon( ":icon/txt" ), tr( "Send Message" ), this );
    connect( mSendMessage, SIGNAL(triggered()), this, SLOT(sendMessage()) );
    mDeleteAction = new QAction( QIcon( ":icon/trash" ), tr( "Delete" ), this );
    connect( mDeleteAction, SIGNAL(triggered()), this, SLOT(deleteItem()) );

    //alt method would be to listen to QPE/PIM for addedContact
    connect(ServerContactModel::instance(), SIGNAL(modelReset()),
             this, SLOT(contactsChanged()));

    //could assume this, but just to be sure the service is available
    mHaveDialer = !QtopiaService::channel("Dialer").isEmpty();

    // add scroll area
    QVBoxLayout *vl = new QVBoxLayout( this );
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);
    QScrollArea *scrollArea = new QScrollArea( this );
    scrollArea->setWidgetResizable( true );
    scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    scrollArea->setFrameStyle( QFrame::NoFrame );

    // container widget
    QWidget *container = new QWidget( this );
    QFormLayout *l = new QFormLayout( container );

    // new widgets
    mName = new QLabel( container );
    //mName->setWordWrap( true );
    mContactTypePic = new QLabel( container );
    mPhoneTypePic = new QLabel( container );
    mNumber = new QLabel( container );
    mPortrait = new QLabel( container );
    mStartDate = new QLabel( container );
    mStartTime = new QLabel( container );
    mDuration = new QLabel( container );
    mTimeZone = new QLabel( container );
    mTimeZoneLabel = new QLabel( tr("Time Zone:"), container );

    //heading
    QHBoxLayout *h = 0;
#ifdef QTOPIA_HOMEUI
    mCallTypeHeader = new QToolButton(container);
    QPalette pal = mCallTypeHeader->palette();
    pal.setBrush(QPalette::Button, pal.brush(QPalette::Base));
    pal.setColor(QPalette::ButtonText, pal.color(QPalette::Text));
    mCallTypeHeader->setPalette(pal);
    mCallTypeHeader->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mCallTypeHeader->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    vl->addWidget(mCallTypeHeader);
#else
    h = new QHBoxLayout();
    h->addStretch();
    mCallTypePic = new QLabel( container );
    mCallType = new QLabel( container );
    h->addWidget( mCallTypePic );
    h->addWidget( mCallType );
    h->addStretch();
    l->addRow( h );
#endif

    l->addRow( mPortrait );

    h = new QHBoxLayout();
    h->addWidget( mName );
    h->addWidget( mContactTypePic );
    h->addStretch();
    if (style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
        mContactBtn = new QPushButton( tr( "Save to Contacts", "dial highlighted number" ), container );
        mContactBtn->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        h->addWidget( mContactBtn, 1 );
    }
    l->addRow( tr("Name:"), h );
    h = new QHBoxLayout();
    h->addWidget( mNumber );
    h->addWidget( mPhoneTypePic );
    h->addStretch();
    if (style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
        QPushButton *dialBtn = new QPushButton( tr( "Dial", "dial highlighted number" ), container );
        dialBtn->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        h->addWidget( dialBtn, 1 );
        connect( dialBtn, SIGNAL(released()), this, SLOT(dialNumber()) );
        if (mContactBtn) {
            // Give the buttons an equal width
            int size = qMax(mContactBtn->sizeHint().width(), dialBtn->sizeHint().width()) + 4;
            mContactBtn->setMaximumWidth(size);
            dialBtn->setMaximumWidth(size);
        }
    }
    l->addRow( tr("Number:"), h );
    l->addRow( tr("Date:"), mStartDate );
    l->addRow( tr("Time:"), mStartTime );
    l->addRow( tr("Duration:", "Duration of phone call"), mDuration );
    l->addRow( mTimeZoneLabel, mTimeZone );

    vl->addWidget( scrollArea );
    scrollArea->setWidget( container );

    updateMenu();

    setWindowTitle( tr("Call History") );
}

void CallHistoryView::openContact()
{
    if( mHaveContact ) {
        QtopiaServiceRequest req( "Contacts", "showContact(QUniqueId)" );
        req << mContact.uid();
        req.send();
    }
}

void CallHistoryView::addContact()
{
    if (!mHaveContact)
        SavePhoneNumberDialog::savePhoneNumber(mCallListItem.number());
}

void CallHistoryView::contactsChanged()
{
    QString number = mCallListItem.number().trimmed();
    if (number.isEmpty())
        return;

    QContact c = ServerContactModel::instance()->matchPhoneNumber(number);
    if (!mHaveContact && c != QContact()) {         //adding contact
        //QContact c = ServerContactModel::instance()->matchPhoneNumber(number);
        //using cci is easiest way to get fieldType
        CallContactItem *cci = new CallContactItem(mCallListItem);
        cci->setContact(c, number);
        setContact(c, cci->fieldType());
    } else if (mHaveContact && c == QContact()) {  //deleting contact
        mContact = QContact();
        mPhoneType = QContactModel::Invalid;
        mHaveContact = false;
    }
    update();
}

void CallHistoryView::sendMessage()
{
    QString name = mName->text();
    if( mHaveContact )
        name = mContact.label();

    // ugly hack - send to the number recorded in this entry.
    QString number = mNumber->text();

    if (!number.isNull() && !number.isEmpty()) {
        QtopiaServiceRequest req( "SMS", "writeSms(QString,QString)");
        req << name << number;
        req.send();
    }
}

void CallHistoryView::updateMenu()
{
    mMenu->clear();
    if ( !mContactBtn ) {
        if( mHaveContact ) {
            // allow send sms regardless of type of phone number.
            mMenu->addAction(mSendMessage);
            mMenu->addAction(mOpenContact);
        } else {
            if ( !mCallListItem.number().trimmed().isEmpty() ) {
                // allow send sms regardless of type of phone number.
                mMenu->addAction(mSendMessage);
                mMenu->addAction(mAddContact);
            }
        }
    }
    if( mDeleteAction )
        mMenu->addAction(mDeleteAction);

    if ( !mCallListItem.number().trimmed().isEmpty() && mHaveDialer )
        QSoftMenuBar::setLabel( this, Qt::Key_Select, "phone/calls" , tr( "Dial", "dial highlighted number" ) );
    else
        QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );
}

CallHistoryView::~CallHistoryView()
{
}

void CallHistoryView::deleteItem()
{
    if( !mDeleteAction )
        return;

    if ( deleteMsg ) {
        delete deleteMsg;
        deleteMsg = 0;
    }
    deleteMsg = QAbstractMessageBox::messageBox( this, tr("Delete Call History Item"),
        "<qt>" + tr("Are you sure you want to delete this item from the call history?") + "</qt>",
        QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No );
    connect(deleteMsg, SIGNAL(finished(int)), this, SLOT(deleteItem(int)));
    QtopiaApplication::showDialog(deleteMsg);
}

void CallHistoryView::deleteItem(int r)
{
    if ( r == QAbstractMessageBox::Yes ) {
        close();
        emit deleteCurrentItem();
    }
}

QContact CallHistoryView::contact( QContactModel::Field &phoneType ) const
{
    phoneType = mPhoneType;
    return mContact;
}

void CallHistoryView::setContact( const QContact &cnt, QContactModel::Field phoneType )
{
    mContact = ServerContactModel::instance()->contact( cnt.uid() );
    mPhoneType = phoneType;
    mHaveContact = true;
    updateMenu();
}

void CallHistoryView::setCallListItem( QCallListItem item )
{
    mCallListItem = item;
    updateMenu();
}

void CallHistoryView::clear()
{
    mHaveContact = false;
    mContact = QContact();
    mCallListItem = QCallListItem();
    updateMenu();
}

void CallHistoryView::update()
{
    // find call type icon
    QString callTypeFileName;
    QString callTypeString;
    switch ( mCallListItem.type() ) {
    case QCallListItem::Dialed:
        callTypeFileName = ":icon/phone/outgoingcall";
        callTypeString = tr("Outgoing Call");
        break;
    case QCallListItem::Received:
        callTypeFileName = ":icon/phone/incomingcall";
        callTypeString = tr("Incoming Call");
        break;
    case QCallListItem::Missed:
        callTypeFileName = ":icon/phone/missedcall";
        callTypeString = tr("Missed Call");
        break;
    }

    // find phone type icon
    QString phoneTypeFileName;
    if ( mHaveContact && mPhoneType != QContactModel::Invalid ) {
        phoneTypeFileName = QContactModel::fieldIdentifier( mPhoneType );
        if (!phoneTypeFileName.isEmpty())
            phoneTypeFileName.prepend(":icon/addressbook/");
    }

    // find a contact photo or generic photo
    QString photoFileName;
    bool hasPhoto = false;
    if ( mHaveContact ) {
        photoFileName = mContact.portraitFile();
        QString baseDirStr = Qtopia::applicationFileName( "addressbook", "contactimages/" );
        if ( !photoFileName.isEmpty() ) {
            photoFileName = baseDirStr + photoFileName;
            hasPhoto = true;
        } else {
            if ( ServerContactModel::instance()->isSimCardContact(mContact.uid()) )
                photoFileName = ":icon/addressbook/sim-contact";
            else
                photoFileName = ":icon/addressbook/generic-contact";
        }
    }

    // calculate duration
    QString buf;
    if( !mCallListItem.end().isNull() && mCallListItem.type() != QCallListItem::Missed ) {
        const int SECS_PER_HOUR= 3600;
        const int SECS_PER_MIN  = 60;
        int duration = mCallListItem.start().secsTo( mCallListItem.end() );
        int hour = duration/SECS_PER_HOUR;
        int minute = (duration % SECS_PER_HOUR)/SECS_PER_MIN;
        int second = duration % SECS_PER_MIN;
        buf.sprintf( "%.2d:%.2d:%.2d", hour, minute, second );
    }

    // set values
    QIcon callTypeIcon(callTypeFileName);
#ifdef QTOPIA_HOMEUI
    mCallTypeHeader->setText(callTypeString);
    mCallTypeHeader->setIcon(callTypeIcon);
#else
    mCallType->setText( callTypeString );
    mCallTypePic->setPixmap( callTypeIcon.pixmap( QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)) );
#endif

    if (mHaveContact) {
        mName->setText( mContact.label() );
        if (mContactBtn) {
            mContactBtn->setText(tr("Open Contact"));
            disconnect( mContactBtn, SIGNAL(released()), 0, 0);
            connect( mContactBtn, SIGNAL(released()), this, SLOT(openContact()) );
        }
    } else {
        mName->setText( tr("Unknown") );
        if (mContactBtn) {
            mContactBtn->setText(tr("Save to Contacts"));
            disconnect( mContactBtn, SIGNAL(released()), 0, 0);
            connect( mContactBtn, SIGNAL(released()), this, SLOT(addContact()) );
        }
    }

    if ( mCallListItem.number().trimmed().isEmpty() )
        mNumber->setText( tr( "Unknown Number" ) );
    else
        mNumber->setText( mCallListItem.number().trimmed() );

    QIcon phoneTypeIcon(phoneTypeFileName);
    mPhoneTypePic->setPixmap( phoneTypeIcon.pixmap( QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)) );

    // elided text for long contact name
    QFontMetrics fm( mName->font() );
    // calculate available width for the label
    int w = rect().width()  // parent rect
            - mName->x()         // starting point of label
            - ( phoneTypeIcon.isNull()
                    ? 0
                    : QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize) ) // icon size
            - 9; // margin
    mName->setText( fm.elidedText( mName->text(), Qt::ElideRight, w ) );

    if (hasPhoto) {
        mPortrait->setPixmap( photoFileName );
        mPortrait->setVisible(true);
        mContactTypePic->clear();
    } else {
        QIcon contactTypeIcon(photoFileName);
        mContactTypePic->setPixmap( contactTypeIcon.pixmap( QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)) );
        mPortrait->clear();
        mPortrait->setVisible(false);
    }
    if ( mCallListItem.start().date() == QDate::currentDate() )
        mStartDate->setText( tr( "Today" ) );
    else if ( mCallListItem.start().date().addDays( 1 ) == QDate::currentDate() )
        mStartDate->setText( tr( "Yesterday" ) );
    else
        mStartDate->setText( QTimeString::numberDateString( mCallListItem.start().date(), QTimeString::Medium ) );
    mStartTime->setText( QTimeString::localHM( mCallListItem.start().time(), QTimeString::Medium ) );
    if ( buf.isEmpty() )
        mDuration->setText( tr( "Did not connect" ) );
    else
        mDuration->setText( buf );
    // add time zone info if it is different to the current one
    QString tzId = mCallListItem.timeZoneId();
    if ( !tzId.isEmpty() && tzId != QTimeZone().current().id() ) {
        QTimeZone tz( tzId.toLatin1().constData() );
        mTimeZoneLabel->setVisible(true);
        mTimeZone->setVisible(true);
        mTimeZone->setText( tz.name() );
    } else {
        mTimeZoneLabel->setVisible(false);
        mTimeZone->setVisible(false);
        mTimeZone->clear();
    }
}

void CallHistoryView::keyPressEvent( QKeyEvent *ke )
{
    // if Select or Call key pressed and the phone number is known, dial the number.
    if( ke->key() == Qt::Key_Call || ke->key() == Qt::Key_Select )
        dialNumber();
    QWidget::keyPressEvent( ke );
}

void CallHistoryView::dialNumber()
{
    if ( !mCallListItem.number().trimmed().isEmpty() ) {
        QtopiaServiceRequest request( "Dialer", "dial(QString,QUniqueId)" );
        request << mCallListItem.number() << mContact.uid();
        request.send();
        emit externalLinkActivated();
        close();
    }
}

// -------------------------------------------------------------
// CallHistoryClearList
#ifndef QTOPIA_HOMEUI
CallHistoryClearList::CallHistoryClearList( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setModal( true );
    showMaximized();
    QVBoxLayout *l = new QVBoxLayout( this );
    mList = new QListWidget( this );
    l->addWidget( mList );

    QListWidgetItem *item = 0;
    item = new QListWidgetItem(tr("All History"));
    item->setIcon(QIcon( ":icon/callhistory/CallHistory" ));
    mList->insertItem(QCallList::All, item);
    item = new QListWidgetItem(tr("Dialed History"));
    item->setIcon(QIcon( ":icon/phone/outgoingcall" ));
    mList->insertItem(QCallList::Dialed, item);
    item = new QListWidgetItem(tr("Received History"));
    item->setIcon(QIcon( ":icon/phone/incomingcall" ));
    mList->insertItem(QCallList::Received, item);
    item = new QListWidgetItem(tr("Missed History"));
    item->setIcon(QIcon( ":icon/phone/missedcall" ));
    mList->insertItem(QCallList::Missed, item);

    connect( mList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(userSelected(QListWidgetItem*)) );

    setWindowTitle( tr("Clear Call History") );
}

void CallHistoryClearList::userSelected(QListWidgetItem * item)
{
    close();
    emit selected((QCallList::ListType)(mList->row(item)));
}

void CallHistoryClearList::setSelected( QCallList::ListType type )
{
    mList->setCurrentRow( (int)type );
}

CallHistoryClearList::~CallHistoryClearList()
{
}
#endif
/*!
    \class PhoneCallHistory
    \inpublicgroup QtTelephonyModule
    \brief The PhoneCallHistory class provides the Qt Extended Phone call history widget.
    \ingroup QtopiaServer::PhoneUI

    An image of this call history can be found in the \l{Server Widget Classes}{server widget gallery}.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  */
// -------------------------------------------------------------
// PhoneCallHistory


/*!
  \fn void PhoneCallHistory::PhoneCallHistory( QWidget* parent, Qt::WFlags fl)
  \internal
*/
PhoneCallHistory::PhoneCallHistory( QWidget *parent, Qt::WFlags fl )
    : QAbstractCallHistory( parent, fl ), mView( 0 ),
      mCallList(DialerControl::instance()->callList()),
      mShowMissedCalls( false ), mAllListShown( false ), mDialedListShown( false ),
      mReceivedListShown( false ), mMissedListShown( false ),
#ifndef QTOPIA_HOMEUI
      mClearList( 0 ),
#endif
      mDialedFindLE( 0 ), mReceivedFindLE( 0 ), mMissedFindLE( 0 ), mDeleteMsg(0)

{
    mAllList = 0;
    mDialedList = 0;
    mReceivedList = 0;
    mMissedList = 0;
    mAllFindLE = 0;
    mDialedFindLE = 0;
    mReceivedFindLE = 0;
    mMissedFindLE = 0;
    mView = 0;
    mTabs = new QTabWidget( this );
    mTabs->setElideMode(Qt::ElideRight);
    connect( mTabs, SIGNAL(currentChanged(int)), this, SLOT(refreshOnFirstShow(int)) );
    connect( mTabs, SIGNAL(currentChanged(int)), this, SLOT(pageChanged(int)));
    if (!style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
        if ( Qtopia::mousePreferred() )
            connect( mTabs, SIGNAL(currentChanged(int)), this, SLOT(focusFindLE(int)) );
        if( Qtopia::mousePreferred() )
            mTabs->setFocusPolicy( Qt::NoFocus );
    }

    QVBoxLayout *l = new QVBoxLayout( this );
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget( mTabs );

    QAction *clearAction = new QAction(QIcon(":icon/trash"), tr("Clear..."), this );
    connect( clearAction, SIGNAL(triggered()), this, SLOT(clearList()) );

#ifndef QTOPIA_HOMEUI
    QAbstractItemDelegate* delegate = new CallContactDelegate(mTabs);
#else
    QAbstractItemDelegate *delegate = new DeskphoneCallItemDelegate(mTabs);
#endif
    constructTab( QCallList::All, clearAction, delegate );
    constructTab( QCallList::Dialed, clearAction, delegate );
    constructTab( QCallList::Received, clearAction, delegate );
    constructTab( QCallList::Missed, clearAction, delegate );

    connect( this, SIGNAL(requestedDial(QString,QUniqueId)), this, SLOT(close()) );

    setWindowTitle( tr("Call History") );
    setObjectName("callhistory");

#ifdef QTOPIA_HOMEUI
    // Show the launcher when the list is visible
    QSoftMenuBar::setLabel(mTabs, Qt::Key_Back, QString(), "Launcher");

    // set dial tone only hint
    QtopiaServiceRequest request( "Dialer", "setDialToneOnlyHint(QString)" );
    request << "callhistory";
    request.send();
#endif

    new CallHistoryService( this );
}

/*!
  \internal
  */
void PhoneCallHistory::constructTab( QCallList::ListType type,
        QAction *clearAction, QAbstractItemDelegate *delegate )
{
#ifdef QTOPIA_HOMEUI
    const bool shortText = true;
#else
    const bool shortText = false;
#endif

    CallHistoryListView *list = 0;
    CallHistoryModel * cclm = 0;

    QWidget *parent = mTabs;
    QLineEdit *findLE = 0;
    QTextEntryProxy *findProxy = 0;
    QLabel *findIcon = 0;
    // only used on touchscreen as parent widget
    QWidget *tab = 0;
    QIcon icon;
    QString text;

    QVBoxLayout *wl = 0;

    if (style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
    } else if( Qtopia::mousePreferred() ) {
        tab = new QWidget( mTabs );
        wl = new QVBoxLayout( tab );
        parent = tab;
    }

    // create call list view
    switch ( type ) {
    case QCallList::All:
        mAllList = new CallHistoryListView( parent );
        list = mAllList;
        icon = QIcon( ":icon/callhistory/CallHistory" );
        if (shortText)
            text = tr("All", "Short for All Calls");
        else
            text = tr("All Calls");
        if (style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
        } else if ( Qtopia::mousePreferred() ) {
            mAllFindLE = new QLineEdit( tab );
            findLE = mAllFindLE;
        } else {
            mAllFindProxy = new QTextEntryProxy( tab, list );
            findProxy = mAllFindProxy;
            mAllFindIcon = new QLabel( tab );
            findIcon = mAllFindIcon;
        }
        break;
    case QCallList::Dialed:
        mDialedList = new CallHistoryListView( parent );
        list = mDialedList;
        icon = QIcon( ":icon/phone/outgoingcall" );
        if (shortText)
            text = tr("Outgoing", "Short for Outgoing Calls");
        else
            text = tr("Outgoing Calls");
        if (style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
        } else if ( Qtopia::mousePreferred() ) {
            mDialedFindLE = new QLineEdit( tab );
            findLE = mDialedFindLE;
        } else {
            mDialedFindProxy = new QTextEntryProxy( tab, list );
            findProxy = mDialedFindProxy;
            mDialedFindIcon = new QLabel( tab );
            findIcon = mDialedFindIcon;
        }
        break;
    case QCallList::Received:
        mReceivedList = new CallHistoryListView( parent );
        list = mReceivedList;
        icon = QIcon( ":icon/phone/incomingcall" );
        if (shortText)
            text = tr("Incoming", "Short for Incoming Calls");
        else
            text = tr("Incoming Calls");
        if (style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
        } else if ( Qtopia::mousePreferred() ) {
            mReceivedFindLE = new QLineEdit( tab );
            findLE = mReceivedFindLE;
        } else {
            mReceivedFindProxy = new QTextEntryProxy( tab, list );
            findProxy = mReceivedFindProxy;
            mReceivedFindIcon = new QLabel( tab );
            findIcon = mReceivedFindIcon;
        }
        break;
    case QCallList::Missed:
        mMissedList = new CallHistoryListView( parent );
        list = mMissedList;
        icon = QIcon( ":icon/phone/missedcall" );
        if (shortText)
            text = tr("Missed", "Short for Missed Calls");
        else
            text = tr("Missed Calls");
        if (style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
        } else if ( Qtopia::mousePreferred() ) {
            mMissedFindLE = new QLineEdit( tab );
            findLE = mMissedFindLE;
        } else {
            mMissedFindProxy = new QTextEntryProxy( tab, list );
            findProxy = mMissedFindProxy;
            mMissedFindIcon = new QLabel( tab );
            findIcon = mMissedFindIcon;
        }
        break;
    }

    connect( &mCallList, SIGNAL(updated()), list, SLOT(refreshModel()) );

    // set model and delegate
    cclm = new CallHistoryModel(mCallList, list);
    cclm->setType( type );
    list->mClearList = clearAction;
    list->setItemDelegate( delegate );
    list->setModel( cclm );
    connect( cclm, SIGNAL(filtered(QString)),
            this, SLOT(updateTabText(QString)) );

    // add widgets
    if (style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
        tab = list;
    } else if( Qtopia::mousePreferred() ) {
        QtopiaApplication::setInputMethodHint( findLE, "addressbook-names" );
        findLE->installEventFilter( this );
        connect( findLE, SIGNAL(textChanged(QString)), this, SLOT(setFilterCur(QString)) );
        wl->addWidget( findLE ); // WIP - wl is the layout.
        wl->addWidget( list );
        list->setFocusPolicy( Qt::NoFocus );
    } else {
        QWidget *container = new QWidget( this );
        int mFindHeight = findProxy->sizeHint().height();
        findIcon->setPixmap( QIcon( ":icon/find" ).pixmap( mFindHeight - 2, mFindHeight - 2 ) );
        findIcon->setMargin( 2 );
        findIcon->setFocusPolicy( Qt::NoFocus );

        QHBoxLayout *findLayout = new QHBoxLayout;
        findLayout->setMargin(0);
        findLayout->addWidget( findIcon );
        findLayout->addWidget( findProxy );

        QVBoxLayout *containerLayout = new QVBoxLayout( container );
        containerLayout->setMargin(0);
        containerLayout->addWidget( list );
        containerLayout->addLayout( findLayout );

        connect( findProxy, SIGNAL(textChanged(QString)),
                this, SLOT(setFilterCur(QString)) );

        findIcon->setVisible( false );
        findProxy->setVisible( false );

        tab = container;
    }
    mTabs->addTab( tab, icon, text );

    connect( list, SIGNAL(requestedDial(QString,QUniqueId)),
            this, SIGNAL(requestedDial(QString,QUniqueId)) );
    connect( list, SIGNAL(activated(QModelIndex)),
            this, SLOT(viewDetails(QModelIndex)) );
    connect( list, SIGNAL(clicked(QModelIndex)),
            this, SLOT(viewDetails(QModelIndex)) );

    list->installEventFilter( this );
}

/*!
  \fn void PhoneCallHistory::requestedDial(const QString&, const QUniqueId&)

  \internal
  */

/*!
  \fn void PhoneCallHistory::viewedMissedCalls()
  \internal
  */

/*!
  \internal
  */
void PhoneCallHistory::refreshOnFirstShow(int index)
{
    CallHistoryListView * list = 0;

    if (index == 0 && !mAllListShown) {
        mAllListShown = true;
        list = mAllList;
    } else if (index == 1 && !mDialedListShown) {
        mDialedListShown = true;
        list = mDialedList;
    } else if (index == 2 && !mReceivedListShown) {
        mReceivedListShown = true;
        list = mReceivedList;
    } else if (index == 3 && !mMissedListShown) {
        mMissedListShown = true;
        list = mMissedList;
    } else
        return;

    CallHistoryModel* chm = qobject_cast<CallHistoryModel*>(list->model());
    if ( chm ) {
        chm->refresh();
#ifndef QTOPIA_HOMEUI
        if ( qApp->keypadNavigationEnabled() && chm->rowCount() )
            list->setCurrentIndex( chm->index( 0 ) );
#endif
    }
}

/*!
  \internal
*/
void PhoneCallHistory::focusFindLE(int index)
{
    if (index == 0)
        mAllFindLE->setFocus();
    else if (index == 1)
        mDialedFindLE->setFocus();
    else if (index == 2)
        mReceivedFindLE->setFocus();
    else if (index == 3)
        mMissedFindLE->setFocus();
}

/*!
  \internal
  */
void PhoneCallHistory::setFilterCur( const QString &f )
{
    if (style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
    } else if( Qtopia::mousePreferred() ) {
        const QObject *s = sender();
        CallHistoryListView *list = 0;
        if( s == mAllFindLE )
            list = mAllList;
        else if( s == mDialedFindLE )
            list = mDialedList;
        else if( s == mReceivedFindLE )
            list = mReceivedList;
        else if( s == mMissedFindLE )
            list = mMissedList;
        if( list ) {
            CallHistoryModel* chm = qobject_cast<CallHistoryModel*>(list->model());
            if ( chm )
                chm->setFilter( f );
        }
    } else {
        QObject *s = sender();
        if ( s == mAllFindProxy )
            mAllFindIcon->setVisible( !f.isEmpty() );
        else if ( s == mDialedFindProxy )
            mDialedFindIcon->setVisible( !f.isEmpty() );
        else if ( s == mReceivedFindProxy )
            mReceivedFindIcon->setVisible( !f.isEmpty() );
        else if ( s == mMissedFindProxy )
            mMissedFindIcon->setVisible( !f.isEmpty() );
        qobject_cast<QWidget*>(s)->setVisible( !f.isEmpty() );
    }
}

/*!
  \internal
  */
void PhoneCallHistory::clearList()
{
#ifndef QTOPIA_HOMEUI
    if( !mClearList ) {
        mClearList = new CallHistoryClearList( this );
        QtopiaApplication::setMenuLike( mClearList, true );
        connect( mClearList, SIGNAL(selected(QCallList::ListType)), this, SLOT(clearList(QCallList::ListType)) );
    }

    switch( mTabs->currentIndex() ) {
        case 1:
        case 2:
        case 3:
            mClearList->setSelected( (QCallList::ListType)mTabs->currentIndex() );
            break;
        default:
            mClearList->setSelected( QCallList::All );
            break;
    }

    QtopiaApplication::execDialog( mClearList );
#else
    clearList( QCallList::All );
#endif
}

/*!
  \internal
  */
void PhoneCallHistory::clearList( QCallList::ListType type )
{
    // createa message string
    QString itemToDelete = tr("all %1 items", "%1 is either dialed, received, or missed");
    QString itemType;
    switch (type) {
        case QCallList::Dialed:
            itemType = tr("dialed", "inserted into the context: all ... items");
            break;
        case QCallList::Received:
            itemType = tr("received", "inserted into the context: all ... items");
            break;
        case QCallList::Missed:
            itemType = tr("missed", "inserted into the context: all ... items");
            break;
        default: // delete all item
            break;
    }
    if (itemType.isEmpty())
        itemToDelete.remove("%1 ");
    else
        itemToDelete = itemToDelete.arg(itemType);

    if ( mDeleteMsg ) {
        delete mDeleteMsg;
        mDeleteMsg = 0;
    }

    mDeleteType = type;

    // show message abox
    mDeleteMsg = QAbstractMessageBox::messageBox( this, tr("Clear Call History"),
        "<qt>" + tr("Are you sure you want to delete: %1?", "%1=all dialed/received/missed call history items").arg(itemToDelete) + "</qt>",
        QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No );
    connect(mDeleteMsg, SIGNAL(finished(int)), this, SLOT(deleteItems(int)));
    mDeleteMsg->showMaximized();
    QtopiaApplication::showDialog(mDeleteMsg);
}

/*!
  \internal
  */
void PhoneCallHistory::deleteItems(int r)
{
    if (r != QAbstractMessageBox::Yes)
        return;

    QList<CallHistoryListView *> changedLists;

    if( mDeleteType == QCallList::All ) {
        mCallList.clear();
        changedLists.append( mAllList );
        changedLists.append( mDialedList );
        changedLists.append( mReceivedList );
        changedLists.append( mMissedList );
    } else {
        switch ( mDeleteType ) {
        case QCallList::Dialed:
            mCallList.removeAll( QCallListItem::Dialed );
            changedLists.append( mDialedList );
            break;
        case QCallList::Received:
            mCallList.removeAll( QCallListItem::Received );
            changedLists.append( mReceivedList );
            break;
        case QCallList::Missed:
            mCallList.removeAll( QCallListItem::Missed );
            changedLists.append( mMissedList );
            break;
        default:
            break;
        }
        // update all list as well
        changedLists.append( mAllList );
    }

    foreach(CallHistoryListView* list, changedLists) {
        list->refreshModel();
        list->updateMenu();
        CallHistoryModel *chm = qobject_cast<CallHistoryModel*>(list->model());
        if ( chm && !chm->filter().isEmpty() )
            chm->setFilter( QString() );
    }

    // update the value space
    if (mDeleteType == QCallList::All || mDeleteType == QCallList::Dialed) {
        QValueSpaceItem item( "/Communications/Calls" );
        item.setValue("LastDialedCall", "");
    }
}

/*!
  \internal
  */
void PhoneCallHistory::showEvent( QShowEvent *e )
{
    QWidget::showEvent( e );
    if (mShowMissedCalls) {
        mTabs->setCurrentIndex( 3 );
        emit viewedMissedCalls();
    } else {
        if (mTabs->currentIndex() == 0)
            refreshOnFirstShow(0);
        mTabs->setCurrentIndex( 0 );
    }

    if (style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
    } else if (Qtopia::mousePreferred())
        focusFindLE(mTabs->currentIndex());
}

/*!
  \internal
  */
void PhoneCallHistory::cleanup()
{
    if (mView) {
        delete mView;
        mView = 0;
    }
    // cleanup filters
    CallHistoryModel *chm = qobject_cast<CallHistoryModel*>(mAllList->model());
    if ( chm )
        chm->setFilter( QString() );
    chm = qobject_cast<CallHistoryModel*>(mDialedList->model());
    if ( chm )
        chm->setFilter( QString() );
    chm = qobject_cast<CallHistoryModel*>(mReceivedList->model());
    if ( chm )
        chm->setFilter( QString() );
    chm = qobject_cast<CallHistoryModel*>(mMissedList->model());
    if ( chm )
        chm->setFilter( QString() );
}

/*!
  \internal
  */
void PhoneCallHistory::pageChanged(int index)
{
    if( index == 3 )
        emit viewedMissedCalls();
}

/*!
  \internal
  */
void PhoneCallHistory::viewDetails(const QModelIndex& idx)
{
    if (!idx.isValid())
        return;

    CallHistoryListView *list = (CallHistoryListView *)sender();
    if (!list)
        return;

    CallHistoryModel * model = qobject_cast<CallHistoryModel*>(list->model());
    if (!model)
        return;

    CallContactItem *item = model->itemAt(idx);
    if (!item)
        return;

    QCallListItem clItem = item->callListItem();

#ifdef QTOPIA_HOMEUI
    QPoint pos = list->clickPos();
    QRect itemRect = list->visualRect(idx);
    if (DeskphoneCallItemDelegate::callButtonRect(itemRect).contains(pos)) {
        QContact c = item->contact();
        QString number = clItem.number();

        QtopiaServiceRequest req("Dialer", "dial(QString,QUniqueId)"); // No tr
        req << number << c.uid();
        req.send();
        return;
    }
#endif
    if (!mView) {
        mView = new CallHistoryView();
        mView->installEventFilter(this);
        connect(mView, SIGNAL(deleteCurrentItem()), this, SLOT(deleteCurrentItem()));
        connect(mView, SIGNAL(externalLinkActivated()), this, SLOT(close()));
    }

    mView->clear();
    mView->setCallListItem(clItem);

    QContact contact = item->contact();
    if (contact != QContact())
        mView->setContact(contact, item->fieldType());

    mView->showMaximized();
    mView->raise();
    mView->activateWindow();
}

/*!
  \internal
*/
void PhoneCallHistory::viewDetails( QCallListItem item, QContact contact, int fieldType )
{
    mViewedItem = item;

    if ( mView ) {
        delete mView;
        mView = 0;
    }

    if( !mView ) {
        mView = new CallHistoryView();
        mView->installEventFilter(this);
        connect( mView, SIGNAL(deleteCurrentItem()), this, SLOT(deleteViewedItem()) );
        connect( mView, SIGNAL(externalLinkActivated()), this, SLOT(close()) );
    }

    mView->setCallListItem( mViewedItem );
    mView->setContact( contact, (QContactModel::Field)fieldType );
    mView->showMaximized();
}

/*!
  \internal
*/
void PhoneCallHistory::showList( QCallList::ListType type )
{
    if (type == QCallList::Missed)
        mShowMissedCalls = true;
    mTabs->setCurrentIndex( (int)type );
    showMaximized();
    raise();
    activateWindow();
}

/*!
  \internal
  */
bool PhoneCallHistory::eventFilter( QObject *o, QEvent *e )
{
    if( o == mAllList ||
        o == mDialedList ||
        o == mReceivedList ||
        o == mMissedList )
    {
        if( e->type() == QEvent::KeyPress )
        {
            QKeyEvent *ke = (QKeyEvent *)e;
            int key =  ke->key();
            QString text = ke->text();
            CallHistoryListView* chList = qobject_cast<CallHistoryListView*>(o);
            if ( !chList )
                return false;
            CallHistoryModel* chModel = qobject_cast<CallHistoryModel*>(chList->model());
            if( !text.isEmpty() && chModel && chModel->rowCount() )
            {
                mFilters[o] += text;
                chModel->setFilter( mFilters[o] );
                QSoftMenuBar::setLabel( chList, Qt::Key_Back, QSoftMenuBar::BackSpace);
                return false;
            }

            switch( key )
            {
                case Qt::Key_No:
                case Qt::Key_Back:
                {
                    QString filter = mFilters[o];
                    if( !filter.isEmpty() && chModel )
                    {
                        //reset filter
                        mFilters[o] = "";
                        if (style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
                        } else if( !Qtopia::mousePreferred() ) {
                            chModel->setFilter( "" );
                            if ( chList == mAllList )
                                mAllFindProxy->clear();
                            else if ( chList == mDialedList )
                                mDialedFindProxy->clear();
                            else if ( chList == mReceivedList )
                                mReceivedFindProxy->clear();
                            else if ( chList == mMissedList )
                                mMissedFindProxy->clear();
                        } else
                        {
                            if( chList == mAllList )
                                mAllFindLE->setText( QString() );
                            else if( chList == mDialedList )
                                mDialedFindLE->setText( QString() );
                            else if( chList == mReceivedList )
                                mReceivedFindLE->setText( QString() );
                            else if( chList == mMissedList )
                                mMissedFindLE->setText( QString() );
                        }
                        QSoftMenuBar::setLabel( chList, Qt::Key_Back,
                                QSoftMenuBar::Back, QSoftMenuBar::AnyFocus );
                    } else {
                        close();
                    }
                    return true;
                    break;
                }
                case Qt::Key_Backspace:
                {
                    QString filter = mFilters[o];
                    if( !filter.isEmpty() && chModel )
                    {
                        filter = filter.left( filter.length()-1 );
                        chModel->setFilter( filter );
                        mFilters[o] = filter;
                        if( filter.isEmpty() )
                            QSoftMenuBar::setLabel( chList, Qt::Key_Back,
                                    QSoftMenuBar::Back, QSoftMenuBar::AnyFocus );
                    }
                    return true;
                }
                case Qt::Key_Hangup:
                {
                    e->ignore();
                    return true;
                }
                default:
                    break;
            }
        }
    }

    else if( Qtopia::mousePreferred() && (o == mAllFindLE ||
            o == mDialedFindLE ||
            o == mReceivedFindLE ||
            o == mMissedFindLE) && e->type() == QEvent::KeyPress )
    {
        CallHistoryListView *receiver = 0;
        if( o == mAllFindLE )
            receiver = mAllList;
        else if( o == mDialedFindLE )
            receiver = mDialedList;
        else if( o == mReceivedFindLE )
            receiver = mReceivedList;
        else
            receiver = mMissedList;
        QKeyEvent *ke = (QKeyEvent *)e;
        switch( ke->key() )
        {
            case Qt::Key_Context1:
            case Qt::Key_Select:
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Up:
            case Qt::Key_Down:
            {
                QtopiaApplication::postEvent( receiver, new QKeyEvent( ke->type(), ke->key(),
                        ke->modifiers(), ke->text(), ke->isAutoRepeat(), ke->count() ) );
                return true;
            }
            case Qt::Key_Call:
            case Qt::Key_Yes:
            {
                const QModelIndex idx = receiver->currentIndex();
                if (idx.isValid()) {
                    emit requestedDial( receiver->numberForIndex( idx ), receiver->contactForIndex( idx ).uid() );
                }
                return true;
            }
        }
    }
    else if ( o == mView )
    {
        // Need to force update on first paint
        // because the sizeHint of mName is not correctly calculated
        // because of the pixmap next to it.
        static bool forceUpdateOnFirstPaint = true;
        if ( forceUpdateOnFirstPaint && e->type() == QEvent::Paint ) {
            mView->update();
            forceUpdateOnFirstPaint = false;
        }
        if (e->type() == QEvent::WindowActivate)
            mView->update();
    }
    return false;
}

/*!
  \internal
  */
void PhoneCallHistory::reset()
{
    mShowMissedCalls = false;
    mFilters.clear();
    mAllListShown = mDialedListShown = mReceivedListShown = mMissedListShown = false;

    if (style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
    } else if( Qtopia::mousePreferred() ) {
        mAllFindLE->setText( QString() );
        mDialedFindLE->setText( QString() );
        mReceivedFindLE->setText( QString() );
        mMissedFindLE->setText( QString() );
    } else {
        mAllFindProxy->clear();
        mDialedFindProxy->clear();
        mReceivedFindProxy->clear();
        mMissedFindProxy->clear();
    }

    cleanup();
}

/*!
  \internal
  */
void PhoneCallHistory::setFilter( const QString &f )
{
    mFilters[mAllList] = mFilters[mDialedList] = mFilters[mReceivedList] = mFilters[mMissedList] = f;
    if (style()->inherits("QThumbStyle")) { // No find dialog for QThumbStyle
    } else if( !Qtopia::mousePreferred() ) {
        CallHistoryModel *chm = qobject_cast<CallHistoryModel*>(mDialedList->model());
        if ( chm )
           chm->setFilter( f );
        chm = qobject_cast<CallHistoryModel*>(mReceivedList->model());
        if ( chm )
           chm->setFilter( f );
        chm = qobject_cast<CallHistoryModel*>(mMissedList->model());
        if ( chm )
           chm->setFilter( f );
    } else {
        mAllFindLE->setText( f );
        mDialedFindLE->setText( f );
        mReceivedFindLE->setText( f );
        mMissedFindLE->setText( f );
    }
}

/*!
  \internal
  */
void PhoneCallHistory::refresh()
{
    CallHistoryListView *list = 0;
    switch( mTabs->currentIndex() )
    {
        case 0:
            list = mAllList;
            mAllListShown = true;
            break;
        case 1:
            list = mDialedList;
            mDialedListShown = true;
            break;
        case 2:
            list = mReceivedList;
            mReceivedListShown = true;
            break;
        case 3:
            list = mMissedList;
            mMissedListShown = true;
            break;
        default:
            qWarning("BUG: CallHistory::refresh - Unknown active page index %d", mTabs->currentIndex());
            return;
    }
    CallHistoryModel *chm = qobject_cast<CallHistoryModel*>(list->model());
    if ( chm )
       chm->refresh();
}

/*!
  \internal
  */
void PhoneCallHistory::showMissedCalls()
{
    mShowMissedCalls = true;
}

/*!
  \internal
  */
void PhoneCallHistory::deleteCurrentItem()
{
    // find current list
    CallHistoryListView *list = 0;
    switch ( mTabs->currentIndex() ) {
    case 0: list = mAllList; break;
    case 1: list = mDialedList; break;
    case 2: list = mReceivedList; break;
    case 3: list = mMissedList; break;
    default: break;
    }
    if ( !list )
        return;

    // find current QCallListItem
    QModelIndex idx = list->currentIndex();
    CallHistoryModel *chm = qobject_cast<CallHistoryModel*>(list->model());
    if ( !chm )
        return;
    CallContactItem *item = chm->itemAt( idx );
    if ( !item )
        return;
    QCallListItem currentItem = item->callListItem();

    // remove it from the call list
    for( int i = mCallList.count()-1; i >= 0; i--) {
        if( currentItem == mCallList.at(i) ) {
            mCallList.removeAt( i );
            break;
        }
    }

    // refresh current list model and menu
    list->refreshModel();

    // preserve highlighted position
    if ( idx.row() < chm->rowCount() )
        list->setCurrentIndex( idx );
    else // if the last item is deleted select the new last item.
        list->setCurrentIndex( chm->index( chm->rowCount() - 1 ) );

    list->updateMenu();

    // if the current one is all list update list depending on type
    if ( list == mAllList ) {
        switch ( currentItem.type() ) {
        case QCallListItem::Dialed: list = mDialedList; break;
        case QCallListItem::Received: list = mReceivedList; break;
        case QCallListItem::Missed: list = mMissedList; break;
        }
    } else { // if not update all list
        list = mAllList;
    }
    list->refreshModel();
    list->updateMenu();
}

/*!
    \internal
*/
void PhoneCallHistory::deleteViewedItem()
{
    // remove it from the call list
    for( int i = mCallList.count()-1; i >= 0; i--) {
        if( mViewedItem == mCallList.at(i) ) {
            mCallList.removeAt( i );
            break;
        }
    }
}

/*!
    \internal
*/
void PhoneCallHistory::updateTabText( const QString & filterStr )
{
    if ( !filterStr.isEmpty() )
        mTabs->setTabText( mTabs->currentIndex(),
                tr( "Calls: %1", "%1 = phone number or name" ).arg( filterStr ) );
    else {
        switch ( mTabs->currentIndex() ) {
            case 0:
                mTabs->setTabText( 0, tr( "All Calls" ) );
            case 1:
                mTabs->setTabText( 1, tr( "Outgoing Calls" ) );
            case 2:
                mTabs->setTabText( 2, tr( "Incoming Calls" ) );
            case 3:
                mTabs->setTabText( 3, tr( "Missed Calls" ) );
            default:
                break;
        }
    }
}

/*!
    \service CallHistoryService CallHistory
    \inpublicgroup QtTelephonyModule
    \brief The CallHistoryService class provides the CallHistory service.

    The \i CallHistory service enables application to view call history list or detailed view.
*/

/*!
  \fn CallHistoryService::CallHistoryService( PhoneCallHistory* parent )
  \internal
  */

/*!
    \internal
*/
CallHistoryService::~CallHistoryService()
{
}

/*!
    Shows the call history list of \a type. The shown list will only show
    entries that match \a filterHint. If \a filterHint is empty no filtering is applied.

    The service implementation should set the \a filterHint via QAbstractCallHistory::setFilter().
*/
void CallHistoryService::showCallHistory( QCallList::ListType type, const QString& filterHint  )
{
    parent->setFilter( filterHint );
    parent->showList( type );
}

/*!
    Shows the complete call history.

    The service implementation should set the filterHint via QAbstractCallHistory::setFilter().
*/
void CallHistoryService::showCallHistory()
{
    showCallHistory((QCallList::ListType)0, QString());
}

/*!
    Shows all missed calls.

    The service implementation should set the filterHint via QAbstractCallHistory::setFilter().
*/
void CallHistoryService::showMissedCalls()
{
    showCallHistory(QCallList::Missed, QString());
}

/*!
    Shows the detailed view of call history \a item with \a contact and \a fieldType data.

    \a fieldType should be an integer representation
    of QContactModel::Field type for the call number for \a item.
*/
void CallHistoryService::viewDetails( QCallListItem item, QContact contact, int fieldType )
{
    parent->viewDetails( item, contact, fieldType );
}

QTOPIA_REPLACE_WIDGET(QAbstractCallHistory,PhoneCallHistory);
