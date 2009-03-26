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

#include "deskcallscreen.h"
#include "deskcallscreendialogs.h"
#include "qtopiaserverapplication.h"
#include "dialercontrol.h"
#include "servercontactmodel.h"
#include "abstractaudiohandler.h"
#include "private/homewidgets_p.h"

#include <QValueSpaceItem>
#include <QValueSpaceObject>
#include <QContact>
#include <QContactModel>
#include <QtopiaServiceRequest>
#include <QtopiaIpcEnvelope>
#include <QPhoneProfileManager>
#include <QAudioStateConfiguration>
#include <QAudioStateInfo>
#include <QSoftMenuBar>

#include <QPainter>
#include <QKeyEvent>
#include <QShowEvent>
#include <QLabel>
#include <QList>
#include <QMapIterator>
#include <QTimer>


static const uint callscreen_SECS_PER_HOUR = 3600;
static const uint callscreen_SECS_PER_MIN  = 60;

static const QFont callscreen_FONT(QLatin1String("DejaVu Sans Condensed"), 9, QFont::Bold);

static const QColor callscreen_BLUE(0, 100, 146);
static const QColor callscreen_LIGHTGRAY(230, 230, 230);

// note the widget must have autoFillBackground() set to true for the gradient to show
static void callscreen_applyLinearGradient(QWidget *w, const QGradientStops &gradientStops)
{
    QLinearGradient grad(0, 0, 0, w->height());
    grad.setStops(gradientStops);
    QPalette pal = w->palette();
    pal.setBrush(QPalette::Window, QBrush(grad));
    w->setPalette(pal);
}



class DeskphoneCallData {
public:
    DeskphoneCallData() : needsAutoVideoCheck(true), wasIncoming(false) {}
    DeskphoneCallData(const QPhoneCall &c) : call(c), needsAutoVideoCheck(true), wasIncoming(false) {
        // Get the number or name to display in the text area.
        numberOrName = call.number();

        QContact cnt;
        QContactModel *m = ServerContactModel::instance();
        if (!call.contact().isNull()) {
            cnt = m->contact(call.contact());
        } else if (!numberOrName.isEmpty()) {
            cnt = m->matchPhoneNumber(numberOrName);
        }

        if (!cnt.uid().isNull()) {
            numberOrName = cnt.label();
            ringTone = cnt.customField( "tone" );
            numberType = describePhoneNumberType(cnt, call.number());
        }

        contact = cnt;
    }

    QString durationString(bool showSeconds) {
        if (!call.incoming() && !call.dialing()) {
            if (!connectTime.isNull()) {
                int elapsed;
                if (disconnectTime.isNull()) {
                    elapsed = connectTime.secsTo(QDateTime::currentDateTime());
                } else {
                    elapsed = connectTime.secsTo(disconnectTime);
                }
                return DeskphoneCallScreen::callDurationString(elapsed, showSeconds);
            }
        }
        return QString();
    }

    QString displayName() const {
        return numberOrName.isEmpty()
                ? DeskphoneCallScreen::tr("Unknown caller")
                : numberOrName;
    }

    static QString describePhoneNumberType(const QContact &contact, const QString &number) {
        QMapIterator<QContact::PhoneType, QString> i(contact.phoneNumbers());
        while (i.hasNext()) {
            if (i.next().value() == number) {
                switch (i.key()) {
                    case QContact::HomePhone:
                    case QContact::HomeMobile:
                    case QContact::HomeFax:
                    case QContact::HomePager:
                    case QContact::HomeVOIP:
                        return QObject::tr("home");
                    case QContact::BusinessPhone:
                    case QContact::BusinessMobile:
                    case QContact::BusinessFax:
                    case QContact::BusinessPager:
                    case QContact::BusinessVOIP:
                        return QObject::tr("work");
                    case QContact::OtherPhone:
                        return QString();
                    case QContact::Mobile:
                        return QObject::tr("mobile");
                    case QContact::Fax:
                        return QObject::tr("fax");
                    case QContact::Pager:
                        return QObject::tr("pager");
                    case QContact::VOIP:
                        return QObject::tr("VOIP");
                }
            }
        }
        return QString();
    }

    QPhoneCall call;
    QPhoneCall::State callState;
    QDateTime dialTime;
    QDateTime connectTime;
    QDateTime disconnectTime;
    QString ringTone;
    QContact contact;
    QString numberOrName;
    QString numberType;
    QString state;
    bool needsAutoVideoCheck;
    bool wasIncoming;
};

//===========================================================================

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    VideoWidget(QWidget *parent = 0) : QWidget(parent) {}
    ~VideoWidget() {}

    virtual QRect globalVideoRect() const = 0;
    virtual void setTransparent(bool value) = 0;
    virtual bool isTransparent() const = 0;
    virtual void setContact(const QContact &contact) = 0;

signals:
    void geometryChanged();
    void pressed();

protected:
    void moveEvent(QMoveEvent *e) {
        QWidget::moveEvent(e);
        emit geometryChanged();
    }
    void resizeEvent(QResizeEvent *e) {
        QWidget::resizeEvent(e);
        emit geometryChanged();
    }
    void mousePressEvent(QMouseEvent *e) {
        QWidget::mousePressEvent(e);
        emit pressed();
    }
};

// this widget can be drawn without the pixmap inside (i.e. for video), if set to transparent
class TransparentFramedImageWidget : public FramedContactWidget
{
    Q_OBJECT
public:
    TransparentFramedImageWidget(QWidget *parent = 0) : FramedContactWidget(parent), transparent(false) {}
    ~TransparentFramedImageWidget() {}

    void setTransparent(bool value) {
        if (transparent != value) {
            transparent = value;
            //setAttribute(Qt::WA_OpaquePaintEvent, transparent);
            update();
        }
    }

    bool isTransparent() const { return transparent; }

protected:
    void paintEvent(QPaintEvent *);

private:
    bool transparent;
};

