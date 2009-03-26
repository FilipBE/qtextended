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

#include "messagenavigator.h"
#include "foldermodel.h"
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QListView>
#include <QMailMessageDelegate>
#include <QMailMessageListModel>
#include <QSoftMenuBar>
#include <QtopiaItemDelegate>
#include <QtopiaServiceRequest>
#include <QTreeView>
#include <QVBoxLayout>


// A stackable widget allowing the selection of a message folder
class FolderSelector : public QWidget
{
    Q_OBJECT

public:
    FolderSelector(QWidget* parent = 0)
        : QWidget(parent), 
          label(new QLabel), 
          treeView(new QTreeView)
    {
        label->setText(tr("Select a folder:"));

        treeView->setModel(&model);
        treeView->setItemDelegate(new QtopiaItemDelegate(this));
        treeView->setSelectionMode(QAbstractItemView::SingleSelection);
        treeView->header()->setVisible(false);
        treeView->installEventFilter(this);

        connect(treeView, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(label);
        layout->addWidget(treeView);
        // Adjust FolderSelector layout for attractive layout - not relevant to example discussion
        int horizontal = style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
        int vertical = style()->pixelMetric(QStyle::PM_LayoutTopMargin);
        label->setContentsMargins(horizontal, vertical, horizontal, 0);
        layout->setContentsMargins(0, 0, 0, 0);
        treeView->setFrameStyle(QFrame::NoFrame);
        // end-Adjust
    }

signals:
    void folderSelected(const QMailMessageSet* folder);
    void done();

protected slots:
    void activated(const QModelIndex& index);

protected:
    virtual bool eventFilter(QObject* obj, QEvent* event);

private:
    FolderModel model;
    QLabel* label;
    QTreeView* treeView;
};

void FolderSelector::activated(const QModelIndex& index) 
{ 
    emit folderSelected(model.itemFromIndex(index)); 
}

bool FolderSelector::eventFilter(QObject* obj, QEvent* event)
{
    // We need to capture the back key, so it doesn't close our window
    if ((obj == treeView) && (event->type() == QEvent::KeyPress)) {
        if (QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event)) {
            if (keyEvent->key() == Qt::Key_Back) {
                emit done();
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}


// A stackable widget allowing the selection of a message
class MessageSelector : public QWidget
{
    Q_OBJECT

public:
    MessageSelector(QWidget* parent = 0)
        : QWidget(parent), 
          label(new QLabel), 
          listView(new QListView),
          delegate(new QMailMessageDelegate(QMailMessageDelegate::AddressbookMode, this))
    {
        label->setWordWrap(true);

        listView->setModel(&model);
        listView->setItemDelegate(delegate);
        listView->setSelectionMode(QAbstractItemView::SingleSelection);
        listView->installEventFilter(this);

        connect(listView, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(label);
        layout->addWidget(listView);
        // Adjust MessageSelector layout for attractive layout - not relevant to example discussion
        int horizontal = style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
        int vertical = style()->pixelMetric(QStyle::PM_LayoutTopMargin);
        label->setContentsMargins(horizontal, vertical, horizontal, 0);
        layout->setContentsMargins(0, 0, 0, 0);
        listView->setResizeMode(QListView::Adjust);
        listView->setLayoutMode(QListView::Batched);
        listView->setFrameStyle(QFrame::NoFrame);
        // end-Adjust
    }

signals:
    void listPrepared();
    void messageSelected(const QMailMessageId& id);
    void done();

public slots:
    void listMessages(const QMailMessageSet* folder);

protected slots:
    void activated(const QModelIndex& index);

protected:
    virtual bool eventFilter(QObject* obj, QEvent* event);

private:
    QMailMessageListModel model;
    QLabel* label;
    QListView* listView;
    QMailMessageDelegate* delegate;
};

void MessageSelector::activated(const QModelIndex& index) 
{ 
    emit messageSelected(model.idFromIndex(index)); 
}

void MessageSelector::listMessages(const QMailMessageSet* folder) 
{
    model.setKey(folder->messageKey()); 

    if (model.isEmpty()) {
        label->setText(tr("No messages in %1").arg(folder->displayName()));
    } else {
        listView->selectionModel()->select(model.index(0, 0), QItemSelectionModel::Select);
        listView->scrollToTop();
        label->setText(tr("Select a message to view the content:"));
    }

    emit listPrepared();
}
// end-listMessages

bool MessageSelector::eventFilter(QObject* obj, QEvent* event)
{
    // We need to capture the back key, so it doesn't close our window
    if ((obj == listView) && (event->type() == QEvent::KeyPress)) {
        if (QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event)) {
            if (keyEvent->key() == Qt::Key_Back) {
                emit done();
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}


MessageNavigator::MessageNavigator(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f),
      folderSelector(new FolderSelector),
      messageSelector(new MessageSelector)
{
    setupUi(this);

    // Connect our components
    connect(folderSelector, SIGNAL(folderSelected(const QMailMessageSet*)), messageSelector, SLOT(listMessages(const QMailMessageSet*)));
    connect(folderSelector, SIGNAL(done()), qApp, SLOT(quit()));

    connect(messageSelector, SIGNAL(listPrepared()), this, SLOT(showMessageList()));
    connect(messageSelector, SIGNAL(messageSelected(QMailMessageId)), this, SLOT(viewMessage(QMailMessageId)));
    connect(messageSelector, SIGNAL(done()), this, SLOT(showFolderTree()));

    widgetStack->addWidget(folderSelector);
    widgetStack->addWidget(messageSelector);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(widgetStack);

    // Adjust MessageNavigator
    layout->setContentsMargins(0, 0, 0, 0);

    // Necessary to create the menu (which contains our help entry):
    (void)QSoftMenuBar::menuFor(this);
    // end-Adjust
    showFolderTree();
}

MessageNavigator::~MessageNavigator()
{
}

void MessageNavigator::showMessageList()
{
    widgetStack->setCurrentWidget(messageSelector);
}

void MessageNavigator::showFolderTree()
{
    widgetStack->setCurrentWidget(folderSelector);
}

void MessageNavigator::viewMessage(const QMailMessageId& id)
{
    // Request that some application display the selected message
    QtopiaServiceRequest req( "Messages", "viewMessage(QMailMessageId)" );
    req << id;
    req.send();
}

#include "messagenavigator.moc"
