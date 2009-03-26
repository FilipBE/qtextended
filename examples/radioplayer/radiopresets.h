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

#ifndef RADIOPRESETS_H
#define RADIOPRESETS_H

#include "radioband.h"
#include <QDialog>
#include <QItemDelegate>
#include <QSqlTableModel>

class RadioPresetPrivate;
class QIODevice;
class QContent;
class QTableView;
class QListWidget;
class QListWidgetItem;
class QModelIndex;

class RadioPreset
{
public:
    RadioPreset();
    RadioPreset( const RadioPreset& other );
    ~RadioPreset();

    RadioPreset& operator=( const RadioPreset& other );

    QString region() const;
    void setRegion( const QString& value );

    QString name() const;
    void setName( const QString& value );

    QString genre() const;
    void setGenre( const QString& value );

    QString band() const;
    void setBand( const QString& value );

    RadioBand::Frequency frequency() const;
    void setFrequency( RadioBand::Frequency value );

    static QList<RadioPreset> loadKRadioPresets( QIODevice *device );

private:
    RadioPresetPrivate *d;

    void copyOnWrite();
};

class RadioPresetsModel : public QSqlTableModel
{
    Q_OBJECT
public:
    RadioPresetsModel( QObject *parent = 0 );
    ~RadioPresetsModel();

    struct Info
    {
        bool isValid;
        QString name;
        QString genre;
    };

    Info stationInfo( RadioBand::Frequency frequency, const QString& band );
    void addToPresets( RadioBand::Frequency frequency, const QString& band,
                       const QString& frequencyDescription,
                       const QString& stationName,
                       const QString& stationGenre,
                       const QString& region = QString(""),
                       bool updateDisplay = true );
    void importStationsFrom( const QContent& content );
    void clearRegion( const QString& region );

    QString region() const { return currentRegion; }
    QStringList regions() const { return regionList; }

    void updateRegion( const QString& region );

signals:
    void updated();

private:
    QString currentRegion;
    QStringList regionList;
};

class RadioPresetsDialog : public QDialog
{
    Q_OBJECT
public:
    RadioPresetsDialog( RadioPresetsModel *model, QWidget *parent );
    ~RadioPresetsDialog();

    RadioBand::Frequency frequency() const { return selectedFrequency; }
    QString band() const { return selectedBand; }

private slots:
    void importStations();
    void selectRegion();
    void personalStations();
    void modelUpdated();
    void clearStations();
    void deleteStation();
    void stationActivated( const QModelIndex& index );

private:
    RadioPresetsModel *radioStations;
    QTableView *stationsView;
    QAction *selectRegionAction;
    QAction *personalStationsAction;
    QAction *clearStationsAction;
    QAction *deleteStationAction;
    RadioBand::Frequency selectedFrequency;
    QString selectedBand;
};

class RadioRegionsDialog : public QDialog
{
    Q_OBJECT
public:
    RadioRegionsDialog( const QStringList& regions,
                        const QString& currentRegion, QWidget *parent );
    ~RadioRegionsDialog();

    QString region() const { return selectedRegion; }

private slots:
    void selected( QListWidgetItem *item );

private:
    QString selectedRegion;
    QListWidget *list;
};

class RadioStationDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    RadioStationDelegate( QObject *parent ) : QItemDelegate( parent ) {}
    ~RadioStationDelegate() {}

    QWidget *createEditor( QWidget *parent,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const;
};

#endif
