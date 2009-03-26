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
#include "pkim.h"
#include "charlist.h"
#include "charmatch.h"

#include <qdawg.h>
#include <qtopiaapplication.h>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <inputmethodinterface.h>

#include <stroke.h>

#include <symbolpicker.h>

#include <QSettings>
#include <QLabel>
#include <QTimer>
#include <QPixmapCache>
#include <QLayout>
#include <QPolygon>
#include <QPainter>
#include <QPaintEngine>
#include <QDesktopWidget>
#include <QStyle>
#include <QInputMethodEvent>
#include <QKeyEvent>

#include <QDebug>

// This define is for internal testing purposes only.
//#define PKIM_DEBUG_RECORD_MOUSE_EVENTS

#ifdef PKIM_DEBUG_RECORD_MOUSE_EVENTS
#include <QFile>
#endif

// These two defines just increase the amount of log & debug output, useful
// for debugging purposes, but annoying otherwise
//#define PKIM_VERBOSE_OUTPUT
//#define DEBUG_PKIM

// FSHANDWRITING_SIMPLE_STROKES turns off "advanced" drawing
// Note that drawing with the composition modes is faster and prettier,
// so this should only be enabled if you know that the target will not
// support composition modes, or for testing purposes
//#define FSHANDWRITING_SIMPLE_STROKES

static const int ABC_AUTOEND_TIME=700;
static const int PRESS_AND_HOLD_TIME=300;
static const int GUESS_TIME_OUT=2000;

class QWSScreenStroke;
static QWSScreenStroke *qt_screenstroke = 0;

// can also do by not accepting points more than X away, and left-click if stroke length is 1
bool could_be_left_click;

class QWSScreenStroke : public QWidget
{
    public:
        QWSScreenStroke();
        ~QWSScreenStroke();

        /* start or add to current stroke */
        void append(const QPoint);
        void end(); // end and clear all strokes
        void fadeStroke(); // fade the current stroke
        void clearAppearance(); // Clear the widget surface

        void paintEvent(QPaintEvent *);
        void showEvent(QShowEvent* e);

        void keyEvent(QKeyEvent *){
            qWarning("Screen drawer incorrectly being given key events");
        }
    private:
        void initializePainting(QPaintEvent *e);
        void paintWithCompositionModes(QPaintEvent *e);
        void paintWithoutCompositionModes(QPaintEvent *e);

        QPoint lastPoint;
        bool currentStroke;
        QPolygon currentStrokePoints;
        bool fullScreenClearFlag;
        QPolygon fadedStrokePoints;
        QRect oldFadedStrokeRect;
        bool fadeStrokeFlag;
        int strokeWidth;
// updateAdjustment is used to make sure that updates() used to increntally
// draw screenstrokes capture the whole stroke.  Should be equal to the radius
// of the biggest pen used in stroke drawing
// For complex drawing this is half the strokeWidth+shadowWidth, rounded up
        int updateAdjustment;
        int shadowWidth;
        int minimumStrokeLengthOptimisation;
        int lastDrawnOptimisation;
        int compositionModesSupported;
};

void fs_end_stroke()
{
    if(qt_screenstroke) {
        qt_screenstroke->end();
    }
}

void fs_fade_stroke()
{
    if (qt_screenstroke) {
        qt_screenstroke->fadeStroke();
    }
}

void fs_append_stroke(const QPoint &pos)
{
    if(qApp) {
        if(qt_screenstroke == 0)
            qt_screenstroke = new QWSScreenStroke();
        qt_screenstroke->append(pos);
    }
}

QWSScreenStroke::QWSScreenStroke() :
    QWidget(0, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)
    , lastPoint(0,0)
    , currentStroke(false)
    , currentStrokePoints()
    , fullScreenClearFlag(false)
    , fadedStrokePoints()
    , oldFadedStrokeRect()
    , fadeStrokeFlag(false)
    , compositionModesSupported(-1)
{
    QRect desktopAvailableRect(QApplication::desktop()->availableGeometry());
    // Generic initialization here
    setPalette(QPalette(QColor(0,0,0,0)));
    setAttribute(Qt::WA_NoSystemBackground);
    setWindowOpacity(0.0);
//    setWindowOpacity(4.0/7.0);
    setFixedSize(desktopAvailableRect.size());
    // This covers the screen, but allows space for the context key bar at the
    // the header
    qLog(Input) << "QWSScreenStroke being instantiated with geometry" << desktopAvailableRect;

    // Note that in qt 4.1 this code leads to a blue widget on 16 bit displays
    // The alternative for those displays is to set opacity to 1.0, and use
    // semi-transparent brushes (see qtopia 4.1 code).  However, This approach
    // requires that the QPaintEngine supports composition modes to avoid
    // very ugly strokes, and composition modes are frequently not supported.
    // If neither transparent widgets nor composition modes are supported
    // properly (non-standard displays using qt < 4.2), the line must be
    // simplified - using a thin black line without shadow is recommended
    // (Although anti-aliasing is supported even without compositon modes)

    setGeometry(desktopAvailableRect);
    setFocusPolicy(Qt::NoFocus);
}

QWSScreenStroke::~QWSScreenStroke()
{
}

void QWSScreenStroke::initializePainting(QPaintEvent *e)
{
    Q_UNUSED(e);
    // check for Porter-Duff support
    // must be called from within paintEvent, as QWSScreenStroke is a widget
    Q_UNUSED(e);
#ifndef FSHANDWRITING_SIMPLE_STROKES
    QPainter p(this);

    QPaintEngine* paintEngine = p.paintEngine();
    if (paintEngine)
    {
        if (paintEngine->hasFeature(QPaintEngine::PorterDuff)) {
            qLog(Input) << "Composition modes supported, using advanced stroke painting";
            compositionModesSupported = 1;
        }
        else
        {
            qLog(Input) << "Composition modes not supported, using simple stroke painting";
            compositionModesSupported = 0;
        }
    }
#else
    compositionModesSupported = 0;
#endif

    if (compositionModesSupported == -1) { // still don't know
        qLog(Input) << "QWSScreenStroke failed to find paintEngine. Painting capabilities still unknown";
        // May need some generic initialization here if flickering occurs
        return;
    };
    // Generic initialization here
    setWindowOpacity(4.0/7.0);

    if (compositionModesSupported == 1)
    {
        // Advanced drawing initialization
        strokeWidth = 5; // TODO: handle default Stroke width more intelligently
        shadowWidth = 2; // possibly add get/set functions?
        // Note that the overlapping drawing relies on
        // minimumStrokeLengthOptimisation being at least as big as the
        // shadowWidth to work properly.
        minimumStrokeLengthOptimisation = 2;
        updateAdjustment = strokeWidth/2 + strokeWidth %2 + shadowWidth/2 + shadowWidth%2;
        qLog(Input) << "strokeWidth: " << strokeWidth << ", updateAdjustment: " << updateAdjustment;

        setWindowOpacity(4.0/7.0);
    }

    if (compositionModesSupported == 0)
    {
        // Simple drawing initialization
        setWindowOpacity(4.0/7.0);
        minimumStrokeLengthOptimisation = 1;
        updateAdjustment = 2;
    }
}

void QWSScreenStroke::fadeStroke()
{
    if(!compositionModesSupported) {
        end();
        return;
    }
    currentStroke = false;
    oldFadedStrokeRect = fadedStrokePoints.boundingRect();
    fadedStrokePoints = currentStrokePoints;
    currentStrokePoints.clear();
    fadeStrokeFlag=true;

    QRect newFadedStrokeRect = fadedStrokePoints.boundingRect();
    newFadedStrokeRect.adjust(-updateAdjustment,-updateAdjustment,
                               updateAdjustment,updateAdjustment);

    if(oldFadedStrokeRect != QRect()){
        oldFadedStrokeRect.adjust(-updateAdjustment,-updateAdjustment,updateAdjustment,updateAdjustment);
        update(oldFadedStrokeRect); // clear old faded stroke
    };
    update(newFadedStrokeRect); // Paint new faded stroke
}

void QWSScreenStroke::clearAppearance()
{
    fullScreenClearFlag=true;
    update();
}


void QWSScreenStroke::append(const QPoint pos)
{
    if (currentStroke)
    {
        if((lastPoint-pos).manhattanLength() >= minimumStrokeLengthOptimisation)
        {
            currentStrokePoints.append(pos);
            update(QRect(lastPoint, pos).normalized().adjusted(-updateAdjustment, -updateAdjustment, updateAdjustment, updateAdjustment));
            lastPoint = pos;
        };
    }
    else
    {
        // this assumes that the IM is not hidden mid-stroke
        //clearAppearance();
        currentStrokePoints.append(pos);
        currentStroke = true;
        lastPoint = pos;
        lastDrawnOptimisation=1; // must start at 1, not 0 - array out of bounds if ever zero, and no point in optimizing the first point anyway
        show();
        update(QRect(pos, pos).adjusted(-updateAdjustment, -updateAdjustment, updateAdjustment, updateAdjustment));
    }
}

void QWSScreenStroke::end()
{
    currentStroke = false;
    currentStrokePoints.clear();
    fadedStrokePoints.clear();
    hide();
}

void QWSScreenStroke::showEvent(QShowEvent *e)
{
    fullScreenClearFlag = true;
    QWidget::showEvent(e);
}

#define QWSSCREENSTROKE_DRAW_OVERLAPPED

