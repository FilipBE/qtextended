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

#include "e1_header.h"
#include "e1_dialog.h"
#include "e1_callhistory.h"
#include "themecontrol.h"
#include "dialercontrol.h"
#include "windowmanagement.h"
#include <QSize>
#include <QPushButton>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <QTimer>
#include <QListWidget>

// declare E1HeaderButton
class E1HeaderButton : public QWidget
{
Q_OBJECT
public:
    E1HeaderButton(const QPixmap &icon,
                    const QString &name,
                    QWidget *parent);

    virtual QSize sizeHint() const;

signals:
    void clicked(const QString &);

protected:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void paintEvent(QPaintEvent *);

private:
    void paintCenter(QPainter *, const QPixmap &);
    QPixmap m_button;
    QPixmap m_tint;

    QPixmap m_icon;
    QString m_name;
    bool m_depressed;
};

// declare E1HeaderAlertButton
class E1HeaderAlertButton : public QWidget
{
    Q_OBJECT
public:
    E1HeaderAlertButton( QWidget* parent );
    virtual QSize sizeHint() const;

signals:
    void clicked();

protected slots:
    void nextFrame();

protected:
    virtual void paintEvent( QPaintEvent* );
    virtual void showEvent( QShowEvent* );
    virtual void hideEvent( QHideEvent* );
    virtual void mouseReleaseEvent(QMouseEvent *);
private:
    QTimer* m_animationTimer;
    QPixmap m_pixmap1;
    QPixmap m_pixmap2;
    QPixmap m_icon;
    bool m_flag;
};


// define E1HeaderButton
E1HeaderButton::E1HeaderButton(const QPixmap &icon,
                                 const QString &name,
                                 QWidget *parent)
: QWidget(parent), m_button(":image/samples/e1_header_button"),
  m_tint(":image/samples/e1_header_button_tint"), m_icon(icon), m_name(name),
  m_depressed(false)
{
    QPalette mpal = palette();
    mpal.setBrush(QPalette::Window, QBrush(QColor(0,0,0,0)));
    setPalette(mpal);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}

QSize E1HeaderButton::sizeHint() const
{
    int width = qMax(m_button.size().width(), m_tint.size().width());
    int height = qMax(m_button.size().height(), m_tint.size().height());

    return QSize(width, height);
}

void E1HeaderButton::mousePressEvent(QMouseEvent *)
{
    m_depressed = true;
    update();
    emit clicked(m_name);
}

void E1HeaderButton::mouseReleaseEvent(QMouseEvent *)
{
    m_depressed = false;
    update();
}

void E1HeaderButton::paintCenter(QPainter *painter, const QPixmap &pix)
{
    painter->drawPixmap((width() - pix.width()) / 2,
                        (height() - pix.height()) / 2,
                        pix.width(), pix.height(),
                        pix);
}

void E1HeaderButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    paintCenter(&painter, m_button);
    if(m_depressed)
        paintCenter(&painter, m_tint);
    paintCenter(&painter, m_icon);
}

// define E1Header
E1Header::E1Header(QWidget *parent, Qt::WFlags wflags)
: QWidget(parent, wflags), m_missedCallsVS("/Communications/Calls/MissedCalls"), m_newMessagesVS("/Communications/Messages/NewMessages")
{
    QHBoxLayout * layout = new QHBoxLayout(this);
    setLayout(layout);

    E1HeaderButton * button = 0;

    button = new E1HeaderButton(QIcon(":icon/home").pixmap(20, 20), "home", this);
    connect(button, SIGNAL(clicked(QString)), this, SLOT(clicked(QString)));
    layout->addWidget(button);

    button = new E1HeaderButton(QIcon(":icon/addressbook/AddressBook").pixmap(20, 20), "contacts", this);
    connect(button, SIGNAL(clicked(QString)), this, SLOT(clicked(QString)));
    layout->addWidget(button);

    m_alertButton = new E1HeaderAlertButton(this);
    m_alertButton->hide();
    connect(m_alertButton, SIGNAL(clicked()), this, SLOT(alertClicked()));
    layout->addWidget(m_alertButton);

    button = new E1HeaderButton(QIcon(":icon/email").pixmap(20, 20), "email", this);
    connect(button, SIGNAL(clicked(QString)), this, SLOT(clicked(QString)));
    layout->addWidget(button);

    button = new E1HeaderButton(QIcon(":icon/phone/calls").pixmap(20, 20), "dialer", this);
    connect(button, SIGNAL(clicked(QString)), this, SLOT(clicked(QString)));
    layout->addWidget(button);

    WindowManagement::dockWindow(this, WindowManagement::Top,
                                             sizeHint());
    WindowManagement::protectWindow(this);

    connect( &m_missedCallsVS, SIGNAL(contentsChanged()), this, SLOT(missedCallsChanged()) );
    missedCallsChanged();
    connect( &m_newMessagesVS, SIGNAL(contentsChanged()), this, SLOT(newMessagesChanged()) );
    newMessagesChanged();
}

void E1Header::clicked(const QString &name)
{
    if("home" == name) {
        QtopiaIpcEnvelope env("QPE/E1", "showHome()");
    } else if("contacts" == name) {
        QtopiaIpcEnvelope env("QPE/Application/addressbook", "raise()");
    } else if("email" == name) {
        QtopiaIpcEnvelope env("QPE/Application/qtmail", "raise()");
    } else if("dialer" == name) {
        QtopiaIpcEnvelope env("QPE/E1", "showTelephony()");
    }
}

