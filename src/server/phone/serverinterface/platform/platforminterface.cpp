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

#include "platforminterface.h"
#include "qabstractmessagebox.h"
#include "qabstractbrowserscreen.h"
#include "qtopiaserverapplication.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>


/*!
    \class PlatformServerInterface
    \inpublicgroup QtBaseModule
    \brief The PlatformServerInterface class provides a simple main widget for Qt Extended.
    \ingroup QtopiaServer::PhoneUI

    It displays a "welcome" screen and enables access to the application browser.

    \sa QAbstractServerInterface
*/

/*!
    Creates a new PlatformServerInterface instance with the given \a parent and
    widget \a flags.
*/
PlatformServerInterface::PlatformServerInterface(QWidget* parent, Qt::WFlags flags)
    : QAbstractServerInterface(parent, flags), browser(0)
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->screenGeometry(desktop->primaryScreen());
    setGeometry(desktopRect);

    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin( 5 );
    vb->setSpacing( 10 );
    vb->addSpacing(35);
    QLabel *label = new QLabel(tr("Welcome to Qt Extended"));
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);
    vb->addWidget(label);
    QLabel *pixmap = new QLabel();
    pixmap->setAlignment(Qt::AlignCenter);
    pixmap->setPixmap(QPixmap(":image/qtopia"));
    vb->addWidget(pixmap);

    initBrowser();

    vb->addSpacing(25);
    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->addStretch();
    QPushButton *btn = new QPushButton(tr("Details"));
    btn->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
    hbox->addWidget(btn);
    QObject::connect(btn, SIGNAL(clicked()), this, SLOT(clicked()));
    hbox->addStretch();
    vb->addLayout(hbox);

    QHBoxLayout* hbox2 = new QHBoxLayout();
    hbox2->addStretch();
    QPushButton *browserBtn = new QPushButton(tr("Applications"));
    browserBtn->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
    QObject::connect(browserBtn, SIGNAL(clicked()), this, SLOT(showBrowser()));
    hbox2->addWidget(browserBtn);
    hbox2->addStretch();
    vb->addLayout(hbox2);


    vb->addStretch(0);
}


/*!
    Destroys the PlatformServerInterface instance.
*/
PlatformServerInterface::~PlatformServerInterface()
{
    if (browser) delete browser;
}

void PlatformServerInterface::clicked()
{
    QString text(tr("This is a placeholder widget for a proper abstract server widget implementation"));
    QAbstractMessageBox * box = qtopiaWidget<QAbstractMessageBox>(this);
    if (box) {
        box->setIcon(QAbstractMessageBox::Information);
        box->setTitle(tr("Qt Extended"));
        box->setText(text);
        box->setButtons(QAbstractMessageBox::Ok, QAbstractMessageBox::NoButton);
        QtopiaApplication::execDialog(box);
        delete box;
    } else {
        QMessageBox::information(this, tr("Qt Extended"), text );
    }

}

void PlatformServerInterface::initBrowser()
{
    if (!browser) {
        browser = qtopiaWidget<QAbstractBrowserScreen>();

        if (!browser)
            qLog(Component) << "PlatformServerInterface: Missing BrowserScreen";
        else  {
            browser->move(QApplication::desktop()->screenGeometry().topLeft());
            browser->resetToView("Main");
        }
    }
}

void PlatformServerInterface::showBrowser()
{
    if (!browser) return;

    browser->showMaximized();
    browser->raise();
    browser->activateWindow();
}
QTOPIA_REPLACE_WIDGET(QAbstractServerInterface, PlatformServerInterface)
