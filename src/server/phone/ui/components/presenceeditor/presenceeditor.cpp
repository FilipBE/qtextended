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

#include "presenceeditor.h"
#include "uifactory.h"

#include <QtopiaItemDelegate>
#include <QSoftMenuBar>

#include <QBoxLayout>
#include <QKeyEvent>
#include <QMessageBox>
#include <QLabel>
#include <QTreeWidget>
#include <QCollectivePresence>
#include <QCollectivePresenceInfo>
#include <QCommServiceManager>
#include <QDebug>

#define PROTOCOL 0
#define STATUS 1

class PresenceEditorPrivate
{
public:
    PresenceEditorPrivate()
        : tree(0)
    {}

    QTreeWidget *tree;

};

/*!
    \class PresenceEditor
    \inpublicgroup QtIPCommsModule
    \brief The PresenceEditor class allows the user to set VoIP presence status.
    \ingroup QtopiaServer::GeneralUI

    If a device supports VoIP and presence the user can set presence status via this dialog.
*/

/*!
  Creates a new PresenceEditor instance with the given \a parent and \a flags.
  */
PresenceEditor::PresenceEditor(QWidget *parent, Qt::WFlags flags)
    : QDialog( parent, flags )
{
    d = new PresenceEditorPrivate;

    QVBoxLayout *vbox = new QVBoxLayout(this);
    QWidget *messageArea = new QWidget(this);
    vbox->addWidget(messageArea);

    QHBoxLayout *hb = new QHBoxLayout(messageArea);
    hb->setMargin(6);
    hb->setSpacing(6);
    QLabel *iconLabel = new QLabel(messageArea);
    iconLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    iconLabel->setPixmap( QPixmap(":image/alert_info") );
    hb->addWidget(iconLabel);

    QLabel *msg = new QLabel(messageArea);
    msg->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    msg->setWordWrap(true);
    msg->setText(tr("Select presence status."));
    hb->addWidget(msg, 100);

    d->tree = new QTreeWidget(this);
    d->tree->setItemDelegate(new QtopiaItemDelegate);
    d->tree->setFrameStyle(QFrame::NoFrame);
    d->tree->setHeaderHidden(true);
    d->tree->installEventFilter(this);
    vbox->addWidget( d->tree );

    connect(d->tree, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(itemActivated()));

    // TODO: Refactor this to accept a concrete service for settings apps
    populateTree(QString());

    QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel );
}

/*!
  Destroys the PresenceEditor instance.
  */
PresenceEditor::~PresenceEditor()
{
}

void PresenceEditor::populateTree(const QString &service)
{
    QStringList services;

    QCommServiceManager *serviceManager = new QCommServiceManager(this);
    if (service.isEmpty()) {
        if (serviceManager->services().count() == 0) {
            new QTreeWidgetItem(d->tree, QStringList(tr("No presence provider(s)")));
            return;
        }
    } else {
        services.append(service);
    }

    QTreeWidgetItem *firstItem = 0;
    foreach ( QString service, serviceManager->services()) {
        if (!serviceManager->interfaces(service).contains("QCollectivePresence"))
            continue;

        QCollectivePresence *provider = new QCollectivePresence(service, this);
        QTreeWidgetItem *provItem = new QTreeWidgetItem(d->tree,
                QStringList(provider->protocol()), PROTOCOL);
        provItem->setData(0, Qt::UserRole, service);
        d->tree->expandItem(provItem);
        if (!firstItem)
            firstItem = provItem;

        QMap<QString, QCollectivePresenceInfo::PresenceType> statusTypes = provider->statusTypes();
        QMap<QString, QCollectivePresenceInfo::PresenceType>::const_iterator it = statusTypes.constBegin();
        while (it != statusTypes.constEnd()) {
            if ((it.value() != QCollectivePresenceInfo::None) &&
                (it.value() != QCollectivePresenceInfo::Offline))
                new QTreeWidgetItem(provItem, QStringList(it.key()), STATUS);
            ++it;
        }

        delete provider;
    }
    if ( firstItem )
        d->tree->setCurrentItem( firstItem );

    delete serviceManager;
}

void PresenceEditor::itemActivated()
{
    QTreeWidgetItem *current = d->tree->currentItem();
    if ( current ) {
        if ( current->type() == PROTOCOL )
            return;

        QString service = current->parent()->data( 0, Qt::UserRole ).toString();
        QCollectivePresence *provider = new QCollectivePresence(service, this);
        QCollectivePresenceInfo info = provider->localInfo();
        QMap<QString, QCollectivePresenceInfo::PresenceType> statusTypes = provider->statusTypes();
        info.setPresence(current->text(0), statusTypes.value(current->text(0)));
        provider->setLocalPresence(info);
        done(QDialog::Accepted);
    }
}

/*!
    \reimp
*/
bool PresenceEditor::eventFilter( QObject *, QEvent *e )
{
    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        if ( ke->key() == Qt::Key_Back )
            done( QDialog::Accepted );
    }
    return false;
}

UIFACTORY_REGISTER_WIDGET(PresenceEditor);
