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

#ifndef PENSETTINGSWIDGET_H
#define PENSETTINGSWIDGET_H

#include <qwidget.h>
#include <qlist.h>
#include <char.h>

class QIMPenSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    QIMPenSettingsWidget( QWidget *parent, const char *name = 0 );
    ~QIMPenSettingsWidget();

    void clear();
    void greyStroke();
    void setReadOnly( bool r ) { readOnly = r; }

    void insertCharSet( QIMPenCharSet *cs, int stretch=1, int pos=-1 );
    void removeCharSet( int );
    void changeCharSet( QIMPenCharSet *cs, int pos );
    void clearCharSets();
    void showCharacter( QIMPenChar *, int speed = 10 );
    virtual QSize sizeHint() const;

public slots:
    void removeStroke();

signals:
    void changeCharSet( QIMPenCharSet *cs );
    void changeCharSet( int );
    void beginStroke();
    void stroke( QIMPenStroke *ch );

protected slots:
    void timeout();

protected:
    enum Mode { Waiting, Input, Output };
    bool selectSet( QPoint );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void paintEvent( QPaintEvent *e );
    virtual void resizeEvent( QResizeEvent *e );

    struct CharSetEntry {
        QIMPenCharSet *cs;
        int stretch;
    };
    typedef QList<CharSetEntry *> CharSetEntryList;
    typedef QList<CharSetEntry *>::iterator CharSetEntryIterator;
    typedef QList<CharSetEntry *>::const_iterator CharSetEntryConstIterator;

protected:
    Mode mode;
    bool autoHide;
    bool readOnly;
    QPoint lastPoint;
    int pointIndex;
    int strokeIndex;
    int currCharSet;
    QTimer *timer;
    QColor strokeColor;
    QRect dirtyRect;
    QIMPenChar *outputChar;
    QIMPenStroke *outputStroke;
    QIMPenStroke *inputStroke;
    QIMPenStrokeList strokes;
    CharSetEntryList charSets;
    int totalStretch;
    QList<QRect> penMoves;
};

#endif
