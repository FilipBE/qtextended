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
#ifndef ABEDITOR_H
#define ABEDITOR_H

#include <qcontact.h>
#include <qappointment.h>
#include <qcontactmodel.h>
#include <qtopia/qcontent.h>

#include <QDialog>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QLineEdit>

class QIconSelector;
class QVBoxLayout;
class QShowEvent;
class QKeyEvent;
class QCheckBox;
class QLabel;
class QComboBox;
class QHBox;
class QTabWidget;
class QToolButton;
class QPushButton;
class QCategorySelector;
class QDateEdit;
class QGroupBox;
class QTextEdit;
class QGridLayout;
class QAction;
class QScrollArea;
class QDLEditClient;
class RingToneButton;
class ReminderPicker;
class GroupView;
class QCategoryManager;
class AbFullEditor;
class FieldLineEdit;
class QContactFieldList;

// detail editor ; constructs a dialog to edit fields specified by a key => value map
class AbDetailEditor : public QDialog
{
    Q_OBJECT
public:
    AbDetailEditor( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~AbDetailEditor();

    QList<QContactModel::Field> guiList( const QMap<QContactModel::Field, QString> &f ) const;
    void setFields( const QMap<QContactModel::Field, QString> &f );
    QMap<QContactModel::Field, QString> fields() const;

protected slots:
    void accept();
protected:
    virtual const QMap<QContactModel::Field, QString> displayNames() const;

    QMap<QContactModel::Field, QString> myFields;
    QComboBox *suffixCombo;
    QComboBox *titleCombo;

    QMap<QContactModel::Field, QLineEdit *> lineEdits;
private:
    QScrollArea *mView;
    QVBoxLayout *editorLayout;
};

//-----------------------------------------------------------------------

class AbstractField : public QLineEdit
{
    Q_OBJECT
public:
    AbstractField( QWidget *parent = 0 );
    ~AbstractField();

    void setFields( const QMap<QContactModel::Field, QString> &fields );
    QMap<QContactModel::Field, QString> fields() const;

    virtual bool isEmpty() const;

    QStringList tokenize( const QString &newText ) const;

    virtual QString fieldName() const = 0;

    bool modified() const;
    void setModified( bool b );

public slots:
    virtual void parse() = 0;
    virtual void fieldsChanged() = 0;
    void details();

protected:
    QMap<QContactModel::Field, QString> myFields;
    bool mModified;

private:
    AbDetailEditor *detailEditor;
};

//-----------------------------------------------------------------------

// AbstractName field handles parsing of user input and calls subdialog to handle details
class AbstractName : public AbstractField
{
    Q_OBJECT
public:
    AbstractName( QWidget *parent = 0 );
    ~AbstractName();

    QString fieldName() const;

    bool isEmpty() const;

public slots:
    void parse();
    void fieldsChanged();

private slots:
    void textChanged();
private:
    bool m_preventModified;
};

//-----------------------------------------------------------------------

class AbEditor : public QDialog
{
    Q_OBJECT
public:
    AbEditor( QWidget* parent = 0, Qt::WFlags fl = 0 )
        : QDialog(parent, fl)
    {}
    ~AbEditor()
    {}

    virtual QContact entry() const = 0;

    virtual bool isEmpty() const = 0;

    virtual void setEntry(const QContact &entry, bool newEntry) = 0;

    virtual bool imageModified() const { return false; }

    virtual QAppointment::AlarmFlags anniversaryReminder() {return QAppointment::NoAlarm;}
    virtual int anniversaryReminderDelay() {return 0;}

