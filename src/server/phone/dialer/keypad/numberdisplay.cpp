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

#include <limits.h>

#include "numberdisplay.h"
#include "savetocontacts.h"
#include "qabstractmessagebox.h"

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

#include <qtopiaservices.h>
#include <qtopiaipcenvelope.h>

#include <QPainter>
#include <QTimer>
#include <QKeyEvent>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QDesktopWidget>
#include <QMenu>

static const int SPEEDDIAL_MAXDIGITS = 2;
static const int SPEEDDIAL_TIMEOUT = 700; // ms
static const int MULTITAP_TIMEOUT = 1000;
const int mgn = 2;


class NumberDisplayMultiTap : public QObject
{
Q_OBJECT
public:
    NumberDisplayMultiTap(QObject *parent = 0);

    bool processKeyPressEvent(const QChar &);
    void processKeyReleaseEvent(const QChar &);
    void reset();

signals:
    void composeKey(const QChar &);
    void completeKey(const QChar &);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    void startPressTimer();
    void stopPressTimer();

    QHash<QChar, QString> m_taps;
    QHash<QChar, QChar> m_holds;

    QChar currentKey;
    QChar lastTapKey;
    int currentTaps;
    int m_timerId;
};

NumberDisplayMultiTap::NumberDisplayMultiTap(QObject *parent)
: QObject(parent), currentTaps(0), m_timerId(0)
{
    QSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat);

    cfg.beginGroup("PhoneTextButtons");
    QString buttons = cfg.value("Buttons").toString();
    for(int ii = 0; ii < buttons.count(); ++ii) {
        QChar ch = buttons[ii];

        QString tap("Tap");
        QString hold("Hold");
        tap += ch;
        hold += ch;

        QString keys = cfg.value(tap).toString();
        if(keys.count() > 1 && keys.at(0) == QChar('\'')) {
            m_taps[ch] = keys.mid(1);
        }
        keys = cfg.value(hold).toString();
        if(keys.count() > 1 && keys.at(0) == QChar('\'')) {
            m_holds[ch] = keys.at(1);
        }
    }
    cfg.endGroup();
}

/*! Returns true if the keypress has been consumed */
bool NumberDisplayMultiTap::processKeyPressEvent(const QChar &ch)
{
    if(!currentKey.isNull()) {
        // Some other key has been seen before the release of the current
        // key - consume all
        return true;
    }

    // A few cases:
    //    If tap is available, and the last tap key is this key we increment
    //          the tap and consume
    //    If tap is available, and the last tap key is not this key we set the
    //          last tap key and consume
    //    If hold is available, we set the hold timer
    //    If neither is available we do not consume the key

    QHash<QChar, QString>::ConstIterator tapIter = m_taps.find(ch);
    QHash<QChar, QChar>::ConstIterator holdIter = m_holds.find(ch);

    // Don't consume if not configured or if a single option tap with no hold
    // is configured
    if((tapIter == m_taps.end() && holdIter == m_holds.end()) ||
       (tapIter != m_taps.end() && 1 == tapIter->count() &&
        holdIter == m_holds.end())) {
        stopPressTimer();

        // If there is a last tap key, we complete it
        if(!lastTapKey.isNull()) {
            QHash<QChar, QString>::ConstIterator iter = m_taps.find(lastTapKey);
            Q_ASSERT(iter != m_taps.end());
            emit completeKey(iter->at(currentTaps));
        }
        lastTapKey = QChar();
        currentTaps = 0;
        return false;
    }

    currentKey = ch;

    // If tap key...
    if(tapIter != m_taps.end()) {
        if(lastTapKey == ch) {
            // Increment the tap counter
            currentTaps = (currentTaps + 1) % tapIter->count();
        } else {
            lastTapKey = ch;
            currentTaps = 0;
        }
        startPressTimer();
        emit composeKey(tapIter->at(currentTaps));
    } else {
        // holdIter != m_holds.end()
        lastTapKey = QChar();
        startPressTimer();
        emit composeKey(*holdIter);
    }
    return true;
}

