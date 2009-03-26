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

#ifndef GENERICCOMPOSER_H
#define GENERICCOMPOSER_H

#include <QLineEdit>

#include <qmailcomposer.h>
#include <qmailcomposerplugin.h>

#ifdef QTOPIA_HOMEUI
#include <private/homewidgets_p.h>
#endif

class AddAttDialog;
class ComposerTextEdit;
class DetailsPage;
class QLabel;
class QAction;
class QStackedWidget;

class GenericComposerInterface : public QMailComposerInterface
{
    Q_OBJECT

public:
    GenericComposerInterface( QWidget *parent = 0 );
    virtual ~GenericComposerInterface();

    bool isEmpty() const;
    QMailMessage message() const;

    void setDefaultAccount(const QMailAccountId& id);
    void setTo(const QString& toAddress);
    void setFrom(const QString& fromAddress);
    void setSubject(const QString& subject);
    void setMessageType(QMailMessage::MessageType type);
    QString from() const;
    QString to() const;
    bool isReadyToSend() const;
    bool isDetailsOnlyMode() const;
    void setDetailsOnlyMode(bool val);
    QString contextTitle() const;
    QMailAccount fromAccount() const;

public slots:
    void setMessage( const QMailMessage &mail );
    void setBody( const QString &txt, const QString &type );
    void clear();
    void attach( const QContent &lnk, QMailMessage::AttachmentsAction action = QMailMessage::LinkToAttachments );
    void reply(const QMailMessage& source, int action);

protected slots:
    void updateSmsLimitIndicator(int length = 0);
    void textChanged();
    void templateText();
#ifndef QTOPIA_NO_SMS
    void smsLengthInfo(uint& estimatedBytes, bool& isUnicode);
    int smsCountInfo();
#endif
    void detailsPage();
    void composePage();
#ifdef QTOPIA_HOMEUI
    void selectRecipients();
    void recipientsActivated();
#endif

private:
    void init();
    QString text() const;
    bool isVCard() const { return m_vCard; }
    void setContext(const QString& title);

private:
    QStackedWidget* m_widgetStack;
    QWidget* m_composerWidget;
    ComposerTextEdit *m_textEdit;
    QLabel *m_smsLimitIndicator;
    DetailsPage* m_detailsWidget;
    QAction *m_showLimitAction;
    QAction *m_templateTextAction;
#ifdef QTOPIA_HOMEUI
    HomeFieldButton *m_toEdit;
    HomeActionButton *m_contactsButton;
    ColumnSizer m_sizer;
#endif
    bool m_vCard;
    QString m_vCardData;
    QString m_title;
    QMailMessage::MessageType m_type;
};

class GenericComposerPlugin : public QMailComposerPlugin
{
    Q_OBJECT

public:
    GenericComposerPlugin();

    QMailComposerInterface* create( QWidget* parent );
};

#endif
