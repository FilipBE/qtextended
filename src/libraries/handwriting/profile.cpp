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

#include "combining_p.h"
#include "profile.h"

#include <QFileInfo>
#include <qtopiaapplication.h>
#include <qtranslatablesettings.h>

/*!
    \class QIMPenProfile
    \inpublicgroup QtInputMethodsModule
    \brief The QIMPenProfile class describes a group of character sets that
    form a profile.

    To improve the quality of character matching it is best to have a
    number of separate character sets that implement a selection of the
    total characters supported, for example it is common to have
    upper case, lower case, and numerals in separate sets and change
    mode to input characters in the different sets.

    \internal
*/

/*!
    Constructs a QIMPenProfile that loads the profile configuration
    from file named \a fn.
*/
QIMPenProfile::QIMPenProfile( const QString &fn )
    : filename( fn )
{
    load();
}

/*!
    Destroys a QIMPenProfile.
*/
QIMPenProfile::~QIMPenProfile()
{
    while ( sets.count() ) delete sets.takeLast();
}

void QIMPenProfile::load()
{
#ifndef QT_NO_TRANSLATION
    static int translation_installed = 0;
    if (!translation_installed) {
        QtopiaApplication::loadTranslations("libqmstroke");
        translation_installed++;
    }
#endif

    QTranslatableSettings config(filename, QSettings::IniFormat);
    config.beginGroup( "Handwriting" );

    pname = config.value( "Name" ).toString();
    pdesc = config.value( "Description" ).toString();

    tstyle = config.value( "CanSelectStyle", false ).toBool();
    istyle = config.value( "CanIgnoreStroke", false ).toBool();

    config.endGroup();

    config.beginGroup( "Settings" );

    pstyle = BothCases;
    QString s = config.value( "Style", "BothCases" ).toString();
    if ( s == "ToggleCases" )
        pstyle = ToggleCases;

    msTimeout = config.value( "MultiTimeout", 500 ).toInt();
    isTimeout = config.value( "IgnoreTimeout", 200 ).toInt();

    // Read user configuration
    QSettings usrConfig("Trolltech",userConfig());
    usrConfig.beginGroup( "Settings" );
    msTimeout = usrConfig.value( "MultiTimeout", msTimeout ).toInt();

    if ( tstyle && usrConfig.contains( "Style" ) ) {
        pstyle = BothCases;
        QString s = usrConfig.value( "Style", "BothCases" ).toString();
        if ( s == "ToggleCases" )
            pstyle = ToggleCases;
    }
    if ( istyle && usrConfig.contains( "IgnoreTimeout" ) ) {
        isTimeout = usrConfig.value( "IgnoreTimeout", isTimeout ).toInt();
    }
}

/*!
    Saves any changes to the profile and character sets it contains.

    Note that any changes to the character sets are stored separately
    to the system character sets.  This allows the user to revert
    back to a system character using the handwriting settings application.
*/
void QIMPenProfile::save() const
{
    QSettings usrConfig("Trolltech",userConfig());
    usrConfig.beginGroup("Settings");
    usrConfig.setValue("MultiTimeout", msTimeout);

    if (tstyle)
        usrConfig.setValue("Style", pstyle == BothCases ? "BothCases" : "ToggleCases" );
    if (istyle)
        usrConfig.setValue("IgnoreTimeout", isTimeout);

    // each set knows if its modified.
    if (!sets.isEmpty())
        saveData();
}

void QIMPenProfile::setStyle( Style s )
{
    if ( tstyle && s != pstyle ) {
        pstyle = s;
    }
}

/*!
    \fn int QIMPenProfile::multiStrokeTimeout() const

    Returns the time between two strokes that determines whether a new character
    has been started, or another stroke of a multi-stroke character has
    be started.

    \sa setMultiStrokeTimeout()
*/

