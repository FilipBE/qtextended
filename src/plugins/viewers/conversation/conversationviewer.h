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

#ifndef CONVERSATIONVIEWER_H
#define CONVERSATIONVIEWER_H

#include <QContact>
#include <QObject>
#include <QString>

#include <qmailviewer.h>
#include <qmailviewerplugin.h>

#ifdef QTOPIA_HOMEUI
#include <private/homewidgets_p.h>
#endif

class QAction;
class QMailMessage;
class QMailMessageListModel;
class QModelIndex;
class QSmoothList;
class QToolButton;
class QUrl;

// A conversation viewer showing a list of messages between conversing parties
class ConversationViewer : public QMailViewerInterface
{
    Q_OBJECT

public:
    ConversationViewer(QWidget* parent = 0);
    virtual ~ConversationViewer();

    virtual void scrollToAnchor(const QString& a);

    virtual QWidget *widget() const;

    virtual void addActions(QMenu* menu) const;

    virtual bool handleIncomingMessages(const QMailMessageIdList &list) const;
    virtual bool handleOutgoingMessages(const QMailMessageIdList &list) const;

signals:
    void finished();

public slots:
    virtual bool setMessage(const QMailMessage& mail);
    virtual void setResource(const QUrl& name, QVariant var);
    virtual void clear();

    virtual void action(QAction* action);

    virtual void linkClicked(const QUrl& link);

protected slots:
#ifdef QTOPIA_HOMEUI
    virtual void senderActivated();
#endif

    void rowsInserted(const QModelIndex &, int start, int end);
    void setCurrentIndex();
    void currentChanged(const QModelIndex &, const QModelIndex &);
    void messageActivated(const QModelIndex &);
    void sendReply();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event);
    virtual void changeEvent(QEvent *e);
    void updateBackground();

private:
    QSmoothList* listView;
#ifdef QTOPIA_HOMEUI
    QWidget *mainWidget;
    ColumnSizer sizer;
    HomeContactButton *fromButton;
    HomeActionButton *replyButton;
    HomeActionButton *deleteButton;
    HomeActionButton *backButton;
#endif
    QMailMessageListModel* model;
    QMailMessageId messageId;
    QContact contact;
    int currentIndex;
};

class ConversationViewerPlugin : public QMailViewerPlugin
{
    Q_OBJECT

public:
    ConversationViewerPlugin();

    virtual QString key() const;
    virtual bool isSupported(QMailMessage::ContentType type, QMailViewerFactory::PresentationType pres) const;

    QMailViewerInterface *create(QWidget *parent);
};

#endif
