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

#include "deskcallscreendialogs.h"
#include "deskcallscreen.h"
#include "servercontactmodel.h"
#include "dialercontrol.h"

#include <private/homewidgets_p.h>
#include <QPhoneCallManager>
#include <QPhoneCall>
#include <QContact>
#include <QContactModel>
#include <QTimeString>
#include <QtopiaServiceRequest>

#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QStackedLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QPainter>
#include <QMouseEvent>
#include <QLabel>
#include <QHash>

static const QFont deskcallscreendialogs_FONT(QLatin1String("DejaVu Sans Condensed"), 8);
static const QFont deskcallscreendialogs_FONT_BOLD(QLatin1String("DejaVu Sans Condensed"), 8, QFont::Bold);

void deskcallscreendialogs_getContactFromCall(const QPhoneCall &call, QContact *contact)
{
    QContactModel *contactModel = ServerContactModel::instance();
    if (!call.contact().isNull())
        *contact = contactModel->contact(call.contact());
    else if (!call.number().isEmpty())
        *contact = contactModel->matchPhoneNumber(call.number());
}

//==================================================================


class CallReviewContactWidget : public FramedContactWidget
{
    Q_OBJECT
public:
    CallReviewContactWidget(QWidget *parent = 0);
    ~CallReviewContactWidget();

    void setContactDetails(const QContact &contact, const QString &fullNumber);

signals:
    void contactCreationRequested(const QString &fullNumber);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    QString m_fullNumber;
};

CallReviewContactWidget::CallReviewContactWidget(QWidget *parent)
    : FramedContactWidget(parent)
{
}

CallReviewContactWidget::~CallReviewContactWidget()
{
}

void CallReviewContactWidget::setContactDetails(const QContact &contact, const QString &fullNumber)
{
    m_fullNumber = fullNumber;
    setContact(contact);
}

void CallReviewContactWidget::paintEvent(QPaintEvent *event)
{
    FramedContactWidget::paintEvent(event);

    if (contact().uid().isNull() && !m_fullNumber.isEmpty()) {
        int frameWidth = frameLineWidth();
        QRect adjustedRect = rect().adjusted(frameWidth, frameWidth, -frameWidth, -frameWidth);

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(adjustedRect, QColor(0, 0, 0, 100));

        QPen pen = p.pen();
        pen.setColor(Qt::white);
        p.setPen(pen);
        p.setFont(deskcallscreendialogs_FONT_BOLD);
        p.drawText(adjustedRect, Qt::AlignCenter | Qt::TextWordWrap, tr("Add to Contacts"));

        drawFrame(&p);
    }
}

void CallReviewContactWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);

    if (contact().uid().isNull() && !m_fullNumber.isEmpty())
        emit contactCreationRequested(m_fullNumber);
}

//==================================================================


CallDialog::CallDialog(QWidget *parent)
    : QDialog(parent)
{
    QtopiaHome::setPopupDialogStyle(this);
}

CallDialog::~CallDialog()
{
}

QWidget *CallDialog::createTitleWidget(const QString &title) const
{
    QLabel *titleLabel = new QLabel(title);
    QFont titleFont = deskcallscreendialogs_FONT_BOLD;
    titleFont.setPointSize(16);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignHCenter);
    return titleLabel;
}

QLayout *CallDialog::createCallerIdLayout(QLabel *nameLabel, QLabel *numberLabel) const
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setSizeConstraint(QLayout::SetMaximumSize);

    nameLabel->setFont(deskcallscreendialogs_FONT_BOLD);
    layout->addWidget(nameLabel, 0, Qt::AlignLeft);
    numberLabel->setFont(deskcallscreendialogs_FONT_BOLD);
    layout->addWidget(numberLabel, 0, Qt::AlignRight);

    return layout;
}

void CallDialog::setupDisplay()
{
    if (layout())
        return;

    // place the main layout in the center and put spacer items around it so
    // that the dialog is only as small as it needs to be
    QGridLayout *grid = new QGridLayout;
    grid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 0, 1, 3);
    grid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 1, 0);
    grid->addLayout(createMainLayout(), 1, 1);
    grid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 1, 2);
    grid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 2, 0, 1, 3);
    setLayout(grid);
}

