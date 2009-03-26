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



#ifndef READMAIL_H
#define READMAIL_H

#include <QMap>
#include <QStack>

#include "viewatt.h"
#include <QMailMessage>
#include <QMailViewerFactory>
#include <QMainWindow>

class QAction;
class QContactModel;
class QLabel;
class QMailViewerInterface;
class QMenu;
class QStackedWidget;
class QUniqueId;
class QUrl;

class ReadMail : public QMainWindow
{
    Q_OBJECT

public:
    enum ResendAction { Reply = 1, ReplyToAll = 2, Forward = 3 };

    ReadMail( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~ReadMail();

    void displayMessage(const QMailMessageId& message, QMailViewerFactory::PresentationType, bool nextAvailable, bool previousAvailable);
    QMailMessageId displayedMessage() const;

    bool handleIncomingMessages(const QMailMessageIdList &list) const;
    bool handleOutgoingMessages(const QMailMessageIdList &list) const;

    void updateWindowTitle() const;

private slots:
    void updateView(QMailViewerFactory::PresentationType);
    void messagesUpdated(const QMailMessageIdList& list);
    void displayContact(const QContact& contact);

signals:
    void resendRequested(const QMailMessage&, int);
    void sendMessageTo(const QMailAddress&, QMailMessage::MessageType);
    void modifyRequested(const QMailMessage&);
    void removeMessage(const QMailMessageId& id, bool userRequest);
    void viewingMail(const QMailMessageMetaData&);
    void getMailRequested(const QMailMessageMetaData&);
    void sendMailRequested(const QMailMessageMetaData&);
    void readReplyRequested(const QMailMessageMetaData&);
    void cancelView();
    void viewNext();
    void viewPrevious();
    void viewMessage(const QMailMessageId &id, QMailViewerFactory::PresentationType);
    void sendMessage(const QMailMessage &message);

public slots:
    void closeView();
    void setSendingInProgress(bool);
    void setRetrievalInProgress(bool);

protected slots:
    void linkClicked(const QUrl &lnk);
    void messageChanged(const QMailMessageId &id);
    void viewFinished();

    void deleteItem();
    void viewAttachments();

    void reply();
    void replyAll();
    void forward();
    void modify();

    void setStatus(int);
    void getThisMail();
    void sendThisMail();

    void storeContact();

protected:
    void keyPressEvent(QKeyEvent *);

private:
    void viewMms();

    void init();
    void showMessage(const QMailMessageId &id, QMailViewerFactory::PresentationType);
    void loadMessage(const QMailMessageId &id);
    void updateButtons();
    void buildMenu(const QString &mailbox);
    void initImages(QMailViewerInterface* view);

    void dialNumber(const QString&);
    void storeContact(const QMailAddress &address, QMailMessage::MessageType);
    void displayContact(const QUniqueId &uid);

    void switchView(QMailViewerInterface* viewer, const QString& title);

    QMailViewerInterface* currentViewer() const;

    QMailViewerInterface* viewer(QMailMessage::ContentType content, QMailViewerFactory::PresentationType type = QMailViewerFactory::StandardPresentation);

    static QString displayName(const QMailMessage &);

    void updateReadStatus();

private slots:
    void contactModelReset();

private:
    QStackedWidget *views;
    bool sending, receiving;
    QMailMessage mail;
    ViewAtt *viewAtt;
    bool isMms;
    bool isSmil;
    bool firstRead;
    bool hasNext, hasPrevious;

    QMenu *context;

    QAction *deleteButton;
    bool initialized;
    QAction *nextButton;
    QAction *attachmentsButton;
    QAction *previousButton;
    QAction *replyButton;
    QAction *replyAllAction;
    QAction *forwardAction;
    QAction *getThisMailButton;
    QAction *sendThisMailButton;
    QAction *modifyButton;
    QAction *storeButton;
    QContactModel *contactModel;
    bool modelUpdatePending;
    QString lastTitle;

    typedef QStack<QPair<QMailViewerInterface*, QString> > ViewerStack;
    ViewerStack currentView;

    typedef QMap<QPair<QMailMessage::ContentType, QMailViewerFactory::PresentationType>, QMailViewerInterface*> ViewerMap;
    ViewerMap contentViews;
};

#endif
