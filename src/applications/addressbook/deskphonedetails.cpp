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

#include "deskphonedetails.h"
#include "deskphonewidgets.h"

#include <QtopiaApplication>
#include <QContactModel>
#include <QFieldDefinition>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QAbstractButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLinearGradient>
#include <QStylePainter>
#include <QListView>
#include <QCollective>


#include "contactdetails.h"
#include "contactbrowser.h"
#include "contactoverview.h"
#if defined(QTOPIA_TELEPHONY)
#include "contactcallhistorylist.h"
#endif
#include "contactmessagehistorylist.h"
#include "deskphonedetails.h"

#include "qcontactmodel.h"
#include "qtopiaapplication.h"
#include "qpimdelegate.h"

#include <QApplication>
#include <QTextDocument>
#include <QTextFrame>
#include <QStackedWidget>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QIcon>
#include <QSize>
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>
#include <QSoftMenuBar>

#include <QMailAddress>
#include <QMailMessage>

// -------------------------------------------------------------
// DeskphoneContactDetails
// -------------------------------------------------------------
DeskphoneContactDetails::DeskphoneContactDetails( QWidget *parent )
    : QWidget( parent ), mStack(0)
#if defined(QTOPIA_TELEPHONY)
    , mCallHistoryDirty(true)
#endif
    , mMessageHistoryDirty(true)
    , mModel(0)
{
    setObjectName("viewing");

    QSoftMenuBar::setLabel(this, Qt::Key_Back,
        QSoftMenuBar::Back, QSoftMenuBar::AnyFocus);
    QSoftMenuBar::setLabel(this, Qt::Key_Select,
        QSoftMenuBar::NoLabel, QSoftMenuBar::AnyFocus);
}

DeskphoneContactDetails::~DeskphoneContactDetails()
{
}

void DeskphoneContactDetails::init( const QContact &entry )
{
    ent = entry;

    /* Create our members, if we haven't */
    if ( !mModel ) {
        mModel = new QContactModel(this);
        connect(mModel, SIGNAL(modelReset()), this, SLOT(modelChanged()));

        mStack = new QStackedWidget();

        QSoftMenuBar::menuFor(this)->addSeparator();
        QActionGroup *group = new QActionGroup(this);
        mShowDetails = new QAction(tr("Show Contact Details"), this);
        mShowDetails->setCheckable(true);
        mShowDetails->setActionGroup(group);
        connect(mShowDetails, SIGNAL(triggered()), this, SLOT(showDetails()));
        QSoftMenuBar::menuFor(this)->addAction(mShowDetails);
        mShowCalls = new QAction(tr("Show Calls"), this);
        mShowCalls->setCheckable(true);
        mShowCalls->setActionGroup(group);
        connect(mShowCalls, SIGNAL(triggered()), this, SLOT(showCalls()));
        QSoftMenuBar::menuFor(this)->addAction(mShowCalls);
        mShowMessages = new QAction(tr("Show Messages"), this);
        mShowMessages->setCheckable(true);
        mShowMessages->setActionGroup(group);
        connect(mShowMessages, SIGNAL(triggered()), this, SLOT(showMessages()));
        QSoftMenuBar::menuFor(this)->addAction(mShowMessages);

        connect(QSoftMenuBar::menuFor(this), SIGNAL(aboutToShow()),
                this, SLOT(updateMenu()));

        mDeskTab = new DeskphoneContactView(0);
        mStack->addWidget(mDeskTab);

#if defined(QTOPIA_TELEPHONY)
        mCallHistoryTab = new ContactCallHistoryList(0);
        connect(mCallHistoryTab, SIGNAL(closeView()), this, SLOT(showDetails()));
        mCallHistoryHeader = new ContactHeader();
        mCallHistoryHeader->setWidget(mCallHistoryTab);
        connect(mCallHistoryHeader, SIGNAL(clicked()), this, SLOT(showDetails()));
        mStack->addWidget(mCallHistoryHeader);
#endif

        mMessageHistoryTab = new ContactMessageHistoryList(0);
        connect(mMessageHistoryTab, SIGNAL(closeView()), this, SLOT(showDetails()));
        mMessageHistoryHeader = new ContactHeader();
        mMessageHistoryHeader->setWidget(mMessageHistoryTab);
        connect(mMessageHistoryHeader, SIGNAL(clicked()), this, SLOT(showDetails()));
        mStack->addWidget(mMessageHistoryHeader);

        QVBoxLayout *v = new QVBoxLayout();
        v->addWidget(mStack);
        v->setMargin(0);
        setLayout(v);
    }

    modelChanged();

    mStack->setCurrentIndex(0);
}