void NumberDisplayMultiTap::processKeyReleaseEvent(const QChar &ch)
{
    if(ch == currentKey) {
        // If we are in a middle of a hold (ie. no lastTapKey), we cancel the
        // hold compose and emit the actual key
        currentKey = QChar();
        if(lastTapKey.isNull()) {
            stopPressTimer();
            emit completeKey(ch);
        }
    }
}

void NumberDisplayMultiTap::reset()
{
    stopPressTimer();
    currentKey = QChar();
    lastTapKey = QChar();
    currentTaps = 0;
}

void NumberDisplayMultiTap::timerEvent(QTimerEvent *)
{
    stopPressTimer();

    if(!currentKey.isNull()) {
        // If there is a current key with a hold value, commit the hold value.
        QHash<QChar, QChar>::ConstIterator iter = m_holds.find(currentKey);
        if(iter != m_holds.end()) {
            emit completeKey(*iter);
        }
        currentKey = QChar();
    } else if(Qt::Key_unknown != lastTapKey) {
        // If there is a last tap key, commit the tap value
        QHash<QChar, QString>::ConstIterator iter = m_taps.find(lastTapKey);
        Q_ASSERT(iter != m_taps.end());
        emit completeKey(iter->at(currentTaps));
    }

    currentTaps = 0;
    lastTapKey = QChar();
}

void NumberDisplayMultiTap::startPressTimer()
{
    if(m_timerId)
        killTimer(m_timerId);
    m_timerId = startTimer(MULTITAP_TIMEOUT);
}

void NumberDisplayMultiTap::stopPressTimer()
{
    if(m_timerId)
        killTimer(m_timerId);
    m_timerId = 0;
}

/*!
  \class NumberDisplay
    \inpublicgroup QtTelephonyModule
  \internal
  */
NumberDisplay::NumberDisplay( QWidget *parent )
: QWidget( parent ), mLargestCharWidth( 0 ), mFontSizes(),
  composeKey(Qt::Key_unknown), delayEmitNumberChanged(false)
{
    QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::BackSpace);

    QMenu *menu = QSoftMenuBar::menuFor( this );
    mSendMessage = new QAction( QIcon( ":icon/txt" ), tr("Send Message"), this );
    connect( mSendMessage, SIGNAL(triggered()), this, SLOT(sendMessage()) );
    menu->addAction( mSendMessage );
    mNewAC = new QAction( QIcon( ":image/addressbook/AddressBook" ), tr("Save to Contacts"), this );
    connect( mNewAC, SIGNAL(triggered()), this, SLOT(addPhoneNumberToContact()) );
    menu->addAction( mNewAC );

    setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed) );
    setFocusPolicy( Qt::StrongFocus );

    mFontSizes += 24;
    mFontSizes += 18;
    mFontSizes += 16;
    mFontSizes += 14;
    mFontSizes += 11;
    mFontSizes += 8;
    mFontSizes += 6;
    mFontSizes += 4;

    QFont f = font();
    f.setBold( true );
    QFontMetrics fm( f );
    QString possibleChars = "0123456789wp+*";
#ifdef QTOPIA_VOIP
    possibleChars += "abcdefghijklmnoqrstuvxyz~-^@$_%!";
#endif
    for( int i = 0 ; i < (int)possibleChars.length() ; ++i )
    {
        int cw = fm.width( possibleChars[i] );
        if( cw > mLargestCharWidth )
            mLargestCharWidth = cw;
    }
    connect(this, SIGNAL(numberChanged(QString)), this,
            SLOT(enableAction(QString)));

    tap = new NumberDisplayMultiTap(this);
    QObject::connect(tap, SIGNAL(composeKey(QChar)),
                     this, SLOT(composeTap(QChar)));
    QObject::connect(tap, SIGNAL(completeKey(QChar)),
                     this, SLOT(completeTap(QChar)));

    tid_speeddial = 0;
    QtopiaApplication::setInputMethodHint( this, QtopiaApplication::PhoneNumber );
}