void TransparentFramedImageWidget::paintEvent(QPaintEvent *e)
{
    if (transparent) {
        // clear the widget area for the video stream
        QPainter p(this);
        p.fillRect(rect(), parentWidget()->palette().brush(QPalette::Window));
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.setBrush(QColor(0, 0, 0, 0));
        int frameWidth = frameLineWidth();
        p.drawRoundedRect(rect().adjusted(frameWidth, frameWidth, -frameWidth, -frameWidth),
                          frameRoundness(), frameRoundness());

        // paint the window frame
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        drawFrame(&p);

    } else {
        // draw the pixmap instead
        FramedContactWidget::paintEvent(e);
    }
}


class SmallVideoWidget : public VideoWidget
{
    Q_OBJECT
public:
    SmallVideoWidget(QWidget *parent = 0) : VideoWidget(parent) {
        m_portraitWidget = new TransparentFramedImageWidget;
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setMargin(0);
        layout->setSpacing(0);
        layout->setAlignment(Qt::AlignHCenter);
        layout->addWidget(m_portraitWidget);
    }
    ~SmallVideoWidget() {}

    QRect globalVideoRect() const {
        QPoint pos = mapToGlobal(QPoint(0,0));
        int frameWidth = m_portraitWidget->frameLineWidth();
        return QRect(pos.x() + frameWidth,
                     pos.y() + frameWidth,
                     width() - frameWidth*2,
                     height() - frameWidth*2);
    }

    void setTransparent(bool value) {
        m_portraitWidget->setTransparent(value);
    }

    bool isTransparent() const { return m_portraitWidget->isTransparent(); }

    void setContact(const QContact &contact) { m_portraitWidget->setContact(contact); }

private:
    TransparentFramedImageWidget *m_portraitWidget;
};


class LargeVideoWidget : public VideoWidget
{
    Q_OBJECT
public:
    LargeVideoWidget(QWidget *parent = 0) : VideoWidget(parent), transparent(false) {
        m_framedPixmapWdgt = new FramedContactWidget;
        QBoxLayout *layout = new QHBoxLayout;
        layout->setContentsMargins(10, 15, 10, 15);
        layout->setSpacing(0);
        layout->setAlignment(Qt::AlignHCenter);
        layout->addWidget(m_framedPixmapWdgt);
        setLayout(layout);

        m_framedPixmapWdgt->hide();     // wait until a pixmap is set

        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setTransparent(bool value) {
        if (transparent != value) {
            transparent = value;
            m_framedPixmapWdgt->setVisible(!transparent);
            update();
        }
    }

    bool isTransparent() const { return transparent; }

    void setContact(const QContact &contact) {
        m_framedPixmapWdgt->setContact(contact);
        if (!m_framedPixmapWdgt->isVisible() && !transparent)
            m_framedPixmapWdgt->show();
    }

    QRect globalVideoRect() const {
        QPoint pos = mapToGlobal(QPoint(0,0));
        return QRect(pos, size());
    }

    bool transparent;

protected:
    void paintEvent(QPaintEvent *);

private:
    FramedContactWidget *m_framedPixmapWdgt;
};

void LargeVideoWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if (transparent) {
        p.fillRect(rect(), parentWidget()->palette().brush(QPalette::Window));

        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.setBrush(QColor(0, 0, 0, 0));
        p.drawRect(rect());

        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.setBrush(QBrush());
        p.setRenderHint(QPainter::Antialiasing);

        p.drawRect(rect());

    } else {
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        if (width() > height()) {   // don't draw if widget has shrunk (e.g. multi-party calls)
            // draw two intersecting rects in bottom-right corner
            QPen pen = p.pen();
            pen.setBrush((callscreen_LIGHTGRAY).darker(150));
            pen.setWidth(2);
            p.setPen(pen);
            QRect largeRect(0, 0, 28, 20);
            largeRect.moveBottomRight(QPoint(width() - 10, height() - 15));
            p.drawRect(largeRect);
            QRect smallRect(0, 0, 22, 15);
            smallRect.moveCenter(largeRect.bottomLeft());
            p.drawRect(smallRect);
        }
    }
}

//===========================================================================

class LargeCallerView : public QFrame
{
    Q_OBJECT
public:
    LargeCallerView(QWidget *parent = 0);

    void callStateChanged();
    void setCall(DeskphoneCallData *callData);
    inline bool hasCall() const { return (callData != 0); }
    inline DeskphoneCallData *deskphoneCallData() { return callData; }
    inline VideoWidget *videoWidget() { return video; }

protected:
    void resizeEvent(QResizeEvent *event);
    void updateFrameColor(QFrame *frame) const;
    void changeEvent(QEvent *event);

private:
    void makeDisabledColor(QColor *color) const;
    void updateBackgrounds();
    void updateText();

    VideoWidget *video;
    QGradientStops videoGradStops;
    QLabel *nameLabel;
    QGradientStops nameLabelGradStops;
    QFrame *horizLine;
    DeskphoneCallData *callData;
};

LargeCallerView::LargeCallerView(QWidget *parent) : QFrame(parent), callData(0)
{
    nameLabel = new QLabel;
    nameLabel->setFont(callscreen_FONT);
    nameLabel->setContentsMargins(3, 3, 3, 3);
    nameLabel->setAutoFillBackground(true);

    nameLabelGradStops << QGradientStop(0.0f, callscreen_LIGHTGRAY)
            << QGradientStop(1.0f, callscreen_LIGHTGRAY.darker(130));

    video = new LargeVideoWidget;
    video->setAutoFillBackground(true);

    videoGradStops << QGradientStop(0.0f, callscreen_BLUE)
              << QGradientStop(0.6f, callscreen_LIGHTGRAY);

    horizLine = new QFrame;
    horizLine->setFrameStyle(QFrame::HLine | QFrame::Plain);
    updateFrameColor(horizLine);
    horizLine->setLineWidth(0);
    horizLine->setMidLineWidth(0);

    QBoxLayout *vb = new QVBoxLayout;
    vb->setMargin(0);
    vb->setSpacing(0);
    vb->addWidget(video);
    vb->addWidget(horizLine);
    vb->addWidget(nameLabel);
    setLayout(vb);

    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(1);
    updateFrameColor(this);
}