/*!
    Sets the time between two strokes that determines whether a new character
    has been started, or another stroke of a multi-stroke character has 
    be started.

    \sa multiStrokeTimeout()
*/
void QIMPenProfile::setMultiStrokeTimeout( int t )
{
    if ( t != msTimeout ) {
        msTimeout = t;
    }
}

/*!
    \fn int QIMPenProfile::ignoreStrokeTimeout() const

    Returns the minimum time the stylus must be pressed for the input
    to be considered a stroke.

    \sa setIgnoreStrokeTimeout()
*/

/*!
    Sets the minimum time the stylus must be pressed for the input
    to be considered a stroke.  This is typically used to determine
    whether input is a stroke, or a click-through in a full-screen
    input method.

    \sa ignoreStrokeTimeout()
*/
void QIMPenProfile::setIgnoreStrokeTimeout( int t )
{
    if ( t != isTimeout ) {
        isTimeout = t;
    }
}

/*!
    Returns a unique identifier for this profile.
*/
const QString QIMPenProfile::identifier() const
{
    QFileInfo fi(filename);
    return fi.baseName();
}

QString QIMPenProfile::userConfig() const
{
    return "handwriting-" + identifier();
}

void QIMPenProfile::loadData()
{
    QSettings config(filename, QSettings::IniFormat);
    config.beginGroup( "CharSets" );

    // accents
    QIMPenCombining *combining = 0;
    QString s = config.value( "Combining" ).toString();
    if ( !s.isEmpty() ) {
        combining = new QIMPenCombining( s );
        if ( combining->isEmpty() ) {
            delete combining;
            combining = 0;
        }
    }
    // uppercase toLatin1
    QIMPenCharSet *cs = 0;
    ProfileSet *ps = 0;
    s = config.value( "Uppercase" ).toString();
    if ( !s.isEmpty() ) {
        cs = new QIMPenCharSet( s );
        if ( !cs->isEmpty() ) {
            if ( combining )
                combining->addCombined( cs );
            ps = new ProfileSet(cs);
            ps->id = "Uppercase"; // no tr
            sets.append( ps );
        } else {
            delete cs;
        }
    }
    // lowercase toLatin1
    s = config.value( "Lowercase" ).toString();
    if ( !s.isEmpty() ) {
        cs = new QIMPenCharSet( s );
        if ( !cs->isEmpty() ) {
            if ( combining )
                combining->addCombined( cs );
            ps = new ProfileSet(cs);
            ps->id = "Lowercase"; // no tr
            sets.append( ps );
        } else {
            delete cs;
        }
    }
    // numeric (may comtain punctuation and symbols)
    s = config.value( "Numeric" ).toString();
    if ( !s.isEmpty() ) {
        cs = new QIMPenCharSet( s );
        if ( !cs->isEmpty() ) {
            ps = new ProfileSet(cs);
            ps->id = "Numeric"; // no tr
            sets.append( ps );
        } else {
            delete cs;
        }
    }
    // punctuation
    s = config.value( "Punctuation" ).toString();
    if ( !s.isEmpty() ) {
        cs = new QIMPenCharSet( s );
        if ( !cs->isEmpty() ) {
            ps = new ProfileSet(cs);
            ps->id = "Punctuation"; // no tr
            sets.append( ps );
        } else {
            delete cs;
        }
    }
    // symbol
    s = config.value( "Symbol" ).toString();
    if ( !s.isEmpty() ) {
        cs = new QIMPenCharSet( s );
        if ( !cs->isEmpty() ) {
            ps = new ProfileSet(cs);
            ps->id = "Symbol"; // no tr
            sets.append( ps );
        } else {
            delete cs;
        }
    }
    // shortcut
    s = config.value( "Shortcut" ).toString();
    if ( !s.isEmpty() ) {
        cs = new QIMPenCharSet( s );
        if ( !cs->isEmpty() ) {
            ps = new ProfileSet(cs);
            ps->id = "Shortcut"; // no tr
            sets.append( ps );
        } else {
            delete cs;
        }
    }
    // now read sets that are specified in list variable.

    config.endGroup();
    config.beginGroup( "Handwriting" );
    QStringList customSets = config.value("customSets").toStringList();
    QStringList::Iterator it;
    for ( it = customSets.begin(); it != customSets.end(); ++it ) {
        QString id(*it);
        config.endGroup();
        config.beginGroup(id);
        cs = new QIMPenCharSet( config.value("Data").toString() );
        if ( !cs->isEmpty() ) {
            if ( combining &&
                (cs->type() & (QIMPenCharSet::Lower | QIMPenCharSet::Upper)) != 0)
                combining->addCombined( cs );
            ps = new ProfileSet(cs);
            ps->id = id;
            sets.append( ps );
        } else {
            delete cs;
        }
    }

    if ( combining )
        delete combining;
}

