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

#include "calltypeselector.h"
#include "qabstractcallpolicymanager.h"
#include "uifactory.h"

#include <QtopiaItemDelegate>
#include <QSoftMenuBar>

#include <QBoxLayout>
#include <QKeyEvent>
#include <QMessageBox>
#include <QLabel>
#include <QListWidget>


class CallTypeSelectorPrivate
{
public:
    CallTypeSelectorPrivate()
        : list(0)
    {}

    QListWidget *list;
    QString result;

};

/*!
  \class CallTypeSelector
    \inpublicgroup QtTelephonyModule
  \brief The CallTypeSelector class allows the user to choose what type of telephony call
  he intends to do.
  \ingroup QtopiaServer::GeneralUI

  This dialog assist during the dial process. If a device supports VoIP and GSM/3G calls 
  the user has to chose which type of call he intend to do. 
  
  The \a setAvailablePolicyManagers() function should be called before the dialog is shown. 
  It initializes the list of available call types. Once the user has made the selection \a selectedPolicyManager() returns
  the decision.

    
  \sa QAbstractCallPolicyManager

*/

/*!
  Creates a new CallTypeSelector instance with the given \a parent and \a flags.
  */
CallTypeSelector::CallTypeSelector (QWidget *parent, Qt::WFlags flags)
    : QDialog( parent, flags )
{
    d = new CallTypeSelectorPrivate;

    QVBoxLayout *vbox = new QVBoxLayout(this);
    QWidget *messageArea = new QWidget(this);
    vbox->addWidget(messageArea);

    QHBoxLayout *hb = new QHBoxLayout(messageArea);
    hb->setMargin(6);
    hb->setSpacing(6);
    QLabel *iconLabel = new QLabel(messageArea);
    iconLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    iconLabel->setPixmap( QPixmap(":image/alert_warning") );
    hb->addWidget(iconLabel);

    QLabel *msg = new QLabel(messageArea);
    msg->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    msg->setWordWrap(true);
    msg->setText(tr("Which type of call do you wish to make?"));
    hb->addWidget(msg, 100);

    d->list = new QListWidget(this);
    d->list->setSortingEnabled(true);
    d->list->setItemDelegate(new QtopiaItemDelegate);
    d->list->setFrameStyle(QFrame::NoFrame);
    vbox->addWidget( d->list );

    connect(d->list, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(itemActivated()));

    QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel );
}

/*!
  Destroys the CallTypeSelector instance.
  */
CallTypeSelector::~CallTypeSelector()
{
}

/*!
  This function must be called before the dialog is shown. It initialises the list
  of call managers using \a availableManagers.

  \a availableManagers contains the list of call types that the user can make. A call manager
  is identified via QAbstractCallPolicyManager::callType().

  This function is marked as invokable and can be called via QMetaObject::invokeMethod().

  \sa QAbstractCallPolicyManager::callType()

  */
void CallTypeSelector::setAvailablePolicyManagers( const QStringList& availableManagers )
{
    QList<QAbstractCallPolicyManager *> policies = qtopiaTasks<QAbstractCallPolicyManager>(); 
    foreach ( QAbstractCallPolicyManager *manager, policies ) 
    {
        if ( !availableManagers.contains( manager->callType() ) )
            continue;

        QListWidgetItem *item = new QListWidgetItem(d->list);
        item->setText(manager->trCallType());
        item->setIcon(QIcon(":icon/" + manager->callTypeIcon()));
        item->setData(Qt::UserRole, manager->callType());
        d->list->addItem(item);
    }
    d->list->setCurrentItem(d->list->item(0));
}

/*!
  Returns the selected call manager, e.g. \c VoIP if the user selected a VoIP call.

  This function is marked as invokable and can be called via QMetaObject::invokeMethod().
  */
QString CallTypeSelector::selectedPolicyManager() const
{
    return d->result;
}

void CallTypeSelector::itemActivated()
{
    QListWidgetItem *current = d->list->currentItem();
    if ( current ) {
        d->result = current->data(Qt::UserRole).toString();
    }
    done(QDialog::Accepted);
}

UIFACTORY_REGISTER_WIDGET(CallTypeSelector);
