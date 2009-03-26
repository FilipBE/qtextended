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
#ifndef DESKPHONEEDITOR_H
#define DESKPHONEEDITOR_H

#include <QWidget>
#include <QMap>
#include <QContact>
#include <QFieldDefinition>
#include <QDialog>
#include <QContent>

#include <qcontact.h>
#include <qtopiaservices.h>

#include "abeditor.h"
#include "deskphonewidgets.h"

class QVBoxLayout;
class QHBoxLayout;
class QStackedWidget;
class QScrollArea;
class QLabel;
class QContactModel;

class DeskphoneEditorView;
class QContactModel;
class HomeActionButton;
class Shadow;
class HomeFieldButton;

class DeskphoneEditor : public AbEditor
{
    Q_OBJECT
public:
    DeskphoneEditor( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~DeskphoneEditor();

    virtual void setEntry(const QContact &entry, bool newEntry);
    virtual bool isEmpty() const;
    virtual bool imageModified() const;

    QContact entry() const;

public slots:
    virtual void accept();

private:
    DeskphoneEditorView *mView;
    QContact mContact;
    bool mNewEntry;
};

//===========================================================================

class DeskphoneEditorView : public QWidget
{
    Q_OBJECT

public:
    DeskphoneEditorView(QWidget *parent = 0);
    ~DeskphoneEditorView();

    virtual void setEntry(const QContact &entry, bool newEntry);
    virtual bool isEmpty() const;
    virtual bool imageModified() const;
    QContact entry() const {return mContact;}

    const QContent& contactImage() {return mContactImage;}

    void showEvent(QShowEvent *e);

private slots:
    void numberClicked(QContactFieldDefinition def);
    void removeNumberClicked(QContactFieldDefinition def);
    void addNumberClicked();

    void buddyClicked(QContactFieldDefinition def);
    void addBuddyClicked();

    void addressClicked(QContact::Location);
    void removeAddressClicked();
    void addAddressClicked();

    void emailClicked();
    void removeEmailClicked();
    void addEmailClicked();

    void miscClicked(ContactMiscFieldWidget::MiscType);
    void removeMiscClicked();
    void addMiscClicked();

    void nameClicked();
    void companyClicked();
    void portraitClicked();

private:
    void populateForm();
    void updateNameAndCompany();
    void addShadow(QWidget *button);
    void addNumberField(const QContactFieldDefinition &def, const QString &value);
    void addBuddyField(const QContactFieldDefinition &def);
    void addEmailField(const QString &label, const QString &value);
    void addAddressField(QContact::Location);
    void addMiscField(ContactMiscFieldWidget::MiscType type, const QString &value);
    void updateShadows();
    void updateAddButtons();

    void clearLayout(QLayout *l);

private:
    QContact mContact;
    bool mInitedGui;
    bool mDirtyImage;
    QContent mContactImage;

    ColumnSizer mNameGroup;
    ColumnSizer mCompanyGroup;
    ColumnSizer mBottomGroup;
    QAbstractButton* mPortrait;
    HomeFieldButton *mName;
    HomeFieldButton *mCompany;
    QContactModel *mModel;
    QScrollArea *mScrollArea;
    QWidget *mFormContainer;
    QMap<HomeActionButton*, HomeFieldButton*> mFieldMap;

    QMap<QWidget*,Shadow*> mRightShadows;
    QMap<QWidget*,Shadow*> mLowerRightShadows;

    QColor mLabelColor;
    HomeActionButton *mAddPhoneButton;
    HomeActionButton *mAddBuddyButton;
    HomeActionButton *mAddEmailButton;
    HomeActionButton *mAddAddressButton;
    HomeActionButton *mAddFieldButton;

    QSize mRightFieldSize;
    QVBoxLayout *mMainLayout;
    QVBoxLayout *mLeftLayout;
    QVBoxLayout *mRightLayout;
    QHBoxLayout *mColumnsLayout;
    int mRow; // temp used for populating form
};

#endif
