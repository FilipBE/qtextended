/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt3Support module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "q3button.h"
#include "qpainter.h"

QT_BEGIN_NAMESPACE

/*!
    \class Q3Button qbutton.h
    \brief The Q3Button class is a compatibility base class of button
    widgets

    \compat

    \bold{In new code, use QAbstractButton.}

    To subclass Q3Button, you must reimplement at least drawButton()
    (to draw the button's outline) and drawButtonLabel() (to draw its
    text or pixmap). It is generally advisable to reimplement
    sizeHint() as well, and sometimes hitButton() (to determine
    whether a button press is within the button).
*/

/*!
    Constructs a standard button called \a name with parent \a parent,
    using the widget flags \a f.
*/

Q3Button::Q3Button( QWidget *parent, const char *name, Qt::WindowFlags f )
    : QAbstractButton( parent, name, f )
{
}

/*!
    Destroys the button.
 */
Q3Button::~Q3Button()
{
}

/*!
    \fn void Q3Button::paintEvent( QPaintEvent *event)

    Handles paint events, received in \a event, for buttons. Small and
    typically complex buttons are painted double-buffered to reduce
    flicker. The actually drawing is done in the virtual functions
    drawButton() and drawButtonLabel().

    \sa drawButton(), drawButtonLabel()
*/
void Q3Button::paintEvent( QPaintEvent *)
{
    QPainter p(this);
    drawButton( &p );
}

/*!
    \fn void Q3Button::drawButton( QPainter *painter)

    Draws the button on the given \a painter. The default
    implementation does nothing.

    This virtual function is reimplemented by subclasses to draw real
    buttons. At some point, these reimplementations should call
    drawButtonLabel().

    \sa drawButtonLabel(), paintEvent()
*/
void Q3Button::drawButton( QPainter * )
{
}

/*!
    \fn void Q3Button::drawButtonLabel( QPainter *painter )

    Draws the button text or pixmap on the given \a painter.

    This virtual function is reimplemented by subclasses to draw real
    buttons. It is invoked by drawButton().

    \sa drawButton(), paintEvent()
*/

void Q3Button::drawButtonLabel( QPainter * )
{
}

QT_END_NAMESPACE