void QWSScreenStroke::paintEvent(QPaintEvent *e)
{
    if(compositionModesSupported == -1) {
        initializePainting(e);
    }

    if(compositionModesSupported == 1)
    {
        paintWithCompositionModes(e);
        return;
    }

    if(compositionModesSupported == -1)
        qLog(Input) << "QWSScreenStroke is drawing without knowing about composition mode support.  Defaulting to simple drawing";

    paintWithoutCompositionModes(e);
}

inline void QWSScreenStroke::paintWithCompositionModes(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter p(this);

    if(fullScreenClearFlag)
    {
        qLog(Input) << "clearing QWSScreenStroke - geometry() is "<< geometry() << ", e->rect() is "<< e->rect() << ", size() is "<<size();
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.eraseRect(rect());

        if(e->rect().contains(rect())){
            fullScreenClearFlag=false;
        }
    }

    if(fadeStrokeFlag == true)
    {
        fadeStrokeFlag = false;
        if(oldFadedStrokeRect != QRect()) {
            // transparent background, so use QPainter::CompositionMode_Clear
            p.setCompositionMode(QPainter::CompositionMode_Clear);
            p.eraseRect(oldFadedStrokeRect);

            if(e->rect().contains(oldFadedStrokeRect))
                oldFadedStrokeRect = QRect();
        }

        if(fadedStrokePoints.size() > 0) {
            p.setCompositionMode(QPainter::CompositionMode_Source); //_SourceOver is fine too
            QColor fadedStrokeColor = qApp->palette().mid().color();
            QPen fadedPen = QPen(fadedStrokeColor, strokeWidth+shadowWidth);
            // this has to match the styles of the stroke below
            fadedPen.setJoinStyle(Qt::RoundJoin);
            fadedPen.setCapStyle(Qt::RoundCap);
            p.setPen(fadedPen);
            p.drawPolyline(fadedStrokePoints);
        }
    }

    if (currentStrokePoints.size() == 0)
        return;

    p.setCompositionMode(QPainter::CompositionMode_SourceOver);

    QColor edge = QApplication::palette().highlightedText().color();
    QColor stroke = QApplication::palette().highlight().color();
    stroke.setAlphaF(1.0);
    edge.setAlphaF(1.0);

    QBrush edgeBrush = QBrush(edge);
    QBrush strokeBrush = QBrush(stroke);

    QPen edgePen = QPen(edge, strokeWidth+shadowWidth);
    edgePen.setJoinStyle(Qt::RoundJoin);
    edgePen.setCapStyle(Qt::RoundCap);
    QPen strokePen = QPen(strokeBrush, strokeWidth);
    strokePen.setJoinStyle(Qt::RoundJoin);
    strokePen.setCapStyle(Qt::RoundCap);

    /* funky stroke drawing here */
    if (currentStrokePoints.size() == 1)
    {
        //draw a point
        p.setPen(edgePen);
        p.drawPoint(lastPoint);
        p.setPen(strokePen);
        p.drawPoint(lastPoint);
    } else // currentStrokePoints.size() > 1
    {
        for(;lastDrawnOptimisation < currentStrokePoints.size();lastDrawnOptimisation++)
        {
            //            if(shadowWidth>0)
            //            {
            p.setPen(edgePen);
            p.drawLine(currentStrokePoints[lastDrawnOptimisation-1], currentStrokePoints[lastDrawnOptimisation]);
            //            };
            p.setPen(strokePen);
            p.drawLine(currentStrokePoints[lastDrawnOptimisation-1], currentStrokePoints[lastDrawnOptimisation]);
            // draw the stroke before last to overwrite the shadow we've just drawn there.
            // Note that this must be drawn either solidly (not transparent),
            // or using QPainter::CompositionMode_Source. Otherwise, the
            // overlapping causes irregular and ugly colouration
            if(lastDrawnOptimisation>=2)
                p.drawLine(currentStrokePoints[lastDrawnOptimisation-2], currentStrokePoints[lastDrawnOptimisation-1]);
        };
    }
}

void QWSScreenStroke::paintWithoutCompositionModes(QPaintEvent *e)
{
    Q_UNUSED(e);

    // Simple drawing here
    QPainter p(this);

    if(fullScreenClearFlag)
    {
        qLog(Input) << "clearing QWSScreenStroke - geometry() is "<< geometry() << ", e->rect() is "<< e->rect() << ", size() is "<<size();
        QColor col = palette().color(QPalette::Window);
        col.setAlpha(255);
        p.fillRect(rect(), col);

        if(e->rect().contains(rect()))
            fullScreenClearFlag = false;
    }

    if(fadeStrokeFlag)
    {
        fadeStrokeFlag = false;
        if(fadedStrokePoints.size() > 0) {
            QColor fadedStrokeColor = palette().mid().color();
            QPen fadedPen = QPen(fadedStrokeColor, 2);
            // this has to match the styles of the stroke below
            fadedPen.setJoinStyle(Qt::RoundJoin);
            fadedPen.setCapStyle(Qt::RoundCap);
            p.setPen(fadedPen);
            if(fadedStrokePoints.size() == 1)
                p.drawPoint(fadedStrokePoints.first());
            else
                p.drawPolyline(fadedStrokePoints);
        }
    }

    if(currentStrokePoints.size() > 0)
    {
        QPen strokePen = QPen(palette().color(QPalette::WindowText), 2);
        strokePen.setJoinStyle(Qt::RoundJoin);
        strokePen.setCapStyle(Qt::RoundCap);

        p.setPen(strokePen);
        if(currentStrokePoints.size() == 1)
            p.drawPoint(currentStrokePoints.first());
        else
            p.drawPolyline(currentStrokePoints);
    }
}

/*! \class PkIM
    \inpublicgroup QtInputMethodsModule
    \brief The PkIM class provides an input method
    based on a standard telephone keypad.  Also provides full screen handwriting input.
*/

/*!
  \fn bool PkIM::isActive() const

  Returns true if key or mouse events are currently being filtered by the input method to generate text.  Otherwise returns false.
 */

/*!
  \fn bool PkIM::isFiltering() const

  Returns true if key or mouse events are currently being filtered by the input method to generate text.  Otherwise returns false.
 */

/*!
  \fn bool PkIM::restrictToHint() const

  Returns true if the input method is restricted to s single mode based of the current input hint.  Otherwise returns false.
 */

/*!
  \fn void PkIM::stateChanged(int state)

  This signal is emitted when the state of the input method changes.  It is used by the server to update icon state and visability appropriately.

  If the input method is filtering \a state will be QtopiaInputMethod::Ready.  Otherwise \a state will be QtopiaInputMethod::Sleeping.
 */

/*!
  \fn void PkIM::setRestrictToHint(bool b)

  If \a b is true will restrict the mode of the input method to the current mode.
 */

/*!
  \fn void PkIM::setStatusWidget(QLabel *l)
  Sets the widget used to display the current status to \a l.  The input method will update the icon for the widget to reflect the input method mode
 */

/*!
  \internal

  Construct a PkIM Input Method.
 */
PkIM::PkIM()
    : protectInputStroke(false), ignoreNextMouse(false), multiStrokeTimeout(0)
    , cMode(Off), lastPoint(), canvasHeight(0), choicePos(-1)
    , choices(), shift(false), autoCapitalize(true), capitalizeNext(true)
    , hintedAutoCapitalization(false), autoCapitalizingPunctuation()
    , autoCapitalizeEveryWord(false)
    , allowHandwriting(true)
    , active(false), lastUnicode(0), status(0), tip(0)
    , tid_abcautoend(0), symbolPicker(0)
    , wordPicker(0), charList(0), microX(0), microY(0), strokeThreshold(0)
    , actionsSinceChangeMode(0), mPasswordHint(false)
    , key_hold_status(null)
{
    qLog(Input) << "PkIM::PkIM";
    penMatcher = new QFSPenMatch( this );
    inputStroke = 0;
    mRestrictToHint = false;
    mPasswordHint = false;
    matcherSet = new InputMatcherSet(this);
    QIMPenStroke::setUseCanvasPosition(false);

    midStroke = false;
    aboutToChangeGuess = false;

    lStrokeTimer = new QTimer(this);
    lStrokeTimer->setSingleShot(true);
    connect(lStrokeTimer, SIGNAL(timeout()), this, SLOT(removeStroke()));

    lClickTimer = new QTimer(this);
    lClickTimer->setSingleShot(true);
    connect(lClickTimer, SIGNAL(timeout()), this, SLOT(ignoreRestOfStroke()));

    lGuessTimer = new QTimer(this);
    lGuessTimer->setSingleShot(true);
    //connect(lGuessTimer, SIGNAL(timeout()), this, SLOT(guessTimedOut()));

    QIMPenStroke::setUseCanvasPosition(false);
    // in non-commercial, words matches to same as text
    wordMatcher = new InputMatcher("words"); // does this need a delete later?

    QtopiaChannel *ch = new QtopiaChannel( "QPE/Handwriting", this );
    connect( ch, SIGNAL(received(QString,QByteArray)),
            this, SLOT(pluginMessage(QString,QByteArray)) );
    loadProfiles();
}

/*!
  \internal
  Destroys the input method.
 */
PkIM::~PkIM()
{
    if (symbolPicker)
        delete symbolPicker;
    if (wordPicker)
        delete wordPicker;
    if (modePicker)
        delete modePicker;
    delete charList;
    delete tip;

    // save current handwriting height.
    // should do this more on just close...
    QSettings config("Trolltech","handwriting");
    config.beginGroup( "Settings" );
    config.setValue( "CanvasHeight", canvasHeight);
    config.setValue( "StrokeThreshold", strokeThreshold);

    if(!hintedAutoCapitalization){
        config.setValue( "AutoCapitalize", autoCapitalize);
        config.setValue( "AutoCapitalizingPunctuation", autoCapitalizingPunctuation);
    }

    if (qt_screenstroke) {
        delete qt_screenstroke;
        qt_screenstroke = 0;
    }

    if(inputStroke) {
        delete inputStroke;
        inputStroke=0;
    }
    lClickTimer->stop();
    lStrokeTimer->stop();
    lGuessTimer->stop();
}

