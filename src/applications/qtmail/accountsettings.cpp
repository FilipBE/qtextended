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

#include "accountsettings.h"
#include "editaccount.h"
#include "emailclient.h"
#ifndef QTOPIA_NO_MMS
#  include "mmseditaccount.h"
#endif
#include "statusdisplay.h"
#include <private/accountconfiguration_p.h>

#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qsoftmenubar.h>
#include <qpushbutton.h>
#include <qlistwidget.h>
#include <qmessagebox.h>
#include <qaction.h>
#include <qmenu.h>
#include <qpainter.h>
#include <qlayout.h>
#include <QMouseEvent>
#include <QMailAccount>
#include <QMailAccountListModel>
#include <QMailStore>
#include <QtopiaItemDelegate>
#include <QtopiaServiceRequest>
#include <QSmoothList>

static bool removableAccountType(QMailMessage::MessageType type)
{
    // Only email accounts can be removed
    return (type == QMailMessage::Email);
}


#ifdef QTOPIA_HOMEUI
#include <private/homewidgets_p.h>
#include <QStylePainter>

class DeskphoneAccountItemDelegate : public QtopiaItemDelegate
{
    Q_OBJECT
public:
    DeskphoneAccountItemDelegate(QWidget * parent = 0)
        : QtopiaItemDelegate(parent),
          mParent(parent)
    {}

