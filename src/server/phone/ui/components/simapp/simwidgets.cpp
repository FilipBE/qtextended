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

#include "simwidgets.h"
#include "simicons.h"
#include <QtopiaApplication>
#include <QSimIconReader>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QTextBrowser>
#include <QListWidget>
#include <QValidator>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QSoftMenuBar>
#include <QtopiaItemDelegate>
#include <QTimer>
#include <QPhoneCallManager>
#include <qtopiaservices.h>
#include <QMessageBox>
#include <QMenu>
#include <QPalette>
#include <QColor>

#ifdef MEDIA_SERVER
#include <QSoundControl>
#endif
#include <QSound>

#ifndef QTOPIA_TEST
#include "applicationmonitor.h"
#endif

#define AUTOCLEARTIMEOUT 3000
#define NORESPONSETIMEOUT 120000
#define MAXREDIALCOUNT 2

class QSimCommand;

class SimListBoxItem : public QListWidgetItem
{
public:
    SimListBoxItem(QListWidget *listbox, const QString &text,
                   uint id, bool hlp, uint icon, bool iconse)
        : QListWidgetItem(text, listbox), identifier(id),
          hasHelp(hlp), iconId(icon), iconSelfExplanatory(iconse)
    {
    }

    uint identifier;
    bool hasHelp;
    uint iconId;
    bool iconSelfExplanatory;

    void changeIcon( const QIcon& icon )
    {
        iconId = 0;     // Don't need to change it again.
        setIcon( icon );
        if ( iconSelfExplanatory )
            setText( "" );      // Don't need the text any more.
    }
};

//===========================================================================

SimCommandView::SimCommandView(const QSimCommand &cmd, QWidget *parent)
    : QFrame(parent), m_command(cmd), tid(0), icons(0)
{
    if (cmd.hasHelp())
        QSoftMenuBar::setLabel( this, Qt::Key_Context1, "help", tr( "Help" ) );

    setNoResponseTimeout(NORESPONSETIMEOUT);
}

SimCommandView::SimCommandView(const QSimCommand &cmd, QSimIconReader *reader, QWidget *parent)
    : QFrame(parent), m_command(cmd), tid(0), icons(0)
{
    if (cmd.hasHelp())
        QSoftMenuBar::setLabel( this, Qt::Key_Context1, "help", tr( "Help" ) );

    setNoResponseTimeout(NORESPONSETIMEOUT);

    if ( reader && ( (int)cmd.iconId() || (int)cmd.otherIconId() ) ) {
        icons = new SimIcons( reader, this );
        connect( icons, SIGNAL(iconsReady()), this, SLOT(iconsReady()) );
        if ( (int)cmd.iconId() )
            icons->needIcon( (int)cmd.iconId() );
        if ( (int)cmd.otherIconId() )
            icons->needIcon( (int)cmd.otherIconId() );
        icons->requestIcons();
    }
}

SimCommandView::~SimCommandView()
{
}

void SimCommandView::setNoResponseTimeout( int msecs )
{
    if ( tid )
        killTimer(tid);
    tid = startTimer(msecs);
}

uint SimCommandView::helpIdentifier() const
{
    return 0;
}

//===========================================================================

// QLabel ignores the <body bgcolor="foo"> directive, so we need to
// adjust the background color by hand when text attributes are used.
static void adjustLabelBackground(QLabel *label, const QByteArray& textAttribute)
{
    static const char * const colors[16] = {
        "#000000",      // Black
        "#808080",      // Dark Grey
        "#800000",      // Dark Red
        "#808000",      // Dark Yellow
        "#008000",      // Dark Green
        "#008080",      // Dark Cyan
        "#000080",      // Dark Blue
        "#800080",      // Dark Magenta
        "#C0C0C0",      // Grey
        "#FFFFFF",      // White
        "#FF0000",      // Bright Red
        "#FFFF00",      // Bright Yellow
        "#00FF00",      // Bright Green
        "#00FFFF",      // Bright Cyan
        "#0000FF",      // Bright Blue
        "#FF00FF"       // Bright Magenta
    };
    if (textAttribute.size() < 4)
        return;
    int back = ((textAttribute[3] & 0xF0) >> 4);
    QColor color(QString::fromLatin1(colors[back]));
    QPalette pal = label->palette();
    pal.setColor(label->backgroundRole(), color);
    label->setPalette(pal);
    label->setAutoFillBackground(true);
}

SimMenu::SimMenu(const QSimCommand &cmd, QWidget *parent)
    : SimCommandView(cmd, parent), title(0)
{
    init();
}

SimMenu::SimMenu(const QSimCommand &cmd, QSimIconReader *reader, QWidget *parent)
    : SimCommandView(cmd, reader, parent), title(0)
{
    init( reader );
}

void SimMenu::init( QSimIconReader *reader )
{
    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin(0);

    // menu title
    if (!m_command.title().isEmpty()) {
        title = new QLabel(m_command.title(), this);
        QFont f = font();
        f.setWeight(QFont::Bold);
        title->setFont(f);
        title->setAlignment(Qt::AlignCenter);
        vb->addWidget(title);
    }

    menu = new QListWidget(this);
    menu->setFrameStyle(QFrame::NoFrame);
    menu->setFocusPolicy(Qt::StrongFocus);
    menu->setItemDelegate(new QtopiaItemDelegate);

    // to send BackwardMove response
    if (m_command.type() == QSimCommand::SelectItem)
        menu->installEventFilter(this);

    vb->addWidget(menu);

    populate(reader);

    connect(menu, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(itemSelected(QListWidgetItem*)));
    connect(menu, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(currentItemChanged(QListWidgetItem*,QListWidgetItem*)));

    // need to make request here
    // to make suer subclass iconsReady slot to be used
    if ( icons )
        icons->requestIcons();
}

void SimMenu::populate(QSimIconReader *reader)
{
    bool selected = false;
    QList<QSimMenuItem> items = m_command.menuItems();
    QList<QSimMenuItem>::Iterator it;
    for (it = items.begin(); it != items.end(); ++it) {
        SimListBoxItem *item =
            new SimListBoxItem(menu, (*it).label(), (*it).identifier(), (*it).hasHelp(), (*it).iconId(), (*it).iconSelfExplanatory());
        if ( (int)(item->iconId) ) {
            if ( !icons && reader ) {
                icons = new SimIcons( reader, this );
                connect( icons, SIGNAL(iconsReady()), this, SLOT(iconsReady()) );
            }
            icons->needIcon( (int)(item->iconId) );
        }
        if (m_command.type() == QSimCommand::SelectItem
            && (*it).identifier() == m_command.defaultItem()) {
            menu->setCurrentItem(item);
            selected = true;
        }

        if ((*it).nextAction()) {
            // indicate next action
        }
    }
    if (!selected)
        menu->setCurrentItem(menu->item(0));

    menu->setEditFocus(true);
}

