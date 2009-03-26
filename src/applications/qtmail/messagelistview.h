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

#ifndef MESSAGELISTVIEW_H
#define MESSAGELISTVIEW_H

#include <QWidget>
#include <QMailMessage>
#include <QMailMessageKey>
#include <QMailMessageSortKey>

class MessageList;
class QFrame;
class QLineEdit;
#ifndef QTOPIA_HOMEUI
class QMailMessageDelegate;
#endif
class QMailMessageListModel;
class QModelIndex;
class QSortFilterProxyModel;
class QTabBar;
class QToolButton;
#ifdef QTOPIA_HOMEUI
class QtopiaHomeMailMessageDelegate;
#endif

class MessageListView : public QWidget
{
    Q_OBJECT

public:
    enum DisplayMode
    {
        DisplayMessages = 0,
        DisplayReceived = 1,
        DisplaySent = 2,
        DisplayDrafts = 3,
        DisplayTrash = 4,
        DisplayFilter = 5
    };

    MessageListView(QWidget* parent = 0);
    virtual ~MessageListView();

    QMailMessageKey key() const;
    void setKey(const QMailMessageKey& key);

    QMailMessageSortKey sortKey() const;
    void setSortKey(const QMailMessageSortKey& sortKey);

    DisplayMode displayMode() const;
    void setDisplayMode(const DisplayMode& m);

    QMailMessageListModel* model() const;

    QMailMessageId current() const;
    void setCurrent(const QMailMessageId& id);
    void setNextCurrent();
    void setPreviousCurrent();

    bool hasNext() const;
    bool hasPrevious() const;

    int rowCount() const;

    QMailMessageIdList selected() const;
    void setSelected(const QMailMessageIdList& idList);
    void setSelected(const QMailMessageId& id);
    void selectAll();
    void clearSelection();

    bool markingMode() const;
    void setMarkingMode(bool set);

    bool ignoreUpdatesWhenHidden() const;
    void setIgnoreUpdatesWhenHidden(bool ignore);

signals:
    void clicked(const QMailMessageId& id);
    void currentChanged(const QMailMessageId& oldId, const QMailMessageId& newId);
    void selectionChanged();
    void displayModeChanged(MessageListView::DisplayMode);
    void backPressed();
    void resendRequested(const QMailMessage&, int);

protected slots:
    void indexClicked(const QModelIndex& index);
    void currentIndexChanged(const QModelIndex& currentIndex, const QModelIndex& previousIndex);
    void filterTextChanged(const QString& text);
    void closeFilterButtonClicked();
    void tabSelected(int index);
    void modelChanged();
    void rowsAboutToBeRemoved(const QModelIndex&, int, int);
    void layoutChanged();

protected:
    void showEvent(QShowEvent* e);
    void hideEvent(QHideEvent* e);

private:
    void init();

private:
    MessageList* mMessageList;
    QFrame* mFilterFrame;
    QLineEdit* mFilterEdit;
    QToolButton* mCloseFilterButton;
    QTabBar* mTabs;
#ifdef QTOPIA_HOMEUI
    QtopiaHomeMailMessageDelegate* mDelegate;
#else
    QMailMessageDelegate* mDelegate;
#endif
    QMailMessageListModel* mModel;
    QSortFilterProxyModel* mFilterModel;
    DisplayMode mDisplayMode;
    bool mMarkingMode;
    bool mIgnoreWhenHidden;
    bool mSelectedRowsRemoved;
};

#endif
