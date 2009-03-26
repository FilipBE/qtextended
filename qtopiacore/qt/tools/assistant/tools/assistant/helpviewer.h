/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef HELPVIEWER_H
#define HELPVIEWER_H

#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include <QtGui/QTextBrowser>
#include <QtGui/QAction>

QT_BEGIN_NAMESPACE

class QHelpEngine;
class CentralWidget;

class QPoint;
class QString;
class QKeyEvent;
class QMouseEvent;
class QContextMenuEvent;

#if defined(USE_WEBKIT)

#include <QWebView>

class HelpViewer : public QWebView
{
    Q_OBJECT

public:
    HelpViewer(QHelpEngine *helpEngine, CentralWidget *parent);
    void setSource(const QUrl &url);

    inline QUrl source() const
    { return url(); }

    inline QString documentTitle() const
    { return title(); }

    inline bool hasSelection() const
    { return !selectedText().isEmpty(); } // ### this is suboptimal

    void resetZoom();
    void zoomIn(int range = 1);
    void zoomOut(int range = 1);

    inline void copy()
    { return triggerPageAction(QWebPage::Copy); }

    inline bool isForwardAvailable() const
    { return pageAction(QWebPage::Forward)->isEnabled(); }
    inline bool isBackwardAvailable() const
    { return pageAction(QWebPage::Back)->isEnabled(); }

public Q_SLOTS:
    void home();
    void backward() { back(); }

Q_SIGNALS:
    void copyAvailable(bool enabled);
    void forwardAvailable(bool enabled);
    void backwardAvailable(bool enabled);
    void highlighted(const QString &);
    void sourceChanged(const QUrl &);

protected:
    virtual void wheelEvent(QWheelEvent *);
    void mouseReleaseEvent(QMouseEvent *e);

private Q_SLOTS:
    void actionChanged();

private:
    QHelpEngine *helpEngine;
    CentralWidget* parentWidget;
    QUrl homeUrl;
};

#else

class HelpViewer : public QTextBrowser
{
    Q_OBJECT

public:
    HelpViewer(QHelpEngine *helpEngine, CentralWidget *parent);
    void setSource(const QUrl &url);

    void resetZoom();
    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    int zoom() const { return zoomCount; }
    void setZoom(int zoom) { zoomCount = zoom; }

    inline bool hasSelection() const
    { return textCursor().hasSelection(); }

private:
    QVariant loadResource(int type, const QUrl &name);    
    void openLinkInNewTab(const QString &link);
    bool hasAnchorAt(const QPoint& pos);
    void contextMenuEvent(QContextMenuEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private slots:
    void openLinkInNewTab();

private:
    int zoomCount;
    bool controlPressed;
    QString lastAnchor;
    QHelpEngine *helpEngine;
    CentralWidget* parentWidget;
};

#endif

QT_END_NAMESPACE

#endif
