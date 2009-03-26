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

#include "radiopresets.h"
#include "radiobandmanager.h"
#include <qatomic.h>
#include <qxml.h>
#include <QDebug>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QHeaderView>
#include <QTableView>
#include <QListWidget>
#include <QVBoxLayout>
#include <QtopiaSql>
#include <QSettings>
#include <QDocumentSelectorDialog>
#include <QtopiaApplication>
#include <QMenu>
#include <QSoftMenuBar>

class RadioPresetPrivate
{
public:
    RadioPresetPrivate()
    {
        ref = 1;
        frequency = 0;
    }

    QAtomicInt ref;
    QString region;
    QString name;
    QString genre;
    QString band;
    RadioBand::Frequency frequency;
};

RadioPreset::RadioPreset()
{
    d = new RadioPresetPrivate();
}

RadioPreset::RadioPreset( const RadioPreset& other )
{
    d = other.d;
    d->ref.ref();
}

RadioPreset::~RadioPreset()
{
    if ( !d->ref.deref() )
        delete d;
}

RadioPreset& RadioPreset::operator=( const RadioPreset& other )
{
    if ( d != other.d ) {
        other.d->ref.ref();
        if ( !d->ref.deref() )
            delete d;
        d = other.d;
    }
    return *this;
}

QString RadioPreset::region() const
{
    return d->region;
}

void RadioPreset::setRegion( const QString& value )
{
    copyOnWrite();
    d->region = value;
}

QString RadioPreset::name() const
{
    return d->name;
}

void RadioPreset::setName( const QString& value )
{
    copyOnWrite();
    d->name = value;
}

QString RadioPreset::genre() const
{
    return d->genre;
}

void RadioPreset::setGenre( const QString& value )
{
    copyOnWrite();
    d->genre = value;
}

QString RadioPreset::band() const
{
    return d->band;
}

void RadioPreset::setBand( const QString& value )
{
    copyOnWrite();
    d->band = value;
}

RadioBand::Frequency RadioPreset::frequency() const
{
    return d->frequency;
}

void RadioPreset::setFrequency( RadioBand::Frequency value )
{
    copyOnWrite();
    d->frequency = value;
}

void RadioPreset::copyOnWrite()
{
    if ( d->ref != 1 ) {
        RadioPresetPrivate *newd = new RadioPresetPrivate();
        newd->region = d->region;
        newd->name = d->name;
        newd->genre = d->genre;
        newd->band = d->band;
        newd->frequency = d->frequency;
        d->ref.deref();
        d = newd;
    }
}

class RadioPresetContentHandler : public QXmlDefaultHandler
{
public:
    RadioPresetContentHandler();
    ~RadioPresetContentHandler();

    bool startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts );
    bool endElement( const QString& namespaceURI, const QString& localName, const QString& qName );
    bool characters( const QString& ch );
    bool ignorableWhitespace( const QString& ch );

    QList<RadioPreset> presets;

    QString region() const;

private:
    bool inStation;
    QString content;
    QString country;
    QString city;
    QString name;
    QString genre;
    QString frequency;
};

RadioPresetContentHandler::RadioPresetContentHandler()
{
    this->inStation = false;
}

RadioPresetContentHandler::~RadioPresetContentHandler()
{
}

bool RadioPresetContentHandler::startElement( const QString& , const QString& localName, const QString& , const QXmlAttributes& )
{
    if ( localName == "FrequencyRadioStation" && !inStation ) {
        name = QString();
        genre = QString();
        frequency = QString();
        inStation = true;
    }
    content = QString();
    return true;
}

// Modified version of QString::simplified() that removes bogus characters.
static QString simplified( const QString& str )
{
    QString temp = str;
    return temp.replace( QChar(0xFFFD), QChar(' ') ).simplified().trimmed();
}

// Round a frequency value to exactly two decimal places, to fix
// obviously bogus values like "99.8998".  And then scale up to
// a real frequency value in Hz.
static RadioBand::Frequency parseFrequency( const QString& freqstr )
{
    double dfreq = freqstr.toDouble();
    if ( dfreq >= 50.0 ) {
        // Frequency value in MHz.
        int freq = (int)( dfreq * 100.0 + 0.5 );
        return ((RadioBand::Frequency)freq) * (RadioBand::Frequency)10000;
    } else {
        // Frequency value in kHz.
        int freq = (int)( dfreq * 10000.0 + 0.5 );
        return ((RadioBand::Frequency)freq) * (RadioBand::Frequency)100;
    }
}

