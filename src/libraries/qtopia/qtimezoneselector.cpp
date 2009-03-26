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


#include "qtimezoneselector.h"

#include <QFile>
#include <QRegExp>
#include <QLayout>
#include <QDesktopWidget>
#include <QSettings>
#include <QStringList>
#include <QComboBox>
#include <QAbstractItemView>
#include <QTimeZone>

#include <stdlib.h>

#include "qtimezone.h"
#include "qworldmap.h"

#include <qworldmapdialog.h>
#include "qtopiaapplication.h"
#include <qsoftmenubar.h>

#include <qtopiachannel.h>


// ============================================================================
//
// QTimeZoneComboBox
//
// ============================================================================

class QTimeZoneComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit QTimeZoneComboBox( QWidget* parent=0 );
    ~QTimeZoneComboBox();

    QString currZone() const;
    QString prevZone() const;

    bool okToShow;

public slots:
    void setToPreviousIndex();
    void setCurrZone( const QString& id );

protected:
    friend class QTimeZoneSelector;
    void updateZones();

private slots:
    void handleSystemChannel(const QString&, const QByteArray&);
    void indexChange( const int index );

private:
    QSettings   cfg;
    QStringList identifiers;
    QStringList extras;
    int         prevIndex1;
    int         prevIndex2;
};



// ============================================================================
//
// QTimeZoneSelectorPrivate
//
// ============================================================================

class QTimeZoneSelectorPrivate : public QObject
{
    Q_OBJECT
public:
    QTimeZoneSelectorPrivate(QTimeZoneSelector *p) : QObject(0), q(p), includeLocal(false) {}
    QTimeZoneSelector *q;
    QTimeZoneComboBox *cmbTz;
    bool includeLocal;

    void showWorldmapDialog();
};


// ============================================================================
//
// QTimeZoneComboBox
//
// ============================================================================

/*!
  \internal
*/
QTimeZoneComboBox::QTimeZoneComboBox( QWidget *p )
:   QComboBox( p ),
    okToShow(1),
    cfg("Trolltech","WorldTime"),
    prevIndex1( 0 ),
    prevIndex2( 0 )
{
    cfg.beginGroup("TimeZones");
    updateZones();
    prevIndex1 = prevIndex2 = currentIndex();

    connect( qobject_cast<QComboBox*>( this ),
             SIGNAL(currentIndexChanged(int)),
             this,
             SLOT(indexChange(int)) );

    // listen on QPE/System
    QtopiaChannel *channel = new QtopiaChannel( "QPE/System", this );
    connect( channel, SIGNAL(received(QString,QByteArray)),
        this, SLOT(handleSystemChannel(QString,QByteArray)) );
}

/*!
  \internal
*/
QTimeZoneComboBox::~QTimeZoneComboBox()
{
}

/*!
  \internal
*/
void QTimeZoneComboBox::updateZones()
{
    QString cur = currentText();
    clear();
    identifiers.clear();
    int curix=0;
    QString tz = QTimeZone::current().id();
    bool tzFound = false; // found the current timezone.
    bool hasCur = !cur.isEmpty();
    int listIndex = 0;
    QStringList comboList;
    if (QTimeZoneSelectorPrivate *tzsp = qobject_cast<QTimeZoneSelectorPrivate*>(parent())) {
        if ( tzsp->includeLocal ) {
            // overide to the 'local' type.
            identifiers.append( "None" ); // No tr
            comboList << tr("None");
            if ( cur == tr("None") || !hasCur )
                curix = 0;
            listIndex++;
        }
    }
    int cfgIndex = 0;
    while (1) {
        QString zn = cfg.value("Zone"+QString::number(cfgIndex), QString()).toString();
        if ( zn.isNull() )
            break;
        if ( zn == tz )
            tzFound = true;

        QString nm = QTimeZone( zn.toLatin1() ).name();
        identifiers.append(zn);
        comboList << nm;
        if ( nm == cur || (!hasCur && zn == tz) )
            curix = listIndex;
        ++cfgIndex;
        ++listIndex;
    }
    //     for (int i=0; i<extras.count(); i++) {
    for (QStringList::Iterator it=extras.begin(); it!=extras.end(); ++it) {
        QTimeZone z( (*it).toLatin1() );
        comboList << z.name();
        identifiers.append(*it);
        if ( *it == cur  || (!hasCur && *it == tz) )
            curix = listIndex;
        ++listIndex;
    }
    if ( !tzFound && !tz.isEmpty()) {
        identifiers.append(tz);
        QString nm = QTimeZone(tz.toLatin1()).name();
        comboList << nm;
        if ( nm == cur  || !hasCur )
            curix = listIndex;
        ++listIndex;
    }
    comboList << tr("More...");
    addItems(comboList);

    setCurrentIndex(curix);
    emit activated(curix);
}