void DeskphoneContactDetails::modelChanged()
{
    mDeskTab->init(ent);
#if defined(QTOPIA_TELEPHONY)
    if (mStack->currentWidget() == mCallHistoryHeader) {
        mCallHistoryTab->init(ent);
        mCallHistoryHeader->init(ent);
        mCallHistoryDirty = false;
    } else {
        mCallHistoryDirty = true;
    }
#endif
    if (mStack->currentWidget() == mMessageHistoryHeader) {
        mMessageHistoryHeader->init(ent);
        mMessageHistoryTab->init(ent);
        mMessageHistoryDirty = false;
    } else {
        mMessageHistoryDirty = true;
    }
}

void DeskphoneContactDetails::showDetails()
{
    mStack->setCurrentWidget(mDeskTab);
}

void DeskphoneContactDetails::showCalls()
{
#if defined(QTOPIA_TELEPHONY)
    if (mCallHistoryDirty) {
        mCallHistoryTab->init(ent);
        mCallHistoryHeader->init(ent);
        mCallHistoryDirty = false;
    }
    mStack->setCurrentWidget(mCallHistoryHeader);
#endif
}

void DeskphoneContactDetails::showMessages()
{
#if defined(QTOPIA_TELEPHONY)
    if (mMessageHistoryDirty) {
        mMessageHistoryHeader->init(ent);
        mMessageHistoryTab->init(ent);
        mMessageHistoryDirty = false;
    }
    mStack->setCurrentWidget(mMessageHistoryHeader);
#endif
}

void DeskphoneContactDetails::updateMenu()
{
#if defined(QTOPIA_TELEPHONY)
    mShowDetails->setChecked(mStack->currentWidget() == mDeskTab);
    mShowCalls->setChecked(mStack->currentWidget() == mCallHistoryHeader);
    mShowMessages->setChecked(mStack->currentWidget() == mMessageHistoryHeader);
#endif
}

/* DeskphoneContactView */
DeskphoneContactView::DeskphoneContactView(QWidget *parent)
    : QWidget(parent), mInitedGui(false)
{
}

DeskphoneContactView::~DeskphoneContactView()
{
}

// Simple widget that paints itself a certain color
class HeaderBackground : public QWidget
{
    Q_OBJECT

    public:
        void paintEvent(QPaintEvent*)
        {
            QPainter p(this);
            QColor c(45, 97, 141);
            p.fillRect(rect(), c);
        }
};

class SqueezeLabel : public QWidget
{
    Q_OBJECT

public:
    SqueezeLabel(QWidget *parent = 0) : QWidget(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }

    QString text() const {return mText;}
    void setText(const QString& text) {mText = text; update();}

    QSize sizeHint() const
    {
        QFontMetrics fm = fontMetrics();
        return QSize(fm.width("WWWWW"), fm.lineSpacing());
    }

    QSize minimumSizeHint() const
    {
        return sizeHint();
    }

protected:

    void paintEvent(QPaintEvent *)
    {
        QStylePainter p(this);
        QFontMetrics fm(fontMetrics());
        p.drawText(rect(), fm.elidedText(text(), Qt::ElideRight, width()));
    }

    QString mText;
};