void LargeCallerView::callStateChanged()
{
    if (callData)
        setEnabled(callData->callState != QPhoneCall::Hold);
    else
        setEnabled(true);
    updateText();
}

void LargeCallerView::setCall(DeskphoneCallData *data)
{
    callData = data;

    if (callData)
        video->setContact(callData->contact);
    else
        video->setContact(QContact());

    callStateChanged();
}

void LargeCallerView::updateText()
{
    if (callData) {
        QString text = callData->displayName();
        if (!callData->numberType.isEmpty())
            text += " [" + callData->numberType + "]";
        switch (callData->callState) {
            case QPhoneCall::Dialing:
            case QPhoneCall::Alerting:
                text = tr("Calling: %1").arg(text);
                break;
            default:
                break;
        }
        nameLabel->setText(nameLabel->fontMetrics().elidedText(text, Qt::ElideRight, nameLabel->width()));
    } else {
        nameLabel->clear();
    }
}

void LargeCallerView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    updateBackgrounds();
    updateText();
}

void LargeCallerView::updateFrameColor(QFrame *frame) const
{
    QPalette p = frame->palette();
    if (isEnabled()) {
        p.setColor(QPalette::WindowText, callscreen_BLUE);
    } else {
        QColor c = callscreen_BLUE;
        makeDisabledColor(&c);
        p.setColor(QPalette::WindowText, c);
    }
    frame->setPalette(p);
}

void LargeCallerView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::EnabledChange) {
        video->setEnabled(isEnabled());
        updateBackgrounds();
        updateFrameColor(horizLine);
        updateFrameColor(this);
    }
}

void LargeCallerView::makeDisabledColor(QColor *color) const
{
    *color = color->darker(150);
}

void LargeCallerView::updateBackgrounds()
{
    if (isEnabled()) {
        callscreen_applyLinearGradient(video, videoGradStops);
        callscreen_applyLinearGradient(nameLabel, nameLabelGradStops);
    } else {
        QGradientStops gradStops = videoGradStops;
        for (int i=0; i<gradStops.size(); i++)
            makeDisabledColor(&gradStops[i].second);
        callscreen_applyLinearGradient(video, gradStops);

        gradStops = nameLabelGradStops;
        for (int i=0; i<gradStops.size(); i++)
            makeDisabledColor(&gradStops[i].second);
        callscreen_applyLinearGradient(nameLabel, gradStops);
    }
}

//===========================================================================

enum DeskphoneConferenceAction
{
    NoConferenceAction,
    ActionJoin,
    ActionSplit
};

enum DeskphoneHoldAction
{
    NoHoldAction,
    ActionHold,
    ActionResume,
    ActionSwap
};

class DeskphoneCallScreenPrivate
{
public:
    DeskphoneCallScreenPrivate();

    DeskphoneCallData *findCall(const QPhoneCall &call) const
    {
        for (int i = 0; i < callItems.count(); ++i) {
            if (callItems.at(i)->call == call)
                return callItems.at(i);
        }

        return 0;
    }

    void addToolButtonsToLayout(QLayout *layout) const {
        layout->addWidget(videoButton);
        layout->addWidget(addCallButton);
        layout->addWidget(speakerButton);
        layout->addWidget(muteButton);
        layout->addWidget(holdButton);
        layout->addWidget(endCallsButton);
    }

    LargeCallerView *findLargeCallerView(DeskphoneCallData *callData) {
        for (int i=0; i<largeCallerViews.size(); i++) {
            DeskphoneCallData *dcd = largeCallerViews[i]->deskphoneCallData();
            if (dcd) {
                if ( (dcd->call.identifier() == callData->call.identifier())
                     || (dcd->call.fullNumber() == callData->call.fullNumber() &&
                            !dcd->call.fullNumber().isEmpty()) ) {
                    return largeCallerViews[i];
                }
            }
        }
        return 0;
    }

    void updateHoldButtonText(DeskphoneHoldAction holdAction) {
        switch (holdAction) {
            case ActionResume:
                holdButton->setText(QObject::tr("Resume", "Take a phone call off hold"));
                break;
            case ActionSwap:
                holdButton->setText(QObject::tr("Swap", "Switch from active call to the held call"));
                break;
            default:
                holdButton->setText(QObject::tr("Hold", "Hold a phone call"));
        }
    }

    static DeskphoneHoldAction matchHoldAction(bool hasActive, bool hasHeld) {
        if (hasActive) {
            if (hasHeld)
                return ActionSwap;
            return ActionHold;
        } else {
            if (hasHeld)
                return ActionResume;
            return NoHoldAction;
        }
    }

    QValueSpaceItem *videoVsi;
    QValueSpaceItem *muteVsi;
    QValueSpaceObject *videoVso;
    QValueSpaceObject *callVso;
    QList<DeskphoneCallData*> callItems;
    QList<LargeCallerView*> largeCallerViews;
    QPointer<LargeCallerView> largeCallerViewWithVideo;
    bool incoming;
    bool dialing;
    int activeCount;
    int holdCount;
    DeskphoneCallData *primaryCall;
    QTimer *durationTimer;
    QPhoneProfileManager *profiles;
    VideoWidget *smallCallerView;
    DeskphoneConferenceAction conferenceAction;
    QAudioStateConfiguration *audioConfig;
    CallReviewDialog *m_callReviewDlg;
    IncomingCallDialog *m_incomingCallDlg;
    bool pressedAddCall;
    bool joinNextOutgoingCall;
    bool joinNextHeldCall;
    QList<QPhoneCall> endedMultiPartyCalls;

    HomeActionButton *videoButton;
    HomeActionButton *addCallButton;
    HomeActionButton *speakerButton;
    HomeActionButton *muteButton;
    HomeActionButton *holdButton;
    HomeActionButton *endCallsButton;
};

