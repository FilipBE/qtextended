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

#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QtCore/QUrl>
#include <QtCore/QPoint>
#include <QtCore/QObject>

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QEvent;
class QLabel;
class QAction;
class QCheckBox;
class QLineEdit;
class QToolButton;

class HelpViewer;
class QTabWidget;
class QHelpEngine;
class CentralWidget;
class PrintHelper;
class MainWindow;

class SearchWidget;
class QHelpSearchEngine;

class FindWidget : public QWidget
{
    Q_OBJECT

public:
    FindWidget(QWidget *parent = 0);
    ~FindWidget();

signals:
    void findNext();
    void findPrevious();

private slots:
    void updateButtons();

private:
    QLineEdit *editFind;
    QCheckBox *checkCase;
    QLabel *labelWrapped;
    QToolButton *toolNext;
    QToolButton *toolClose;
    QToolButton *toolPrevious;
    QCheckBox *checkWholeWords;

    friend class CentralWidget;
};

class CentralWidget : public QWidget
{
    Q_OBJECT

public:
    CentralWidget(QHelpEngine *engine, MainWindow *parent);
    ~CentralWidget();

    void setLastShownPages();
    bool hasSelection() const;
    QUrl currentSource() const;
    QString currentTitle() const;
    bool isHomeAvailable() const;
    bool isForwardAvailable() const;
    bool isBackwardAvailable() const;
    QList<QAction*> globalActions() const;
    void setGlobalActions(const QList<QAction*> &actions);
    HelpViewer *currentHelpViewer() const;
    void activateTab(bool onlyHelpViewer = false);
    void activateSearch();
    void createSearchWidget(QHelpSearchEngine *searchEngine);
    void removeSearchWidget();

    static CentralWidget *instance();

public slots:
    void zoomIn();
    void zoomOut();
    void findNext();
    void nextPage();
    void resetZoom();
    void previousPage();
    void findPrevious();
    void copySelection();
    void showTextSearch();
    void print();
    void pageSetup();
    void printPreview();
    void updateBrowserFont();
    void setSource(const QUrl &url);
    void setSourceInNewTab(const QUrl &url, qreal zoom = 0.0);
    void findCurrentText(const QString &text);
    HelpViewer *newEmptyTab();
    void home();
    void forward();
    void backward();

signals:
    void currentViewerChanged();
    void copyAvailable(bool yes);
    void sourceChanged(const QUrl &url);
    void highlighted(const QString &link);
    void forwardAvailable(bool available);
    void backwardAvailable(bool available);
    void addNewBookmark(const QString &title, const QString &url);

protected:
    void keyPressEvent(QKeyEvent *);

private slots:
    void newTab();
    void closeTab();
    void setTabTitle(const QUrl& url);
    void currentPageChanged(int index);
    void showTabBarContextMenu(const QPoint &point);
    void printPreview(QPrinter *printer);

private:
    void connectSignals();    
    bool eventFilter(QObject *object, QEvent *e);
    void find(QString ttf, bool forward, bool backward);
    void initPrinter();
    QString quoteTabTitle(const QString &title) const;

private:
    int lastTabPage;
    QString collectionFile;
    QList<QAction*> globalActionList;

    QWidget *findBar;
    QTabWidget* tabWidget;
    FindWidget *findWidget;
    QHelpEngine *helpEngine;
    QPrinter *printer;
    bool usesDefaultCollection;
    
    SearchWidget* m_searchWidget;
};

QT_END_NAMESPACE

#endif  // CENTRALWIDGET_H
