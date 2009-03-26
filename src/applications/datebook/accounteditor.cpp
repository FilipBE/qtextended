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

#include "accounteditor.h"
#include "googleaccount.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QProgressBar>
#include <QAppointmentModel>
#include <QVBoxLayout>
#include <QPimContext>
#include <QAction>
#include <QSoftMenuBar>
#include <QMenu>
#include <QDialog>
#include <QTimer>
#include <QtopiaApplication>
#include <QKeyEvent>
#include <QPimDelegate>
#include <QPainter>
#include <QtopiaItemDelegate>
#include <private/qgooglecontext_p.h>

#ifndef QT_NO_OPENSSL


class AccountWidgetItem : public QListWidgetItem
{
public:
    // constructors based of known types.
    AccountWidgetItem(const QPimSource &source, QGoogleCalendarContext *context, QListWidget *parent = 0)
        : QListWidgetItem(context->icon(),
                context->name(source.identity).isEmpty()
                    ? context->email(source.identity) : context->name(source.identity),
                parent)
        , mSource(source), mContext(context)
        {
            if (!context->name(source.identity).isEmpty())
                setData(EmailRole, context->email(source.identity));
        }

    AccountWidgetItem(QPimContext *context, QListWidget *parent = 0)
        : QListWidgetItem(context->icon(), context->title(), parent)
        , mContext(context) {}

    ~AccountWidgetItem() {}

    void updateText()
    {
        QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext*>(mContext);
        if (gcal) {
            QString name = gcal->name(mSource.identity);
            QString email = gcal->email(mSource.identity);
            if (name.isEmpty()) {
                setText(email);
                setData(EmailRole, QVariant());
            } else {
                setText(name);
                setData(EmailRole, email);
            }
        }
    }

    QPimSource source() const { return mSource; }
    QPimContext *context() const { return mContext; }

    enum {EmailRole = Qt::UserRole};

private:
    const QPimSource mSource;
    QPimContext *mContext;
};


class AccountDelegate : public QPimDelegate
{
    Q_OBJECT;

public:
    AccountDelegate(QObject *parent);
    virtual ~AccountDelegate();
protected:
    QList<StringPair> subTexts ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    int subTextsCountHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    void drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index,
        QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const;
    QSize decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& s) const;

protected:
    QSize mPrimarySize;
};

AccountDelegate::AccountDelegate(QObject *parent)
    : QPimDelegate(parent)
{
    int dim = qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize);
    mPrimarySize = QSize(dim,dim);
}

AccountDelegate::~AccountDelegate()
{

}

int AccountDelegate::subTextsCountHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
    // Always one line of subtexts
    return 1;
}

QList<StringPair> AccountDelegate::subTexts(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);

    QList< StringPair > subTexts;
    QString email = index.model()->data(index, AccountWidgetItem::EmailRole).toString();
    if (!email.isEmpty())
        subTexts.append(qMakePair(QString(), email));
    return subTexts;
}

void AccountDelegate::drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index,
        QList<QRect>& leadingFloats, QList<QRect>& ) const
{
    Q_UNUSED(option);

    QIcon pi = qvariant_cast<QIcon>(index.model()->data(index, Qt::DecorationRole));
    QRect drawRect = option.rect;
    QPoint drawOffset;

    // draw the primary icon
    // 8px padding, 4 on either side
    if (rtl)
        drawRect.setLeft(drawRect.right() - mPrimarySize.width() - 8);
    else
        drawRect.setRight(mPrimarySize.width() + 8);

    drawOffset = QPoint(drawRect.left() + ((drawRect.width() - mPrimarySize.width())/2), drawRect.top() + ((drawRect.height() - mPrimarySize.height()) / 2));
    p->drawPixmap(drawOffset, pi.pixmap(mPrimarySize));

    leadingFloats.append(drawRect);
}

QSize AccountDelegate::decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& s) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(mPrimarySize.width() + s.width(), qMax(mPrimarySize.height() + 2, s.height()));
}

