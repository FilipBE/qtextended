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

#include "modemnetwork.h"

#include <qtopiaapplication.h>
#include <qwaitwidget.h>
#include <qsoftmenubar.h>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMenu>
#include <QLabel>
#include <QSpinBox>
#include <QTimer>
#include <QTranslatableSettings>
#include <QtopiaItemDelegate>
#include <QKeyEvent>
#include <QScrollArea>
#include <QDesktopWidget>

//------------------------------------------------
// Code similar to that in phonesettings.h
// Modified to work across 2 lines (\n separated)
// reasonably optimised for fast execution,
// although sacrifices space to do so.

class TwoLineFloatingTextListItem : public QListWidgetItem
{
public:
    TwoLineFloatingTextListItem ( QListWidget * parent = 0, int type = QListWidgetItem::Type );
    TwoLineFloatingTextListItem ( const QString & text, QListWidget * parent = 0, int type = QListWidgetItem::Type );
    TwoLineFloatingTextListItem ( const QIcon & icon, const QString & text, QListWidget * parent = 0, int type = QListWidgetItem::Type );
    TwoLineFloatingTextListItem ( const QListWidgetItem & other );
    bool operator<( const QListWidgetItem& other ) const;
};

TwoLineFloatingTextListItem::TwoLineFloatingTextListItem ( QListWidget * parent, int type )
: QListWidgetItem( parent, type ) {}

TwoLineFloatingTextListItem::TwoLineFloatingTextListItem ( const QString & text, QListWidget * parent, int type )
: QListWidgetItem( text, parent, type ) {}

TwoLineFloatingTextListItem::TwoLineFloatingTextListItem ( const QIcon & icon, const QString & text, QListWidget * parent, int type )
: QListWidgetItem( icon, text, parent, type ) {}

TwoLineFloatingTextListItem::TwoLineFloatingTextListItem ( const QListWidgetItem & other )
: QListWidgetItem( other )
{
    this->data( Qt::UserRole ) = other.data( Qt::UserRole);
    this->data( Qt::UserRole + 1 ) = other.data( Qt::UserRole + 1 );
}

bool TwoLineFloatingTextListItem::operator<( const QListWidgetItem& other ) const
{
    // first, compare the country
    QString thisFirst = this->data( Qt::UserRole + 1 ).toString();
    QString otherFirst = other.data( Qt::UserRole + 1 ).toString();

    if ( thisFirst < otherFirst )
        return true;

    if ( thisFirst > otherFirst )
        return false;

    // if the country is equal, compare the network name
    if ( this->data( Qt::UserRole ).toString() < other.data( Qt::UserRole ).toString() )
        return true;

    return false;
}

class TwoLineFloatingTextList : public QListWidget
{
    Q_OBJECT

public:
    TwoLineFloatingTextList( QWidget *parent, int w );

protected:
    void keyPressEvent( QKeyEvent *e );

protected slots:
    void newCurrentRow( int row );
    void floatText();

private:
    QTimer *timer;
    QFontMetrics *fm;

    int oldRow;                // the previously selected item.
    int firstLineLastChar;     // last char of first line displayed
    int secondLineLastChar;    // last char of second line displayed
    int firstLineMaxIndex;     // last char index of first line
    int secondLineMaxIndex;    // last char index of second line
    bool needToFloatFirst;     // whether we need to float the first line
    bool needToFloatSecond;    // whether we need to float the second line
    QString firstLine;         // complete text of first line
    QString secondLine;        // complete text of second line
    QString displayFirst;      // current shown text of first line
    QString displaySecond;     // current shownt ext of second line
    int firstLineWidth;        // current width of first line
    int secondLineWidth;       // current width of second line
    int availableWidth;        // width available to display in

    bool finishedFloating;     // whether we have finished or not.
};

