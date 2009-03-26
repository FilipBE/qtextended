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

#include "e2_dialer.h"
#include "e2_telephonybar.h"
#include "e2_colors.h"
#include "e2_bar.h"
#include <QPainter>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include "qtopiainputevents.h"
#include "dialercontrol.h"
#include "e2_callscreen.h"

E2Dialer::E2Dialer(E2Button *b, QWidget *parent, Qt::WFlags flags)
: QWidget(parent, flags), m_activeCallCount(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);

    m_history = new E2CallHistory(0);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addSpacing(3);
    layout->addStretch(10);
    layout->addLayout(hlayout, 2);
    layout->addStretch(10);
    E2DialerButton *hist = new E2DialerButton(QPixmap(":image/samples/e2_d_history.png"), false, this);
    QObject::connect(hist, SIGNAL(clicked()), this, SLOT(showHistory()));
    hlayout->addWidget(hist);
    m_lineEdit = new QLineEdit(this);
    QObject::connect(m_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
    QFont f = m_lineEdit->font();
    f.setPointSize(20);
    m_lineEdit->setFont(f);
    m_lineEdit->setFixedHeight(33);
    hlayout->addWidget(m_lineEdit);
    E2DialerButton *back = new E2DialerButton(QPixmap(":image/samples/e2_d_back.png"), false, this);
    QObject::connect(back, SIGNAL(clicked()), this, SLOT(backspace()));
    hlayout->addWidget(back);
    hlayout->addSpacing(3);

    QHBoxLayout *csLayout = new QHBoxLayout;
    csLayout->setSpacing(0);
    csLayout->setMargin(0);
    layout->addLayout(csLayout);

    QVBoxLayout *csvLayout = new QVBoxLayout;
    csvLayout->setSpacing(0);
    csvLayout->setMargin(0);
    csLayout->addLayout(csvLayout);
    csvLayout->addStretch(20);
    m_callscreen = new E2DialerButton(QPixmap(":image/samples/e2_tocallscreen"), false, this);
    csvLayout->addWidget(m_callscreen);
    QObject::connect(m_callscreen, SIGNAL(clicked()),
                     this, SIGNAL(toCallscreen()));


    QVBoxLayout *vgrid = new QVBoxLayout;
    csLayout->addStretch(20);
    csLayout->addLayout(vgrid);
    csLayout->addStretch(20);
    QGridLayout *grid = new QGridLayout;
    m_grid = grid;
    grid->setSpacing(10);
    vgrid->addLayout(grid);
    vgrid->addSpacing(4);

    m_numbers.append(0); // Reserve for 0

    for(int ii = 0; ii < 3; ++ii)  { // Row
        for(int jj = 0; jj < 3; ++jj) { // Column

            int number = ii * 3 + (jj + 1);
            QString pixName(":image/samples/e2_");
            pixName += QString::number(number);

            E2DialerButton *num = new E2DialerButton(QPixmap(pixName),
                                                         true, this);
            QObject::connect(num, SIGNAL(clicked()), this, SLOT(number()));
            m_numbers.append(num);
            grid->addWidget(num, ii, jj);
        }
    }

    E2DialerButton *star = new E2DialerButton(QPixmap(":image/samples/e2_star"), true, this);
    grid->addWidget(star, 3, 0);
    QObject::connect(star, SIGNAL(clicked()), this, SLOT(star()));

    E2DialerButton *zero = new E2DialerButton(QPixmap(":image/samples/e2_0"), true, this);
    grid->addWidget(zero, 3, 1);
    QObject::connect(zero, SIGNAL(clicked()), this, SLOT(number()));
    m_numbers[0] = zero;

    E2DialerButton *hash = new E2DialerButton(QPixmap(":image/samples/e2_hash"), true, this);
    grid->addWidget(hash, 3, 2);
    QObject::connect(hash, SIGNAL(clicked()), this, SLOT(hash()));

    m_button = b;
    QObject::connect(m_button, SIGNAL(clicked()), this, SLOT(callNow()));

    connect(DialerControl::instance(), SIGNAL(activeCount(int)),
            this, SLOT(activeCallCount(int)));

    textChanged();
}

void E2Dialer::showHistory()
{
    m_history->show();
    e2Center(m_history);
}

void E2Dialer::textChanged()
{
    setActive();
}

