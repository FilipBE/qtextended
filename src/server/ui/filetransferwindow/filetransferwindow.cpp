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

#include "filetransferwindow.h"
#include "filetransfertask.h"
#include "qtopiaserverapplication.h"
#include "taskmanagerentry.h"

#include <qmimetype.h>
#include <qsoftmenubar.h>

#include <QListView>
#include <QAbstractListModel>
#include <QtopiaItemDelegate>
#include <QCloseEvent>
#include <QFont>
#include <QMutableListIterator>
#include <QList>
#include <QPainter>
#include <QTabWidget>
#include <QMenu>

#include <QDebug>


static QString pretty_print_size(qint64 fsize)
{
    static const char *size_suffix[] = {
        QT_TRANSLATE_NOOP("CustomPushService", "B"),
        QT_TRANSLATE_NOOP("CustomPushService", "KB"),
        QT_TRANSLATE_NOOP("CustomPushService", "MB"),
        QT_TRANSLATE_NOOP("CustomPushService", "GB"),
    };

    double max = fsize;

    int i = 0;
    for (; i < 4; i++) {
        if (max > 1024.0) {
            max /= 1024.0;
        }
        else {
            break;
        }
    }

    // REALLY big file?
    if (i == 4)
        i = 0;

    if (fsize < 1024) {
        return QString::number(fsize) + qApp->translate("CustomPushService", size_suffix[i]);
    } else {
        return QString::number(max, 'f', 2)
                + qApp->translate("CustomPushService", size_suffix[i]);
    }
}


class FileTransfer
{
public:
    enum Direction {
        Incoming,
        Outgoing
    };

    enum State {
        WaitingToTransfer,
        Transferring,
        Finished
    };

    FileTransfer();
    ~FileTransfer();
    FileTransfer(int id, Direction direction, const QString &name,
            const QString &mimeType, const QString &description);

    inline bool isValid() const { return m_valid; }
    inline int id() const { return m_id; }
    inline Direction direction() const { return m_direction; }
    inline State state() const { return m_state; }
    inline const QString &name() const { return m_name; }
    inline const QString &mimeType() const { return m_mimeType; }
    inline const QString &description() const { return m_description; }
    inline const QIcon &icon() const { return m_icon; }

    inline void setContentId(const QContentId &contentId) { m_contentId = contentId; }
    inline const QContentId &contentId() const { return m_contentId; }

    void start(int total);
    inline void setBytesTransferred(qint64 amount) { m_done = amount; }
    inline qint64 bytesTransferred() const { return m_done; }
    inline qint64 totalBytes() const { return m_total; }

    void finish(bool failed, bool aborted);
    inline bool failed() const { return m_failed; }
    inline bool aborted() const { return m_aborted; }

private:
    bool m_valid;
    int m_id;
    Direction m_direction;
    State m_state;
    QString m_name;
    QString m_mimeType;
    QString m_description;
    QContentId m_contentId;

    qint64 m_done;
    qint64 m_total;
    bool m_failed;
    bool m_aborted;

    QIcon m_icon;
};

FileTransfer::FileTransfer()
    : m_valid(false)
{
}

FileTransfer::FileTransfer(int id, Direction direction, const QString &name, const QString &mimeType, const QString &description)
    : m_valid(true),
      m_id(id),
      m_direction(direction),
      m_state(WaitingToTransfer),
      m_name(name),
      m_mimeType(mimeType),
      m_description(description),
      m_contentId(QContent::InvalidId),
      m_done(0),
      m_total(0),
      m_failed(false),
      m_aborted(false)
{
    if (m_mimeType.isEmpty())
        m_mimeType = QMimeType(name).id();

    m_icon = QMimeType(m_mimeType).icon();

    QContent c(name, false);
    if (c.isValid())
        m_contentId = c.id();
}

FileTransfer::~FileTransfer()
{
}

void FileTransfer::start(int total)
{
    m_total = total;
    m_state = Transferring;
}