DeskphoneCallScreenPrivate::DeskphoneCallScreenPrivate()
{
    incoming = false;
    dialing = false;
    activeCount = 0;
    holdCount = 0;
    primaryCall = 0;
    durationTimer = 0;
    profiles = 0;
    conferenceAction = NoConferenceAction;
    audioConfig = 0;
}
/*!
    \class DeskphoneCallScreen
    \inpublicgroup QtTelephonyModule
    \brief The DeskphoneCallScreen class provides the call screen for Qt Extended Home.
    \ingroup QtopiaServer::PhoneUI

    An image of this call screen can be found in the \l{Server Widget Classes}{server widget gallery}

    This class is a Qt Extended \l{QtopiaServerApplication#qt-extended-server-widgets}{server widget}.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
    Creates a new DeskphoneCallScreen instance with the given \a parent and
    widget \a flags.
*/
DeskphoneCallScreen::DeskphoneCallScreen(QWidget *parent, Qt::WFlags flags)
    : QAbstractCallScreen(parent, flags),
      d(new DeskphoneCallScreenPrivate)
{
    setupUi(this);

    d->videoButton = new CheckableHomeActionButton(tr("Video\non"), tr("Video\noff"),
            QtopiaHome::standardColor(QtopiaHome::Green));
    d->addCallButton = new HomeActionButton(tr("Add call"), QtopiaHome::Green);
    d->speakerButton = new CheckableHomeActionButton(tr("Speaker"), tr("Speaker\noff"),
            QtopiaHome::standardColor(QtopiaHome::Green));
    d->muteButton = new CheckableHomeActionButton(tr("Mute"), tr("Unmute"),
            QtopiaHome::standardColor(QtopiaHome::Red));
    d->holdButton = new HomeActionButton(tr("Hold"), QtopiaHome::Red);
    d->endCallsButton = new HomeActionButton(tr("End\ncall"), QtopiaHome::Red);
    d->addToolButtonsToLayout(toolButtonsLayout);

    d->videoVsi = new QValueSpaceItem("/Communications/Calls/Video", this);
    connect(d->videoVsi, SIGNAL(contentsChanged()), this, SLOT(videoStateChanged()));
    d->videoVso = new QValueSpaceObject("/Communications/Calls/Video", this);
    d->callVso = new QValueSpaceObject("/Communications/Calls/Primary", this);
    connect(d->videoButton, SIGNAL(pressed()), this, SLOT(toggleVideo()));
    d->videoButton->setEnabled(false);

    connect(d->speakerButton, SIGNAL(clicked()), this, SLOT(toggleSpeaker()));
    connect(d->addCallButton, SIGNAL(clicked()), this, SLOT(addCall()));
    connect(d->muteButton, SIGNAL(pressed()), this, SLOT(toggleMute()));
    connect(d->holdButton, SIGNAL(clicked()), this, SLOT(toggleHold()));
    connect(d->endCallsButton, SIGNAL(pressed()), this, SLOT(endCalls()));

    d->durationTimer = new QTimer(this);
    connect(d->durationTimer, SIGNAL(timeout()), this, SLOT(updateDuration()));

    d->profiles = new QPhoneProfileManager(this);
    d->audioConfig = new QAudioStateConfiguration(this);
    connect(d->audioConfig,
            SIGNAL(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)),
            this, SLOT(audioStateChanged()));
    audioStateChanged();

    d->muteVsi = new QValueSpaceItem("/Volume/CurrentMuted", this);
    connect(d->muteVsi, SIGNAL(contentsChanged()), this, SLOT(muteChanged()));
    muteChanged();

    // Hide the conference button until conference operations are applicable.
    conferenceButton->setFont(d->videoButton->font());
    QPalette pal = conferenceButton->palette();
    pal.setColor(QPalette::Text, Qt::white);
    conferenceButton->setPalette(pal);
    conferenceButton->hide();
    connect(conferenceButton, SIGNAL(clicked()), this, SLOT(conference()));

    d->smallCallerView = new SmallVideoWidget(smallView);
    QVBoxLayout *vb = new QVBoxLayout(smallView);
    vb->setMargin(0);
    vb->setSpacing(0);
    vb->addWidget(d->smallCallerView);
    connect(d->smallCallerView, SIGNAL(geometryChanged()), this, SLOT(publishVideoGeometry()));
    connect(d->smallCallerView, SIGNAL(pressed()), this, SLOT(switchWindows()));

    QHBoxLayout *hb = new QHBoxLayout;
    hb->setContentsMargins(0, 3, 3, 3);
    hb->setSpacing(6);
    largeView->setLayout(hb);

    // add a default large view (looks better than blank space)
    addLargeCallerView(0);

    DialerControl *dc = DialerControl::instance();
    connect(dc, SIGNAL(callDropped(QPhoneCall)),
            SLOT(callDropped(QPhoneCall)));
    connect(dc, SIGNAL(callIncoming(QPhoneCall)),
            SLOT(callIncoming(QPhoneCall)));
    connect(dc, SIGNAL(callConnected(QPhoneCall)),
            SLOT(callConnected(QPhoneCall)));
    connect(dc, SIGNAL(callPutOnHold(QPhoneCall)),
            SLOT(callPutOnHold(QPhoneCall)));

    d->pressedAddCall = false;
    d->joinNextOutgoingCall = false;
    d->largeCallerViewWithVideo = 0;
    d->joinNextHeldCall = false;

    d->m_callReviewDlg = new CallReviewDialog(this);
    d->m_incomingCallDlg = new IncomingCallDialog(this);
    connect(d->m_incomingCallDlg, SIGNAL(finished(int)), SLOT(finishedIncomingCallDialog(int)));

    setAutoFillBackground(true);

    QTimer::singleShot(100, this, SLOT(checkForIncomingCall()));
}

/*!
    \internal
*/
DeskphoneCallScreen::~DeskphoneCallScreen()
{
    delete d;
}

