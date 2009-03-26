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

#ifndef DESKPHONE_EMAILCOMPOSER_H
#define DESKPHONE_EMAILCOMPOSER_H

#include <QContent>
#include <QList>
#include <QWidget>
#include <qmailcomposer.h>
#include <qmailcomposerplugin.h>

class AddAttDialog;
class QLabel;
class QWidget;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QToolButton;
class HomeActionButton;
class HomeFieldButton;
class ColumnSizer;

class EmailComposerInterface : public QMailComposerInterface
{
    Q_OBJECT

public:
    EmailComposerInterface( QWidget *parent = 0 );
    virtual ~EmailComposerInterface();

    bool isEmpty() const;
    QMailMessage message() const;

public:
    void setDefaultAccount(const QMailAccountId& id);
    void setTo(const QString& toAddress);
    void setFrom(const QString& fromAddress);
    void setSubject(const QString& subject);
    QString from() const;
    QString to() const;
    bool isReadyToSend() const;
    bool isDetailsOnlyMode() const;
    void setDetailsOnlyMode(bool val);
    QString contextTitle() const;
    QMailAccount fromAccount() const;

signals:
    void attachmentsChanged();

public slots:
    void setMessage( const QMailMessage &mail );
    void setBody( const QString &txt, const QString &type );
    void clear();
    void attach( const QContent &lnk, QMailMessage::AttachmentsAction = QMailMessage::LinkToAttachments );
    void setSignature( const QString &sig );
    void reply(const QMailMessage& source, int action);

protected slots:
    void selectAttachments();
    void selectRecipients();
    void updateAttachmentsLabel();
    void setCursorPosition();
    void subjectClicked();
    void toClicked();

protected:
    void keyPressEvent( QKeyEvent *e );

private:
    void init();
    void setPlainText( const QString& text, const QString& signature );
    void setContext(const QString& context);

private:
    AddAttDialog *m_addAttDialog;
    int m_index;
    QTextEdit* m_bodyEdit;
    HomeActionButton* m_contactsButton;
    HomeFieldButton* m_toEdit;
    HomeFieldButton* m_subjectEdit;
    ColumnSizer* m_sizer;
    QToolButton* m_attachmentsButton;
    QWidget *m_widget;

    typedef QPair<QContent, QMailMessage::AttachmentsAction> AttachmentDetail;
    QList<AttachmentDetail> m_attachments;

    QString m_signature;
    QString m_title;
    QString m_from;
};

class EmailComposerPlugin : public QMailComposerPlugin
{
    Q_OBJECT

public:
    EmailComposerPlugin();
    QMailComposerInterface* create( QWidget* parent );
};

#endif