void DeskphoneContactView::init(const QContact &entry)
{
    mContact = entry;

    if (!mInitedGui) {
        QVBoxLayout *svlayout = new QVBoxLayout;
        svlayout->setMargin(0);
        QVBoxLayout *main = new QVBoxLayout;
        main->setMargin(0);
        main->setSpacing(0);

        QWidget *scrollPane = new QWidget();
        mScrollArea = new QScrollArea();
        mScrollArea->setFrameStyle(QFrame::NoFrame);
        mScrollArea->setWidgetResizable(true);
        mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        mScrollArea->setFocusPolicy(Qt::NoFocus);

        // Top area (portrait, name & company)
        QVBoxLayout *vtop = new QVBoxLayout;
        vtop->setMargin(0);
        vtop->setSpacing(0);
        QHBoxLayout *top = new QHBoxLayout;
        top->setMargin(0);
        top->setSpacing(0);
        QVBoxLayout *topright = new QVBoxLayout;
        topright->setMargin(0);
        topright->setSpacing(0);

        mPortrait = new FramedContactWidget();
        mPortrait->setFixedSize(QSize(62,62)); // QContact::portraitSize());

        QFont labelFont = font();
        labelFont.setWeight(80);
        labelFont.setPointSizeF(labelFont.pointSize() * 1.2);

        mNameLabel = new SqueezeLabel();
        mNameLabel->setFont(labelFont);

        mCompanyLabel = new SqueezeLabel();

        mPresenceLabel = new SqueezeLabel();
        mPresenceLabel->setForegroundRole(QPalette::BrightText);

        int lineHeight = fontMetrics().lineSpacing();
        int charWidth = fontMetrics().width('W');

        top->addSpacing(charWidth / 2);
        top->addWidget(mPortrait);
        top->addSpacing(charWidth / 2);
        topright->addSpacing(lineHeight / 4);
        topright->addWidget(mNameLabel);
        topright->addWidget(mCompanyLabel);
        topright->addWidget(mPresenceLabel);
        top->addLayout(topright);

        vtop->addSpacing(lineHeight / 3);
        vtop->addLayout(top);
        vtop->addSpacing(lineHeight / 3);

        QWidget *w = new HeaderBackground;
        w->setLayout(vtop);
        main->addWidget(w);
        main->addWidget(new Shadow(Shadow::Top));

        mInitedGui = true;
        mDetailsLayout = new QVBoxLayout;
        mDetailsLayout->setMargin(0);
        mDetailsLayout->setSpacing(0);

        main->addLayout(mDetailsLayout);
        main->addStretch(100);

        scrollPane->setLayout(main);
        mScrollArea->setWidget(scrollPane);
        svlayout->addWidget(mScrollArea);
        setLayout(svlayout);
    }

    populateForm();
}