QString CallDialog::callerNameFromContact(const QContact &contact) const
{
    return (!contact.label().isEmpty() ? contact.label() : tr("Unknown"));
}

void CallDialog::setupAffirmativeButton(QAbstractButton *button, bool connectSlot)
{
    setupButton(button, QtopiaHome::standardColor(QtopiaHome::Green));
    if (connectSlot)
        connect(button, SIGNAL(clicked()), SLOT(accept()));
}

void CallDialog::setupNegativeButton(QAbstractButton *button, bool connectSlot)
{
    setupButton(button, QtopiaHome::standardColor(QtopiaHome::Red));
    if (connectSlot)
        connect(button, SIGNAL(clicked()), SLOT(reject()));
}

void CallDialog::setupButton(QWidget *w, const QColor &color) const
{
    // for push buttons, the Home edition style uses Base & Text colour roles
    // for background and foreground colours instead of Button & ButtonText
    QPalette p = w->palette();
    p.setColor(QPalette::Base, color);
    p.setColor(QPalette::Text, Qt::white);
    w->setPalette(p);

    w->setFont(deskcallscreendialogs_FONT_BOLD);
}

//==================================================================

class IncomingCallWidget : public QWidget
{
    Q_OBJECT
public:
    IncomingCallWidget() : QWidget(0), portrait(new FramedContactWidget)
    {
    }

    ~IncomingCallWidget()
    {
        delete portrait;
    }

    QPointer<FramedContactWidget> portrait;
};

struct IncomingCallDialogPrivate
{
    IncomingCallDialog::PhoneState state;
    IncomingCallDialog::AnswerMode answerMode;
    QStackedLayout *stackedLayout;
    QHash<IncomingCallDialog::PhoneState, IncomingCallWidget *> stateWidgets;
    QLabel *nameLabel;
    QLabel *numberLabel;
    QContact contact;
    QPhoneCall call;
};


IncomingCallDialog::IncomingCallDialog(QWidget *parent)
    : CallDialog(parent),
      d(new IncomingCallDialogPrivate)
{
    d->state = UnknownState;
    d->answerMode = NoAnswerMode;
    d->stackedLayout = new QStackedLayout;
    d->nameLabel = new QLabel;
    d->numberLabel = new QLabel;

    setupDisplay();

    connect(DialerControl::instance(), SIGNAL(callConnected(QPhoneCall)),
            SLOT(callConnected(QPhoneCall)));
    connect(DialerControl::instance(), SIGNAL(callDropped(QPhoneCall)),
            SLOT(callDropped(QPhoneCall)));
}

IncomingCallDialog::~IncomingCallDialog()
{
    delete d;
}

void IncomingCallDialog::setPhoneCall(const QPhoneCall &call)
{
    d->call = call;
    deskcallscreendialogs_getContactFromCall(call, &d->contact);
    d->nameLabel->setText(callerNameFromContact(d->contact));
    d->numberLabel->setText(d->call.fullNumber());

    updateState();
    if (d->stateWidgets.contains(d->state))
        d->stateWidgets[d->state]->portrait->setContact(d->contact);
}

IncomingCallDialog::AnswerMode IncomingCallDialog::answerMode() const
{
    return d->answerMode;
}

QLayout *IncomingCallDialog::createMainLayout()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(5);
    layout->setSizeConstraint(QLayout::SetMaximumSize);
    layout->addWidget(createTitleWidget(tr("Incoming call")));
    layout->addLayout(createCallerIdLayout(d->nameLabel, d->numberLabel));
    layout->addLayout(d->stackedLayout);
    return layout;
}

void IncomingCallDialog::callConnected(const QPhoneCall &call)
{
    if (!isVisible())
        return;

    if (!d->call.isNull() && call.identifier() == d->call.identifier()) {
        accept();
        return;
    }

    // if another call has been connected, check whether to change
    // the look of this dialog
    updateState();
}