AccountEditor::AccountEditor( QWidget *parent )
    : QWidget(parent)
        , mModel(0)
{
    mChoices = new QListWidget;
    mChoices->setItemDelegate(new AccountDelegate(mChoices));
    mChoices->setFrameStyle(QFrame::NoFrame);
    mProgress = new QProgressBar;

    QVBoxLayout *vbl = new QVBoxLayout(this);
    vbl->setMargin(0);
    vbl->addWidget(mChoices);
    vbl->addWidget(mProgress);

    setLayout(vbl);

    connect(mChoices, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
                this, SLOT(currentAccountChanged(QListWidgetItem*)));
    connect(mChoices, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(editCurrentAccount()));
    mChoices->installEventFilter(this);

    actionAdd = new QAction(QIcon(":icon/new"), tr("Add Account"), this);
    actionAdd->setWhatsThis(tr("Create a new account"));
    connect(actionAdd, SIGNAL(triggered()), this, SLOT(addAccount()));

    actionEdit = new QAction(QIcon(":icon/edit"), tr("Edit Account"), this);
    actionEdit->setWhatsThis(tr("Edit the selected account"));
    connect(actionEdit, SIGNAL(triggered()), this, SLOT(editCurrentAccount()));

    actionRemove = new QAction(QIcon(":icon/trash"), tr("Delete Account"), this);
    actionRemove->setWhatsThis(tr("Delete the selected account"));
    connect(actionRemove, SIGNAL(triggered()), this, SLOT(removeCurrentAccount()));

    actionSync = new QAction(QIcon(":icon/sync"), tr("Sync Account"), this);
    actionSync->setWhatsThis(tr("Syncs the selected accounts"));
    connect(actionSync, SIGNAL(triggered()), this, SLOT(syncCurrentAccount()));

    actionSyncAll = new QAction(QIcon(":icon/sync"), tr("Sync All"), this);
    actionSyncAll->setWhatsThis(tr("Syncs all the accounts"));
    connect(actionSyncAll, SIGNAL(triggered()), this, SLOT(syncAllAccounts()));

    QMenu *contextMenu = QSoftMenuBar::menuFor(this);

    contextMenu->addAction(actionAdd);
    contextMenu->addAction(actionEdit);
    contextMenu->addAction(actionRemove);
    contextMenu->addAction(actionSync);
    contextMenu->addAction(actionSyncAll);

    mProgress->hide();
    populate();
    updateActions();

    progressHideTimer = new QTimer(this);
    progressHideTimer->setInterval(2000);
    progressHideTimer->setSingleShot(true);
    connect(progressHideTimer, SIGNAL(timeout()), this, SLOT(hideProgressBar()));
}

AccountEditor::~AccountEditor()
{
}

void AccountEditor::hideProgressBar()
{
    mProgress->hide();
}

/*
   Returns true if the \a model has any contexts that allow adding, editing, removing or
   syncing of accounts that are understood by this widget.   Otherwise returns false.
*/
bool AccountEditor::editableAccounts(const QAppointmentModel *model)
{
    Q_ASSERT(model != 0);

    foreach(QPimContext *c, model->contexts()) {
        QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(c);
        if (gcal)
            return true;
    }
    return false;
}

void AccountEditor::setModel(QAppointmentModel *model)
{
    mModel = model;
    foreach(QPimContext *c, mModel->contexts()) {
        QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(c);
        if (gcal) {
            connect(gcal, SIGNAL(syncProgressChanged(int,int)), this, SLOT(updateProgress()));
            connect(gcal, SIGNAL(syncStatusChanged(QString,int)),
                    this, SLOT(processSyncStatus(QString,int)));
        }
    }
    populate();
    updateActions();
}

void AccountEditor::updateAccountName(const QString& account)
{
    // Find the item with this account
    for (int i=0; i < mChoices->count(); i++) {
        AccountWidgetItem *item = (AccountWidgetItem*)mChoices->item(i);

        if (item && item->source().identity == account) {
            item->updateText();
            mChoices->update();
            break;
        }
    }
}

