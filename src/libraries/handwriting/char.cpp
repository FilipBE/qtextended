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

#include <qfile.h>
#include <math.h>
#include <limits.h>
#include <errno.h>
#include <qdatastream.h>
#include <qtopiaapplication.h>
#include "combining_p.h"
#include "char.h"

#define QIMPEN_MATCH_THRESHOLD      200000

// should be internal, with class member access
struct QIMPenSpecialKeys {
    uint code;
    const char *name;
    uint q23code;
};

// fortunately there is as yet no collision between old Qt 2.3 key codes, and Qt 4.x key codes.
const QIMPenSpecialKeys qimpen_specialKeys[] = {
    { Qt::Key_Escape,           QT_TRANSLATE_NOOP("Handwriting","[Esc]"), 0x1000 },
    { Qt::Key_Tab,              QT_TRANSLATE_NOOP("Handwriting","[Tab]"), 0x1001 },
    { Qt::Key_Backspace,        QT_TRANSLATE_NOOP("Handwriting","[BackSpace]"), 0x1003 },
    { Qt::Key_Return,           QT_TRANSLATE_NOOP("Handwriting","[Return]"), 0x1004 },
    { QIMPenChar::Caps,         QT_TRANSLATE_NOOP("Handwriting","[Uppercase]"), 0x4001 },
    { QIMPenChar::CapsLock,     QT_TRANSLATE_NOOP("Handwriting","[Caps Lock]"), 0x4003 },
    { QIMPenChar::Shortcut,     QT_TRANSLATE_NOOP("Handwriting","[Shortcut]"), 0x4002 },
    { QIMPenChar::Punctuation,  QT_TRANSLATE_NOOP("Handwriting","[Punctuation]"), 0x4004 },
    { QIMPenChar::Symbol,       QT_TRANSLATE_NOOP("Handwriting","[Symbol]"), 0x4005 },
    { QIMPenChar::NextWord,     QT_TRANSLATE_NOOP("Handwriting","[Next Word]"), 0x4007 },
    { QIMPenChar::WordPopup,    QT_TRANSLATE_NOOP("Handwriting","[Word Menu]"), 0x4008 }, // word popup
    { QIMPenChar::SymbolPopup,  QT_TRANSLATE_NOOP("Handwriting","[Symbol Menu]"), 0x4009 },
    { QIMPenChar::ModePopup,    QT_TRANSLATE_NOOP("Handwriting","[Mode Menu]"), 0x400A },
    { Qt::Key_unknown,          0, 0 } };

/*!
  \class QIMPenChar
    \inpublicgroup QtInputMethodsModule
  \brief The QIMPenChar class handles a single character for the stroke recognition libraries.

  \preliminary

  Each QIMPenChar associates a unicode character with a series of strokes.

  QIMPenChar can calculate closeness of match to another character.

  \ingroup userinput
*/

/*!
    \enum QIMPenChar::Mode

    This enum is used to refer to several special characters used by Qtopia.  It includes modifiers like the shift and control keys, as well as system spefic values used to indicate actions to the system like SymbolPopup.

    \value ModeBase  Base value for all Modes
    \value Caps  Shift
    \value Shortcut  Used by the system
    \value CapsLock  Caps lock
    \value Punctuation  Used by the system
    \value Symbol  Used by the system
    \value NextWord  Used by the system
    \value WordPopup  Used by the system
    \value SymbolPopup  Used by the system
    \value ModePopup  Used by the system

    \table
    \header \o enum \o Value
    \row \o    ModeBase \o 0x4000
    \row \o    Caps \o 0x4001
    \row \o    Shortcut \o 0x4002
    \row \o    CapsLock \o 0x4003
    \row \o    Punctuation \o 0x4004
    \row \o    Symbol \o 0x4005
    \row \o    NextWord \o 0x4007
    \row \o    WordPopup \o 0x4008
    \row \o    SymbolPopup \o 0x4009
    \row \o    ModePopup \o 0x400A
    \endtable

*/

