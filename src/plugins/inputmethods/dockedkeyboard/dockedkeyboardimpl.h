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

#ifndef DOCKEDKEYBOARDIMPL_H
#define DOCKEDKEYBOARDIMPL_H

#include <inputmethodinterface.h>

class DockedKeyboard;
class KeyboardFrame;

class DockedKeyboardInputMethod : public QtopiaInputMethod
{
    Q_OBJECT
public:
    DockedKeyboardInputMethod(QObject * =0);
    ~DockedKeyboardInputMethod();

    QString name() const;
    QString identifier() const;
    QString version() const;

    void setHint(const QString& hint, bool);

    State state() const;
    int properties() const { return RequireMouse | InputModifier | InputWidget | DockedInputWidget; }
    QIcon icon() const;

    // state managment.
    void reset();

    QWidget *inputWidget( QWidget *parent );
    QWSInputMethod *inputModifier( );

private slots:
    void sendKeyPress( ushort unicode, ushort keycode, ushort modifiers, bool press, bool repeat );
private:
    DockedKeyboard *input;

    void queryResponse ( int property, const QVariant & result );
};

#endif