void SimMenu::currentItemChanged(QListWidgetItem *cur, QListWidgetItem *)
{
    SimListBoxItem *si = (SimListBoxItem *)cur;
    if (si->hasHelp)
        QSoftMenuBar::setLabel( this, Qt::Key_Context1, "help", tr( "Help" ) );
    else
        QSoftMenuBar::setLabel( this, Qt::Key_Context1, QSoftMenuBar::NoLabel );
}

void SimMenu::itemSelected(QListWidgetItem *item)
{
    SimListBoxItem *si = (SimListBoxItem *)item;
    if (m_command.type() == QSimCommand::SetupMenu) {
        QSimEnvelope env;
        env.setType(QSimEnvelope::MenuSelection);
        env.setSourceDevice(QSimCommand::Keypad);
        env.setMenuItem(si->identifier);
        emit sendEnvelope(env);
    } else {
        QSimTerminalResponse resp;
        resp.setCommand(m_command);
        resp.setMenuItem(si->identifier);
        emit sendResponse(resp);
    }
}

void SimMenu::iconsReady()
{
    // set menu title icon if any
    if ( m_command.iconId() != 0 ) {
        QIcon icon = icons->icon( m_command.iconId() );
        if ( !icon.isNull() ) {
            title->setPixmap(icon.pixmap(QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)));
        }
    }
    // set menu item icons
    for ( int index = 0; index < menu->count(); ++index ) {
        SimListBoxItem *si = (SimListBoxItem *)menu->item( index );
        if ( !si->iconId )
            continue;
        QIcon icon = icons->icon( (int)(si->iconId) );
        if ( !icon.isNull() )
            si->changeIcon( icon );
    }
    if ( m_command.type() == QSimCommand::SelectItem )
        setNoResponseTimeout(NORESPONSETIMEOUT);
}

void SimMenu::timerEvent(QTimerEvent *)
{
    killTimer(tid);
    tid = 0;

    QSimTerminalResponse resp;
    resp.setCommand(m_command);
    resp.setResult(QSimTerminalResponse::NoResponseFromUser);
    emit sendResponse(resp);
}

uint SimMenu::helpIdentifier() const
{
    SimListBoxItem *si = (SimListBoxItem *)menu->currentItem();
    if (si)
        return si->identifier;
    else
        return 0;
}

bool SimMenu::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        // for SelectItem command, BackwardMove result sent
        if (ke->key() == Qt::Key_Back || ke->key() == Qt::Key_No) {
            QSimTerminalResponse resp;
            resp.setCommand(m_command);
            resp.setResult(QSimTerminalResponse::BackwardMove);
            emit sendResponse(resp);
            return true;
        }
    }
    return false;
}


//===========================================================================

SimText::SimText(const QSimCommand &simCmd, QSimIconReader *reader, QWidget *parent)
    : SimCommandView(simCmd, reader, parent)
{
    browser = new QTextBrowser( this );

    browser->setFocusPolicy(Qt::StrongFocus);
    browser->setFrameStyle(NoFrame);
    browser->installEventFilter(this);

    updateSoftMenuBar();

    if ( (int)simCmd.iconId() ) {
        icons->needIconInFile( (int)simCmd.iconId() );
        icons->requestIcons();
    } else {
        write();
    }
}

void SimText::updateSoftMenuBar()
{
    QSoftMenuBar::setLabel( browser, Qt::Key_Context1, QSoftMenuBar::NoLabel );
    if ( m_command.clearAfterDelay() )
        QSoftMenuBar::setLabel( browser, Qt::Key_Select, QSoftMenuBar::NoLabel );
    else
        QSoftMenuBar::setLabel( browser, Qt::Key_Select, "", tr( "OK" ) );
}

void SimText::setCommand(const QSimCommand &simCmd, QSimIconReader *reader)
{
    m_command = simCmd;
    if ( (int)simCmd.iconId() ) {
        if ( !icons && reader ) {
            icons = new SimIcons( reader, this );
            connect( icons, SIGNAL(iconsReady()), this, SLOT(iconsReady()) );
        }
        icons->needIconInFile( (int)simCmd.iconId() );
        icons->requestIcons();
    } else {
        write();
    }
    updateSoftMenuBar();
    initTimer();
}

void SimText::initTimer()
{
    if (tid) {
        killTimer(tid);
        tid = 0;
    }
    // 3 seconds timeout for auto clear
    // 2 minutes timeout for no response
    if (m_command.clearAfterDelay())
        tid = startTimer(AUTOCLEARTIMEOUT);
    else
        setNoResponseTimeout(NORESPONSETIMEOUT);
}

void SimText::write()
{
    if (m_command.type() == QSimCommand::DisplayText) {
        if (m_command.textAttribute().isEmpty())
            browser->setHtml("<qt>"+Qt::escape( m_command.text() )+"</qt>");
        else
            browser->setHtml(m_command.textHtml());
    } else {
        // This is a notification about sending SMS, SS, etc.
        if (m_command.text().isEmpty()) {
            browser->setHtml(tr("<qt><center>Please wait</center></qt>"));
        } else if (m_command.textAttribute().isEmpty()) {
            browser->setHtml("<qt>"+Qt::escape( m_command.text() )+"</qt>");
        } else {
            browser->setHtml(m_command.textHtml());
        }
    }
}

void SimText::timerEvent(QTimerEvent * /*te*/)
{
    killTimer(tid);
    tid = 0;

    if ( m_command.immediateResponse() ) {
        // Response already sent. No more response.
        emit hideApp();
    } else {
        if ( m_command.clearAfterDelay() )
            response( QSimTerminalResponse::Success );
        else
            response( QSimTerminalResponse::NoResponseFromUser );
    }
}

void SimText::showEvent(QShowEvent *e)
{
    if ( m_command.immediateResponse() ) {
        // send immediate response to the sim tool kit
        response( QSimTerminalResponse::Success );
    }
    initTimer();

    SimCommandView::showEvent(e);
}

void SimText::iconsReady()
{
    QString iconFile = icons->iconFile( m_command.iconId() );

    QTextDocument *document = browser->document();

    QTextCursor cursor( document );

    QTextImageFormat icon;
    icon.setName( iconFile );
    cursor.insertImage( icon );

    cursor.insertBlock();

    if ( !m_command.iconSelfExplanatory() ) // display text as well
        cursor.insertText( m_command.text() );
}