/*!
    \fn void QIMPenChar::setFlag( int f )
    Set flag \a f for this character.
*/

/*!

    \fn void QIMPenChar::clearFlag( int f )
    Clear flag \a f for this character.
*/

/*!
    \fn bool QIMPenChar::testFlag( int f )
    Return true if flag \a f is set for this character, otherwise return false.
*/

/*!
    \enum QIMPenChar::Flags

    This enum specifies how split() should behave with respect to empty strings.

    \value System  The character is a system character, and should be turned off rather than deleted when disabled
    \value Deleted  The character is a system character that has been disabled.
    \value CombineRight  The character is aligned to the right when combining characters to create some interantional characters like áâä, etc..
    \value Data  Kept in order to read old files

*/

/*!
    \fn uint QIMPenChar::key() const
    Return the key for this character.
    \sa setKey(), repCharacter()
*/

/*!
    \fn const QIMPenStrokeList &QIMPenChar::penStrokes() const
    Returns the list of penStrokes for this character.
*/

/*!
    \fn QChar QIMPenChar::repCharacter() const
    Returns the QChar this character represents.
    \sa key(), setRepCharacter()
*/

/*!
    \fn QPoint QIMPenChar::startingPoint() const
    Returns the starting point of the first stroke of this character.
    \sa strokeLength(), addStroke(), strokeCount(), isEmpty()
*/

/*!
    \enum QIMPenCharSet::Type

    This enum describes the types of characters contained in a QIMPenCharSet

    \value Unknown
    \value Lower  Lower case letters
    \value Upper  Upper case letters
    \value Combining  Letters formed by combining two parts (characters with umlauts, accents etc..)
    \value Numeric
    \value Punctuation
    \value Symbol
    \value Shortcut

    \sa setType(), type()
*/

/*!
    \fn uint QIMPenChar::strokeCount() const
    Returns the number of strokes in this character.
*/

/*!
    \fn bool QIMPenChar::isEmpty() const
    Return true if this character has no strokes, otherwise return false.
*/

/*!
    Constructs an empty QIMPenChar.
*/
QIMPenChar::QIMPenChar()
{
    flags = 0;
}
/*!
    Constructs a copy of the given \a character.
*/
QIMPenChar::QIMPenChar( const QIMPenChar &character )
{
    mUnicode = character.mUnicode;
    mKey = character.mKey;
    flags = character.flags;
    QIMPenStroke *s = 0;
    foreach(QIMPenStroke *it, character.strokes) {
        s = new QIMPenStroke( *it );
        Q_CHECK_PTR( s );
        strokes.append( s );
    }
}

/*!
    Destroys this character and cleans up.
*/
QIMPenChar::~QIMPenChar()
{
    // "autodelete"
    while ( strokes.count() )
        delete strokes.takeLast();
}

/*!
    Assigns a copy of the given \a character to this character and returns a refernece to this character.
*/
QIMPenChar &QIMPenChar::operator=( const QIMPenChar &character )
{
    while ( strokes.count() )
        delete strokes.takeLast();
    mUnicode = character.mUnicode;
    mKey = character.mKey;
    flags = character.flags;
    QIMPenStrokeConstIterator it = character.strokes.constBegin();
    QIMPenStroke *s = 0;
    while ( it != character.strokes.constEnd() ) {
        s = new QIMPenStroke( **it );
        Q_CHECK_PTR( s );
        strokes.append( s );
        ++it;
    }

    return *this;
}

