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

#ifndef ADDATTDIALOGPHONE_H
#define ADDATTDIALOGPHONE_H

#include <QAction>
#include <QDialog>

#include "addatt.h"

class QDocumentSelector;

// phone version, wraps AddAtt
class AddAttDialog : public QDialog, public AddAttBase
{
    Q_OBJECT
public:
    AddAttDialog(QWidget *parent = 0, QString name = QString(), Qt::WFlags f = 0);
    QList< AttachmentItem* > attachedFiles() const;
    void getFiles() {}
    void clear();
    void setMailMessageParts(QMailMessage *mail);
    QDocumentSelector* documentSelector() const;

public slots:
    void removeAttachment(AttachmentItem*);
    void removeCurrentAttachment();
    void attach( const QContent &doclnk, QMailMessage::AttachmentsAction );
    void selectAttachment();
    virtual void done(int r);

protected slots:
    void openFile(const QContent&);
    void updateDisplay(bool);

signals:
    void currentChanged(bool);
    void attachmentsChanged();

private:
    AddAtt *addAtt;
    QDocumentSelector *fileSelector;
    QDialog *fileSelectorDialog;
    QAction *removeAction;
};

#endif
