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

#include "n810kbdhandler.h"

#ifdef QT_QWS_N810
#include <QScreen>
#include <QSocketNotifier>
#include <QTimer>

#include "qscreen_qws.h"
#include "qwindowsystem_qws.h"
#include "qapplication.h"
#include "qnamespace.h"

#include <qtopialog.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef QT_QWS_N810
#define QT_QWS_N810
#endif

//#ifdef QT_QWS_N810
#define DEVICE "/dev/input/event0"



static const N810Keys n810KeyMap[] = {
    // code, keyCode, FnKeyCode, unicode, shiftUnicode, fnUnicode
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    { 12, Qt::Key_Minus, Qt::Key_Minus, '-', '-', '_' },
    { 13, Qt::Key_Plus, Qt::Key_Plus, '+', '+', '=' },
    { 14, Qt::Key_Backspace, Qt::Key_Delete, 8, '[', '[' }, //make this FN forward delete
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    { 16, Qt::Key_Q, Qt::Key_1, 'q', 'Q', '1' },
    { 17, Qt::Key_W, Qt::Key_2, 'w', 'W', '2' },
    { 18, Qt::Key_E, Qt::Key_3, 'e', 'E', '3' },
    { 19, Qt::Key_R, Qt::Key_4, 'r', 'R', '4' },
    { 20, Qt::Key_T, Qt::Key_5, 't', 'T', '5' },
    { 21, Qt::Key_Y, Qt::Key_6, 'y', 'Y', '6' },
    { 22, Qt::Key_U, Qt::Key_7, 'u', 'U', '7' },
    { 23, Qt::Key_I, Qt::Key_8, 'i', 'I', '8' },
    { 24, Qt::Key_O, Qt::Key_9, 'o', 'O', '9' },
    { 25, Qt::Key_P, Qt::Key_0, 'p', 'P', '0' },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    { 28, Qt::Key_Return, Qt::Key_Return, 13, 13, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    { 30, Qt::Key_A, Qt::Key_Exclam, 'a', 'A', '!' },
    { 31, Qt::Key_S, Qt::Key_QuoteDbl,'s','S', '"' },
    { 32, Qt::Key_D, Qt::Key_At, 'd', 'D', '@' },
    { 33, Qt::Key_F, Qt::Key_NumberSign, 'f', 'F','#' },
    { 34, Qt::Key_G, Qt::Key_Backslash, 'g', 'G', '\\' },
    { 35, Qt::Key_H, Qt::Key_Slash, 'h', 'H', '/' },
    { 36, Qt::Key_J, Qt::Key_ParenLeft, 'j', 'J', '(' },
    { 37, Qt::Key_K, Qt::Key_ParenRight, 'k', 'K', ')' },
    { 38, Qt::Key_L, Qt::Key_Asterisk, 'l', 'L', '*'},
    { 39, Qt::Key_Semicolon, Qt::Key_Colon, ';', ':',  0xA3 }, //pound
    { 40, Qt::Key_Apostrophe, Qt::Key_Question, '\'', '\'', '?' },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    { 42, Qt::Key_Shift, Qt::Key_Shift, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    { 44, Qt::Key_Z, Qt::Key_yen, 'z', 'Z', 0xA5 }, // yen
    { 45, Qt::Key_X, Qt::Key_unknown, 'x', 'X', '^'},
    { 46, Qt::Key_C, Qt::Key_AsciiTilde, 'c', 'C', '~' },
    { 47, Qt::Key_V, Qt::Key_Percent, 'v', 'V', '%' },
    { 48, Qt::Key_B, Qt::Key_Ampersand, 'b', 'B', '&' },
    { 49, Qt::Key_N, Qt::Key_Dollar, 'n', 'N', '$' },
    { 50, Qt::Key_M, Qt::Key_unknown, 'm', 'M', 0x20AC }, //euro
    { 51, Qt::Key_Comma, Qt::Key_Less,',', '<',  '|'}, // make this FN |
    { 52, Qt::Key_Period, Qt::Key_Greater, '.', '>', '`' }, // make this FN `
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    { 57, Qt::Key_Space, Qt::Key_Space, ' ', ' ', 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    {0, Qt::Key_unknown, 0xffff, 0xffff, 0xffff, 0xffff },
    { 62, Qt::Key_Context1, 0xffff, 0xffff, 0xffff, 0xffff },
    { 63, Qt::Key_Home, 0xffff, 0xffff, 0xffff, 0xffff },
    { 64, Qt::Key_unknown , Qt::Key_unknown, 0xffff, 0xffff, 0xffff } //max
//     { 97, Qt::Key_Control, Qt::Key_Control,0xffff,0xffff },
//     { 127, Qt::Key_unknown , Qt::Key_unknown,0xffff,0xffff},// Chr
//     { 464, Qt::Key_unknown, Qt::Key_unknown, 0xffff, 0xffff  } //Fn

};

struct N810Input {
    unsigned int   dummy1;
    unsigned int   dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int   value;
};


N810KbdHandler::N810KbdHandler()
{
    numKeyPress = 0;
    keyFunction = false;
    shift = false;
    controlButton = false;

    qLog(Input) << "Loaded N810 keypad plugin";
    setObjectName( "N810 Keypad Handler" );


    kbdFD = ::open( DEVICE, O_RDONLY, 0);
    if (kbdFD >= 0) {
        qLog(Input) << "Opened event0 as keypad input";
        m_notify = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
        connect( m_notify, SIGNAL(activated(int)),
                    this, SLOT(readKbdData()));
    } else {
        qWarning("Cannot open event0 for keypad (%s)", strerror(errno));
        return;
    }


    powerFd = ::open("/dev/input/event1", O_RDONLY, 0);
    if (powerFd >= 0) {
        powerNotify = new QSocketNotifier( powerFd, QSocketNotifier::Read, this );
        connect( powerNotify, SIGNAL(activated(int)),
                    this, SLOT(readPowerKbdData()));
    } else {
        qWarning("Cannot open /dev/input/event1 for keypad (%s)", strerror(errno));
        return;
    }

    keytimer = new QTimer(this);
    connect( keytimer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    keytimer->setSingleShot(true);

}

N810KbdHandler::~N810KbdHandler()
{
    if (kbdFD >= 0) {
        ::close(kbdFD);
        ::close(powerFd);
        kbdFD = -1;
        powerFd = -1;
    }
}

const N810Keys *N810KbdHandler::keyMap() const
{
    return n810KeyMap;
}

void N810KbdHandler::readKbdData()
{

    N810Input event;
    bool useRepeat=true;

    int n = read(kbdFD, &event, sizeof(N810Input));
    if(n != (int)sizeof(N810Input)) {
        qWarning("A keypressed: n=%03d",n);
        return;
    }


    if(event.type == 0)
        return;

    int unicode = 0xffff;
    bool isPress = (event.value);
    int qtKeyCode = 0;

    qLog(Input) << "keypressed: type=" << event.type<< "code=" << event.code
                << "value=" << event.value;

    if (event.code == 97) {
        controlButton = event.value;
        return;
    }

    if (controlButton && (event.code >= 0x2c && event.code <= 0x2f)) {
        if  (event.code == 0x2c) { unicode='Z'-'@'; qtKeyCode = Qt::Key_Z; } // Undo
        else if (event.code == 0x2e) { unicode='C'-'@'; qtKeyCode = Qt::Key_C; } // Copy
        else if (event.code == 0x2f) { unicode='V'-'@'; qtKeyCode = Qt::Key_V; } // Paste
        else if (event.code == 0x2d) { unicode='X'-'@'; qtKeyCode = Qt::Key_X; } // Cut
        if (qtKeyCode) {
            processKeyEvent(unicode, qtKeyCode, Qt::ControlModifier, !isPress, false);
            return;
        }
    }

    switch(event.code)
    {
//common

    case 0x5e: //power button on top
        qtKeyCode = Qt::Key_Hangup;
        unicode  = 0xffff;
        break;
    case 0x3f:
        qtKeyCode = Qt::Key_Home;
        unicode  = 0xffff;
        break;
    case 0x3e:
        qtKeyCode = Qt::Key_Context1;
        unicode  = 0xffff;
        break;
    case 0x01:
        qtKeyCode = Qt::Key_Back;
        unicode  = 0xffff;
        break;
    case 0x74:
        qtKeyCode = Qt::Key_Hangup;
        unicode  = 0xffff;
        break;

#ifndef QT_QWS_N810
//n800
    case 0x1C:
        qtKeyCode = Qt::Key_Select;
        unicode  = 0xffff;
        break;
    case 0x48:
        qtKeyCode = transformDirKey(Qt::Key_Up);
        unicode  = 0xffff;
        break;
    case 0x50:
        qtKeyCode = transformDirKey(Qt::Key_Down);
        unicode  = 0xffff;
        break;
    case 0x4B:
        qtKeyCode = transformDirKey(Qt::Key_Left);
        unicode  = 0xffff;
        break;
    case 0x4D:
        qtKeyCode = transformDirKey(Qt::Key_Right);
        unicode  = 0xffff;
        break;
    case 0x42:
        qtKeyCode = Qt::Key_VolumeUp;
        unicode = 0xffff;
        break;
    case 0x41:
        qtKeyCode = Qt::Key_VolumeDown;
        unicode = 0xffff;
        break;

#else
// n810
/*

12 -
13 +
14 backspace
16-25: qwertyuiop
28 Enter
30-38:asdfghjkl
39 ;
40:,
42 (shift)
44-50:zxcvbnm
51 ,
52 .
57 Space
64 (maximize)
97 Ctlr
127 (chr)
464 Fn

*/
    case 0x60: //96
        qtKeyCode = Qt::Key_Select;
        unicode  = 0xffff;
        break;
    case 0x67://103
        qtKeyCode = Qt::Key_Up;
        unicode  = 0xffff;
        break;
    case 0x6c://108
        qtKeyCode = Qt::Key_Down;
        unicode  = 0xffff;
        break;
    case 0x69://105
        qtKeyCode = Qt::Key_Left;
        unicode  = 0xffff;
        break;
    case 0x6a://106
        qtKeyCode = Qt::Key_Right;
        unicode  = 0xffff;
        break;
    case 0x41://65
        qtKeyCode = Qt::Key_VolumeUp;
        unicode = 0xffff;
        break;
    case 0x42://66
        qtKeyCode = Qt::Key_VolumeDown;
        unicode = 0xffff;
        break;
    };

#endif

#ifdef QT_QWS_N810
        if (qtKeyCode == 0 ) {
            int specialKey = getKeyCode( event.code, keyFunction);

            if (event.code == 42) {
                shift = event.value;
                qtKeyCode = specialKey;
            }

            if (event.code == 97) qtKeyCode = Qt::Key_Control;

            if ( specialKey == 0 && event.value == 0) { //is function press
                keyFunction = false;
                return;
            } else {
                qtKeyCode = specialKey;
            }
        }

        if (event.code < 65 )
        unicode = getUnicode( event.code);
#endif

        processKeyEvent(unicode, qtKeyCode, Qt::NoModifier, isPress, false);

        if( isPress != 0 && event.code != 0x02 && useRepeat) {
            beginAutoRepeat(unicode, qtKeyCode, Qt::NoModifier);
        } else {
                endAutoRepeat();
        }
}

void N810KbdHandler::readPowerKbdData()
{
    N810Input event;

    int n = read(powerFd, &event, sizeof(N810Input));
    if(n != (int)sizeof(N810Input)) {
//      qWarning("P keypressed: n=%03d",n);
        return;
    }

    int unicode = 0xffff;
    int qtKeyCode = 0;
    bool isPress = (event.value);

    qLog(Input) << "keypressed power: type=" << event.type<< "code=" << event.code
                << "value=" << event.value;

    switch(event.code)
    {
    case 0x74: //116
        qtKeyCode = Qt::Key_Hangup;
        keytimer->start(1000);
        break;
    default:
//         qWarning("Dropped!");
        return; //drop any other bits
        break;
    }

    if(isPress) {
        beginAutoRepeat(unicode, qtKeyCode, Qt::NoModifier);
    } else {
        keytimer->stop();
        endAutoRepeat();
    }
}


int N810KbdHandler::getUnicode(int code)
{
    int returnCode;

    qLog(Input)<<"getUnicode" << code << shift;
    const N810Keys *currentKey = 0;
    currentKey = &keyMap()[code];

    if ( !shift) {
        if ( !keyFunction ) {
            returnCode = currentKey->unicode;
        } else {
            returnCode = currentKey->fnUnicode;
        }
    } else {
        returnCode = currentKey->shiftUnicode;
    }

    if (controlButton) {
        returnCode = currentKey->shiftUnicode - 64;

    }

    return returnCode;

}

int N810KbdHandler::getKeyCode(int code, bool isFunc)
{
    if (code == 464) {
        keyFunction = true;
        return 0;
    }

    if ( code == 42) {
        return -1;
    }

    const N810Keys *currentKey = 0;
    currentKey = &keyMap()[code];

    if ( !isFunc ) {
        qLog(Input) << "findKey" <<  currentKey->keyCode;
        return currentKey->keyCode;
    } else {
        qLog(Input) << "findKey" << currentKey->FnKeyCode;
        return currentKey->FnKeyCode;
    }

    return -2;
}

void N810KbdHandler::timerUpdate()
{
    int unicode = 0xffff;
    int qtKeyCode =  Qt::Key_Hangup; //we have only one purpose here
    bool isPress = true;

    processKeyEvent(unicode, qtKeyCode, Qt::NoModifier, isPress, false);
}


#endif // QT_QWS_N810