/*!
    Returns the name of this character.  This is usually the unicode of the character.  However, the following special keys have the following names:
\table
\header \o Key Code \o Name
\row \o Qt::Key_Escape \o "[Esc]"
\row \o Qt::Key_Tab \o "[Tab]"
\row \o Qt::Key_Backspace \o "[BackSpace]"
\row \o Qt::Key_Return \o "[Return]"
\row \o QIMPenChar::Caps \o "[Uppercase]"
\row \o QIMPenChar::CapsLock \o "[Caps Lock]"
\row \o QIMPenChar::Shortcut \o "[Shortcut]"
\row \o QIMPenChar::Punctuation \o "[Punctuation]"
\row \o QIMPenChar::Symbol \o "[Symbol]"
\row \o QIMPenChar::NextWord \o "[Next Word]"
\row \o QIMPenChar::WordPopup \o "[Word Menu]"
\row \o QIMPenChar::SymbolPopup \o "[Symbol Menu]"
\row \o QIMPenChar::ModePopup \o "[Mode Menu]"
\endtable
*/
QString QIMPenChar::name() const
{
    if (mKey) {
        QString n;
        for ( int i = 0; qimpen_specialKeys[i].code != Qt::Key_unknown; i++ ) {
            if ( qimpen_specialKeys[i].code == mKey ) {
                n = qApp->translate("Handwriting", qimpen_specialKeys[i].name);
                return n;
            }
        }
    }
    return QString(mUnicode);
}

/*!
    Clears the data of this character.
*/
void QIMPenChar::clear()
{
    mUnicode = 0;
    mKey = 0;
    flags = 0;
    while ( strokes.count() )
        delete strokes.takeLast();
}

/*!
    Returns the length of stroke \a n.  Returns 0 if this character has less then \a n strokes.
*/
unsigned int QIMPenChar::strokeLength( int n ) const
{
    return strokes.count() > n ? strokes[n]->length() : 0;
    /*
    QIMPenStrokeIterator it( strokes );
    while ( it.current() && n ) {
        ++it;
        --n;
    }

    if ( it.current() )
        return it.current()->length();

    return 0;
    */
}

/*!
  Adds \a stroke to the character
*/
void QIMPenChar::addStroke( QIMPenStroke *stroke )
{
    QIMPenStroke *newStroke = new QIMPenStroke( *stroke );
    strokes.append( newStroke );
}

/*!
  Return an indicator of the closeness of this character to \a pen.
  Lower value is better.
*/
int QIMPenChar::match( QIMPenChar *pen )
{
/*
    if ( strokes.count() > pen->strokes.count() )
        return INT_MAX;
*/
    int err = 0;
    int maxErr = 0;
    int diff = 0;
    QIMPenStrokeIterator it1 = strokes.begin();
    QIMPenStrokeIterator it2 = pen->strokes.begin();
    err = (*it1)->match( *it2 );
    if ( err > maxErr )
        maxErr = err;
    ++it1;
    ++it2;
    /* currently the multi-stroke gravity checks are not
       canvas height dependent, they should be */
    while ( err < 400000
            && it1 != strokes.end()
            && it2 != pen->strokes.end() )
    {
        // the difference between the center of this stroke
        // and the center of the first stroke.
        QPoint p1 = (*it1)->boundingRect().center() -
                    strokes[0]->boundingRect().center();

        // scale to canvas height
        p1 = p1 * 75 / (*it1)->canvasHeight();

        // the difference between the center of this stroke
        // and the center of the first stroke.
        QPoint p2 = (*it2)->boundingRect().center() -
                    pen->strokes[0]->boundingRect().center();

        // scale to canvas height
        p1 = p1 * 75 / (*it1)->canvasHeight();

        int xdiff = qAbs( p1.x() - p2.x() ) - 6;
        int ydiff = qAbs( p1.y() - p2.y() ) - 5;
        if ( xdiff < 0 )
            xdiff = 0;
        if ( ydiff < 0 )
            ydiff = 0;
        if ( xdiff > 10 || ydiff > 10 ) { // not a chance
            return INT_MAX;
        }
        diff += xdiff*xdiff + ydiff*ydiff;
        err = (*it1)->match( *it2 );
        if ( err > maxErr )
            maxErr = err;
        ++it1;
        ++it2;
    }

    maxErr += diff * diff * 6; // magic weighting :)

    return maxErr;
}

