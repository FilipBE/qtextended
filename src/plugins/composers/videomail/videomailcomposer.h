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

#ifndef VIDEOMAILCOMPOSER_H
#define VIDEOMAILCOMPOSER_H

#include <qmailcomposer.h>
#include <qmailcomposerplugin.h>

class QLineEdit;
class QTextEdit;
class QToolButton;
class VideoRecorderWidget;
class ColumnSizer;
class HomeActionButton;
class HomeFieldButton;

class VideomailComposerInterface : public QMailComposerInterface
{
    Q_OBJECT

public:
    VideomailComposerInterface(QWidget* parent);
    virtual ~VideomailComposerInterface();

    bool isEmpty() const;
    QMailMessage message() const;
    void setDefaultAccount(const QMailAccountId& id);
    void setTo(const QString& toAddress);
    void setFrom(const QString& fromAddress);
    QString from() const;
    QString to() const;
    QString subject() const;
    bool isReadyToSend() const;
    bool isDetailsOnlyMode() const;
    void setDetailsOnlyMode(bool val);
    QString contextTitle() const;
    QMailAccount fromAccount() const;

public slots:
    void setMessage(const QMailMessage& mail);
    void clear();
    void setBody(const QString &text, const QString &type);
    void attach(const QContent &lnk,
                QMailMessage::AttachmentsAction action = QMailMessage::LinkToAttachments);
    void setSignature(const QString &sig);
    void reply(const QMailMessage& source, int type);
    void setSubject(const QString& val);

private slots:
    void selectRecipients();
    void toClicked();

private:
    void init();
    void setContext(const QString& context);

private:
    QToolButton* m_toButton;
    ColumnSizer* m_sizer;
    HomeActionButton* m_contactsButton;
    HomeFieldButton* m_toEdit;
    VideoRecorderWidget* m_videoRecorderWidget;
    QString m_title;
    QString m_fromAddress;
};

class VideomailComposerPlugin : public QMailComposerPlugin
{
    Q_OBJECT

public:
    VideomailComposerPlugin();

    QMailComposerInterface* create( QWidget* parent );
};

#endif

