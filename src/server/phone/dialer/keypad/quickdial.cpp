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

#include "numberdisplay.h"
#include "quickdial.h"

#include "callcontactlist.h"
#include "dialercontrol.h"
#include "dtmfaudio.h"
#include "servercontactmodel.h"
#include "qtopiaserverapplication.h"
#ifdef QTOPIA_TELEPHONY
#include "abstractdialfilter.h"
#endif

#include <qphonenumber.h>
#include <qsoftmenubar.h>
#include <quniqueid.h>

#include <QLayout>
#include <QDebug>
#include <QKeyEvent>
#include <QTimer>


// declare QuickDialModel
class QuickDialModel : public CallContactModel
{
    Q_OBJECT
public:
    QuickDialModel( QCallList& callList, QObject *parent = 0 );

    void init();

    void setWildcardNumberActive(bool);
    bool wildcardNumberActive() const;

public slots:
    void refresh();
    void populate();

private:
    QContactModel *clm;
    bool wcActive;
    bool activated;
};

// declare QuickDialContactView
class QuickDialContactView : public CallContactListView
{
    Q_OBJECT
public:
    QuickDialContactView(QWidget *parent);

private slots:
    void selectedNumber(const QModelIndex& idx);

signals:
    void numberSelected(const QString&, const QUniqueId&);
};

QuickDialContactView::QuickDialContactView(QWidget *parent)
    : CallContactListView(parent)
{
    connect( this, SIGNAL(activated(QModelIndex)), this, SLOT(selectedNumber(QModelIndex)) );
}

void QuickDialContactView::selectedNumber(const QModelIndex& idx)
{
    /* sub view instead if QContact */
    CallContactItem* cci = cclm->itemAt(idx);
    if (cci && !cci->contactID().isNull()) {
        /* should be able to select from list of numbers , if there is more than one */
        /* open dialog listing phone numbers of selected contact */

        QContact ent = cci->contact();

        QMap<QContact::PhoneType, QString> numbers = ent.phoneNumbers();

#if !defined(QTOPIA_VOIP)
        // If we don't have VOIP, we can't dial VOIP numbers
        numbers.remove(QContact::HomeVOIP);
        numbers.remove(QContact::BusinessVOIP);
        numbers.remove(QContact::VOIP);
#endif

        if (numbers.count() == 1) {
            QMap<QContact::PhoneType, QString>::iterator it = numbers.begin();
            emit numberSelected(it.value(), ent.uid());
        } else {
            QPhoneTypeSelector s(ent, QString());
            if (QtopiaApplication::execDialog(&s) && !s.selectedNumber().isEmpty())
                emit numberSelected(s.selectedNumber(), ent.uid());
        }
    } else {
        emit numberSelected(cci->number(), QUniqueId());
    }
}

//---------------------------------------------------------------------------

QuickDialModel::QuickDialModel( QCallList& callList, QObject *parent )
    : CallContactModel( callList, parent ), clm(0), wcActive(false), activated(false)
{
}

void QuickDialModel::init()
{
    wcActive = false;
    activated = false;

    CallContactModel::resetModel(); //delete existing items
    reset();
}

void QuickDialModel::setWildcardNumberActive(bool b)
{
    if (wcActive == b)
        return;
    wcActive = b;
    refresh();
}

bool QuickDialModel::wildcardNumberActive() const
{
    return wcActive;
}

void QuickDialModel::refresh()
{
    QString filStr(filter());
    if (filStr.isEmpty()) {
        // Don't show the initial unfiltered list, because we will have some filtering
        // data arrive immediately afterward
        if (!activated)
            return;
    } else {
        activated = true;
    }

    if (wcActive) {
        populate();
        return;
    }

    if (!clm) {
        clm = new ServerContactModel;
        clm->setParent( this );
        connect(clm, SIGNAL(modelReset()), this, SLOT(populate()));
    }

    if (filStr != clm->filterText()) {
        clm->setFilter(filStr, QContactModel::ContainsPhoneNumber);
    } else {
        populate();
    }
}

