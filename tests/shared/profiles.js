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

Profiles = {

    create_profile: function()
    {
        prompt(
            "Precondition: A wav audio file is available on the device.\n"+
            "Precondition: The Qtopia device is able to be called.\n"+
            "* Select 'New' from the context menu.\n"+
            "* Enter the text 'My profile' into the 'Name' field.\n"+
            "* Move the Volume slider to the penultimate to notch.\n"+
            "* Select the 'Tones' tab.\n"+
            "* In the 'Ring tone' group - Press the 'Tone' button.\n"+
            "* From the 'Select Ringtone' dialog select 'Other...'.\n"+
            "* Select a test audio file from the list.\n"+
            "* Verify that the file selected is now listed in the 'Select Ringtone' dialog.\n"+
            "* Select the added ring tone from the list.\n"+
            "* Verify that the name of the file is now listed as the 'Tone' buttons label.\n"+
            "* Select the 'Back' option.\n"
        );
    },

    create_props_profile: function()
    {
        prompt(
            "Precondition: Qtopia is in default condition.\n"+
            "Precondition: 'General' profile is active.\n"+
            "* Create a new profile called 'Props' - Select 'New' from the 'Options' context menu in 'Profiles'.\n"+
            "* Focus the 'General' profile and select 'Edit' from the 'Options' context menu.\n"+
            "* Select the 'Properties' tab.\n"+
            "* Select 'Capture Properties' from the 'Options' context menu.\n"+
            "* Verify that a dialog box appears with the check-boxes Power Management, Appearance, Call Forwarding.\n"+
            "* Select all those options and press 'Back', once they are listed - press 'Back' again.\n"+
            "* Close 'Profiles' and open the 'Appearance' application.\n"+
            "* Change the field 'Theme' to 'Crisp'.\n"+
            "* Select 'Back' and open 'Call Forwarding'.\n"+
            "* Select 'Voice Calls' tab, select 'Always' check-box.\n"+
            "* If/when prompted select 'Use other number' & 'Next', then delete the numbers present.\n"+
            "* Select 'Type number...', enter a number, accept it.\n"+
            "* It will appear in the list - select it and then return to the 'Settings' menu.\n"+
            "* Open 'Power Management' and set 'Dim light' to 10s, 'Display off' to 20s.\n"+
            "* Exit 'Power Management' and open 'Profiles'.\n"+
            "* Focus the 'Props' profile and select 'Edit' from the 'Options' context menu.\n"+
            "* Select the 'Properties' tab.\n"+
            "* Select 'Capture Properties' from the 'Options' context menu.\n"+
            "* Verify that a dialog box appears with the checkboxes Power Management, Appearance, Call Forwarding.\n"+
            "* Select all those options and press 'Back', once they are listed - press 'Back' again.\n"+
            "* Focus the 'General' profile and select 'Edit' from the 'Options' context menu.\n"+
            "* Select the 'Properties' tab.\n"+
            "* Delete each item on the list - Focus the item and select 'Delete' from the 'Options' context menu.\n"+
            "* Exit 'Profiles'.\n"
        );
    },

/*
    \usecase return the 'Selected' value stored in [Profiles] in PhoneProfile.conf
*/
    getCurrentProfile: function()
    {
        return getSetting( "$HOME/Settings/Trolltech/PhoneProfile.conf","Profiles","Selected");
    }

};

