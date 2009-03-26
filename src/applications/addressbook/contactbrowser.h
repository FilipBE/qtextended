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
#ifndef CONTACTBROWSER_H
#define CONTACTBROWSER_H

#include <QDLBrowserClient>

#include "contactdocument.h"
#include "qcontent.h"

class QContact;
class QContactModel;
class QtopiaServiceRequest;
class ContactTextEdit;
class ContactToneButton;
class ContactPictureButton;

class ContactBrowser : public QDLBrowserClient
{
    Q_OBJECT

public:
    ContactBrowser( QWidget *parent, const char *name = 0 );
    ~ContactBrowser();
    void init(const QContact& contact, ContactDocument::ContactDocumentType docType);

    void setModel(QContactModel *model);
    void processKeyPressEvent(QKeyEvent *e) {keyPressEvent(e);}

    QContact contact() const;

public slots:
    void linkClicked(const QString& link);
    void linkHighlighted(const QString& link);

signals:
    void okPressed();
    void previous();
    void next();
    void externalLinkActivated();
    void closeView();

protected:
    void keyPressEvent( QKeyEvent *e );
    void mouseMoveEvent(QMouseEvent *) {}

    void setSource(const QUrl & name);

private:
    QRect fakeSelectionRect(QTextDocument* doc, const QTextCursor &cursor);
    QString mLink;
    ContactDocument *mDocument;
};

#endif