/*!
  \reimp

  \internal
  Updates the location on the screen of the cursor
  or sub-focus of the current input widget.
 */
void PkIM::queryResponse ( int property, const QVariant & result )
{
    if(property==Qt::ImMicroFocus) {
        QRect resultRect = result.toRect();
        if(resultRect.isValid()) setMicroFocus(resultRect.x(), resultRect.y());
    }
}

/*!
  Updates the internal representation of the location on the screen of the cursor.  \a x and \a y are relative to the top level window.
*/
void PkIM::setMicroFocus( int x, int y )
{
    if(x == microX && y == microY) return;

#ifdef PKIM_VERBOSE_OUTPUT
    qLog(Input) << "PkIM::setMicroFocus - changed to ("<<x<<", "<<y<<")";
#endif // PKIM_VERBOSE_OUTPUT
    microX = x;
    microY = y;
    if (active) {
        if (symbolPicker && symbolPicker->isVisible())
            symbolPicker->setMicroFocus(x, y);
        if (wordPicker && wordPicker->isVisible())
            wordPicker->setMicroFocus(x, y);
        if (modePicker && modePicker->isVisible())
            modePicker->setMicroFocus(x, y);
    }
}

/*!
  \internal

  Handles mouse clicks not handled for full screen mouse input.

  Ends the currently composed word.
 */
void PkIM::mouseHandler( int i, int isPress)
{
    // parameters are/were index and isPress.
    //make click on word accept word? is most likely me    penMatcher = new QFSPenMatch( this );
    qLog(Input) << "PkIM::mouseHandler( int " << i << ", int " <<isPress <<")";
    if (!isPress) {
        sendAndEnd();
        endWord();
    }
}

/*!
  Sets the mode of the input method to the hint \a s.  If the hint
  starts with only will take the remaining string as the hint and restrict the mode to the one
  that best matches that hint.

  \table
    \header \o Hint \o Description
    \row \o words \o One keypress per letter.  Uses dictionary lookup. Pressing the '*' key cycles through dictionary matches.
    \row \o propernouns \o Interprets keys and dictionary like Words.  In addition, the first letter of every word is capitalized.  Note that the propernouns hint will also be recognized as an auxiliary hint, see below for details.
    \row \o email
    \row \o text \o Repeatedly press a key to select the desired letter.  Words over 3 letters are added to the dictionary.
    \row \o phone \o Enter numbers directly.  '*' key cycles through '*','+','P' and 'W'
    \row \o int \o Same as phone.
  \endtable

    Possible hints include "" "words" "phone" "int" "email" "propernouns" and "text"
    additionally, adding "password", with or without a preceding space
    will enable password mode.  For PkIM, this just means
    that words will not be saved to the dictionary.

    PkIM also recognizes a small set of auxiliary (or modifier) hints.  These are appended to the hint, separated by spaces. PkIM does not recognize order in auxiliary hints, so "text autocapitalization propernouns" is equivalent to "text propernouns autocapitalization".
    \table
        \header \o name \o Description
        \row \o autocapitalization \o The first word of the field, and the first word of a sentence have their first letter capitalized.  The capitalization is implemented using the
        \row \o noautocapitalization \o All words not capitalized by the user are in lower-case
        \row \o propernouns \o As an auxiliary hint, propernouns further modifies autocapitalization, such that the first letter of every word is capitalized instead of just the first in a sentence.  Hinting propernouns implies autocapitalization, so "text autocapitalization propernouns" and "text propernouns" are equivalent.   Combining the noautocapitalization hint with the propernouns hint may lead to unexpected results.  \bold{Note:} propernouns can also be used as the primary hint on it's own, where it is equivalent to "words autocapitalization propernouns", and also equivalent to the QtopiaApplication::ProperNouns hint.
    \endtable

    \sa processAuxiliaryHints(), QtopiaApplication::ProperNouns
 */
void PkIM::setHint(const QString& s)
    // Currently the input method assumes only one mode anyway.
    // consequently, "only" is currently only used by the server during startup

{
    QStringList args = s.split(" ");
    QString h=args.first();

    if (!profile) {
        qLog(Input) << "PkIM::setHint("<<h<<") - no profile";
        return;
    };

    bool wasOff = cMode == Off;
    bool setCharSetCalled = false;
    bool onlyHintFlag=false; // ignored currently

    // Turn off IM if no hint provided - "" hint
    if (h.isNull() || h.isEmpty())
    {
        if (cMode != Off) {
            cMode = Off;
            active = false;
            // XXX updateStatusIcon();
            emit stateChanged(QtopiaInputMethod::Sleeping);
            qLog(Input) << "PkIM::setHint("<<h<<") Sleeping";
        }
        else
            qLog(Input) << "PkIM::setHint("<<h<<") - already off, no action taken";
        return;
    };

    // update microfocus
    qwsServer->sendIMQuery ( Qt::ImMicroFocus );

    if (args.contains("only"))
    {
        onlyHintFlag=true; // This is currently mostly ignored, as only one charSet can be activated at once anyway.  Multiple input types beyond the first will be ignored.
        qLog(Input) << "PkIM detected ""only"" hint";
    };

    //check for password hint
    if(args.contains("password"))
    {
        qLog(Input) << "PkIM detected ""password"" hint";
        mPasswordHint=true;
    } else
    {
        mPasswordHint=false;
    };

    if ((h == "phone" || h == "int") && profile->charSet("Number")) { // no tr
        if (cMode != Int) {
            qLog(Input) << "PkIM::setHint(" << h << ") - changing to numbers mode";
            penMatcher->setCharSet( profile->charSet("Number") ); // no tr
            matcherSet->setHintedMode("int");
            setCharSetCalled = true;
            cMode = Int;
            active = true;
            setModePixmap();
            // XXX updateStatusIcon();
        }
        else{
            qLog(Input) << "PkIM::setHint(" << h << ") - already in numbers mode, taking no action";

        }
    }
    else if (h == "pkim" ||  h == "words" || h == "propernouns")
    {
        matcherSet->clearNamedMode();
        active = (matcherSet->setHintedMode(h) != 0);

        if (active)
        {
            processAuxiliaryHints(args);
            setModePixmap();
            emit stateChanged(QtopiaInputMethod::Ready);

            if (cMode != Dict) {
                qLog(Input) << "PkIM::setHint(" << h << ") - changing to words mode";
                penMatcher->setCharSet( profile->charSet("Text") ); // no tr
                setCharSetCalled = true;
                cMode = Dict;
                active = true;
                // XXX updateStatusIcon();
            } else
                qLog(Input) << "PkIM::setHint(" << h << ") - already in words mode";
        }
        else qLog(Input) << "PkIM::setHint(" << h << ") - , but IM not active or matcherSet does not support this mode";
    }
    else
    {
        // text, or unrecognized named input hints.
        active = (matcherSet->setHintedMode(h) != 0);
        penMatcher->setCharSet( profile->charSet("Text") ); // no tr
        setCharSetCalled = true;
        if (active){
            processAuxiliaryHints(args);

            setModePixmap();
            qLog(Input) << "PkIM::setHint("<<h<<") - (text)";
        } else
            qLog(Input) << "PkIM::setHint("<<h<<") - (text), but matcherSet does not support that mode.";
        cMode = Lower;
    };

    if (wasOff && cMode != Off){
        qLog(Input) << "PkIM now Ready";
        emit stateChanged(QtopiaInputMethod::Ready);
    } else if (!wasOff && cMode == Off) {
        qLog(Input) << "PkIM now Sleeping";
        emit stateChanged(QtopiaInputMethod::Sleeping);
    };

    return;
};

/*!
    This helper function takes the \a inputHint,
    processes any additional hint information and changes the
    internal state of the input method accordingly.
    \bold{Note:} This function requires the primary input hint as well
    as the auixilliary hints in order to resolve ambiguity introduced
    by the "propernouns" hint.
    \sa setHint()
*/
void PkIM::processAuxiliaryHints(QStringList inputHint)
{
    QString primaryHint = inputHint.first();
    QStringList auxiliaryHints = inputHint;
    auxiliaryHints.removeFirst();
    bool isNames = primaryHint == "propernouns";

    qLog(Input) << "PkIM::processAuxiliaryHints( "<<auxiliaryHints<< ")";
    if (isNames || auxiliaryHints.contains("autocapitalization")) {
        qLog(Input) << "PkIM detected ""autocapitalization"" hint";
        hintedAutoCapitalization = true;
        autoCapitalize = true;
        // TODO: Check whether the cursor is at the start of a word,
        // and set capitalizeNext appropriately
        capitalizeNext = true;
    } else if(mPasswordHint || auxiliaryHints.contains("noautocapitalization")) {
        qLog(Input) << "PkIM detected ""noautocapitalization"" hint";
        hintedAutoCapitalization = true;
        autoCapitalize = false;
        capitalizeNext = false;
    } else {
        hintedAutoCapitalization = false;
        QSettings config("Trolltech","handwriting");
        config.beginGroup( "Settings" );
        autoCapitalize = config.value( "AutoCapitalize", true).toBool();
        capitalizeNext = autoCapitalize;
    };

    if (auxiliaryHints.contains("nohandwriting")) {
        allowHandwriting = false;
    } else
        allowHandwriting = true;

    // TODO: capitalizeNext should be set depending on the input field
    // capitalizeNext should be false when text precedes the cursor
    shift = autoCapitalize && capitalizeNext;
    autoCapitalizeEveryWord = isNames || auxiliaryHints.contains("propernouns");
    if(autoCapitalizeEveryWord){
        qLog(Input) << "PkIM detected ""propernouns"" hint - capitalizing ever word";
    };
}