void NumberDisplay::enableAction( const QString &n )
{
    if( n.isEmpty() )
    {
        if( mNewAC->isEnabled() )
            mNewAC->setEnabled( false );
        if( mSendMessage )
            mSendMessage->setEnabled( false );
    }
    else
    {
        if( !mNewAC->isEnabled() )
            mNewAC->setEnabled( true );
        if( !mSendMessage->isEnabled() )
            mSendMessage->setEnabled( true );
    }
}

void NumberDisplay::sendMessage()
{
    if( !number().isEmpty() )
    {
        QtopiaServiceRequest req( "SMS", "writeSms(QString,QString)" );
        req << QString() << number();
        req.send();
    }
}

void NumberDisplay::addPhoneNumberToContact()
{
    if( !number().isEmpty() )
        SavePhoneNumberDialog::savePhoneNumber(number());
}

void NumberDisplay::appendNumber( const QString &numbers, bool speedDial )
{
    // add spaces for line breaking as appropriate
    if ( tid_speeddial ) {
        killTimer(tid_speeddial);
        tid_speeddial = 0;
    }

    delayEmitNumberChanged = false;

    if (!mWildcardNumber.isEmpty()) {
        // ensure we are not already over length
        if (mWildcardNumber.count(QLatin1Char('D')) <= mNumber.length()) {
            return;
        }
    }

    QString newNumber = mNumber;

    newNumber += numbers.left(numbers.count() - 1);
    QChar lastChar = numbers.at(numbers.count() - 1);

    if(speedDial) {
        if(!tap->processKeyPressEvent(lastChar)) {
            if(mWildcardNumber.isEmpty() && (int)newNumber.length() <= SPEEDDIAL_MAXDIGITS) {
                tid_speeddial = startTimer(SPEEDDIAL_TIMEOUT);
            }
            newNumber.append(lastChar);
        }
    } else {
        newNumber.append(lastChar);
    }

    if(newNumber != mNumber) {
        mNumber = newNumber;
        update();
        // if we are waiting for a release to stop speeddial, delay sending
        // numberChanged() until keyRelease, otherwise the long processing
        // time for contact filtering may cause speeddial to fire before
        // the release event is processed.
        if (tid_speeddial)
            delayEmitNumberChanged = true;
        else
            emit numberChanged( mNumber );
    }
}

void NumberDisplay::setNumber( const QString &n )
{
    mNumber = n;
    processNumberChange();
}

QString NumberDisplay::number() const
{
    if (!mWildcardNumber.isEmpty()) {
        QString n = mWildcardNumber;
        foreach (QChar c, mNumber) {
            int i = n.indexOf(QLatin1Char('D'));
            if (i != -1)
                n.replace(i, 1, c);
        }
        n.remove(QLatin1Char('D'));
        return n;
    }
    return mNumber;
}

QString NumberDisplay::wildcardNumber() const
{
    return mWildcardNumber;
}

void NumberDisplay::setWildcardNumber(const QString &n)
{
    mNumber.clear();
    mWildcardNumber = n;
    mWildcardNumber.replace(QLatin1Char('d'), QLatin1Char('D'));
    processNumberChange();
}

void NumberDisplay::clear()
{
    mNumber.clear();
    mWildcardNumber.clear();
    processNumberChange();
}

void NumberDisplay::processNumberChange()
{
    if ( tid_speeddial > 0 ) {
        killTimer(tid_speeddial);
    }
    delayEmitNumberChanged = false;
    tap->reset();
    tid_speeddial = 0;
    composeKey = QChar();
    repaint();
    emit numberChanged( number() );
}

QSize NumberDisplay::sizeHint() const
{
    QSize sh;
    QFont f = font();
    f.setBold( true );
    f.setPointSize( mFontSizes[mFontSizes.count()-1] );
    QFontMetrics fm( f );
    f.setPointSize( mFontSizes[0] );
    QFontMetrics bfm( f );
    // the height is the maximum of the height of the bounding rect of the largest font and the smallest font * 2 + margins
    QDesktopWidget *desktop = QApplication::desktop();
    sh = QSize(desktop->availableGeometry(desktop->screenNumber(this)).width(),
                qMax( bfm.lineSpacing()+mgn*4, fm.lineSpacing()*2+mgn*4 ) );
    return sh;
}

