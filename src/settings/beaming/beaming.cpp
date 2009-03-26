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
#include "beaming.h"
#include "ircontroller.h"

#include <QSignalMapper>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTimer>
#include <QLayout>
#include <QListWidget>
#include <QStringList>
#include <QLabel>
#include <qtopianamespace.h>
#include <qsoftmenubar.h>
#include <qirlocaldevice.h>

static void bold_selected_item(QListWidget *list)
{
    QListWidgetItem* item = list->currentItem();
    if ( !item )
        return;

    QFont f = item->font();
    f.setBold( false );
    for ( int i = list->count()-1; i >= 0; i-- ) {
        list->item(i)->setFont(f);
    }
    f.setBold( true );
    item->setFont(f);
}

Beaming::Beaming( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ), state( -1 ), protocol( -1 )
{
    setWindowTitle(tr("Beaming"));

    // Take care of the case where we have no Infrared adapters
    QStringList list = QIrLocalDevice::devices();
    if (list.size() == 0) {
        QVBoxLayout *vbl = new QVBoxLayout(this);
        vbl->setMargin(4);
        vbl->setSpacing(2);

        QLabel *label = new QLabel(tr("<P>No infrared devices found"));
        label->setWordWrap(true);
        vbl->addWidget(label);
        this->setLayout(vbl);
        showMaximized();
        return;
    }

    QTabWidget *tabs = new QTabWidget;

    QWidget *deviceState = new QWidget;    

    irc = new IRController(this);

    QVBoxLayout *vbl = new QVBoxLayout;

    bg = new QButtonGroup;
    QSignalMapper* sm = new QSignalMapper(this);
    for (int i=0; i<=(int)IRController::LastState; i++) {
        QRadioButton *b = new QRadioButton;
        vbl->addWidget(b);
        bg->addButton(b);
        b->setText(IRController::stateDescription((IRController::State)i));
        if ( (IRController::State)i == irc->state() )
            b->setChecked(true);
        if ( irc->state() == IRController::Off ) {
            if ( (IRController::State)i == IRController::On1Item )
                b->setFocus();
        } else {
            if ( (IRController::State)i == IRController::Off )
                b->setFocus();
        }
        connect(b,SIGNAL(clicked()), sm, SLOT(map()));
        sm->setMapping(b,i);
    }
    connect(sm,SIGNAL(mapped(int)),this,SLOT(chooseState(int)));

    deviceState->setLayout(vbl);
    tabs->addTab(deviceState, tr("Settings"));

    vbl = new QVBoxLayout;
    QWidget *encoding = new QWidget;

    int pc = irc->protocolCount();
    if ( pc ) {
        lb = new QListWidget;
        vbl->addWidget(lb);
        for (int i=0; i<pc; i++) {
            QString n = irc->protocolName(i);
            QIcon ic = irc->protocolIcon(i);
            QListWidgetItem *item = new QListWidgetItem(n);
            if (!ic.isNull())
                item->setIcon(ic);

            lb->addItem(item);

            if (i == irc->currentProtocol())
                lb->setCurrentItem(item);
        }

        bold_selected_item(lb);

        connect(lb,SIGNAL(itemActivated(QListWidgetItem*)),this,SLOT(chooseProtocol(QListWidgetItem*)));
    }

    encoding->setLayout(vbl);
    tabs->addTab(encoding, tr("Encoding"));

    vbl = new QVBoxLayout;
    vbl->addWidget(tabs);
    setLayout(vbl);

    connect(irc, SIGNAL(stateChanged(IRController::State)),
            this, SLOT(stateChanged(IRController::State)));

    // add context menu for help
    QSoftMenuBar::menuFor( this );
}

void Beaming::chooseState(int c)
{
    state = c;

    if ( state != -1 )
        irc->setState( (IRController::State) state );
}

void Beaming::chooseProtocol(QListWidgetItem *item)
{
    if ( protocol != -1 )
        irc->setProtocol( protocol );

    protocol = lb->row(item);
    bold_selected_item(lb);
}

Beaming::~Beaming()
{
}

void Beaming::stateChanged(IRController::State s)
{
    state = s;
    bg->buttons()[s]->setChecked(true);
}