void FileTransfer::finish(bool failed, bool aborted)
{
    m_failed = failed;
    m_aborted = aborted;
    m_state = Finished;
}

//=========================================================================

class FileTransferListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    FileTransferListModel(QObject *parent = 0);

    int rowCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role) const;

    QModelIndex indexFromId(int id) const;
    const FileTransfer &transferForIndex(const QModelIndex &index) const;
    FileTransfer &transferForIndex(const QModelIndex &index);

    void addTransfer(const FileTransfer &item);
    void removeFinishedTransfers();
    int count() const;

    void updateTransferProgress(int id, qint64 done, qint64 total);
    void transferFinished(int id, FileTransferTask *task, bool error, bool aborted);

private:
    int indexOfTransfer(int id) const;

    QList<FileTransfer> m_items;
};

FileTransferListModel::FileTransferListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int FileTransferListModel::rowCount(const QModelIndex &) const
{
    return m_items.size();
}

QVariant FileTransferListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 )
        return QVariant();

    const FileTransfer &item = m_items[index.row()];
    switch (role) {
        case Qtopia::AdditionalDecorationRole:
            if (item.state() == FileTransfer::Finished &&
                    !item.failed() && !item.aborted()) {
                return QIcon(":icon/ok");
            }
        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignBottom | Qt::AlignLeft);
        case Qt::DecorationRole:
            return item.icon();
        case Qt::DisplayRole:
            if (!item.description().isEmpty())
                return item.description();
            if (!item.name().contains(QDir::separator()))
                return item.name();
            // crop file path
            return QFileInfo(item.name()).fileName();
    }
    return QVariant();
}

QModelIndex FileTransferListModel::indexFromId(int id) const
{
    int i = indexOfTransfer(id);
    if (i == -1)
        return QModelIndex();
    return index(i);
}

const FileTransfer &FileTransferListModel::transferForIndex(const QModelIndex &index) const
{
    Q_ASSERT(index.row() >= 0 && index.row() < m_items.size());
    return m_items[index.row()];
}

FileTransfer &FileTransferListModel::transferForIndex(const QModelIndex &index)
{
    Q_ASSERT(index.row() >= 0 && index.row() < m_items.size());
    return m_items[index.row()];
}

void FileTransferListModel::addTransfer(const FileTransfer &item)
{
    beginInsertRows(QModelIndex(), 0, 0);
    m_items.insert(0, item);
    endInsertRows();
}

void FileTransferListModel::removeFinishedTransfers()
{
    QMutableListIterator<FileTransfer> i(m_items);
    while (i.hasNext()) {
        if (i.next().state() == FileTransfer::Finished)
            i.remove();
    }
}

int FileTransferListModel::count() const
{
    return m_items.count();
}

void FileTransferListModel::updateTransferProgress(int id, qint64 done, qint64 total)
{
    int index = indexOfTransfer(id);
    if (index == -1)
        return;
    FileTransfer &item = m_items[index];
    if (item.state() == FileTransfer::WaitingToTransfer)
        item.start(total);
    item.setBytesTransferred(done);

    emit dataChanged(this->index(index, 0), this->index(index, 0));
}

void FileTransferListModel::transferFinished(int id, FileTransferTask *task, bool error, bool aborted)
{
    int index = indexOfTransfer(id);
    if (index == -1)
        return;
    FileTransfer &item = m_items[index];
    item.finish(error, aborted);
    if (task && item.contentId() == QContent::InvalidId)
        item.setContentId(task->transferContentId(id));

    emit dataChanged(this->index(index, 0), this->index(index, 0));
}

int FileTransferListModel::indexOfTransfer(int id) const
{
    // new items are inserted at start of list, so start from there
    for (int i=0; i<m_items.size(); i++) {
        if (m_items[i].id() == id)
            return i;
    }
    return -1;
}

//===========================================================================


class FileTransferDelegate : public QtopiaItemDelegate
{
    Q_OBJECT
public:
    FileTransferDelegate(FileTransferListModel *model, QObject *parent = 0);
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index ) const;