void QIMPenProfile::saveData() const
{
    ProfileSetListConstIterator it = sets.begin();
    for ( ; it != sets.end(); ++it )
        (*it)->set->save();
}

/*!
  returns the set for the set identified by \a id.
*/
QIMPenCharSet *QIMPenProfile::charSet( const QString &id )
{
    if ( sets.isEmpty() )
        loadData();
    ProfileSetListIterator it = sets.begin();
    for ( ; it != sets.end(); ++it ) {
        if ( (*it)->id == id)
            return (*it)->set;
    }
    return 0;
}

/*!
  Returns the first set in list that matches the type given
*/
QIMPenCharSet *QIMPenProfile::find( QIMPenCharSet::Type t )
{
    if ( sets.isEmpty() )
        loadData();
    ProfileSetListIterator it = sets.begin();
    for ( ; it != sets.end(); ++it ) {
        if ( (*it)->set->type() == t)
            return (*it)->set;
    }
    return 0;
}

/*!
  Returns the translated title for the set identified by \a id.
*/
QString QIMPenProfile::title( const QString &id )
{
    if ( sets.isEmpty() )
        loadData();
    QTranslatableSettings config(filename, QSettings::IniFormat);

    // Try from config file
    config.beginGroup( id );
    if (config.contains("Title"))
        return config.value("Title", id).toString();
    // Try from Profile Translations
    { config.endGroup(); config.beginGroup( "CharSets" ); };
    if (config.contains(id) && qApp)
        return qApp->translate("QIMPenProfile", id.toAscii());
    return id;
}

/*!
  Returns the translated description for the set identified by \a id.
*/
QString QIMPenProfile::description( const QString &id )
{
    if ( sets.isEmpty() )
        loadData();
    QTranslatableSettings config(filename, QSettings::IniFormat);

    // Try from config file
    config.beginGroup( id );
    if (config.contains("Description"))
        return config.value("Description", id).toString();
    return QString();
}

/*!
  Returns the list of identifiers for sets loaded by this profile.
*/
QStringList QIMPenProfile::charSets()
{
    if ( sets.isEmpty() )
        loadData();
    QStringList result;
    ProfileSetListIterator it = sets.begin();
    for ( ; it != sets.end(); ++it ) {
        result.append( (*it)->id);
    }
    return result;
}

QIMPenCharSet *QIMPenProfile::uppercase()
{
    return find(QIMPenCharSet::Upper);
}

QIMPenCharSet *QIMPenProfile::lowercase()
{
    return find(QIMPenCharSet::Lower);
}

QIMPenCharSet *QIMPenProfile::numeric()
{
    return find(QIMPenCharSet::Numeric);
}

QIMPenCharSet *QIMPenProfile::punctuation()
{
    return find(QIMPenCharSet::Punctuation);
}

QIMPenCharSet *QIMPenProfile::symbol()
{
    return find(QIMPenCharSet::Symbol);
}

QIMPenCharSet *QIMPenProfile::shortcut()
{
    return find(QIMPenCharSet::Shortcut);
}
