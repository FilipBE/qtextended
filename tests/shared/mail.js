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

/*
    Mail handling functions for QtUiTest.

    Requirements:
        - Perl 5 (perl must be in path).
        - Mail::IMAPClient (tested with version 3.07)
        - Mail::Sendmail
        - MIME::Base64 and MIME::QuotedPrint

    The above perl modules can be installed via cpan.
*/

Mail = {

/*
    Return all new messages in the Inbox on the given IMAP server,
    connecting via port, with the given username and password.
*/
    getNewMessage: function(server, username, password, port)
    {
        var reply = runProcess( "perl", [ "-e",
        "use Mail::IMAPClient;\
        use MIME::Base64;\
        use MIME::QuotedPrint;\
        $server = Mail::IMAPClient->new(\
        Server => \"" + server + "\",\
        User=> \"" + username + "\",\
        Password => \"" + password + "\",\
        Port => \"" + port + "\",\
        Uid => \"0\",\
        Peek => \"1\" ) || die (\"Could not connect to " + server + ":" + port + " with " + username + ": $! $?\\n\");\
        $server->select( \"INBOX\" ) || die (\"Could not select INBOX\\n\");\
        $number_of_messages = $server->message_count(\"INBOX\" );\
        $|=1;\
        foreach $msg ( 1..$number_of_messages ) {\
        my $msg_subject = $server->subject( $msg );\
        print \"Subject: $msg_subject\\n\";\
        my $msg_date = $server->date( $msg );\
        print \"Date: $msg_date\\n\";\
        $msg_body = $server->body_string( $msg );\
        my $encoding = $server->get_header( $msg, \"Content-Transfer-Encoding\" );\
        if ($encoding eq \"base64\") { $msg_body = decode_base64( $msg_body ); }\
        elsif ($encoding eq \"quoted-printable\") { $msg_body = decode_qp( $msg_body ); }\
        print $msg_body; }\
        print \"\\n\";\
        $server->logout();"
        ], "" );

        return reply;
    },

/*
    Send an email:
        to      - recipient address
        subject - message subject line
        msg     - message body text
        frm     - sender name
        replyto - sender address
*/
    sendMessage: function( to, subject, msg, frm, replyto, cc, bcc )
    {

        var senderAddress = frm+"<" +replyto+ ">";
        var reply = runProcess( "perl", [ "-e",
        "use Mail::Sendmail;\
        %mail = (\
            To      => '" + to + "',\
            From    => '" + senderAddress + "',\
            Subject => '" + subject + "'," +
            ((cc != undefined) ?  "Cc  => '" + cc + "'," : "") +
            ((bcc != undefined) ? "Bcc => '" + bcc + "'," : "") +
            "Message => '" + msg + "'\
        );\
        sendmail(%mail) or die $Mail::Sendmail::error;"
        ], "" );

        return reply;
    },
/*
    Remove all messages in the Inbox on the given IMAP server, connecting via port,
    with the given username and password.
*/
    expungeMessages: function( server, username, password, port )
    {
        var reply = runProcess( "perl", [ "-e",
        "use Mail::IMAPClient;\
        $server = Mail::IMAPClient->new(\
        Server => \"" + server + "\",\
        User=> \"" + username + "\",\
        Password => \"" + password + "\",\
        Port => \"" + port + "\",\
        Uid => \"0\",\
        Peek => \"1\" ) || die (\"Could not connect to " + server + ":" + port + " with " + username + ": $! $?\n\");\
        $server->select( \"INBOX\" ) || die (\"Could not select INBOX\n\" );\
        $number_of_messages = $server->message_count( \"INBOX\" );\
        foreach $msg ( 1..$number_of_messages ) {\
        $server->delete_message( $msg ) || die (\"Could not delete msg $msg\n\" ); }\
        $server->expunge( \"INBOX\" ) || die (\"Could not expunge INBOX\n\" );\
        $server->logout();"
        ], "" );

        return reply;
    },

/*
    Return number of new messages in the Inbox on the given IMAP server,
    connecting via port, with the given username and password.
*/
    getNewMessageCount: function(server, username, password, port)
    {
        var reply = runProcess( "perl", [ "-e",
        "use Mail::IMAPClient;\
        $server = Mail::IMAPClient->new(\
        Server => \"" + server + "\",\
        User=> \"" + username + "\",\
        Password => \"" + password + "\",\
        Port => \"" + port + "\",\
        Uid => \"0\",\
        Peek => \"1\" ) || die (\"Could not connect to " + server + ":" + port + " with " + username + ": $! $?\\n\");\
        $server->select( \"INBOX\" ) || die (\"Could not select INBOX\\n\");\
        print $server->message_count(\"INBOX\" );\
        print \"\\n\";\
        $server->logout();"
        ], "" );

        return reply;
    },

/*
    Verify that the required Perl modules are installed. Returns
    true if the modules are installed, false if they are not.
*/
    verifySetup: function()
    {
        var reply = runProcess( "perl", [ "-e",
        "eval \"use Mail::IMAPClient; use MIME::Base64; use MIME::QuotedPrint; use Mail::Sendmail;\";\
        print $@;"
        ], "" );

        return (reply == "");
    }
}
