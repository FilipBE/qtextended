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

#ifndef PREDICTIVEKEYBOARDIMPL_H
#define PREDICTIVEKEYBOARDIMPL_H

#include <QtopiaInputMethod>
#include <QWidget>
#include <QTimeLine>
#include <QTextEdit>

#include <QDebug>

#include <private/pred_p.h>
#include "predictivekeyboard.h"

class QPaintEvent;
class QMouseEvent;

class PredictiveKeyboard;
class PredictiveKeyboardWidget;
class WordPredict;
class QPaintEvent;


class PredictiveKeyboardInputMethod : public QtopiaInputMethod
{
    Q_OBJECT
public:
    PredictiveKeyboardInputMethod(QObject * =0);
    ~PredictiveKeyboardInputMethod();

    QString name() const;
    QString identifier() const;
    QString version() const;

    State state() const;
    int properties() const { return RequireMouse | InputModifier | DockedInputWidget; }
    QIcon icon() const;

    void setHint(const QString &, bool );

    // state managment.
    void reset();

    QWidget *inputWidget( QWidget* parent = 0);
    QWSInputMethod *inputModifier();

    void menuActionActivated(int v);
    QList<QIMActionDescription*> menuDescription();

private slots:
    //void sendKeyPress( ushort unicode, ushort keycode, ushort modifiers, bool press, bool repeat );
private:
    friend class PredictiveKeyboard;
    friend class PredictiveKeyboardWidget;

    PredictiveKeyboard *input;

    void queryResponse ( int property, const QVariant & result );
};

#endif