void QuickDialModel::populate()
{
    CallContactModel::resetModel(); //delete existing items
    CallContactModel::refresh(); //reread CallListItems

    QList<QCallListItem> cl = mRawCallList;

    //create contacts that match - alphabetical order
    //const int numContacts = mAllContacts.count();
    const QString filStr = filter();
    const int filLen = filStr.length();

#ifndef QTOPIA_HOMEUI
    // Only match contacts if we have a filter
    if( !clm->filterText().isEmpty() ) {
#endif

    int numMatchingEntries = 0; //limit number of entries in list
    int index = 0;
    if (!wcActive) {
        QUniqueId cntid = clm->id(index++);
        while (!cntid.isNull())
        {
            QCallListItem clItem;
            CallContactItem* newItem = 0;

            // assumed matched by label...
            newItem = new CallContactItem(clItem, this);
            newItem->setContact(clm, cntid);
            callContactItems.append(newItem);

            numMatchingEntries++;

            /* should only sow enough to fit on the screen,
               although perhaps have a way of searching out the rest
               later.  Scrolling through 3000+ item != quick */
            if (numMatchingEntries > 20)
                break;
            cntid = clm->id(index++);
        }
    }

#ifndef QTOPIA_HOMEUI
    }
#endif

    //create remaining calllist items
    if (filLen==0 || filLen >= 3) {
        foreach(QCallListItem clItem, cl)
        {
            if( clItem.isNull() )
                continue;

            QString number = clItem.number();
            if( filLen == 0 ||
                (QPhoneNumber::matchPrefix( clItem.number(), filStr ) != 0) )
            {
                CallContactItem *newItem = new CallContactItem(clItem, this);
                callContactItems.append(newItem);
            }
        }
    }

    // Tell the list our content has changed
    reset();
}

//---------------------------------------------------------------------------

/*!
    \class PhoneQuickDialerScreen
    \inpublicgroup QtTelephonyModule
    \brief The PhoneQuickDialerScreen class implements a keypad based dialer UI.
    \ingroup QtopiaServer::PhoneUI

    An image of this dialer screen can be found in
    the \l{Server Widget Classes}{server widget gallery}.

    This class is a Qt Extended \l{QtopiaServerApplication#qt-extended-server-widgets}{server widget}.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa QAbstractServerInterface, QAbstractDialerScreen
  */


/*!
  \fn void PhoneQuickDialerScreen::numberSelected(const QString&, const QUniqueId&)

  \internal
*/

/*!
  Constructs a new PhoneQuickDialerScreen object with the specified
  \a parent and widget flags \a fl.
*/
PhoneQuickDialerScreen::PhoneQuickDialerScreen( QWidget *parent, Qt::WFlags fl )
    : QAbstractDialerScreen( parent, fl ), mSpeedDial( false )
      , delayedDialTimer( 0 ), delayedDial( false )
{
    QCallList &callList = DialerControl::instance()->callList();
    QVBoxLayout *l = new QVBoxLayout( this );
    l->setContentsMargins(0, 0, 0, 0);

    mNumberDS = new NumberDisplay( this );
    l->addWidget(mNumberDS);
    QtopiaApplication::setInputMethodHint( mNumberDS, QtopiaApplication::AlwaysOff );

    QSoftMenuBar::setLabel( this, Qt::Key_Select, "phone/calls" , tr( "Dial", "dial highlighted number" ) );
    QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Cancel );
    mDialList = new QuickDialContactView( this );
    l->addWidget(mDialList);
    mDialList->setEmptyMessage( tr("Type in phone number.") );

    mDialModel = new QuickDialModel(callList, mDialList);
    mDialModel->init();
    mDialList->setModel(mDialModel);
    CallContactDelegate * delegate = new CallContactDelegate( mDialList );
    mDialList->setItemDelegate( delegate );

    connect( mDialList, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
        mDialList, SLOT(updateMenu()) );

    connect( mNumberDS, SIGNAL(numberChanged(QString)), this,
                                                    SLOT(rejectEmpty(QString)) );
    connect( mNumberDS, SIGNAL(speedDialed(QString)), this,
                                    SIGNAL(speedDial(QString)) );
    connect( mNumberDS, SIGNAL(numberSelected(QString)), this,
                                    SLOT(selectedNumber(QString)) );
    connect( mNumberDS, SIGNAL(hangupActivated()), this, SLOT(close()) );

    connect( this, SIGNAL(numberSelected(QString,QUniqueId)), this,
                                            SIGNAL(requestDial(QString,QUniqueId)) );

    connect( mDialList, SIGNAL(requestedDial(QString,QUniqueId)), mDialList,
                                    SIGNAL(numberSelected(QString,QUniqueId)) );
    connect( mDialList, SIGNAL(numberSelected(QString,QUniqueId)), this,
                                    SLOT(selectedNumber(QString,QUniqueId)) );
    connect( mDialList, SIGNAL(hangupActivated()), this, SLOT(close()) );
    setWindowTitle( tr("Quick Dial") );

    mNumberDS->installEventFilter( this );
    mDialList->installEventFilter( this );
    // Set the dialog to the maximum possible size.

    dtmf = qtopiaTask<DtmfAudio>();
    if (dtmf)
        connect( mNumberDS, SIGNAL(numberKeyPressed(int)),
            dtmf, SLOT(playDtmfKeyTone(int)) );
    else
        qLog(Component) <<"PhoneQuickDialerScreen: DtmfAudio not available";


    if ( Qtopia::hasKey( Qtopia::Key_Hook ) ) {
        delayedDialTimer = new QTimer( this );
        delayedDialTimer->setSingleShot( true );
        delayedDialTimer->setInterval( 3000 );
        connect( delayedDialTimer, SIGNAL(timeout()), this, SLOT(delayedDialTimeout()) );
        connect( mNumberDS, SIGNAL(numberKeyPressed(int)), this, SLOT(numberKeyPressed(int)) );
        connect( mDialModel, SIGNAL(modelReset()), this, SLOT(resetDelayedDialTimer()) );
    }
}