void PkIM::setMode(const QString &h, bool hasShift)
{
    qLog(Input) << "PkIM::setMode(const QString &"<<h<<", bool "<<hasShift<<")";

    if (h != matcherSet->currentMode()->id()){
        sendAndEnd(); // actual mode change, so commit current word
        endWord();
    }
    if (h != matcherSet->currentMode()->id() || hasShift != shift) {
        if (!h.isEmpty() && matcherSet->setCurrentMode(h)) {
            active = true;
            shift = hasShift;
            capitalizeNext = hasShift && capitalizeNext; // TODO: allow user override, and check for context
            setModePixmap();
            emit stateChanged(QtopiaInputMethod::Ready);
        } else {
            active = false;
            emit stateChanged(QtopiaInputMethod::Sleeping);
        }
    }
}

void PkIM::symbolSelected(int unicode, int keycode)
{
    Q_UNUSED(keycode);
    text = QChar(unicode);
    sendAndEnd();
    endWord();
}

void PkIM::wordSelected(const QString &s)
{
    qLog(Input) << "PkIM::wordSelected("<<s<<")";
    int pos;
    for (pos = 0; pos < choices.count(); pos++) {
        if (choices[pos] == s)
            break;
    }
    if (pos < choices.count()) {
        choice = pos;
        text = choices[choice];
        qLog(Input) << "PkIM::wordSelected() - pos = "<<pos<<" choice = "<<choice;
        sendPreeditString( text, text.length() );
        sendAndEnd();
        endWord(); //word probably in the dictionary, but need to clear it.
    }
    qLog(Input) << "PkIM::wordSelected() (after send- pos = "<<pos<<"choices.count() = "<<choices.count()<<", choice = "<<choice;
    wordMatcher->reset();
    choices.clear();
    choicePos = -1;
}

/*!
  \internal
  Resets the input method to its initial state.
 */
void PkIM::reset()
{
    qLog(Input) << "PkIM::reset() - word was " << word << ", text was "<<text;
    if (active) {
        sendAndEnd();
        lastUnicode = 0;
        active = true;
        word.clear();
        text.clear();
        if (symbolPicker)
            symbolPicker->hide();
        if (wordPicker)
            wordPicker->hide();
        if (modePicker)
            modePicker->hide();
        emit stateChanged(QtopiaInputMethod::Sleeping);

        /* reset code from qfsinput */
        /* could occur mid stroke */
        if (inputStroke) {
#ifndef QT_NO_QWS_CURSOR
            qwsServer->setCursorVisible(true);
#endif
            if (!protectInputStroke) {
                // only true in cases where it will be deleted anyway,
                // but might be used later.  Sending mouse events can
                // result in resetting of the im.
                delete inputStroke;
                inputStroke = 0;
            }

            if (lClickTimer) lClickTimer->stop();

            could_be_left_click = false;
            ignoreNextMouse = true; // ??
            midStroke = false;
            //            updateStatusIcon();
        }
    }

    fs_end_stroke();

    /* clear matcher */
    if (lStrokeTimer) lStrokeTimer->stop();

    penMatcher->clear();
    wordMatcher->reset();

}

void PkIM::addWordToDictionary(const QString& word)
{
    // first check for dictionary mode.  Don't save word in dictionary mode
    if(mPasswordHint){
        qLog(Input) << "PkIM::addWordToDictionary() - password hinted, so don't save words - rejected";
        return;
    }

    // If the user deletes a word, assume that they don't want it
    // to be automatically added back. They can always use the
    // tool that was used for deletion to add it manually.
    //
    if ( Qtopia::dawg("deleted").contains(word) ){
        qLog(Input) << "PkIM::addWordToDictionary(const QString& "<<word<<") - word already in ""deleted"" dictionary - rejected";
        return;
    }

    QString lword = word.toLower();

    if ( Qtopia::dawg("preferred").contains(word)
            || Qtopia::dawg("preferred").contains(lword) ){
        qLog(Input) << "PkIM::addWordToDictionary(const QString& "<<word<<") - word already in ""preferred"" dictionary - rejected";
        return;
    };
    if ( Qtopia::addedDawg().contains(word)
            || Qtopia::addedDawg().contains(lword) ){
        qLog(Input) << "PkIM::addWordToDictionary(const QString& "<<word<<") - word already in ""added"" dictionary - rejected";
        return;
    };
    if ( Qtopia::fixedDawg().contains(word)
            || Qtopia::fixedDawg().contains(lword) ) {
        qLog(Input) << "PkIM::addWordToDictionary(const QString& "<<word<<") - word already in ""fixed"" dictionary - rejected";
        return;
    };

    qLog(Input) << "PkIM::addWordToDictionary(const QString& "<<word<<") - adding word";
    Qtopia::addWords(QStringList(word));

    if ( !tip ) {
        tip = new QLabel(0,Qt::Tool | Qt::FramelessWindowHint);
        tip_hider = new QTimer(this);
        tip_hider->setSingleShot(true);
        tip->setBackgroundRole( QPalette::Base );
        connect(tip_hider,SIGNAL(timeout()),tip,SLOT(hide()));
    }
    tip->setText(tr("<small>Added</small>&nbsp;<b>%1</b>").arg(word)); // sep required, position required
    tip->resize(tip->sizeHint());
    tip->move(qMax(0,microX-tip->width()),microY+2);
    tip->show();
    tip_hider->start(1000);
}

/*!
  \fn void PkIM::endWord()

  Ends the current word, and, if appropriate, adds it to the dictionary.
  Words are added if they are are 3 letters or longer, and the input method is
  in a text entry mode (as opposed to words modes, where this word will
  almost always either come from the dictionary already, or be garbage)
 */

void PkIM::endWord()
{
    QString choice;
    if (choicePos >= 0)
        text = choices[choicePos];

    qLog(Input) << "PkIM::endWord() - choice (local string) = "<<choice<<", word is "<<word<<", and will be appended with text = "<<text;
    sendAndEnd();

    int wl = word.length();
    if (autoCapitalize && (
            (wl>0  && autoCapitalizingPunctuation.contains(word[wl-1]))
            || autoCapitalizeEveryWord
            || autoCapitalizingPunctuation.contains(' ')) )
    {
        qLog(Input) << "PkIM : Autocapitalizing."<<autoCapitalizingPunctuation<<")";
        capitalizeNext = true;
    }

    if ( wl ) {
        QChar l = word[wl-1];
        while ( !l.isLetter() && wl > 2 ) {
            word.truncate(--wl);
            l = word[wl-1];
        }

#ifndef NO_DICT_MODE
        // Don't add in Dict mode, because most cases will be false
        // positives (eg. user didn't look at screen), and the consequence
        // is bad (garbage match next time).
        InputMatcher *matcher = matcherSet->currentMode();
        if ( wl > 2 && !matcher->lookup() ) {
            qLog(Input) << "adding "<<word<<" to dictionary";
            addWordToDictionary(word);
        };
#endif
    }
    word.clear();
}

void PkIM::sendAndEnd(bool forceCommit)
{
    qLog(Input) << "PkIM::sendAndEnd() - word = "<<word<<", text = "<<text<<" choices = "<<choices;

    if (forceCommit || !text.isEmpty() ){
        sendCommitString(text,0,0);
        word += text;
    }

    InputMatcher *matcher = matcherSet->currentMode();
    matcher->reset();
    choices.clear();
    choicePos = -1;


    text.clear();
    bool modech = shift;
    shift = autoCapitalize && capitalizeNext;
    if (charList)
        charList->hide();
    if ( modech != shift)
        setModePixmap();
}

struct WordWraps
{
    const char *prefix;
    const char *postfix;
};

WordWraps wraps[] = {
    { 0, "'s"},
    { "\"", "'s\""},
    { "\"", "\""},
    { "\"", "'s"},
    { "\"", 0},
    { 0, 0},
};