void NumberDisplay::paintEvent( QPaintEvent *e )
{
    QWidget::paintEvent( e );

    QPainter p( this );

    int x = 0, y = 0, w = width(), h = height();

    p.save();

    QPen pen = p.pen();
    p.setRenderHint(QPainter::Antialiasing);
    QBrush bg = palette().brush( (hasFocus() ? QPalette::Highlight : QPalette::Background) );
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundRect(x,y,w,h,800/w,800/h);

    if( hasFocus() )
        pen.setColor( palette().color( QPalette::HighlightedText ) );
    else
        pen.setColor( palette().color( QPalette::Text ) );
    p.setPen( pen );

    x += mgn;
    y += mgn;
    w -= (mgn*2);
    h -= (mgn*2);

    p.setBrush(Qt::NoBrush);
    p.drawRoundRect(x,y,w,h,800/w,800/h);

    p.restore();

    x += mgn;
    y += mgn;
    w -= (mgn*2);
    h -= (mgn*2);

    QFont f = font();
    f.setBold( true );

    QString n;
    QString composeString;
    composeString.append(QLatin1String("<u>"));
    composeString.append(composeKey);
    composeString.append("</u>");
    if (!mWildcardNumber.isEmpty()) {
        n = mWildcardNumber;
        foreach (QChar c, mNumber) {
            int i = n.indexOf(QLatin1Char('D'));
            if (i != -1)
                n.replace(i, 1, c);
        }
        if (!composeKey.isNull()) {
            int i = n.indexOf(QLatin1Char('D'));
            if (i != -1)
                n.replace(i, 1, composeString);
        }
        // replace remainging with tr'd string
        n.replace(QLatin1Char('D'), tr("?", "place holder for wildcard numbers in fixed dialing.  eg. 2468?? needs to be completed with two extra digits"));
    } else {
        n = mNumber;
        if(!composeKey.isNull()) {
            n.append(composeString);
        }
    }

    QTextDocument doc;
    doc.setHtml( n );
    doc.setPageSize( QSizeF( w, INT_MAX ) );

    int fontSize = 0;
    while( fontSize < mFontSizes.count() )
    {
        f.setPointSize( mFontSizes[fontSize] );
        doc.setDefaultFont( f );
        QSize size = doc.documentLayout()->documentSize().toSize();
        //we want the largest point size that will fit.
        if( size.width() <= w && size.height() <= h )
            break;
        ++fontSize;
    }

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette = palette();
    ctx.palette.setColor( QPalette::Text, (hasFocus() ? ctx.palette.color( QPalette::HighlightedText ) : ctx.palette.color( QPalette::Text )) );
    ctx.palette.setBrush( QPalette::Background, bg );
    p.translate( x, y );
    p.setClipRect( QRectF( 0, 0, w, h ) );
    doc.documentLayout()->draw( &p, ctx );
}

void NumberDisplay::keyReleaseEvent( QKeyEvent *e )
{
    if (delayEmitNumberChanged) {
        delayEmitNumberChanged = false;
        emit numberChanged(number());
    }

    if(!e->isAutoRepeat() && e->text().length())
        tap->processKeyReleaseEvent(e->text()[0]);

    if ( !e->isAutoRepeat() && tid_speeddial ) {
        killTimer(tid_speeddial);
        tid_speeddial = 0;
    }
}

void NumberDisplay::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == tid_speeddial ) {
        killTimer(tid_speeddial);
        tid_speeddial = 0;
        delayEmitNumberChanged = false;
        if ( mWildcardNumber.isEmpty() && (int)mNumber.length() <= SPEEDDIAL_MAXDIGITS )
            emit speedDialed(mNumber);
    }
}

