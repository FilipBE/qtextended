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

#include "deskphoneeditor.h"
#include "deskphonewidgets.h"

#include <QtopiaApplication>
#include <QContactModel>
#include <QFieldDefinition>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QAbstractButton>
#include <QLinearGradient>
#include <QStylePainter>
#include <QListView>
#include <QImageReader>

#include "private/qtopiainputdialog_p.h"
#include "contactdetails.h"
#include "contactbrowser.h"
#include "contactoverview.h"
#if defined(QTOPIA_TELEPHONY)
#include "contactcallhistorylist.h"
#endif
#include "contactmessagehistorylist.h"
#include "deskphonedetails.h"
#include "qimagesourceselector.h"

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
#include <QAction>
#include <QMenu>
#include <QSoftMenuBar>
#include <QCommServiceManager>
#include <QCollectivePresence>

class DeskphoneAddressEditor : public QDialog
{
    Q_OBJECT;
public:
    DeskphoneAddressEditor(QWidget *w, QContactAddress a);

    QContactAddress address() {return mAddress;}

private slots:
    void streetClicked();
    void cityClicked();
    void stateClicked();
    void zipClicked();
    void countryClicked();

private:
    void init();
    QString prompt(const QString& title, const QString& label, const QString& value);

private:
    QContactAddress mAddress;
    ColumnSizer mSizer;
    HomeFieldButton *mStreet;
    HomeFieldButton *mCity;
    HomeFieldButton *mState;
    HomeFieldButton *mZip;
    HomeFieldButton *mCountry;
};


// -------------------------------------------------------------
// DeskphoneEditor
// -------------------------------------------------------------
DeskphoneEditor::DeskphoneEditor(QWidget* parent, Qt::WFlags fl)
   : AbEditor(parent, fl),
    mNewEntry(false)
{
    setObjectName("edit");
    mView = new DeskphoneEditorView();
    QVBoxLayout *vl = new QVBoxLayout();
    vl->addWidget(mView);
    vl->setMargin(0);
    setLayout(vl);

    setModal(true);

    setWindowState(windowState() | Qt::WindowMaximized);
}

DeskphoneEditor::~DeskphoneEditor()
{

}

QContact DeskphoneEditor::entry() const
{
    return mContact;
}

void DeskphoneEditor::setEntry(const QContact &entry, bool newEntry)
{
    mView->setEntry(entry, newEntry);
    mNewEntry = newEntry;
}

bool DeskphoneEditor::isEmpty() const
{
    return mView->isEmpty();
}

bool DeskphoneEditor::imageModified() const
{
    return mView->imageModified();
}

