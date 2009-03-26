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

#include "emaildialogphone.h"

#include <QLayout>
#include <QPainter>
#include <QKeyEvent>
#include <QMenu>
#include <QGroupBox>
#include <QPushButton>
#include <QIcon>

#include <qsoftmenubar.h>
#include <qtopiaapplication.h>
#include <qtopiaitemdelegate.h>

EmailDialog::EmailDialog( QWidget *parent, bool readonly )
    : QDialog( parent )
{
    QVBoxLayout *l = new QVBoxLayout( this );
    mList = new EmailDialogList( this, readonly );
    mList->setFrameStyle( QFrame::NoFrame );
    mList->setItemDelegate(new QtopiaItemDelegate());

    mCreatingEmail = false;

    l->addWidget( mList );
    if (!readonly) {
        mEditBox = new QGroupBox( tr("Email Address:"), this );
        mEdit = new EmailLineEdit( mEditBox );
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->addWidget(mEdit);
        if (Qtopia::mousePreferred()) {
            // Add an ok button
            QPushButton *pb = new QPushButton();
            pb->setIcon(QIcon(":icon/ok"));
            pb->setFocusPolicy(Qt::NoFocus);
            connect(pb, SIGNAL(clicked()), this, SLOT(updateCurrentText()));
            hbox->addWidget(pb);
        }
        mEditBox->setLayout(hbox);
        mEditBox->hide();

        connect( mEdit, SIGNAL(editingFinished()), this, SLOT(updateCurrentText()) );
        connect( mList, SIGNAL(editItem()), SLOT(edit()));
        connect( mList, SIGNAL(newItem()), SLOT(newEmail()));
        mEdit->installEventFilter(this);

        connect( mEdit, SIGNAL(moveUp()), mList, SLOT(moveUp()) );
        connect( mEdit, SIGNAL(moveDown()), mList, SLOT(moveDown()) );
        l->addWidget( mEditBox );
    } else {
        mEdit = 0;
        connect( mList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(accept()) );
    }
    mList->setFocus();
    connect ( mList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(currentChanged(QListWidgetItem*)));

    setWindowState(windowState() | Qt::WindowMaximized);

    setWindowTitle( tr("Email List") );
}

EmailDialog::~EmailDialog()
{
}

void EmailDialog::edit()
{
    mEdit->selectAll();
    mEditBox->show();
    mEdit->setEditFocus(true);
}

bool EmailDialog::eventFilter( QObject *o, QEvent *e )
{
    if ( o == mEdit && e->type() == QEvent::LeaveEditFocus )
        mEditBox->hide();
    return false;
}

void EmailDialog::showEvent( QShowEvent *e )
{
    QDialog::showEvent( e );
    mList->update();
}

void EmailDialog::setEmails( const QString &def, const QStringList &em )
{
    mList->setEmails( def, em );
}

QString EmailDialog::defaultEmail() const
{
    return mList->defaultEmail();
}

QStringList EmailDialog::emails() const
{
    return mList->emails();
}

QString EmailDialog::selectedEmail() const
{
    return mSelected;
}

void EmailDialog::currentChanged(QListWidgetItem *current)
{
    if (current)
        mSelected = current->text();
    else
        mSelected.clear();
    mCreatingEmail = false;
    if (mEdit)
        mEdit->setText(mSelected);
}

void EmailDialog::newEmail()
{
    Q_ASSERT(mEdit);    // only used in non read only
    mCreatingEmail = true;
    mEdit->clear();
    edit();
}

void EmailDialog::updateCurrentText()
{
    Q_ASSERT(mEdit);    // only used in non read only
    QString t = mEdit->text();
    // If we were editing, update the text.. otherwise create a new one
    if (mCreatingEmail) {
        mCreatingEmail = false;
        if (!t.simplified().isEmpty())
            mList->addEmail(t);
    } else {
        if (t.simplified().isEmpty())
            mList->deleteEmail();
        else
            mList->updateEmail( t );  // Set text of current item.
    }
    mEditBox->hide();
}


/* ================================================================== */

EmailDialogListItem::EmailDialogListItem( EmailDialogList *parent, const QString& txt, int after )
    : QListWidgetItem( txt, 0 )
{
    parent->insertItem(after, this);
}

EmailDialogList::EmailDialogList( QWidget *parent, bool ro )
    : QListWidget( parent ), readonly(ro)
{
    mDefaultPix = QIcon( ":image/email" );
    mNormalPix = QIcon();
    newItemItem = 0;

    mDefaultIndex = -1;
    if( !Qtopia::mousePreferred() )
        setEditFocus( false );
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QMenu *menu = QSoftMenuBar::menuFor( this );

    mNewAction = menu->addAction(
            QIcon(":icon/new"), tr("New"), this, SIGNAL(newItem()) );
    mSetDefaultAction = menu->addAction(
            QIcon(":icon/email"), tr("Set as default"),  this, SLOT(setAsDefault()) );
    mDeleteAction = menu->addAction(
            QIcon(":icon/trash"), tr("Delete"), this, SLOT(deleteEmail()) );

    connect( this, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(editItem(QListWidgetItem*)) );
    connect( this, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(updateMenus()));
}

