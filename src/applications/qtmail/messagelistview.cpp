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

#include "messagelistview.h"

#include "qtopialog.h"
#include <QMailFolder>
#include <QMailFolderKey>
#include <QMailMessageKey>
#include <QMailMessageSortKey>
#include <QMailMessageListModel>
#include <QMailMessageDelegate>
#include <QSmoothList>
#ifdef QTOPIA_HOMEUI
#  include <private/homewidgets_p.h>
#endif
#include <private/qtopiainputdialog_p.h>
#include <QKeyEvent>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QModelIndex>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTabBar>
#include <QToolButton>
#include <QVBoxLayout>


#ifdef QTOPIA_HOMEUI
static const QMap<MessageListView::DisplayMode,QMailMessageKey>& keyMap()
{
    static QMap<MessageListView::DisplayMode, QMailMessageKey> map;

    // IMAP folders are those with a parent account ID that is not invalid
    static QMailFolderKey imapFolderKey(QMailFolderKey::ParentAccountId, QMailAccountId(), QMailDataComparator::NotEqual);

    static QMailMessageKey inboxFolderKey(QMailMessageKey::ParentFolderId, QMailFolderId(QMailFolder::InboxFolder));
    static QMailMessageKey outboxFolderKey(QMailMessageKey::ParentFolderId, QMailFolderId(QMailFolder::OutboxFolder));
    static QMailMessageKey draftsFolderKey(QMailMessageKey::ParentFolderId, QMailFolderId(QMailFolder::DraftsFolder));
    static QMailMessageKey sentFolderKey(QMailMessageKey::ParentFolderId, QMailFolderId(QMailFolder::SentFolder));
    static QMailMessageKey trashFolderKey(QMailMessageKey::ParentFolderId, QMailFolderId(QMailFolder::TrashFolder));
    static QMailMessageKey imapKey(QMailMessageKey::ParentFolderId, imapFolderKey);
    static QMailMessageKey inboxWithImapKey(inboxFolderKey | imapKey);
    static QMailMessageKey sentWithOutboxKey(outboxFolderKey | sentFolderKey);

    if (map.isEmpty()) {
        map.insert(MessageListView::DisplayMessages, (inboxWithImapKey | sentWithOutboxKey)); // received or sent
        map.insert(MessageListView::DisplayReceived, inboxWithImapKey);
        map.insert(MessageListView::DisplaySent, sentWithOutboxKey);
        map.insert(MessageListView::DisplayDrafts, draftsFolderKey);
        map.insert(MessageListView::DisplayTrash, trashFolderKey);
        map.insert(MessageListView::DisplayFilter, QMailMessageKey()); // everything
    }

    return map;
}
#endif


class MessageList : public QSmoothList
{
    Q_OBJECT

public:
    MessageList(QWidget* parent = 0);
    virtual ~MessageList();

    QPoint clickPos() const { return pos; }

signals:
    void backPressed();

protected:
    void keyPressEvent(QKeyEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);

    QPoint pos;
};

MessageList::MessageList(QWidget* parent)
:
    QSmoothList(parent)
{
}

MessageList::~MessageList()
{
}

void MessageList::keyPressEvent(QKeyEvent* e)
{
    switch( e->key() ) {
        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Select:
        case Qt::Key_Enter:
        {
            if(currentIndex().isValid())
                emit clicked(currentIndex());
        }
        break;
        case Qt::Key_No:
        case Qt::Key_Back:
        case Qt::Key_Backspace:
            emit backPressed();
        break;
        default:  QSmoothList::keyPressEvent( e );
    }
}

void MessageList::mouseReleaseEvent(QMouseEvent* e)
{
    pos = e->pos();

    QSmoothList::mouseReleaseEvent(e);
}


MessageListView::MessageListView(QWidget* parent)
:
    QWidget(parent),
    mMessageList(new MessageList(this)),
    mFilterFrame(new QFrame(this)),
    mFilterEdit(new QLineEdit(this)),
    mCloseFilterButton(0),
    mTabs(new QTabBar(this)),
#ifdef QTOPIA_HOMEUI
    mDelegate(new QtopiaHomeMailMessageDelegate(QtopiaHomeMailMessageDelegate::QtmailUnifiedMode, this)),
#else
    mDelegate(new QMailMessageDelegate(QMailMessageDelegate::QtmailMode, this)),