void DeskphoneContactView::populateForm()
{
    QLayoutItem *child;
    while ((child = mDetailsLayout->takeAt(0)) != 0) {
        delete child->widget();

        QLayout *l = child->layout();
        if (l) {
            QLayoutItem* sub;
            while ((sub = l->takeAt(0)) != 0) {
                delete sub->widget();
                delete sub;
            }
        }
        delete child;
    }

    mFieldMap.clear();
    QStringList phoneFields = QContactFieldDefinition::fields("phone");
    QStringList buddyFields = QContactFieldDefinition::fields("chat");
    bool added = false;

    // Name & Company
    mNameLabel->setText(mContact.label());
    mCompanyLabel->setText(mContact.label() != mContact.company() ? mContact.company() : QString());

    // Presence message
    QPresenceStringMap map = QContactModel::contactField(mContact, QContactModel::PresenceMessage).value<QPresenceStringMap>();
    QPresenceTypeMap typemap = QContactModel::contactField(mContact, QContactModel::PresenceStatus).value<QPresenceTypeMap>();
    QCollectivePresenceInfo::PresenceType type = QtopiaHome::chooseBestPresence(typemap);

    // Pick something at random
    QString uri = typemap.key(type);
    mPresenceLabel->setText(map.value(uri));

    // Portrait
    mPortrait->setContact(mContact);

    // numbers
    foreach (QString phoneField, phoneFields) {
        QContactFieldDefinition def(phoneField);
        QString value = def.value(mContact).toString();

        if (!value.isEmpty()) {
            addNumberField(def, value);
            added = true;
        }
    }

    if (added)
        mDetailsLayout->addWidget(new Shadow(Shadow::Bottom));

    added = false;

    // Now buddies
    foreach (QString buddyField, buddyFields) {
        QContactFieldDefinition def(buddyField);
        QString value = def.value(mContact).toString();

        if (!value.isEmpty()) {
            addBuddyField(def);
            added = true;
        }
    }

    if (added)
        mDetailsLayout->addWidget(new Shadow(Shadow::Bottom));

    // email
    QStringList emails = mContact.emailList();
    QStringList::Iterator it;
    int emailCount = 0;
    added = false;
    for (it = emails.begin() ; it != emails.end() ; ++it) {
        QString trimmed = (*it).trimmed();
        if(!trimmed.isEmpty()) {
            addEmailField(tr("Email"), Qt::escape(trimmed));
            added = true;
            ++emailCount;
        }
    }

    if (added)
        mDetailsLayout->addWidget(new Shadow(Shadow::Bottom));

    // addresses
    QString home = mContact.displayAddress(QContact::Home);
    QString business = mContact.displayAddress(QContact::Business);
    QString other = mContact.displayAddress(QContact::Other);

    added = false;
    if (!home.isEmpty()) {
        added = true;
        addAddressField(tr("Home"), home);
    }

    if (!business.isEmpty()) {
        added = true;
        addAddressField(tr("Business"), business);
    }

    if (!other.isEmpty()) {
        added = true;
        addAddressField(tr("Other"), other);
    }

    if (added)
        mDetailsLayout->addWidget(new Shadow(Shadow::Bottom));

    // Misc fields
    added = false;
    if (mContact.gender() != QContact::UnspecifiedGender) {
        addMiscField(ContactMiscFieldWidget::Gender, mContact.gender() == QContact::Male ? tr("Male") : tr("Female"));
        added = true;
    }
    if (mContact.birthday().isValid()) {
        addMiscField(ContactMiscFieldWidget::Birthday, mContact.birthday().toString());
        added = true;
    }
    if (!mContact.spouse().isEmpty()) {
        addMiscField(ContactMiscFieldWidget::Spouse, mContact.spouse());
        added = true;
    }
    if (mContact.anniversary().isValid()) {
        addMiscField(ContactMiscFieldWidget::Anniversary, mContact.anniversary().toString());
        added = true;
    }
    if (!mContact.children().isEmpty()) {
        addMiscField(ContactMiscFieldWidget::Children, mContact.children());
        added = true;
    }
    if (!mContact.homeWebpage().isEmpty()) {
        addMiscField(ContactMiscFieldWidget::Webpage, mContact.homeWebpage());
        added = true;
    }

    if (added)
        mDetailsLayout->addWidget(new Shadow(Shadow::Bottom));
}

void DeskphoneContactView::addNumberField(const QContactFieldDefinition &def, const QString &value)
{
    HomeFieldButton *cfw = new ContactDefinedFieldWidget(def, value, mGroup);
    connect(cfw, SIGNAL(numberClicked(QContactFieldDefinition)), this, SLOT(numberClicked(QContactFieldDefinition)));
    mFieldMap.insert(cfw, value);

    if (!def.hasTag("fax")) {
        // Message button
        ContactDefinedActionButton *message = new ContactDefinedActionButton(def, tr("Send\ntext"), QColor(0, 80, 0), Qt::white);
        mFieldMap.insert(message, value);
        connect(message, SIGNAL(clicked(QContactFieldDefinition)), this, SLOT(messageClicked(QContactFieldDefinition)));

        QHBoxLayout *hl = new QHBoxLayout;
        hl->addWidget(cfw, 1);
        hl->addWidget(message);
        mDetailsLayout->addLayout(hl);
    } else
        mDetailsLayout->addWidget(cfw);
}

