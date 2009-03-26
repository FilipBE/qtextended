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

#include "messagebox.h"

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <qtopialog.h>

#include <qmessagebox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qevent.h>
#include <QPixmap>
#include <QPushButton>
#include <QButtonGroup>

// declare PhoneMessageBoxPrivate
class PhoneMessageBoxPrivate
{
public:
    PhoneMessageBoxPrivate()
    : iconLabel(0),
    msg(0),
    btn0(PhoneMessageBox::NoButton),
    btn1(PhoneMessageBox::NoButton),
    btn2(PhoneMessageBox::NoButton),
    defaultBtnNum(0),
    escapeBtnNum(-1),
    customButton(false),
    icon(PhoneMessageBox::NoIcon),
    yesKey(0),
    vbox(0),
    showPushButtons(false) {}

    QString title;

    QLabel *iconLabel;
    QLabel *msg;

    PhoneMessageBox::Button btn0;
    PhoneMessageBox::Button btn1;
    PhoneMessageBox::Button btn2;
    int defaultBtnNum;
    int escapeBtnNum;
    bool customButton;

    PhoneMessageBox::Icon icon;

    int yesKey;

    QVBoxLayout *vbox;

    QPushButton *pushBtn0;
    QPushButton *pushBtn1;
    QPushButton *pushBtn2;

    bool showPushButtons;
};


/*!
    \class PhoneMessageBox
    \inpublicgroup QtUiModule
    \brief The PhoneMessageBox class implements the Qt Extended Phone message box.
    \ingroup QtopiaServer::PhoneUI

    An image of this message box can be found in the \l{Server Widget Classes}{server widget gallery}.

    This class is a Qt Extended \l{QtopiaServerApplication#qt-extended-server-widgets}{server widget}.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa QAbstractServerInterface, QAbstractMessageBox
*/

/*!
  Constructs a new PhoneMessageBox instance with the specified \a parent
  and widget \a flags.
  */
PhoneMessageBox::PhoneMessageBox(QWidget *parent, Qt::WFlags flags)
: QAbstractMessageBox(parent, flags)
{
    d = new PhoneMessageBoxPrivate;

    d->vbox = new QVBoxLayout(this);
#ifdef QTOPIA_HOMEUI
    d->showPushButtons = true; //XXX configured somehow
    d->vbox->setMargin(40);
#endif
    d->vbox->addStretch(1);
    QWidget *messageArea = new QWidget(this);
    d->vbox->addWidget(messageArea);
    QHBoxLayout *hb = new QHBoxLayout(messageArea);
    d->iconLabel = new QLabel(messageArea);
    d->iconLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    hb->addWidget(d->iconLabel);
    d->msg = new QLabel(messageArea);
    d->msg->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    d->msg->setWordWrap(true);
    hb->addWidget(d->msg, 100);

    if (d->showPushButtons) {
        QButtonGroup *bgroup = new QButtonGroup(this);
        QHBoxLayout *btnBox = new QHBoxLayout(this);
        btnBox->addStretch(1);
        d->vbox->addLayout(btnBox);
        d->pushBtn0 = new QPushButton(this);
        btnBox->addWidget(d->pushBtn0);
        bgroup->addButton(d->pushBtn0, 0);
        d->pushBtn0->hide();
        d->pushBtn1 = new QPushButton(this);
        btnBox->addWidget(d->pushBtn1);
        bgroup->addButton(d->pushBtn1, 1);
        d->pushBtn1->hide();
        d->pushBtn2 = new QPushButton(this);
        btnBox->addWidget(d->pushBtn2);
        bgroup->addButton(d->pushBtn2, 2);
        d->pushBtn2->hide();
        connect(bgroup, SIGNAL(buttonClicked(int)), this, SLOT(buttonClicked(int)));
        btnBox->addStretch(1);
    }
    d->vbox->addStretch(1);

    if(parent)
        parent->installEventFilter(this);
}

/*!
  \reimp
  */
