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

Content = {

    create_txt_file: function(filename, text)
    {
        startApplication( "Notes" );
        select( "New" );
        enter( text, undefined, NoCommit );
        select( "Accept", softMenu() );
        select( "Back", softMenu() );
        return getList().contains(filename);
    },

/*
    Rescan documents on the test system.

    When files are transferred during a test, it may be necessary to call this
    function before Qtopia's content system will see the files.
*/
    rescan: function()
    {
        return ipcSend("QPE/DocAPI", "scanPath(QString,int)", "all", 1);
    },

/*
    Put five different broad types of file - text, audio, graphic, video and binary to specified
    location.   Don't rescan system in this function as some test are testing that function.
*/
    put_test_files: function(destinationPath)
    {
        putFile( baseDataPath() + "Test_File_Text.txt", destinationPath + "text/plain/Test_File_Text.txt");
        putFile( baseDataPath() + "Test_File_Wav.wav", destinationPath + "audio/x-wav/Test_File_Wav.wav");
        putFile( baseDataPath() + "Test_File_Jpeg.jpg", destinationPath + "image/jpeg/Test_File_Jpeg.jpg");
        putFile( baseDataPath() + "Test_File_3gpp.3gp", destinationPath + "video/3gpp/Test_File_3gpp.3gp");
        putFile( baseDataPath() + "Test_File_Binary", destinationPath + "Test_File_Binary");
    },

/*
    Put five different broad types of file - text, audio, graphic, video and binary to specified
    location.   Don't rescan system in this function as some test are testing that function.
*/
    remove_test_files: function(destinationPath)
    {
        deletePath( destinationPath + "text/plain/Test_File_Text.txt");
        deletePath( destinationPath + "audio/x-wav/Test_File_Wav.wav");
        deletePath( destinationPath + "image/jpeg/Test_File_Jpeg.jpg");
        deletePath( destinationPath + "video/3gpp/Test_File_3gpp.3gp");
        deletePath( destinationPath + "Test_File_Binary");
    }

};

