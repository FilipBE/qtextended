/****************************************************************************
*
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

try {
    include("accountsdata.js");
} catch(e) {
    throw(
        "Could not load account data.\n" +
        ((e instanceof Error) ? (e.toString()+" at:\n"+e.backtrace().join("\n")+"\n\n") : "\n") + 
        "In order to run this testcase, it is necessary to create an accountsdata.js file\n" +
        "containing the details of test accounts.  Please read the comments in accounts.js\n" +
        "for more information.");
}

/*
    HOW TO ADD TEST ACCOUNT DETAILS

    To make test accounts available to testcases, create an accountsdata.js file in the same
    directory as accounts.js.  accountsdata.js must define a single object named AccountsData
    with the fields described below.

    ****************
    WARNING:    Do not add in-use business/personal mail/sms or other account details here.
                Mail can and is deleted, or 'expunged' from accounts within testcases
    ****************

    AccountsData.blackHoleEmailAddress:
        email address which successfully receives and discards mail

    AccountsData.mmsServer:
        An object containing one field for each supported MMS provider.  The key is
        the provider name and the value is the provider URI.

    AccountsData.accounts:
        An object containing one field for each defined test account.
        The key is an identifier used for the test account (by convention, an integer).
        The value is an object containing any number of the following fields:

        e-mail related:
            accountname: User selected name for account as to be shown in list
            mailtype: "POP" or "IMAP"
            username: account user name
            password: account password
            inserver: incoming email server
            inport: Incoming email port number
            inencryption: "None" or "SSL"
            deletemail: Delete mail from server, 0 or 1
            skiplarger: Skip large emails, 0 or 1
            skipsize: Size of emails to consider skipping
            interval: Interval checking, 0 or 1
            intervaltime: Interval checking time
            roaminginterval: Interval checking while roaming, 0 or 1
            push: push enabled, 0 or 1
            outencryption: "None" or "SSL" or "TLS"
            alias: Name of sender
            outserver: outgoing mail account server
            address: email address in full
            outport: outgoing email port number
            authentication: "Login" or "Plain" or "POP"
            authusername: Authentication user name
            authpassword: Authentication user password

        gtalk-related:
            alias:      name that other users see
            username:   login user name
            password:   login pass word
            server:     login server
            component:  login component - e.g. @gmail.com
            port:       port number

    Example:

        AccountsData = {
            blackHoleEmailAddress: "blackhole@example.com",
            mmsServer: {
                FoneBone: "http://fonebone.example.com:8002"
            },
            accounts: {
                0: {
                    accountname: "Bob01",
                    mailtype:    "POP",
                    username:    "bob01",
                    password:    "boblovesjane",
                    inserver:    "pop.example.com",
                    outserver:   "smtp.example.com",
                    address:     "bob01@example.com",
                    alias:       "Bob Jones the First"
                },
                1: {
                    alias:       "Robot Friend",
                    username:    "r.friend007",
                    password:    "ten_gigahertz_pal",
                    server:      "talk.example.com",
                    component:   "@example.com",
                    port:        5223
                }
            }
        }
*/

Accounts = {
    blackHoleEmailAddress: function()
    { return AccountsData.blackHoleEmailAddress;      },

    getAccount: function(accountNumber)
    { return AccountsData.accounts[accountNumber]     },

    mmsServer: function( serviceprovider )
    { return AccountsData.mmsServer[serviceprovider]; }
};