void PkIM::compose()
{
    bool added_number = false;
    InputMatcher *matcher = matcherSet->currentMode();
    if ( matcher->count() > 0 )  {
        QString lastset = matcher->atReverse(0);
        // if best match for last char is punctuation
        // really should be 'lastset contains punc'
        if (lastset[0].isPunct()) {
            choices = matcher->findAll(text, QString());
            choice = 0;
            text = choices[choice];
        } else {
            capitalizeNext = false;
            choices = matcher->choices();
            qLog(Input) << "PkIM::compose() - choices = " << choices;
            if (matcher->count() == 1) {
                QStringList all = matcher->findAll();
                QStringList::Iterator it;
                for (it = all.begin(); it != all.end(); ++it)
                    if (!choices.contains(*it))
                        choices.append(*it);
            }

            WordWraps *w = wraps;
            while(choices.count() < 1 && (w->prefix != 0 || w->postfix != 0)) {
                choices = matcher->choices(true, false, w->prefix, w->postfix);
                w += 1;
            }
            // still no choices? try with number word
            w = wraps;
            while(choices.count() < 1 && (w->prefix != 0 || w->postfix != 0)) {
                QString nword = matcher->numberWord(w->prefix, w->postfix);
                if (!nword.isEmpty()) {
                    added_number = true;
                    choices += nword;
                }
                w += 1;
            }

            // still no choices, then try anyting that may fit in prefix/postfix
            w = wraps;
            while(choices.count() < 1 && (w->prefix != 0 || w->postfix != 0)) {
                // only returns a word if sets have error values...
                QString wword = matcher->writtenWord(w->prefix, w->postfix);
                if (!wword.isEmpty())
                    choices += wword;
                w += 1;
            }
            // always append the number word as a choice.
            if (!added_number) {
                QString nword = matcher->numberWord(QString(), QString());
                if (!nword.isEmpty()) {
                    added_number = true;
                    choices += nword;
                }
            }
            choice = 0;
            if ( choices.count() >= 1 ) {
                text = choices[choice];
            } else {
                int punc = text.length()-1;
                while ( punc>=0 && !text[punc].isPunct() )
                    punc--;
                // Punctuation in unfound word. Stop trying to compose.
                if ( punc >= 0 ) {
                    InputMatcherGuessList gl = matcher->pop();
                    sendPreeditString(text, text.length());
                    sendAndEnd();
                    endWord();
                    matcher->pushGuessSet(gl);
                    compose();
                    return;
                } else {
                    // try nword and written word.
                    QString nword = matcher->numberWord();
                    // only returns a word if sets have error values...
                    QString wword = matcher->writtenWord();
                    if (!nword.isEmpty())
                        choices += nword;
                    if (!wword.isEmpty() && wword != nword)
                        choices += wword;

                    // just grab anything we can.
                    if (choices.count() == 0) {
                        QString prefix = text;
                        while (choices.count() == 0) {
                            choices = matcher->findAll(prefix, QString());
                            if (prefix.isEmpty())
                                break;
                            prefix = prefix.mid(1);
                        }
                    }
                    text = choices[choice];
                }
            }
        }
    } else {
        text.clear();
    }
    qLog(Input) << "PkIM::compose() - text = "<<text<<", choices = "<<choices;
    sendPreeditString(text, text.length());
}

void PkIM::revertLookup()
{
    qLog(Input) << "PkIM::revertLookup()";
    actionsSinceChangeMode++;
    InputMatcher *matcher = matcherSet->currentMode();
    if ( matcher->count() ) {
        InputMatcherGuessList gl = matcher->pop();
        shift = gl.shift;
        text.truncate(matcher->count()-1);
        compose();

    }

    if ( !matcher->count() )
    {
        // sendAndEnd();
#ifndef QT_NO_QWS_CURSOR
        qwsServer->setCursorVisible(true);
#endif
    };

    if (word.length())
        word.truncate(word.length()-1);

    qwsServer->sendIMQuery ( Qt::ImMicroFocus );
}

void PkIM::appendLookup(const QString &set)
{
    qLog(Input) << "PkIM::appendLookup("<<set<<")";
    actionsSinceChangeMode++;
    // add new set,

    // SHOULD BE SAME from here in this func and same in appendGuess
    matcherSet->currentMode()->pushSet(set, shift);
    if (shift) {
        shift = false;
        setModePixmap();
    }
    compose();

    qwsServer->sendIMQuery ( Qt::ImMicroFocus );
}

void PkIM::appendGuess(const InputMatcherGuessList  &set)
{
    qLog(Input) << "PkIM::appendGuess(const InputMatcherGuessList  &set)";
    actionsSinceChangeMode++;

    InputMatcherGuessList gl = set;
    if (!matcherSet->currentMode()->lookup()) {
        if (gl.longest() != matcherSet->currentMode()->count()+1) {
            // just push and compose
            bool wasShift = shift;
            sendAndEnd();
            shift = wasShift;
        }
    }
    gl.shift = shift;
    // SHOULD BE SAME from here in this func and same in appendLookup
    matcherSet->currentMode()->pushGuessSet(gl);
    qwsServer->sendIMQuery ( Qt::ImMicroFocus );
    if (shift) {
        shift = false;
        setModePixmap();
    }
    compose();
}


void PkIM::setModePixmap()
{
    if ( status ) {
        QPixmap pm = matcherSet->currentMode()->pixmap(shift);
        status->setPixmap(pm);
    }
}

void PkIM::symbolPopup()
{
    if(restrictToHint())
        return;
    if (!symbolPicker) {
        symbolPicker = new SymbolPicker();
        connect(symbolPicker, SIGNAL(symbolClicked(int,int)),
                this, SLOT(symbolSelected(int,int)));
    }
    sendAndEnd();
    endWord(); // no symbols in words
    //    symbolPicker->setAppFont(font());
    symbolPicker->setMicroFocus(microX, microY); // j: TODO: microX,microY were 20,20 in QFSIMPenInput - figure out what these mean, and why.
    symbolPicker->show();
}


#ifdef PKIM_DEBUG_RECORD_MOUSE_EVENTS

static void dumpMouseEventsToFile(const QPoint &posIn, int state, int w)
{
    // This code is only for internal testing purposes.
    // setting this flag is not recommended.
    static int fileCounter = -1;
    static bool fileOpened = false;
    static QFile* Mouse_Event_Output_File;

    if ( state & Qt::LeftButton && !fileOpened )
    {
        ++fileCounter;
        QString prefix = QString::number(fileCounter);
        while (prefix.length() < 4) prefix.prepend('0');
        Mouse_Event_Output_File = new QFile("mouseevents"+prefix+".csv");
        fileOpened = Mouse_Event_Output_File->open(QIODevice::WriteOnly);
        if (!fileOpened)
            qWarning() << "!Failed to open file " << ("mouseevents"+prefix+".csv") << " for writing";
    };

    if(!fileOpened) {
        if(Mouse_Event_Output_File) {
            QFile::FileError e = Mouse_Event_Output_File->error();
            if(e)
                qWarning() << "Mouse_Event_Output_File not open, error is " << e;
        };
        return;
    };

    QTextStream out(Mouse_Event_Output_File);
    out << posIn.x() << ',' << posIn.y() << ',' << state << ',' << w << '\n';
    out.flush();
    if( !(state & Qt::LeftButton) )
    {
        Mouse_Event_Output_File->close();
        fileOpened = false;
    }
}
#endif

/*!
  \internal
  Filters mouse events for full screen handwriting input.
 */
bool PkIM::filter(const QPoint &posIn, int state, int w)
{
#ifdef PKIM_DEBUG_RECORD_MOUSE_EVENTS
    dumpMouseEventsToFile(posIn, state, w);
#endif

    QPoint pos = posIn;
    //    Q_ASSERT(penMatcher->charSet() != 0); // off - suddenly failing.

    //    if(cMode == Off) return false;
    if (!midStroke) {
        if (symbolPicker && symbolPicker->isVisible())
            return symbolPicker->filterMouse(pos, state, w);

        if (wordPicker && wordPicker->isVisible())
            return wordPicker->filterMouse(pos, state, w);
        if (modePicker && modePicker->isVisible())
            return modePicker->filterMouse(pos, state, w);
    }

    if (!active || !profile) {
        return false; // not filtering
    }

    if (!allowHandwriting)
        return false;

    qwsServer->sendIMQuery ( Qt::ImMicroFocus );

    // Looks like the stroke is ours, so translate it into the local context
    if(qApp) {
        if(qt_screenstroke == 0)
            qt_screenstroke = new QWSScreenStroke();
    };

    if(qt_screenstroke)
        pos = qt_screenstroke->mapFromGlobal(posIn);
    else
        qLog(Input) << "qt_screenstroke not instantiated. Failing to translate mouse pos";

    if (state & Qt::LeftButton) {
        return filterMouseButtonDown(pos, state, w);
    } else {
        return filterMouseButtonUp(pos, state, w);
    };
}


/*!
    A helper function that implements the logic for handling mouse input
    while the mouse button is pressed.  Returns true if the mouse event is
    consumed, or false if the event is rejected (and should be passed on
    to the system).
*/
bool PkIM::filterMouseButtonDown(const QPoint &pos, int state, int w)
{
    Q_UNUSED(state);
    Q_UNUSED(w);
    if(lStrokeTimer->isActive())lStrokeTimer->stop();
    if (ignoreNextMouse) {
        // ignore mouse events until mouse release
        return false;
        // XXX updateStatusIcon();
    }
    if (midStroke) {
        // mouse move
        Q_ASSERT(inputStroke);
        if (could_be_left_click && (pos-inputStroke->startingPoint()).manhattanLength() > strokeThreshold) {
            lClickTimer->stop();
            could_be_left_click = false;
        }
        int dx = qAbs( pos.x() - lastPoint.x() );
        int dy = qAbs( pos.y() - lastPoint.y() );
        if ( dx + dy > 1 ) {
            inputStroke->addPoint( pos );
        }
    } else {
        // Mouse pressed and not already tracking stroke, so start a new
        // stroke.

        if (qApp) {
            /* first check largest scroll bar size (1/8th of screen)
               then check actual scroll bar size */
            int dw = QApplication::desktop()->availableGeometry().width();
            if (pos.x() > dw - (dw >> 3)
                    // QStyle::PM_ScrollBarExtent
                    && pos.x() > dw - qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent))
                return false;
        }
#ifndef QT_NO_QWS_CURSOR
        qwsServer->setCursorVisible(false);
#endif
        // mouse press
        lClickTimer->start(profile->ignoreStrokeTimeout()); // make configurable
        could_be_left_click = !lStrokeTimer->isActive();
        lStrokeTimer->stop();
        inputStroke = new QIMPenStroke;
        // should be configurable.  canvas height should be user-drawing height.
        inputStroke->setCanvasHeight(canvasHeight);
        inputStroke->beginInput(pos);
        midStroke = true;
    }
    fs_append_stroke(pos);
    lastPoint = pos;
    return true;
}

