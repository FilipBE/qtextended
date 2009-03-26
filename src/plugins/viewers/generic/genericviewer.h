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

#ifndef GENERICVIEWER_H
#define GENERICVIEWER_H

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
class QPushButton;
class QToolButton;
class Browser;

// A generic viewer able to show email, SMS or basic MMS
class GenericViewer : public QMailViewerInterface
{
    Q_OBJECT

public:
    GenericViewer(QWidget* parent = 0);
    virtual ~GenericViewer();

    virtual void scrollToAnchor(const QString& a);

    virtual QWidget *widget() const;

    virtual void addActions(QMenu* menu) const;
#ifdef QTOPIA_HOMEUI
    QString prettyName(QMailAddress address);
    QString recipients();
#endif

public slots:
    virtual bool setMessage(const QMailMessage& mail);
    virtual void setResource(const QUrl& name, QVariant var);
    virtual void clear();

    virtual void action(QAction* action);

    virtual void linkClicked(const QUrl& link);

protected slots:
    virtual void linkHighlighted(const QUrl& link);
#ifdef QTOPIA_HOMEUI
    virtual void replyActivated();
    virtual void senderActivated();
    virtual void recipientsActivated();
#endif

private:
    virtual void setPlainTextMode(bool plainTextMode);
    virtual void print() const;

    bool eventFilter(QObject* watched, QEvent* event);

    Browser* browser;
#ifdef QTOPIA_HOMEUI
    QWidget *mainWidget;
    ColumnSizer sizer;
    HomeContactButton *fromButton;
    HomeFieldButton *toButton;
    HomeActionButton *replyButton;
    HomeActionButton *deleteButton;
    HomeActionButton *backButton;
#endif
    QAction* plainTextModeAction;
    QAction* richTextModeAction;
    QAction* printAction;
    QAction* dialAction;
    QAction* messageAction;
    QAction* storeAction;
    QAction* contactAction;
    const QMailMessage* message;
    bool plainTextMode;
    bool containsNumbers;
    QContact contact;
};

class GenericViewerPlugin : public QMailViewerPlugin
{
    Q_OBJECT

public:
    GenericViewerPlugin();

    virtual QString key() const;
    virtual bool isSupported(QMailMessage::ContentType type, QMailViewerFactory::PresentationType pres) const;

    QMailViewerInterface *create(QWidget *parent);
};

#endif
