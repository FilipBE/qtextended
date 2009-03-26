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

#include "themecontrol.h"
#include "serverthemeview.h"
#include "themedcontextlabel.h"
#include <QVBoxLayout>
#include <QTimer>
#include <ThemeItem>
#include <QThemedView>
#include <QThemeItem>
#include <QThemeImageItem>
#include <QThemeTextItem>

/*!
    \class ThemedContextLabel
    \inpublicgroup QtUiModule
    \ingroup QtopiaServer::PhoneUI
    \brief The ThemedContextLabel class provides a themable dockable soft key bar for phones

    The ThemedContextLabel class provides a \l {Theming}{themable} dockable soft key bar for Qt Extended and
    is an example of how the BaseContextLabel class can be used.
    An image of this context label using the Qt Extended theme can be found in the
    \l{Server Widget Classes}{server widget gallery}.

    This class is a Qt Extended \l{QtopiaServerApplication#qt-extended-server-widgets}{server widget}.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
    Create an instance of the ThemedContextLabel class with the given \a parent
    and widget \a flags.
*/
ThemedContextLabel::ThemedContextLabel(QWidget *parent, Qt::WFlags flags)
: BaseContextLabel(parent,flags),
    themeInit(false), loadedTheme(false), themedButtons(0)
{
    phoneThemedView = new QThemedView( this );
    QObject::connect(this, SIGNAL(buttonsChanged()),
            this,SLOT(updateLabels()));
    QObject::connect(phoneThemedView, SIGNAL(itemPressed(QThemeItem*)),
            this, SLOT(itemPressed(QThemeItem*)));
    QObject::connect(phoneThemedView, SIGNAL(itemReleased(QThemeItem*)),
            this, SLOT(itemReleased(QThemeItem*)));

    if(softMenuProvider()->keyCount()) {
        themedButtons = new ThemedButton [softMenuProvider()->keyCount()];
        for(int ii = 0; ii < softMenuProvider()->keyCount(); ++ii) {
            themedButtons[ii].imgItem = 0;
            themedButtons[ii].txtItem = 0;
        }
    }
    QVBoxLayout *vb = new QVBoxLayout();
    vb->addWidget(phoneThemedView);
    vb->setContentsMargins(0, 0, 0, 0);
    setLayout(vb);
    setWindowTitle("_decoration_");
    reservedSize();

    ThemeControl *ctrl = qtopiaTask<ThemeControl>();
    if ( ctrl ) {
        ctrl->registerThemedView( phoneThemedView, "Context");
        QObject::connect(ctrl,SIGNAL(themeChanged()),this,SLOT(themeLoaded()));
        themeLoaded();
    } else { 
        qLog(Component) << "BaseContextLabel: ThemeControl not available, Theme will not work properly";
    }
}

/*!
    Delete the themed context label.
*/
ThemedContextLabel::~ThemedContextLabel()
{
    delete phoneThemedView;
    delete[] themedButtons;
}

/*!
    Returns the size needed by the themed context label to get displayed properly.
*/
QSize ThemedContextLabel::reservedSize() const
{
    QSettings qpeCfg("Trolltech", "qpe");
    qpeCfg.beginGroup("Appearance");
    QString themeDir = Qtopia::qtopiaDir() + "etc/themes/";
    QString theme = qpeCfg.value("Theme").toString();
    QSettings themeCfg(themeDir + theme, QSettings::IniFormat);
    themeCfg.beginGroup("Theme");
    double percentage = themeCfg.value("ContextSize", 0.10 ).toDouble();
    QRect contextRect = qApp->desktop()->screenGeometry();
    contextRect.setHeight(qRound(percentage * contextRect.height()));
    phoneThemedView->setFixedSize(contextRect.size());
    return contextRect.size();
}

/*!
    This slot is called when the ThemeItem \a item is pressed.
    The standard implementation calls the function BaseContextLabel::buttonPressed(int).
*/
void ThemedContextLabel::itemPressed(QThemeItem *item)
{
    initializeButtons();
    buttonPressed(buttonForItem(item));
}

