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

//TESTED_COMPONENT=QA: Testing Framework (18707)

testcase = {
    init: function() {
        waitForQtopiaStart();
    },

/*
    \req QTOPIA-78
*/
    putFile_data: {
        simple: [ "testfile_1.txt", "$HOME/my_test_file", "This is a test file, for copying.\n" ],
        mkpath: [ "testfile_1.txt", "$HOME/a/new/dir/tree/my_test_file", "This is a test file, for copying.\n" ]
    },

    putFile: function(source, destination, contents) {
        deletePath( destination );
        putFile( baseDataPath() + "/" + source, destination );
        compare(getData(destination), contents);
    },

/*
    \req QTOPIA-78
*/
    putData_data: {
        simple: [ "$HOME/my_test_data", "Some data, oh yeah!" ],
        mkpath: [ "$HOME/a/dir/which/did/not/previously/exist", "Got some more data for you!" ]
    },

    putData: function(destination, data) {
        deletePath( destination );
        putData( data, destination );
        compare( getData(destination), data );
    },

/*
    \req BUG-228663
*/
    putFilePermissions_data: {
        test_same: [ "testscript.sh", "$HOME/my_script.sh", undefined ],
        test_urw:  [ "testscript.sh", "$HOME/my_script_2.sh", QFile.WriteOwner | QFile.ReadOwner ],
        test_urwar:[ "testscript.sh", "$HOME/my_script_3.sh", QFile.WriteOwner | QFile.ReadOwner | QFile.ReadOther ]
    },

    putFilePermissions: function(source, destination, permissions) {
        deletePath( destination );
        putFile( baseDataPath() + "/" + source, destination, permissions );
        //TODO: Have to manually verify permissions are set correctly...
    }
} // end of test