/*!
  \internal
*/
void QTimeZoneComboBox::indexChange( const int index )
{
    prevIndex2 = prevIndex1;
    if ( index < 0 )
        prevIndex1 = 0;
    else
        prevIndex1 = index;
}

/*!
  \internal
*/
void QTimeZoneComboBox::setToPreviousIndex()
{
    okToShow = false;
    setCurrentIndex( prevIndex2 );
}

/*!
  \internal
*/
QString QTimeZoneComboBox::prevZone() const
{
    if (identifiers.count() && prevIndex2 < identifiers.count())
        return identifiers[prevIndex2];
    return QString();
}

/*!
  \internal
*/
QString QTimeZoneComboBox::currZone() const
{
    if (identifiers.count() && identifiers.count() > currentIndex()) // Not "More..."
        return identifiers[currentIndex()];
    return QString();
}

/*!
  \internal
*/
void QTimeZoneComboBox::setCurrZone( const QString& id )
{
    for (int i=0; i<identifiers.count(); i++) {
        if ( identifiers[i] == id ) {
            if ( currentIndex() != i ) {
                setCurrentIndex(i);
                emit activated(i);
            }
            return;
        }
    }

     QString name = QTimeZone(id.toLatin1()).name();
    int index = count() - 1;
    if ( index > 0 ) {
        insertItem( index, name );
        setCurrentIndex( index );
        QModelIndex modelIndex = model()->index(index, modelColumn(), rootModelIndex());
        view()->setCurrentIndex( modelIndex );
        identifiers.append(id);
        extras.append(id);
        emit activated( index );
    }
}

/*!
  \internal
*/
void QTimeZoneComboBox::handleSystemChannel(const QString&msg, const QByteArray&)
{
    if ( msg == "timeZoneListChange()" ) {
        updateZones();
    }
}

// ============================================================================
//
// QTimeZoneSelectorPrivate
//
// ============================================================================

/*!
  \internal
*/
void QTimeZoneSelectorPrivate::showWorldmapDialog( void )
{
    if (!cmbTz->okToShow) {
        cmbTz->setToPreviousIndex();
        cmbTz->okToShow = true;
        return;
    }

    QWorldmapDialog* map = new QWorldmapDialog( q );

    if ( cmbTz->prevZone().isEmpty() || ( cmbTz->prevZone() == "None" ) )
        map->setZone( QTimeZone::current() );
    else
        map->setZone( QTimeZone( cmbTz->prevZone().toLatin1() ) );

    connect(map, SIGNAL(zoneSelected(const QString& )),
            cmbTz,SLOT(setCurrZone(const QString &)));

    if (QtopiaApplication::execDialog(map) == QDialog::Accepted
        && map->selectedZone().isValid()
        && map->selectedZone() != QTimeZone::current()) {
        cmbTz->setCurrZone(map->selectedZone().id());
    } else {
        cmbTz->setToPreviousIndex();
    }
}

// ============================================================================
//
// QTimeZoneSelector
//
// ============================================================================