/*!
    \reimp
*/
void DeskphoneCallScreen::stateChanged()
{
    const QList<QPhoneCall> &calls = DialerControl::instance()->allCalls();

    //  see if any calls have ended.
    for (int i = d->callItems.count()-1; i>=0; i--) {
        DeskphoneCallData *callData = d->callItems.at(i);
        if (!calls.contains(callData->call)) {
            if (callData->disconnectTime.isNull())
                callData->disconnectTime = QDateTime::currentDateTime();
        }
    }

    d->incoming = false;
    d->dialing = false;
    d->activeCount = 0;
    d->holdCount = 0;
    d->primaryCall = 0;
    int primaryOrder = 9;

    // any calls added/changed state
    QList<QPhoneCall>::ConstIterator it;
    for (it = calls.begin(); it != calls.end(); ++it) {
        int sortOrder = 9;
        const QPhoneCall &call(*it);
        DeskphoneCallData *callData = d->findCall(call);
        if (!callData) {
            callData = new DeskphoneCallData(call);
            d->callItems.append(callData);
        }

        LargeCallerView *callerView = d->findLargeCallerView(callData);
        QPhoneCall::State prevState = callData->callState;

        if (callData->connectTime.isNull() && call.established())
            callData->connectTime = QDateTime::currentDateTime();

        if (call.state() != callData->callState) {
            callData->callState = call.state();
            if (callerView)
                callerView->callStateChanged();
        }

        if (call.state() == QPhoneCall::Connected) {
            sortOrder = 2;
            d->activeCount++;

            if (prevState != call.state()) {
                if (callData->wasIncoming)
                    addLargeCallerView(callData);
            }

        } else if (call.state() == QPhoneCall::Hold) {
            sortOrder = 3;
            d->holdCount++;

        } else if (call.state() == QPhoneCall::Dialing ||
                call.state() == QPhoneCall::Alerting) {
            d->dialing = true;
            sortOrder = 1;
            callData->callState = QPhoneCall::Dialing;
            callData->dialTime = QDateTime::currentDateTime();

            if (prevState != call.state() && call.state() == QPhoneCall::Dialing)
                addLargeCallerView(callData);

        } else if (call.state() == QPhoneCall::Incoming) {
            d->incoming = true;
            sortOrder = 0;
            callData->callState = QPhoneCall::Incoming;
            callData->wasIncoming = true;
        }

        if (sortOrder < primaryOrder) {
            d->primaryCall = callData;
            primaryOrder = sortOrder;
        }
    }

    if (d->incoming)
        QSoftMenuBar::setLabel(this, Qt::Key_Context2, ":icon/phone/answer", tr("Answer"));
    else
        QSoftMenuBar::setLabel(this, Qt::Key_Context2, QSoftMenuBar::NoLabel);

    DialerControl *control = DialerControl::instance();
    if (control->isConnected() || control->isDialing() || control->hasIncomingCall()) {
        d->endCallsButton->setEnabled(true);
        if (d->activeCount > 1) {
            d->endCallsButton->setText(tr("End all\ncalls"));
        } else {
            d->endCallsButton->setText(tr("End\ncall"));
        }
    } else {
        d->endCallsButton->setEnabled(false);
    }

    // Determine the label for the Hold/Resume/Swap button.
    d->updateHoldButtonText(d->matchHoldAction(d->activeCount > 0, d->holdCount > 0));

    if (d->primaryCall) {
        d->durationTimer->start(1000);
        d->callVso->setAttribute("Identifier", d->primaryCall->displayName());

        // Determine the state and label of the conference button.
        if (d->primaryCall->callState == QPhoneCall::Connected) {
            if (d->holdCount > 0) {
                // Calls active and on hold: button should be "Merge".
                conferenceButton->setText(tr("Merge"));
                QPalette pal = conferenceButton->palette();
                pal.setBrush(QPalette::Base, QtopiaHome::standardColor(QtopiaHome::Green));
                conferenceButton->setPalette(pal);
                conferenceButton->show();
                d->conferenceAction = ActionJoin;
            } else if (d->activeCount > 1) {
                // Many calls active on a conference: button should be "Split".
                conferenceButton->setText(tr("Split"));
                QPalette pal = conferenceButton->palette();
                pal.setBrush(QPalette::Base, QtopiaHome::standardColor(QtopiaHome::Red));
                conferenceButton->setPalette(pal);
                conferenceButton->show();
                d->conferenceAction = ActionSplit;
            } else {
                // Single active call: no conference actions possible.
                d->conferenceAction = NoConferenceAction;
                conferenceButton->hide();
            }
        } else {
            d->conferenceAction = NoConferenceAction;
            conferenceButton->hide();
        }

    } else {
        d->callVso->removeAttribute("Identifier");
        d->callVso->removeAttribute("Duration");
        conferenceButton->hide();
    }

    AbstractAudioHandler *callAudio = AbstractAudioHandler::audioHandler("CallAudio");
    bool nonActiveDialing = d->dialing && !d->activeCount;
    if (callAudio)
        callAudio->activateAudio(d->activeCount || d->holdCount || nonActiveDialing /* || d->incoming*/ );

    // Do we need to turn video on automatically once the call is connected?
    if (d->primaryCall && d->primaryCall->callState == QPhoneCall::Connected &&
        d->primaryCall->needsAutoVideoCheck && d->profiles) {

        d->primaryCall->needsAutoVideoCheck = false;
        switch (d->profiles->activeProfile().videoOption()) {

            case QPhoneProfile::AlwaysOff: break;

            case QPhoneProfile::OnForIncoming:
            {
                if (d->primaryCall->wasIncoming)
                    toggleVideo();
            }
            break;

            case QPhoneProfile::OnForOutgoing:
            {
                if (!(d->primaryCall->wasIncoming))
                    toggleVideo();
            }
            break;

            case QPhoneProfile::AlwaysOn:
            {
                toggleVideo();
            }
            break;
        }
    }
}

void DeskphoneCallScreen::callDropped(const QPhoneCall &call)
{
    // don't show dialog if call was never connected
    if (!call.connectTime().isNull()) {
        bool multiPartyCall = false;
        int index = d->endedMultiPartyCalls.indexOf(call);
        if (index >= 0) {
            d->endedMultiPartyCalls.removeAt(index);
            multiPartyCall = true;
        }
        d->m_callReviewDlg->setPhoneCall(call, multiPartyCall);
        QtopiaApplication::showDialog(d->m_callReviewDlg);
    }

    DeskphoneCallData *callData = d->findCall(call);
    if (callData)
        removeLargeCallerView(callData);

    // resume any held calls
    if (DialerControl::instance()->hasCallsOnHold()) {
        QList<QPhoneCall> heldCalls = DialerControl::instance()->callsOnHold();
        if (heldCalls.size() > 0)
            heldCalls[0].activate();
    }
}