QString RadioPresetContentHandler::region() const
{
    if ( country.isEmpty() && city.isEmpty() )
        return QObject::tr("Unknown");
    else if ( country.isEmpty() )
        return city;
    else if ( city.isEmpty() )
        return country;
    else
        return city + ", " + country;
}

bool RadioPresetContentHandler::endElement( const QString& , const QString& localName, const QString& )
{
    if ( localName == "country" )
        country = simplified( content );
    else if ( localName == "city" )
        city = simplified( content );
    else if ( localName == "name" && inStation ) {
        name = simplified( content );
        int index = name.indexOf( '@' );
        if ( index >= 0 ) {
            genre = name.left( index );
            name = name.mid( index + 1 );
        }
    }
    else if ( localName == "frequency" && inStation )
        frequency = content;
    else if ( localName == "FrequencyRadioStation" && inStation &&
              !country.isEmpty() && !city.isEmpty() ) {
        RadioPreset preset;
        preset.setRegion( region() );
        preset.setName( simplified( name ) );
        preset.setGenre( simplified( genre ) );
        preset.setFrequency( parseFrequency( frequency ) );
        preset.setBand
            ( RadioBand::standardBandForFrequency( preset.frequency() ) );
        presets.append( preset );
    }
    return true;
}

bool RadioPresetContentHandler::characters( const QString& ch )
{
    content += ch;
    return true;
}

bool RadioPresetContentHandler::ignorableWhitespace( const QString& ch )
{
    content += ch;
    return true;
}

QList<RadioPreset> RadioPreset::loadKRadioPresets( QIODevice *device )
{
    RadioPresetContentHandler handler;
    QXmlSimpleReader reader;
    reader.setContentHandler( &handler );
    QXmlInputSource inputSource( device );
    reader.parse( inputSource );
    return handler.presets;
}

static QSqlDatabase presetsDatabase()
{
    QSqlDatabase db = QtopiaSql::instance()->applicationSpecificDatabase( "radioplayer" );
    QtopiaSql::instance()->ensureTableExists( "radiostations", db );
    return db;
}

RadioPresetsModel::RadioPresetsModel( QObject *parent )
    : QSqlTableModel( parent, presetsDatabase() )
{
    currentRegion = "";

    // Set the model to operate on the "radiostations" table.
    setTable( "radiostations" );
    setEditStrategy( QSqlTableModel::OnFieldChange );
    setSort( 0, Qt::AscendingOrder );   // sort on frequency

    // Select the initial region.
    QSettings settings( "Trolltech", "Radio" );
    settings.beginGroup( "Stations" );
    updateRegion( settings.value( "CurrentRegion" ).toString() );
}

RadioPresetsModel::~RadioPresetsModel()
{
}

RadioPresetsModel::Info RadioPresetsModel::stationInfo
        ( RadioBand::Frequency frequency, const QString& band )
{
    Info info;
    info.isValid = false;
    QSqlQuery query( database() );
    query.prepare( "select name, genre from radiostations where "
                     "frequency = :frequency and "
                     "band = :band and "
                     "region = :region");
    query.bindValue( ":frequency", frequency );
    query.bindValue( ":band", band );
    query.bindValue( ":region", currentRegion );
    if ( query.exec() && query.first() ) {
        info.name = query.value(0).toString();
        info.genre = query.value(1).toString();
        info.isValid = true;
    }
    return info;
}

void RadioPresetsModel::addToPresets
        ( RadioBand::Frequency frequency, const QString& band,
          const QString& frequencyDescription,
          const QString& stationName,
          const QString& stationGenre,
          const QString& region, bool updateDisplay )
{
    // Perform the modification on the specified region.
    currentRegion = region;

    // Don't change the database if the frequency is already registered.
    Info info = stationInfo( frequency, band );
    if ( info.isValid ) {
        if ( updateDisplay )
            updateRegion( region );
        return;
    }

    // Insert a new record into the table.
    QSqlQuery insert( database() );
    insert.prepare( "insert into radiostations "
                    "(frequency, band, frequencydescription, "
                    "name, genre, region)"
                    "values (:frequency, :band, :frequencydescription, "
                    ":name, :genre, :region)");
    insert.bindValue( ":frequency", frequency );
    insert.bindValue( ":band", band );
    insert.bindValue( ":frequencydescription", frequencyDescription );
    insert.bindValue( ":name", stationName );
    insert.bindValue( ":genre", stationGenre );
    insert.bindValue( ":region", region );
    insert.exec();

    // Update the model contents.
    if ( updateDisplay )
        updateRegion( region );
}

