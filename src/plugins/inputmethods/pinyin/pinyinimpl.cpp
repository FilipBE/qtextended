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

#include <QStringList>
#include <QDebug>
#include "pinyinimpl.h"
#include "pinyinim.h"

/*
   Constructs the PinyinImpl
*/
PinyinImpl::PinyinImpl(QObject *parent)
    : QtopiaInputMethod(parent), input(0), ref(0)
{
    /* XPM */
    static const char * const pix_xpm[] = {
	"16 13 2 1",
	" 	c None",
	"#	c #000000000000",
	"    #   #     # ",
	"    #    #   #  ",
	"    #           ",
	"  ##### ####### ",
	"    #    #   #  ",
	"    #    #   #  ",
	"    ###  #   #  ",
	"  ###  ######## ",
	"    #    #   #  ",
	"    #    #   #  ",
	"    #   #    #  ",
	"    #  #     #  ",
	"  ### #      #  "};
    icn = QIcon(QPixmap((const char **)pix_xpm));
}

/*
   Destroys the PinyinImpl
*/
PinyinImpl::~PinyinImpl()
{
    if (input)
	delete input;
}

/*
   Returns the QWSInputMethod provided by the plugin.
*/
QWSInputMethod *PinyinImpl::inputModifier( )
{
    if ( !input )
	input = new PinyinIM( );
    return input;
}

void PinyinImpl::setHint(const QString &hint, bool)
{
    inputModifier();
    if (hint.isEmpty() || hint == "numbers" || hint == "phone") {
        if (input && input->active()) {
            input->setActive(false);
            emit stateChanged();
        }
    } else if (!input->active()) {
        input->setActive(true);
        emit stateChanged();
    }
}

/*
   Resets the state of the input method.
*/
void PinyinImpl::reset()
{
    if ( input )
	input->reset();
}

/*
   Returns the state of the input method. this allows
   the server to hide the input method icon if no
   input methods are active.
*/
QtopiaInputMethod::State PinyinImpl::state() const
{
    return (input && input->active()) ? Ready : Sleeping;
}

/*
   Returns the icon for the input method plugin
*/
QIcon PinyinImpl::icon() const
{
    return icn;
}


/*
   Returns the name of the input method plugin suitable
   for displaying to the user.
*/
QString PinyinImpl::name() const
{
    return qApp->translate( "InputMethods", "Pinyin" );
}

/*
   Returns the name of the input method plugin suitable
   for using to identify it in code
*/
QString PinyinImpl::identifier() const
{
    return "Pinyin";
}

/*
  Returns the version string for the input method.
*/
QString PinyinImpl::version() const
{
    return "1.0.0";
}

/*
  Returns the properties describing the capability and
  requirements of the input method.

  In this case indicates that the input method should
  only be loaded if there is a keypad present and
  that the input method will modify key or mouse input.
*/
int PinyinImpl::properties() const
{
    return RequireKeypad | InputModifier;
}

/* Required to make the plugin loadable.  Note also the 
   ref member and initializing ref to 0 in the constructor
*/
QTOPIA_EXPORT_PLUGIN(PinyinImpl)
