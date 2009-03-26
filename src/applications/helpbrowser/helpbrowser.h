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

#ifndef HELPBROWSER_H
#define HELPBROWSER_H

#include <qtopia/qsoftmenubar.h>
#include <qtopiaabstractservice.h>

#include <QMainWindow>
#include <QTextBrowser>
#include <QStack>
#include <QUrl>
#include "navigationbar_p.h"

class QAction;
class QLabel;

class MagicTextBrowser : public QTextBrowser {
    Q_OBJECT
public:
    MagicTextBrowser( QWidget* parent );

    virtual QVariant loadResource (int type, const QUrl &name);

private:
    // Generate help page links
    QString generate( const QString& );
};

class HelpBrowser : public QMainWindow
{
    Q_OBJECT
public:
    HelpBrowser( QWidget* parent=0, Qt::WFlags f=0 );

    virtual ~HelpBrowser();

    bool eventFilter( QObject*, QEvent* );

public slots:
    void setDocument( const QString &doc );
    void updateLabels();

private slots:
    void goHome();

    void textChanged(QUrl);

protected:
    void closeEvent( QCloseEvent* );

private:
    void init();

    MagicTextBrowser *browser;
    QAction *backAction, *forwardAction;
#ifdef DEBUG
    QLabel *location;
#endif
    NavigationBar *navigationBar;

    QMenu *contextMenu;
};

class HelpService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class HelpBrowser;
private:
    HelpService( HelpBrowser *parent )
        : QtopiaAbstractService( "Help", parent )
        { this->parent = parent; publishAll(); }

public:
    ~HelpService();

public slots:
    void setDocument( const QString& doc );

private:
    HelpBrowser *parent;
};

#endif