void PhoneMessageBox::setButtons(Button button0, Button button1)
{
    d->btn0 = button0;
    d->btn1 = button1;

    if(d->btn1 != NoButton) {
        if(d->btn0 == Yes || d->btn0 == Ok) {
            const QList<int> &cbtns = QSoftMenuBar::keys();
            if (cbtns.count()) {
                if (cbtns[0] != Qt::Key_Back)
                    d->yesKey = cbtns[0];
                else if (cbtns.count() > 1)
                    d->yesKey = cbtns[cbtns.count()-1];
            }
            if(d->yesKey)
                QSoftMenuBar::setLabel(this, d->yesKey, "", d->btn0 == Yes ? tr("Yes") : tr("OK"));
        }
        if (d->btn1 == No || d->btn1 == Cancel) {
            QSoftMenuBar::setLabel(this, Qt::Key_Back, "", d->btn1 == No ? tr("No") : tr("Cancel"));
        }
    }

    if (d->showPushButtons) {
        if (d->btn0 == Yes) {
            d->pushBtn0->setText(tr("Yes"));
        } else if (d->btn0 == Ok) {
            d->pushBtn0->setText(tr("OK"));
        }
        d->pushBtn0->setVisible(d->btn0 == Yes || d->btn0 == Ok);

        if (d->btn1 == No) {
            d->pushBtn1->setText(tr("No"));
        } else if (d->btn1 == Cancel) {
            d->pushBtn1->setText(tr("Cancel"));
        }
        d->pushBtn1->setVisible(d->btn1 == No || d->btn1 == Cancel);

        d->pushBtn2->hide();
        layout()->activate();
    }
}

/*!
  \reimp
  */
void PhoneMessageBox::setButtons(const QString &button0Text, const QString &button1Text, const QString &button2Text,
        int defaultButtonNumber, int escapeButtonNumber)
{
    d->customButton = true;

    d->btn0 = button0Text.isEmpty() ? (Button)-2 : (Button)0;
    d->btn1 = button1Text.isEmpty() ? (Button)-2 : (Button)1;
    d->btn2 = button2Text.isEmpty() ? (Button)-2 : (Button)2;

    d->defaultBtnNum = defaultButtonNumber;
    d->escapeBtnNum = escapeButtonNumber;

    if (button1Text.isEmpty() && button2Text.isEmpty()) {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, "", button0Text.isEmpty() ? QString() : button0Text);
        return;
    }

    if (Qtopia::hasKey(Qt::Key_Context1))
        QSoftMenuBar::setLabel(this, Qt::Key_Context1, "", button0Text.isEmpty() ? QString() : button0Text);
    else if (Qtopia::hasKey(Qt::Key_Menu))
        QSoftMenuBar::setLabel(this, Qt::Key_Menu, "", button0Text.isEmpty() ? QString() : button0Text);
    else
        qLog(UI) << "Cannot set context label" << button0Text;

    if (!button1Text.isEmpty() && button2Text.isEmpty()) {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, "", button1Text);
    } else if (button1Text.isEmpty() && !button2Text.isEmpty()) {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, "", button2Text);
    } else {
        QSoftMenuBar::setLabel(this, Qt::Key_Select, "", button1Text);
        QSoftMenuBar::setLabel(this, Qt::Key_Back, "", button2Text);
    }

    if (d->showPushButtons) {
        d->pushBtn0->setVisible(!button0Text.isEmpty());
        d->pushBtn0->setText(button0Text);
        d->pushBtn1->setVisible(!button0Text.isEmpty());
        d->pushBtn1->setText(button1Text);
        d->pushBtn2->setVisible(!button0Text.isEmpty());
        d->pushBtn2->setText(button2Text);
        layout()->activate();
    }
}

/*!
  \reimp
  */
QString PhoneMessageBox::title() const
{
    return d->title;
}

/*!
  \reimp
  */
void PhoneMessageBox::setTitle(const QString &title)
{
    d->title = title;
    setWindowTitle(title);
}

/*!
  \reimp
  */
PhoneMessageBox::Icon PhoneMessageBox::icon() const
{
    return d->icon;
}

/*!
  \reimp
  */