void IncomingCallDialog::callDropped(const QPhoneCall &call)
{
    if (!isVisible())
        return;

    if (!d->call.isNull() && call.identifier() == d->call.identifier()) {
        reject();
        return;
    }

    // if another call has been dropped, check whether to change
    // the look of this dialog
    updateState();
}

void IncomingCallDialog::affirmativeButtonPressed(int id)
{
    d->answerMode = AnswerMode(id);
    accept();
}

void IncomingCallDialog::updateState()
{
    PhoneState state = UnknownState;
    if (DialerControl::instance()->hasActiveCalls())
        state = OtherCallsActive;
    else
        state = NoOtherCallsActive;
    phoneStateChanged(state);
}

void IncomingCallDialog::phoneStateChanged(PhoneState state)
{
    if (state == d->state)
        return;
    d->state = state;

    IncomingCallWidget *widget = d->stateWidgets.value(state, 0);
    if (!widget) {
        widget = new IncomingCallWidget;
        setupCallWidget(widget, state);
        d->stateWidgets[state] = widget;
        d->stackedLayout->addWidget(widget);
    }
    d->stackedLayout->setCurrentWidget(widget);
    adjustSize();
}

void IncomingCallDialog::setupCallWidget(IncomingCallWidget *callWidget, PhoneState state)
{
    if (state == IncomingCallDialog::NoOtherCallsActive) {
        QGridLayout *grid = new QGridLayout;
        grid->setHorizontalSpacing(20);
        grid->setVerticalSpacing(8);
        grid->setMargin(0);

        // ensure the layout will shrink if changing from OtherCallsActive
        // state to NoOtherCallsActive
        grid->setSizeConstraint(QLayout::SetMaximumSize);
        callWidget->portrait->setMinimumSize(65, 65);
        callWidget->portrait->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        grid->addWidget(callWidget->portrait, 0, 0, 2, 1);

        QPushButton *button;
        button = new QPushButton(tr("Answer", "Answer incoming call"));
        setupAffirmativeButton(button);
        grid->addWidget(button, 0, 1);

        button = new QPushButton(tr("Ignore", "Hang up incoming call"));
        setupNegativeButton(button);
        grid->addWidget(button, 1, 1);

        callWidget->setLayout(grid);

    } else if (state == IncomingCallDialog::OtherCallsActive) {
        QGridLayout *grid = new QGridLayout;
        grid->setHorizontalSpacing(10);
        grid->setVerticalSpacing(5);
        grid->setMargin(0);

        grid->addWidget(callWidget->portrait, 0, 0, 2, 1);

        QButtonGroup *buttons = new QButtonGroup(this);
        connect(buttons, SIGNAL(buttonClicked(int)),
                SLOT(affirmativeButtonPressed(int)));

        QAbstractButton *button;
        button = new QPushButton(tr("Hold and answer"));
        setupAffirmativeButton(button, false);
        buttons->addButton(button, HoldAndAnswer);
        grid->addWidget(button, 0, 1);

        button = new QPushButton(tr("End and answer"));
        setupAffirmativeButton(button, false);
        buttons->addButton(button, EndAndAnswer);
        grid->addWidget(button, 1, 1);

        button = new QPushButton(tr("Merge Calls"));
        setupAffirmativeButton(button, false);
        buttons->addButton(button, MergeCalls);
        grid->addWidget(button, 2, 1);

        button = new QPushButton(tr("Ignore", "Hang up incoming call"));
        setupNegativeButton(button);
        grid->addWidget(button, 2, 0);

        callWidget->setLayout(grid);
    }

    callWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    callWidget->setContentsMargins(0, 0, 0, 5);
}


//==================================================================

struct CallReviewDialogPrivate
{
    CallReviewContactWidget *portrait;
    QLabel *dateLabel;
    QLabel *timeLabel;
    QLabel *durationLabel;
    QLabel *nameLabel;
    QLabel *numberLabel;
    QContact contact;
    QPhoneCall call;
    bool multiPartyCall;
};