void DeskphoneEditor::accept()
{
    mContact = mView->entry();
    if(mNewEntry && mView->isEmpty()) {
        reject();
    } else {
        if (mView->imageModified()) {
            QIODevice* io = mView->contactImage().open();
            QImageReader reader( io );
            if ( reader.supportsOption( QImageIOHandler::Size )) {
                QSize imageSize( reader.size() );
                QSize boundedSize = imageSize.boundedTo( QContact::portraitSize() );

                if (imageSize != boundedSize ) {
                    imageSize.scale( QContact::portraitSize(), Qt::KeepAspectRatio );
                    reader.setQuality( 49 ); // Otherwise Qt smooth scales
                    reader.setScaledSize( imageSize );
                }
            }

            QPixmap pixmap = QPixmap::fromImage( reader.read() );
            if ( !reader.supportsOption( QImageIOHandler::Size ))
                pixmap = pixmap.scaled( QContact::portraitSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

            delete io;

            mContact.changePortrait( pixmap );
        }

        QDialog::accept();
    }
}

/* DeskphoneEditorView */
DeskphoneEditorView::DeskphoneEditorView(QWidget *parent)
    : QWidget(parent), mInitedGui(false), mDirtyImage(false), mModel(new QContactModel(this))
{
}

DeskphoneEditorView::~DeskphoneEditorView()
{
}

bool DeskphoneEditorView::isEmpty() const
{
    return false;
}

bool DeskphoneEditorView::imageModified() const
{
    return mDirtyImage;
}

void DeskphoneEditorView::setEntry(const QContact &entry, bool )
{
    mContact = entry;

    mDirtyImage = false;
    if (!mInitedGui) {
        QVBoxLayout *svlayout = new QVBoxLayout;
        svlayout->setMargin(0);

        mMainLayout = new QVBoxLayout;
        mMainLayout->setMargin(0);
        mMainLayout->setSpacing(0);

        QWidget *scrollPane = new QWidget();
        mScrollArea = new QScrollArea();
        mScrollArea->setFrameStyle(QFrame::NoFrame);
        mScrollArea->setWidgetResizable(true);
        mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        mScrollArea->setFocusPolicy(Qt::NoFocus);

        // Top area (portrait, name & company)
        QHBoxLayout *top = new QHBoxLayout;
        top->setMargin(0);
        top->setSpacing(0);
        QVBoxLayout *topright = new QVBoxLayout;
        topright->setMargin(0);
        topright->setSpacing(0);

        mPortrait = new ContactPortraitButton();
        mPortrait->setBackgroundRole(QPalette::Highlight);
        connect(mPortrait, SIGNAL(clicked()), this, SLOT(portraitClicked()));

        QFont labelFont = font();
        labelFont.setWeight(80);
        labelFont.setPointSizeF(labelFont.pointSize() * 1.2);

        mName = new HomeFieldButton(QString(), QString(), mNameGroup);
        mName->setFont(labelFont);
        connect(mName, SIGNAL(clicked()), this, SLOT(nameClicked()));

        mCompany = new HomeFieldButton(QString(), QString(), mCompanyGroup);
        connect(mCompany, SIGNAL(clicked()), this, SLOT(companyClicked()));

        mMainLayout->addSpacing(14); // Magic numbers from concept drawings
        top->addSpacing(19);
        top->addWidget(mPortrait);
        top->addSpacing(13);
        topright->addSpacing(3);
        topright->addWidget(mName);
        topright->addWidget(mCompany);
        topright->addStretch(100);
        top->addLayout(topright);
        mMainLayout->addLayout(top);
        mMainLayout->addSpacing(10);
        mMainLayout->addWidget(new Shadow(Shadow::Top));

        mInitedGui = true;
        scrollPane->setLayout(mMainLayout);
        mScrollArea->setWidget(scrollPane);
        svlayout->addWidget(mScrollArea);
        setLayout(svlayout);

        /*
           This is code copied from HomeActionButton so we can set
           a max width on the right hand column.
           */

        QFont f = font();
        f.setWeight(80);
        f.setPointSize((int) (f.pointSize() * 0.8f));
        QSize r = QFontMetrics(f).size(0, tr("Remove"));
        mRightFieldSize = QSize(qMax(r.width() + 8, 54), qMax(r.height(), 30));

        mColumnsLayout = new QHBoxLayout;
        mLeftLayout = new QVBoxLayout;
        mRightLayout = new QVBoxLayout;

        mColumnsLayout->setSpacing(0);
        mColumnsLayout->setMargin(0);
        mLeftLayout->setSpacing(0);
        mLeftLayout->setMargin(0);
        mRightLayout->setSpacing(0);
        mRightLayout->setMargin(0);

        mColumnsLayout->addLayout(mLeftLayout);
        mColumnsLayout->addLayout(mRightLayout);
        mMainLayout->addLayout(mColumnsLayout);
    }

    populateForm();
    updateNameAndCompany();

    // Set focus to something
    setFocus();
    focusNextChild();
}

void DeskphoneEditorView::updateNameAndCompany()
{
    QString name = mContact.label();
    QString company = mContact.company();

    if (name == company)
        name.clear();

    if (name.isEmpty())
        mName->setContents(tr("<Name>"), QString());
    else
        mName->setContents(QString(), name);

    if (company.isEmpty())
        mCompany->setContents(tr("<Company>"), QString());
    else
        mCompany->setContents(QString(), company);
}

void DeskphoneEditorView::addShadow(QWidget *button)
{
    QLayoutItem * item = mRightLayout->itemAt(mRow - 1);
    bool added = item && item->widget() && !qobject_cast<Shadow*>(item->widget());

    // We occasionally need to update the two right hand shadows
    // so store them, mapped from this button
    Shadow *s = new Shadow(added ? Shadow::RightAndBottom : Shadow::Right);
    mRightShadows.insert(button, s);
    s->setFixedWidth(mRightFieldSize.width());
    s->setMaximumHeight(button->sizeHint().height());

    mLeftLayout->insertWidget(mRow, button);
    mRightLayout->insertWidget(mRow++, s);

    mLeftLayout->insertWidget(mRow, new Shadow(Shadow::Bottom));
    s = new Shadow(Shadow::OutsideCorner);
    mLowerRightShadows.insert(button, s);
    mRightLayout->insertWidget(mRow++, s);
}

void DeskphoneEditorView::clearLayout(QLayout *l)
{
    if (!l)
        return;

    QLayoutItem *child;
    while ((child = l->takeAt(0)) != 0) {
        if (child->layout()) {
            QLayoutItem *subChild;
            while ((subChild = child->layout()->takeAt(0)) != 0) {
                delete subChild->widget();
                delete subChild;
            }
        }
        delete child->widget();
        delete child;
    }
}

void DeskphoneEditorView::populateForm()
{
    clearLayout(mLeftLayout);
    clearLayout(mRightLayout);
    mFieldMap.clear();
    mRightShadows.clear();
    mLowerRightShadows.clear();

    QString photoFile = mContact.portraitFile();
    if( !photoFile.isEmpty() ) {
        QString baseDirStr = Qtopia::applicationFileName( "addressbook", "contactimages/" );
        QString portraitFilename = photoFile.startsWith(QChar(':')) ? photoFile : baseDirStr + photoFile;
        mContactImage = QContent(portraitFilename);
        mPortrait->setIcon( QIcon(portraitFilename) );
    } else {
        mContactImage = QContent();
        mPortrait->setIcon( QIcon() );
    }

    QStringList phoneFields = QContactFieldDefinition::fields("phone");

    mRow = 0;

    // numbers
    foreach (QString phoneField, phoneFields) {
        QContactFieldDefinition def(phoneField);
        QString value = def.value(mContact).toString();
        if (!value.isEmpty())
            addNumberField(def, value);
    }

    // now the "add new phone" button
    mAddPhoneButton = new HomeActionButton(tr("New phone number"), QColor(19, 109, 6), Qt::white);
    connect(mAddPhoneButton, SIGNAL(clicked()), this, SLOT(addNumberClicked()));
    addShadow(mAddPhoneButton);

    QStringList buddyFields = QContactFieldDefinition::fields("chat");

    // numbers
    foreach (QString buddyField, buddyFields) {
        QContactFieldDefinition def(buddyField);
        QString value = def.value(mContact).toString();
        if (!value.isEmpty())
            addBuddyField(def);
    }

    // and the add buddy button
    mAddBuddyButton = new HomeActionButton(tr("New chat address"), QColor(19, 109, 6), Qt::white);
    connect(mAddBuddyButton, SIGNAL(clicked()), this, SLOT(addBuddyClicked()));
    addShadow(mAddBuddyButton);

    // email
    QStringList emails = mContact.emailList();
    QStringList::Iterator it;
    int emailCount = 0;
    for (it = emails.begin() ; it != emails.end() ; ++it) {
        QString trimmed = (*it).trimmed();
        if(!trimmed.isEmpty()) {
            addEmailField(tr("Email"), Qt::escape(trimmed));
            ++emailCount;
        }
    }

    // now the "add new email" button
    mAddEmailButton = new HomeActionButton(tr("New email address"), QColor(19, 109, 6), Qt::white);
    connect(mAddEmailButton, SIGNAL(clicked()), this, SLOT(addEmailClicked()));
    addShadow(mAddEmailButton);

    // addresses
    addAddressField(QContact::Home);
    addAddressField(QContact::Business);
    addAddressField(QContact::Other);

    // now the "add new address" button
    mAddAddressButton = new HomeActionButton(tr("New address"), QColor(19, 109, 6), Qt::white);
    connect(mAddAddressButton, SIGNAL(clicked()), this, SLOT(addAddressClicked()));
    addShadow(mAddAddressButton);

    // fields
    if (mContact.gender() != QContact::UnspecifiedGender)
        addMiscField(ContactMiscFieldWidget::Gender, mContact.gender() == QContact::Male ? tr("Male") : tr("Female"));
    if (mContact.birthday().isValid())
        addMiscField(ContactMiscFieldWidget::Birthday, mContact.birthday().toString());
    if (!mContact.spouse().isEmpty())
        addMiscField(ContactMiscFieldWidget::Spouse, mContact.spouse());
    if (mContact.anniversary().isValid())
        addMiscField(ContactMiscFieldWidget::Anniversary, mContact.anniversary().toString());
    if (!mContact.children().isEmpty())
        addMiscField(ContactMiscFieldWidget::Children, mContact.children());
    if (!mContact.homeWebpage().isEmpty())
        addMiscField(ContactMiscFieldWidget::Webpage, mContact.homeWebpage());

    // Add field button..
    mAddFieldButton = new HomeActionButton(tr("New field"), QColor(19, 109, 6), Qt::white);
    connect(mAddFieldButton, SIGNAL(clicked()), this, SLOT(addMiscClicked()));
    addShadow(mAddFieldButton);

    // Now add a spacer at the end
    mLeftLayout->addSpacing(40);
    mRightLayout->addSpacing(40);

    // Update the buttons
    updateAddButtons();
}

void DeskphoneEditorView::updateAddButtons()
{
    // See if we need to hide any of the add buttons (e.g.
    // phone buttons, misc field buttons, address buttons)
    bool showAddPhone = false;
    foreach(QString type, QContactFieldDefinition::fields("phone")) {
        if (QContactFieldDefinition(type).value(mContact).toString().isEmpty()) {
            showAddPhone = true;
            break;
        }
    }

    bool showAddBuddy = false;
    foreach(QString type, QContactFieldDefinition::fields("chat")) {
        if (QContactFieldDefinition(type).value(mContact).toString().isEmpty()) {
            showAddBuddy = true;
            break;
        }
    }

    bool showAddAddress = false;
    if (mContact.address(QContact::Other).isEmpty())
        showAddAddress = true;
    if (!showAddAddress && mContact.address(QContact::Home).isEmpty())
        showAddAddress = true;
    if (!showAddAddress && mContact.address(QContact::Business).isEmpty())
        showAddAddress = true;

    bool showAddMisc = false;
    if (mContact.spouse().isEmpty())
        showAddMisc = true;
    if (!showAddMisc && mContact.children().isEmpty())
        showAddMisc = true;
    if (!showAddMisc && mContact.anniversary().isNull())
        showAddMisc = true;
    if (!showAddMisc && mContact.birthday().isNull())
        showAddMisc = true;
    if (!showAddMisc && mContact.homeWebpage().isEmpty())
        showAddMisc = true;
    if (!showAddMisc && mContact.gender() == QContact::UnspecifiedGender)
        showAddMisc = true;

    mAddPhoneButton->setVisible(showAddPhone);
    mAddBuddyButton->setVisible(showAddBuddy);
    mAddAddressButton->setVisible(showAddAddress);
    mAddFieldButton->setVisible(showAddMisc);

    updateShadows();
}

void DeskphoneEditorView::showEvent(QShowEvent *e)
{
    updateShadows();
    QWidget::showEvent(e);
}

void DeskphoneEditorView::updateShadows()
{
    // We have 10 shadows to update
    // The ones directly to the right of the "add" buttons
    // and the shadows underneath those ones.

    // If the button is visible, the right hand shadow will depend on
    // whether there is something above it, and the lower right will
    // be an outside corner).
    //
    // If the button is invisible, the right hand shadow will be
    // invisible and the lower right will be a bottom shadow

    QList<QWidget*> widgets;
    widgets << mAddPhoneButton << mAddBuddyButton << mAddEmailButton << mAddAddressButton << mAddFieldButton;

    foreach(QWidget *w, widgets) {
        if (w->isVisible()) {
            // See if there is something other than a shadow above it
            Shadow *s = mRightShadows[w];
            int row = mRightLayout->indexOf(s) - 1;
            QLayoutItem *item = mRightLayout->itemAt(row);
            if (item && item->widget() && !qobject_cast<Shadow*>(item->widget()))
                s->setType(Shadow::RightAndBottom);
            else
                s->setType(Shadow::Right);
            s->setFixedWidth(mRightFieldSize.width());
            s->setVisible(true);
            mLowerRightShadows[w]->setType(Shadow::OutsideCorner);
            mLowerRightShadows[w]->setFixedWidth(mRightFieldSize.width());
        } else {
            mRightShadows[w]->setVisible(false);
            mRightShadows[w]->setFixedWidth(mRightFieldSize.width());
            mLowerRightShadows[w]->setType(Shadow::Bottom);
            mLowerRightShadows[w]->setFixedWidth(mRightFieldSize.width());
        }
    }
}

void DeskphoneEditorView::addNumberField(const QContactFieldDefinition &def, const QString &value)
{
    HomeFieldButton *cfw = new ContactDefinedFieldWidget(def, value, mBottomGroup, true);
    HomeActionButton *remove = new ContactDefinedActionButton(def, tr("Remove"), QColor(170, 30, 45), Qt::white);
    remove->setBuddy(cfw);

    mFieldMap.insert(remove, cfw);

    connect(cfw, SIGNAL(numberClicked(QContactFieldDefinition)), this, SLOT(numberClicked(QContactFieldDefinition)));
    connect(remove, SIGNAL(clicked(QContactFieldDefinition)), this, SLOT(removeNumberClicked(QContactFieldDefinition)));

    mLeftLayout->insertWidget(mRow, cfw);
    mRightLayout->insertWidget(mRow++, remove);
}

class ContactPhoneTypePicker : public HomeFieldButton
{
    Q_OBJECT;
public:
    ContactPhoneTypePicker(QString, QString, QStringList types, int current);
    ~ContactPhoneTypePicker();

    int current() const {return mCurrent;}

private slots:
    void gotClicked();

private:
    ColumnSizer *mSizer;
    int mCurrent;
    QString mTitle;
    QString mLabel;
    QStringList mTypes;
};

ContactPhoneTypePicker::ContactPhoneTypePicker(QString title, QString label, QStringList types, int current)
    : HomeFieldButton(label, QString(), *(mSizer = new ColumnSizer), true),
    mCurrent(current),
    mTitle(title),
    mLabel(label),
    mTypes(types)
{
    connect(this, SIGNAL(clicked()), this, SLOT(gotClicked()));

    setField(mTypes.value(mCurrent));
}

ContactPhoneTypePicker::~ContactPhoneTypePicker()
{
    delete mSizer;
}

void ContactPhoneTypePicker::gotClicked()
{
    // Pop a full screen picker
    bool ok = false;

    QString value = QtopiaInputDialog::getItem(this, mTitle, mLabel, mTypes, mCurrent, &ok);

    if (ok) {
        mCurrent = mTypes.indexOf(value);
        setField(value);
    }
}

void DeskphoneEditorView::addNumberClicked()
{
    QContactFieldDefinition def;
    QStringList phoneTypes;
    QStringList phoneLabels;

    // TODO - Once we support multiple values, we don't need to remove any more
    // Remove existing phone types
    foreach(QString type, QContactFieldDefinition::fields("phone")) {
        def = QContactFieldDefinition(type);
        if (def.value(mContact).toString().isEmpty()) {
            phoneTypes.append(type);
            phoneLabels.append(def.label());
        }
    }

    ContactPhoneTypePicker *cptp = new ContactPhoneTypePicker(tr("Phone type"), tr("Phone type:"), phoneLabels, 0);
    QLineEdit *le = new QLineEdit;
    le->setFocus();

    QtopiaInputDialog dlg(this, tr("Add number"), cptp, le);

    if (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted) {
        if (!le->text().isEmpty()) {
            def = QContactFieldDefinition(phoneTypes.value(cptp->current()));
            def.setValue(mContact, le->text());
            mRow = mLeftLayout->indexOf(mAddPhoneButton);
            addNumberField(def, le->text());
            updateAddButtons();
        }
    }
}

void DeskphoneEditorView::numberClicked(QContactFieldDefinition def)
{
    // get the CFW
    ContactDefinedFieldWidget *cfw = qobject_cast<ContactDefinedFieldWidget*>(sender());
    if (cfw) {
        QString number = def.value(mContact).toString();

        QStringList phoneTypes;
        QStringList phoneLabels;

        // Put the original type in
        phoneTypes << def.id();
        phoneLabels << def.label();

        // Remove existing phone types
        foreach(QString type, QContactFieldDefinition::fields("phone")) {
            QContactFieldDefinition posdef(type);
            if (posdef.value(mContact).toString().isEmpty()
                    && def.id() != posdef.id()) {
                phoneTypes.append(type);
                phoneLabels.append(posdef.label());
            }
        }

        ContactPhoneTypePicker *cptp = new ContactPhoneTypePicker(tr("Phone type"), tr("Phone type:"), phoneLabels, 0);
        QLineEdit *le = new QLineEdit;
        le->setText(number);
        le->setFocus();

        QtopiaInputDialog dlg(this, tr("Edit number"), cptp, le);

        if (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted) {
            QContactFieldDefinition newDef(phoneTypes.value(cptp->current()));
            if (cptp->current() != 0) // the initial value)
                def.setValue(mContact, QString());

            if (le->text().isEmpty()) {
                delete mLeftLayout->takeAt(mLeftLayout->indexOf(cfw));
                cfw->hide();
                cfw->deleteLater();
                HomeActionButton *b = mFieldMap.key(cfw);
                if (b) {
                    mFieldMap.remove(b);
                    delete b;
                }
                updateAddButtons();
            } else {
                newDef.setValue(mContact, le->text());
                cfw->setDefinition(newDef);
                cfw->setField(le->text());
            }
        }
    }
}

void DeskphoneEditorView::addBuddyField(const QContactFieldDefinition &def)
{
    QString value = def.value(mContact).toString();

    ContactBuddyFieldWidget *cfw = new ContactBuddyFieldWidget(def, value, mBottomGroup, true);
    HomeActionButton *remove = new ContactDefinedActionButton(def, tr("Remove"), QColor(170, 30, 45), Qt::white);
    remove->setBuddy(cfw);
    cfw->updateLabel(mContact);

    mFieldMap.insert(remove, cfw);

    connect(cfw, SIGNAL(numberClicked(QContactFieldDefinition)), this, SLOT(buddyClicked(QContactFieldDefinition)));
    connect(remove, SIGNAL(clicked(QContactFieldDefinition)), this, SLOT(removeNumberClicked(QContactFieldDefinition)));

    mLeftLayout->insertWidget(mRow, cfw);
    mRightLayout->insertWidget(mRow++, remove);
}

void DeskphoneEditorView::addBuddyClicked()
{
    QContactFieldDefinition def;
    QStringList chatTypes;
    QStringList chatLabels;

    // Similar to phone types
    foreach(QString type, QContactFieldDefinition::fields("chat")) {
        def = QContactFieldDefinition(type);
        if (def.value(mContact).toString().isEmpty()) {
            chatTypes.append(type);
            chatLabels.append(def.label());
        }
    }

    ContactPhoneTypePicker *cptp = new ContactPhoneTypePicker(tr("Chat type"), tr("Chat type:"), chatLabels, 0);
    QLineEdit *le = new QLineEdit;
    le->setFocus();

    QtopiaInputDialog dlg(this, tr("Add chat address"), cptp, le);

    if (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted) {
        if (!le->text().isEmpty()) {
            // Let QContactFieldDefinition sort out the protocol stuff
            def = QContactFieldDefinition(chatTypes.value(cptp->current()));
            def.setValue(mContact, le->text());
            mRow = mLeftLayout->indexOf(mAddBuddyButton);
            addBuddyField(def);
            updateAddButtons();
        }
    }
}

void DeskphoneEditorView::buddyClicked(QContactFieldDefinition def)
{
    // Allow editing the URI, but nothing else
    ContactBuddyFieldWidget *cfw = qobject_cast<ContactBuddyFieldWidget*>(sender());
    if (cfw) {
        bool ok = false;
        QString value = def.value(mContact).toString();
        QString newvalue = QtopiaInputDialog::getText(this, tr("Edit chat address"), tr("Chat address:"), QLineEdit::Normal, QtopiaApplication::Named, def.inputHint(), value, &ok);

        if (ok) {
            if (newvalue.isEmpty()) {
                def.setValue(mContact, QString());
                delete mLeftLayout->takeAt(mLeftLayout->indexOf(cfw));
                cfw->hide();
                cfw->deleteLater();
                HomeActionButton *b = mFieldMap.key(cfw);
                if (b) {
                    mFieldMap.remove(b);
                    delete b;
                }
                updateAddButtons();
            } else {
                def.setValue(mContact, newvalue);
                cfw->setDefinition(def);
                cfw->setField(newvalue);
                cfw->updateLabel(mContact);
            }
        }
    }
}

void DeskphoneEditorView::addEmailField(const QString &label, const QString &value)
{
    HomeFieldButton *cfw = new HomeFieldButton(label, value, mBottomGroup, true);
    HomeActionButton *remove = new HomeActionButton(tr("Remove"), QColor(170, 30, 45), Qt::white);
    remove->setBuddy(cfw);

    mFieldMap.insert(remove, cfw);

    connect(cfw, SIGNAL(clicked()), this, SLOT(emailClicked()));
    connect(remove, SIGNAL(clicked()), this, SLOT(removeEmailClicked()));

    mLeftLayout->insertWidget(mRow, cfw);
    mRightLayout->insertWidget(mRow++, remove);
}

void DeskphoneEditorView::addEmailClicked()
{
    bool ok = false;
    QString email = QtopiaInputDialog::getText(this, tr("Edit email address"), tr("Email:"), QLineEdit::Normal, QtopiaApplication::Named, "email", QString(), &ok);
    if (ok) {
        mContact.insertEmail(email);
        mRow = mLeftLayout->indexOf(mAddEmailButton);
        addEmailField(tr("Email"), email);
    }
}

void DeskphoneEditorView::emailClicked()
{
    HomeFieldButton *cfw = qobject_cast<HomeFieldButton*>(sender());
    if (cfw) {
        QString oldemail = cfw->field();
        bool ok = false;
        QString email = QtopiaInputDialog::getText(this, tr("Edit email address"), tr("Email:"), QLineEdit::Normal, QtopiaApplication::Named, "email", oldemail, &ok);
        if (ok) {
            // Remove the old
            if (!oldemail.isEmpty())
                mContact.removeEmail(oldemail);
            if (!email.isEmpty()) {
                mContact.insertEmail(email);
                cfw->setField(email);
            } else {
                // empty email, delete it
                delete mLeftLayout->takeAt(mLeftLayout->indexOf(cfw));
                cfw->hide();
                cfw->deleteLater();

                HomeActionButton *b = mFieldMap.key(cfw);
                if (b) {
                    delete b;
                    mFieldMap.remove(b);
                }
            }
        }
    }
}

void DeskphoneEditorView::portraitClicked()
{
    // pop a picture chooser
    QImageSourceSelectorDialog *iface = new QImageSourceSelectorDialog(this);
    iface->setMaximumImageSize(QContact::portraitSize());
    iface->setContent(mContactImage);
    iface->setWindowTitle(tr("Contact Portrait"));
    if( QtopiaApplication::execDialog( iface ) == QDialog::Accepted ) {
        mDirtyImage = true;
        mContactImage = iface->content();
        mPortrait->setIcon( QIcon(mContactImage.fileName()) );
    }
    delete iface;
}


void DeskphoneEditorView::addAddressField(QContact::Location loc)
{
    QString value = mContact.displayAddress(loc);
    if (value.isEmpty())
        return;

    QString label;

    switch(loc) {
        case QContact::Home:
            label = tr("Home");
            break;
        case QContact::Business:
            label = tr("Business");
            break;
        case QContact::Other:
            label = tr("Other");
            break;
    }

    HomeFieldButton *cfw = new ContactAddressFieldWidget(loc, label, value, mBottomGroup, true);
    HomeActionButton *remove = new HomeActionButton(tr("Remove"), QColor(170, 30, 45), Qt::white);
    remove->setBuddy(cfw);

    mFieldMap.insert(remove, cfw);

    connect(cfw, SIGNAL(locationClicked(QContact::Location)), this, SLOT(addressClicked(QContact::Location)));
    connect(remove, SIGNAL(clicked()), this, SLOT(removeAddressClicked()));

    mLeftLayout->insertWidget(mRow, cfw);
    mRightLayout->insertWidget(mRow++, remove);
}

void DeskphoneEditorView::addMiscField(ContactMiscFieldWidget::MiscType type, const QString &value)
{
    HomeFieldButton *cfw = new ContactMiscFieldWidget(type, value, mBottomGroup, true);
    HomeActionButton *remove = new HomeActionButton(tr("Remove"), QColor(170, 30, 45), Qt::white);
    remove->setBuddy(cfw);

    mFieldMap.insert(remove, cfw);

    connect(cfw, SIGNAL(miscClicked(ContactMiscFieldWidget::MiscType)), this, SLOT(miscClicked(ContactMiscFieldWidget::MiscType)));
    connect(remove, SIGNAL(clicked()), this, SLOT(removeMiscClicked()));

    mLeftLayout->insertWidget(mRow, cfw);
    mRightLayout->insertWidget(mRow++, remove);
}

void DeskphoneEditorView::addressClicked(QContact::Location loc)
{
    QContactAddress a = mContact.address(loc);
    DeskphoneAddressEditor dlg(this, a);

    if (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted) {
        mContact.setAddress(loc, dlg.address());
        ContactAddressFieldWidget *cfw = qobject_cast<ContactAddressFieldWidget*>(sender());
        if (cfw)
            cfw->setField(mContact.displayAddress(loc));
    }
}

void DeskphoneEditorView::miscClicked(ContactMiscFieldWidget::MiscType )
{
    // Edit the specific misc field
    ContactMiscFieldWidget *cfw = qobject_cast<ContactMiscFieldWidget*>(sender());
    if (cfw) {
        bool ok = false;
        switch (cfw->type()) {
            case ContactMiscFieldWidget::Birthday:
            {
                QDate date = QtopiaInputDialog::getDate(this, tr("Birthday"), tr("Birthday"), mContact.birthday(), QDate(), QDate(), &ok);
                if (ok) {
                    mContact.setBirthday(date);
                    cfw->setField(date.toString());
                }
                break;
            }
            case ContactMiscFieldWidget::Anniversary:
            {
                QDate date = QtopiaInputDialog::getDate(this, tr("Anniversary"), tr("Anniversary"), mContact.anniversary(), QDate(), QDate(), &ok);
                if (ok) {
                    mContact.setAnniversary(date);
                    cfw->setField(date.toString());
                }
                break;
            }
            case ContactMiscFieldWidget::Spouse:
            {
                QString str = QtopiaInputDialog::getText(this, tr("Spouse"), tr("Spouse"), QLineEdit::Normal, QtopiaApplication::ProperNouns, QString(), mContact.spouse(), &ok);
                if (ok) {
                    mContact.setSpouse(str);
                    cfw->setField(str);
                }
                break;
            }

            case ContactMiscFieldWidget::Children:
            {
                QString str = QtopiaInputDialog::getText(this, tr("Children"), tr("Children"), QLineEdit::Normal, QtopiaApplication::ProperNouns, QString(), mContact.children(), &ok);
                if (ok) {
                    mContact.setChildren(str);
                    cfw->setField(str);
                }
                break;
            }

            case ContactMiscFieldWidget::Webpage:
            {
                QString str = QtopiaInputDialog::getText(this, tr("Webpage"), tr("Webpage"), QLineEdit::Normal, QtopiaApplication::Named, "url", mContact.homeWebpage(), &ok);
                if (ok) {
                    mContact.setHomeWebpage(str);
                    cfw->setField(str);
                }
                break;
            }

            case ContactMiscFieldWidget::Gender:
            {
                QStringList genders;
                genders << tr("Male") << tr("Female");
                QString gender = QtopiaInputDialog::getItem(this, tr("Gender"), tr("Gender"), genders, mContact.gender() == QContact::Male, &ok);
                if (ok) {
                    switch(genders.indexOf(gender)) {
                        case 0:
                            mContact.setGender(QContact::Male);
                            cfw->setField(gender);
                            break;
                        case 1:
                            mContact.setGender(QContact::Female);
                            cfw->setField(gender);
                            break;
                        default:
                            break;
                    }
                }
            }

            default:
                // nada
                break;
        }

    }
}

void DeskphoneEditorView::removeNumberClicked(QContactFieldDefinition def)
{
    // remove buttons and adjust the contact
    HomeActionButton *b = qobject_cast<HomeActionButton*>(sender());
    if (b) {
        HomeFieldButton *cfw = mFieldMap.value(b);
        mFieldMap.remove(b);

        // Fix our contact
        def.setValue(mContact, QString());

        delete mRightLayout->takeAt(mRightLayout->indexOf(b));
        b->hide();
        b->deleteLater();
        delete cfw;

        updateAddButtons();
    }
}

void DeskphoneEditorView::removeEmailClicked()
{
    // remove buttons and adjust the contact
    HomeActionButton *b = qobject_cast<HomeActionButton*>(sender());
    if (b) {
        HomeFieldButton *cfw = mFieldMap.value(b);
        mFieldMap.remove(b);

        mContact.removeEmail(cfw->field());

        delete mRightLayout->takeAt(mRightLayout->indexOf(b));
        b->hide();
        b->deleteLater();
        delete cfw;

        updateShadows();
    }
}

void DeskphoneEditorView::removeAddressClicked()
{
    // remove buttons and adjust the contact
    HomeActionButton *b = qobject_cast<HomeActionButton*>(sender());
    if (b) {
        ContactAddressFieldWidget *cfw = qobject_cast<ContactAddressFieldWidget*>(mFieldMap.value(b));

        mContact.setAddress(cfw->location(), QContactAddress());

        mFieldMap.remove(b);

        delete mRightLayout->takeAt(mRightLayout->indexOf(b));
        b->hide();
        b->deleteLater();
        delete cfw;

        updateAddButtons();
    }
}

void DeskphoneEditorView::removeMiscClicked()
{
    HomeActionButton *b = qobject_cast<HomeActionButton*>(sender());
    if (b) {
        ContactMiscFieldWidget *cfw = qobject_cast<ContactMiscFieldWidget*>(mFieldMap.value(b));

        switch(cfw->type()) {
            case ContactMiscFieldWidget::Birthday:
                mContact.setBirthday(QDate());
                break;
            case ContactMiscFieldWidget::Anniversary:
                mContact.setAnniversary(QDate());
                break;
            case ContactMiscFieldWidget::Spouse:
                mContact.setSpouse(QString());
                break;
            case ContactMiscFieldWidget::Children:
                mContact.setChildren(QString());
                break;
            case ContactMiscFieldWidget::Webpage:
                mContact.setHomeWebpage(QString());
                break;
            case ContactMiscFieldWidget::Gender:
                mContact.setGender(QContact::UnspecifiedGender);
                break;
            default:
                break;
        }

        mFieldMap.remove(b);

        delete mRightLayout->takeAt(mRightLayout->indexOf(b));
        b->hide();
        b->deleteLater();
        delete cfw;

        updateAddButtons();
    }
}

void DeskphoneEditorView::addAddressClicked()
{
    // Make something up
    QContact::Location loc = QContact::Home;
    if (!mContact.address(loc).isEmpty())
        loc = QContact::Business;
    if (!mContact.address(loc).isEmpty())
        loc = QContact::Other;
    if (!mContact.address(loc).isEmpty())
        return; // we shouldn't actually reach this

    // Make something up
    QContactAddress a;

    DeskphoneAddressEditor dlg(this, a);

    if (QtopiaApplication::execDialog(&dlg) == QDialog::Accepted) {
        mContact.setAddress(loc, dlg.address());

        // Hmm.
        mRow = mLeftLayout->indexOf(mAddAddressButton);
        addAddressField(loc);
        updateAddButtons();
    }
}

void DeskphoneEditorView::addMiscClicked()
{
    // Provide a list of misc fields
    bool ok = false;

    QList<ContactMiscFieldWidget::MiscType> types;

    // Add any fields that we don't have
    if (mContact.gender() == QContact::UnspecifiedGender)
        types.append(ContactMiscFieldWidget::Gender);
    if (mContact.spouse().isEmpty())
        types.append(ContactMiscFieldWidget::Spouse);
    if (mContact.children().isEmpty())
        types.append(ContactMiscFieldWidget::Children);
    if (mContact.anniversary().isNull())
        types.append(ContactMiscFieldWidget::Anniversary);
    if (mContact.birthday().isNull())
        types.append(ContactMiscFieldWidget::Birthday);
    if (mContact.homeWebpage().isEmpty())
        types.append(ContactMiscFieldWidget::Webpage);

    QStringList fields;
    foreach(ContactMiscFieldWidget::MiscType type, types)
        fields << ContactMiscFieldWidget::typeLabel(type);

    // and choose a new field
    QString field = QtopiaInputDialog::getItem(this, tr("Add field"), tr("Type"), fields, 0, &ok);

    if (ok) {
        // Now, depending on the type...
        switch (ContactMiscFieldWidget::type(field)) {
            case ContactMiscFieldWidget::Birthday:
            {
                QDate date = QtopiaInputDialog::getDate(this, tr("Birthday"), tr("Birthday"), QDate::currentDate(), QDate(), QDate(), &ok);
                if (ok) {
                    mContact.setBirthday(date);
                    mRow = mLeftLayout->indexOf(mAddFieldButton);
                    addMiscField(ContactMiscFieldWidget::Birthday, date.toString());
                }
                break;
            }
            case ContactMiscFieldWidget::Anniversary:
            {
                QDate date = QtopiaInputDialog::getDate(this, tr("Anniversary"), tr("Anniversary"), QDate::currentDate(), QDate(), QDate(), &ok);
                if (ok) {
                    mContact.setAnniversary(date);
                    mRow = mLeftLayout->indexOf(mAddFieldButton);
                    addMiscField(ContactMiscFieldWidget::Anniversary, date.toString());
                }
                break;
            }
            case ContactMiscFieldWidget::Spouse:
            {
                QString str = QtopiaInputDialog::getText(this, tr("Spouse"), tr("Spouse"), QLineEdit::Normal, QtopiaApplication::ProperNouns, QString(), QString(), &ok);
                if (ok) {
                    mContact.setSpouse(str);
                    mRow = mLeftLayout->indexOf(mAddFieldButton);
                    addMiscField(ContactMiscFieldWidget::Spouse, str);
                }
                break;
            }

            case ContactMiscFieldWidget::Children:
            {
                QString str = QtopiaInputDialog::getText(this, tr("Children"), tr("Children"), QLineEdit::Normal, QtopiaApplication::ProperNouns, QString(), QString(), &ok);
                if (ok) {
                    mContact.setChildren(str);
                    mRow = mLeftLayout->indexOf(mAddFieldButton);
                    addMiscField(ContactMiscFieldWidget::Children, str);
                }
                break;
            }

            case ContactMiscFieldWidget::Webpage:
            {
                QString str = QtopiaInputDialog::getText(this, tr("Webpage"), tr("Webpage"), QLineEdit::Normal, QtopiaApplication::Named, "url", QString(), &ok);
                if (ok) {
                    mContact.setHomeWebpage(str);
                    mRow = mLeftLayout->indexOf(mAddFieldButton);
                    addMiscField(ContactMiscFieldWidget::Webpage, str);
                }
                break;
            }

            case ContactMiscFieldWidget::Gender:
            {
                QStringList genders;
                genders << tr("Male") << tr("Female");
                QString gender = QtopiaInputDialog::getItem(this, tr("Gender"), tr("Gender"), genders, 0, &ok);
                if (ok) {
                    switch(genders.indexOf(gender)) {
                        case 0:
                            mContact.setGender(QContact::Male);
                            mRow = mLeftLayout->indexOf(mAddFieldButton);
                            addMiscField(ContactMiscFieldWidget::Gender, gender);
                            break;
                        case 1:
                            mContact.setGender(QContact::Female);
                            mRow = mLeftLayout->indexOf(mAddFieldButton);
                            addMiscField(ContactMiscFieldWidget::Gender, gender);
                            break;
                        default:
                            break;
                    }
                }
            }

            default:
                // nada
                break;
        }
        updateAddButtons();
    }
}

void DeskphoneEditorView::nameClicked()
{
    QString name = mContact.label();
    bool ok = false;
    if (name == mContact.company())
        name.clear();
    name = QtopiaInputDialog::getText(this, tr("Name"),tr("Name"), QLineEdit::Normal, QtopiaApplication::Named, QString(), name, &ok);
    if (!ok)
        return;
    // This is suboptimal
    QContact parsed = QContact::parseLabel(name);
    mContact.setFirstName(parsed.firstName());
    mContact.setMiddleName(parsed.middleName());
    mContact.setLastName(parsed.lastName());
    mContact.setSuffix(parsed.suffix());
    mContact.setNameTitle(parsed.nameTitle());
    mContact.setNickname(parsed.nickname());
    mContact.setFirstNamePronunciation(parsed.firstNamePronunciation());
    mContact.setLastNamePronunciation(parsed.lastNamePronunciation());

    updateNameAndCompany();
}

void DeskphoneEditorView::companyClicked()
{
    bool ok = false;
    QString company = QtopiaInputDialog::getText(this, tr("Company"), tr("Company"), QLineEdit::Normal, QtopiaApplication::Named, QString(), mContact.company(), &ok);
    if(!ok)
        return;
    mContact.setCompany(company);
    updateNameAndCompany();
}

DeskphoneAddressEditor::DeskphoneAddressEditor(QWidget *w, QContactAddress a)
    : QDialog(w),
    mAddress(a)
{
    init();
    setWindowTitle(tr("Editing address"));
}

void DeskphoneAddressEditor::init()
{
    mStreet = new HomeFieldButton(tr("Street"), mAddress.street, mSizer, true);
    mCity = new HomeFieldButton(tr("City"), mAddress.city, mSizer, true);
    mState = new HomeFieldButton(tr("State"), mAddress.state, mSizer, true);
    mZip = new HomeFieldButton(tr("Zip"), mAddress.zip, mSizer, true);
    mCountry = new HomeFieldButton(tr("Country"), mAddress.country, mSizer, true);

    connect(mStreet, SIGNAL(clicked()), this, SLOT(streetClicked()));
    connect(mCity, SIGNAL(clicked()), this, SLOT(cityClicked()));
    connect(mState, SIGNAL(clicked()), this, SLOT(stateClicked()));
    connect(mZip, SIGNAL(clicked()), this, SLOT(zipClicked()));
    connect(mCountry, SIGNAL(clicked()), this, SLOT(countryClicked()));

    QVBoxLayout *vl = new QVBoxLayout;
    vl->setSpacing(0);
    vl->addWidget(mStreet);
    vl->addWidget(mCity);
    vl->addWidget(mState);
    vl->addWidget(mZip);
    vl->addWidget(mCountry);
    vl->addStretch(1);

    setLayout(vl);
}

QString DeskphoneAddressEditor::prompt(const QString& title, const QString& label, const QString& value)
{
    bool ok = false;
    QString ret = QtopiaInputDialog::getText(this, title, label, QLineEdit::Normal, QtopiaApplication::Words, QString(), value, &ok);

    if (ok)
        return ret;
    return value;
}

void DeskphoneAddressEditor::streetClicked()
{
    mAddress.street = prompt(tr("Street"), tr("Street"), mAddress.street);
    mStreet->setField(mAddress.street);
}

void DeskphoneAddressEditor::cityClicked()
{
    mAddress.city = prompt(tr("City"), tr("City"), mAddress.city);
    mCity->setField(mAddress.city);
}

void DeskphoneAddressEditor::stateClicked()
{
    mAddress.state = prompt(tr("State"), tr("State"), mAddress.state);
    mState->setField(mAddress.state);
}

void DeskphoneAddressEditor::zipClicked()
{
    mAddress.zip = prompt(tr("Zip"), tr("Zip"), mAddress.zip);
    mZip->setField(mAddress.zip);
}

void DeskphoneAddressEditor::countryClicked()
{
    mAddress.country = prompt(tr("Country"), tr("Country"), mAddress.country);
    mCountry->setField(mAddress.country);
}

#include "deskphoneeditor.moc"