void PhoneMessageBox::setIcon(Icon i)
{
    d->icon = i;
    QPixmap pm;
    switch (d->icon) {
        case Information:
            pm = QPixmap(":image/alert_info");
            break;
        case Warning:
            pm = QPixmap(":image/alert_warning");
            break;
        case Critical:
            pm = QMessageBox::standardIcon( QMessageBox::Critical );
            break;
        case Question:
            pm = QMessageBox::standardIcon( QMessageBox::Question );
            break;
        case NoIcon:
            pm = QMessageBox::standardIcon( QMessageBox::NoIcon );
    }
    d->iconLabel->setPixmap(pm);
}

/*!
  \reimp
  */
void PhoneMessageBox::setIconPixmap(const QPixmap& pixmap)
{
    d->icon = NoIcon;
    d->iconLabel->setPixmap( pixmap );
}

/*!
  \reimp
  */
QString PhoneMessageBox::text() const
{
    return d->msg->text();
}

/*!
  \reimp
  */
void PhoneMessageBox::setText(const QString &text)
{
    d->msg->setText(text);
}

/*!
  \internal
  */
void PhoneMessageBox::keyPressEvent(QKeyEvent *ke)
{
    if (d->customButton) {
        if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
            done(d->defaultBtnNum);
            ke->accept();
        } else if (ke->key() == Qt::Key_Escape) {
            done(d->escapeBtnNum);
            ke->accept();
        } else if (ke->key() == Qt::Key_Context1) {
            if (d->btn0 != (Button)-2 && (d->btn1 != (Button)-2 || d->btn2 != (Button)-2))
                done(d->btn0);
            ke->accept();
        } else if (ke->key() == Qt::Key_Select) {
            if (d->btn0 != (Button)-2 && d->btn1 != (Button)-2 && d->btn2 != (Button)-2)
                done(d->btn1);
            ke->accept();
        } else if (ke->key() == Qt::Key_Back) {
            if (d->btn0 != (Button)-2 && d->btn1 == (Button)-2 && d->btn2 == (Button)-2)
                done(d->btn0);
            else if (d->btn0 != (Button)-2 && d->btn1 != (Button)-2 && d->btn2 == (Button)-2)
                done(d->btn1);
            else if (d->btn0 == (Button)-2 && d->btn1 != (Button)-2 && d->btn2 == (Button)-2)
                done(d->btn1);
            else if (d->btn0 != (Button)-2 && d->btn1 == (Button)-2 && d->btn2 != (Button)-2)
                done(d->btn2);
            else if (d->btn0 != (Button)-2 && d->btn1 != (Button)-2 && d->btn2 != (Button)-2)
                done(d->btn2);
            else if (d->btn0 == (Button)-2 && d->btn1 == (Button)-2 && d->btn2 != (Button)-2)
                done(d->btn2);
            else if (d->btn0 == (Button)-2 && d->btn1 != (Button)-2 && d->btn2 != (Button)-2)
                done(d->btn2);
            ke->accept();
        } else if (ke->key() == Qt::Key_Hangup) {
            done(d->escapeBtnNum);
            ke->accept();
        }
        return;
    }

    if (ke->key() == Qt::Key_Yes || ke->key() == d->yesKey) {
        done(d->btn0);
        ke->accept();
    } else if (ke->key() == Qt::Key_No || ke->key() == Qt::Key_Back) {
        if (d->btn1 != NoButton)
            done(d->btn1);
        else
            done(d->btn0);
        ke->accept();
    }
}

/*!
    \internal
*/
bool PhoneMessageBox::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::WindowActivate && isVisible()) {
        raise();
    }

    return false;
}

/*!
    \internal
*/
void PhoneMessageBox::addContents(QWidget *c)
{
    d->vbox->addWidget(c);
}

/*!
    \internal
*/
void PhoneMessageBox::buttonClicked(int btn)
{
    switch (btn) {
    case 0:
        done(d->btn0);
        break;
    case 1:
        done(d->btn1);
        break;
    case 2:
        done(d->btn2);
        break;
    }
}

QTOPIA_REPLACE_WIDGET(QAbstractMessageBox, PhoneMessageBox);