/*!
  Destroys the PhoneQuickDialerScreen object.
  */
PhoneQuickDialerScreen::~PhoneQuickDialerScreen()
{
}

/*! \internal */
void PhoneQuickDialerScreen::showEvent( QShowEvent *e )
{
    QAbstractDialerScreen::showEvent( e );
    if( mNumber.length() )
    {
        // append digits after show event
        mNumberDS->appendNumber( mNumber, mSpeedDial );
        mNumber = QString();
        mSpeedDial = false;
    }
}

/*! \internal */
void PhoneQuickDialerScreen::hideEvent( QHideEvent *e )
{
    // Clear the existing content
    mDialModel->init();

    QAbstractDialerScreen::hideEvent( e );
}

/*! \internal */
void PhoneQuickDialerScreen::selectedNumber( const QString &num )
{
    selectedNumber( num, QUniqueId() );
}

/*! \internal */
void PhoneQuickDialerScreen::selectedNumber( const QString &num, const QUniqueId &cnt )
{
    if ( delayedDialTimer && delayedDialTimer->isActive() )
        delayedDialTimer->stop();

    if( num.isEmpty() )
    {
        close();
        return;
    }
#ifdef QTOPIA_TELEPHONY
    // Filter for special GSM key sequences.
    if ( AbstractDialFilter::defaultFilter() ) {
        AbstractDialFilter::Action act = AbstractDialFilter::defaultFilter()->filterInput(num,true);
        if ( act == AbstractDialFilter::ActionTaken ) {
            mNumber = QString();
            close();
            return;
        }
    }
#endif

    if (num.contains(QChar('d'), Qt::CaseInsensitive) &&
               !num.contains(QChar(':')) && !num.contains(QChar('@'))) {
        mDialModel->setWildcardNumberActive(true);
        mNumberDS->setWildcardNumber(num);
        mNumberDS->setFocus();
    } else {
        mNumber = num;
        close();
        emit numberSelected( mNumber, cnt );
    }
}

/*! \internal */
bool PhoneQuickDialerScreen::eventFilter( QObject *o, QEvent *e )
{
    QEvent::Type t = e->type();
    if( t != QEvent::KeyPress )
        return false;
    QKeyEvent *ke = (QKeyEvent *)e;
    int key = ke->key();
    QChar ch( key );
    if( o == mDialList )
    {
        switch( key )
        {
            case Qt::Key_Up:
            {
                if( !mDialModel->rowCount() ||
                    mDialList->currentIndex() == mDialModel->index(0) )
                {
                    mDialList->clearSelection();
                    mNumberDS->setFocus();
                    mNumberDS->setEditFocus(true);
                    return true;
                }
                break;
            }
            case Qt::Key_Down:
            {
                if( !mDialModel->rowCount() ||
                    mDialList->currentIndex() == mDialModel->index(mDialModel->rowCount()-1) )
                {
                    mDialList->clearSelection();
                    mNumberDS->setFocus();
                    mNumberDS->setEditFocus(true);
                    return true;
                }
                break;
            }
            case Qt::Key_0:
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_8:
            case Qt::Key_9:
            {
                mNumberDS->appendNumber( ke->text() );
                mNumberDS->setFocus();
                mNumberDS->setEditFocus(true);
                return true;
            }
            case Qt::Key_Backspace:
            {
                mNumberDS->setFocus();
                mNumberDS->setEditFocus(true);
                QString curNumber = mNumberDS->number();
                if( !curNumber.isEmpty() )
                    mNumberDS->backspace();
                return true;
            }
            default:
                break;
        }
    }
    else if( o == mNumberDS )
    {
        QModelIndex idx;
        if ( key == Qt::Key_Down || key == Qt::Key_Up ) {
            if ( !mDialModel->rowCount() )
                return true;    // Do not move focus if no matching number exists
            if ( key == Qt::Key_Down )
                idx = mDialModel->index(0);
            else
                idx = mDialModel->index(mDialModel->rowCount()-1);
        }
        if (idx.isValid()) {
            mDialList->setFocus();
            mDialList->setCurrentIndex(idx);
            mDialList->setEditFocus(true);
            return true;
        }
    }
    return false;
}