void NumberDisplay::keyPressEvent( QKeyEvent *e )
{
    if(e->isAutoRepeat()) {
        e->accept();
        return;
    }

    int key = e->key();
    if(e->text().length() && tap->processKeyPressEvent(e->text()[0])) {
        e->accept();
        return;
    }

    switch( key )
    {
#ifdef QTOPIA_VOIP
        case Qt::Key_At:
        case Qt::Key_Colon:
        case Qt::Key_AsciiCircum:
        case Qt::Key_Underscore:
        case Qt::Key_cent:
        case Qt::Key_sterling:
        case Qt::Key_hyphen:
        case Qt::Key_Atilde:
        case Qt::Key_Ntilde:
        case Qt::Key_AsciiTilde:
        case Qt::Key_Otilde:
        case Qt::Key_Minus:
        case Qt::Key_Period:
        case Qt::Key_A:
        case Qt::Key_B:
        case Qt::Key_C:
        case Qt::Key_D:
        case Qt::Key_E:
        case Qt::Key_F:
        case Qt::Key_G:
        case Qt::Key_H:
        case Qt::Key_I:
        case Qt::Key_J:
        case Qt::Key_K:
        case Qt::Key_L:
        case Qt::Key_M:
        case Qt::Key_N:
        case Qt::Key_O:
        case Qt::Key_P:
        case Qt::Key_Q:
        case Qt::Key_R:
        case Qt::Key_S:
        case Qt::Key_T:
        case Qt::Key_U:
        case Qt::Key_V:
        case Qt::Key_W:
        case Qt::Key_X:
        case Qt::Key_Y:
        case Qt::Key_Z:
#endif
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        case Qt::Key_NumberSign:
        {
            // If the device has a physical hook, such as on desk phones, then '#'
            // is used to indicate end of number.  Otherwise it is a normal digit.
            emit numberKeyPressed( key );
            if ( key != Qt::Key_NumberSign || !Qtopia::hasKey( Qtopia::Key_Hook ) )
               appendNumber( QString(QChar(key)).toLower() );
            e->accept();
            break;
        }
        case Qt::Key_Back:
        {
            if( Qtopia::hasKey( Qt::Key_Backspace ) )
                emit hangupActivated();
            else
                backspace();
            e->accept();
            break;
        }
        case Qt::Key_Backspace:
        {
            backspace();
            e->accept();
            break;
        }
        case Qt::Key_No:
        {
            if( Qtopia::hasKey( Qt::Key_Backspace ) || Qtopia::hasKey( Qt::Key_Back ) )
                emit hangupActivated();
            else
                backspace();
            e->accept();
            break;
        }
        case Qt::Key_Yes:
        case Qt::Key_Call:
        case Qt::Key_Select:
        {
            QString num = number();
            if( !num.isEmpty() ) {
                if (!mWildcardNumber.isEmpty() && mWildcardNumber.count(QLatin1Char('D')) != mNumber.length()) {
                    QAbstractMessageBox::information(this, tr("Incomplete number"),
                            tr("Phone number must be completed before dialing"));
                } else {
                    emit numberSelected( num );
                }
            } else {
                emit hangupActivated();
            }
            break;
        }
        case Qt::Key_Hangup:
        case Qt::Key_Flip:
        {
            emit hangupActivated();
            e->accept();
            break;
        }
        default:
        {
            e->ignore();
        }
    }
}

void NumberDisplay::backspace()
{
    if (composeKey.isNull())
        setNumber( mNumber.left( mNumber.length()-1 ) );
    else
        processNumberChange();  // Clear compose key only.
    if( mNumber.isEmpty() && mWildcardNumber.isEmpty() )
        emit hangupActivated();
}

void NumberDisplay::composeTap(const QChar &ch)
{
    composeKey = ch;
    update();
}

void NumberDisplay::completeTap(const QChar &ch)
{
    composeKey = QChar();
    mNumber.append(ch);
    emit numberChanged(number());
    update();
}

#include "numberdisplay.moc"

