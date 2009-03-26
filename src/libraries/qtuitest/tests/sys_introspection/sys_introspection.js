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

/* Selftest for widget/object introspection features */

//TESTED_COMPONENT=QA: Testing Framework (18707)

testcase = {
    initTestCase: function() {
        waitForQtopiaStart();
    },

/*
    \req QTOPIA-78

    \groups
*/
    isVisible: function() {
        startApplication("testapp1");

        verify( !isVisible("Nonexistent Thing") );
        verify( isVisible("Button1") );

        select( "Cow Tab", tabBar() );

        verify( !isVisible("CowButton15") );
        select( "CowButton15" );
        verify( isVisible("CowButton15") );
    },

/*
    \req QTOPIA-78

    \groups
*/
    verifyImage: function() {
        startApplication("testapp1");

        /* Wait until transparent clock widget has "definitely" disappeared */
        wait(5000);

        verifyImage( "foo", "", "Verify that a simple form is displayed and focus is on \"First Tab\"" );
    },

/*
    \req QTOPIA-78

    \groups
*/
    getGeometry: function() {
        startApplication("testapp1");
        /* following line could be useful if getting the error message */
        // print( "x: " + getGeometry( "Button1" ).x() + " y: " + getGeometry( "Button1" ).y() + " width: " + getGeometry( "Button1" ).width() + " height: " + getGeometry( "Button1" ).height() );
        var expected = new QRect( 8, 144, 224, 29 );
        verify ( expected == getGeometry( "Button1" ), "Returned values do not match Expected values - this maybe because of theme/UI or other changes." )
    }
} // end of test

