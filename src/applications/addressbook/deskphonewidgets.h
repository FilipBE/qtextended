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

#ifndef DESKPHONEWIDGETS_H
#define DESKPHONEWIDGETS_H

#include <QFieldDefinition>
#include <QLabel>
#include <QAbstractButton>
#include <QLinearGradient>
#include <QStylePainter>
#include <QStyleOption>
#include <QDebug>
#include <QAbstractItemDelegate>
#include <QContact>
#include "private/homewidgets_p.h"

//===========================================================================

/* Specialization that has a QContactFieldDefinition (for phone numbers - chat is further specialized) */
class ContactDefinedFieldWidget : public HomeFieldButton
{
    Q_OBJECT;
public:
    ContactDefinedFieldWidget(const QContactFieldDefinition& def, const QString& field, ColumnSizer& group, bool editing = false)
        : HomeFieldButton(def.label(), field, group, editing), mFieldDef(def)
    {
        connect(this, SIGNAL(clicked()), this, SLOT(forwardClickedSignal()));
    }

    QContactFieldDefinition definition() const {return mFieldDef;}
    virtual void setDefinition(const QContactFieldDefinition& def) {mFieldDef = def; setLabel(def.label());}

signals:
    void numberClicked(QContactFieldDefinition def);

private slots:
    void forwardClickedSignal() {emit numberClicked(mFieldDef);}

protected:
    QContactFieldDefinition mFieldDef;
};

/* Specialization for buddies - has presence information */
class ContactBuddyFieldWidget : public ContactDefinedFieldWidget
{
    Q_OBJECT;
public:
    ContactBuddyFieldWidget(const QContactFieldDefinition& def, const QString& field, ColumnSizer& group, bool editing = false)
        : ContactDefinedFieldWidget(def, field, group, editing)
    {
    }

    QSize minimumFieldSize() const;
    void updateLabel(const QContact& contact);
    void drawField(QPainter& p, QRect r) const;

private:
    enum {kAvatarHeight = 54}; // Same size as the top one, minus the frame
    enum {kAvatarWidth = 54};
    QPixmap mAvatar;
};

/* Specialization for addresses */
class ContactAddressFieldWidget : public HomeFieldButton
{
    Q_OBJECT;
public:
    ContactAddressFieldWidget(QContact::Location loc, const QString& label, const QString& field, ColumnSizer& group, bool editing = false)
        : HomeFieldButton(label, field, group, editing), mLocation(loc)
    {
        connect(this, SIGNAL(clicked()), this, SLOT(forwardClickedSignal()));
    }

    QContact::Location location() const {return mLocation;}
signals:
    void locationClicked(QContact::Location def);

private slots:
    void forwardClickedSignal() {emit locationClicked(mLocation);}

protected:
    QContact::Location mLocation;
};

/* Specialization for misc fields (birthday etc) */
class ContactMiscFieldWidget : public HomeFieldButton
{
    Q_OBJECT;
public:
    typedef enum {Invalid, Birthday, Anniversary, Spouse, Children, Webpage, Gender} MiscType;
    Q_ENUMS(MiscType);

    static QStringList typeLabels();
    static QString typeLabel(MiscType type);
    static MiscType type(QString type);

    ContactMiscFieldWidget(MiscType field, const QString& label, ColumnSizer& group, bool editing = false)
        : HomeFieldButton(typeLabel(field), label, group, editing), mType(field)
    {
        connect(this, SIGNAL(clicked()), this, SLOT(forwardClickedSignal()));
    }

    MiscType type() const {return mType;}

signals:
    void miscClicked(ContactMiscFieldWidget::MiscType type);

private slots:
    void forwardClickedSignal() {emit miscClicked(mType);}

protected:
    MiscType mType;
};


// Button for a defined field (send text etc)
class ContactDefinedActionButton : public HomeActionButton
{
    Q_OBJECT;
public:
    ContactDefinedActionButton(const QContactFieldDefinition& def, const QString& text, QColor bg, QColor fg)
        : HomeActionButton(text, bg, fg), mFieldDef(def)
    {
        connect(this, SIGNAL(clicked()), this, SLOT(forwardClickedSignal()));
    }

signals:
    void clicked(QContactFieldDefinition def);
private slots:
    void forwardClickedSignal() {emit clicked(mFieldDef);}
private:
    QContactFieldDefinition mFieldDef;
};


//===========================================================================

class ContactHeader : public QAbstractButton
{
    Q_OBJECT;
public:
    ContactHeader(QWidget *p = 0);

    void setWidget(QWidget *w);
    QWidget *widget() const {return mChild;}

    void init(QContact c);

    void resizeEvent(QResizeEvent*);
    void paintEvent(QPaintEvent*);
    QSize sizeHint() const;

protected:
    void resizeChild();

    QPixmap *shadow()
    {
        static QPixmap p(":image/shadow_over");
        return &p;
    }
protected:
    QPointer<QWidget> mChild;
    QSize mHeaderSize;
    QString mName;
    QString mCompany;
    QContact mContact;
    QFont mNameFont;
    QPixmap mPixmap;
};

//===========================================================================

class ContactPortraitButton : public QAbstractButton
{
    Q_OBJECT;
public:
    ContactPortraitButton();

    void paintEvent(QPaintEvent*);
    QSize sizeHint() const;
};

#endif

