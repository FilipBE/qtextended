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

#ifndef e1_DIALER_H
#define e1_DIALER_H

#include <QWidget>
#include <QPixmap>
#include <QColor>
#include <QString>

class E1Button;
class E1Bar;
class QResizeEvent;
class QLineEdit;
class QMoveEvent;
class E1Dialer : public QWidget
{
    Q_OBJECT
public:

    E1Dialer( E1Button *, QWidget* parent = 0, Qt::WFlags f = 0 );

    void setActive();

    void setNumber(const QString &);
signals:
    void sendNumber(const QString &);
    void toCallScreen();

protected slots:
    void historyClicked();
    void eraseClicked();
    void activeCallCount(int);
    void selectCallHistory();
    void buttonClicked( const QString& txt );
    void addPlus();
    void addPause();
    void addWait();
    void numberChanged(const QString&);
    void textButtonClicked();

protected:
    void resizeEvent( QResizeEvent* );

private:
    QLineEdit* m_input;
    E1Button* m_textButton;
    E1Bar* m_bar;
    E1Bar* m_callscreenBar;
    int m_activeCallCount;
};

// declare E1DialerButton
class E1DialerButton : public QWidget
{
    Q_OBJECT
public:
    enum State
    {
        Up,
        Down
    };

    E1DialerButton( QWidget* parent, Qt::WindowFlags f = 0 );

    void setBackgroundEnabled( bool e );
    void setText( const QString& txt );
    void setFgPixmap( const QPixmap& );
    //void setColor( const E1DialerButton::State& st, const QColor& color );

signals:
    void clicked( const QString& ch );

protected:
    void paintEvent( QPaintEvent* e );
    void mousePressEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );
    void resizeEvent( QResizeEvent* e );
    void moveEvent( QMoveEvent* e );

private:
    void generateBgPixmap( const E1DialerButton::State& st );

    E1DialerButton::State m_state;
    QPixmap m_fgPixmap;
    QImage m_bgImage;
    QString m_text;
    QPixmap m_bgForState[2];
    QColor m_colorForState[2];
    bool m_backgroundEnabled;
};


#endif