void DeskphoneCallScreen::callIncoming(const QPhoneCall &call)
{
    d->m_incomingCallDlg->setPhoneCall(call);
    QtopiaApplication::showDialog(d->m_incomingCallDlg);
}

void DeskphoneCallScreen::callConnected(const QPhoneCall &call)
{
    if (d->joinNextOutgoingCall) {
        DeskphoneCallData *callData = d->findCall(call);
        if (callData && !callData->wasIncoming) {
            join();
            d->joinNextOutgoingCall = false;
        }
    }
}

void DeskphoneCallScreen::callPutOnHold(const QPhoneCall &)
{
    if (d->joinNextHeldCall) {
        QList<QPhoneCall> heldCalls = DialerControl::instance()->callsOnHold();
        if (heldCalls.size() > 0)
            heldCalls[0].join();
    }
    d->joinNextHeldCall = false;
}

void DeskphoneCallScreen::finishedIncomingCallDialog(int result)
{
    IncomingCallDialog *dlg = qobject_cast<IncomingCallDialog *>(sender());
    if (!dlg)
        return;

    if (result == QDialog::Accepted) {
        switch (dlg->answerMode()) {
            case IncomingCallDialog::EndAndAnswer:
            {
                // don't call endCalls() because emitting both hangupCall() and
                // acceptIncoming() causes speaker to toggle on then off again
                DialerControl::instance()->endCall();
                acceptCall();

                break;
            }
            case IncomingCallDialog::MergeCalls:
            {
                // join won't work straight away, have to hold the call first,
                // then join it
                d->joinNextHeldCall = true;
                acceptCall();
                break;
            }
            default:    // None or HoldAndAnswer
            {
                acceptCall();
            }
        }
    } else {
        if (DialerControl::instance()->hasActiveCalls()) {
            // can't use endCalls() because hangupCall() ends the active
            // call, not the incoming call
            DialerControl::instance()->incomingCall().hangup();

        } else {
            endCalls();
        }
    }
}

void DeskphoneCallScreen::checkForIncomingCall()
{
    QPhoneCall incoming = DialerControl::instance()->incomingCall();
    if (!incoming.isNull())
        callIncoming(incoming);
}

/*!
    \reimp
*/
void DeskphoneCallScreen::changeEvent(QEvent *event)
{
    QAbstractCallScreen::changeEvent(event);

    // Guard against cases where user pressed Add Call, cancelled the dialer,
    // then tried to start a separate call without using Add Call.
    if (event->type() == QEvent::ActivationChange) {
        if (!isActiveWindow()) {
            if (d->pressedAddCall)      // moving to dialer window
                d->joinNextOutgoingCall = true;
            else                        // moving to some other window
                d->joinNextOutgoingCall = false;
        }

        d->pressedAddCall = false;
    }
}

/*!
    \reimp
*/
void DeskphoneCallScreen::showEvent(QShowEvent *e)
{
    QAbstractCallScreen::showEvent(e);
    publishVideoGeometry();
}

/*!
    \reimp
*/
void DeskphoneCallScreen::hideEvent(QHideEvent *)
{
    d->durationTimer->stop();
    d->videoVso->removeAttribute("DecoderWindow");
    d->videoVso->removeAttribute("EncoderWindow");
}

/*!
    \reimp
*/
void DeskphoneCallScreen::moveEvent(QMoveEvent *e)
{
    QAbstractCallScreen::moveEvent(e);
    if (isVisible())
        publishVideoGeometry();
}

/*!
    \reimp
*/
void DeskphoneCallScreen::resizeEvent(QResizeEvent *e)
{
    QAbstractCallScreen::resizeEvent(e);
    if (isVisible())
        publishVideoGeometry();

    QGradientStops gradStops;
    gradStops << QGradientStop(0.0f, callscreen_LIGHTGRAY.darker(120))
              << QGradientStop(0.5f, callscreen_LIGHTGRAY)
              << QGradientStop(0.0f, callscreen_LIGHTGRAY.darker(120))
              << QGradientStop(1.0f, callscreen_LIGHTGRAY.darker(170));
    callscreen_applyLinearGradient(this, gradStops);

    updateCallerViewSizes();
}

/*!
    \reimp
*/
void DeskphoneCallScreen::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {

        case Qt::Key_Context1:
        {
            if (!e->isAutoRepeat())
                endCalls();
        }
        break;

        case Qt::Key_Context2:
        {
            if (!e->isAutoRepeat())
                acceptCall();
        }
        break;

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
        case Qt::Key_Asterisk:
        case Qt::Key_NumberSign:
        {
            // Generate DTMF digits during a call.
            if (DialerControl::instance()->hasActiveCalls())
                DialerControl::instance()->activeCalls().first().tone(e->text());
        }
        break;
    }
    QAbstractCallScreen::keyPressEvent(e);
}

void DeskphoneCallScreen::updateDuration()
{
    QMutableListIterator<DeskphoneCallData*> i(d->callItems);
    while (i.hasNext()) {
        DeskphoneCallData *callData = i.next();
        if (callData->call.dropped()) {
            // Remove dropped calls after a short delay
            if (!callData->disconnectTime.isNull() &&
                callData->disconnectTime.time().elapsed() > 100) {
                if (d->primaryCall == callData) {
                    d->callVso->removeAttribute("Identifier");
                    d->callVso->removeAttribute("Duration");
                    d->primaryCall = 0;
                }

                i.remove();
                delete callData;
            }
        }
    }

    if (d->primaryCall)
        d->callVso->setAttribute("Duration", d->primaryCall->durationString(false));

    if (d->callItems.isEmpty())
        hide();
}