TwoLineFloatingTextList::TwoLineFloatingTextList( QWidget *parent, int w )
: QListWidget( parent ), availableWidth( w )
{
    timer = new QTimer( this );

    oldRow = 0;
    firstLineLastChar = 0;
    secondLineLastChar = 0;
    firstLineMaxIndex = 0;
    secondLineMaxIndex = 0;
    needToFloatFirst = false;
    needToFloatSecond = false;
    firstLine = "";
    secondLine = "";
    displayFirst = "";
    displaySecond = "";
    firstLineWidth = 0;
    secondLineWidth = 0;
    finishedFloating = true;

    fm = new QFontMetrics(font());

    connect( this, SIGNAL(currentRowChanged(int)), this, SLOT(newCurrentRow(int)) );
    connect( timer, SIGNAL(timeout()), this, SLOT(floatText()) );

    setItemDelegate(new QtopiaItemDelegate);
    setFrameStyle(NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void TwoLineFloatingTextList::newCurrentRow( int row )
{
    if ( timer->isActive() )
        timer->stop();

    if ( row < 0 )
        return;

    // first, regenerate the text of the old row if required.
    if ( oldRow != row && ( needToFloatFirst || needToFloatSecond ) ) {
        QListWidgetItem *old = item( oldRow );
        old->setText( old->data( Qt::UserRole ).toString() + "\n"
                    + old->data( Qt::UserRole + 1 ).toString() );
    }

    // extract each line from this item.
    oldRow = row;
    QListWidgetItem *current = item( row );
    firstLine = current->data( Qt::UserRole ).toString();
    secondLine = current->data( Qt::UserRole + 1 ).toString();

    // Determine whether we need to float the lines.
    if ( fm->width( firstLine ) >= availableWidth ) {
        needToFloatFirst = true;
    } else {
        needToFloatFirst = false;
    }

    if ( fm->width( secondLine ) >= availableWidth ) {
        needToFloatSecond = true;
    } else {
        needToFloatSecond = false;
    }

    // either display the line, or float it
    if ( needToFloatFirst || needToFloatSecond ) {
        firstLineLastChar = 0;
        secondLineLastChar = 0;
        displayFirst = "";
        displaySecond = "";
        firstLineWidth = 0;
        secondLineWidth = 0;
        firstLineMaxIndex = firstLine.length() - 1;
        secondLineMaxIndex = secondLine.length() - 1;
        timer->start( 1200 );
    } else {
        current->setText( firstLine + "\n" + secondLine );
    }
}

void TwoLineFloatingTextList::floatText()
{
    QListWidgetItem *current = currentItem();
    if ( !current ) {
        return;
    }

    // modify the timer interval if started.
    if ( timer->interval() == 1200 ) {
        timer->stop();
        timer->start( 500 );
    }

    // this ensures we display the last char for 500 msec
    finishedFloating = true;

    if ( needToFloatFirst ) {
        // build the initial string if required.
        if ( firstLineLastChar == 0 ) {
            while ( firstLineWidth < availableWidth && firstLineLastChar <= (firstLineMaxIndex+1) ) {
                QChar curr = firstLine.at( firstLineLastChar++ );
                displayFirst += curr;
                firstLineWidth += fm->width( curr );
            }
            firstLineLastChar--;
            finishedFloating = false;
        } else {
            // alternatively, float along one character
            if ( firstLineLastChar < firstLineMaxIndex ) {
                // more characters to display
                finishedFloating = false;
                firstLineLastChar += 1;
                QChar curr = firstLine.at( firstLineLastChar );
                firstLineWidth += fm->width( curr );
                displayFirst += curr;
                while ( firstLineWidth >= availableWidth ) {
                    firstLineWidth -= fm->width( displayFirst.at( 0 ) );
                    displayFirst.remove( 0, 1 );
                }
            }
        }
    } else {
        displayFirst = firstLine;
    }

    if ( needToFloatSecond ) {
        // build the initial string if required.
        if ( secondLineLastChar == 0 ) {
            while ( secondLineWidth < availableWidth && secondLineLastChar < (secondLineMaxIndex+1) ) {
                QChar curr = secondLine.at( secondLineLastChar++ );
                displaySecond += curr;
                secondLineWidth += fm->width( curr );
            }
            secondLineLastChar--;
            finishedFloating = false;
        } else {
            // alternatively, float along one character
            if ( secondLineLastChar < secondLineMaxIndex ) {
                // more characters to display
                finishedFloating = false;
                secondLineLastChar += 1;
                QChar curr = secondLine.at( secondLineLastChar );
                secondLineWidth += fm->width( curr );
                displaySecond += curr;
                while ( secondLineWidth >= availableWidth ) {
                    secondLineWidth -= fm->width( displaySecond.at( 0 ) );
                    displaySecond.remove( 0, 1 );
                }
            }
        }
    } else {
        displaySecond = secondLine;
    }

    // If we have waited at the end for 500 msec, then return
    if ( finishedFloating ) {
        newCurrentRow( currentRow() ); // refloat the line.
        return;
    }

    // display the lines for this TwoLineFloatingTextListItem
    current->setText( displayFirst + "\n" + displaySecond );

    // Display the current lines for 500 msec.
    if ( !timer->isActive() )
        timer->start( 500 );
}

void TwoLineFloatingTextList::keyPressEvent( QKeyEvent *e )
{
    if ( !hasEditFocus() ) {
        QListWidget::keyPressEvent( e );
        return;
    }

    int curRow = currentRow();
    if ( e->key() == Qt::Key_Up ) {
        if ( curRow == 0 )
            setCurrentRow( count() - 1 );
        else
            setCurrentRow( curRow - 1 );
    } else if ( e->key() == Qt::Key_Down ) {
        if ( curRow == count() - 1 )
            setCurrentRow( 0 );
        else
           setCurrentRow( curRow + 1 );
    } else if ( e->key() == Qt::Key_Back ) {
        setEditFocus( false );
    } else {
        QListWidget::keyPressEvent( e );
    }
}
//----------------------------------------------------------------------------

//------------------------------------------------
// Code similar to code from phonesettings.cpp -
// modified to work for a group of radiobuttons.
class FloatingTextRadioButton : public QRadioButton
{
    Q_OBJECT

public:
    FloatingTextRadioButton( QWidget *parent, int w, QString t );
    QString getCompleteText();

public slots:
    void focusInEvent( QFocusEvent *event );

protected slots:
    void floatText();
    void showItemText();

private:
    QTimer *timer;
    QString completeText;
    int availableWidth;
    int maxChars;
    static int lastCharIndex; // index of the last character shown
};

int FloatingTextRadioButton::lastCharIndex = 0;

FloatingTextRadioButton::FloatingTextRadioButton( QWidget *parent, int w, QString t )
: QRadioButton( parent )
{
    timer = new QTimer( this );
    availableWidth = w;
    completeText = t;

    // first, calculate the maximum number of characters allowed.
    int totalWidth = 0;
    QFontMetrics fm(font());
    maxChars = 0;
    while ( maxChars < completeText.length() && totalWidth < availableWidth )
    {
        totalWidth += fm.width(completeText.at(maxChars));
        maxChars += 1;
    }

    if ( totalWidth >= availableWidth || maxChars < completeText.length() )
        maxChars -= 2; // to ensure that we don't go over our max width.

    showItemText();
    connect( timer, SIGNAL(timeout()), this, SLOT(floatText()) );
}

QString FloatingTextRadioButton::getCompleteText()
{
    return completeText;
}

void FloatingTextRadioButton::showItemText()
{
    QString shownText = completeText.mid( 0, maxChars );
    setText( shownText );
}

void FloatingTextRadioButton::focusInEvent( QFocusEvent* event )
{
    if ( timer->isActive() )
        timer->stop();

    if ( completeText.length() > maxChars )
        timer->singleShot( 800, this, SLOT(floatText()) );

    Q_UNUSED( event );
}

void FloatingTextRadioButton::floatText()
{
    if ( !timer->isActive() ) {
        lastCharIndex = maxChars - 1;
        timer->start( 500 );
    }

    QFontMetrics fm(font());
    QString shownText = completeText.mid( lastCharIndex - maxChars + 1, maxChars );
    //check if the new string would fit the screen
    while ( availableWidth < fm.width( shownText ) ) // if it doesn't fit
        shownText = shownText.mid( 1 );     // remove the first character

    setText( shownText );
    lastCharIndex++;
    if ( lastCharIndex == completeText.length() ) {
        // reset our index, and stop floating the text.
        lastCharIndex = maxChars - 1;
        timer->stop();
    }
}
//------------------------------------------------

ModemNetworkRegister::ModemNetworkRegister( QWidget *parent )
    : QListWidget( parent )
{
    init();

    connect( this, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(operationSelected(QListWidgetItem*)) );
    if ( m_client ) {
        connect( m_client, SIGNAL(availableOperators(QList<QNetworkRegistration::AvailableOperator>)),
                this, SLOT(selectOperator(QList<QNetworkRegistration::AvailableOperator>)) );
        connect( m_client, SIGNAL(setCurrentOperatorResult(QTelephony::Result)),
                 this, SLOT(setCurrentOperatorResult(QTelephony::Result)) );
    }
    if ( m_bandSel ) {
        connect( m_bandSel, SIGNAL(band(QBandSelection::BandMode,QString)),
                this, SLOT(band(QBandSelection::BandMode,QString)) );
        connect( m_bandSel, SIGNAL(bands(QStringList)),
                this, SLOT(selectBand(QStringList)) );
        connect( m_bandSel, SIGNAL(setBandResult(QTelephony::Result)),
                this, SLOT(setBandResult(QTelephony::Result)) );
        m_bandSel->requestBand();
    }
}

ModemNetworkRegister::~ModemNetworkRegister()
{
}

void ModemNetworkRegister::init()
{
    setItemDelegate( new QtopiaItemDelegate );
    setFrameStyle( QFrame::NoFrame );

    m_client = new QNetworkRegistration( "modem", this );
    m_bandSel = new QBandSelection( "modem", this );

    setObjectName( "modem" );

    (void) new QListWidgetItem( tr( "Current status" ), this );
    (void) new QListWidgetItem( tr( "Search mode" ), this );
    (void) new QListWidgetItem( tr( "Select operator" ), this );
    (void) new QListWidgetItem( tr( "Preferred networks" ), this );
    if ( m_bandSel->available() )
        (void) new QListWidgetItem( tr( "Select band" ), this );

    setCurrentRow( 0 );

    m_waitWidget = new QWaitWidget( this );
    m_waitWidget->setCancelEnabled( true );
}

void ModemNetworkRegister::operationSelected( QListWidgetItem * )
{
    switch ( currentRow() ) {
    case 0:
        showCurrentOperator();
        break;
    case 1:
        selectSearchMode();
        break;
    case 2:
        if ( !m_waitWidget->isVisible() ) {
            m_client->requestAvailableOperators();
            m_waitWidget->show();
        }
        break;
    case 3:
        preferredOperators();
        break;
    case 4:
        if ( !m_waitWidget->isVisible() ) {
            m_bandSel->requestBands();
            m_waitWidget->show();
        }
        break;
    default:
        break;
    }
}

static QMap<QString,QString> countries;
static QString countryForOperatorId( const QString& id )
{
    if ( !id.startsWith( QChar('2') ) )
        return QString();
    if ( countries.isEmpty() ) {
        QTranslatableSettings settings( "Trolltech", "GsmOperatorCountry" );
        settings.beginGroup( "Countries" );
        foreach ( QString key, settings.allKeys() )
            countries.insert( key, settings.value( key ).toString() );
    }
    return countries.value( id.mid(1,3) );
}

void ModemNetworkRegister::showCurrentOperator()
{
    QDialog dlg( this );
    dlg.setWindowTitle( tr( "Current Network" ) );
    QVBoxLayout layout( &dlg );

    // operator
    QLabel lbl( "<b>" + tr( "Operator:" ) + "</b>", &dlg );
    QLabel name( m_client->currentOperatorName(), &dlg );
    // country
    QLabel lblc( "<b>" + tr( "Country:" ) + "</b>", &dlg );
    QLabel country
        ( countryForOperatorId( m_client->currentOperatorId() ), &dlg );
    // technology
    QLabel lbl1( "<b>" + tr( "Technology:" ) + "</b>", &dlg );
    QString state = m_client->currentOperatorTechnology().isEmpty() ?
        tr( "Unknown" ) : m_client->currentOperatorTechnology();
    QLabel technology( state, &dlg );
    // registration
    QLabel lbl2( "<b>" + tr( "Registration state:" ) + "</b>", &dlg );
    switch ( m_client->registrationState() ) {
    case QTelephony::RegistrationNone:
        state = tr( "Not registered" );
        break;
    case QTelephony::RegistrationHome:
        state = tr( "Registered to home network" );
        break;
    case QTelephony::RegistrationSearching:
        state = tr( "Searching" );
        break;
    case QTelephony::RegistrationDenied:
        state = tr( "Registration denied" );
        break;
    case QTelephony::RegistrationUnknown:
        state = tr( "Registered but unknown state" );
        break;
    case QTelephony::RegistrationRoaming:
        state = tr( "Roaming" );
        break;
    default:
        state = tr( "Unknown" );
        break;
    }
    QLabel regState( state, &dlg );
    regState.setWordWrap( true );
    // mode
    QLabel lbl3( "<b>" + tr( "Mode:" ) + "</b>", &dlg );
    switch( m_client->currentOperatorMode() ) {
    case QTelephony::OperatorModeAutomatic:
        state = tr( "Automatic" );
        break;
    case QTelephony::OperatorModeManual:
        state = tr( "Manual" );
        break;
    case QTelephony::OperatorModeManualAutomatic:
        state = tr( "Manual/Automatic");
        break;
    default:
        state = tr( "Other state" );
        break;
    }
    QLabel mode( state, &dlg );
    layout.addWidget( &lbl );
    layout.addWidget( &name );
    layout.addWidget( &lblc );
    layout.addWidget( &country );
    layout.addWidget( &lbl1 );
    layout.addWidget( &technology );
    layout.addWidget( &lbl2 );
    layout.addWidget( &regState );
    layout.addWidget( &lbl3 );
    layout.addWidget( &mode );
    // band
    if ( m_bandSel->available() ) {
        QLabel lbl4( "<b>" + tr( "Band:" ) + "</b>", &dlg );
        state = m_curBandMode == QBandSelection::Automatic ? tr( "Automatic" ) : tr( "Manual/" ) + m_curBand;
        QLabel band( state, &dlg );
        layout.addWidget( &lbl4 );
        layout.addWidget( &band );
    }
    layout.addStretch();

    QtopiaApplication::execDialog( &dlg );
}

void ModemNetworkRegister::band( QBandSelection::BandMode mode, const QString &value )
{
    m_curBandMode = mode;
    m_curBand = value;
}

void ModemNetworkRegister::selectBand( const QStringList& bandList )
{
    if ( m_waitWidget->isVisible() )
        m_waitWidget->hide();
    else // assume operation is cancelled by the user by closing wait widget.
        return;

    QDialog *dlg = new QDialog( this );
    dlg->setWindowTitle( tr( "Select band" ) );
    QVBoxLayout *dlayout = new QVBoxLayout( dlg );
    QWidget *container = new QWidget;
    QVBoxLayout *clayout = new QVBoxLayout( container );
    QButtonGroup *group = new QButtonGroup( container );
    int id = 0;

    // calculate the available width for the FloatingTextRadioButtons
    int leftMargin = 0;
    int rightMargin = 0;
    int is = style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth) + 
            style()->pixelMetric(QStyle::PM_RadioButtonLabelSpacing) +
            style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    dlayout->getContentsMargins( &leftMargin, 0, &rightMargin, 0 );
    is += leftMargin + rightMargin;
    clayout->getContentsMargins( &leftMargin, 0, &rightMargin, 0 );
    is += leftMargin + rightMargin;
    QDesktopWidget *desktop = QApplication::desktop();
    int availWidth = desktop->availableGeometry(desktop->screenNumber(container)).width() - is;

    // automatic
    FloatingTextRadioButton *autoBand = new FloatingTextRadioButton( container, availWidth, tr( "Automatic" ) );
    if ( m_curBandMode == QBandSelection::Automatic )
        autoBand->setChecked( true );
    group->addButton( autoBand, id++ );
    clayout->addWidget( autoBand );

    // available bands
    FloatingTextRadioButton *btn;
    foreach ( QString band, bandList ) {
        btn = new FloatingTextRadioButton( container, availWidth, band );
        if ( m_curBandMode == QBandSelection::Manual && band == m_curBand )
            btn->setChecked( true );
        group->addButton( btn, id++ );
        clayout->addWidget( btn );
    }

    id = group->checkedId();
    if ( id < 0 ) { // if nothing is checked, check and activate Auto option
        id = group->id( autoBand );
        autoBand->setChecked( true );
        m_bandSel->setBand( QBandSelection::Automatic, QString() );
    }

    QScrollArea *scrollArea = new QScrollArea( dlg );
    scrollArea->setFocusPolicy(Qt::NoFocus);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    dlayout->addWidget( scrollArea );

    if ( QtopiaApplication::execDialog( dlg ) ) {
        if ( id == group->checkedId() ) // no change
            return;

        if ( autoBand->isChecked() )
            m_bandSel->setBand( QBandSelection::Automatic, QString() );
        else
            m_bandSel->setBand( QBandSelection::Manual, 
                    ((FloatingTextRadioButton*)(group->checkedButton()))->getCompleteText() );
    }
}

void ModemNetworkRegister::setBandResult( QTelephony::Result result )
{
    if ( result == QTelephony::OK )
        m_bandSel->requestBand();
    else {
        QMessageBox::warning( this, tr( "Error Occurred" ),
                "<qt>" + tr( "Band selection failed. Please try again."
                    "If the problem persists, please consult your network operator or phone manufacturer." ) + "</qt>" );
    }
}

void ModemNetworkRegister::selectSearchMode()
{
    QDialog *dlg = new QDialog( this );
    dlg->setWindowTitle( tr( "Search Mode" ) );
    QVBoxLayout *layout = new QVBoxLayout( dlg );
    QButtonGroup *group = new QButtonGroup( dlg );
    QRadioButton *automatic = new QRadioButton( tr( "Automatic" ), dlg );
    group->addButton( automatic, (int)QTelephony::OperatorModeAutomatic );
    layout->addWidget( automatic );
    QRadioButton *manual = new QRadioButton( tr( "Manual" ), dlg );
    group->addButton( manual, (int)QTelephony::OperatorModeManual );
    layout->addWidget( manual );
    layout->addStretch();

    QTelephony::OperatorMode originalMode = m_client->currentOperatorMode();
    if ( originalMode == QTelephony::OperatorModeAutomatic )
        automatic->setChecked( true );
    else
        manual->setChecked( true );

    if ( QtopiaApplication::execDialog( dlg ) ) {
        if ( (int)originalMode == group->checkedId() )
            return;
        if ( automatic->isChecked() )
            m_client->setCurrentOperator( QTelephony::OperatorModeAutomatic );
        else
            m_client->setCurrentOperator( QTelephony::OperatorModeManual,
                    m_client->currentOperatorId(), m_client->currentOperatorTechnology() );
    }
}

void ModemNetworkRegister::selectOperator( const QList<QNetworkRegistration::AvailableOperator> &result )
{
    if ( m_waitWidget->isVisible() )
        m_waitWidget->hide();
    else // assume operation is cancelled by the user by closing wait widget.
        return;

    if ( result.count() == 0 ) {
        if ( QMessageBox::question( this, tr( "No operator found" ),
                tr( "<qt>Would you like to search again?</qt>" ),
                QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) {
                m_waitWidget->show();
                m_client->requestAvailableOperators();
        }
        return;
    }

    m_result = result;

    m_opDlg = new QDialog( this );
    m_opDlg->setWindowTitle( tr( "Available operators" ) );
    QVBoxLayout *layout = new QVBoxLayout( m_opDlg );

    // calculate the available width for the TwoLineFloatingTextLists
    int iconMetric = QApplication::style()->pixelMetric( QStyle::PM_SmallIconSize );
    int leftMargin = 0;
    int rightMargin = 0;
    layout->getContentsMargins( &leftMargin, 0, &rightMargin, 0 );
    int is = style()->pixelMetric(QStyle::PM_ScrollBarExtent) +
            leftMargin + rightMargin + iconMetric + 24; // magic number
    QDesktopWidget *desktop = QApplication::desktop();
    int availWidth = desktop->availableGeometry(desktop->screenNumber(m_opDlg)).width() - is;

    m_opList = new TwoLineFloatingTextList( m_opDlg, availWidth );
    m_opList->setIconSize( QSize( iconMetric, iconMetric ) );
    m_opList->setUniformItemSizes( true );
    m_opList->setAlternatingRowColors( true );
    layout->addWidget( m_opList );
    QtopiaApplication::setMenuLike( m_opDlg, true );

    TwoLineFloatingTextListItem *item = 0;
    QString utran = tr("3G", "3g/umts/utran network");
    QString gsm = tr("GSM", "GSM network");
    foreach( QNetworkRegistration::AvailableOperator op, result ) {
        QString name = op.name;
        if ( op.technology == "GSM" ) //no tr
            name = name + " (" + gsm  +")" ;
        else if ( op.technology == "UTRAN" )
            name = name + " (" + utran  +")" ;
        QString country = countryForOperatorId( op.id );
        if ( !country.isEmpty() )
            country = "[ " + country + " ]";
        else
            country = "[ " + tr( "Unknown" ) + " ]";

        item = new TwoLineFloatingTextListItem( QString(name + "\n" + country), m_opList );
        item->setData( Qt::UserRole, name );
        item->setData( Qt::UserRole + 1, country );
        switch ( op.availability ) {
        case QTelephony::OperatorUnavailable:
            item->setIcon( QIcon( ":icon/close" ) );
            break;
        case QTelephony::OperatorAvailable:
            item->setIcon( QIcon( ":icon/globe" ) );
            break;
        case QTelephony::OperatorCurrent:
            item->setSelected( true );
            item->setIcon( QIcon( ":icon/done" ) );
            m_originalOp = item;
            break;
        case QTelephony::OperatorForbidden:
            item->setIcon( QIcon( ":icon/uninstall" ) );
            break;
        default:
            break;
        }
    }

    connect( m_opList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(acceptOpDlg()) );

    m_opDlg->showMaximized();
    QtopiaApplication::execDialog( m_opDlg );
}

void ModemNetworkRegister::acceptOpDlg()
{
    if ( m_originalOp == m_opList->currentItem() ) {
        m_opDlg->accept();
        return;
    }

    int i = m_opList->currentRow();

    if ( m_result.at( i ).availability == QTelephony::OperatorForbidden ) {
        QMessageBox::warning( this, tr( "Forbidden" ),
                "<qt>" + tr( "You cannot use a forbidden operator. "
                    "Please select an available operator." ) + "</qt>" );
        return;
    }

    // cannot do manual registraion in automatic mode
    // ask if the user wants the mode to be changed.
    if ( m_client->currentOperatorMode() == QTelephony::OperatorModeAutomatic ) {
        if ( QMessageBox::question( this, tr( "Automatic Mode" ),
                tr( "<qt>Would you wish to change search mode to Manual"
                    " and register to the selected operator?</qt>" ),
                QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) {
            m_client->setCurrentOperator( QTelephony::OperatorModeManual,
                m_result.at( i ).id,
                m_result.at( i ).technology );
        }
    } else { // manual mode
        m_client->setCurrentOperator( m_client->currentOperatorMode(),
                m_result.at( i ).id,
                m_result.at( i ).technology );
    }

    m_opDlg->accept();
}

void ModemNetworkRegister::setCurrentOperatorResult( QTelephony::Result result )
{
    if ( result != QTelephony::OK ) {
        QMessageBox::warning( this, tr( "Select operator" ),
                "<qt>" + tr( "Failed to register to the selected network operator." ) +
                "</qt>" );
    }
}

void ModemNetworkRegister::preferredOperators()
{
    PreferredOperatorsDialog dlg( this );
    dlg.showMaximized();
    QtopiaApplication::execDialog( &dlg );
}

//-----------------------------------------------------
PreferredOperatorsDialog::PreferredOperatorsDialog( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), m_addNetworkDlg(0)
{
    init();

    if ( m_PNOClient ) {
        connect( m_PNOClient, SIGNAL(operatorNames(QList<QPreferredNetworkOperators::NameInfo>)),
                this, SLOT(operatorNames(QList<QPreferredNetworkOperators::NameInfo>)) );
        connect( m_PNOClient, SIGNAL(preferredOperators(QPreferredNetworkOperators::List,
                        const QList<QPreferredNetworkOperators::Info>&)),
                this, SLOT(preferredOperators(QPreferredNetworkOperators::List,
                        const QList<QPreferredNetworkOperators::Info>&)) );
    }
    connect( m_list, SIGNAL(currentRowChanged(int)),
            this, SLOT(rowChanged(int)) );
}

PreferredOperatorsDialog::~PreferredOperatorsDialog()
{
}

void PreferredOperatorsDialog::showEvent( QShowEvent *e )
{
    QTimer::singleShot( 0, this, SLOT(requestOperatorInfo()) );
    QDialog::showEvent( e );
}

void PreferredOperatorsDialog::requestOperatorInfo()
{
    m_PNOClient->requestOperatorNames();
    m_PNOClient->requestPreferredOperators( QPreferredNetworkOperators::Current );
    m_waitWidget->show();
}

void PreferredOperatorsDialog::init()
{
    setWindowTitle( tr( "Preferred Networks" ) );
    QVBoxLayout *layout = new QVBoxLayout( this );

    // calculate the available width for the TwoLineFloatingTextLists
    int leftMargin = 0;
    int rightMargin = 0;
    layout->getContentsMargins( &leftMargin, 0, &rightMargin, 0 );
    int is = style()->pixelMetric(QStyle::PM_ScrollBarExtent) +
            leftMargin + rightMargin + 18; // magic number
    QDesktopWidget *desktop = QApplication::desktop();
    int availWidth = desktop->availableGeometry(desktop->screenNumber(this)).width() - is;

    m_list = new TwoLineFloatingTextList( this, availWidth );
    layout->addWidget( m_list );

    QMenu *contextMenu = QSoftMenuBar::menuFor( this );
    m_add = contextMenu->addAction( QIcon( ":icon/new" ), tr( "Add..." ), this, SLOT(addNetwork()) );
    m_remove = contextMenu->addAction( QIcon( ":icon/trash" ), tr( "Remove" ), this, SLOT(removeNetwork()) );
    m_up = contextMenu->addAction( QIcon( ":icon/up" ), tr( "Move up" ), this, SLOT(moveUp()) );
    m_down = contextMenu->addAction( QIcon( ":icon/down" ), tr( "Move down" ), this, SLOT(moveDown()) );
    QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );

    m_waitWidget = new QWaitWidget( this );
    m_waitWidget->setCancelEnabled( true );
    connect( m_waitWidget, SIGNAL(cancelled()), this, SLOT(reject()) );
    m_PNOClient = new QPreferredNetworkOperators( "modem", this );
}

void PreferredOperatorsDialog::operatorNames( const QList<QPreferredNetworkOperators::NameInfo> &names )
{
    m_operatorNames = names;
    if ( m_currentOpers.count() > 0 ) {
        m_waitWidget->hide();
        populateList();
    }
}

void PreferredOperatorsDialog::preferredOperators( QPreferredNetworkOperators::List,
        const QList<QPreferredNetworkOperators::Info> &names )
{
    m_currentOpers = names; // will contain latest user selection
    m_originalOpers = names; // will be used to compare changes when dialog accepted
    if ( m_operatorNames.count() > 0 ) {
        m_waitWidget->hide();
        populateList();
    }
}

void PreferredOperatorsDialog::populateList()
{
    QList<QPreferredNetworkOperators::Info> resolved =
        m_PNOClient->resolveNames( m_currentOpers, m_operatorNames );
    for ( int i = 0; i < resolved.count(); i++ ) {
        QString name = resolved.at( i ).name;
        QString country = countryForOperatorId
            ( "2" + QString::number( resolved.at( i ).id ) );
        if ( !country.isEmpty() )
            country = "[ " + country + " ]";
        else
            country = "[ " + tr( "Unknown" ) + " ]";
        TwoLineFloatingTextListItem *item = new TwoLineFloatingTextListItem( QString(name + "\n" + country), m_list );
        item->setData( Qt::UserRole, name );
        item->setData( Qt::UserRole + 1, country );
    }
    if ( resolved.count() > 0 )
        m_list->setCurrentRow( 0 );

    if ( !m_addNetworkDlg )
        initAddNetworkDlg();
}

void PreferredOperatorsDialog::initAddNetworkDlg()
{
    m_addNetworkDlg = new QDialog( this );
    m_addNetworkDlg->setModal( true );
    m_addNetworkDlg->setWindowTitle( tr( "Select Network" ) );
    QVBoxLayout *layout = new QVBoxLayout( m_addNetworkDlg );

    // calculate the available width for the TwoLineFloatingTextLists
    int leftMargin = 0;
    int rightMargin = 0;
    layout->getContentsMargins( &leftMargin, 0, &rightMargin, 0 );
    int is = style()->pixelMetric(QStyle::PM_ScrollBarExtent) +
            leftMargin + rightMargin + 18; // magic number
    QDesktopWidget *desktop = QApplication::desktop();
    int availWidth = desktop->availableGeometry(desktop->screenNumber(m_addNetworkDlg)).width() - is;

    m_addNetworkList = new TwoLineFloatingTextList( m_addNetworkDlg, availWidth );
    layout->addWidget( m_addNetworkList );

    for ( int i = 0; i < m_operatorNames.count(); i++ ) {
        // filter out network that are alrealy in the preferred list
        if ( isPreferred( m_operatorNames.at( i ).id ) )
            continue;

        // display id number of the networks with the same name
        QString name = m_operatorNames.at( i ).name;
        if ( ( i > 0 && m_operatorNames.at( i - 1 ).name == name )
          || i != m_operatorNames.count() - 1 && m_operatorNames.at( i + 1 ).name == name )
            name = name + "(" + QString::number( m_operatorNames.at( i ).id ) + ")";
        QString country = countryForOperatorId
            ( "2" + QString::number( m_operatorNames.at( i ).id ) );
        if ( !country.isEmpty() )
            country = "[ " + country + " ]";
        else
            country = "[ " + tr( "Unknown" ) + " ]";

        TwoLineFloatingTextListItem *item = new TwoLineFloatingTextListItem( QString(name + "\n" + country), m_addNetworkList, m_operatorNames.at( i ).id );
        item->setData( Qt::UserRole, name );
        item->setData( Qt::UserRole + 1, country );
    }

    // now sort the list
    m_addNetworkList->sortItems();

    connect( m_addNetworkList, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(networkSelected(QListWidgetItem*)) );

    QtopiaApplication::setMenuLike( m_addNetworkDlg, true );
}

void PreferredOperatorsDialog::addNetwork()
{
    if ( !m_addNetworkDlg )
        initAddNetworkDlg();
    if ( m_addNetworkList->count() )
        m_addNetworkList->setCurrentRow( 0 );

    m_addNetworkDlg->showMaximized();
    QtopiaApplication::execDialog( m_addNetworkDlg );
}

bool PreferredOperatorsDialog::isPreferred( unsigned int id )
{
    for ( int i = 0; i < m_currentOpers.count(); i++ ) {
        if ( m_currentOpers.at( i ).id == id )
            return true;
    }
    return false;
}

// necessary to make the Select Position dialog sane.
bool PreferredOperatorsDialog::eventFilter(QObject *obj, QEvent *event)
{
    QSpinBox *spinBox = (QSpinBox*)obj;
    if (event->type() == QEvent::KeyPress) {
        bool ok = false;
        int dbgVal = spinBox->cleanText().toInt( &ok );
        if ( ((QKeyEvent*)event)->key() == Qt::Key_Select ) {
            if ( ok && dbgVal > 0 ) {
                savedPrefNetOpLoc = spinBox->value();
                QSoftMenuBar::setLabel( spinBox->parentWidget(), Qt::Key_Back, QSoftMenuBar::Ok );
            } else {
                savedPrefNetOpLoc = -1;
                QSoftMenuBar::setLabel( spinBox->parentWidget(), Qt::Key_Back, QSoftMenuBar::Cancel );
            }
        } else if ( ((QKeyEvent*)event)->key() == Qt::Key_Back ) {
            if ( ok && dbgVal > 0 ) {
                savedPrefNetOpLoc = spinBox->value();
            } else {
                savedPrefNetOpLoc = -1;
            }
        }
    }
    return false;
}

void PreferredOperatorsDialog::networkSelected( QListWidgetItem *item )
{
    // get preferred position
    QDialog dlg( this );
    dlg.setWindowTitle( tr( "Select position" ) );
    QVBoxLayout layout( &dlg );
    layout.setSpacing( 2 );
    layout.setMargin( 2 );
    QString networkName = item->data( Qt::UserRole ).toString();
    networkName.replace("\n", "");
    QLabel label( &dlg );
    label.setTextFormat(Qt::RichText);
    label.setText( tr( "Add %1 to preferred network list.<br><br>Please select a position between 1 and %2.",
                "%1 = name of network, %2 = number of networks" )
            .arg( networkName ).arg( m_currentOpers.count() + 1 ) );
    label.setWordWrap( true );
    QSpinBox spinBox( &dlg );
    spinBox.setMinimum( 1 );
    spinBox.setMaximum( m_currentOpers.count() + 1 );
    spinBox.setValue( m_currentOpers.count() + 1 );
    layout.addWidget( &label );
    layout.addWidget( &spinBox );
    
    // make window bigger so it won't be auto-resized and re-positioned (looks
    // bad) when the predictive keyboard appears for the spinbox
    layout.addStretch();

    QPreferredNetworkOperators::Info oper;

    if ( m_currentOpers.count() == 0 ) {
        oper.index = 1;
    } else {
        savedPrefNetOpLoc = -1;
        spinBox.installEventFilter( this );
        if ( QtopiaApplication::execDialog( &dlg ) && savedPrefNetOpLoc > 0 ) {
            oper.index = savedPrefNetOpLoc;
            m_addNetworkDlg->accept();
        } else
            return;
    }

    oper.format = 2;
    oper.id = item->type();
    oper.name = item->data( Qt::UserRole ).toString() + "\n" + item->data( Qt::UserRole + 1 ).toString();

    // add new item to current list
    m_currentOpers.insert( oper.index - 1, oper );

    // add 1 to index of items below this new one
    for ( int i = oper.index; i < m_currentOpers.count(); i++ )
        updateIndex( i, true );

    TwoLineFloatingTextListItem *newItem = new TwoLineFloatingTextListItem( oper.name );
    newItem->setData( Qt::UserRole, item->data( Qt::UserRole ).toString() );
    newItem->setData( Qt::UserRole + 1, item->data( Qt::UserRole + 1 ).toString() );
    m_list->insertItem( oper.index - 1, newItem );
    m_list->setCurrentRow( oper.index - 1 );
}

void PreferredOperatorsDialog::removeNetwork()
{
    int index = m_list->currentRow();

    // update network to delete selected operator
    QPreferredNetworkOperators::Info delItem;
    delItem.index = m_currentOpers.at( index ).index;
    delItem.id = 0;
    delItem.name = QString();
    delItem.format = 0;

    m_currentOpers.removeAt( index );

    // substract 1 from index of items below this new one
    for ( int i = index; i < m_currentOpers.count(); i++ )
        updateIndex( i, false );

    delete m_list->takeItem( m_list->currentRow() );
    // ensure 'Remove' menu to disappear when item count is down to one.
    rowChanged( m_list->currentRow() );
}

void PreferredOperatorsDialog::updateIndex( int pos, bool increase )
{
    int i = increase ? 1 : -1;
    QPreferredNetworkOperators::Info newItem;
    newItem.index = m_currentOpers.at( pos ).index + i;
    newItem.format = m_currentOpers.at( pos ).format;
    newItem.id = m_currentOpers.at( pos ).id;
    newItem.name = m_currentOpers.at( pos ).name;
    newItem.technologies = m_currentOpers.at( pos ).technologies;

    m_currentOpers.removeAt( pos );
    m_currentOpers.insert( pos, newItem );
}

void PreferredOperatorsDialog::moveUp()
{
    int i = m_list->currentRow();
    if ( i == 0 )
        return;

    swap( i - 1, i );

    TwoLineFloatingTextListItem *item = (TwoLineFloatingTextListItem *)m_list->takeItem( i );
    m_list->insertItem( i - 1, item );
    m_list->setCurrentRow( i - 1 );
}

void PreferredOperatorsDialog::moveDown()
{
    int i = m_list->currentRow();
    if ( i == m_list->count() - 1 )
        return;

    swap( i, i + 1 );

    TwoLineFloatingTextListItem *item = (TwoLineFloatingTextListItem *)m_list->takeItem( i );
    m_list->insertItem( i + 1, item );
    m_list->setCurrentRow( i + 1 );
}

void PreferredOperatorsDialog::swap( int first, int second )
{
    // swap information except index
    QPreferredNetworkOperators::Info oper1 = m_currentOpers.takeAt( first );
    QPreferredNetworkOperators::Info oper2 = m_currentOpers.takeAt( first );
    QPreferredNetworkOperators::Info operNew1;
    QPreferredNetworkOperators::Info operNew2;

    operNew1.index = oper2.index;
    operNew1.format = oper1.format;
    operNew1.id = oper1.id;
    operNew1.name = oper1.name;
    operNew1.technologies = oper1.technologies;

    operNew2.index = oper1.index;
    operNew2.format = oper2.format;
    operNew2.id = oper2.id;
    operNew2.name = oper2.name;
    operNew2.technologies = oper2.technologies;

    m_currentOpers.insert( first, operNew2 );
    m_currentOpers.insert( second, operNew1 );
}

void PreferredOperatorsDialog::rowChanged( int i )
{
    m_remove->setVisible( m_list->count() > 1 );
    m_remove->setEnabled( m_list->count() > 1 );
    m_up->setVisible( i != 0 );
    m_up->setEnabled( i != 0 );
    m_down->setVisible( i != m_list->count() - 1 );
    m_down->setEnabled( i != m_list->count() - 1 );
}

void PreferredOperatorsDialog::checkIndex()
{
    // index should be in order
    for ( int i = 1; i < m_currentOpers.count(); i++ ) {
        if ( m_currentOpers.at( i ).index != m_currentOpers.at( i - 1 ).index + 1 ) {
            QPreferredNetworkOperators::Info item;
            item.index = m_currentOpers.at( i - 1 ).index + 1;
            item.format = m_currentOpers.at( i ).format;
            item.id = m_currentOpers.at( i ).id;
            item.name = m_currentOpers.at( i ).name;
            item.technologies = m_currentOpers.at( i ).technologies;

            m_currentOpers.removeAt( i );
            m_currentOpers.insert( i , item );
        }
    }
}

void PreferredOperatorsDialog::accept()
{
    QList<QPreferredNetworkOperators::Info> list;
    list = m_PNOClient->resolveNames( m_currentOpers, m_operatorNames );

    // ensure the index is in order
    checkIndex();

    // update networks changed its position.
    for ( int i = 0; i < m_currentOpers.count(); i++ ) {
        if ( i >= m_originalOpers.count() ||
                m_originalOpers.at( i ).id != m_currentOpers.at( i ).id
                || ( m_originalOpers.at( i ).id == m_currentOpers.at( i ).id
                    && m_originalOpers.at( i ).index != m_currentOpers.at( i ).index ) ) {
            m_PNOClient->writePreferredOperator(
                    QPreferredNetworkOperators::Current,
                    m_currentOpers.at( i ) );
        }
    }
    for ( int j = m_currentOpers.count(); j < m_originalOpers.count(); j++ ) {
        // remove the remaining items
        QPreferredNetworkOperators::Info delItem;
        delItem.index = m_originalOpers.at( j ).index;
        delItem.id = 0;
        delItem.name = QString();
        delItem.format = 0;

        m_PNOClient->writePreferredOperator(
                QPreferredNetworkOperators::Current,
                delItem );
    }

    QDialog::accept();
}

#include "modemnetwork.moc"