/*!
  Return the bounding rect of this character.  It may have sides with
  negative coords since its origin is where the user started drawing
  the character.
*/
QRect QIMPenChar::boundingRect()
{
    QRect br;
    QIMPenStrokeConstIterator st = strokes.constBegin();
    while ( st != strokes.constEnd() ) {
        br |= (*st)->boundingRect();
        st++;
    }

    return br;
}

/*!
  If \a code is in the set of qimpen_specialKeys, sets the key code for the character
  to \a code and clear the unicode value.  Otherwise does nothing.
*/
void QIMPenChar::setKey(uint code)
{
    const QIMPenSpecialKeys *k = qimpen_specialKeys;
    while(k->code != Qt::Key_unknown) {
        if (code == k->code)
            break;
        ++k;
    }

    if (k->code == Qt::Key_unknown)
        return;

    switch(code) {
        default:
            mUnicode = 0;
            break;
        case Qt::Key_Tab:
            mUnicode = 9;
            break;
        case Qt::Key_Return:
            mUnicode = 13;
            break;
        case Qt::Key_Backspace:
            mUnicode = 8;
            break;
        case Qt::Key_Escape:
            mUnicode = 27;
            break;
    }
    mKey = code;
}

/*!
  Sets the unicode for the character to \a code and clears the key value.
*/
void QIMPenChar::setRepCharacter(QChar code)
{
    mUnicode = code;
    uint scan_uni = mUnicode.unicode();

    if ( scan_uni >= 'a' && scan_uni <= 'z' ) {
        mKey = Qt::Key_A + scan_uni - 'a';
    } else if ( scan_uni >= 'A' && scan_uni <= 'Z' ) {
        mKey = Qt::Key_A + scan_uni - 'A';
    } else if ( scan_uni == ' ' ) {
        mKey = Qt::Key_Space;
    } else {
        mKey = Qt::Key_unknown;
    }
}


/*!
    This is an overloaded member function, provided for convenience.

    Writes the given character \a ws to \a stream.
*/
QDataStream &operator<< (QDataStream &stream, const QIMPenChar &ws)
{
    /* handle 2.3 legacy of only 16bit keys instead of 28.
       convert both times as needs to work with peoples old files.
       Only needs to handle keys in qimpen_specialKeys */
    uint ch = ws.mUnicode.unicode();
    const QIMPenSpecialKeys *k = qimpen_specialKeys;
    while(k->code != Qt::Key_unknown) {
        if (ws.mKey == k->code) {
            ch = k->q23code << 16;
            break;
        }
        k++;
    }
    stream << ch;

    // never write data, its old hat.
    if ( ws.flags & QIMPenChar::Data )
        stream << (ws.flags ^ QIMPenChar::Data);
    else
        stream << ws.flags;
    stream << ws.strokes.count();
    QIMPenStrokeConstIterator it = ws.strokes.constBegin();
    while ( it != ws.strokes.constEnd() ) {
        stream << **it;
        ++it;
    }

    return stream;
}

/*!
    This is an overloaded member function, provided for convenience.

    Read a character from \a stream and store in \a ws.
*/
QDataStream &operator>> (QDataStream &stream, QIMPenChar &ws)
{
    /* handle 2.3 legacy of only 16bit keys instead of 28.
       convert both times as needs to work with peoples old files.
       Only needs to handle keys in qimpen_specialKeys */
    uint ch;
    stream >> ch;
    ws.setRepCharacter(ch & 0x0000ffff);
    if (ch & 0xffff0000) {
        // is special key
        ch = ch >> 16;
        const QIMPenSpecialKeys *k = qimpen_specialKeys;
        while(k->code != Qt::Key_unknown) {
            if (ch == k->q23code) {
                ws.setKey(k->code);
                break;
            }
            k++;
        }
    }

    stream >> ws.flags;
    if ( ws.flags & QIMPenChar::Data ) {
        QString d;
        stream >> d;
        // then throw away.
    }
    unsigned size;
    stream >> size;
    for ( unsigned i = 0; i < size; i++ ) {
        QIMPenStroke *st = new QIMPenStroke();
        stream >> *st;
        ws.strokes.append( st );
    }

    return stream;
}

