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

#include "messageboard.h"

struct InfoData {
    int id;
    int priority;
    QString text;
    QString pixmap;

    bool operator<=(const InfoData &d) const {
        return priority <= d.priority;
    }
    bool operator<(const InfoData &d) const {
        return priority < d.priority;
    }
    bool operator>(const InfoData &d) const {
        return priority > d.priority;
    }
};

class MessageBoardPrivate
{
public:
    QList<InfoData> messages; 
    QSet<int> usedIds;
};

/*!
  \class MessageBoard
    \inpublicgroup QtUiModule
    \inpublicgroup QtTelephonyModule
  \brief The MessageBoard class provides a message hub for user information.
  \ingroup QtopiaServer::Task

  The class stores information to be shown to the user. Messages on the board are sorted 
  according to their priority (0 being the highest priority). New messages can be added 
  to the board via postMessage() and the message with the highest priority can be retrieved 
  via message(). 

  The MessageBoard uses qStableSort to sort messages according to their id. This means that 
  new messages with the same priority as an already existing message will appear after the old 
  message.

  This class is a Qt Extended server task. It is part of the Qt Extended server and cannot be used by 
  other Qt Extended applications.

  \sa QAbstractHomeScreen, MessageBoard::Note
*/

/*!
    \class MessageBoard::Note
    \inpublicgroup QtUiModule
    \inpublicgroup QtTelephonyModule
    \brief The Note class provides a simple container for messages.

    This structure has the following fields:

    \list
        \o \c {QString text} - The \c text field contains the text to be displayed to the user.
        \o \c {QString pixmap}  - The \c pixmap field contains the path (e.g. \c ":image/ok") to the picture associated with the text.
        \o \c {int id} - The \c id field contains the unique identifier for this note on the MessageBoard. It can be a number between 0 and 65536.
    \endlist

    \sa MessageBoard
*/

/*!
  Creates a new MessageBoard instance with the given \a parent.
  */
MessageBoard::MessageBoard( QObject *parent )
    : QObject( parent )
{
    d = new MessageBoardPrivate();
}

/*!
    Destroys the message boad instance.
*/

MessageBoard::~MessageBoard()
{
    delete d;
}

/*!
  Adds a new message with the given \a text, \a pixmap and \a priority. This function
  returns -1 if the new message could not be added.
  */
int MessageBoard::postMessage( const QString& pixmap, const QString& text, int priority )
{
    static int nextInfoId = 0;

    //make sure we don't reuse already used id
    InfoData info;
    int i;
    nextInfoId = (( nextInfoId+1 ) % 65536); 
    for ( i = 0; (d->usedIds.contains(nextInfoId) && i<65536); i++) 
        nextInfoId = (( nextInfoId+1 ) % 65536); 
    if ( i < 65536 ) {
        d->usedIds.insert( nextInfoId );
    } else {
        qWarning() << "MessageBoard full. Cannot add more messages";
        return -1;
    }

    info.id = nextInfoId;
    info.priority = priority;
    info.text = text;
    info.pixmap = pixmap;

    d->messages.append(info);
    qStableSort(d->messages);

    emit boardUpdated();
    return info.id;
}

/*!
  Removes the message with the given \a identifier from the message board.
  */
void MessageBoard::clearMessage( int identifier )
{
    QList<InfoData>::Iterator it;
    for (it = d->messages.begin(); it != d->messages.end(); ++it) {
        if ((*it).id == identifier) {
            d->usedIds.remove(identifier);
            d->messages.erase(it);
            emit boardUpdated();
            break;
        }
    }
}

/*!
  \fn void MessageBoard::boardUpdated() 

  Emitted when a new message is either added or removed from the board.
  */


/*!
  This function returns the message with the highest priority.
  */
MessageBoard::Note MessageBoard::message() const
{
    MessageBoard::Note note;
    if ( d->messages.count() > 0 ) {
        note.text = d->messages[0].text;
        note.pixmap = d->messages[0].pixmap;
        note.id = d->messages[0].id;
    }

    return note;
}

/*!
  Returns the message with the given \a id. If no message with the given
  \a id exists the returned note contains an empty string.
  */

MessageBoard::Note MessageBoard::message( int id ) const
{
    Note note;
    QList<InfoData>::Iterator it;
    for (it = d->messages.begin(); it != d->messages.end(); ++it) {
        if ((*it).id == id) {
            note.text = (*it).text;
            note.pixmap = (*it).pixmap;
            note.id = id;
            break;
        }
    }

    return note;
}

/*!
  Returns \c true if there are no messages on the board.
  */
bool MessageBoard::isEmpty() const
{
    return d->messages.isEmpty();
}

QTOPIA_TASK(MessageBoard, MessageBoard);
QTOPIA_TASK_PROVIDES(MessageBoard, MessageBoard);