void RadioPresetsModel::importStationsFrom( const QContent& content )
{
    QIODevice *device = content.open();
    if ( device ) {
        QList<RadioPreset> presets = RadioPreset::loadKRadioPresets( device );
        delete device;

        foreach ( RadioPreset preset, presets ) {
            addToPresets( preset.frequency(), preset.band(),
                          RadioBandManager::formatFrequency
                                ( preset.frequency() ), 
                          preset.name(), preset.genre(),
                          preset.region(), false );
        }

        // Refresh the model display after the additions.
        updateRegion( currentRegion );
    }
}

void RadioPresetsModel::clearRegion( const QString& region )
{
    QSqlQuery remove( database() );
    remove.prepare( "delete from radiostations where region = :region" );
    remove.bindValue( ":region", region );
    remove.exec();
    if ( region == currentRegion )
        updateRegion( "" );
    else
        updateRegion( currentRegion );
}

void RadioPresetsModel::updateRegion( const QString& region )
{
    currentRegion = region;
    setFilter( "region=\"" + region + "\"" );
    select();

    // Save the region to the settings file.
    QSettings settings( "Trolltech", "Radio" );
    settings.beginGroup( "Stations" );
    settings.setValue( "CurrentRegion", region );
    settings.sync();

    // Get the list of all regions in the database.
    QSqlQuery query( database() );
    query.prepare( "select distinct region from radiostations" );
    regionList = QStringList();
    if ( query.exec() && query.first() ) {
        do
        {
            QString reg = query.value(0).toString();
            if ( !reg.isEmpty() )
                regionList += reg;
        }
        while ( query.next() );
    }

    emit updated();
}

RadioPresetsDialog::RadioPresetsDialog
        ( RadioPresetsModel *model, QWidget *parent )
    : QDialog( parent )
{
    selectedFrequency = 0;
    radioStations = model;
    connect( radioStations, SIGNAL(updated()), this, SLOT(modelUpdated()) );

    QVBoxLayout *vbox = new QVBoxLayout( this );
    stationsView = new QTableView( this );
    stationsView->setAlternatingRowColors( true );
    vbox->addWidget( stationsView );

    // Tell the view to display the table contents.
    stationsView->setModel( radioStations );
    stationsView->setColumnHidden( 0, true );   // hide frequency
    stationsView->setColumnHidden( 1, true );   // hide band
    stationsView->setColumnHidden( 4, true );   // hide genre
    stationsView->setColumnHidden( 5, true );   // hide region
    stationsView->horizontalHeader()->hide();
    stationsView->horizontalHeader()->setResizeMode( 3, QHeaderView::Stretch );
    stationsView->verticalHeader()->hide();
    stationsView->setItemDelegate( new RadioStationDelegate( this ) );

    connect( stationsView,
             SIGNAL(activated(QModelIndex)),
             this, SLOT(stationActivated(QModelIndex)) );

    QMenu *menu = QSoftMenuBar::menuFor( this );

    QAction *importStationsAction =
        new QAction( tr( "Import Stations ..." ), this );
    connect( importStationsAction, SIGNAL(triggered()), this, SLOT(importStations()) );
    importStationsAction->setWhatsThis( tr( "Import station frequency database" ) );
    menu->addAction( importStationsAction );

    selectRegionAction =
        new QAction( tr( "Select Region ..." ), this );
    connect( selectRegionAction, SIGNAL(triggered()), this, SLOT(selectRegion()) );
    selectRegionAction->setWhatsThis( tr( "Select presets from a specific region" ) );
    menu->addAction( selectRegionAction );

    personalStationsAction =
        new QAction( tr( "Personal Stations" ), this );
    connect( personalStationsAction, SIGNAL(triggered()), this, SLOT(personalStations()) );
    selectRegionAction->setWhatsThis( tr( "Select presets from personal settings" ) );
    menu->addAction( personalStationsAction );

    clearStationsAction =
        new QAction( tr( "Clear Stations" ), this );
    connect( clearStationsAction, SIGNAL(triggered()), this, SLOT(clearStations()) );
    clearStationsAction->setWhatsThis( tr( "Clear all stations in the selected region" ) );
    menu->addAction( clearStationsAction );

    deleteStationAction =
        new QAction( tr( "Delete Station" ), this );
    connect( deleteStationAction, SIGNAL(triggered()), this, SLOT(deleteStation()) );
    deleteStationAction->setWhatsThis( tr( "Delete the currently selected station" ) );
    menu->addAction( deleteStationAction );

    modelUpdated();
}

RadioPresetsDialog::~RadioPresetsDialog()
{
}