//===========================================================================

/*!
    Returns true if \a m has a lower error than this (i.e. if m is a better match).  Returns false otherwise.
*/
bool QIMPenCharMatch::operator>( const QIMPenCharMatch &m ) const
{
    return error > m.error;
}

/*!
    Returns true if \a m has a higher error than this (i.e. if m is a worse match).  Returns false otherwise.
*/
bool QIMPenCharMatch::operator<( const QIMPenCharMatch &m ) const
{
    return error < m.error;
}

/*!
    Returns true if \a m has a lower or equal error than this (i.e. if m is not a worse match).  Returns false otherwise.
*/
bool QIMPenCharMatch::operator<=( const QIMPenCharMatch &m ) const
{
    return error <= m.error;
}

//===========================================================================

/*!
  \class QIMPenCharSet
    \inpublicgroup QtInputMethodsModule

  \preliminary
  \brief The QIMPenCharSet class maintains a set of related characters.
  \ingroup userinput
*/

/*!
    Construsts a default QIMPenCharSet() with title "abc", type Unknown, and a 0 stroke maximum.
*/
QIMPenCharSet::QIMPenCharSet()
{
    desc = qApp->translate("Handwriting","Unnamed","Character set name");
    csTitle = "abc";
    csType = Unknown;
    maxStrokes = 0;
}

/*!
  Construct and load a characters set from file \a fn.
*/
QIMPenCharSet::QIMPenCharSet( const QString &fn )
{
    desc = qApp->translate("Handwriting","Unnamed","Character set name");
    csTitle = "abc"; // No tr;
    csType = Unknown;
    maxStrokes = 0;
    load( fn );
}

/*!
    Delete this character set and clean up.
*/
QIMPenCharSet::~QIMPenCharSet()
{
    // autodelete
    while ( chars.count() )
        delete chars.takeLast();
}

/*!
    \fn bool QIMPenCharSet::isEmpty() const
    Returns true if this QIMPenCharSet contains no QIMPenChar, otherwise returns false.
*/

/*!
    Clear all data.
*/
void QIMPenCharSet::clear()
{
    while(chars.count())
        delete chars.takeLast();
}

/*!
    \fn const QIMPenCharList &QIMPenCharSet::characters() const
    Returns the list of characters in this QIMPenCharSet
*/

/*!
    \fn uint QIMPenCharSet::count() const
    Returns the number of QIMPenChars in this QIMPenCharSet
*/

/*!
    Sets the human readable \a description of this QIMPenCharSet
*/
void QIMPenCharSet::setDescription( const QString &description )
{
    if (description != desc) {
        desc = description;
    }
}

/*!
    \fn QString QIMPenCharSet::description() const
    Returns the human readable description of this QIMPenCharSet
*/

/*!
    Sets the \a title of this character set.
    \bold{Note}: in early versions this dictates the type as well
    \sa setType()
*/
void QIMPenCharSet::setTitle( const QString &title )
{
    if (title != csTitle) {
        csTitle = title;
    }
}

/*!
    \fn QString QIMPenCharSet::title() const
    Returns the title of this QIMPenCharSet
    \sa setTitle()
*/

/*!
    Sets the \a type of this character set (eg "abc", "ABC", "123" or "Combining");
    \sa setTitle()
*/
void QIMPenCharSet::setType( Type type )
{
    if (type != csType) {
        csType = type;
    }
}

/*!
    \fn Type QIMPenCharSet::type() const
    returns the Type for this QIMPenCharSet
    \sa Type, setType()
*/

/*!
    Returns the filename used to save this character set.
    \sa setFilename()
*/
const QString &QIMPenCharSet::filename( ) const
{
    return userFilename;
}

/*!
    Sets the filename \a fn to be used to save this character set.
    \sa filename()
*/
void QIMPenCharSet::setFilename( const QString &fn )
{
    if (fn != userFilename) {
        userFilename = fn;
    }
}

