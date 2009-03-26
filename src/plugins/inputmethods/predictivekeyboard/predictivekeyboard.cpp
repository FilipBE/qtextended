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

#include "predictivekeyboard.h"
#include "keyboard.h"
#include <QVariant>
#include <qwindowsystem_qws.h>
#include <QAction>
#include <QtopiaApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <qtopialog.h>
#include <QScreen>

PredictiveKeyboard::PredictiveKeyboard(QWidget* parent)
: QWSInputMethod(), mKeyboard(0), mActive(0)
{
    // The predictivekeyboard frame is meaningless after the
    // PredictiveKeyboard IM is destroyed, so keep control of it by never
    // parenting PredictiveKeyboardWidget;  This should also help keep the
    // predictivekeyboard on top of other widgets.
    Q_UNUSED(parent);

    QObject::connect(qwsServer, SIGNAL(windowEvent(QWSWindow*,QWSServer::WindowEvent)), this, SLOT(windowEvent(QWSWindow*,QWSServer::WindowEvent)));
};

PredictiveKeyboard::~PredictiveKeyboard()

{
    if(mKeyboard) {
        delete mKeyboard;
        mKeyboard = 0;
    }
};

static KeyboardWidget::Config createKeyboardConfig(QSettings& cfg)
{
    KeyboardWidget::Config config;

    QScreen * screen = QScreen::instance();

    int swidth = screen->width();
    int sheight = screen->height();

    config.minimumStrokeMotionPerPeriod =
        cfg.value("MinimumStrokeMotionPerPeriod", sheight/10).toInt();
    config.strokeMotionPeriod =
        cfg.value("StrokeMotionPeriod", 200).toInt();
    config.maximumClickStutter =
        cfg.value("MaximumClickStutter", swidth / 12).toInt();
    config.maximumClickTime =
        cfg.value("MaximumClickTime", 100).toInt();
    config.minimumPressTime =
        cfg.value("MinimumPressTime", 300).toInt();
    config.minimumStrokeDirectionRatio =
        cfg.value("MinimumStrokeDirectionRatio", 2.0f).toDouble();
    config.selectCircleDiameter =
        cfg.value("SelectCircleDiameter", swidth / 4).toInt();
    config.selectCircleOffset =
        cfg.value("SelectCircleOffset", -config.selectCircleDiameter).toInt();
    config.magnifyShowTime =
        cfg.value("MagnifyShowTime", 180).toInt();
    config.boardChangeTime =
        cfg.value("BoardChangeTime", 400).toInt();

    config.keySize.setWidth(cfg.value("KeySizeWidth", swidth).toInt());
    config.keySize.setHeight(cfg.value("KeySizeHeight",
        sheight > swidth ? sheight / 4 : sheight / 3).toInt());

    config.keyMargin =
        cfg.value("KeyMargin", swidth / 15).toInt();

    config.bottomMargin =
        cfg.value("BottomMargin", swidth / 24).toInt();
    config.maxGuesses =
        cfg.value("MaxGuesses", 5).toInt();
    config.optionWordSpacing =
        cfg.value("OptionWordSpacing", swidth / 24).toInt();
    config.optionsWindowHeight =
        cfg.value("OptionsWindowHeight", -1).toInt();
    config.reallyNoMoveSensitivity =
        cfg.value("ReallyNoMoveSensitivity", swidth / 48).toInt();
    config.moveSensitivity =
        cfg.value("MoveSensitivity", config.maximumClickStutter).toInt();
    config.excludeDistance =
        cfg.value("ExcludeDistance", (swidth * 10) / 48).toInt();

    return config;
}