/*!
    A helper function that implements the logic for handling mouse input
    while the mouse button is not pressed.  Returns true if the mouse event
    is consumed,  or false if the event is rejected (and should be passed on to the system).
*/
bool PkIM::filterMouseButtonUp(const QPoint &pos, int state, int w)
{
    Q_UNUSED(pos);
    Q_UNUSED(state);
    // take the first (released) mouse event,
    // but not the rest.
    if (ignoreNextMouse) {
        // !((state & Qt::LeftButton) marks the end of a stroke,
        // so we've finished ignoring a mouse stroke.
        ignoreNextMouse = false;
    }
    if (!midStroke)
        return false;
    midStroke = false;

    if (inputStroke) {
        inputStroke->endInput();

#ifndef QT_NO_QWS_CURSOR
        qwsServer->setCursorVisible(true);
#endif
        lClickTimer->stop();
        fs_fade_stroke();
        if (could_be_left_click) {
            fs_end_stroke(); // not a stroke (it's a click-through), so end
            // this hides the fullscreen widget so it doesn't get
            // the mouse events about to be created

            protectInputStroke = true;
            // send mouse
            // must have started a stroke, so we'll assume we've got a qt_screenstroke
            if(qt_screenstroke){
                sendAndEnd();
                sendMouseEvent(qt_screenstroke->mapToGlobal(
                            inputStroke->startingPoint()), Qt::LeftButton, w);
                sendMouseEvent(qt_screenstroke->mapToGlobal(
                            inputStroke->startingPoint()), Qt::NoButton, w);
            } else {
                qWarning() << "PkIM: qt_screenstroke not instantiated when it should be";
                sendMouseEvent(inputStroke->startingPoint(), Qt::LeftButton, w);
                sendMouseEvent(inputStroke->startingPoint(), Qt::NoButton, w);
            }
            protectInputStroke = false;
        } else {
            processStroke(inputStroke);
            int lch = penMatcher->lastCanvasHeight();
            // ignore outriders.
            if (lch && (lch >> 1) < canvasHeight && (canvasHeight >> 1) < lch) {
                // um, may modify ratio to make slower or faster adjustment?
                canvasHeight = int(float(canvasHeight)*0.9+float(lch)*0.1);
            }
            // end of stroke, start lStrokeTimer
            lStrokeTimer->start(multiStrokeTimeout); // make configurable
        }
        delete inputStroke;
        inputStroke = 0;
        return true;
    } else {
        return false;
    };
}

#ifdef PKIM_VERBOSE_OUTPUT
#define REPORT_FILTER_KEY_RESULT_MACRO(x) qDebug() << "PkIM::filter(" << unicode << ", " << keycode << ", " << modifiers <<  ", " << (isPress?"true":"false") << ", "  << (autoRepeat?"true":"false") << ") " << x
#define REPORT_PROCESS_CHAR_RESULT_MACRO(x) qDebug() << "PkIM::processInputMatcherChar(" << unicode << ", " << keycode << ", " <<  ", " << (isPress?"true":"false") <<  ", "<< (autoRepeat?"true":"false") << ") " << x
#else
#define REPORT_FILTER_KEY_RESULT_MACRO(x)
#define REPORT_PROCESS_CHAR_RESULT_MACRO(x)
#endif // PKIM_VERBOSE_OUTPUT

/*!
    \internal
    Processes input based on InputMatcherChar
*/
bool PkIM::processInputMatcherChar(InputMatcher* matcher, int unicode, int keycode, bool isPress, bool autoRepeat)
{
    InputMatcherChar item = (matcher->map())[unicode];
    // If key has a hold function, use autorepeats to progress hold status
    if (autoRepeat && key_hold_status != null ) {
        if (key_hold_status == waiting_for_hold) {
            REPORT_PROCESS_CHAR_RESULT_MACRO(" - consumed, hold detected.  Applying hold function");
            actionsSinceChangeMode++;
            key_hold_status = waiting_for_release;

            // Do hold
            InputMatcherChar item = hold_item;
            // insertText different for hold and tap
            if (item.holdfunc == insertText) {
                // send the hold characer
                text += item.holdarg;
                sendAndEnd();
            } else if (item.holdfunc != noFunction) {
                applyFunction(item.holdfunc);
            }

            return true;
        } else if (key_hold_status == waiting_for_release) {
            // consume autorepeats for keys with a hold function
            REPORT_PROCESS_CHAR_RESULT_MACRO(" - autorepeat consumed while waiting for release");
            return true;
        }
    };

    // Handle presses.
    // If there's a hold function, start the hold process.
    // If there's a tap function, consume the event and wait for release
    // If there's no function (the key is unrecognized), end any word,
    // ignore the key, and pass it on.
    if (isPress) {
        //check if there is a hold for it.
        if (item.holdfunc != noFunction) {
            key_hold_status = waiting_for_hold;
            hold_uc = unicode;
            hold_key = keycode;
            hold_item = item;
            REPORT_PROCESS_CHAR_RESULT_MACRO(" - consumed, setting hold for it");
            return true; // we will hold, need to wait for release
        }
        // XXX Add check for single char taparg.
        // nature of func may need to wait till we have spec that uses it though.
        // in this case should just modify uni-code and pass through.
        if (item.tapfunc != noFunction) {
            REPORT_PROCESS_CHAR_RESULT_MACRO(" - consumed, no action till key release");
            return true; // we will do it on release.
        }
        // no tap set, no hold set.  send, end, and let this key go through.
        REPORT_PROCESS_CHAR_RESULT_MACRO(" - rejected and terminating word, no tap set or hold set");
        sendAndEnd();
        return false;
    } else {     // release
        // if it's a hold, we've already acted on it, so consume it
        // but take no action.
        if ( key_hold_status == waiting_for_release) {
            key_hold_status = null;
            return true;
        }

        actionsSinceChangeMode++;
        // if it's not a hold, then it's a press
        // (autorepeats for hold have been consumed already, so we know
        // that we want to act on it here whether or not it's an
        // autorepeat).

        // check out tap functionality.
        // XXX Add check for single char taparg.
        // this function different depending if tap or hold.
        if (item.tapfunc == insertText) {
            if (!matcher->lookup()) {
                if ( tid_abcautoend ){
                    killTimer(tid_abcautoend);
                    tid_abcautoend = 0;
                }
                if (unicode == lastUnicode && item.taparg.length() > 1) {
                    nextWord();
                } else {
                    bool wasShift = shift;
                    sendAndEnd();
                    shift = wasShift;

                    appendLookup(item.taparg);

                    if (choices.count() == 1) {
                        sendAndEnd();
                    }
                }
            } else {
                appendLookup(item.taparg);
            }
            if (!charList)
                charList = new CharList();

            // SHOULD BE SHARED - only with the newer drawing method
            if (!matcher->lookup() && choices.count() > 1) {
                if (item.showList) {
                    charList->setChars(choices);
                    if ( !charList->isVisible() )
                        charList->setMicroFocus(microX, microY);
                    charList->setCurrent(choices[choice]);
#ifdef PKIM_VERBOSE_OUTPUT
                    qLog(Input) << "CharList being shown at " << microX << ", " << microY;
#endif // PKIM_VERBOSE_OUTPUT
                    charList->show();
                }
                tid_abcautoend = startTimer(ABC_AUTOEND_TIME);
            }
            lastUnicode = unicode;
            REPORT_PROCESS_CHAR_RESULT_MACRO(" - consumed - inserting text");
            return true;
        }
        if (item.tapfunc != noFunction) {
            REPORT_PROCESS_CHAR_RESULT_MACRO(" - consumed - applying tap function");
            applyFunction(item.tapfunc);
            return true;
        }
        // No tap function, so reject key.
        REPORT_PROCESS_CHAR_RESULT_MACRO(" - rejected, no tap function");
        return false; // sendAndEnd done on press
    }
    REPORT_PROCESS_CHAR_RESULT_MACRO(" - rejected. unicode in matcher map, but not processed, or no tap function");
    return false; // sendAndEnd done on press
}

/*!
    A simple helper function for PkIM.  Remove the last character from the preedit text and word.  If this leaves the preedit text empty, end the preedit session.

    A true return value consumes the key event that triggered this call.
    \sa filter()
*/
bool PkIM::processBackspace()
{
    if (word.length()) {
        word.truncate(word.length()-1);
    }
    if (text.length()) {
        revertLookup();
        if(!text.length())
            sendAndEnd(true);
        return true;
    }
    return false;
}