    virtual ~DeskphoneAccountItemDelegate() {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
    {
        QWidget *w = qobject_cast<QWidget*>(parent());
        QFontMetrics fm(titleFont(option));
        QSize hint(w ? w->width() : 200, fm.height() * 7/4);
        return hint;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItem opt = option;

        QString accountName = index.data(QMailAccountListModel::NameTextRole).toString();
        QMailMessage::MessageType type = static_cast<QMailMessage::MessageType>(index.data(QMailAccountListModel::MessageTypeRole).toInt());

        // prepare
        painter->save();
        if (hasClipping())
            painter->setClipRect(opt.rect);

        drawBackground(painter, option, index);
        QFontMetrics fm(titleFont(opt));

        if (!imageCache) {
            QString deleteStr(tr( "Delete" ));
            const int margin = 10;
            const int width = fm.boundingRect(opt.rect, 
                                Qt::AlignHCenter | Qt::AlignVCenter,
                                deleteStr ).width() + margin*2;
            imageCache = new QImage(width,
                                    opt.rect.height(),
                                    QImage::Format_ARGB32_Premultiplied);
            imageCache->fill(0);

            QStyleOptionButton option;
            option.initFrom(mParent);
            option.rect = imageCache->rect();
            HomeActionButton::setPaletteFromColor(&option.palette,
                                                 QtopiaHome::Red);
            QStylePainter p(imageCache, mParent);
            HomeActionButton::paintButton(&option,
                                          &p,
                                          imageCache->rect(),
                                          deleteStr);
        }

        painter->setRenderHint(QPainter::Antialiasing);

        //draw button
        QRect cbrect;
        if (removableAccountType(type)) {
            cbrect = QRect(imageCache->rect());
            cbrect.moveRight(opt.rect.width());
            painter->drawImage(cbrect, *imageCache);
        }

        //draw text
        QStyle *style = QApplication::style();
        int textWidth = opt.rect.width() - cbrect.width() - 2;
        int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
        QRect textRect = QRect(textMargin, 0,
                               textWidth - textMargin, opt.rect.height());
        painter->drawText(textRect,
                          Qt::AlignLeft | Qt::AlignVCenter,
                          fm.elidedText(accountName, opt.textElideMode, textWidth ));

        painter->restore();
    }

    static QRect deleteButtonRect(const QRect &rect) {
        const int width = imageCache->rect().width();
        return QRect(rect.right()-width, rect.y(), width, rect.height()-1);
    }

private:
    QFont titleFont(const QStyleOptionViewItem &option) const
    {
        QFont fmain = option.font;
        fmain.setWeight(QFont::Bold);
        return fmain;
    }

    static QImage *imageCache;
    QWidget *mParent;
};

QImage *DeskphoneAccountItemDelegate::imageCache = 0;

class DeskphoneAccountList : public QSmoothList
{
    Q_OBJECT

public:
    DeskphoneAccountList( QWidget *parent, Qt::WFlags fl = 0 )
        :QSmoothList(parent, fl) {}
    virtual ~DeskphoneAccountList() {}
    QPoint clickPos() const {
        return cPos;
    }

protected:
    void mousePressEvent(QMouseEvent *event) {
        cPos = event->pos();
        QSmoothList::mousePressEvent(event);
    }

private:
    QPoint cPos;
};
#endif

AccountSettings::AccountSettings(EmailClient *parent, const char *name, bool modal, const QMailAccountId& defaultId )
    : QDialog(parent),
      preExisting(false)
{
    setObjectName( name );
    setModal( modal );
    AccountSettings::defaultId = defaultId;
    setWindowTitle(tr("Account settings"));
    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setContentsMargins(0, 0, 0, 0);

    QMailAccountKey accountKey;
    QList<QMailMessage::MessageType> types;
    types << QMailMessage::Email 
#ifndef QTOPIA_NO_MMS
          << QMailMessage::Mms 
#endif
#ifndef QTOPIA_NO_COLLECTIVE
          << QMailMessage::Instant
#endif
          ;
    foreach(QMailMessage::MessageType type, types)
	accountKey |= QMailAccountKey(QMailAccountKey::MessageType, type);

    accountModel = new QMailAccountListModel();
    accountModel->setKey(accountKey);
    accountModel->setSortKey(QMailAccountSortKey(QMailAccountSortKey::Id, Qt::AscendingOrder));
    
#ifdef QTOPIA_HOMEUI
    accountView = new DeskphoneAccountList(this);
    accountView->setModel(accountModel);
    accountView->setItemDelegate(new DeskphoneAccountItemDelegate(this));
#else
    accountView = new QSmoothList(this);
    accountView->setModel(accountModel);
    accountView->setItemDelegate(new QtopiaItemDelegate);
#endif
    if (accountModel->rowCount())
        accountView->setCurrentIndex(accountModel->index(0, 0));
    vb->addWidget(accountView);
    context = QSoftMenuBar::menuFor(accountView);

#ifdef QTOPIA_HOMEUI
    QPushButton *newAccountButton = new QPushButton(tr("Add account..."), this);
    connect(newAccountButton, SIGNAL(clicked()), this, SLOT(addAccount()));
    vb->addWidget(newAccountButton);
#endif
    
    addAccountAction = new QAction( QIcon(":icon/new"), tr("Add account..."), this );
    connect(addAccountAction, SIGNAL(triggered()), this, SLOT(addAccount()));
    context->addAction( addAccountAction );
    removeAccountAction = new QAction( QIcon(":icon/trash"), tr("Remove account..."), this );
    connect(removeAccountAction, SIGNAL(triggered()), this, SLOT(removeAccount()));
    context->addAction(removeAccountAction);

    statusDisplay = new StatusDisplay(this);
    statusDisplay->setVisible(false);
    vb->addWidget(statusDisplay);
    connect(accountView, SIGNAL(activated(QModelIndex)),
	    this, SLOT(accountSelected(QModelIndex)) );
    connect( context, SIGNAL(aboutToShow()), 
	     this, SLOT(updateActions()) );
    connect(parent, SIGNAL(updateProgress(uint,uint)), 
            this, SLOT(displayProgress(uint,uint)));

    retrievalAction = new QMailRetrievalAction(this);
    connect(retrievalAction, SIGNAL(activityChanged(QMailServiceAction::Activity)), 
            this, SLOT(activityChanged(QMailServiceAction::Activity)));
}

QMailAccountId AccountSettings::defaultAccountId() const
{
    return defaultId;
}

void AccountSettings::addAccount()
{
    QMailAccount newAccount;
    editAccount(&newAccount);
}

void AccountSettings::showEvent(QShowEvent* e)
{
    accountModel->setSynchronizeEnabled(true);
    QDialog::showEvent(e);
}

void AccountSettings::hideEvent(QHideEvent* e)
{
    accountModel->setSynchronizeEnabled(false);
    QDialog::hideEvent(e);
}

void AccountSettings::removeAccount()
{
    QModelIndex index = accountView->currentIndex();    
    if (!index.isValid())
      return;

    QMailAccount account(accountModel->idFromIndex(index));

    QString message = tr("<qt>Delete account: %1</qt>").arg(Qt::escape(account.accountName()));
    if (QMessageBox::warning( this, tr("Email"), message, tr("Yes"), tr("No"), 0, 0, 1 ) == 0) {
        // Display any progress signals from the parent
        statusDisplay->setVisible(true);
        emit deleteAccount(accountModel->idFromIndex(index));
        statusDisplay->setVisible(false);
    }
}

void AccountSettings::accountSelected(QModelIndex index)
{
    if (!index.isValid())
      return;

    QMailAccount account(accountModel->idFromIndex(index));

#ifdef QTOPIA_HOMEUI
    if (removableAccountType(account.messageType())) {
        QPoint pos = qobject_cast<DeskphoneAccountList*>(accountView)->clickPos();
        QRect itmRect = accountView->visualRect(index);
        if (DeskphoneAccountItemDelegate::deleteButtonRect(itmRect).contains(pos)) {
            accountView->setCurrentIndex(index);
            removeAccount();
            return;
        }
    }
#endif

    if (account.messageType() != QMailMessage::Sms)
        editAccount(&account);
}

void AccountSettings::updateActions()
{
    QModelIndex index = accountView->currentIndex();
    if (!index.isValid())
        return;

    QMailAccount account(accountModel->idFromIndex(index));
    removeAccountAction->setVisible(removableAccountType(account.messageType()));
}

void AccountSettings::editAccount(QMailAccount *account)
{
    QDialog *editAccountView;
    int ret = 0;
    bool defaultServer = (defaultAccountId() == account->id());

    AccountConfiguration config;
    if (account->id().isValid()) {
        config = AccountConfiguration(account->id());
    } else {
        // Scan through the existing account list to retrieve the first encountered username
        foreach (const QMailAccountId &id, QMailStore::instance()->queryAccounts()) {
            AccountConfiguration accountConfig(id);
            if (!accountConfig.userName().isEmpty()) {
                config.setUserName(accountConfig.userName());
                break;
            }
        }
    }

#ifndef QTOPIA_NO_MMS
    if (account->messageType() == QMailMessage::Mms) {
        MmsEditAccount *e = new MmsEditAccount(this);
        e->setModal(true);
        e->setAccount(account, &config);
        editAccountView = e;
        ret = QtopiaApplication::execDialog(editAccountView);
    } else
#endif
#ifndef QTOPIA_NO_COLLECTIVE
    if (account->messageType() == QMailMessage::Instant) {
        // Use the settings app to change configuration 
        QtopiaServiceRequest serv("Launcher", "execute(QString)");
        serv << "gtalksettings";
        serv.send();

        // If the configuration changes, the messageserver will report it
        return;
    } else
#endif
    {
        EditAccount *e = new EditAccount(this, "EditAccount");
        e->setModal(true);
        e->setAccount(account, &config, defaultServer);
        editAccountView = e;
        ret = QtopiaApplication::execDialog(editAccountView);
        defaultServer = e->isDefaultAccount();
    }

    delete editAccountView;

    if (ret == QDialog::Accepted) {
        preExisting = account->id().isValid();
        if (preExisting) {
            QMailStore::instance()->updateAccount(account, &config);
        } else {
            QMailStore::instance()->addAccount(account, &config);
            accountView->setCurrentIndex(accountModel->index(accountModel->rowCount() - 1, 0), QSmoothList::ImmediateVisible);
        }

        if (defaultServer) {
            if (defaultId != account->id()) {
                if (defaultId.isValid()) {
                    QMessageBox::warning(qApp->activeWindow(),
                                         tr("New default account"),
                                         tr("<qt>Your previous default mail account has been unchecked</qt>"),
                                         QMessageBox::Ok);
                }

		defaultId = account->id();
            }
        }

#ifndef QTOPIA_HOMEUI
        // Folders are not relevant to Home edition
        if (account->messageSources().contains("imap4", Qt::CaseInsensitive)) {
            QTimer::singleShot(0, this, SLOT(retrieveFolders()));
        }
#endif
    }
}

void AccountSettings::retrieveFolders()
{
    QModelIndex index(accountView->currentIndex());
    if (index.isValid()) {
        QMailAccountId id(accountModel->idFromIndex(index));
        
        // See if the user wants to retrieve the folders for this account
        if (QMessageBox::question(qApp->activeWindow(),
                                  preExisting ? tr("Account Modified") : tr("Account Added"),
                                  tr("Do you wish to retrieve the folder structure for this account?"),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            statusDisplay->setVisible(true);
            statusDisplay->displayStatus(tr("Retrieving folders..."));

            retrievalAction->retrieveFolders(id);
        }
    }
}

void AccountSettings::activityChanged(QMailServiceAction::Activity activity)
{
    if (activity == QMailServiceAction::Successful) {
        statusDisplay->displayStatus(tr("Folders retrieved"));
    } else if (activity == QMailServiceAction::Failed) {
        QString caption(tr("Retrieve Failure"));
        QString action(tr("%1 - Error retrieving folders: %2", "%1: account name, %2: error text"));

        const QMailServiceAction::Status status(retrievalAction->status());
        QMailAccount account(status.accountId);
        action = action.arg(account.accountName()).arg(status.text);

        qLog(Messaging) << "retrieveFolders failed:" << action;
        statusDisplay->setVisible(false);
        QMessageBox::warning(0, caption, action, QMessageBox::Ok);
    }
}

void AccountSettings::displayProgress(uint value, uint range)
{
    if (statusDisplay->isVisible()) {
        statusDisplay->displayProgress(value, range);

        if (value == 0) 
            statusDisplay->displayStatus(tr("Deleting messages"));
    }
}

#ifdef QTOPIA_HOMEUI
#include "accountsettings.moc"
#endif