/*!
    Returns the complete path to the system file that will be loaded by load(), including the current filename.
    \sa userPath(), load(), filename()
*/
QString QIMPenCharSet::systemPath( ) const
{
    static const QString sysPath(Qtopia::qtopiaDir() + "etc/qimpen/"); // no tr

    return sysPath + userFilename;
}

/*!
    Returns the complete path to the user file that will be loaded by load(), including the current filename.
    \sa systemPath(), load(), filename()
*/
QString QIMPenCharSet::userPath() const
{
    return Qtopia::applicationFileName("qimpen",userFilename); // no tr
}

/*!
  Load a character set from file \a fn, checking both system and user directories.  Returns true if at least one file is opened with no errors.  If there characters in both the user and system files, entries in the user file override those in the system file.
*/
bool QIMPenCharSet::load( const QString &fn )
{
    clear();
    if (!fn.isEmpty())
        setFilename( fn );

    bool ok = false;
    for (int isUser = 0; isUser < 2; ++isUser) {
        QString path;

        if (isUser == 1)
            path = userPath();
        else
            path = systemPath();

        QFile file( path );
        if ( file.open( QIODevice::ReadOnly ) ) {
            QDataStream ds( &file );
            ds.setVersion(QDataStream::Qt_2_1);
            QString version;
            ds >> version;
            ds >> csTitle;
            ds >> desc;
            int major = version.mid( 4, 1 ).toInt();
            int minor = version.mid( 6 ).toInt();
            if ( major >= 1 && minor > 0 ) {
                ds >> (qint8 &)csType;
            } else {
                if ( csTitle == "abc" ) // no tr
                    csType = Lower;
                else if ( csTitle == "ABC" ) // no tr
                    csType = Upper;
                else if ( csTitle == "123" ) // no tr
                    csType = Numeric;
                else if ( fn == "Combining" ) // No tr
                    csType = Combining;
            }
            while ( !ds.atEnd() && file.error() == QFile::NoError ) {
                QIMPenChar *pc = new QIMPenChar;
                ds >> *pc;
                if ( isUser == 1 )
                    markDeleted( *pc ); // override system
                addChar( pc );
            }
            if ( file.error() == QFile::NoError )
                ok = true;
        }
    }

    return ok;
}

/*!
  Save this character set.  Returns true if the file was saved successfully, and false if there were any errors encountered while saving.
*/
bool QIMPenCharSet::save( ) const
{
    bool ok = false;

    // in 4.0 format, store keys as strings and use Global::stringToKey or equiv.
    QString fn = userPath();
    QString tmpFn = fn + ".new"; // no tr
    QFile file( tmpFn );
    if ( file.open( QIODevice::WriteOnly|QIODevice::Unbuffered ) ) {
        QDataStream ds( &file );
        ds.setVersion(QDataStream::Qt_2_1);
        ds << QString( "QPT 1.1" ); // no tr
        ds << csTitle;
        ds << desc;
        ds << (qint8)csType;
        QIMPenCharIterator ci = chars.constBegin();
        for ( ; ci != chars.constEnd(); ++ci ) {
            QIMPenChar *pc = *ci;
            // only save user char's, not system char's.
            if ( !pc->testFlag( QIMPenChar::System ) ) {
                ds << *pc;
            }
            if ( file.error() != QFile::NoError )
                break;
        }
        if ( file.error() == QFile::NoError )
            ok = true;
    }

    if ( ok ) {
        if ( ::rename( tmpFn.toLatin1(), fn.toLatin1() ) < 0 ) {
            qWarning( "problem renaming file %s to %s, errno: %d",
                    (const char *)tmpFn.toLatin1(), (const char *)fn.toLatin1(), errno );
            // remove the tmp file, otherwise, it will just lay around...
            QFile::remove( tmpFn.toLatin1() );
            ok = false;
        }
    }

    return ok;
}

/*!
    Returns the item at index position \a i in the list. \a i must be a valid index position in the list.
*/
QIMPenChar *QIMPenCharSet::at( int i )
{
    return chars.at(i);
}