void E1Header::alertClicked()
{
    // what's ontop of the stack?
    E1Dialog* dialog = new E1Dialog( 0,E1Dialog::Return);
    QListWidget* contentsWidget = new QListWidget( this );
    connect( this, SIGNAL(itemActivated(QListWidgetItem*)), contentsWidget, SLOT(accept()) );
    connect( contentsWidget, SIGNAL(itemClicked(QListWidgetItem*)), dialog, SLOT(accept()) );
    dialog->setContentsWidget( contentsWidget );

    QListWidgetItem* missedItem = 0;
    QListWidgetItem* messagesItem = 0;
    if( m_alertStack.contains( "MissedCalls" ) ) {
        // start call history in 'missed' mode
        missedItem = new QListWidgetItem( QString::number(m_missedCallsVS.value().toInt()) + " Missed Call(s)", contentsWidget );
    }
    if( m_alertStack.contains( "NewMessages" ) ) {
        messagesItem = new QListWidgetItem( QString::number(m_newMessagesVS.value().toInt()) + " New Message(s)", contentsWidget );
        // display messages application list
        // QtopiaIpcEnvelope env("QPE/Application/qtmail", "raise()");
    }
//    dialog->setGeometry( QApplication::desktop()->availableGeometry() );
    if( dialog->exec() == QDialog::Accepted ) {
        QListWidgetItem* curItem = contentsWidget->item( contentsWidget->currentRow() );
        if( curItem == missedItem ) {
            // show the missed calls screen
            DialerControl::instance()->resetMissedCalls();
            E1Dialog* chdialog = new E1Dialog(0, E1Dialog::Return);
            E1CallHistoryList* list = new E1CallHistoryList( chdialog, E1CallHistory::Missed );
            chdialog->setContentsWidget( list );
            QObject::connect(list, SIGNAL(closeMe()), chdialog, SLOT(reject()));
            chdialog->exec();
            delete chdialog;
        } else if( curItem == messagesItem ) {
            QtopiaServiceRequest req("SMS", "viewSms()");
            req.send();
        }
    }
    delete dialog;
}

void E1Header::setAlertEnabled( bool e )
{
    if( e ) {
        m_alertButton->show();

    } else {
        m_alertButton->hide();
    }
}

void E1Header::missedCallsChanged()
{
    if( m_missedCallsVS.value().toInt() != 0 ) {
        if( !m_alertStack.contains( "MissedCalls" ) ) {
            m_alertStack.append( "MissedCalls" );
            setAlertEnabled( true );
        }
    } else {
        m_alertStack.removeAll( "MissedCalls" );
        if( m_alertStack.isEmpty() )
            setAlertEnabled( false );
    }
}


void E1Header::newMessagesChanged()
{
    if( m_newMessagesVS.value().toInt() != 0 ) {
        if( !m_alertStack.contains( "NewMessages" ) ) {
            m_alertStack.append( "NewMessages" );
            setAlertEnabled( true );
        }
    } else {
        m_alertStack.removeAll( "NewMessages" );
        if( m_alertStack.isEmpty() )
            setAlertEnabled( false );
    }
}

QSize E1Header::sizeHint() const
{
    QSize rv = QWidget::sizeHint();
    rv.setHeight(53);
    return rv;
}

void E1Header::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(0, 0, width(), height(), QColor(50, 50, 50));
    p.setPen(Qt::black);
}

// define E1HeaderAlertButton
E1HeaderAlertButton::E1HeaderAlertButton( QWidget* parent )
    : QWidget( parent ), m_pixmap1(":image/samples/alert_frame1"), m_pixmap2(":image/samples/alert_frame2"), m_icon(":image/samples/alert_icon"), m_flag(false)
{
    m_animationTimer = new QTimer( this );
    m_animationTimer->setInterval( 250 );
    connect( m_animationTimer, SIGNAL(timeout()), this, SLOT(nextFrame()) );
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QSize E1HeaderAlertButton::sizeHint() const
{
    int width = qMax(m_pixmap1.size().width(), m_pixmap2.size().width());
    int height = qMax(m_pixmap1.size().height(), m_pixmap2.size().height());
    return QSize(width, height);
}

void E1HeaderAlertButton::showEvent( QShowEvent* e )
{
    m_animationTimer->start();
    QWidget::showEvent( e );
}

void E1HeaderAlertButton::hideEvent( QHideEvent* e )
{
    m_animationTimer->stop();
    QWidget::hideEvent( e );
}

void E1HeaderAlertButton::nextFrame()
{
    m_flag = !m_flag;
    repaint();
}

void E1HeaderAlertButton::paintEvent( QPaintEvent* )
{
    QPixmap* pix;
    if( m_flag )
        pix = &m_pixmap1;
    else
        pix = &m_pixmap2;
    QPainter p( this );
    p.drawPixmap( rect(), *pix );
    p.drawPixmap( rect(), m_icon );
}

void E1HeaderAlertButton::mouseReleaseEvent(QMouseEvent *e )
{
    QWidget::mouseReleaseEvent( e );
    emit clicked();
}

#include "e1_header.moc"