void EmailDialogList::updateMenus()
{
    mSetDefaultAction->setVisible(currentItem() != newItemItem);
    mDeleteAction->setVisible(currentItem() != newItemItem);
}

void EmailDialogList::editItem(QListWidgetItem*)
{
    if (currentItem() == newItemItem)
        emit newItem();
    else
        emit editItem();
}

void EmailDialogList::setEmails( const QString &def, const QStringList &em )
{
    clear();
    if ( !readonly )
        newItemItem = new QListWidgetItem( QIcon(":image/email-new"), tr("Add Email Address"), this );
    QStringList::ConstIterator it;
    mDefaultIndex = -1;
    int idxCount = 0;
    EmailDialogListItem *prevItem = 0;
    for( it = em.begin() ; it != em.end() ; ++idxCount, ++it )
    {
        QString emTxt = (*it).simplified() ;
        if( emTxt.isEmpty() )
            continue;

        EmailDialogListItem *newItem = new EmailDialogListItem( this, emTxt, idxCount );
        if( emTxt == def && mDefaultIndex == -1 )
        {
            newItem->setIcon( mDefaultPix );
            mDefaultIndex = idxCount;
        } else {
            newItem->setIcon( mNormalPix );
        }
        prevItem = newItem;
    }
    if( count() )
    {
        setCurrentRow( 0 );
        scrollToItem(currentItem());
    } else if ( !readonly ) {
        emit newItem();
    }
}

QString EmailDialogList::defaultEmail() const
{
    if( mDefaultIndex != -1 )
        return item( mDefaultIndex )->text();
    return QString();
}

QStringList EmailDialogList::emails() const
{
    QStringList em;
    for( int i = 0 ; i < count() ; ++i )
    {
        if( item(i) != newItemItem && !item(i)->text().trimmed().isEmpty() )
            em += item( i )->text();
    }
    return em;
}

void EmailDialogList::deleteEmail()
{
    const int ci = currentRow();
    if( ci != -1  && currentItem() != newItemItem)
    {
        delete takeItem( ci );
        if( count() )
        {
            int ni = (ci > 0 ? ci-1 : 0);
            setCurrentRow(ni);
            if( ci == mDefaultIndex )
            {
                mDefaultIndex = -1;
                setAsDefault();
            }
        }
        else
        {
            mDefaultIndex = -1;
        }
    }
}

void EmailDialogList::setAsDefault()
{
    if( currentItem() != newItemItem )
    {
        if( mDefaultIndex != -1 )
            ((EmailDialogListItem *)item( mDefaultIndex ))->setIcon(mNormalPix);
        mDefaultIndex = currentRow();
        ((EmailDialogListItem *)item( mDefaultIndex ))->setIcon( mDefaultPix );
    }
}

void EmailDialogList::addEmail( const QString &email )
{
    int lastIdx = count() - 1;
    EmailDialogListItem *newItem = new EmailDialogListItem( this, email, lastIdx );
    setCurrentItem(newItem);
    scrollToItem(newItem);
    if( lastIdx == 0 )
        setAsDefault();
    else
        newItem->setIcon(mNormalPix);
    emit itemActivated(newItem);
}

void EmailDialogList::updateEmail(const QString &email)
{
    if (currentItem() != newItemItem) {
        ((EmailDialogListItem *)currentItem())->setText( email );
    }
}

void EmailDialogList::moveUp()
{
    if( !count() )
        return;

    if( ( currentRow() != -1) && item( currentRow() )->text().isEmpty() )
    {
        deleteEmail();
    }

    int curIdx = currentRow();
    --curIdx;
    if( curIdx < 0 )
        curIdx = count()-1;
    setCurrentRow( curIdx );
}

void EmailDialogList::moveDown()
{
    if( !count() )
        return;

    if( ( currentRow() != -1) && item( currentRow() )->text().isEmpty() )
    {
        deleteEmail();
    }

    int curIdx = currentRow();
    ++curIdx;
    if( curIdx >= (int) count() )
        curIdx = 0;
    setCurrentRow( curIdx );
}

EmailLineEdit::EmailLineEdit( QWidget *parent, const char *name )
    : QLineEdit( parent )
{
    QtopiaApplication::setInputMethodHint(this, "email");
    setObjectName( name );
}

void EmailLineEdit::keyPressEvent( QKeyEvent *ke )
{
    if( ke->key() == Qt::Key_Up )
    {
        emit moveUp();
        ke->accept();
    }
    else if( ke->key() == Qt::Key_Down )
    {
        emit moveDown();
        ke->accept();
    }
    else
        QLineEdit::keyPressEvent( ke );
}