/*!
    \internal
    Filters key events for keypad/dictionary style input.

    This is far too complicated and needs to be refactored.

    Always ignore the Hangup key, and ignore everything ig the IM is off.

    Assuming that this is a new press, send it to \l processInputMatcherChar() to find out what it does, and whether it has a hold function.

    If the key has a hold function, we don't know yet whether it has been held for long enough (\l timerEvent()), so consume autorepeats without doing anything until we know.
*/
bool PkIM::filter(int unicode, int keycode, int modifiers,
        bool isPress, bool autoRepeat)
{
    if (Qt::Key_Hangup == keycode) {
        if ( symbolPicker ) symbolPicker->hide();
        if ( wordPicker ) wordPicker->hide();
        if ( modePicker ) modePicker->hide();
        return false;
    }

    if (symbolPicker && symbolPicker->isVisible()) {
        if (!symbolPicker->filterKey(unicode, keycode, modifiers, isPress, autoRepeat))
            symbolPicker->hide();
        REPORT_FILTER_KEY_RESULT_MACRO(" - forwarded to symbolPicker");
        return true;
    }

    if (wordPicker && wordPicker->isVisible()) {
        if (!wordPicker->filterKey(unicode, keycode, modifiers, isPress, autoRepeat))
            wordPicker->hide();
        REPORT_FILTER_KEY_RESULT_MACRO(" - forwarded to wordPicker");
        return true;
    }

    if (modePicker && modePicker->isVisible()) {
        if (!modePicker->filterKey(unicode, keycode, modifiers, isPress, autoRepeat))
            modePicker->hide();
        REPORT_FILTER_KEY_RESULT_MACRO(" - forwarded to modePicker");
        return true;
    }

    // doesn't filter anything if off
    if ( !active ) {
        REPORT_FILTER_KEY_RESULT_MACRO(" - rejected, IM not active");
        return false;
    };

    if (Qt::Key_Select == keycode && !text.isEmpty()) {
        sendAndEnd();
        REPORT_FILTER_KEY_RESULT_MACRO(" - Key_Select while preediting, commiting word and consuming key");
        return true;
    };

    // check for the "toggle mode" key that is present on some keypads.
    if (Qt::Key_F26 == keycode) {
        if (!isPress)
            toggleMode();
        return true;
    }

    /*if (autoRepeat && unicode) {
        REPORT_FILTER_KEY_RESULT_MACRO(" - rejected, autorepeat without hold function");
        return false;
    };*/

    qwsServer->sendIMQuery ( Qt::ImMicroFocus );
    // may change this to table lookup.
    InputMatcher *matcher = matcherSet->currentMode();
    QMap<QChar, InputMatcherChar> set = matcher->map();

    if (set.contains(unicode)) {
        return processInputMatcherChar(matcher, unicode, keycode, isPress, autoRepeat);
    } else if ( isPress && ((keycode == Qt::Key_Backspace)
                || (keycode == Qt::Key_Back)
                )) {
        qLog(Input) << " PkIM::filter() - ((isPress || !Qtopia::mousePreferred()) && ("<<"keycode == "<<(keycode==Qt::Key_Backspace?"Qt::Key_Backspace":"Qt::Key_Back")<<")";
        return processBackspace();
        sendAndEnd();  // dead code.
        qLog(Input) << "calling sendAndEnd on empty text";
    } else if (isPress && set.count() > 0) {
        qLog(Input) << "PkIM::filter() - (isPress && set.count() > 0)";
        /* if not in set, (and set not empty), and not any other special key
           (like Back) then send and end.
           This includes events sent to qvfb from the desktop keyboard. */
        sendAndEnd();
        word.clear();  // better to lose a potential word than add garbage.
        REPORT_FILTER_KEY_RESULT_MACRO(" - rejected, not recognized. (word ended)");
        return false;
    }
    REPORT_FILTER_KEY_RESULT_MACRO(" - rejected");
    return false;
}

// different per phone and mode.
void PkIM::timerEvent(QTimerEvent* e)
{
    if ( e->timerId() == tid_abcautoend ) {
        lastUnicode = 0;
        sendAndEnd();
        killTimer(tid_abcautoend);
        tid_abcautoend = 0;
    }
}

void PkIM::applyFunction(InputMatcherFunc fn)
{
    qLog(Input) << "void PkIM::applyFunction(InputMatcherFunc "<<fn<<")";
    switch(fn) {
        case changeShift:
            toggleShift();
            break;
        case changeMode:
            toggleMode();
            break;
        case modifyText:
            nextWord();
            break;
        case insertSpace:
            qLog(Input)<< "applyFunction(InputMatcher) insertSpace";
            endWord();
            QWSServer::sendKeyEvent(0x20, Qt::Key_Space, 0, true, false);
            break;
        case insertText:
            // shouldn't get here
            qWarning("insertText function not supposed to be handled in PkIM::applyFunction()");
            break;
        case insertSymbol:
            endWord();
            symbolPopup();
            break;
            // not implemented.
        case changeInputMethod:
            QtopiaIpcEnvelope("QPE/InputMethod", "changeInputMethod()");
            break;
        case noFunction:
            // do nothing.
            ;
    }
}

void PkIM::toggleShift()
{
    shift = !shift;
    setModePixmap();
}

void PkIM::toggleMode()
{
    qLog(Input) << "PkIM::toggleMode()";
    lastUnicode = 0;
    if(restrictToHint())
        return;
    sendAndEnd();
    endWord();
    lastUnicode=0;
    // press, release.. here.
    if (actionsSinceChangeMode < 3) {
        matcherSet->nextMode();
    } else {
        matcherSet->toggleHinted();
    }
    if(!shift && autoCapitalize && capitalizeNext) toggleShift();
    setModePixmap();
    actionsSinceChangeMode = 0;
}

void PkIM::nextWord()
{
    if ( choices.count()) {
        choice = (choice+1)%choices.count();
        text = choices[choice];
        sendPreeditString(text, text.length());
    }
    qLog(Input) << "PkIM::nextWord() - text is "<<text<<" choice is "<<choice;
}


void PkIM::wordPopup()
{
    if (choices.count()) {
        if (!wordPicker) {
            wordPicker = new WordPicker();
            connect(wordPicker, SIGNAL(wordChosen(QString)),
                    this, SLOT(wordSelected(QString)));
        }
        wordPicker->setChoices(choices);
        //  wordPicker->setAppFont(font());
        wordPicker->setMicroFocus(microX, microY); // j: 20,20 in qfsinput
        wordPicker->show();
    }
    qLog(Input) << "PkIM::wordPopup() - text is "<<text<<" choice is "<<choice;
}


void PkIM::modePopup()
{
    if (restrictToHint())
        return;
    if (!modePicker) {
        modePicker = new ModePicker(matcherSet);
        connect(modePicker, SIGNAL(modeSelected(QString,bool)),
                this, SLOT(setMode(QString,bool)));
    }
    //    modePicker->setAppFont(font());
    modePicker->setMicroFocus(microX, microY);
    modePicker->show();
}

void PkIM::loadProfiles()
{
    qLog(Input) << "PkIM::loadProfiles()";
    profileList.clear();
    profile = 0;
    //if (shortcutCharSet)
    //delete shortcutCharSet;
    //shortcutCharSet = new QIMPenCharSet();
    //shortcutCharSet->setTitle( tr("Shortcut") );
    QString path = Qtopia::qtopiaDir() + "etc/qimpen";
    qLog(Input) << "PkIM::loadProfiles() << path="<<path;
    QDir dir( path, "*.conf" );
    QStringList list = dir.entryList();
    //    qLog(Input) << "PkIM::loadProfiles() list = "<<list;  // This is causing segfaults when inline optimizations on some hardward.  When this is resolved, this should be uncommented.
    QStringList::Iterator it;

    for ( it = list.begin(); it != list.end(); ++it ) {
        QIMPenProfile *p = new QIMPenProfile( path + '/' + *it );
        profileList.append( p );
        //if ( p->shortcut() ) {
        //QIMPenCharIterator it( p->shortcut()->characters() );
        //for ( ; it.current(); ++it ) {
        //shortcutCharSet->addChar( new QIMPenChar(*it.current()) );
        //}
        //}
    }

    qLog(Input) << "PkIM::loadProfiles() - profileList.size() = "<< profileList.size();

    QSettings config("Trolltech","handwriting");
    config.beginGroup( "Settings" );
    QString prof = config.value( "FSProfile", "fs" ).toString();
    // 150 is just the default.  Needs to be close (e.g about what is expected on device for most people)
    // to start with, but _WILL_ adapt over time.
    canvasHeight = config.value( "CanvasHeight", 150).toInt();
    strokeThreshold = config.value( "StrokeThreshold", 4).toInt();
    if(!hintedAutoCapitalization)
        autoCapitalize = config.value( "AutoCapitalize", true).toBool();
    autoCapitalizingPunctuation = config.value( "AutoCapitalizingPunctuation", ".!?").toString();

    selectProfile( prof );
}

void PkIM::processStroke(QIMPenStroke *stroke)
{
    qLog(Input) << "PkIM::processStroke(QIMPenStroke *stroke)";
    penMatcher->addStroke(stroke);
    QList<QFSPenMatch::Guess> ml = penMatcher->currentMatches();
    QListIterator<QFSPenMatch::Guess> it(ml);
    QString tmpString;
    QFSPenMatch::Guess tmpGuess;
    while(it.hasNext()) {
        tmpGuess = it.next();
        tmpString += QString(tmpGuess.text) + ", ";
    }
    sendMatchedCharacters(ml);
}

void PkIM::selectProfile( const QString &ident )
{
    qLog(Input) << "PkIM::selectProfile - ident="<<ident;
    QListIterator<QIMPenProfile *> it( profileList );

    QIMPenProfile *targetprofile  = 0;
    while(it.hasNext()) {
        QIMPenProfile *p = it.next();
        if ( p->identifier() == ident ) {
            targetprofile = p;
            qLog(Input) << "PkIM::selectProfile - found target profile with charsets "<<p->charSets().size() ;
            break;
        }
    }

    if ( !targetprofile ) {
        qLog(Input) << "PkIM::selectProfile -FAILED TO FIND TARGET PROFILE - PROFILE NOT SET";
        return;
    };

    profile = targetprofile;

    //pw->clearCharSets();
    baseSets.clear();

    multiStrokeTimeout = profile->multiStrokeTimeout();

    penMatcher->clear();
    penMatcher->setCharSet( profile->charSet("Text") ); // no tr

    if ( profile->charSet("Text") ) // no tr
        baseSets.append( profile->charSet("Text"));  // no tr

    if ( profile->charSet("Number") ) { // no tr
        baseSets.append( profile->charSet("Number") ); // no tr
    }
}