/*! \reimp */
void PhoneQuickDialerScreen::reset()
{
    mNumberDS->clear();
    mNumberDS->setFocus();
    mDialModel->setWildcardNumberActive(false);
    mNumber = QString();
    mDialList->setCurrentIndex(QModelIndex());
    mDialList->scrollToTop();
}

/*! \internal */
void PhoneQuickDialerScreen::rejectEmpty( const QString &t )
{
    if( t.isEmpty() && !isVisible() ) {
        close();
     } else {
#ifdef QTOPIA_TELEPHONY
        // Fitler special GSM key sequences that act immediately (e.g. *#06#).
        if (AbstractDialFilter::defaultFilter()) {
            AbstractDialFilter::Action action = 
                            AbstractDialFilter::defaultFilter()->filterInput(t);
            if ( action == AbstractDialFilter::ActionTaken ) {
                mNumber = QString();
                close();
            }
        }
#endif

        mDialModel->setFilter(t);
    }
}

/*! \internal */
void PhoneQuickDialerScreen::appendDigits( const QString &digits, bool refresh,
                              bool speedDial )
{
    if( !refresh && isVisible() )
        qWarning("BUG: appending digits that will never be seen to quick dial");
    if( refresh )
        mNumberDS->appendNumber( digits, speedDial );
    else {
        mSpeedDial = mSpeedDial | speedDial;
        mNumber += digits;
    }
}

/*! \reimp */
QString PhoneQuickDialerScreen::digits() const
{
    if( mNumber.isEmpty() )
        return mNumberDS->number();
    return mNumber;
}

/*! \reimp */
void PhoneQuickDialerScreen::setDigits(const QString &digits)
{
    reset();
    appendDigits(digits, false, false);
}

/*! \reimp */
void PhoneQuickDialerScreen::appendDigits(const QString &digits)
{
    appendDigits(digits, false, true);
}

/*! \reimp */
void PhoneQuickDialerScreen::doOffHook()
{
    if ( !delayedDialTimer )
        return;
    delayedDial = true;
    if ( !mNumberDS->number().isEmpty() )
        selectedNumber( mNumberDS->number() );
}

/*! \reimp */
void PhoneQuickDialerScreen::doOnHook()
{
    if ( !delayedDialTimer )
        return;
    delayedDial = false;
}

/*!
  \internal
  */
void PhoneQuickDialerScreen::resetDelayedDialTimer()
{
    if ( delayedDialTimer && delayedDialTimer->isActive() )
        delayedDialTimer->stop();
    if ( !delayedDialTimer || !delayedDial || mDialModel->rowCount() )
        return;
    delayedDialTimer->start();
}

/*!
  \internal
  */
void PhoneQuickDialerScreen::delayedDialTimeout()
{
    if ( !delayedDialTimer || !delayedDial )
        return;
    selectedNumber( mNumberDS->number() );
}

/*!
  \internal
  */
void PhoneQuickDialerScreen::numberKeyPressed( int key )
{
    if ( key == Qt::Key_NumberSign && Qtopia::hasKey( Qtopia::Key_Hook ) ) {
        if ( delayedDialTimer && delayedDialTimer->isActive() )
            delayedDialTimer->stop();
        delayedDial = false;
        selectedNumber( mNumberDS->number() );
    }
}

QTOPIA_REPLACE_WIDGET_WHEN(QAbstractDialerScreen, PhoneQuickDialerScreen, Keypad);

#include "quickdial.moc"

