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

#ifndef WEBVIEWER_H
#define WEBVIEWER_H

#include <QMainWindow>
#include <QContent>
#include <QFile>

class QUrl;
class QWebPage;
class QtopiaWebView;
class QNetworkReply;
class NavigationBar;
class SoftNavigationBar;
class ProgressWidget;

class WebViewer : public QMainWindow
{
    Q_OBJECT
public:
    WebViewer(QWidget *parent = 0, Qt::WFlags=0);
    ~WebViewer();

    void keyPressEvent(QKeyEvent *ke);

private slots:
    void addBindings();
    void loadFinished();
    void load(const QUrl&, bool);
    void setDocument(const QString&);
    void handleUnsupportedContent(QNetworkReply*);
    void downloadMore();
    void finishedDownload();
    void downloadProgress(qint64 p, qint64 total);
    void addToSpeedDial();
    void goTo();

private:
    QUrl standardPage(const QString& id) const;
    QtopiaWebView *view;
    QWebPage *unsecurePage, *securePage;
    NavigationBar *navigationBar;
    SoftNavigationBar *softNavigationBar;
    QObject *bindings;
    bool downloading;
    QContent downloadcontent;
    QFile downloadto;
    QAction *actionSpeedDial;
};

#endif