bool SimText::eventFilter(QObject * /*o*/, QEvent *e)
{
    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Back) {

            if ( !m_command.immediateResponse() )
                response( QSimTerminalResponse::BackwardMove );
            else
                hideApp();

            if (tid) {
                killTimer(tid);
                tid = 0;
            }
            return true;
        } else if (ke->key() == Qt::Key_Select) {
            if (m_command.clearAfterDelay())
                return false;
            if (!m_command.immediateResponse())
                response( QSimTerminalResponse::Success );
            else
                emit hideApp();

            if (tid) {
                killTimer(tid);
                tid = 0;
            }
            return true;
        } else {
            e->accept();
        }
    }
    return false;
}

void SimText::response( const QSimTerminalResponse::Result &result )
{
    QSimTerminalResponse resp;
    resp.setCommand( m_command );
    resp.setResult( result );
    emit sendResponse( resp );
}

//===========================================================================

SimInKey::SimInKey(const QSimCommand &simCmd, QSimIconReader *reader, QWidget *parent)
    : SimCommandView( simCmd, reader, parent ), iconLabel(0), edit(0), asteriskTid(0)
{
    QVBoxLayout *vb = new QVBoxLayout(this);
    if ( (int)simCmd.iconId() ) {
        iconLabel = new QLabel( this );
        vb->addWidget( iconLabel );
    }
    if (!simCmd.text().isEmpty() ) {
        if (!simCmd.textAttribute().isEmpty()) {
            text = new QLabel(simCmd.textHtml(), this);
            adjustLabelBackground(text, simCmd.textAttribute());
        } else {
            text = new QLabel( Qt::escape( simCmd.text() ), this);
        }
        text->setTextFormat(Qt::RichText);
        text->setWordWrap(true);
        vb->addWidget(text);
    }

    vb->addStretch();

    digitsOnly = simCmd.wantDigits();
    wantYesNo = simCmd.wantYesNo();
    if (!digitsOnly || Qtopia::mousePreferred()) {
        edit = new QLineEdit(this);
        edit->setMaxLength(1);
        vb->addWidget(edit);
        connect(edit, SIGNAL(textChanged(QString)),
                this, SLOT(textChanged(QString)));
        if (digitsOnly)
            QtopiaApplication::setInputMethodHint(edit,QtopiaApplication::Number);
        else
            QtopiaApplication::setInputMethodHint(edit,QtopiaApplication::Text);
        if ( simCmd.hasHelp() )
            QSoftMenuBar::setLabel( edit, Qt::Key_Context1, "help", tr( "Help" ) );
        else
            QSoftMenuBar::setLabel( edit, Qt::Key_Context1, QSoftMenuBar::NoLabel );
    } else if (wantYesNo) {
        QSoftMenuBar::setLabel( this, Qt::Key_Context1, "", tr( "Yes" ) );
        QSoftMenuBar::setLabel( this, Qt::Key_Back, "", tr( "No" ) );
        setFocusPolicy(Qt::StrongFocus);
    } else {
        setFocusPolicy(Qt::StrongFocus);
    }
    QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );

    // need to make request here
    // to make suer subclass iconsReady slot to be used
    if ( icons )
        icons->requestIcons();
}

void SimInKey::textChanged(const QString &t)
{
    if (!t.isEmpty()) {
        response(t.left(1));
        edit->clear();
    }
}

void SimInKey::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Back || e->key() == Qt::Key_No) {
        if ( edit ) {
            if ( edit->text().isEmpty() ) {
                response( QSimTerminalResponse::BackwardMove );
            } else {
                response( edit->text().left( 1 ) );
            }
        } else if ( wantYesNo ) {
            response( "No" );
        } else {
            response( QSimTerminalResponse::BackwardMove );
        }
    } else if (e->key() == Qt::Key_Context1 ) {
        if ( m_command.hasHelp() ) {
            response( QSimTerminalResponse::HelpInformationRequested );
        } else if ( wantYesNo ) {
            response( "Yes" );
        }
    } else if (digitsOnly && !e->text().isEmpty() && !wantYesNo) {
        // Need to support a way to enter '+'
        // '+' can be entered by pressing '*' twice in a short period time(1sec)
        if ( e->key() != Qt::Key_Asterisk )
            response(e->text());
        else {
            if ( !asteriskTid )
                asteriskTid = startTimer( 1000 );
            else {
                killTimer( asteriskTid );
                asteriskTid = 0;
                response( "+" );
            }
        }
    } else if (e->key() == Qt::Key_Hangup
            || e->key() == Qt::Key_Call) {
        SimCommandView::keyPressEvent(e);
    }
}

void SimInKey::timerEvent(QTimerEvent *te)
{
    if ( te->timerId() == asteriskTid ) {
        killTimer( asteriskTid );
        asteriskTid = 0;
        response( "*" );
    } else if ( te->timerId() == tid ) {
        killTimer(tid);
        tid = 0;

        // no response from user time out
        response( QSimTerminalResponse::NoResponseFromUser );
    }
}

void SimInKey::iconsReady()
{
    if ( m_command.iconSelfExplanatory() ) {
        text->clear();
    }

    if ( iconLabel ) {
        QIcon icon = icons->icon( m_command.iconId() );
        iconLabel->setPixmap( icon.pixmap( QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize) ) );
    }
}

void SimInKey::response( const QString &result )
{
    QSimTerminalResponse resp;
    resp.setCommand( m_command );
    resp.setText( result );
    emit sendResponse( resp );
}

void SimInKey::response( const QSimTerminalResponse::Result &result )
{
    QSimTerminalResponse resp;
    resp.setCommand( m_command );
    resp.setResult( result );
    emit sendResponse( resp );
}

//===========================================================================

class SimWidgetsDigitValidator : public QValidator
{
public:
    SimWidgetsDigitValidator(bool plus, QWidget *parent);

    virtual State validate(QString &, int &) const;

private:
    bool allowPlus;
};

SimWidgetsDigitValidator::SimWidgetsDigitValidator(bool plus, QWidget *parent)
    : QValidator(parent), allowPlus(plus)
{
}

QValidator::State SimWidgetsDigitValidator::validate(QString &input, int &) const
{
    QString valid("0123456789*#");
    if (allowPlus)
        valid += '+';

    QString fixed;
    for (int i=0; i < input.length(); i++) {
        if (valid.contains(input[i]))
            fixed += input[i];
    }

    input = fixed;

    return QValidator::Intermediate;
}