/*!
    This slot is called when the ThemeItem \a item is released.
    The standard implementation calls the function BaseContextLabel::buttonReleased(int).
*/
void ThemedContextLabel::itemReleased(QThemeItem *item)
{
    Q_UNUSED(item);
    initializeButtons();
    buttonReleased(buttonForItem(item));
}

/*!
    \internal
*/
void ThemedContextLabel::initializeButtons()
{
    if (loadedTheme && !themeInit) {
        int availBtns = 0;
        int maxbuttons = softMenuProvider()->keyCount() >= 3 ? 3 : softMenuProvider()->keyCount();
        QThemeImageItem *img[3] = { NULL, NULL, NULL };
        QThemeTextItem *txt[3] = { NULL, NULL, NULL };

        if (softMenuProvider()->keyCount()) {
            for (int i = 0; i < maxbuttons; i++) {
                themedButtons[i].imgItem = 0;
                themedButtons[i].txtItem = 0;
                baseButtons().at(i)->setChanged(true);
                QThemeImageItem *ii = (QThemeImageItem *)phoneThemedView->findItem("button"+QString::number(i));
                QThemeTextItem *ti = (QThemeTextItem *)phoneThemedView->findItem("tbutton"+QString::number(i));

                if (ii || ti) {
                    img[availBtns] = ii;
                    txt[availBtns] = ti;
                    availBtns++;
                }
            }
        }

        if (softMenuProvider()->keyCount()) {
            themedButtons[0].imgItem = img[0];
            themedButtons[0].txtItem = txt[0];
        }

        if (availBtns == softMenuProvider()->keyCount()) {
            for (int i = 1; i < availBtns; i++) {
                themedButtons[i].imgItem = img[i];
                themedButtons[i].txtItem = txt[i];
            }
        } else if (availBtns < softMenuProvider()->keyCount()) {
            if (availBtns == 2) {
                themedButtons[2].imgItem = img[1];
                themedButtons[2].txtItem = txt[1];
            }
        } else {
            if (softMenuProvider()->keyCount() == 2) {
                themedButtons[1].imgItem = img[2];
                themedButtons[1].txtItem = txt[2];
            }
        }
        themeInit = true;

        updateLabels();
    }
}

/*!
    \internal
*/
void ThemedContextLabel::updateLabels()
{
    initializeButtons();
    for (int idx = 0; idx < softMenuProvider()->keyCount(); idx++) {
        if (baseButtons().at(idx)->changed()) {
            if (themedButtons[idx].txtItem)
                themedButtons[idx].txtItem->setText(softMenuProvider()->key(idx).text());
            if (themedButtons[idx].imgItem)
                themedButtons[idx].imgItem->setImage(softMenuProvider()->key(idx).pixmap());
            baseButtons().at(idx)->setChanged(false);
        }
    }
}

/*! \internal */
int ThemedContextLabel::buttonForItem(QThemeItem *item) const
{
    int pressed = -1;
    for( int i = 0 ; i < softMenuProvider()->keyCount() ; ++i )
    {
        if( themedButtons[i].imgItem == item )
        {
            pressed = i;
            break;
        }
        if( themedButtons[i].txtItem == item )
        {
            pressed = i;
            break;
        }
    }
    return pressed;
}

/*! \internal */
void ThemedContextLabel::themeLoaded()
{
    loadedTheme = true;
    themeInit = false;
    QTimer::singleShot(0, this, SLOT(initializeButtons()));
}

/*! \reimp */
void ThemedContextLabel::moveEvent(QMoveEvent *e)
{
    BaseContextLabel::moveEvent(e);
    // Update bar to repaint the background in its new position.
    phoneThemedView->update();
}

QTOPIA_REPLACE_WIDGET(QAbstractContextLabel,ThemedContextLabel);