void RadioPresetsDialog::importStations()
{
    QDocumentSelectorDialog dialog( this );
    dialog.setWindowTitle( tr("Import") );
    QContentFilter filter
        ( QContentFilter::MimeType, "application/x-kradio-presets" );
    dialog.setFilter( filter );
    if ( QtopiaApplication::execDialog( &dialog ) == QDialog::Accepted )
        radioStations->importStationsFrom( dialog.selectedDocument() );
}

void RadioPresetsDialog::selectRegion()
{
    RadioRegionsDialog dialog
        ( radioStations->regions(), radioStations->region(), this );
    if ( QtopiaApplication::execDialog( &dialog ) == QDialog::Accepted )
        radioStations->updateRegion( dialog.region() );
}

void RadioPresetsDialog::personalStations()
{
    radioStations->updateRegion( "" );
}

void RadioPresetsDialog::modelUpdated()
{
    if ( radioStations->regions().isEmpty() ) {
        selectRegionAction->setVisible( false );
        selectRegionAction->setEnabled( false );
        personalStationsAction->setVisible( false );
        personalStationsAction->setEnabled( false );
    } else {
        selectRegionAction->setVisible( true );
        selectRegionAction->setEnabled( true );
        if ( radioStations->region().isEmpty() ) {
            personalStationsAction->setVisible( false );
            personalStationsAction->setEnabled( false );
        } else {
            personalStationsAction->setVisible( true );
            personalStationsAction->setEnabled( true );
        }
    }
    if ( radioStations->rowCount() > 0 ) {
        clearStationsAction->setVisible( true );
        clearStationsAction->setEnabled( true );
        deleteStationAction->setVisible( true );
        deleteStationAction->setEnabled( true );
    } else {
        clearStationsAction->setVisible( false );
        clearStationsAction->setEnabled( false );
        deleteStationAction->setVisible( false );
        deleteStationAction->setEnabled( false );
    }
}

void RadioPresetsDialog::clearStations()
{
    QString region = radioStations->region();
    QString prompt;
    if ( region.isEmpty() )
        prompt = tr("<qt>Clear all personal stations?</qt>");
    else
        prompt = tr("<qt>Clear all stations in %1?</qt>", "%1=a region name( e.g. Tokio )").arg( region );
    if ( QMessageBox::warning( this, tr("Clear Stations"), prompt,
           QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes ) {
        radioStations->clearRegion( region );
    }
}

void RadioPresetsDialog::deleteStation()
{
    QModelIndex index = stationsView->currentIndex();
    if ( index.isValid() ) {
        QSqlRecord record = radioStations->record( index.row() );
        QString name = record.value( "name" ).toString();
        if ( name.isEmpty() )
            name = record.value( "frequencydescription" ).toString();
        if ( QMessageBox::warning( this, tr("Delete Station"),
                tr("<qt>Delete %1?</qt>").arg( name ),
             QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes ) {
            radioStations->removeRow( index.row() );
        }
    }
}

void RadioPresetsDialog::stationActivated( const QModelIndex& index )
{
    if ( index.isValid() ) {
        QSqlRecord record = radioStations->record( index.row() );
        selectedFrequency = record.value( "frequency" ).toLongLong();
        selectedBand = record.value( "band" ).toString();
        accept();
    }
}

RadioRegionsDialog::RadioRegionsDialog
        ( const QStringList& regions, const QString& currentRegion,
          QWidget *parent )
    : QDialog( parent )
{
    setWindowTitle( tr("Regions") );

    QVBoxLayout *vbox = new QVBoxLayout( this );
    list = new QListWidget( this );
    list->setAlternatingRowColors( true );
    vbox->addWidget( list );

    QListWidgetItem *current = 0;
    foreach ( QString region, regions ) {
        QListWidgetItem *item = new QListWidgetItem( region, list );
        if ( region == currentRegion )
            current = item;
    }
    list->sortItems();
    if ( !current )
        current = list->item(0);
    if ( current )
        list->setCurrentItem( current );

    connect( list, SIGNAL(itemActivated(QListWidgetItem*)),
             this, SLOT(selected(QListWidgetItem*)) );
}

RadioRegionsDialog::~RadioRegionsDialog()
{
}

void RadioRegionsDialog::selected( QListWidgetItem *item )
{
    selectedRegion = item->text();
    accept();
}

QWidget *RadioStationDelegate::createEditor
    ( QWidget *parent, const QStyleOptionViewItem &option,
      const QModelIndex &index ) const
{
    // Don't allow editing of the frequency column, only the station name.
    if ( index.column() == 2 )
        return 0;
    return QItemDelegate::createEditor( parent, option, index );
}