//===========================================================================

SimInput::SimInput(const QSimCommand &simCmd, QSimIconReader *reader, QWidget *parent)
    : SimCommandView(simCmd, reader, parent), iconLabel(0), lineEdit(0), multiEdit(0), focusWidget(0)
{
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *vb = new QVBoxLayout(this);
    if ( (int)simCmd.iconId() ) {
        iconLabel = new QLabel( this );
        vb->addWidget( iconLabel );
    }
    if (!simCmd.text().isEmpty()) {
        if (!simCmd.textAttribute().isEmpty()) {
            text = new QLabel(simCmd.textHtml(), this);
            adjustLabelBackground(text, simCmd.textAttribute());
        } else {
            text = new QLabel( Qt::escape( simCmd.text() ), this);
        }
        text->setTextFormat(Qt::RichText);
        vb->addWidget(text);
    }
    echo = simCmd.echo();
    digitsOnly = simCmd.wantDigits();

    vb->addStretch();

    if (echo && !digitsOnly) {
        multiEdit = new QTextEdit(this);
        multiEdit->setAcceptRichText( false );
        //XXX multiEdit->setMaxLength(simCmd.maximumLength());
        multiEdit->setFrameStyle(QFrame::NoFrame);
        multiEdit->installEventFilter(this);
        vb->addWidget(multiEdit);
        connect(multiEdit, SIGNAL(textChanged()), this, SLOT(validateInput()));

        QtopiaApplication::setInputMethodHint(multiEdit,QtopiaApplication::Text);

        if ( !simCmd.defaultText().isEmpty() )
            multiEdit->setText( simCmd.defaultText() );

        focusWidget = multiEdit;
    } else {
        lineEdit = new QLineEdit(this);
        lineEdit->setMaxLength(simCmd.maximumLength());
        if (!echo) {
            lineEdit->setEchoMode(QLineEdit::Password);
            QtopiaApplication::setInputMethodHint(lineEdit,QtopiaApplication::Number);
        } else if (digitsOnly) {
            lineEdit->setValidator(new SimWidgetsDigitValidator(echo, this));
            QtopiaApplication::setInputMethodHint(lineEdit,QtopiaApplication::PhoneNumber);
        } else {
            QtopiaApplication::setInputMethodHint(lineEdit,QtopiaApplication::Text);
        }
        lineEdit->installEventFilter(this);
        vb->addWidget(lineEdit);
        connect(lineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateInput()));

        if ( !simCmd.defaultText().isEmpty() )
            lineEdit->setText( simCmd.defaultText() );

        focusWidget = lineEdit;
    }

    if ( focusWidget ) {
        QMenu *menu = QSoftMenuBar::menuFor( focusWidget );
        if ( simCmd.hasHelp() )
            menu->addAction( QIcon(":icon/help"), tr( "Help" ), this, SLOT(sendHelpRequest()) );;
        QSoftMenuBar::setHelpEnabled( focusWidget, false );
        QSoftMenuBar::setLabel( focusWidget, Qt::Key_Back, QSoftMenuBar::Back );

        focusWidget->installEventFilter( this );
        focusWidget->setEditFocus( true );
    }

    validateInput();

    // need to make request here
    // to make suer subclass iconsReady slot to be used
    if ( icons )
        icons->requestIcons();
}

void SimInput::iconsReady()
{
    if ( m_command.iconSelfExplanatory() ) {
        text->clear();
    }

    if ( iconLabel ) {
        QIcon icon = icons->icon( m_command.iconId() );
        iconLabel->setPixmap( icon.pixmap( QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize) ) );
    }
}

void SimInput::setInput(const QString &input)
{
    if (echo && !digitsOnly)
        multiEdit->setText(input);
    else
        lineEdit->setText(input);
}

void SimInput::keyPressEvent(QKeyEvent *e)
{
    if ( e->key() == Qt::Key_Call
            || e->key() == Qt::Key_Hangup )
        SimCommandView::keyPressEvent(e);
}

bool SimInput::eventFilter(QObject *o, QEvent *e)
{
    if ( o != focusWidget)
        return false;

    if( Qtopia::mousePreferred() ) {
        if (e->type() == QEvent::LeaveEditFocus) {
            done();
        }
    }

    // FIXME temporary workaround until menu can be removed from line & text edit.
    if ( e->type() == QEvent::KeyRelease ) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        Qt::Key key = (Qt::Key)ke->key();
        if ( key == Qt::Key_Back ) {
            if (multiEdit && multiEdit->toPlainText().length())
                return false;
            if (lineEdit && lineEdit->text().length())
                return false;

            QSimTerminalResponse resp;
            resp.setCommand(m_command);
            resp.setResult(QSimTerminalResponse::BackwardMove);
            emit sendResponse(resp);
        }
    } else if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        Qt::Key key = (Qt::Key)ke->key();
        if ( key == Qt::Key_Select ) {
            done();
            return true;
        } else if ( key == Qt::Key_Up || key == Qt::Key_Down )
            return true;
    }
    return false;
}

void SimInput::done()
{
    QString in;
    if (multiEdit)
        in = multiEdit->toPlainText();
    else
        in = lineEdit->text();

    if (uint(in.length()) >= m_command.minimumLength() &&
            (uint(in.length()) <= m_command.maximumLength() || m_command.maximumLength() == 255)){
        QSimTerminalResponse resp;
        resp.setCommand(m_command);
        resp.setText(in);
        emit sendResponse(resp);
    }else{
        qLog(STK) << "Text input not in the bounds of min and max length";
    }
}

void SimInput::validateInput()
{
    if (!multiEdit && !lineEdit)
        return;

    // check that input is valid
    QWidget *inputWidget=0;
    QString in;

    if (multiEdit){
        inputWidget  = multiEdit;
        in = multiEdit->toPlainText();
    }else{
        inputWidget = lineEdit;
        in = lineEdit->text();
    }

    if (in.length())
        QSoftMenuBar::setLabel(inputWidget, Qt::Key_Back, QSoftMenuBar::BackSpace);
    else
        QSoftMenuBar::setLabel(inputWidget, Qt::Key_Back, QSoftMenuBar::Back);

    if (uint(in.length()) >= m_command.minimumLength() &&
            (uint(in.length()) <= m_command.maximumLength() || m_command.maximumLength() == 255)){
        QSoftMenuBar::setLabel(inputWidget, Qt::Key_Select, QSoftMenuBar::Ok);
    }else{
        QSoftMenuBar::setLabel(inputWidget, Qt::Key_Select, QSoftMenuBar::NoLabel);
    }

    // reset no response timer
    setNoResponseTimeout(NORESPONSETIMEOUT);
}