protected:
    virtual void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                        const QRect &rect, const QString &text) const;

private:
    static int progressHorizontalMargin();
    static int progressVerticalMargin();
    static int progressBarHeight(int totalRowHeight);
    static QFont progressFont(const QModelIndex &index);
    static void getProgressRect(QRect *rect, const QStyleOptionViewItem &option);
    static const qreal PROGRESS_HEIGHT_PROPORTION;

    void drawProgressBar(QPainter *painter, const QStyleOptionViewItem &option,
                         const QRect &rect, qint64 progress, qint64 max) const;

    FileTransferListModel *m_model;
};


// this is the proportion of the height of the progress bar within the total row height
const qreal FileTransferDelegate::PROGRESS_HEIGHT_PROPORTION = (3/7.0);

FileTransferDelegate::FileTransferDelegate(FileTransferListModel *model, QObject *parent)
    : QtopiaItemDelegate(parent),
      m_model(model)
{
}

int FileTransferDelegate::progressHorizontalMargin()
{
    return QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
}

int FileTransferDelegate::progressVerticalMargin()
{
    return QApplication::style()->pixelMetric(QStyle::PM_FocusFrameVMargin);
}

int FileTransferDelegate::progressBarHeight(int totalRowHeight)
{
    return int(totalRowHeight * PROGRESS_HEIGHT_PROPORTION) + progressVerticalMargin();
}

QFont FileTransferDelegate::progressFont(const QModelIndex &index)
{
    QFont f = index.data(Qt::FontRole).value<QFont>();
    f.setPointSize(f.pointSize() - 1);
    return f;
}

void FileTransferDelegate::getProgressRect(QRect *rect, const QStyleOptionViewItem &option)
{
    *rect = option.rect;

    // margin between progress and borders
    int progressHMargin = progressHorizontalMargin();
    int progressVMargin = progressVerticalMargin();

    rect->setHeight(progressBarHeight(option.rect.height()) - progressVMargin);
    rect->moveBottom(option.rect.bottom() - progressVMargin);

    // line up with left edge of filename, taking into account the margins
    // on both side of the icon, and the margin on the side of the text
    int hOffsetForIcon = option.decorationSize.width() +
            (QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin)+1)*3;
    if (option.direction == Qt::LeftToRight)
        rect->adjust(hOffsetForIcon, 0, -progressHMargin, 0);
    else
        rect->adjust(progressHMargin, 0, -hOffsetForIcon, 0);
}

void FileTransferDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
    QRect textRect = rect;
    textRect.setY(option.rect.y());
    textRect.setHeight(option.rect.height() -
            progressBarHeight(option.rect.height()));
    QtopiaItemDelegate::drawDisplay(painter, option, textRect, text);
}

QSize FileTransferDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize sz = QtopiaItemDelegate::sizeHint(option, index);
    int minProgressHeight = QFontMetrics(progressFont(index)).height();
    int height = int(minProgressHeight / PROGRESS_HEIGHT_PROPORTION) +
            progressVerticalMargin();
    sz.setHeight(height);
    return sz;
}

void FileTransferDelegate::drawProgressBar(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, qint64 progress, qint64 max) const
{
    // set the progress bar style options
    QStyleOptionProgressBar progressOption;
    progressOption.rect = rect;
    progressOption.fontMetrics = option.fontMetrics;
    progressOption.direction = QApplication::layoutDirection();
    progressOption.minimum = 0;
    progressOption.maximum = max;
    progressOption.textAlignment = Qt::AlignCenter;
    progressOption.textVisible = true;
    progressOption.state = QStyle::State_Enabled;

    if (progress > max || max == 0) {
        progressOption.progress = 0;
        progressOption.text = QString("%1/?").arg(pretty_print_size(progress));
    } else {
        progressOption.progress = progress;
        progressOption.text = QString("%1/%2").
                arg(pretty_print_size(progress)).
                arg(pretty_print_size(max));
    }

    // draw the progress bar
    QApplication::style()->drawControl(QStyle::CE_ProgressBar,
            &progressOption, painter);
}

void FileTransferDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    // paint everything except the progress bar / result text
    QtopiaItemDelegate::paint(painter, option, index);

    QRect progressRect;
    getProgressRect(&progressRect, option);

    painter->save();
    painter->setFont(progressFont(index));
    const FileTransfer &item = m_model->transferForIndex(index);
    if (item.state() == FileTransfer::Finished) {
        if (item.aborted()) {
            painter->drawText(progressRect, Qt::AlignLeft, tr("Stopped."));
        } else if (item.failed()) {
            painter->drawText(progressRect, Qt::AlignLeft,
                    tr("Transfer error."));
        } else {
            painter->drawText(progressRect, Qt::AlignLeft,
                    QString("%1").arg(pretty_print_size(item.bytesTransferred())));
        }
    } else {
        drawProgressBar(painter, option, progressRect,
                item.bytesTransferred(), item.totalBytes());
    }
    painter->restore();
}

//====================================================================

/*
    The FilteredTransferListView class provides a filtered list view of
    file transfers.

    It will only display file transfers that match the transfer direction
    given in the constructor.
*/
class FilteredTransferListView : public QListView
{
    Q_OBJECT
public:
    FilteredTransferListView(FileTransfer::Direction direction, QWidget *parent = 0);

protected slots:
    virtual void rowsInserted(const QModelIndex &parent, int start, int end);

private:
    FileTransfer::Direction m_direction;
};

FilteredTransferListView::FilteredTransferListView(FileTransfer::Direction direction, QWidget *parent)
    : QListView(parent),
      m_direction(direction)
{
}

void FilteredTransferListView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QListView::rowsInserted(parent, start, end);

    FileTransferListModel *m = qobject_cast<FileTransferListModel*>(model());
    if (m) {
        for (int i=start; i<=end; i++) {
            const FileTransfer &transfer = m->transferForIndex(m->index(i));
            if (transfer.direction() != m_direction)
                setRowHidden(i, true);
        }
    }
}

//====================================================================

/*!
  \class FileTransferWindow
    \inpublicgroup QtInfraredModule
    \inpublicgroup QtBluetoothModule
  \ingroup QtopiaServer::GeneralUI
  \ingroup QtopiaServer::Task
  \brief The FileTransferWindow class is shown while the Qt Extended server performs a file transfer.

  A file transfer job can be created by subclassing the FileTransferTask. This window will show up once
  any one of the existing file transfer tasks sends the FileTransferTask::incomingTransferStarted()
  or the FileTransferTask::outgoingTransferStarted() signal.

  This class is part of the Qt Extended server and cannot be used by any Qt Extended application.

  \sa FileTransferTask
  */

/*!
  Creates a FileTransferWindow instance with the given \a parent and \a flags.
  */
FileTransferWindow::FileTransferWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_model(new FileTransferListModel(this)),
      m_tabs(new QTabWidget),
      m_incomingView(new FilteredTransferListView(FileTransfer::Incoming)),
      m_outgoingView(new FilteredTransferListView(FileTransfer::Outgoing)),
      m_taskManagerEntry(0),
      m_stopAction(0)
{
    setUpView(m_incomingView);
    setUpView(m_outgoingView);
    m_tabs->addTab(m_incomingView, tr("Received"));
    m_tabs->addTab(m_outgoingView, tr("Sent"));

    setUpTaskConnections();

    QMenu *menu = QSoftMenuBar::menuFor(m_tabs);
    m_stopAction = menu->addAction(QIcon(":icon/cancel"),
            tr("Stop transfer"), this, SLOT(stopCurrentTransfer()));
    m_stopAction->setVisible(false);

    setObjectName(QLatin1String("filetransferwindow"));
    setWindowTitle(QObject::tr("Send/Receive Files"));
    setCentralWidget(m_tabs);

    m_taskManagerEntry = new TaskManagerEntry(windowTitle(), "sync", this);
    connect(m_taskManagerEntry, SIGNAL(activated()), SLOT(showWindow()));
}