#endif
    mModel(new QMailMessageListModel(this)),
    mFilterModel(new QSortFilterProxyModel(this)),
    mDisplayMode(DisplayMessages),
    mMarkingMode(false),
    mIgnoreWhenHidden(true),
    mSelectedRowsRemoved(false)
{
    init();
}

MessageListView::~MessageListView()
{
}

QMailMessageKey MessageListView::key() const
{
    return mModel->key();
}

void MessageListView::setKey(const QMailMessageKey& key)
{
    mModel->setKey(key);
}

QMailMessageSortKey MessageListView::sortKey() const
{
    return mModel->sortKey();
}

void MessageListView::setSortKey(const QMailMessageSortKey& sortKey)
{
    mModel->setSortKey(sortKey);
}

QMailMessageListModel* MessageListView::model() const
{
    return mModel;
}

void MessageListView::init()
{
    mFilterModel->setSourceModel(mModel);
    mFilterModel->setFilterRole(QMailMessageListModel::MessageFilterTextRole);
    mFilterModel->setDynamicSortFilter(true);

    mMessageList->setItemDelegate(mDelegate);
    mMessageList->setModel(mFilterModel);
    mMessageList->setEmptyText(tr("No Messages"));

    mTabs->setFocusPolicy(Qt::NoFocus);
#ifdef QTOPIA_HOMEUI
    mTabs->addTab(tr("Messages"));
    mTabs->addTab(tr("Received"));
    mTabs->addTab(tr("Sent"));
    mTabs->addTab(tr("Drafts"));
    mTabs->addTab(tr("Trash"));
#endif

    mCloseFilterButton = new QToolButton(this);
#ifdef QTOPIA_HOMEUI
    QPalette pal = mCloseFilterButton->palette();
    pal.setBrush(QPalette::Button, QtopiaHome::standardColor(QtopiaHome::Green));
    mCloseFilterButton->setPalette(pal);
#endif
    mCloseFilterButton->setText(tr("Done"));
    mCloseFilterButton->setFocusPolicy(Qt::NoFocus);

    connect(mMessageList, SIGNAL(clicked(QModelIndex)),
            this, SLOT(indexClicked(QModelIndex)));
    connect(mMessageList, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentIndexChanged(QModelIndex,QModelIndex)));
    connect(mMessageList, SIGNAL(backPressed()),
            this, SIGNAL(backPressed()));

    connect(mFilterEdit, SIGNAL(textChanged(QString)),
            this, SLOT(filterTextChanged(QString)));

    connect(mCloseFilterButton, SIGNAL(clicked()),
            this, SLOT(closeFilterButtonClicked()));

    connect(mTabs, SIGNAL(currentChanged(int)),
            this, SLOT(tabSelected(int)));

    connect(mModel, SIGNAL(modelChanged()),
            this, SLOT(modelChanged()));

    connect(mFilterModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(mFilterModel, SIGNAL(layoutChanged()),
            this, SLOT(layoutChanged()));

    QHBoxLayout* hLayout = new QHBoxLayout(mFilterFrame);
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->setSpacing(0);
    hLayout->addWidget(new QLabel("Search",this));
    hLayout->addWidget(mFilterEdit);
    hLayout->addWidget(mCloseFilterButton);
    mFilterFrame->setLayout(hLayout);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(0);
    vLayout->addWidget(mTabs);
    vLayout->addWidget(mFilterFrame);
    vLayout->addWidget(mMessageList);

    this->setLayout(vLayout);
    setFocusProxy(mMessageList);

    setDisplayMode(DisplayMessages);
}

QMailMessageId MessageListView::current() const
{
    return mMessageList->currentIndex().data(QMailMessageListModel::MessageIdRole).value<QMailMessageId>();
}

void MessageListView::setCurrent(const QMailMessageId& id)
{
    QModelIndex index = mModel->indexFromId(id);
    if (index.isValid()) {
        QModelIndex filteredIndex = mFilterModel->mapFromSource(index);
        if (filteredIndex.isValid())
            mMessageList->setCurrentIndex(filteredIndex);
    }
}

void MessageListView::setNextCurrent()
{
    QModelIndex index = mMessageList->currentIndex();
    if ((index.row() + 1) < mFilterModel->rowCount()) {
        QModelIndex nextIndex = mFilterModel->index(index.row()+1,0);
        mMessageList->setCurrentIndex(nextIndex);
    }
}

