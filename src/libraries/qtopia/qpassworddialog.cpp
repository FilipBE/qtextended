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

#include "qpassworddialog.h"
#include <qtopiaapplication.h>
#include <QSettings>
#include <QApplication>
#include <QPushButton>
#include <QDesktopWidget>
#include <QEvent>
#include <QKeyEvent>

#include <md5hash.h>
#include <qsoftmenubar.h>
#include <QLayout>
#include <unistd.h> //for sleep
#include "ui_passwordbase_p.h"

static int execDialog( QDialog *dialog, bool nomax )
{
    return QtopiaApplication::execDialog( dialog, nomax );
}

static int execDialog( QDialog *dialog )
{
    return execDialog( dialog, false ); // nomax defaults to false in QtopiaApp
}


class QPasswordWidget : public QWidget, public Ui::PasswordBase
{
    Q_OBJECT

public:
    QPasswordWidget( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QPasswordWidget();

    void reset();
    void setPrompt( const QString& );
    QString password() const;
    void resetLabels( bool useOkLabel );

signals:
    void passwordEntered( const QString& );

public slots:
    void key();

protected:
    void keyPressEvent( QKeyEvent * );

private:
    void input( const QString & );
    friend class QPasswordDialog;
    QPasswordDialog::InputMode mode;
};

QPasswordWidget::QPasswordWidget( QWidget* parent, Qt::WFlags fl )
    : QWidget( parent, fl )
{
    setupUi(this);

    /*
    The button layout and softkeys differ depending on whether
    Qtopia::mousePreferred() returns true or false.

    if mouse is preferred:
        - all buttons are displayed
        - Key_Back is OK/Next, and Key_Select does nothing
    otherwise:
        - all buttons are hidden
        - Key_Back means 'backspace', and Key_Select is OK/Next
    */

    if  (Qtopia::mousePreferred()) {
        button_backspace->setIcon(QIcon(":icon/i18n/backspace"));
        connect(button_0,SIGNAL(clicked()),this,SLOT(key()));
        connect(button_1,SIGNAL(clicked()),this,SLOT(key()));
        connect(button_2,SIGNAL(clicked()),this,SLOT(key()));
        connect(button_3,SIGNAL(clicked()),this,SLOT(key()));
        connect(button_4,SIGNAL(clicked()),this,SLOT(key()));
        connect(button_5,SIGNAL(clicked()),this,SLOT(key()));
        connect(button_6,SIGNAL(clicked()),this,SLOT(key()));
        connect(button_7,SIGNAL(clicked()),this,SLOT(key()));
        connect(button_8,SIGNAL(clicked()),this,SLOT(key()));
        connect(button_9,SIGNAL(clicked()),this,SLOT(key()));
        connect(button_backspace,SIGNAL(clicked()),this,SLOT(key()));
    } else {
        button_0->hide();
        button_1->hide();
        button_2->hide();
        button_3->hide();
        button_4->hide();
        button_5->hide();
        button_6->hide();
        button_7->hide();
        button_8->hide();
        button_9->hide();
        button_backspace->hide();
    }

    resetLabels(true);

    QPalette pal = display->palette();
    QBrush base = pal.brush(QPalette::Normal, QPalette::Base);
    QColor text = pal.color(QPalette::Normal, QPalette::Text);
    pal.setBrush(QPalette::Disabled, QPalette::Background, base);
    pal.setColor(QPalette::Disabled, QPalette::Text, text);
    display->setPalette(pal);

    display->setMaxLength(8);

    setFocusPolicy(Qt::StrongFocus);

    reset();

    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this, false );
}

/*
    Destroys the object and frees any allocated resources
*/
QPasswordWidget::~QPasswordWidget()
{
    // no need to delete child widgets, Qt does it all for us
}

/*!
    \reimp
*/

void QPasswordWidget::key()
{
    QPushButton* s = (QPushButton*)sender();
    if ( s == button_backspace )
        display->backspace();
    else
        input(s->text());
}

/*!
    \reimp
*/

void QPasswordWidget::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Qt::Key_Back ) {
        if ( Qtopia::mousePreferred() )
            emit passwordEntered( display->text() );
        else
            display->backspace();

    } else if (  e->key() == Qt::Key_Select ) {
        if (!Qtopia::mousePreferred())
            emit passwordEntered( display->text() );
        // ignore Key_Select if mouse preferred

    } else if ( e->key() == Qt::Key_NumberSign && mode == QPasswordDialog::Pin ) {
        // Key_NumberSign (#), is required for GCF compliance.
        // GSM 02.30, section 4.6.1, Entry of PIN and PIN2.
        emit passwordEntered( display->text() );

    } else {
        QString t = e->text().left(1);
        if ( t[0]>='0' && t[0]<='9' ) {
            input(t);
        }

        QWidget::keyPressEvent( e );
    }
}

void QPasswordWidget::input( const QString &c )
{
    display->setText( display->text() + c );
}

void QPasswordWidget::setPrompt( const QString& s )
{
    prompt->setText( s );
}

void QPasswordWidget::reset()
{
    display->clear();
}

QString QPasswordWidget::password() const
{
    if (display->text().isEmpty())
        return "";
    return ( mode == QPasswordDialog::Crypted ?
            MD5::hash(display->text()) : display->text() );
}

void QPasswordWidget::resetLabels( bool useOkLabel )
{
    QSoftMenuBar::StandardLabel okLabel =
            ( useOkLabel ? QSoftMenuBar::Ok : QSoftMenuBar::Next);

    if (Qtopia::mousePreferred()) {
        QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
        QSoftMenuBar::setLabel(this, Qt::Key_Back, okLabel);
    } else {
        QSoftMenuBar::setLabel(this, Qt::Key_Select, okLabel);
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
    }
}