QList<QFSPenMatch::Guess> punctuationGuess(int length, int error)
{
    qLog(Input) << "QList<QFSPenMatch::Guess> punctuationGuess(int "<<length<<", int "<<error<<")";
    QFSPenMatch::Guess guess;
    QList<QFSPenMatch::Guess> result;

    guess.length = length;

    guess.text = '.';
    guess.key = Qt::Key_Period;
    guess.error = error++;
    result.append(guess);

    guess.text = ',';
    guess.key = Qt::Key_Comma;
    guess.error = error++;
    result.append(guess);

    guess.text = '\'';
    guess.key = Qt::Key_QuoteDbl; // could'nt find right key
    guess.error = error++;
    result.append(guess);

    guess.text = '"';
    guess.key = Qt::Key_QuoteDbl;
    guess.error = error++;
    result.append(guess);

    guess.text = ':';
    guess.key = Qt::Key_Colon;
    guess.error = error++;
    result.append(guess);

    guess.text = '-';
    guess.key = Qt::Key_Minus;
    guess.error = error++;
    result.append(guess);

    return result;
}

// matcher will send erase when it changes its mind...
/*!
  Adds the characters in the character list \a cl to its current state.  This may result
  in sending key events or adding letters to the currently composed word.

 */
void PkIM::sendMatchedCharacters(const QList<QFSPenMatch::Guess> &cl)
{
    qLog(Input) << "PkIM::sendMatchedCharacters(const QList<QFSPenMatch::Guess> &cl) ";
    // if non guess, end word, apply action.
    // if guess then, then create list and send.
    // <word ending guesses?  QStringList does not
    // include whether it ends the word or not.  but would like to know for x, \r and t, space
    // collisions.

    if (cl.empty())
    {
        qLog(Input) << "PkIM::sendMatchedCharacters() - cl.empty() == true;";
        return;
    };
    QFSPenMatch::Guess ch = cl.first();

    switch(ch.key) {
        // will be filtered by our own filter method,
        // hence has same behavior as going through
        // key filter.
        case Qt::Key_Backspace:
            QWSServer::sendKeyEvent( 8, Qt::Key_Backspace, 0, true, false);
            QWSServer::sendKeyEvent( 8, Qt::Key_Backspace, 0, false, false);
            break;
        case Qt::Key_Tab:
            endWord();
            QWSServer::sendKeyEvent( 9, Qt::Key_Tab, 0, true, false);
            QWSServer::sendKeyEvent( 9, Qt::Key_Tab, 0, false, false);
            break;
        case Qt::Key_Return:
            endWord();
            QWSServer::sendKeyEvent( 13, Qt::Key_Return, 0, true, false);
            QWSServer::sendKeyEvent( 13, Qt::Key_Return, 0, false, false);
            break;
        case Qt::Key_Escape:
            endWord();
            QWSServer::sendKeyEvent( 27, Qt::Key_Escape, 0, true, false);
            QWSServer::sendKeyEvent( 27, Qt::Key_Escape, 0, false, false);
            break;
        case Qt::Key_Space:
            qLog(Input) << "case Qt::Key_Space: ("<<Qt::Key_Space<<")";
            endWord();
            QWSServer::sendKeyEvent(Qt::Key_Space, Qt::Key_Space, 0, true, false);
            QWSServer::sendKeyEvent(Qt::Key_Space, Qt::Key_Space, 0, false, false);
            break;
        case QIMPenChar::Punctuation:
            appendGuess(punctuationGuess(ch.length, ch.error));
            break;
        case QIMPenChar::NextWord:
            nextWord();
            break;
        case QIMPenChar::Caps:
            toggleShift();
            break;
        case QIMPenChar::SymbolPopup:
            endWord();
            symbolPopup();
            break;
        case QIMPenChar::WordPopup:
            wordPopup();
            break;
        case QIMPenChar::ModePopup:
            endWord();
            modePopup();
            break;
        default:
            appendGuess(cl);
            break;
    }
}

void PkIM::appendGuess(const QList<QFSPenMatch::Guess> &cl)
{
    InputMatcherGuessList send;
    foreach (QFSPenMatch::Guess guess, cl) {
        IMIGuess imiguess;
        imiguess.c = guess.text.unicode();
        imiguess.length = guess.length;
        imiguess.error = guess.error;
        send.append( imiguess );
    }
    if (send.count())
        appendGuess(send);
}

void PkIM::updateWord()
{
    if (choicePos == -1) {
        // no words, clear IM
        //        QInputMethodEvent ime;
        //        sendIMEvent(QWSServer::InputMethodCommitToPrev, "", 0); // TEMP
    } else {
        QString choice = choices[choicePos];
        QList<QInputMethodEvent::Attribute> attributes;
        QInputMethodEvent *ime = new QInputMethodEvent(choice, attributes);
        //        sendIMEvent(QWSServer::InputMethodPreedit, choice, choice.length()); // TEMP
        sendEvent(ime);
    }
    qLog(Input) << "PkIM::updateWord() - choicePos is "<<choicePos<<((choicePos<choices.size() && choicePos>=0)?choices[choicePos]:"");
}

void PkIM::updateChoices()
{
    qLog(Input) << "PkIM::updateChoices() - choice pos = "<< choicePos;
    QString lastChosen;
    if (choicePos != -1)
        lastChosen = choices[choicePos];

    if (wordMatcher->count() == 0) {
        choices.clear();
        choicePos = -1;
        //  sendIMEvent(QWSServer::InputMethodCommitToPrev, "", 0); // TEMP
        return;
    }
    // if last is punct, then last choice plus punct choices.
    if (choicePos != -1 && wordMatcher->atReverse(0)[0].isPunct()) {
        choices = wordMatcher->findAll(choices[choicePos], QString());
    } else {
        choices = wordMatcher->allChoices();
        if (choices.count() < 1) {
            // Set empty, try punctuation, then any kind of word.
            int punc = lastChosen.length()-1;
            while ( punc>=0 && !lastChosen[punc].isPunct() )
                punc--;
            // Punctuation in unfound word. Stop trying to compose.
            if ( punc >= 0 ) {
                InputMatcherGuessList gl = wordMatcher->pop();
                //      sendIMEvent( QWSServer::InputMethodCommitToPrev, lastChosen, lastChosen.length(), 0 ); // TEMP
                wordMatcher->reset();
                wordMatcher->pushGuessSet(gl);
                choices = wordMatcher->allChoices();
                return;
            }

            // try nword and written word.
            QString nword = wordMatcher->numberWord();
            // only returns a word if sets have error values...
            QString wword = wordMatcher->writtenWord();
            if (!nword.isEmpty())
                choices += nword;
            if (!wword.isEmpty() && wword != nword)
                choices += wword;
        }
    }

    if (choices.count()) {
        choicePos = 0;
    } else {
        // should have case to handle unfound words.  (e.g. change mode and spell suggestion)
        choicePos = -1;
    }
    updateWord();
}

void PkIM::removeStroke()
{
    qLog(Input) << "PkIM::removeStroke();";
    fs_end_stroke();
    penMatcher->clear();
}

void PkIM::ignoreRestOfStroke()
{
    qLog(Input) << "PkIM::ignoreRestOfStroke()";
    fs_end_stroke();
#ifndef QT_NO_QWS_CURSOR
    qwsServer->setCursorVisible(true);
#endif
    protectInputStroke = true;
    Q_ASSERT(inputStroke);
    Q_ASSERT(qt_screenstroke);
    sendAndEnd();
    sendMouseEvent(qt_screenstroke->mapToGlobal(inputStroke->startingPoint()), Qt::LeftButton, 0); //TEMP
    protectInputStroke = false;
    delete inputStroke;
    inputStroke = 0;
    lClickTimer->stop();
    could_be_left_click = false;
    ignoreNextMouse = true;
    midStroke = false;
    // XXX updateStatusIcon();
}

void PkIM::pluginMessage(const QString &m, const QByteArray &a)
{
    if (m == "settingsChanged()") {
        qLog(Input) << "PkIM::pluginMessage(const QString &"<< m<< ", const QByteArray &"<< a << ") - loadProfiles()";
        loadProfiles();
    }
}

/*!
    This function handles update information from the server.  \a type describes what has updated, as described in QWSInputMethod.
    \sa QWSInputMethod::updateHandler()
*/
void PkIM::updateHandler ( int type )
{
    switch(type){
        case(QWSInputMethod::Update):
            sendQuery(Qt::ImMicroFocus); // TODO: Check whether this made all the other microfocus queries redundant
            break;
        case(QWSInputMethod::FocusIn):
            break;
        case(QWSInputMethod::FocusOut):
            sendAndEnd(); // PkIM recieves Reset before FocusOut, and reset calls sendAndEnd(), so this is mostly for completeness
            break;
        case(QWSInputMethod::Reset):
            reset();
            break;
        case(QWSInputMethod::Destroyed):
            break;
        default:
            ;
    };
};

/*!
    Returns the current password hint status.  Input methods may react to this in different ways, and at their own discretion, but common actions include obscuring preedit text or not using dictionaries.
*/
bool PkIM::passwordHint() const
{
    return mPasswordHint;
};

/*!
    Sets the current password hint to \a password.
*/
void PkIM::setPasswordHint(bool password)
{
    mPasswordHint = password;
};
