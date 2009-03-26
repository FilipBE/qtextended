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

#include <qtranslatablesettings.h>
#include <qtopianamespace.h>

class QTranslatableSettingsPrivate
{
public:
    QString trfile;
    QString trcontext;
};

/*!
    \class QTranslatableSettings
    \inpublicgroup QtBaseModule

    \brief The QTranslatableSettings class provides persistent application
    settings that can be translated.

    QTranslatableSettings is functionally equivalent to QSettings except
    that strings read by QTranslatableSettings can be translated.

    Translatable settings are denoted in the settings file
    by square brackets '[]' in the key.  For example:
    \code
    [Plugin]
    Name[] = Clock
    [Translation]
    File=clockplugin
    Context=Clock
    \endcode
    The \i Name key is translatable and the [Translation] group specifies
    the file containing the translations.

    Caution should be used when passing a QTranslatableSettings object to
    a function that takes a QSettings& argument, since that function will
    \i not use any translations.

    \sa QSettings

    \ingroup io
*/


/*!
    Constructs a QTranslatableSettings object to access settings of the
    application called \a application from the organization called \a
    organization with parent \a parent.

    For example:
    \code
        QTranslatableSettings settings("Moose Tech", "Facturo-Pro");
    \endcode

    The scope is QSettings::UserScope and the format is
    QSettings::NativeFormat.

    \sa {Fallback Mechanism}
*/
QTranslatableSettings::QTranslatableSettings(const QString &organization,
          const QString &application, QObject *parent)
    : QSettings(organization,application,parent) { initTranslation(); }


/*!
    Constructs a QTranslatableSettings object to access settings of the
    application called \a application from the organization called \a
    organization with parent \a parent.

    If \a scope is QSettings::UserScope, the QTranslatableSettings object searches
    user-specific settings before searching system-wide
    settings as a fallback. If \a scope is
    QSettings::SystemScope, the QTranslatableSettings object ignores user-specific
    settings and provides access to system-wide settings.

    The storage format is always QSettings::NativeFormat.

    If no application name is given, the QTranslatableSettings object will
    only access the organization-wide \l{Fallback Mechanism}{locations}.
*/
QTranslatableSettings::QTranslatableSettings(Scope scope, const QString &organization,
          const QString &application, QObject *parent)
    : QSettings(scope,organization,application,parent) { initTranslation(); }

/*!
    Constructs a QTranslatableSettings object to access settings of the
    application called \a application from the organization called
    \a organization with parent \a parent.

    If \a scope is QSettings::UserScope, the QTranslatableSettings object searches
    user-specific settings before searching system-wide
    settings as a fallback. If \a scope is
    QSettings::SystemScope, the QTranslatableSettings object ignores user-specific
    settings and provides access to system-wide settings.

    If \a format is QSettings::NativeFormat, the native API is used for
    storing settings. If \a format is QSettings::IniFormat, the INI format
    is used.

    If no application name is given, the QTranslatableSettings object will
    only access the organization-wide \l{Fallback Mechanism}{locations}.
*/
QTranslatableSettings::QTranslatableSettings(Format format, Scope scope, const QString &organization,
          const QString &application, QObject *parent)
    : QSettings(format,scope,organization,application,parent) { initTranslation(); }


/*!
    Constructs a QTranslatableSettings object to access the settings
    stored in the file called \a fileName, with parent \a parent. The file is created if it does not exist.

    If \a format is QSettings::NativeFormat, the meaning of \a fileName
    depends on the platform as follows:
    \list
    \o Unix/X11  - \a fileName is the name of an INI file
    \o Mac OS X -  \a fileName is the name of a .plist file
    \o Windows -  \a fileName is a path in the system registry.
    \endlist

    If \a format is QSettings::IniFormat, \a fileName is the name of an INI
    file.

    \sa QSettings::fileName()
*/
QTranslatableSettings::QTranslatableSettings(const QString &fileName, Format format, QObject *parent)
    : QSettings(fileName,format,parent) { initTranslation(); }

