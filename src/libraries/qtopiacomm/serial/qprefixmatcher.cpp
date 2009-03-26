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

#include "qprefixmatcher_p.h"
#include <qtopialog.h>

class QPrefixMatcherTarget
{
public:
    QObject *target;
    int index;
    QPrefixMatcherTarget *next;
};

class QPrefixMatcherNode
{
public:
    QPrefixMatcherNode()
    {
        ch = 0;
        marker = 0;
        mayBeCommand = 0;
        type = QPrefixMatcher::Unknown;
        next = 0;
        children = 0;
        targets = 0;
    }
    ~QPrefixMatcherNode();

    ushort  ch;         // Character at this point in the prefix match.
    short   marker : 1; // Non-zero if this is a complete prefix match.
    short   mayBeCommand : 1;   // Non-zero if prefix that may be a command.
    QPrefixMatcher::Type type;  // Type associated with this node.
    QPrefixMatcherNode *next;       // Next character at this level.
    QPrefixMatcherNode *children;   // Children of this level.
    QPrefixMatcherTarget *targets;  // Information about targets for this match.

    void remove( QObject *target );
};

QPrefixMatcherNode::~QPrefixMatcherNode()
{
    // Delete the children associated with this node.
    QPrefixMatcherNode *c = children;
    QPrefixMatcherNode *cn;
    while ( c != 0 ) {
        cn = c->next;
        delete c;
        c = cn;
    }

    // Delete the targets associated with this node.
    QPrefixMatcherTarget *t = targets;
    QPrefixMatcherTarget *tn;
    while ( t != 0 ) {
        tn = t->next;
        delete t;
        t = tn;
    }
}

void QPrefixMatcherNode::remove( QObject *target )
{
    // Remove the target from child nodes.
    QPrefixMatcherNode *c = children;
    while ( c != 0 ) {
        c->remove( target );
        c = c->next;
    }

    // Remove the target from this node.
    QPrefixMatcherTarget *t = targets;
    while ( t != 0 ) {
        if ( t->target == target )
            t->target = 0;
        t = t->next;
    }
}

QPrefixMatcher::QPrefixMatcher( QObject *parent )
    : QObject( parent )
{
    root = new QPrefixMatcherNode();
}

QPrefixMatcher::~QPrefixMatcher()
{
    delete root;
}

// Add a prefix to this matcher, together with a positive type code.
// Optionally, an object target and slot can be supplied.  When the
// prefix is matched by "lookup", it will automatically dispatch
// matching values to the indicated slot.  The slot should have the
// prototype "name(const QString&)".
void QPrefixMatcher::add( const QString& prefix, QPrefixMatcher::Type type,
                          bool mayBeCommand, QObject *target, const char *slot )
{
    // The prefix needs at least one character.
    if ( prefix.isEmpty() )
        return;

    // Traverse the tree to find the insert position.
    QPrefixMatcherNode *node;
    QPrefixMatcherNode **reference;
    node = root->children;
    reference = &(root->children);
    for ( int posn = 0; posn < prefix.length(); ++posn ) {
        ushort ch = prefix[posn].unicode();
        while ( node != 0 && node->ch != ch ) {
            reference = &(node->next);
            node = node->next;
        }
        if ( !node ) {
            node = new QPrefixMatcherNode();
            if ( !node )
                return;     // Just in case.
            node->ch = ch;
            *reference = node;
        }
        if ( ( posn + 1 ) < prefix.length() ) {
            reference = &(node->children);
            node = node->children;
        }
    }
    node->type = type;
    node->marker = 1;
    node->mayBeCommand = mayBeCommand;

    // Add the target slot information, if necessary.
    if ( target && slot ) {
        // Resolve the slot to an index that can be used with qt_metacall.
        if ( *slot >= '0' && *slot <= '9' )
            ++slot;
        QByteArray name = QMetaObject::normalizedSignature( slot );
        int index = target->metaObject()->indexOfMethod( name.constData() );
        if ( index == -1 ) {
            qLog(AtChat) << "QPrefixMatcher: "
                         << target->metaObject()->className()
                         << "::"
                         << name
                         << " is not an accessible slot";
            return;
        }

        // If we haven't seen this target before, then trap
        // its destroyed() signal so that we can stop sending
        // it signals when it disappears.
        if ( !targets.contains( target ) ) {
            targets.insert( target );
            connect( target, SIGNAL(destroyed()),
                     this, SLOT(targetDestroyed()) );
        }

        // If the target is already on this node, don't add it again.
        QPrefixMatcherTarget *t = node->targets;
        while ( t != 0 ) {
            if ( t->target == target && t->index == index )
                return;
            t = t->next;
        }

        // Add the target information to the current node.
        t = new QPrefixMatcherTarget();
        t->target = target;
        t->index = index;
        t->next = node->targets;
        node->targets = t;
    }
}

// Determine if "command" appears to start with "prefix".
static bool commandMatch( const QString& prefix, const QString& command )
{
    QString pref = "AT" + prefix;
    if ( pref.length() > 0 && pref[ pref.length() - 1 ] == QChar(':') ) {
        return command.startsWith( pref.left( pref.length() - 1 ) );
    } else {
        return command.startsWith( pref );
    }
}

// Look up the prefix of a value, returning its type code.
// If there is no match, return Unknown.  If the prefix has an
// associated slot, then activate the slot.  The "command"
// indicates the context of the lookup so that a prefix with the
// "mayBeCommand" flag set will not result in slot invocation.
QPrefixMatcher::Type QPrefixMatcher::lookup
        ( const QString& value, const QString& command ) const
{
    QPrefixMatcherNode *current = root->children;
    QPrefixMatcherNode *best = 0;
    int bestPosn = 0;
    int posn;
    ushort ch;

    // Bail out if the tree is currently empty.
    if ( !current )
        return QPrefixMatcher::Unknown;

    // Scan the string for a suitable prefix match.  We try to find
    // the longest such match so that "+CXYZ: W" will override "+CXYZ:".
    posn = 0;
    while ( posn < value.length() ) {
        ch = value[posn++].unicode();
        if ( ch >= 'a' && ch <= 'z' )
            ch = ch - 'a' + 'A';
        while ( current != 0 && current->ch != ch ) {
            current = current->next;
        }
        if ( !current ) {
            break;
        }
        if ( current->marker ) {
            best = current;
            bestPosn = posn;
        }
        current = current->children;
    }

    // Did we find something that matched?
    if ( best ) {
        if ( !best->mayBeCommand ||
             !commandMatch( value.left( bestPosn ), command ) ) {
            // Invoke the target slots associated with this match.
            QPrefixMatcherTarget *t = best->targets;
            void *a[2];
            while ( t != 0 ) {
                a[0] = (void *)0;
                a[1] = (void *)&value;
                if ( t->target ) {
                    t->target->qt_metacall( QMetaObject::InvokeMetaMethod,
                                            t->index, a );
                }
                t = t->next;
            }
            return best->type;
        } else if ( best->type != QPrefixMatcher::Notification ) {
            return best->type;
        }
    }

    // If we get here, we were unable to find a match.
    return QPrefixMatcher::Unknown;
}

void QPrefixMatcher::targetDestroyed()
{
    QObject *target = sender();
    if ( targets.contains( target ) ) {
        targets.remove( target );
        root->remove( target );
    }
}