void DeskphoneContactView::addBuddyField(const QContactFieldDefinition &def)
{
    QString value = def.value(mContact).toString();

    ContactBuddyFieldWidget *cfw = new ContactBuddyFieldWidget(def, value, mGroup);

    /* Update the label (needs a contact) */
    cfw->updateLabel(mContact);

    connect(cfw, SIGNAL(numberClicked(QContactFieldDefinition)), this, SLOT(buddyChatClicked(QContactFieldDefinition)));
    mFieldMap.insert(cfw, QCollective::encodeUri(def.provider(), value));

/*
    // Message button
    ContactDefinedActionButton *message = new ContactDefinedActionButton(def, tr("Chat"), QColor(0, 80, 0), Qt::white);
    mFieldMap.insert(message, value);
    connect(message, SIGNAL(clicked(QContactFieldDefinition)), this, SLOT(buddyChatClicked(QContactFieldDefinition)));

    QHBoxLayout *hl = new QHBoxLayout;
    hl->addWidget(cfw, 1);
    hl->addWidget(message);
    mDetailsLayout->addLayout(hl);
*/
    mDetailsLayout->addWidget(cfw);
}

void DeskphoneContactView::addEmailField(const QString &label, const QString &value)
{
    HomeFieldButton *cfw = new HomeFieldButton(label, value,  mGroup);
    mDetailsLayout->addWidget(cfw);

    mFieldMap.insert(cfw, value);
    connect(cfw, SIGNAL(clicked()), this, SLOT(emailClicked()));
}

void DeskphoneContactView::addAddressField(const QString &label, const QString &value)
{
    HomeFieldButton *cfw = new HomeFieldButton(label, value, mGroup);
    // Until we get web support
    cfw->setEnabled(false);
    mDetailsLayout->addWidget(cfw);

    mFieldMap.insert(cfw, value);
}

void DeskphoneContactView::addMiscField(ContactMiscFieldWidget::MiscType type, const QString &value)
{
    HomeFieldButton *cfw = new ContactMiscFieldWidget(type, value, mGroup, false);
    cfw->setEnabled(false);

    mDetailsLayout->addWidget(cfw);

    mFieldMap.insert(cfw, value);
}

void DeskphoneContactView::numberClicked(QContactFieldDefinition)
{
    if (QAbstractButton *pb = qobject_cast<QAbstractButton*>(sender())) {
        QString value = mFieldMap[pb];
        QtopiaServiceRequest req( "Dialer", "dial(QString,QUniqueId)" ); // No tr
        req << value;
        req << mContact.uid();
        req.send();
    }
}

void DeskphoneContactView::messageClicked(QContactFieldDefinition def)
{
    if (qobject_cast<QAbstractButton*>(sender())) {
        QString value = QCollective::encodeUri(def.provider(), def.value(mContact).toString());
        QMailAddressList list;
        list << QMailAddress(mContact.label(), value);
        QtopiaServiceRequest req( "Messages", "composeMessage(QMailMessage::MessageType,QMailAddressList,QString,QString)");
        req << QMailMessage::AnyType << list << QString() << QString();
        req.send();
    }
}

void DeskphoneContactView::buddyClicked(QContactFieldDefinition)
{
    if (QAbstractButton *pb = qobject_cast<QAbstractButton*>(sender())) {
        QString value = mFieldMap[pb];
        // FIXME: What to do?
    }
}

void DeskphoneContactView::buddyChatClicked(QContactFieldDefinition)
{
    if (QAbstractButton *pb = qobject_cast<QAbstractButton*>(sender())) {
        QString value = mFieldMap[pb];
        QMailAddressList list;
        list << QMailAddress(mContact.label(), value);
        QtopiaServiceRequest req( "Messages", "composeMessage(QMailMessage::MessageType,QMailAddressList,QString,QString)");
        req << QMailMessage::Instant << list << QString() << QString();
        req.send();
    }
}

void DeskphoneContactView::emailClicked()
{
    if (QAbstractButton *pb = qobject_cast<QAbstractButton*>(sender())) {
        QString value = mFieldMap[pb];
        QtopiaServiceRequest req( "Email", "writeMail(QString,QString)" );
        req << mContact.label() << value;
        req.send();
    }
}

#include "deskphonedetails.moc"