/*!
    Constructs a QTranslatableSettings object for accessing settings of the
    application and organization set previously with a call to
    QCoreApplication::setOrganizationName(),
    QCoreApplication::setOrganizationDomain(), and
    QCoreApplication::setApplicationName().

    The scope is QSettings::UserScope and the format is QSettings::NativeFormat.

    The code

    \code
        QSettings settings("Moose Soft", "Facturo-Pro");
    \endcode

    is equivalent to

    \code
        QCoreApplication::setOrganizationName("Moose Soft");
        QCoreApplication::setApplicationName("Facturo-Pro");
        QSettings settings;
    \endcode

    If QCoreApplication::setOrganizationName() and
    QCoreApplication::setApplicationName() has not been previously
    called, the QTranslatableSettings object will not be able to read or write
    any settings, and QSettings::status() will return AccessError.

    On Mac OS X, if both a name and an Internet domain are specified
    for the organization, the domain is preferred over the name. On
    other platforms, the name is preferred over the domain.

    \sa QCoreApplication::setOrganizationName(),
        QCoreApplication::setOrganizationDomain(),
        QCoreApplication::setApplicationName()
*/
QTranslatableSettings::QTranslatableSettings(QObject *parent) :
    QSettings(parent) { initTranslation(); }

void QTranslatableSettings::initTranslation()
{
    d = new QTranslatableSettingsPrivate;
    d->trfile = QSettings::value("Translation/File").toString();
    d->trcontext = QSettings::value("Translation/Context").toString();
}


/*!
  Destroys the QTranslatableSettings object.

  Any unsaved changes will eventually be written to permanent storage.
*/
QTranslatableSettings::~QTranslatableSettings()
{
    if ( d )
        delete d;
    d = 0;
}

/*!
    Returns the value for setting \a key. If the setting does not
    exist \a defaultValue is returned.

    If no default value is specified, a default QVariant is
    returned.

    If the key is translatable, that is, it has square brackets [] after it
    in the INI settings file, the translated value will be returned.

    for example:

    \code
        QTranslatableSettings settings;
        settings.setValue("animal/snake", 58);
        settings.value("animal/snake", 1024).toInt();   // returns 58
        settings.value("animal/zebra", 1024).toInt();   // returns 1024
        settings.value("animal/zebra").toInt();         // returns 0
    \endcode

    \sa QSettings::setValue(), contains(), QSettings::remove()
*/
QVariant QTranslatableSettings::value(const QString &key, const QVariant &defaultValue) const
{
    // We try untranslated first, because:
    //  1. It's the common case
    //  2. That way the value can be WRITTEN (becoming untranslated)
    QVariant r = QSettings::value( key );
    if ( !r.isNull() )
        return r;

    if ( !d->trcontext.isNull() ) {
        r = QSettings::value( key + "[]" );
        if ( !r.isNull() )
            return Qtopia::translate(d->trfile,d->trcontext,r.toString());
    } else {
        QStringList l = Qtopia::languageList();
        r = QSettings::value( key + "["+l[0]+"]" );
        if ( !r.isNull() )
            return r;
        if ( l.count()>1 ) {
            r = QSettings::value( key + "["+l[1]+"]" );
            if ( !r.isNull() )
                return r;
        }
    }

    return defaultValue;
}

/*!
    \overload

    Reimplemented for internal reasons.

    \sa QSettings::allKeys()
*/
QStringList QTranslatableSettings::allKeys() const
{
    QStringList r = QSettings::allKeys();
    stripTranslations(r);
    return r;
}

/*!
    \overload

    Reimplemented for internal reasons.

    \sa QSettings::childKeys()

*/
QStringList QTranslatableSettings::childKeys() const
{
    QStringList r = QSettings::childKeys();
    stripTranslations(r);
    return r;
}

/*!
    \overload

    Reimplemented for internal reasons.

    Returns true if there exists a setting called \a key; returns
    false otherwise.

    If a group is set using beginGroup(), \a key is taken to be
    relative to that group.

    \sa QSettings::contains()
*/
bool QTranslatableSettings::contains(const QString &key) const
{
    bool y = QSettings::contains(key);
    if ( y ) return y;
    y = QSettings::contains(key+"[]");
    if ( y ) return y;
    QStringList l = Qtopia::languageList();
    y = QSettings::contains( key + "["+l[0]+"]" );
    if ( y ) return y;
    if ( l.count()>1 )
        y = QSettings::contains( key + "["+l[1]+"]" );
    return y;
}

void QTranslatableSettings::stripTranslations(QStringList& r)
{
    for (QStringList::Iterator it = r.begin(); it!=r.end(); ++it) {
        QString& k = *it;
        if ( k.at(k.length()-1) == ']' )
            k = k.left(k.indexOf('['));
    }
}

