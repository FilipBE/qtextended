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

#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include "ui_helpdialog.h"
#include "index.h"
#include "helpwindow.h"
#include "docuparser.h"

#include <QList>
#include <QPair>
#include <QListWidget>
#include <QTreeWidget>
#include <QMap>
#include <QStringList>
#include <QValidator>
#include <qmenu.h>
#include <QHash>

QT_BEGIN_NAMESPACE

class QProgressBar;
class MainWindow;
class QTextBrowser;
class IndexListModel;

class HelpNavigationListItem : public QListWidgetItem
{
public:
    HelpNavigationListItem(QListWidget *ls, const QString &txt);

    void addLink(const QString &link);
    QStringList links() const { return linkList; }
private:
    QStringList linkList;

};

class SearchValidator : public QValidator
{
    Q_OBJECT
public:
    SearchValidator(QObject *parent)
        : QValidator(parent) {}
    ~SearchValidator() {}
    QValidator::State validate(QString &str, int &) const;
};

class HelpDialog : public QWidget
{
    Q_OBJECT
public:
    HelpDialog(QWidget *parent, MainWindow *h);

    inline QTabWidget *tabWidget() const
    { return ui.tabWidget; }

    QString titleOfLink(const QString &link);
    bool eventFilter(QObject *, QEvent *);
    bool lastWindowClosed() { return lwClosed; }

    void timerEvent(QTimerEvent *e);
    static QString removeAnchorFromLink(const QString &link);

signals:
    void showLink(const QString &s);
    void showSearchLink(const QString &s, const QStringList &terms);

public slots:
    void initialize();
    void startSearch();
    void addBookmark();
    void currentTabChanged(int index);
    void locateContents(const QString &link);

private slots:
    void on_buttonAdd_clicked();
    void on_buttonRemove_clicked();
    void on_termsEdit_returnPressed();
    void on_helpButton_clicked();
    void on_searchButton_clicked();
    void on_resultBox_itemActivated(QListWidgetItem*);
    void updateSearchButton(const QString &txt);

    void showResultPage(QListWidgetItem *);

    void showTopic(QTreeWidgetItem *);
    void loadIndexFile();
    void insertContents();
    void setupFullTextIndex();
    void showTopic();
    void searchInIndex(const QString &s);
    void toggleContents();
    void toggleIndex();
    void toggleBookmarks();
    void toggleSearch();
    void lastWinClosed();
    void setIndexingProgress(int prog);
    void showListItemMenu(const QPoint &pos);
    void showIndexItemMenu(const QPoint &pos);
    void showTreeItemMenu(const QPoint &pos);
    void insertBookmarks();
    void processEvents();

private:
    typedef QList<ContentItem> ContentList;
    void removeOldCacheFiles(bool onlyFulltextSearchIndex = false);
    void buildKeywordDB();
    quint32 getFileAges();
    void showIndexTopic();
    void showBookmarkTopic();
    void setupTitleMap();
    void saveBookmarks();
    void showContentsTopic();
    void showInitDoneMessage();
    void buildContentDict();
    QTreeWidgetItem * locateLink(QTreeWidgetItem *item, const QString &link);

private:
    Ui::HelpDialog ui;

    IndexListModel *indexModel;
    QMap<QString, QString> titleMap;
    bool indexDone, bookmarksInserted, titleMapDone, contentsInserted;
    bool lwClosed;
    MainWindow *help;
    QString documentationPath;
    Index *fullTextIndex;
    QStringList terms, foundDocs;
    bool initDoneMsgShown;
    void getAllContents();
    QList<QPair<QString, ContentList> > contentList;
    QMenu *itemPopup;
    QString cacheFilesPath;
    QStringList keywordDocuments;

    QAction *actionOpenCurrentTab;
    QAction *actionOpenLinkInNewWindow;
    QAction *actionOpenLinkInNewTab;
};

QT_END_NAMESPACE

#endif // HELPDIALOG_H
