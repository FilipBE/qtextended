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

#include <QWorldmapDialog>
#include <QDialog>
#include <QLayout>
#include <QTimeZone>
#include <QWorldmap>
#include <QtopiaApplication>
#include <QApplication>
#include <qsoftmenubar.h>
#include <QDesktopWidget>


/*!
  \class QWorldmapDialog
    \inpublicgroup QtBaseModule


  \brief The QWorldmapDialog class provides a dialog for selecting a time zone, using the QWorldmap class.
  \ingroup time

  QWorldmapDialog presents a dialog of a map of the world.


  \sa QTimeZone
  \sa QWorldmap
*/

/*!
    Creates a new QWorldmapDialog with parent \a parent.
*/
QWorldmapDialog::QWorldmapDialog(QWidget* parent)
:   QDialog(parent),
    realResult(0),
    mMap(0),
    mZone(),
    reallyDone(0)
{
    qLog(Time) << __PRETTY_FUNCTION__;


    setWindowTitle( tr( "Select Time Zone" ) );
    QVBoxLayout *bl = new QVBoxLayout(this);
    mMap = new QWorldmap(this);

    bl->addWidget( mMap );
    bl->setSpacing(0);
    bl->setMargin(0);

    connect(mMap, SIGNAL(newZone(QTimeZone)),
            this, SLOT(selected(QTimeZone)));

    connect(mMap, SIGNAL(selectZoneCanceled()),
            this, SLOT(cancelled()));

    if( Qtopia::mousePreferred()) {
        mMap->setFocus();
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Select);
    } else {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel);
    }
}

/*!
  Destroys a QWorldmapDialog object.
*/
QWorldmapDialog::~QWorldmapDialog()
{
 }


/*!
    \fn void QWorldmapDialog::zoneSelected(const QString& zone)
    This signal is emitted whenever the time zone is changed.
    \a zone is the new time zone.
*/

/*!
    \internal
*/
void QWorldmapDialog::showEvent(QShowEvent *e)
{
    if ( mZone.isValid() )
        mMap->setZone( mZone );

    mMap->selectNewZone();
    QDialog::showEvent(e);
}

/*!
    Set the time zone to \a zone.
*/
void QWorldmapDialog::setZone( const QTimeZone& zone )
{
    mZone = zone;
}

/*!
    \internal
*/
void QWorldmapDialog::selected( const QTimeZone& zone )
{
    if (zone.isValid()) {
        mZone = zone;
    }
}

/*!
    \internal
*/
void QWorldmapDialog::selected()
{
    qLog(Time) << __PRETTY_FUNCTION__;
 }

/*!
    \internal
*/
void QWorldmapDialog::cancelled()
{
    qLog(Time) << __PRETTY_FUNCTION__;
}


/*!
    Returns the selected QTimeZone.
*/
QTimeZone QWorldmapDialog::selectedZone() const
{
    return mZone;
}

/*!
    \internal
*/
void QWorldmapDialog::doneIt(int result)
{
    qLog(Time) << __PRETTY_FUNCTION__ << result << mMap->zone().isValid();
    if (mMap->zone().isValid() && result == 1) {
        mZone = mMap->zone();
        result = QDialog::Accepted;
        emit zoneSelected(mZone.id());
    } else {
        result = QDialog::Rejected;
    }
    QDialog::done(result);
}


/*!
    \internal
*/
void QWorldmapDialog::keyPressEvent( QKeyEvent *ke )
{
    qLog(Time) << __PRETTY_FUNCTION__ << ke;
    switch ( ke->key() )
    {
    case Qt::Key_Back:
        if( Qtopia::mousePreferred())
            doneIt(1);
        else
            doneIt(0);
        break;
    case Qt::Key_Select:
        if( !Qtopia::mousePreferred())
            doneIt(1);
        else
            doneIt(0);
        break;
        ke->ignore();

    };
}