/*!
    \fn unsigned QIMPenCharSet::maximumStrokes() const
    Returns the highest number of strokes any character in this QIMPenCharSet has had.
    \bold{Note:} This is guaranteed to be at least as high as the current maximum number of strokes, but can be higher if the previous highest stroke character is deleted.
*/

/*!
    Marks all system QIMPenChar that match \a ch as deleted.  Has no effect on user-created QIMPenChar.
*/
void QIMPenCharSet::markDeleted( const QIMPenChar &ch )
{
    QIMPenCharIterator ci = chars.constBegin();
    for ( ; ci != chars.constEnd(); ++ci ) {
        QIMPenChar *pc = *ci;
        if ( pc->key() == ch.key() && pc->repCharacter() == ch.repCharacter()
                && pc->testFlag( QIMPenChar::System ) )
            pc->setFlag( QIMPenChar::Deleted );
    }
}

/*!
  Find the best matches for \a ch in this character set.
*/
QIMPenCharMatchList QIMPenCharSet::match( QIMPenChar *ch )
{
    QIMPenCharMatchList matches;

    QIMPenCharIterator ci = chars.begin();
    // for each character in set.
    for ( ; ci != chars.end(); ++ci ) {
        QIMPenChar *tmplChar = *ci;
        if ( tmplChar->testFlag( QIMPenChar::Deleted ) ) {
            continue;
        }
        int err;
        // if the stroke to match against has equal or less strokes
        if ( ch->penStrokes().count() <= tmplChar->penStrokes().count() ) {
            // check how well it matches,
            err = ch->match( tmplChar );
            // and if it is less than the threshold
            if ( err <= QIMPEN_MATCH_THRESHOLD ) {
                // compare stroke count again
                if (tmplChar->penStrokes().count() != ch->penStrokes().count())
                    err = qMin(err*3, QIMPEN_MATCH_THRESHOLD);
                QIMPenCharMatchIterator it;
                // Correct the error if an existing char match
                // has greater error (e.g. two ways of writing 'a')
                for ( it = matches.begin(); it != matches.end(); ++it ) {
                    if ( it->penChar->repCharacter() == tmplChar->repCharacter() &&
                            it->penChar->key() == tmplChar->key() &&
                         it->penChar->penStrokes().count() == tmplChar->penStrokes().count() ) {
                        if ( it->error > err )
                            it->error = err;
                        break;
                    }
                }
                // if this char isn't already a match
                if ( it == matches.end() ) {
                    // add it as a match
                    QIMPenCharMatch m;
                    m.error = err;
                    m.penChar = tmplChar;
                    matches.append( m );
                }
            }
        }
    }
    // sort and return.
    qStableSort( matches );
    return matches;
}

/*!
  Add a character \a ch to this set.
  QIMPenCharSet will delete this character when it is no longer needed.
*/
void QIMPenCharSet::addChar( QIMPenChar *ch )
{
    if ( (uint)(ch->penStrokes().count()) > maxStrokes )
        maxStrokes = ch->penStrokes().count();
    chars.append( ch );
}

/*!
  Remove a character by reference \a ch from this set.
  QIMPenCharSet will delete this character.
*/
void QIMPenCharSet::removeChar( QIMPenChar *ch )
{
    delete chars.takeAt( chars.indexOf( ch ));
}

/*!
  Move the character \a ch up the list of QIMPenChar.
*/
void QIMPenCharSet::up( QIMPenChar *ch )
{
    int idx = chars.indexOf( ch );
    if ( idx >= 0 ) {
        if ( idx + 1 < chars.count() )
            chars.swap( idx, idx + 1 );
    }
}

/*!
  Move the character \a ch down the list of QIMPenChar.
*/
void QIMPenCharSet::down( QIMPenChar *ch )
{
    int idx = chars.indexOf( ch );
    if ( idx > 0 ) {
        chars.swap( idx, idx - 1 );
    }
}