void AccountEditor::processSyncStatus(const QString &account, int status)
{
    QObject *s = sender();
    QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(s);
    if (gcal) {
        switch (status)
        {
            case QGoogleCalendarContext::NotStarted:
            case QGoogleCalendarContext::InProgress:
                break; // ignore
            case QGoogleCalendarContext::Completed:
                updateAccountName(account);
                progressHideTimer->start();
                break;
            default:
                // error.
                QMessageBox::critical(this, tr("Sync Error"),
                        tr("An error occurred syncing account %1. %2", "1 is account name, 2 is a status message from the synced context").arg(account).arg(gcal->statusMessage(status)));
                hideProgressBar();
                break;
        }
    }
}



void AccountEditor::addAccount()
{
    QListWidget *accountTypes = new QListWidget;
    accountTypes->setItemDelegate(new AccountDelegate(accountTypes));
    accountTypes->setFrameStyle(QFrame::NoFrame);
    QDialog diag;
    QVBoxLayout *vl = new QVBoxLayout;
    vl->addWidget(accountTypes);
    vl->setMargin(0);
    diag.setWindowTitle(tr("Account Type", "window title"));
    diag.setLayout(vl);

    foreach(QPimContext *c, mModel->contexts()) {
        QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(c);
        if (gcal) {
            new AccountWidgetItem(gcal, accountTypes);
        }
    }

    // Select the first item
    accountTypes->setCurrentRow(0);
    if (accountTypes->currentItem())
        accountTypes->currentItem()->setSelected(true);

    connect(accountTypes, SIGNAL(itemActivated(QListWidgetItem*)), &diag, SLOT(accept()));

    // and make this a menu like dialog so we can cancel
    QtopiaApplication::setMenuLike(&diag, true);

    if (QtopiaApplication::execDialog(&diag)) {
        AccountWidgetItem *item = (AccountWidgetItem*)accountTypes->currentItem();
        if (item) {
            // there can be only one....
            QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(item->context());
            if (gcal) {
                GoogleAccount gdiag;
                gdiag.setObjectName("google-account");
                if (QtopiaApplication::execDialog(&gdiag)) {
                    QString account = gcal->addAccount(gdiag.email(), gdiag.password());
                    gcal->setName(account, gdiag.name());
                    gcal->setFeedType(account, gdiag.feedType());
                }
            }
        }
    }
    populate();
    updateActions();
}

void AccountEditor::removeCurrentAccount()
{
    AccountWidgetItem *item = (AccountWidgetItem *)mChoices->currentItem();
    if (!item)
        return;

    QString id = item->source().identity;
    QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(item->context());

    if (gcal) {
        if (!gcal->name(id).isEmpty())
            id = gcal->name(id);
        else
            id = gcal->email(id);
    }

    if (Qtopia::confirmDelete(this, tr("Delete Account"), id)) {
        int row = mChoices->currentRow();
        if (gcal) {
            gcal->removeAccount(item->source().identity);
            mChoices->takeItem(mChoices->row(item));
            delete item;
        }
        populate();
        if (row >= mChoices->count())
            row = mChoices->count() - 1;
        if (row >= 0)
            mChoices->setCurrentRow(row);
        if (mChoices->currentItem())
            mChoices->currentItem()->setSelected(true);
        updateActions();
    }
}

void AccountEditor::editCurrentAccount()
{
    AccountWidgetItem *item = (AccountWidgetItem *)mChoices->currentItem();
    if (!item)
        return;
    QGoogleCalendarContext *gcal;
    gcal = qobject_cast<QGoogleCalendarContext *>(item->context());
    if (gcal) {
        GoogleAccount diag;
        diag.setObjectName("google-account");
        QString account = item->source().identity;
        QString oldEmail = gcal->email(account);
        QGoogleCalendarContext::FeedType oldFeedType = gcal->feedType(account);
        diag.setEmail(oldEmail);
        diag.setPassword(gcal->password(account));
        diag.setName(gcal->name(account));
        diag.setFeedType(gcal->feedType(account));
        if (QtopiaApplication::execDialog( &diag )) {
            if (oldEmail != diag.email() || oldFeedType != diag.feedType()) {
                gcal->removeAccount(account);
                account = gcal->addAccount(diag.email(), diag.password());
            } else {
                gcal->setPassword(account, diag.password());
            }
            gcal->setName(account, diag.name());
            gcal->setFeedType(account, diag.feedType());
        }
    }
    populate();
    updateActions();
}