void DeskphoneCallScreen::videoStateChanged()
{
    bool videoPossible = d->videoVsi->value("VideoPossible", false).toBool();
    bool decoderWindowActive = d->videoVsi->value("DecoderWindowActive", false).toBool();
    bool encoderWindowActive = d->videoVsi->value("EncoderWindowActive", false).toBool();
    bool privacyEnabled = d->videoVsi->value("PrivacyEnabled", false).toBool();
    bool windowsSwapped = d->videoVsi->value("WindowsSwapped", false).toBool();
    bool windowsZoomed = d->videoVsi->value("WindowsZoomed", false).toBool();

    if (encoderWindowActive && d->primaryCall)
        d->primaryCall->needsAutoVideoCheck = false;

    d->videoButton->setEnabled(videoPossible);
    d->videoButton->setChecked(encoderWindowActive && !privacyEnabled);

    bool smallViewTransparent = false;
    bool largeViewTransparent = false;
    QContactModel *contactModel = ServerContactModel::instance();

    VideoWidget *largeViewVideo = 0;
    QContact otherPartyContact;
    if (d->largeCallerViewWithVideo) {
        largeViewVideo = d->largeCallerViewWithVideo->videoWidget();
        if (d->largeCallerViewWithVideo->deskphoneCallData())
            otherPartyContact = d->largeCallerViewWithVideo->deskphoneCallData()->contact;
    }

    if (windowsSwapped) {
        if (decoderWindowActive)
            smallViewTransparent = true;
        if (encoderWindowActive && !privacyEnabled)
            largeViewTransparent = true;
        if (largeViewVideo)
            largeViewVideo->setContact(contactModel->personalDetails());
        d->smallCallerView->setContact(otherPartyContact);
    } else {
        if (encoderWindowActive && !privacyEnabled)
            smallViewTransparent = true;
        if (decoderWindowActive)
            largeViewTransparent = true;
        if (largeViewVideo)
            largeViewVideo->setContact(otherPartyContact);
        d->smallCallerView->setContact(contactModel->personalDetails());
    }

    d->smallCallerView->setTransparent(smallViewTransparent);
    if (largeViewVideo)
        largeViewVideo->setTransparent(largeViewTransparent);

    if (largeViewVideo) {
        if (windowsZoomed)
            largeViewVideo->grabMouse();
        else
            largeViewVideo->releaseMouse();
    }
}

void DeskphoneCallScreen::endCalls()
{
    DialerControl *dc = DialerControl::instance();

    // must emit hangupCall() to notify dial proxy, etc.
    if (dc->isConnected() || dc->isDialing() || dc->hasIncomingCall()) {
        if (d->activeCount > 1)
            d->endedMultiPartyCalls = dc->activeCalls();
        else
            d->endedMultiPartyCalls.clear();
        emit hangupCall();
    }
}

void DeskphoneCallScreen::acceptCall()
{
    if (DialerControl::instance()->hasIncomingCall())
        emit acceptIncoming();
}

void DeskphoneCallScreen::addCall()
{
    d->pressedAddCall = true;

    QtopiaServiceRequest req("Dialer", "showDialer()");
    req.send();
}

void DeskphoneCallScreen::toggleVideo()
{
    if (d->videoVsi->value("VideoPossible", false).toBool()) {
        QtopiaServiceRequest req("VideoCalling", "toggleVideo()");
        req.send();
    }
}

void DeskphoneCallScreen::toggleSpeaker()
{
    QtopiaServiceRequest req("Dialer", "speaker()");
    req.send();
}

void DeskphoneCallScreen::toggleMute()
{
    QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "toggleMuted()");
}

void DeskphoneCallScreen::muteChanged()
{
    bool muted = d->muteVsi->value("", false).toBool();
    d->muteButton->setChecked(muted);
}

void DeskphoneCallScreen::audioStateChanged()
{
    QByteArray profileName = d->audioConfig->currentState().profile();
    bool speakerActive = profileName.toLower().contains("speaker");
    d->speakerButton->setChecked(speakerActive);
}

void DeskphoneCallScreen::switchWindows()
{
    if (d->videoVsi->value("VideoPossible", false).toBool()) {
        QtopiaServiceRequest req("VideoCalling", "switchWindows()");
        req.send();
    }
}

void DeskphoneCallScreen::toggleZoom()
{
    if (d->videoVsi->value("VideoPossible", false).toBool()) {
        QtopiaServiceRequest req("VideoCalling", "toggleZoom()");
        req.send();
    }
}

void DeskphoneCallScreen::conference()
{
    switch (d->conferenceAction) {
        case NoConferenceAction:        break;
        case ActionJoin:        join(); break;
        case ActionSplit:       split(); break;
    }
}

void DeskphoneCallScreen::hold()
{
    DialerControl *dc = DialerControl::instance();
    if (dc->hasActiveCalls() && !dc->hasCallsOnHold()) {
        // TODO: what if there are multiple joined active calls?
        QList<QPhoneCall> calls = dc->activeCalls();
        if (calls.size() > 0)
            calls[0].hold();

        // Update the state of the hold button now,
        // to give more immediate feedback than waiting for
        // the telephony system to report a state change.
        d->updateHoldButtonText(ActionResume);
    }
}

void DeskphoneCallScreen::unhold()
{
    DialerControl *dc = DialerControl::instance();
    if (!dc->hasActiveCalls() && dc->hasCallsOnHold()) {
        QList<QPhoneCall> calls = dc->callsOnHold();
        if (calls.size() > 0) {
            // Activate the entire call group.
            calls[0].activate(QPhoneCall::Group);
        }

        // Update the state of the hold button now,
        // to give more immediate feedback than waiting for
        // the telephony system to report a state change.
        d->updateHoldButtonText(ActionHold);
    }
}

void DeskphoneCallScreen::swap()
{
    DialerControl *dc = DialerControl::instance();
    if (dc->hasActiveCalls() && dc->hasCallsOnHold()) {
        QList<QPhoneCall> calls = dc->callsOnHold();
        if (calls.size() > 0) {
            // Activate the entire held group, which will put the currently
            // active calls on hold as a side-effect.
            calls[0].activate(QPhoneCall::Group);
        }
    }
}