/*!
    \class QPasswordDialog
    \inpublicgroup QtBaseModule


    \brief The QPasswordDialog class provides a dialog widget for entering a PIN code.
*/

/*!
    \enum QPasswordDialog::InputMode

    This enum describes the various input modes. The input mode for a dialog
    is used to specify how the password is returned (encrypted or plain text),
    as well as other UI differences if the dialog is shown using \l {QPasswordDialog::}{getPassword()}.

    \value Crypted  Password will be returned one-way encrypted (using MD5). If the dialog is shown
                    using \l {QPasswordDialog::}{getPassword()}, it will be maximized.
    \value Plain  Password will be returned as plain text. If the dialog is shown
                    using \l {QPasswordDialog::}{getPassword()}, it will \bold not be maximized.
    \value Pin  Same as Plain, but '#' can also be used to accept the dialog.
*/

/*!
    Constructs a new QPasswordDialog with the given \a parent and widget \a flags.

    The input mode for the dialog is set by default to \l {Crypted}.
*/
QPasswordDialog::QPasswordDialog( QWidget* parent, Qt::WFlags flags)
    : QDialog( parent, flags )
{
    m_passw = new QPasswordWidget( this );
    QBoxLayout *l = new QVBoxLayout( this );
    l->addWidget( m_passw );
    setFocusProxy( m_passw );

    // defaults
    m_passw->mode = QPasswordDialog::Crypted;
    setWindowTitle(tr("Authentication"));

    connect( m_passw, SIGNAL(passwordEntered(QString)),
             this, SLOT(accept()) );
}

/*!
    Destroys the QPasswordDialog.
*/
QPasswordDialog::~QPasswordDialog()
{
}

/*!
    Sets the \a prompt to be displayed by the dialog.
*/
void QPasswordDialog::setPrompt(const QString& prompt)
{
    m_passw->setPrompt( prompt );
}

/*!
    Returns the prompt used by the dialog.
*/
QString QPasswordDialog::prompt() const
{
    return m_passw->prompt->text();
}

/*!
    Sets the input mode for the dialog to \a mode.
*/
void QPasswordDialog::setInputMode( QPasswordDialog::InputMode mode )
{
    m_passw->mode = mode;
}

/*!
    Returns the input mode used by the dialog.
*/
QPasswordDialog::InputMode QPasswordDialog::inputMode() const
{
    return m_passw->mode;
}

/*!
    Resets any password previously entered in the dialog.
*/
void QPasswordDialog::reset()
{
    m_passw->reset();
}

/*!
    Returns the password entered in the dialog.

    If the input mode is \l {Crypted}, the password will be returned one-way encrypted (using MD5);
    otherwise it will be returned as plain text.
*/
QString QPasswordDialog::password() const
{
    return m_passw->password();
}


/*!
    Creates and displays a password dialog with the given \a parent and \a prompt; returns the entered password.

    \a mode specifies whether the returned password is one-way encrypted (using MD5) or
    plain text. It also determines whether or not the dialog will be maximized.
    If the operation is required to accept more than one password -- for example,
    if accepting an old password and a new password -- set \a last to false to set the context label to 'Next'.

    The returned value is a null string if the user cancels the operation,
    or an empty string if the user enters no password (but confirms the
    dialog).
*/
QString QPasswordDialog::getPassword( QWidget* parent,
                                      const QString& prompt,
                                      InputMode mode,
                                      bool last )
{
    bool max = true;

    if ( mode == Plain || mode == Pin )
        max = false;

    QPasswordDialog pd( parent );
    pd.m_passw->resetLabels( last );
    pd.setPrompt( prompt );
    pd.setInputMode( mode );

    int r;
    if ( max )
        pd.showMaximized();
    r = execDialog( &pd, !max );

    if ( r == QDialog::Accepted ) {
        return pd.password();
    } else {
        return QString();
    }
}


/*!
    \obsolete
    Prompt, fullscreen, for the user's passcode until they get it right.

    If \a atPowerOn is true, the dialog is only used if the user's
    preference request it at poweron.
*/
void QPasswordDialog::authenticateUser( QWidget* parent, bool atPowerOn )
{
    QSettings cfg( "Trolltech", "Security" );
    cfg.beginGroup( "Passcode" );
    QString passcode = cfg.value( "passcode" ).toString();
    if ( !passcode.isEmpty()
        && (!atPowerOn || cfg.value( "passcode_poweron", 0 ).toInt()) ) {
        // Do it as a fullscreen modal dialog
        QPasswordDialog pd( parent );
        pd.setInputMode( QPasswordDialog::Crypted );
        pd.setWindowFlags( Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint );

        if ( Qtopia::mousePreferred() ) {
            QRect desk = qApp->desktop()->geometry();
            pd.setGeometry( 0, 0, desk.width(), desk.height() );
        }

        execDialog( &pd );
        while ( pd.password() != passcode ) {
            pd.reset();
            execDialog( &pd );
        }
    } else if ( atPowerOn ) {
    // refresh screen   #### should probably be in caller
    // Not needed (we took away the screen blacking)
    //if ( qwsServer )
        //qwsServer->refresh();
    }
}

/*!
    \obsolete
    Returns true if \a text matches the user's passcode; otherwise returns false.
*/
bool QPasswordDialog::authenticateUser(const QString &text)
{
    QSettings cfg("Trolltech","Security");
    cfg.beginGroup("Passcode");
    QString passcode = cfg.value("passcode").toString();
    return (passcode.isEmpty() || MD5::hash(text) == passcode);
}

#include "qpassworddialog.moc"