void AccountEditor::syncAllAccounts()
{
    for (int i = 0; i < mChoices->count(); ++i) {
        AccountWidgetItem *accountItem = (AccountWidgetItem *)mChoices->item(i);
        QGoogleCalendarContext *gcal;
        gcal = qobject_cast<QGoogleCalendarContext *>(accountItem->context());
        if (gcal) {
            gcal->syncAccount(accountItem->source().identity);
            continue;
        }
    }
    updateProgress();
}

void AccountEditor::syncCurrentAccount()
{
    AccountWidgetItem *item = (AccountWidgetItem *)mChoices->currentItem();
    if (!item)
        return;
    QGoogleCalendarContext *gcal;
    gcal = qobject_cast<QGoogleCalendarContext *>(item->context());
    if (gcal) {
        gcal->syncAccount(item->source().identity);
        updateProgress();
        return; // redundent, left here for future account types.
    }
}

void AccountEditor::currentAccountChanged(QListWidgetItem *qlwi)
{
    if (qlwi != NULL) {
        qlwi->setSelected(true);
    }
    updateActions();
}

bool AccountEditor::eventFilter(QObject *o, QEvent *e)
{
    if (o) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *kk = (QKeyEvent*) e;
            if( o == mChoices) {
                if (kk->key() == Qt::Key_Select) {
                    if (mChoices->count() == 0)
                        addAccount();
                    else
                        editCurrentAccount();
                    return true;
                }
            }
        }
    }
    return false;
}

void AccountEditor::updateActions()
{
    bool hasCurrent;
    bool currentSyncable;
    bool anySyncable;
    AccountWidgetItem *accountItem = (AccountWidgetItem *)mChoices->currentItem();
    if (accountItem) {
        hasCurrent = true;
        if (accountItem->context()->inherits("QGoogleCalendarContext"))
            currentSyncable = true;
        else
            currentSyncable = false;
    } else {
        hasCurrent = false;
        currentSyncable = false;
    }

    if (currentSyncable) {
        anySyncable = true;
    } else {
        anySyncable = false;
        for (int i = 0; i < mChoices->count(); ++i) {
            AccountWidgetItem *accountItem = (AccountWidgetItem *)mChoices->item(i);
            QGoogleCalendarContext *gcal;
            gcal = qobject_cast<QGoogleCalendarContext *>(accountItem->context());
            if (gcal) {
                anySyncable = true;
                break;
            }
        }
    }

    actionEdit->setVisible(hasCurrent);
    actionRemove->setVisible(hasCurrent);
    actionSync->setVisible(currentSyncable);
    actionSyncAll->setVisible(anySyncable);

    if (hasCurrent)
        QSoftMenuBar::setLabel(mChoices, Qt::Key_Select, QSoftMenuBar::Edit);
    else
        QSoftMenuBar::setLabel(mChoices, Qt::Key_Select, "new", tr("New"));
}

void AccountEditor::updateProgress()
{
    if (mProgress->isHidden())
        mProgress->show();
    int amount = 0;
    int total = 0;
    foreach(QPimContext *c, mModel->contexts()) {
        QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(c);
        if (gcal) {
            int a, t;
            gcal->syncProgress(a, t);
            if (t <= 0 || a > t) {
                mProgress->setValue(0);
                mProgress->setMaximum(0);
                return;
            }
            amount += a;
            amount += t;
        }
    }
    mProgress->setMaximum(total);
    mProgress->setValue(amount);
}

void AccountEditor::populate()
{
    if (!mModel)
        return;
    mChoices->clear();
    foreach(QPimContext *c, mModel->contexts()) {
        QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(c);
        if (gcal) {
            QStringList accounts = gcal->accounts();
            QPimSource s;
            s.context = gcal->id();
            foreach(QString a, accounts) {
                s.identity = a;
                new AccountWidgetItem(s, gcal, mChoices);
            }
        }
    }
}

#include "accounteditor.moc"
#endif