void MessageListView::setPreviousCurrent()
{
    QModelIndex index = mMessageList->currentIndex();
    if (index.row() > 0) {
        QModelIndex previousIndex = mFilterModel->index(index.row()-1,0);
        mMessageList->setCurrentIndex(previousIndex);
    }
}

bool MessageListView::hasNext() const
{
    QModelIndex index = mMessageList->currentIndex();
    return index.row() < (mFilterModel->rowCount() - 1);
}

bool MessageListView::hasPrevious() const
{
    QModelIndex index = mMessageList->currentIndex();
    return index.row() > 0;
}

int MessageListView::rowCount() const
{
    return mFilterModel->rowCount();
}

QMailMessageIdList MessageListView::selected() const
{
    QMailMessageIdList selectedIds;

    if (mMarkingMode) {
        // get the selected model indexes and convert to ids
        for (int i = 0, count = mFilterModel->rowCount(); i < count; ++i) {
            QModelIndex index(mFilterModel->index(i, 0));
            if (static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt()) == Qt::Checked)
                selectedIds.append(index.data(QMailMessageListModel::MessageIdRole).value<QMailMessageId>());
        }
    } else {
        if (current().isValid())
            selectedIds.append(current());
    }

    return selectedIds;
}

void MessageListView::setSelected(const QMailMessageIdList& ids)
{
    if (mMarkingMode) {
        foreach (const QMailMessageId& id, ids)
            setSelected(id);
    }
}

void MessageListView::setSelected(const QMailMessageId& id)
{
    QModelIndex index = mModel->indexFromId(id);
    if (index.isValid()) {
        if (mMarkingMode) {
            mModel->setData(index, static_cast<int>(Qt::Checked), Qt::CheckStateRole);
        } else {
            QModelIndex filteredIndex(mFilterModel->mapFromSource(index));
            if (filteredIndex.isValid())
                mMessageList->setCurrentIndex(filteredIndex);
        }
    }
}

void MessageListView::selectAll()
{
    bool modified(false);

    for (int i = 0, count = mFilterModel->rowCount(); i < count; ++i) {
        QModelIndex idx(mFilterModel->index(i, 0));
        if (static_cast<Qt::CheckState>(idx.data(Qt::CheckStateRole).toInt()) == Qt::Unchecked) {
            mFilterModel->setData(idx, static_cast<int>(Qt::Checked), Qt::CheckStateRole);
            modified = true;
        }
    }
        
    if (modified)
        emit selectionChanged();
}

void MessageListView::clearSelection()
{
    bool modified(false);

    for (int i = 0, count = mFilterModel->rowCount(); i < count; ++i) {
        QModelIndex idx(mFilterModel->index(i, 0));
        if (static_cast<Qt::CheckState>(idx.data(Qt::CheckStateRole).toInt()) == Qt::Checked) {
            mFilterModel->setData(idx, static_cast<int>(Qt::Unchecked), Qt::CheckStateRole);
            modified = true;
        }
    }
        
    if (modified)
        emit selectionChanged();
}

bool MessageListView::markingMode() const
{
    return mMarkingMode;
}

void MessageListView::setMarkingMode(bool set)
{
    if (mMarkingMode != set) {
        mMarkingMode = set;
        mDelegate->setDisplaySelectionState(mMarkingMode);

        // Re-set the delegate to force a repaint of all items
        mMessageList->setItemDelegate(mDelegate);
        emit selectionChanged();
    }
}

bool MessageListView::ignoreUpdatesWhenHidden() const
{
    return mIgnoreWhenHidden;
}

void MessageListView::setIgnoreUpdatesWhenHidden(bool ignore)
{
    if (ignore != mIgnoreWhenHidden) {
        mIgnoreWhenHidden = ignore;

        if (!ignore && mModel->ignoreMailStoreUpdates())
            mModel->setIgnoreMailStoreUpdates(false);
    }
}

void MessageListView::showEvent(QShowEvent* e)
{
    mModel->setIgnoreMailStoreUpdates(false);

    QWidget::showEvent(e);
}

void MessageListView::hideEvent(QHideEvent* e)
{
    if (mIgnoreWhenHidden)
        mModel->setIgnoreMailStoreUpdates(true);

    QWidget::hideEvent(e);
}