void SimInput::sendHelpRequest()
{
    QSimTerminalResponse resp;
    resp.setCommand( m_command );
    resp.setResult(QSimTerminalResponse::HelpInformationRequested);
    emit sendResponse(resp);
}

void SimInput::timerEvent(QTimerEvent *)
{
    killTimer(tid);
    tid = 0;

    // no response from user time out
    QSimTerminalResponse resp;
    resp.setCommand(m_command);
    resp.setResult( QSimTerminalResponse::NoResponseFromUser );
    emit sendResponse(resp);
}

//===========================================================================

SoftKeysMenu::SoftKeysMenu(const QSimCommand& cmd, QSimIconReader *reader, QWidget *parent)
    : SimCommandView(cmd, reader, parent)
{
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin(0);

    // menu title
    if (!cmd.title().isEmpty()) {
        title = new QLabel(cmd.title(), this);
        QFont f = font();
        f.setWeight(QFont::Bold);
        title->setFont(f);
        title->setAlignment(Qt::AlignCenter);
        vb->addWidget(title);
    }

    QLabel *label = new QLabel( tr("Please select a menu item using soft keys."), this );
    label->setWordWrap(true);
    vb->addWidget(label);

    QList<QSimMenuItem> items = cmd.menuItems();
    const QList<int> softKeys = QSoftMenuBar::keys();

    QList<QSimMenuItem>::Iterator it;
    int i = 0;
    for (it = items.begin(); it != items.end(); ++it) {

        // keep back button intact
        if ( softKeys.at( i ) == (int)Qt::Key_Back )
            continue;

        if ( (*it).iconId() != 0 ) {
            icons->needIconInFile( (int)(*it).iconId() );
        } else {
            QSoftMenuBar::setLabel(this, softKeys.at( i++ ), "", (*it).label());
        }
        if ((*it).nextAction()) {
            // indicate next action
        }
    }

    // set no label to unused softkeys.
    for ( ; i < softKeys.count(); i++ )
        if ( softKeys.at( i ) != (int)Qt::Key_Back )
            QSoftMenuBar::setLabel(this, softKeys.at( i ), QSoftMenuBar::NoLabel );

    if ( icons )
        icons->requestIcons();
}

SoftKeysMenu::~SoftKeysMenu()
{
}

void SoftKeysMenu::keyPressEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Hangup
            || ke->key() == Qt::Key_Call) {
        SimCommandView::keyPressEvent(ke);
        return;
    }

    if (ke->key() == Qt::Key_Back) {
        QSimTerminalResponse resp;
        resp.setCommand(m_command);
        resp.setResult(QSimTerminalResponse::BackwardMove);
        emit sendResponse(resp);
        return;
    }

    const QList<int> softKeys = QSoftMenuBar::keys();
    int index = softKeys.indexOf( ke->key() );
    if ( index == -1 )
        return;

    QList<QSimMenuItem> items = m_command.menuItems();

    if ( index >= items.count() )
        return;

    QSimMenuItem item = items.at( index );

    if ( m_command.type() == QSimCommand::SetupMenu ) {
        QSimEnvelope env;
        env.setType(QSimEnvelope::MenuSelection);
        env.setSourceDevice(QSimCommand::Keypad);
        env.setMenuItem(item.identifier());
        emit sendEnvelope(env);
    } else {
        QSimTerminalResponse resp;
        resp.setCommand(m_command);
        resp.setMenuItem(item.identifier());
        emit sendResponse(resp);
    }
}

void SoftKeysMenu::iconsReady()
{
    QList<QSimMenuItem> items = m_command.menuItems();
    const QList<int> softKeys = QSoftMenuBar::keys();

    int index = 0;
    for ( ; index < items.count(); ++index ) {
        QString iconFile = icons->iconFile( items.at(index).iconId() );
        if ( !iconFile.isEmpty() ) {
            QSoftMenuBar::setLabel(this, softKeys.at(index),
                    iconFile, items.at(index).label());
        } else {
            QSoftMenuBar::setLabel(this, softKeys.at(index),
                    "", items.at(index).label());
        }
    }

    // set no label to unused softkeys.
    for ( ; index < softKeys.count(); index++ )
        if ( softKeys.at( index ) != (int)Qt::Key_Back )
            QSoftMenuBar::setLabel(this, softKeys.at( index ), QSoftMenuBar::NoLabel );
}


//===========================================================================

SimTone::SimTone(const QSimCommand &simCmd, QSimIconReader *reader, QWidget *parent)
    : SimCommandView(simCmd, reader, parent)
#ifdef MEDIA_SERVER
, soundcontrol(0)
#else
, currentSound(0)
#endif
{
    soundTimer = new QTimer( this );
    soundTimer->setSingleShot( true );
    connect( soundTimer, SIGNAL(timeout()), this, SLOT(stopSound()) );
    connect( soundTimer, SIGNAL(timeout()), this, SLOT(success()) );

    // display text
    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin(0);

    // tone title
    if (simCmd.textAttribute().isEmpty()) {
        if (!simCmd.text().isEmpty())
            title = new QLabel(simCmd.text(), this);
        else
            title = new QLabel(tr("Playing a sound."), this);
        title->setWordWrap(true);
        QFont f = font();
        f.setWeight(QFont::Bold);
        title->setFont(f);
        title->setAlignment(Qt::AlignCenter);
    } else {
        title = new QLabel(simCmd.textHtml(), this);
        adjustLabelBackground(title, simCmd.textAttribute());
        title->setTextFormat(Qt::RichText);
        title->setWordWrap(true);
    }
    vb->addWidget(title);

    playTone( findFile( simCmd.tone() ) );

    if ( icons )
        icons->requestIcons();
}

SimTone::~SimTone()
{
    // stop the tone
    // in case the session is terminated by the user
    stopSound();
}