CallReviewDialog::CallReviewDialog( QWidget *parent)
    : CallDialog(parent),
      d(new CallReviewDialogPrivate)
{
    d->portrait = new CallReviewContactWidget;
    d->dateLabel = new QLabel;
    d->timeLabel = new QLabel;
    d->durationLabel = new QLabel;
    d->nameLabel = new QLabel;
    d->numberLabel = new QLabel;
    d->multiPartyCall = false;

    d->portrait->setMinimumSize(65, 65);
    connect(d->portrait, SIGNAL(contactCreationRequested(QString)),
            SLOT(contactCreationRequested(QString)));

    setupDisplay();
}

CallReviewDialog::~CallReviewDialog()
{
    delete d;
}

void CallReviewDialog::setPhoneCall(const QPhoneCall &call)
{
    d->call = call;

    if (d->multiPartyCall) {
        QFont f = d->nameLabel->font();
        f.setItalic(true);
        d->nameLabel->setFont(f);
        d->nameLabel->setText(tr("Multiparty call"));
        d->numberLabel->clear();
        d->portrait->setContactDetails(QContact(), QString());
    } else {
        QFont f = d->nameLabel->font();
        f.setItalic(false);
        d->nameLabel->setFont(f);
        deskcallscreendialogs_getContactFromCall(call, &d->contact);
        d->nameLabel->setText(callerNameFromContact(d->contact));
        d->numberLabel->setText(d->call.fullNumber());
        d->portrait->setContactDetails(d->contact, call.fullNumber());
    }

    QDateTime startTime = call.startTime();
    QDateTime connectTime = call.connectTime();
    QDateTime endTime = call.endTime();
    if (endTime.isNull())
        endTime = QDateTime::currentDateTime();
    int duration = 0;
    if (!connectTime.isNull())
        duration = connectTime.secsTo(endTime);

    d->dateLabel->setText(QTimeString::numberDateString(startTime.date()));
    d->timeLabel->setText(QTimeString::localHMS(startTime.time()));
    d->durationLabel->setText(DeskphoneCallScreen::callDurationString(
                   duration, false));
}

void CallReviewDialog::setPhoneCall(const QPhoneCall &call, bool isMultiPartyCall)
{
    d->multiPartyCall = isMultiPartyCall;
    setPhoneCall(call);
}

void CallReviewDialog::contactCreationRequested(const QString &fullNumber)
{
    accept();

    QtopiaServiceRequest req("Contacts", "createNewContact(QString)");
    req << fullNumber;
    req.send();
}

QLayout *CallReviewDialog::createMainLayout()
{
    QVBoxLayout *details = new QVBoxLayout;
    details->setSpacing(0);
    details->setMargin(0);
    details->addLayout(createDetailLayout(tr("Date:"), d->dateLabel));
    details->addLayout(createDetailLayout(tr("Time:"), d->timeLabel));
    details->addLayout(createDetailLayout(tr("Duration:"), d->durationLabel));
    details->addStretch();

    QPushButton *closeButton = new QPushButton(tr("Close"));
    setupNegativeButton(closeButton);

    QGridLayout *grid = new QGridLayout;
    grid->setMargin(0);
    grid->setHorizontalSpacing(20);
    grid->setVerticalSpacing(8);
    grid->addWidget(createTitleWidget(tr("Call Review")), 0, 0, 1, 2);
    grid->addLayout(createCallerIdLayout(d->nameLabel, d->numberLabel), 1, 0, 1, 2);
    grid->addWidget(d->portrait, 2, 0);
    grid->addLayout(details, 2, 1);
    grid->addWidget(closeButton, 3, 0, 1, 2, Qt::AlignHCenter);
    return grid;
}

QLayout *CallReviewDialog::createDetailLayout(const QString &label, QWidget *fieldWdgt) const
{
    QHBoxLayout *layout = new QHBoxLayout;
    QLabel *labelWdgt = new QLabel(label);
    labelWdgt->setFont(deskcallscreendialogs_FONT_BOLD);
    layout->addWidget(labelWdgt);
    fieldWdgt->setFont(deskcallscreendialogs_FONT);
    layout->addWidget(fieldWdgt);
    layout->setMargin(0);
    layout->setSpacing(labelWdgt->fontMetrics().width(' '));
    layout->setAlignment(Qt::AlignLeft);
    return layout;
}


#include "deskcallscreendialogs.moc"
