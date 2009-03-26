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

UserInterface = {

/*
    Move to an item in the list, without actually selecting it
*/
    highlight: function(listitem)
    {
        var top = getSelectedText();
        var attempts = 100;
        while( !getSelectedText().contains(listitem) ){
            keyClick(Qt.Key_Down);
            if( attempts <= 0 )
                break;
            --attempts;
        }
        return getSelectedText().contains(listitem);
    },

/*
    Move to an item in a specified list, without actually selecting it
*/
    highlightNonFocus: function(listitem, qpath)
    {
        var top = getSelectedText(qpath);
        var attempts = 100;
        while( !getSelectedText(qpath).contains(listitem) ){
            keyClick(Qt.Key_Down);
            if( attempts <= 0 )
                break;
            --attempts;
        }
        return getSelectedText(qpath).contains(listitem);
    },

/*
    Change the input method to specified type
*/
    changePKIMtype: function(type)
    {
        // TODO: Do through user interface
        prompt( "Change the input mode to " + type );
/*        for(var i=0;i<5;++i){
            keyPress(Qt.Key_Asterisk);
            wait( 2000 );
            keyRelease(Qt.Key_Asterisk);
            wait( 500 );
        }
*/
    },

    manualHomeUI: function( text )
    {
        prompt( text );
    }
}
