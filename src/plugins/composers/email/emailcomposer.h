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

#ifndef EMAILCOMPOSER_H
#define EMAILCOMPOSER_H

#include <QContent>
#include <QList>
#include <qmailcomposer.h>
#include <qmailcomposerplugin.h>

class AddAttDialog;
class QLabel;
class BodyTextEdit;
class QStackedWidget;
class DetailsPage;

class EmailComposerInterface : public QMailComposerInterface
{
    Q_OBJECT

public:
    EmailComposerInterface( QWidget *parent = 0 );
    virtual ~EmailComposerInterface();

    bool isEmpty() const;
    QMailMessage message() const;

    void setDefaultAccount(const QMailAccountId& id);
    void setTo(const QString& toAddress);
    void setFrom(const QString& fromAddress);
    void setCc(const QString& ccAddress);
    void setBcc(const QString& bccAddress);
    void setSubject(const QString& subject);
    QString from() const;
    QString to() const;
    QString cc() const;
    QString bcc() const;
    bool isReadyToSend() const;
    bool isDetailsOnlyMode() const;
    void setDetailsOnlyMode(bool val);
    QString contextTitle() const;
    QMailAccount fromAccount() const;

public slots:
    void setMessage( const QMailMessage &mail );
    void setBody( const QString &txt, const QString &type );
    void clear();
    void attach( const QContent &lnk, QMailMessage::AttachmentsAction = QMailMessage::LinkToAttachments );
    void setSignature( const QString &sig );
    void reply(const QMailMessage& source, int action);

protected slots:
    void selectAttachment();
    void updateLabel();
    void setCursorPosition();
    void updateAttachmentsLabel();
    void detailsPage();
    void composePage();

private:
    void init();
    void setPlainText( const QString& text, const QString& signature );
    void setContext(const QString& title);

private:
    AddAttDialog *m_addAttDialog;
    int m_cursorIndex;
    QWidget* m_composerWidget;
    BodyTextEdit* m_bodyEdit;
    QLabel* m_attachmentsLabel;
    QStackedWidget* m_widgetStack;
    DetailsPage* m_detailsWidget;

    typedef QPair<QContent, QMailMessage::AttachmentsAction> AttachmentDetail;
    QList<AttachmentDetail> m_attachments;

    QString m_signature;
    QString m_title;
};

class EmailComposerPlugin : public QMailComposerPlugin
{
    Q_OBJECT

public:
    EmailComposerPlugin();

    QMailComposerInterface* create( QWidget* parent );
};

#endif
