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

#ifndef SINGLEVIEW_P_H
#define SINGLEVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qcontent.h>
#include <qcontentset.h>
#include <qcategorymanager.h>

#include <qwidget.h>
#include <qpixmap.h>
#include <qpoint.h>

#include <QAbstractListModel>
#include <QItemSelectionModel>

class SingleView : public QWidget
{
    Q_OBJECT
public:
    explicit SingleView( QWidget* parent = 0, Qt::WFlags f = 0 );
    virtual ~SingleView(){}

    void setModel( QAbstractListModel* );

    QAbstractListModel* model() const;

    void setSelectionModel( QItemSelectionModel* );

    QItemSelectionModel* selectionModel() const;

signals:
    // Select key pressed
    void selected();

    // Stylus held
    void held( const QPoint& );

protected slots:
    // Update current selection
    void currentChanged( const QModelIndex&, const QModelIndex& );

protected:
    // Draw scaled image onto widget
    void paintEvent( QPaintEvent* );

    // Update current selection
    void keyPressEvent( QKeyEvent* );

    // Emit held signal
    void mousePressEvent( QMouseEvent* );

    virtual QPixmap loadThumbnail( const QString &filename, const QSize &size );

private slots:

    void contentChanged(const QContentIdList&,const QContent::ChangeType);

private:
    // Move selection forward one image
    void moveForward();

    // Mode selection back one image
    void moveBack();

    bool right_pressed;

    QString current_file;
    QPixmap buffer;
    QAbstractListModel *model_;
    QItemSelectionModel *selection_;
};

#endif
