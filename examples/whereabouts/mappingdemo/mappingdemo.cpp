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

#include "mappingdemo.h"

#include <QWhereabouts>
#include <QWhereaboutsFactory>
#include <QNmeaWhereabouts>
#include <QtopiaApplication>
#include <QSoftMenuBar>
#include <qtopianamespace.h>

#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QStackedWidget>
#include <QTimer>
#include <QHttp>
#include <QFile>
#include <QRadioButton>

// this uses the special 'localhost' key for the gmaps key
const QString GMAPS_STATICMAP_URL_TEMPLATE =  "http://maps.google.com/staticmap?center=%1,%2&zoom=14&size=%3x%4&maptype=mobile&markers=%1,%2&key=ABQIAAAAnfs7bKE82qgb3Zc2YyS-oBT2yXp_ZAY8_ufC3CFXhHIE1NvwkxSySz_REpPq-4WZA27OwgbtyR3VcA";


class WhereaboutsInfoWidget : public QWidget
{
    Q_OBJECT
public:
    WhereaboutsInfoWidget(QWidget *parent = 0);

public slots:
    void whereaboutsStateChanged(QWhereabouts::State state);
    void whereaboutsUpdated(const QWhereaboutsUpdate &update);

private slots:
    void reloadMap();
    void httpRequestFinished(int id, bool error);

private:
    QHttp *m_http;
    QLabel *m_mapWidget;
    QStackedWidget *m_infoStack;
    QLabel *m_labelTime;
    QLabel *m_labelPosn;
    QLabel *m_labelCourseSpeed;
    QTimer *m_timer;

    QWhereaboutsUpdate m_lastUpdate;
};


WhereaboutsInfoWidget::WhereaboutsInfoWidget(QWidget *parent)
    : QWidget(parent),
      m_http(new QHttp(this)),
      m_mapWidget(new QLabel),
      m_infoStack(new QStackedWidget),
      m_labelTime(new QLabel),
      m_labelPosn(new QLabel),
      m_labelCourseSpeed(new QLabel),
      m_timer(0)
{
    m_http->setHost("maps.google.com");
    connect(m_http, SIGNAL(requestFinished(int,bool)),
            SLOT(httpRequestFinished(int,bool)));

    m_mapWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    m_infoStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    QFont f = m_labelTime->font();
    f.setPointSize(f.pointSize() - 1);
    m_labelPosn->setFont(f);
    m_labelCourseSpeed->setFont(f);
    m_labelTime->setFont(f);
    m_labelCourseSpeed->setText(tr("Bearing unknown; speed unknown"));

    // widget to hold the labels that display the GPS information
    QWidget *statsWidget = new QWidget;
    QVBoxLayout *statsLayout = new QVBoxLayout;
    statsLayout->setMargin(0);
    statsLayout->setSpacing(0);
    statsLayout->addWidget(m_labelPosn);
    statsLayout->addWidget(m_labelCourseSpeed);
    statsLayout->addWidget(m_labelTime);
    statsWidget->setLayout(statsLayout);
    m_infoStack->addWidget(new QLabel(tr("GPS not available!")));
    m_infoStack->addWidget(new QLabel(tr("Connected, waiting for fix...")));
    m_infoStack->addWidget(statsWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_mapWidget);
    mainLayout->addWidget(m_infoStack);
    setLayout(mainLayout);
}

void WhereaboutsInfoWidget::whereaboutsStateChanged(QWhereabouts::State state)
{
    switch (state) {
        case QWhereabouts::NotAvailable:
            m_infoStack->setCurrentIndex(0);    // "GPS not available!"
            break;
        case QWhereabouts::PositionFixAcquired:
            m_infoStack->setCurrentIndex(2);
            if (!m_timer) {
                QTimer *timer = new QTimer(this);
                connect(timer, SIGNAL(timeout()), SLOT(reloadMap()));
                timer->start(1200);
            }
            break;
        default:
            m_infoStack->setCurrentIndex(1);    // "Connected, waiting for fix..."
    }
}

void WhereaboutsInfoWidget::whereaboutsUpdated(const QWhereaboutsUpdate &update)
{
    m_labelPosn->setText(tr("Position: %1")
            .arg(update.coordinate().toString(QWhereaboutsCoordinate::DecimalDegreesWithHemisphere)));

    if (update.dataValidityFlags() & QWhereaboutsUpdate::Course &&
            update.dataValidityFlags() & QWhereaboutsUpdate::GroundSpeed) {
        m_labelCourseSpeed->setText(tr("Bearing %1%2, travelling at %3 km/h")
                .arg(QString::number(update.course())).arg(QChar(0x00B0))   // degrees symbol
                .arg(QString::number(update.groundSpeed() * 3.6, 'f', 1)));
    }

    m_labelTime->setText(tr("(Last update: %1)").
            arg(update.updateDateTime().toLocalTime().time().toString()));

    m_lastUpdate = update;
}