    virtual QAppointment::AlarmFlags birthdayReminder() {return QAppointment::NoAlarm;}
    virtual int birthdayReminderDelay() {return 0;}
};

class QDelayedScrollArea;
class AbFullEditor : public AbEditor
{
    Q_OBJECT
public:
    AbFullEditor( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~AbFullEditor();

    void setCategory(int);
    void setNameFocus();
    QContact entry() const { return ent; }

    bool isEmpty() const;

    bool imageModified() const;

    void setEntry(const QContact &entry, bool newEntry);

    virtual QAppointment::AlarmFlags anniversaryReminder() {return anniversaryAppt.alarm();}
    virtual int anniversaryReminderDelay() {return anniversaryAppt.alarmDelay();}

    virtual QAppointment::AlarmFlags birthdayReminder() {return birthdayAppt.alarm();}
    virtual int birthdayReminderDelay() {return birthdayAppt.alarmDelay();}

protected slots:
    void editPhoto();

    void editGroups();

    void updateContextMenu();

    void showSpecWidgets( bool s );
    void catCheckBoxChanged( bool b );

    //Communication between Contact tab and details on other tabs

    void specFieldsFilter( const QString &newValue );

    void accept();
    void reject();
    void tabClicked( QWidget *tab );
    void editEmails();
    void prepareTab(int);

    void toneSelected( const QContent &tone );

    void activateFieldAction(const QString &, const QString &value);
protected:
    void closeEvent(QCloseEvent *e);
    void showEvent( QShowEvent *e );

private:
    void init();
    void initMainUI();

    void setupTabs();

    void setupTabCommon();
    void setupTabWork();
    void setupTabHome();
    void setupTabOther();

    void setEntryWork();
    void setEntryHome();
    void setEntryOther();

    void contactFromFields(QContact &);

    void updateGroupButton();

    void updateAppts();

private:
    bool mImageModified;
    bool mNewEntry;

    QContent mContactImage;

    QContact ent;
    QTextEdit *txtNote;
    QDLEditClient *txtNoteQC;
    QTabWidget *tabs;
    QDelayedScrollArea *contactTab, *businessTab, *personalTab, *otherTab;
    QWidget *summaryTab;
    QTextEdit *summary;

    bool lastUpdateInternal;

    QMap<QContactModel::Field, QLineEdit *> lineEdits;

    QVBoxLayout* mainVBox;

    //
    //  Contact Tab
    //

    AbstractName *abName;
    QLineEdit *phoneLE, *mobileLE;
    QPushButton *catPB;
    QComboBox *genderCombo;
    QCheckBox *categoryCB;
    QLineEdit *emailLE;
    QPushButton *emailBtn;
    QDateEdit *bdayEdit;
    QDateEdit *anniversaryEdit;
    QGroupBox *bdayCheck;
    QGroupBox *anniversaryCheck;
    QHBox *ehb;

    //
    //  Widgets specific to the contact type
    //

    QLineEdit *specCompanyLE, *specJobTitleLE;
    QLabel *specCompanyLA, *specJobTitleLA;

    //
    //  Business tab widgets
    //

    QLineEdit *companyLE, *companyProLE, *jobTitleLE,
              *busWebPageLE, *deptLE, *officeLE,
              *professionLE, *managerLE, *assistantLE;

    //Home tab widgets
    QLineEdit *homeWebPageLE,
              *spouseLE, *anniversaryLE, *childrenLE;

    // Phone number fields.
    FieldLineEdit *busPhoneLE, *busFaxLE, *busMobileLE, *busPagerLE,
                  *homePhoneLE, *homeFaxLE, *homeMobileLE;

#if defined(QTOPIA_VOIP)
    QLineEdit *busVoipLE, *homeVoipLE;
#endif

    QTextEdit *busStreetME, *homeStreetME;
    QLineEdit *busCityLE, *busStateLE, *busZipLE, *busCountryLE,
              *homeCityLE, *homeStateLE, *homeZipLE, *homeCountryLE;

    ReminderPicker *anniversaryRP, *birthdayRP;
    QAppointment anniversaryAppt, birthdayAppt;

    QToolButton *photoPB;
    QContactFieldList *phoneNumbers;

    QAction *actionEmailDetails;
#if defined(QTOPIA_TELEPHONY)
    RingToneButton *editTonePB, *editVideoTonePB;
#endif
    QWidget *wOtherTab;
    QWidget *wBusinessTab;
    QWidget *wPersonalTab;
    GroupView *mGroupPicker;
    QCategoryManager *mCatMan;
    QStringList mGroupList;
    QDialog *mGroupDialog;

    QAction* actionAddGroup;
    QAction* actionRemoveGroup;
    QAction* actionRenameGroup;
#if defined(QTOPIA_TELEPHONY)
    QAction* actionSetRingTone;
#endif
};

#ifdef QTOPIA_CELL
class AbSimEditor : public AbEditor
{
    Q_OBJECT
public:
    AbSimEditor( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~AbSimEditor();

    void setNameFocus();

    QContact entry() const { return ent; }

    bool isEmpty() const;

    void setEntry(const QContact &entry, bool newEntry);

protected slots:
    void accept();
    void reject();
private:
    void initSimUI();

    //
    //  SIM-plified contact dialog
    //

    QWidget *simEditor;
    QLineEdit *simName;
    QContactFieldList *phoneNumbers;

    QContact ent;

    bool mNewEntry;
};
#endif


#endif