QWidget* PredictiveKeyboard::widget(QWidget*)
{
    if(!mKeyboard) {
        QSettings xcfg("Trolltech", "PredictiveKeyboard");

        xcfg.beginGroup("Settings");
        KeyboardWidget::Config config = createKeyboardConfig(xcfg);
        mKeyboard = new KeyboardWidget(config, 0);
        xcfg.endGroup();

        QSettings lcfg("Trolltech", "/PredictiveKeyboardLayout");
        QString ilang = QSettings("Trolltech", "locale").value("Language/InputLanguages").toStringList().at(0);
        QSettings icfg("Trolltech", ilang+"/PredictiveKeyboardLayout");

        QSettings *cfg = icfg.contains("Board1/Rows") ? &icfg : &lcfg;

        QString bg;
        for (int b=1; cfg->contains((bg="Board"+QString::number(b))+"/Rows"); ++b) {
            cfg->beginGroup(bg);

            KeyboardWidget::BoardType type = KeyboardWidget::Other;
            QString boardtype = cfg->value("Type").toString();
            if (boardtype == "Numeric") type = KeyboardWidget::Numeric;
            else if (boardtype == "Words") type = KeyboardWidget::Words;
            else if (boardtype == "Letters") type = KeyboardWidget::Letters;

            mKeyboard->addBoard(
                type,
                cfg->value("Rows").toStringList(),
                cfg->value("Caps").toStringList(),
                cfg->value("Equivalences").toStringList()
            );

            cfg->endGroup();
        }

        QObject::connect(mKeyboard, SIGNAL(preedit(QString)), this, SLOT(preedit(QString)));
        QObject::connect(mKeyboard, SIGNAL(commit(QString)), this, SLOT(submitWord(QString)));
        QObject::connect(mKeyboard, SIGNAL(commit(QString)), this, SLOT(checkMicroFocus()));
        QObject::connect(mKeyboard, SIGNAL(backspace()), this, SLOT(erase()));
        KeyboardWidget::instantiatePopupScreen();
    }

    return mKeyboard;
};

void PredictiveKeyboard::checkMicroFocus()
{
    qwsServer->sendIMQuery ( Qt::ImMicroFocus );
}

void PredictiveKeyboard::queryResponse ( int property, const QVariant & result )
{
    if(property == Qt::ImMicroFocus) {
        QRect r = result.toRect();
        QPoint p = r.topLeft();

        if(mActive) {
            QPoint bp = mActive->requestedRegion().boundingRect().topLeft();
            p += bp;
        }

        mKeyboard->setAcceptDest(p);
    }
};

void PredictiveKeyboard::resetState()
{
    if(mKeyboard)
        mKeyboard->reset();

};

void PredictiveKeyboard::windowEvent(QWSWindow *w, QWSServer::WindowEvent e)
{
    if(e == QWSServer::Active)
        mActive = w;
}

void PredictiveKeyboard::updateHandler(int type)
{

    switch(type){
        case QWSInputMethod::Update:
            break;
        case QWSInputMethod::FocusIn:
            break;
        case QWSInputMethod::FocusOut:
            break;
        case QWSInputMethod::Reset:
            resetState();
            break;
        case QWSInputMethod::Destroyed:
            break;
    };
};


void PredictiveKeyboard::erase()
{
    // TODO: Find out how to do this properly.  Sending a key event seems to
    // have fewer unexpected consequences, but doesn't seem consistent with
    // the QIMEvents that represent all the rest of the output.
    // Sending the commit string works most of the time.

//    QWSInputMethod::sendCommitString (QString(), -1, 1); // TODO - fix case where no text to left of cursor (currently deletes character on right)

    // 8 for unicode is from pkim. It doesn't match any source I could find.
    QWSServer::sendKeyEvent (8, Qt::Key_Backspace, 0, true, false);
    QWSServer::sendKeyEvent (8, Qt::Key_Backspace, 0, false, false);
}

void PredictiveKeyboard::submitWord(QString word)
{
    QWSInputMethod::sendCommitString (word);
}

void PredictiveKeyboard::preedit(QString text)
{
    Q_UNUSED(text);
//  TODO: Get rid of the preedit flicker (on mousepress).
//  Until then, no preedit will look better.
    QWSInputMethod::sendPreeditString (text, text.length());
}

bool PredictiveKeyboard::filter ( int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat )
{
    return mKeyboard && mKeyboard->filter ( unicode, keycode, modifiers, isPress, autoRepeat );

    Q_UNUSED(unicode);
    Q_UNUSED(modifiers);
    Q_UNUSED(modifiers);
    Q_UNUSED(autoRepeat);

    if (isPress && !mKeyboard->hasText())
        return false;

    //Handle backspace
    if(keycode == Qt::Key_Back) {
        if(!isPress){
            mKeyboard->doBackspace();
        }
        return true;
    }

    //Handle Select Key
    if(keycode == Qt::Key_Select) {
        if(!isPress){
            mKeyboard->acceptWord();
        }
        return true;
    }

    return false;
}