void DeskphoneCallScreen::join()
{
    DialerControl *dc = DialerControl::instance();
    if (dc->hasActiveCalls() && dc->hasCallsOnHold()) {
        QList<QPhoneCall> calls = dc->callsOnHold();
        if (calls.size() > 0) {
            // Perform a join which will convert all active and held calls
            // into a single active group.
            calls[0].join();
        }
    }
}

void DeskphoneCallScreen::split()
{
    DialerControl *dc = DialerControl::instance();
    if (dc->hasActiveCalls()) {
        QList<QPhoneCall> calls = dc->activeCalls();
        if (calls.size() > 0) {
            calls[0].hold();
            calls[0].activate(QPhoneCall::CallOnly);
        }
    }
}

// Used for the physical hardware Hold button.
void DeskphoneCallScreen::toggleHold()
{
    DialerControl *dc = DialerControl::instance();
    DeskphoneHoldAction holdAction = d->matchHoldAction(dc->hasActiveCalls(), dc->hasCallsOnHold());
    switch (holdAction) {
        case ActionHold:
            hold();
            break;
        case ActionResume:
            unhold();
            break;
        case ActionSwap:
            swap();
            break;
        default:
            break;
    }
}

void DeskphoneCallScreen::publishVideoGeometry()
{
    static const QString rectStr("%1,%2,%3,%4");

    QRect rect = d->smallCallerView->globalVideoRect();
    d->videoVso->setAttribute("EncoderWindow",
            rectStr.arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height()));

    if (d->largeCallerViewWithVideo) {
        rect = d->largeCallerViewWithVideo->videoWidget()->globalVideoRect();
        d->videoVso->setAttribute("DecoderWindow",
                rectStr.arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height()));
    }
}

/*!
    Returns a string describing the duration of a call (e.g. "1:23") using
    the given \a elapsed seconds. If \a showSeconds is true, the seconds
    will be shown in the string (e.g. "1:23:30").
*/
QString DeskphoneCallScreen::callDurationString(int elapsed, bool showSeconds)
{
    QString duration;
    int hour = elapsed/callscreen_SECS_PER_HOUR;
    int minute = (elapsed % callscreen_SECS_PER_HOUR)/callscreen_SECS_PER_MIN;
    int second = elapsed % callscreen_SECS_PER_MIN;
    QString buf;
    if (!hour)
        buf.sprintf( "%.2d:%.2d", minute, second );
    else if (showSeconds)
        buf.sprintf( "%.2d:%.2d:%.2d", hour, minute, second );
    else
        buf.sprintf( "%.2d:%.2d", hour, minute );
    duration = buf;
    return duration;
}

void DeskphoneCallScreen::addLargeCallerView(DeskphoneCallData *callData)
{
    if (d->largeCallerViews.size() == 1 && !d->largeCallerViews[0]->hasCall()) {
        d->largeCallerViews[0]->setCall(callData);

    } else {
        if (!d->findLargeCallerView(callData)) {
            LargeCallerView *newLargeView = new LargeCallerView;
            newLargeView->setCall(callData);
            if (largeView->layout())
                largeView->layout()->addWidget(newLargeView);
            d->largeCallerViews.append(newLargeView);
            updateCallerViewSizes();
        }
    }

    if (d->largeCallerViews.size() == 1)
        setVideoEnabledLargeCallerView(d->largeCallerViews[0]);
}

void DeskphoneCallScreen::removeLargeCallerView(DeskphoneCallData *callData)
{
    int viewIndex = -1;
    for (int i=0; i<d->largeCallerViews.size(); i++) {
        if (d->largeCallerViews[i]->deskphoneCallData() == callData) {
            viewIndex = i;
            break;
        }
    }
    if (viewIndex == -1)
        return;

    if (d->largeCallerViews[viewIndex] == d->largeCallerViewWithVideo)
        setVideoEnabledLargeCallerView(0);

    // don't remove the view if it's the only one left
    if (d->largeCallerViews.size() == 1) {
        d->largeCallerViews[0]->setCall(0);
    } else {
        delete d->largeCallerViews.takeAt(viewIndex);
        updateCallerViewSizes();
    }
}

void DeskphoneCallScreen::updateCallerViewSizes()
{
    // We must set fixed sizes for the large caller displays, or else
    // they will resize themselves when switching between image/video modes.

    int displaysCount = d->largeCallerViews.count();
    if (displaysCount == 0 || !largeView->layout())
        return;

    int spacing = largeView->layout()->spacing();
    if (displaysCount > 1)
        spacing = spacing * (displaysCount - 1);
    int displayWidth = (largeView->width() - spacing) / displaysCount;

    for (int i=0; i<d->largeCallerViews.size(); i++)
        d->largeCallerViews[i]->setFixedWidth(displayWidth);
}

void DeskphoneCallScreen::setVideoEnabledLargeCallerView(LargeCallerView *view)
{
    if (d->largeCallerViewWithVideo == view)
        return;

    if (d->largeCallerViewWithVideo) {
        disconnect(d->largeCallerViewWithVideo->videoWidget(), SIGNAL(geometryChanged()),
                this, SLOT(publishVideoGeometry()));
        disconnect(d->largeCallerViewWithVideo->videoWidget(), SIGNAL(pressed()),
                this, SLOT(toggleZoom()));
        d->largeCallerViewWithVideo->videoWidget()->setTransparent(false);
    }

    d->largeCallerViewWithVideo = view;
    if (view) {
        // ensure signal is not connected twice
        disconnect(view->videoWidget(), SIGNAL(geometryChanged()),
                this, SLOT(publishVideoGeometry()));
        disconnect(view->videoWidget(), SIGNAL(pressed()),
                this, SLOT(toggleZoom()));

        connect(view->videoWidget(), SIGNAL(geometryChanged()),
                this, SLOT(publishVideoGeometry()));
        connect(view->videoWidget(), SIGNAL(pressed()),
                this, SLOT(toggleZoom()));
    }
}

QTOPIA_REPLACE_WIDGET(QAbstractCallScreen, DeskphoneCallScreen);

#include "deskcallscreen.moc"