QString SimTone::findFile(const QSimCommand::Tone &tone)
{
    QString file;
    switch ( tone ) {
        case QSimCommand::ToneDial:
            file = ":sound/simapp/dialtone"; break;

        case QSimCommand::ToneBusy:
            file = ":sound/simapp/busy"; break;

        case QSimCommand::ToneCongestion:
            file = ":sound/simapp/congestion"; break;

        case QSimCommand::ToneRadioAck:
            file = ":sound/simapp/radioack"; break;

        case QSimCommand::ToneDropped:
            file = ":sound/simapp/dropped"; break;

        case QSimCommand::ToneError:
            file = ":sound/simapp/error"; break;

        case QSimCommand::ToneCallWaiting:
            file = ":sound/simapp/waiting"; break;

        case QSimCommand::ToneRinging:
            file = ":sound/simapp/ringing"; break;

        case QSimCommand::ToneGeneralBeep:
            file = ":sound/simapp/beep"; break;

        case QSimCommand::TonePositiveBeep:
            file = ":sound/simapp/posbeep"; break;

        case QSimCommand::ToneNegativeBeep:
            file = ":sound/simapp/negbeep"; break;
        default:
            {
                // tone not supported by ME. report to SIM
                QSimTerminalResponse resp;
                resp.setCommand( m_command );
                resp.setResult( QSimTerminalResponse::BeyondMECapabilities );
                emit sendResponse( resp );
            }
    }
    return file;
}

void SimTone::stopSound()
{
    soundTimer->stop();
#ifndef DUMMY_TEST_SOUNDS
#ifdef MEDIA_SERVER
    if (soundcontrol) {
        delete soundcontrol->sound();
        delete soundcontrol;
        soundcontrol = 0;
    }
#else
    if ( currentSound ) {
        currentSound->stop();
        delete currentSound;
        currentSound = 0;
    }
#endif
#endif
}

void SimTone::success()
{
    QSimTerminalResponse resp;
    resp.setCommand( m_command );
    resp.setResult( QSimTerminalResponse::Success );
    emit sendResponse( resp );
}

void SimTone::playTone(const  QString &file)
{
#ifndef DUMMY_TEST_SOUNDS
#ifdef MEDIA_SERVER
    soundcontrol = new QSoundControl( new QSound( file ) );
    soundcontrol->setPriority( QSoundControl::RingTone );
    soundcontrol->setVolume( 50 );
    if ( m_command.toneTime() ) {
        soundcontrol->sound()->setLoops( 100 );
        soundTimer->start( (int)m_command.toneTime() );
    }
    soundcontrol->sound()->play();
#else
    // Play the sound.
    currentSound = new QSound( file, this );
    if ( m_command.toneTime() ) {
        // Play continuously until the specified time expires.
        currentSound->setLoops( 100 );
        soundTimer->start( (int)m_command.toneTime() );
    }
    // Play the file once and then stop automatically.
    currentSound->play();
#endif
#else
    Q_UNUSED(file)
    if ( m_command.toneTime() ) {
        soundTimer->start( (int)m_command.toneTime() );
    } else {
        soundTimer->start( 500 );
    }
#endif
}

void SimTone::iconsReady()
{
    if ( m_command.iconId() != 0 ) {
        QIcon icon = icons->icon( m_command.iconId() );
        if ( !icon.isNull() ) {
            title->setPixmap(icon.pixmap(QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize)));
        }
    }
}

//===========================================================================

SimSetupCall::SimSetupCall(const QSimCommand&cmd, QWidget *parent)
    : SimCommandView(cmd, parent), redialCount(0), otherTextBox(0)
{
    // create call manager
    callManager = new QPhoneCallManager( this );

    init();
}

SimSetupCall::SimSetupCall(const QSimCommand&cmd, QSimIconReader *reader, QWidget *parent)
    : SimCommandView(cmd, reader, parent), redialCount(0), otherTextBox(0)
{
    // create call manager
    callManager = new QPhoneCallManager( this );

    // need to make request here
    // to make suer subclass iconsReady slot to be used
    if ( icons )
        icons->requestIcons();
}

void SimSetupCall::iconsReady()
{
    init();
}

void SimSetupCall::init()
{
    // handles only voice calls
    if (m_command.callClass() != QSimCommand::Voice) {
        QSimTerminalResponse resp;
        resp.setCommand(m_command);
        resp.setResult(QSimTerminalResponse::BeyondMECapabilities);
        sendResponse(resp);
        return;
    }

    // current call status
    int activeCount = 0;
    int holdCount = 0;
    QList<QPhoneCall> calls = callManager->calls();
    for (QList<QPhoneCall>::ConstIterator it = calls.begin(); it != calls.end(); ++it) {
        if ((*it).state() == QPhoneCall::Connected)
            activeCount++;
        else if ((*it).state() == QPhoneCall::Hold)
            holdCount++;
    }

    bool doCall = false;
    bool endActive = false;
    bool holdActive = false;
    busyOnCall = false;
    callControlProblem = false;

    switch (m_command.disposition()) {
        case QSimCommand::IfNoOtherCalls:
            if (!activeCount)
                doCall = true;
            else
                busyOnCall = true;
            break;
        case QSimCommand::PutOnHold:
            doCall = true;
            if (activeCount)
                holdActive = true;
            break;
        case QSimCommand::Disconnect:
            doCall = true;
            if (activeCount)
                endActive = true;
            break;
    }

    if (!doCall && !busyOnCall)
        callControlProblem = true;

    // create widgets
    QVBoxLayout *vb = new QVBoxLayout(this);
    if ( (int)m_command.iconId() ) {
        iconLabel = new QLabel( this );
        vb->addWidget( iconLabel );
        QIcon icon = icons->icon( m_command.iconId() );
        iconLabel->setPixmap( icon.pixmap( QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize) ) );
    }
    text = new QLabel( this );
    text->setTextFormat(Qt::RichText);
    text->setWordWrap(true);
    vb->addWidget(text);
    QString message;

    if ( !m_command.iconSelfExplanatory() )
        message = m_command.text();

    if ( !busyOnCall && !callControlProblem ) {
        message += "<br><br>"
            + tr( "Do you wish to continue dialing %1?",
                    "%1 = phone number" ).arg(m_command.number());
    }

    if (endActive)
        message += "<br><br>" + tr( "All active calls will be ended." );
    if (holdActive)
        message += "<br><br>" + tr( "All active calls will be on hold." );
    if (busyOnCall)
        message += "<br><br>" + tr( "Cannot place this call because there is an active call." );
    if (callControlProblem)
        message += "<br><br>" + tr( "Cannot place this call due to temporary call control problem." );

    text->setText( message );

    vb->addStretch();

    // setup soft keys to Yes, No
    if ( busyOnCall || callControlProblem ) {
        QSoftMenuBar::setLabel( this, Qt::Key_Context1, QSoftMenuBar::NoLabel );
        QSoftMenuBar::setLabel( this, Qt::Key_Back, "", tr( "OK" ) );
    } else {
        QSoftMenuBar::setLabel( this, Qt::Key_Context1, "", tr( "Yes" ) );
        QSoftMenuBar::setLabel( this, Qt::Key_Back, "", tr( "No" ) );
    }
    QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );

    setFocusPolicy( Qt::StrongFocus );
}