void MessageListView::filterTextChanged(const QString& text)
{
    mFilterModel->setFilterRegExp(QRegExp(QRegExp::escape(text)));
}

MessageListView::DisplayMode MessageListView::displayMode() const
{
    return mDisplayMode;
}

void MessageListView::setDisplayMode(const DisplayMode& m)
{
    if(m == DisplayFilter) {
#ifdef QTOPIA_HOMEUI
        bool ok = false;
        QString ret = QtopiaInputDialog::getText(this,
                                                 tr("Search"),
                                                 tr("Search"),
                                                 QLineEdit::Normal,
                                                 QtopiaApplication::Words,
                                                 QString(), mFilterEdit->text(), &ok);
        ok &= !ret.isEmpty();
        if(!ok) return; //no search criteria, so ignore mode switch.
        mDisplayMode = m;
        mFilterEdit->setText(ret);
#endif
        mFilterFrame->setVisible(true);
        mTabs->setVisible(false);
        mFilterEdit->setFocus();
    } else {
        mDisplayMode = m;
        mFilterEdit->clear();
        mFilterFrame->setVisible(false);
        mTabs->setVisible(true);
        mTabs->setCurrentIndex(static_cast<int>(mDisplayMode));
    }

#ifdef QTOPIA_HOMEUI
    mModel->setKey(keyMap().value(mDisplayMode));
    bool unified(mDisplayMode == DisplayMessages || mDisplayMode == DisplayFilter || mDisplayMode == DisplayTrash);
    mDelegate->setDisplayMode(unified ? QtopiaHomeMailMessageDelegate::QtmailUnifiedMode
                                      : QtopiaHomeMailMessageDelegate::QtmailMode);
#endif
}

void MessageListView::closeFilterButtonClicked()
{
    setDisplayMode(DisplayMessages);
}

void MessageListView::tabSelected(int index)
{
    emit displayModeChanged(static_cast<DisplayMode>(index));
}

void MessageListView::modelChanged()
{
    mMarkingMode = false;

    if (mFilterModel->rowCount() && (mMessageList->selectionMode() != QSmoothList::NoSelection)) {
        mMessageList->setCurrentIndex(mFilterModel->index(0, 0));
        emit selectionChanged();
    } else {
        mMessageList->clearSelection();
    }
}

void MessageListView::indexClicked(const QModelIndex& index)
{ 
    if (mMarkingMode) {
        bool checked(static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt()) == Qt::Checked);
        mFilterModel->setData(index, static_cast<int>(checked ? Qt::Unchecked : Qt::Checked), Qt::CheckStateRole);
        emit selectionChanged();
    } else {
        QMailMessageId id(index.data(QMailMessageListModel::MessageIdRole).value<QMailMessageId>());
        if (id.isValid()) {
#ifdef QTOPIA_HOMEUI
            QPoint pos = mMessageList->clickPos();
            if (mDelegate->replyButtonRect(mMessageList->visualRect(index)).contains(pos)) {
                emit resendRequested(QMailMessage(id), 0);
                return;
            }
#endif
            emit clicked(id);
        }
    }
}

void MessageListView::currentIndexChanged(const QModelIndex& currentIndex,
                                          const QModelIndex& previousIndex)
{
    QMailMessageId currentId(currentIndex.data(QMailMessageListModel::MessageIdRole).value<QMailMessageId>());
    QMailMessageId previousId(previousIndex.data(QMailMessageListModel::MessageIdRole).value<QMailMessageId>());

    emit currentChanged(currentId, previousId);

    if (!mMarkingMode)
        emit selectionChanged();
}

void MessageListView::rowsAboutToBeRemoved(const QModelIndex&, int start, int end)
{
    if (mMarkingMode && !mSelectedRowsRemoved) {
        // See if any of the removed rows are currently selected
        for (int row = start; row <= end; ++row) {
            QModelIndex idx(mFilterModel->index(row, 0));
            if (static_cast<Qt::CheckState>(idx.data(Qt::CheckStateRole).toInt()) == Qt::Checked) {
                mSelectedRowsRemoved = true;
                break;
            }
        }
    }
}

void MessageListView::layoutChanged()
{
    if (mSelectedRowsRemoved) {
        mSelectedRowsRemoved = false;
        emit selectionChanged();
    }
}

#include "messagelistview.moc"