void FileTransferWindow::setUpView(QListView *view)
{
    view->setModel(m_model);
    view->setItemDelegate(new FileTransferDelegate(m_model, this));
    view->setAlternatingRowColors(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setFrameStyle(QFrame::NoFrame);
    view->setTextElideMode(Qt::ElideMiddle);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setSpacing(3);
    view->setUniformItemSizes(true);

    connect(view, SIGNAL(activated(QModelIndex)),
            SLOT(activated(QModelIndex)));
    connect(view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            SLOT(currentChanged(QModelIndex,QModelIndex)));

    // no items in list to begin with, so remove 'select' label
    QSoftMenuBar::setLabel(view, Qt::Key_Select, QSoftMenuBar::NoLabel);
}

void FileTransferWindow::setUpTaskConnections()
{
    // connect to all available tasks that implement FileTransferTask
    QList<FileTransferTask *> transferTasks = qtopiaTasks<FileTransferTask>();
    for (int i=0; i<transferTasks.size(); i++) {
        connect(transferTasks[i],
                    SIGNAL(outgoingTransferStarted(int,QString,QString,QString)),
                this, SLOT(outgoingTransferStarted(int,QString,QString,QString)));
        connect(transferTasks[i],
                    SIGNAL(incomingTransferStarted(int,QString,QString,QString)),
                this, SLOT(incomingTransferStarted(int,QString,QString,QString)));
        connect(transferTasks[i], SIGNAL(transferProgress(int,qint64,qint64)),
                this, SLOT(transferProgress(int,qint64,qint64)));
        connect(transferTasks[i], SIGNAL(transferFinished(int,bool,bool)),
                this, SLOT(transferFinished(int,bool,bool)));

        // all tasks will get our signal to abort a curent transfer - so maybe
        // the slots should be called directly instead of just hooking up the
        // signal, so that a task won't asked to abort a transfer that it
        // didn't start?
        connect(this, SIGNAL(abortTransfer(int)),
                transferTasks[i], SLOT(abortTransfer(int)));
    }
}

/*!
  \fn void FileTransferWindow::abortTransfer(int)
  \internal
  */

void FileTransferWindow::showWindow()
{
    showMaximized();
    activateWindow();
    raise();
}

void FileTransferWindow::activated(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const FileTransfer &file = m_model->transferForIndex(index);
    if (file.contentId() != QContent::InvalidId) {
        QContent content(file.contentId());
        content.execute();
    }
}

void FileTransferWindow::currentChanged(const QModelIndex &current, const QModelIndex &)
{
    if (!current.isValid())
        return;

    // don't show 'select' label if file can't be opened (i.e. is incoming
    // file and haven't fully received it yet)
    const FileTransfer &file = m_model->transferForIndex(current);
    QListView *view = (file.direction() == FileTransfer::Incoming ?
                m_incomingView : m_outgoingView);
    if (file.contentId() == QContent::InvalidId)
        QSoftMenuBar::setLabel(view, Qt::Key_Select, QSoftMenuBar::NoLabel);
    else
        QSoftMenuBar::setLabel(view, Qt::Key_Select, QSoftMenuBar::Select);

    m_stopAction->setVisible(file.state() != FileTransfer::Finished);
}

void FileTransferWindow::stopCurrentTransfer()
{
    QListView *view = qobject_cast<QListView *>(m_tabs->currentWidget());
    if (!view)
        return;
    QModelIndex index = view->currentIndex();
    if (!index.isValid())
        return;
    const FileTransfer &file = m_model->transferForIndex(index);
    emit abortTransfer(file.id());
}

void FileTransferWindow::incomingTransferStarted(int id, const QString &name, const QString &mimeType, const QString &description)
{
    transferStarted(true, id, name, mimeType, description);
}

void FileTransferWindow::outgoingTransferStarted(int id, const QString &name, const QString &mimeType, const QString &description)
{
    transferStarted(false, id, name, mimeType, description);
}

void FileTransferWindow::transferStarted(bool incoming, int id, const QString &name, const QString &mimeType, const QString &description)
{
    QString realMimeType = (mimeType.isEmpty() ? QMimeType(name).id() : mimeType);
    ObjectType type = objectType(realMimeType);
    if (type == OtherObjectType) {
        addTransfer(FileTransfer(
                id,
                (incoming ? FileTransfer::Incoming : FileTransfer::Outgoing),
                name,
                realMimeType,
                description));
    } else {
        // Keep track of incoming vObjects so can display error if transfer
        // fails. Don't worry about incoming transfer failures since they are
        // processed silently and apps notify users of incoming transfers.
        if (!incoming)
            m_vObjects[id] = type;
    }
}

void FileTransferWindow::addTransfer(const FileTransfer &transfer)
{
    m_model->addTransfer(transfer);
    QListView *view = (transfer.direction() == FileTransfer::Incoming ?
            m_incomingView : m_outgoingView);
    view->setCurrentIndex(m_model->indexFromId(transfer.id()));
    m_tabs->setCurrentWidget(view);

    m_stopAction->setVisible(true); // new transfer is now selected in view
    showWindow();
    m_taskManagerEntry->show();
}

void FileTransferWindow::transferProgress(int id, qint64 done, qint64 total)
{
    m_model->updateTransferProgress(id, done, total);
}

void FileTransferWindow::transferFinished(int id, bool error, bool aborted)
{
    if (m_vObjects.contains(id)) {
        if (error) {
            QString errMsg;
            if (m_vObjects[id] == VCardObject)
                errMsg = tr("vCard transfer failed");
            else if (m_vObjects[id] == VCalendarObject)
                errMsg = tr("vCalendar transfer failed");

            // use non-modal error dialog, otherwise connection is kept
            // open until dialog is closed
            QMessageBox *msgBox = new QMessageBox(QMessageBox::Warning,
                    tr("Transfer Error"), errMsg);
            msgBox->setAttribute(Qt::WA_DeleteOnClose);
            QtopiaApplication::showDialog(msgBox);
        }
        return;
    }

    QModelIndex index = m_model->indexFromId(id);

    if (!index.isValid())
        return;

    m_model->transferFinished(id, qobject_cast<FileTransferTask *>(sender()),
                error, aborted);

    QListView *view = qobject_cast<QListView*>(m_tabs->currentWidget());
    if (view->currentIndex() == index) {
        const FileTransfer &transfer = m_model->transferForIndex(index);
        if (transfer.contentId() != QContent::InvalidId) {
            // indicate file can now be opened
            QSoftMenuBar::setLabel(view, Qt::Key_Select, QSoftMenuBar::Select);
        }
        m_stopAction->setVisible(false);
    }
}

/*!
  \reimp
  */
void FileTransferWindow::closeEvent(QCloseEvent *event)
{
    // remove all completed transfers
    m_model->removeFinishedTransfers();
    if (m_model->count() == 0)
        m_taskManagerEntry->hide();

    QMainWindow::closeEvent(event);
}

FileTransferWindow::ObjectType FileTransferWindow::objectType(const QString &mimeType)
{
    if (mimeType.compare("text/x-vcard", Qt::CaseInsensitive) == 0)
        return VCardObject;
    if (mimeType.compare("text/x-vcalendar", Qt::CaseInsensitive) == 0)
        return VCalendarObject;
    return OtherObjectType;
}


// Tasks.cfg ensures the FileTransferWindow task is started later so that the
// FileTransferTask instances (e.g. BluetoothFileSendService) are started
// before the FileTransferWindow. (Otherwise, the FileTransferWindow would
// not find the FileTransferTask instances on construction.)
QTOPIA_TASK(FileTransferWindow, FileTransferWindow);

#include "filetransferwindow.moc"
