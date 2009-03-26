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

#ifndef PREDICTIVEKEYBOARD_H
#define PREDICTIVEKEYBOARD_H

#include <qwindowsystem_qws.h>
#include <qtopiaipcenvelope.h>
#include <QDebug> //tmp

/*
    PredictiveKeyboard is an input method for Qtopia.  PredictiveKeyboard displays a popup widget depiciting keys onscreen (PredictiveKeyboardWidget), and converts them into key events.

*/

class QAction;
class KeyboardWidget;
class PredictiveKeyboard : public QWSInputMethod
{
    Q_OBJECT
public:
    PredictiveKeyboard(QWidget* parent = 0);
    virtual ~PredictiveKeyboard();

    void queryResponse ( int property, const QVariant & result );

    QWidget* widget(QWidget *parent = 0);
    void resetState();
    QAction* menuActionToDuplicate(){ return mAction;};
    virtual bool filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat);
    // Suppress hidden function warnings
    virtual inline bool filter( const QPoint&, int, int ) {return false;}

signals:
    void stateChanged();

public slots:
    void checkMicroFocus();
    void erase();
    void submitWord(QString word);
    void preedit(QString text);
    void windowEvent(QWSWindow *w, QWSServer::WindowEvent e);

protected:
    virtual void updateHandler(int type);

private:
    friend class PredictiveKeyboardInputMethod;
    QAction* mAction;
    KeyboardWidget *mKeyboard;
    QWSWindow *mActive;
};

#endif
