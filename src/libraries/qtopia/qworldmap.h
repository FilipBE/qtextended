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
#ifndef QWORLDMAP_H
#define QWORLDMAP_H

#define QABS(a) ((a) >= 0 ? (a) : -(a))
#define QMAX(a,b)       ((a) > (b) ? (a) : (b))
#define QMIN(a,b)       ((a) < (b) ? (a) : (b))

// Qtopia includes
#include <QTimeZone>

// Qt includes
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QAbstractScrollArea>

// Forward class declarations
class QWorldmapPrivate;

// ============================================================================
//
// QWorldmap
//
// ============================================================================

class QTOPIA_EXPORT QWorldmap : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit QWorldmap( QWidget *parent = 0);
    ~QWorldmap();

    int heightForWidth( int w ) const;
    bool isZoom() const;
    bool isDaylight() const;
    bool isReadOnly() const;
    QTimeZone zone() const;

    virtual bool event(QEvent *event);

public slots:
    void selectNewZone();
    void toggleZoom();
    void select();
    void setDaylight( const bool show );
    void setZone( const QTimeZone& zone );
    void setZone( const QPoint& pos );
    void setReadOnly( const bool readOnly = true );
    void setContinuousSelect(const bool selectMode = false);

signals:
    void selecting();
    void newZone( const QTimeZone& zone );
    void selectZoneCanceled();
    void buttonSelected();

protected:
    virtual void keyPressEvent( QKeyEvent * );
    virtual void keyReleaseEvent( QKeyEvent * );
    virtual void resizeEvent( QResizeEvent *);
    virtual void paintEvent( QPaintEvent *);
    virtual void mouseMoveEvent( QMouseEvent *event );
    virtual void mousePressEvent( QMouseEvent *event );
    virtual void mouseReleaseEvent( QMouseEvent *event );
    virtual void scrollContentsBy( int dx, int dy );

private slots:
    void update( void );
    void redraw( void );
    void initCities();
    void cursorTimeout();
    void cityLabelTimeout();
    void selectCanceled();

private:
    bool selectionMode;
#ifdef TEST_QWORLDMAP
    void testAccess();
    void drawCities( QPainter *p );
#endif
    void updateCursor();
    void setCursorPoint( int ox, int oy, QString city = QString() );
    void showCity( const QTimeZone &city );
    void drawCity( QPainter *p, const QTimeZone &pCity );
    void makeMap( int width, int height );

    void zoneToZoomedWin( int zoneX, int zoneY, int &winX, int &winY );
    void startSelecting();
    void stopSelecting();

    QWorldmapPrivate* d;
    void setCityLabelText();
};

#endif