void SimSetupCall::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Qt::Key_Context1 ) { // Yes
        if ( m_command.disposition() != QSimCommand::IfNoOtherCalls ) {
            disposeCalls();
            // take some time before making a new call
            // after hanging up, putting on hold
            QTimer::singleShot( 1000, this, SLOT(dial()) );
        } else {
            dial();
        }
    } else if ( e->key() == Qt::Key_Back ) {
        QSimTerminalResponse resp;
        resp.setCommand(m_command);

        if ( busyOnCall ) {
            resp.setResult( QSimTerminalResponse::MEUnableToProcess );
            resp.setCause( QSimTerminalResponse::BusyOnCall );
        } else if ( callControlProblem ) {
            resp.setResult( QSimTerminalResponse::TemporaryCallControlProblem );
        } else {
            resp.setResult(QSimTerminalResponse::UserDidNotAccept);
        }
        sendResponse(resp);
    } else if ( e->key() == Qt::Key_Call
            || e->key() == Qt::Key_Hangup ) {
        SimCommandView::keyPressEvent(e);
    }
}

void SimSetupCall::disposeCalls()
{
    QList<QPhoneCall> calls = callManager->calls();

    if ( m_command.disposition() == QSimCommand::Disconnect ) {
        foreach ( QPhoneCall call, calls ) {
            if ( call.connected() )
                call.hangup( QPhoneCall::CallOnly );
        }
    } else if ( m_command.disposition() == QSimCommand::PutOnHold) {
        // hack to find out if this is a multiparty call
        // if more than one call is active at once, multipary call
        multiPartyCall = false;
        int count = 0;
        foreach ( QPhoneCall call, calls ) {
            if ( call.connected() )
                count++;

            if ( count > 1 ) {
                multiPartyCall = true;
                break;
            }
        }

        foreach ( QPhoneCall call, calls ) {
            if ( call.connected() ) {
                if ( call.canHold() ) {
                    call.connectRequestFailed(this,
                            SLOT(requestFailed(QPhoneCall,QPhoneCall::Request)));
                    call.hold();
                } else { // ME not support call hold
                    QSimTerminalResponse resp;
                    resp.setCommand(m_command);
                    resp.setResult(QSimTerminalResponse::BeyondMECapabilities);
                    sendResponse(resp);
                    return;
                }
            }
        }
    }
}

void SimSetupCall::callStateChanged( const QPhoneCall& call )
{
    // send the response to a setup call
    bool done = false;
    QSimTerminalResponse::Result result = QSimTerminalResponse::Success;

    if (call.state() == QPhoneCall::Connected) {
        // send sub address, DTMF tones
        if ( !m_command.subAddress().isEmpty() && call.canTone() ) {
            foreach ( QPhoneCall c, callManager->calls() ) {
                if ( c.connected() ) {
                    c.tone( m_command.subAddress() );
                    break;
                }
            }
        }
        result = QSimTerminalResponse::Success;
        done = true;

        // need to display other text while connected.
        if ( otherTextBox ) {
            otherTextBox->setWindowTitle( tr( "Connected" ) );
            otherTextBox->setText( m_command.otherText() );
        }
    } else if (call.state() == QPhoneCall::HangupLocal) {
        // user pressed hangup
        result = QSimTerminalResponse::UserClearedDownCall;
        done = true;

        if ( otherTextBox ) {
            delete otherTextBox;
            otherTextBox = 0;
        }
    } else if (call.state() == QPhoneCall::HangupRemote) {
        // network fail
        if ( m_command.withRedial() ) {
            if ( redialCount < MAXREDIALCOUNT ) {
                redialCount++;
                dial();
            } else {
                // final redial attampt failed
                result = QSimTerminalResponse::NetworkUnableToProcess;
                done = true;
            }
        } else {
            result = QSimTerminalResponse::NetworkUnableToProcess;
            done = true;
        }

        if ( otherTextBox ) {
            delete otherTextBox;
            otherTextBox = 0;
        }
    }

    if (done) {
        QSimTerminalResponse resp;
        resp.setCommand(m_command);
        resp.setResult(result);
        sendResponse(resp);
        call.disconnectStateChanged(this, SLOT(callStateChanged(QPhoneCall)));
    }

    // need to display other text and/or icon while dialing.
    if ( call.dialing() && !m_command.otherText().isEmpty() ) {
        if ( !otherTextBox ) {
            otherTextBox = new QMessageBox(0);
            otherTextBox->setWindowTitle( tr( "Dialing" ) );

            if ( m_command.otherIconId() )
                otherTextBox->setIconPixmap
                    ( icons->icon( m_command.otherIconId() )
                      .pixmap( QApplication::style()->
                          pixelMetric(QStyle::PM_MessageBoxIconSize) ) );

            if ( !m_command.otherIconSelfExplanatory() )
               otherTextBox->setText( m_command.otherText() );

            otherTextBox->resize( otherTextBox->size() );
            otherTextBox->show();
        }
    }
}

void SimSetupCall::requestFailed( const QPhoneCall& call, QPhoneCall::Request request )
{
    Q_UNUSED(call)
    if ( request == QPhoneCall::HoldFailed ) {
        QSimTerminalResponse resp;
        resp.setCommand( m_command );
        if ( multiPartyCall )
            resp.setResult( QSimTerminalResponse::SsReturnError );
        else
            resp.setResult( QSimTerminalResponse::NetworkUnableToProcess );
        sendResponse( resp );
    }
}

void SimSetupCall::dial()
{
    QDialOptions dialopts;
    dialopts.setNumber( m_command.number() );
    QPhoneCall call = callManager->create( "Voice", "modem" );
    call.connectStateChanged(this, SLOT(callStateChanged(QPhoneCall)));
    call.dial( dialopts );
    callStateChanged(call);
}

//===========================================================================

SimChannel::SimChannel(const QSimCommand &cmd, QWidget *parent)
    : SimCommandView(cmd, parent)
{
    init();
}

SimChannel::SimChannel(const QSimCommand &cmd, QSimIconReader *reader, QWidget *parent)
    : SimCommandView(cmd, reader, parent)
{
    if ( icons )
        icons->requestIcons();
}

void SimChannel::iconsReady()
{
    init();
}