void WhereaboutsInfoWidget::reloadMap()
{
    // Google Maps does not provide maps larger than 640x480
    int width = qMin(m_mapWidget->width(), 640);
    int height = qMin(m_mapWidget->height(), 480);
    QString url = GMAPS_STATICMAP_URL_TEMPLATE
                        .arg(QString::number(m_lastUpdate.coordinate().latitude()))
                        .arg(QString::number(m_lastUpdate.coordinate().longitude()))
                        .arg(QString::number(width))
                        .arg(QString::number(height));
    m_http->get(url);
}

void WhereaboutsInfoWidget::httpRequestFinished(int id, bool error)
{
    Q_UNUSED(id);
    if (error) {
        qWarning() << "Cannot load Google Maps image, got HTTP error:"
                << m_http->errorString();
        return;
    }

    if (m_http->currentRequest().method() == "GET") {
        QHttpResponseHeader response = m_http->lastResponse();
        if (response.statusCode() != 200) {
            qWarning() << "Request for Google Maps image failed, server responded"
                    << response.statusCode() << response.reasonPhrase();
            return;
        }

        QPixmap pixmap(m_mapWidget->width(), m_mapWidget->height());
        QByteArray data = m_http->readAll();
        if (!pixmap.loadFromData(data, "GIF")) {
            qWarning() << "Cannot read Google Maps image!";
            return;
        }
        m_mapWidget->setPixmap(pixmap);
        m_mapWidget->adjustSize();
    }
}


//============================================================

/*
    This example shows the basics of using the Qt Extended Whereabouts library to
    read GPS data. It reads GPS data and then displays the location using
    Google Maps.

    When you run the example, you can choose one of the following
    sources of GPS position data:

    1) The default GPS data source, as specified in
       $QPEDIR/etc/Settings/Trolltech/Whereabouts.conf. See the
       QWhereaboutsFactory documentation for more details.

       If your device has built-in GPS hardware and is a supported Qt Extended
       device configuration, this option may be already configured to use the
       GPS hardware. Otherwise, the default data source may be the built-in
       source which requires GPSd (http://gpsd.berlios.de) to fetch position
       data.

    2) The nmea_sample.txt file in this example's directory, which
       is a sample of recorded NMEA data. This option does not require
       GPS hardware or GPSd.

    If you would like to add your own custom GPS data sources, see the
    QWhereaboutsPlugin documentation.
*/

MappingDemo::MappingDemo(QWidget *parent, Qt::WFlags f)
    : QMainWindow(parent, f)
{
    int dialogResult = QDialog::Rejected;
    QWhereabouts *whereabouts = chooseWhereabouts(&dialogResult);
    if (!whereabouts) {
        if (dialogResult == QDialog::Accepted)
            QMessageBox::warning(this, tr("Error"), tr("Cannot find a location data source."));
        QTimer::singleShot(0, this, SLOT(close()));
        return;
    }

    WhereaboutsInfoWidget *display = new WhereaboutsInfoWidget;
    display->whereaboutsStateChanged(whereabouts->state());

    connect(whereabouts, SIGNAL(stateChanged(QWhereabouts::State)),
            display, SLOT(whereaboutsStateChanged(QWhereabouts::State)));
    connect(whereabouts, SIGNAL(updated(QWhereaboutsUpdate)),
            display, SLOT(whereaboutsUpdated(QWhereaboutsUpdate)));

    whereabouts->startUpdates();

    setCentralWidget(display);
    setWindowTitle(tr("Mapping Demo"));
}

QWhereabouts *MappingDemo::chooseWhereabouts(int *dialogResult)
{
    QRadioButton *buttonDefault = new QRadioButton(tr("Default GPS source"));
    buttonDefault->setChecked(true);
    QRadioButton *buttonNmea = new QRadioButton(tr("Sample NMEA log"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("Choose GPS data source:")));
    layout->addWidget(buttonDefault);
    layout->addWidget(buttonNmea);

    QDialog dialog;
    dialog.setLayout(layout);
    dialog.setWindowTitle(tr("Mapping Demo"));
    QSoftMenuBar::setLabel(&dialog, Qt::Key_Back, QSoftMenuBar::Ok);
    *dialogResult = QtopiaApplication::execDialog(&dialog);

    if (*dialogResult == QDialog::Rejected)
        return 0;

    if (buttonDefault->isChecked()) {
        QWhereabouts *whereabouts = QWhereaboutsFactory::create();
        if (whereabouts)
            whereabouts->setParent(this);
        return whereabouts;

    } else {
        QFile *sampleFile =
                new QFile(Qtopia::qtopiaDir() + "etc/whereabouts/nmea_sample.txt", this);
        sampleFile->open(QIODevice::ReadOnly);
        QNmeaWhereabouts *whereabouts = new QNmeaWhereabouts(this);
        whereabouts->setUpdateMode(QNmeaWhereabouts::SimulationMode);
        whereabouts->setSourceDevice(sampleFile);
        return whereabouts;
    }
}

#include "mappingdemo.moc"
