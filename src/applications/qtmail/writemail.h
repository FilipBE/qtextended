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

#ifndef WRITEMAIL_H
#define WRITEMAIL_H

#include <QDialog>
#include <QMainWindow>
#include <QString>
#include <QMailMessage>
#include <private/detailspage_p.h>

class QAction;
class QContent;
class QMailComposerInterface;
class QStackedWidget;
class SelectComposerWidget;

#ifdef QTOPIA_HOMEUI
class WriteMail : public QDialog
#else
class WriteMail : public QMainWindow
#endif //QTOPIA_HOMEUI
{
    Q_OBJECT

public:
    WriteMail(QWidget* parent = 0);
    virtual ~WriteMail();

    void reply(const QMailMessage& replyMail, int action);
    void modify(const QMailMessage& previousMessage);
    void setRecipients(const QString &emails, const QString &numbers);
    void setRecipient(const QString &recipient);
    void setSubject(const QString &subject);
    void setBody(const QString &text, const QString &type);
    bool hasContent();
#ifndef QTOPIA_NO_SMS
    void setSmsRecipient(const QString &recipient);
#endif
    void setDefaultAccount(const QMailAccountId& defaultId);
    QString composer() const;
    bool forcedClosure();

public slots:
    bool saveChangesOnRequest();
    bool newMail( QMailMessage::MessageType = QMailMessage::AnyType, bool detailsOnly = false );
    void attach( const QContent &dl, QMailMessage::AttachmentsAction );

signals:
    void editAccounts();
    void noSendAccount(const QMailMessage::MessageType);
    void saveAsDraft(const QMailMessage&);
    void enqueueMail(const QMailMessage&);
    void discardMail();
    void finished();

protected slots:
    bool sendStage();
    void messageModified();
    void reset();
    void discard();
    bool draft();
    bool composerSelected(const QPair<QString, QMailMessage::MessageType> &selection);
    void contextChanged();

#ifdef QTOPIA_HOMEUI
    void accept();
    void reject();
    void changeEvent(QEvent* e);
#endif

private:
    bool largeAttachments();
    uint largeAttachmentsLimit() const;
    bool buildMail();
    void init();
    QString signature() const;
    bool isComplete() const;
    bool changed() const;
    void setComposer( const QString &id );
    void setTitle(const QString& title);
    void emitFinished();

private:
    QMailMessage mail;
    QMailComposerInterface *m_composerInterface;
    QAction *m_cancelAction, *m_draftAction;
    QStackedWidget* m_widgetStack;
    QWidget *m_mainWindow;
    QMailAccountId defaultAccountId;
    bool hasMessageChanged;
    bool _detailsOnly;
    SelectComposerWidget* m_selectComposerWidget;

};

#endif