/*!
  \class QTimeZoneSelector
    \inpublicgroup QtBaseModule


  \brief The QTimeZoneSelector class provides a widget for selecting a time zone.
  \ingroup time

  QTimeZoneSelector presents a list of common city names, corresponding to time zones, for the user to choose from.
  A "More..." option allows the user to access all time zones via a world map.

  The cities included in the initial list are determined by the configuration file \c WorldTime.conf.
  This file has the following format:

  \code
  [TimeZones]
  Zone0=Area/Location
  ...
  ZoneN=Area/Location
  \endcode

  where each Area/Location is a \l {http://www.twinsun.com/tz/tz-link.htm}{zoneinfo} identifier. For example:

  \code
  [TimeZones]
  Zone0=America/New_York
  Zone1=America/Los_Angeles
  Zone2=Europe/Oslo
  ...
  \endcode

  The selector list also includes the current time zone, regardless of whether or not that time zone
  is represented by a city in the configuration file.

  \sa QTimeZone
*/

/*!
    Creates a new QTimeZoneSelector with parent \a p.  The selector will be
    populated with the appropriate timezones (see \l {Detailed Description}
    for an explanation of which zones will be included).
*/
QTimeZoneSelector::QTimeZoneSelector(QWidget* p) :
    QWidget(p)
{
    QHBoxLayout *hbl = new QHBoxLayout(this);
    hbl->setMargin(0);
#ifndef QT_NO_TRANSLATION
    static int transLoaded = 0;
    if (!transLoaded) {
        QtopiaApplication::loadTranslations("timezone");
        transLoaded++;
    }
#endif
    d = new QTimeZoneSelectorPrivate(this);

    // build the combobox before we do any updates...
    d->cmbTz = new QTimeZoneComboBox( this );
    d->cmbTz->setObjectName( "timezone combo" );
    hbl->addWidget(d->cmbTz);

    setFocusPolicy(Qt::TabFocus);
    setFocusProxy(d->cmbTz);

    // set up a connection to catch a newly selected item and throw our
    // signal
    QObject::connect( d->cmbTz, SIGNAL(activated(int)),
                      this, SLOT(tzActivated(int)) );
}

/*!
  Destroys the QTimeZoneSelector.
*/
QTimeZoneSelector::~QTimeZoneSelector()
{
    delete d;
}

/*!
  If \a b is true, allow no timezone ("None") as an option.
  If \a b is false then a specific timezone must be selected.
*/
void QTimeZoneSelector::setAllowNoZone(bool b)
{
    d->includeLocal = b;
    d->cmbTz->updateZones();
}

/*!
  \property QTimeZoneSelector::allowNoZone
  \brief true if no timezone is included as an option; false otherwise.
*/

/*!
  Returns true if no timezone ("None") is included as an option; otherwise returns false.
*/
bool QTimeZoneSelector::allowNoZone() const
{
    return d->includeLocal;
}

/*!
  \property QTimeZoneSelector::currentZone
  \brief the currently selected timezone.
*/

/*!
  Returns the currently selected timezone as a string in Area/Location format, for example,
  \code Australia/Brisbane \endcode
*/
QString QTimeZoneSelector::currentZone() const
{
    return d->cmbTz->currZone();
}

/*!
  Sets the current timezone to \a id. The \a id should be
  a \l QString in Area/Location format, for example, \code Australia/Brisbane \endcode
*/
void QTimeZoneSelector::setCurrentZone( const QString& id )
{
    d->cmbTz->setCurrZone( id );
}

/*! \fn void QTimeZoneSelector::zoneChanged( const QString& id )
  This signal is emitted whenever the time zone is changed.
  The \a id
  is a \l QString in Area/Location format, for example, \code Australia/Brisbane \endcode
*/

/*!
  \internal
*/
void QTimeZoneSelector::tzActivated( int idx )
{
    if (idx == d->cmbTz->count()-1) {
        d->showWorldmapDialog();
    } else {
        emit zoneChanged( d->cmbTz->currZone() );
    }
}

#include "qtimezoneselector.moc"