void SimChannel::init()
{
    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin(0);

    if ( (int)m_command.iconId() ) {
        iconLabel = new QLabel( this );
        vb->addWidget( iconLabel );
        QIcon icon = icons->icon( m_command.iconId() );
        iconLabel->setPixmap( icon.pixmap( QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize) ) );
    }

    // alpha identifier
    text = new QLabel( Qt::escape( m_command.text() ), this);
    text->setTextFormat(Qt::RichText);
    text->setWordWrap(true);
    vb->addWidget(text);

    vb->addStretch();

    if ( m_command.iconSelfExplanatory() )
        text->clear();

    if ( !text->text().isEmpty() )
        text->setText( text->text() + "<br><br>" );

    // if busy on call
    busyOnCall = false;
    QPhoneCallManager callManager( this );
    foreach ( QPhoneCall call, callManager.calls() ) {
        if ( call.connected() ) {
            busyOnCall = true;
            break;
        }
    }

    if ( !busyOnCall ) {
        // set soft key labels
        QSoftMenuBar::setLabel( this, Qt::Key_Context1, "", tr( "Yes" ) );
        QSoftMenuBar::setLabel( this, Qt::Key_Back, "", tr( "No" ) );
        QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );
        text->setText( text->text() + tr( "Do you wish to open a channel?" ) );
    } else {
        QSoftMenuBar::setLabel( this, Qt::Key_Context1, QSoftMenuBar::NoLabel );
        QSoftMenuBar::setLabel( this, Qt::Key_Back, "", tr( "OK" ) );
        QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );
        text->setText( text->text()
                + tr( "Cannot open this channel because there is an active call." ) );
    }

    setFocusPolicy(Qt::StrongFocus);
}

void SimChannel::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Qt::Key_Context1 ) { // Yes
        QSimTerminalResponse resp;
        resp.setCommand(m_command);
        resp.setResult(QSimTerminalResponse::Success);
        sendResponse(resp);
    } else if ( e->key() == Qt::Key_Back ) { // user did not accept
        QSimTerminalResponse resp;
        resp.setCommand(m_command);
        if ( !busyOnCall ) {
            resp.setResult(QSimTerminalResponse::UserDidNotAccept);
        } else {
            resp.setResult(QSimTerminalResponse::MEUnableToProcess);
            resp.setCause(QSimTerminalResponse::BusyOnCall);
        }
        sendResponse(resp);
    } else if ( e->key() == Qt::Key_Call
            || e->key() == Qt::Key_Hangup ) {
        SimCommandView::keyPressEvent(e);
    }
}

//===========================================================================

SimLaunchBrowser::SimLaunchBrowser(const QSimCommand &cmd, QWidget *parent)
    : SimCommandView(cmd, parent), isAvailable(false), isRunning(false)
{
    init();
}

SimLaunchBrowser::SimLaunchBrowser(const QSimCommand &cmd, QSimIconReader *reader, QWidget *parent)
    : SimCommandView(cmd, reader, parent), isAvailable(false), isRunning(false)
{
    if ( icons )
        icons->requestIcons();
}

void SimLaunchBrowser::iconsReady()
{
    init();
}

void SimLaunchBrowser::init()
{
    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin(0);

    if ( (int)m_command.iconId() ) {
        iconLabel = new QLabel( this );
        vb->addWidget( iconLabel );
        QIcon icon = icons->icon( m_command.iconId() );
        iconLabel->setPixmap( icon.pixmap( QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize) ) );
    }

    // alpha identifier
    text = new QLabel( Qt::escape( m_command.text() ), this);
    text->setTextFormat(Qt::RichText);
    text->setWordWrap(true);
    vb->addWidget(text);

    vb->addStretch();

    if ( m_command.iconSelfExplanatory() )
        text->clear();

    if ( !text->text().isEmpty() )
        text->setText( text->text() + "<br><br>" );

    // set soft key labels
    QSoftMenuBar::setLabel( this, Qt::Key_Context1, QSoftMenuBar::NoLabel );
    QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );
    QSoftMenuBar::setLabel( this, Qt::Key_Back, "", tr( "OK" ) );

    setFocusPolicy(Qt::StrongFocus);

    // check browser's availability
    if ( !QtopiaService::apps( "WebAccess" ).count() ) {
        text->setText( text->text() + tr( "Browser is not available." ) );
        return;
    } else {
        isAvailable = true;
    }

    // check if browser is running
#ifndef QTOPIA_TEST
    UIApplicationMonitor monitor;
    if ( monitor.runningApplications().contains( QtopiaService::app( "WebAccess" ) ) )
        isRunning = true;
#endif

    if ( !isRunning ) {
        text->setText( text->text()
                + tr( "Browser will start and open %1.", "%1 = url" ).arg( m_command.url() ) );
        return;
    }

    switch ( m_command.browserLaunchMode() ) {
        case QSimCommand::IfNotAlreadyLaunched:
            {
                text->setText( text->text()
                        + tr( "Cannot open %1 because browser is already running.", "%1 = url" )
                        .arg( m_command.url() ) );
            }
            break;
        case QSimCommand::UseExisting:
            {
                text->setText( text->text()
                        + tr( "Browser will open the new web address, %1.", "%1 = url" )
                        .arg( m_command.url() ) );
            }
            break;
        case QSimCommand::CloseExistingAndLaunch:
            {
                text->setText( text->text()
                        + tr( "Browser will finish the current session "
                            "and open the new web address, %1.", "%1 = url" )
                        .arg( m_command.url() ) );
            }
            break;
    }
}

void SimLaunchBrowser::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Qt::Key_Back ) {
        QSimTerminalResponse resp;
        resp.setCommand(m_command);
        bool success = false;
        if ( !isAvailable ||
                ( isRunning &&
                  m_command.browserLaunchMode() == QSimCommand::IfNotAlreadyLaunched ) ) {
            resp.setResult(QSimTerminalResponse::MEUnableToProcess);
            resp.setCause(QSimTerminalResponse::BrowserUnavailable);
        } else {
            success = true;
            resp.setResult(QSimTerminalResponse::Success);
        }
        sendResponse(resp);

        if ( success ) {
            // send service request
            if ( isRunning &&
                    m_command.browserLaunchMode() == QSimCommand::CloseExistingAndLaunch ) {
                QtopiaServiceRequest clear( "WebAccess", "clearSession()" );
                clear.send();
            }

            QtopiaServiceRequest open( "WebAccess", "openURL(QString)" );
            open << m_command.url();
            open.send();
        }
    } else if ( e->key() == Qt::Key_Call
            || e->key() == Qt::Key_Hangup ) {
        SimCommandView::keyPressEvent(e);
    }
}


