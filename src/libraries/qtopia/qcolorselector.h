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

#ifndef QCOLORSELECTOR_H
#define QCOLORSELECTOR_H

#include <qpushbutton.h>
#include <qdialog.h>

#include <qtopiaglobal.h>

class QColorSelectorPrivate;

class QTOPIA_EXPORT QColorSelector : public QWidget
{
    Q_OBJECT
public:
    explicit QColorSelector( QWidget *parent=0, Qt::WindowFlags f=0 );
    ~QColorSelector();

    QColor color() const;
    QSize sizeHint() const;

    void setDefaultColor( const QColor & );
    const QColor &defaultColor() const;

public slots:
    void setColor( const QColor &c );

signals:
    void selected( const QColor &c );

protected:
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void keyPressEvent( QKeyEvent * );
    void showEvent( QShowEvent * );
    QRect rectOfColor( int ) const;

private:
    QColorSelectorPrivate *d;
};

class QColorSelectorDialogPrivate;
class QTOPIA_EXPORT QColorSelectorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QColorSelectorDialog( const QColor &c, QWidget *parent=0, Qt::WindowFlags f = 0 );
    QColorSelectorDialog( QWidget *parent=0, Qt::WindowFlags f = 0 );
    ~QColorSelectorDialog();

    QColor color() const;

    void setDefaultColor( const QColor & );
    const QColor &defaultColor() const;

    static QColor getColor( const QColor &c = Qt::white, QWidget *parent = 0 );

public slots:
    void setColor( const QColor &c );

signals:
    void selected( const QColor &c );

private slots:
    void colorSelected( const QColor &c );

private:
    void init();

private:
    QColorSelectorDialogPrivate *d;
};

class QColorButtonPrivate;

class QTOPIA_EXPORT QColorButton : public QPushButton
{
    Q_OBJECT
public:
    explicit QColorButton( QWidget *parent=0 );
    explicit QColorButton( const QColor &c, QWidget *parent=0 );
    ~QColorButton();

    QColor color() const;

    void setDefaultColor( const QColor & );
    const QColor &defaultColor() const;

public slots:
    void setColor( const QColor &c );

signals:
    void selected( const QColor &c );

private slots:
    void colorSelected( const QColor & );
    void showSelector();

protected:
    void paintEvent( QPaintEvent *e );
    void drawButtonLabel( QPainter * );

private:
    void init();

private:
    QColorButtonPrivate *d;
};

#endif
