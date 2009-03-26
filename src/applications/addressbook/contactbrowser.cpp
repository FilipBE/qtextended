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

#include "contactbrowser.h"
#include "qfielddefinition.h"

#include "contactdocument.h"
#include "ui_actiondialog.h"

#include <QSoftMenuBar>
#include <QtopiaApplication>
#include <QKeyEvent>
#include <QTextCursor>
#include <QTextLine>
#include <QLineEdit>
#include <QPainter>
#include <QVBoxLayout>
#include <QImageSourceSelectorDialog>
#include <QtopiaItemDelegate>
#include <QDL>

/* ContactBrowser */
ContactBrowser::ContactBrowser( QWidget *parent, const char * objectName)
    : QDLBrowserClient( parent, "contactnotes" )
{
    setObjectName(objectName);

    setFrameStyle(NoFrame);
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    connect(this, SIGNAL(highlighted(QString)),
            this, SLOT(linkHighlighted(QString)));

    QSoftMenuBar::setLabel(this, Qt::Key_Back,
        QSoftMenuBar::Back, QSoftMenuBar::AnyFocus);
    QSoftMenuBar::setLabel(this, Qt::Key_Select,
        QSoftMenuBar::NoLabel, QSoftMenuBar::AnyFocus);
    mDocument = NULL;
}

ContactBrowser::~ContactBrowser()
{
}

void ContactBrowser::setSource(const QUrl & name)
{
    linkClicked(name.toString());
}

void ContactBrowser::linkHighlighted(const QString& link)
{
    if (mDocument) {
        ContactDocument::ContactAnchorType cType = mDocument->getAnchorType(link);

        /* Now display the new one */
        switch(cType) {
            case ContactDocument::None:
                setEditFocus(true);
                QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
                break;

            case ContactDocument::DialLink:
                setEditFocus(true);
                QSoftMenuBar::setLabel(this, Qt::Key_Select, "phone/calls", tr("Dial"));
                break;

            case ContactDocument::EmailLink:
                setEditFocus(true);
                QSoftMenuBar::setLabel(this, Qt::Key_Select, "email", tr("Email"));
                break;

            case ContactDocument::QdlLink:
                setEditFocus(true);
                QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::Select);
                break;
            case ContactDocument::CustomLink:
                setEditFocus(true);
                QSoftMenuBar::setLabel(this, Qt::Key_Select, "activate", tr("Activate"));
                break;
        }
    }
    mLink = link;
}

void ContactBrowser::init( const QContact& contact, ContactDocument::ContactDocumentType docType)
{
    mLink.clear();
    if (!mDocument)
        mDocument = new ContactDocument(this);
    mDocument->textDocument()->setTextWidth(width() - 10);
    mDocument->init(this, contact, docType);
    setDocument(mDocument->textDocument());
    loadLinks(contact.customField(QDL::CLIENT_DATA_KEY));
    verifyLinks();
}

QContact ContactBrowser::contact() const
{
    if (mDocument)
        return mDocument->contact();
    return QContact();
}

void ContactBrowser::linkClicked(const QString& link)
{
    if (!mDocument)
        return;

    ContactDocument::ContactAnchorType type = mDocument->getAnchorType(link);
    QString number = mDocument->getAnchorTarget(link);
    QContact contact = mDocument->contact();

    if (type == ContactDocument::DialLink) {
        QDialog diag;
        Ui::ActionDialog ui;
        ui.setupUi(&diag);
        QtopiaApplication::setMenuLike(&diag, true);
        ui.actionList->setItemDelegate(new QtopiaItemDelegate());


        // should add mms? send vcard?
        ui.actionList->addItem( new QListWidgetItem(QIcon(":icon/phone/calls"),
                    tr("Call %1", "call the phone number").arg(number)));
        ui.actionList->addItem( new QListWidgetItem(QIcon(":icon/email"),
                    tr("Text %1", "send a text message to the phone number").arg(number)));
        ui.actionList->setCurrentRow(0);


        if (QtopiaApplication::execDialog(&diag)) {
            if (ui.actionList->currentRow() == 0) {
                QtopiaServiceRequest req( "Dialer", "dial(QString,QUniqueId)" );
                req << number << contact.uid();
                req.send();
                emit externalLinkActivated();
            } else {
                QtopiaServiceRequest req( "SMS", "writeSms(QString,QString)" );
                req << contact.label() << number;
                req.send();
                emit externalLinkActivated();
            }
        }
    } else if (type == ContactDocument::EmailLink) {
        QtopiaServiceRequest req( "Email", "writeMail(QString,QString)" );
        req << contact.label() << number;
        req.send();
        emit externalLinkActivated();
    } else if (type == ContactDocument::QdlLink) {
        activateLink(link);
    } else if (type == ContactDocument::CustomLink) {
        QDialog diag;
        Ui::ActionDialog ui;
        ui.setupUi(&diag);
        QtopiaApplication::setMenuLike(&diag, true);
        ui.actionList->setItemDelegate(new QtopiaItemDelegate());

        QContactFieldDefinition def(mDocument->getAnchorField(link));

        QStringList actionIds = def.browseActions();
        foreach(QString a, actionIds) {
            QString label = QContactFieldDefinition::actionLabel(a);
            QIcon icon = QContactFieldDefinition::actionIcon(a);
            if (label.contains("%1"))
                label = label.arg(number);
            ui.actionList->addItem( new QListWidgetItem(icon, label ));
        }

        ui.actionList->setCurrentRow(0);

        if (QtopiaApplication::execDialog(&diag)) {
            QString chosen = actionIds[ui.actionList->currentRow()];
            QtopiaServiceRequest request = QContactFieldDefinition::actionRequest(chosen, contact, number);
            request.send();
            emit externalLinkActivated();
        }
    }
}

void ContactBrowser::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
        case Qt::Key_Back:
            emit closeView();
            return;
        case Qt::Key_Call:
            if ( mDocument->getAnchorType(mLink) == ContactDocument::DialLink ) {
                QtopiaServiceRequest req( "Dialer", "dial(QString,QUniqueId)" );
                req << mDocument->getAnchorTarget(mLink) << mDocument->contact().uid();
                req.send();
                emit externalLinkActivated();
                return;
            }
        case Qt::Key_Left:
            e->ignore();
            return;
        case Qt::Key_Right:
            e->ignore();
            return;
    }

    QTextBrowser::keyPressEvent(e);
}


QRect ContactBrowser::fakeSelectionRect(QTextDocument* , const QTextCursor &cursor)
{
    QTextCursor tc = cursor;
    tc.setPosition(cursor.selectionStart());
    QRectF r = cursorRect(tc);
    tc.setPosition(cursor.selectionEnd());
    r |= cursorRect(tc);
    return r.toRect();
}