void E2Dialer::setNumber(const QString &number)
{
    m_lineEdit->setText(number);
}

void E2Dialer::callNow()
{
    if(isVisible()) {
        QString text = m_lineEdit->text();
        Q_ASSERT(!text.isEmpty());
        m_lineEdit->clear();
        emit sendNumber(text);
    }
}

void E2Dialer::backspace()
{
    QtopiaInputEvents::sendKeyEvent(0xFFFF, Qt::Key_Backspace, 0, true, false);
    QtopiaInputEvents::sendKeyEvent(0xFFFF, Qt::Key_Backspace, 0, false, false);
}

void E2Dialer::addDtmf(const QString &s)
{
    Q_ASSERT(m_activeCallCount);

    DialerControl::instance()->activeCalls().first().tone(s);
}

void E2Dialer::number()
{
    QObject *s = sender();
    Q_ASSERT(s);
    for(int ii = 0; ii < m_numbers.count(); ++ii) {
        if(s == m_numbers.at(ii)) {
            QString t = QString::number(ii);
            if(m_activeCallCount)
                addDtmf(t);

            QtopiaInputEvents::sendKeyEvent(t[0].unicode(), Qt::Key_0 + ii, 0, true, false);
            QtopiaInputEvents::sendKeyEvent(t[0].unicode(), Qt::Key_0 + ii, 0, false, false);
            return;
        }
    }

    qFatal("E2Dialer: No number found");
}

void E2Dialer::hash()
{
    if(m_activeCallCount)
        addDtmf("#");

    QtopiaInputEvents::sendKeyEvent(QChar('#').unicode(), Qt::Key_NumberSign, 0, true, false);
    QtopiaInputEvents::sendKeyEvent(QChar('#').unicode(), Qt::Key_NumberSign, 0, false, false);
}

void E2Dialer::star()
{
    if(m_activeCallCount)
        addDtmf("*");

    QtopiaInputEvents::sendKeyEvent(QChar('*').unicode(), Qt::Key_Asterisk, 0, true, false);
    QtopiaInputEvents::sendKeyEvent(QChar('*').unicode(), Qt::Key_Asterisk, 0, false, false);
}

void E2Dialer::setActive()
{
    if(isVisible()) {

        QString text = m_lineEdit->text();
        if(m_activeCallCount) {
            m_button->setText(QString());
            m_button->setEnabled(false);
            m_callscreen->show();
        } else {
            m_callscreen->hide();
            m_button->setText("Call");
            if(text.isEmpty())
                m_button->setEnabled(false);
            else
                m_button->setEnabled(true);
        }

        m_lineEdit->setFocus();
    }
}

void E2Dialer::activeCallCount( int c )
{
    if( m_activeCallCount != c ) {

        if(m_activeCallCount && !c) {
            m_lineEdit->setText(QString());
        }

        m_activeCallCount = c;
        setActive();
    }
}

// define E2DialerButton
E2DialerButton::E2DialerButton(const QPixmap &pix, bool highlight, QWidget *parent)
: QWidget(parent), m_highlight(highlight), m_pressed(false), m_button(pix)
{
    if(highlight)
        setFixedSize(m_button.width() + 2, m_button.height() + 2);
    else
        setFixedSize(m_button.width(), m_button.height());
}

void E2DialerButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if(!m_highlight) {
        p.drawPixmap(0, 0, m_button);
    } else if(m_pressed) {
        p.drawPixmap(2, 2, m_button);
    } else {
        p.drawPixmap(0, 0, m_button);
        p.setPen(REALLY_DARK_GREY);
        p.drawPoint(m_button.width() - 1, m_button.height() - 1);
        p.drawLine(2, m_button.height(), m_button.width(), m_button.height());
        p.drawLine(m_button.width(), 2, m_button.width(), m_button.height());
        p.setPen(GREY);
        p.drawLine(3, m_button.height() + 1, m_button.width() + 1, m_button.height() + 1);
        p.drawLine(m_button.width() + 1, 3, m_button.width() + 1, m_button.height() + 1);
    }
}

void E2DialerButton::mousePressEvent(QMouseEvent *)
{
    m_pressed = true;
    update();
}

void E2DialerButton::mouseReleaseEvent(QMouseEvent *)
{
    m_pressed = false;
    emit clicked();
    update();
}

