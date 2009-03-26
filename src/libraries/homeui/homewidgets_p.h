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

#ifndef HOMEWIDGETS_P_H
#define HOMEWIDGETS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtopiaglobal.h>
#include <qcollectivepresenceinfo.h>

#include <QAbstractButton>
#include <QContact>
#include <QSet>
#include <QWidget>

class QPixmap;
class QStyleOptionButton;
class QStylePainter;
class QShowEvent;
class QPaintEvent;
class QEvent;
class QContactModel;

//===========================================================================

namespace QtopiaHome
{
    QTOPIA_EXPORT void setPopupDialogStyle(QDialog *);

    enum HomeColor { Green = 0, Red };

    QTOPIA_EXPORT const QColor &standardColor(HomeColor);

    QTOPIA_EXPORT QColor presenceColor(QCollectivePresenceInfo::PresenceType type);
    QTOPIA_EXPORT QCollectivePresenceInfo::PresenceType chooseBestPresence(const QMap<QString, QCollectivePresenceInfo::PresenceType> &map);

    QTOPIA_EXPORT QCollectivePresenceInfo::PresenceType bestPresence(const QContact &contact);
}

//===========================================================================

/*
    Simple class that measures the width of a number of fields and
    returns the maximum (for aligning things in columns).

    Objects measured need to have a slot/invokable with signature
    'int columnWidth()'
*/

class QTOPIA_EXPORT ColumnSizer : public QObject
{
    Q_OBJECT;
public:
    ColumnSizer();
    void updateFields();
    int columnWidth();
    void addField(QObject *);

private slots:
    void objectDestroyed();

private:
    int mColumnWidth;
    QSet<QObject*> mFields;
};

//===========================================================================

/* Generic button with a specific visual style */
class QTOPIA_EXPORT HomeActionButton : public QAbstractButton
{
    Q_OBJECT;
public:
    enum { kLabelPadding = 4 };
    enum { kMinimumVerticalMargin = 5 };
    enum { kMinimumButtonWidth = 48 };

    HomeActionButton(const QString& text, QColor bg, QColor fg);
    HomeActionButton(QtopiaHome::HomeColor color = QtopiaHome::Green);
    HomeActionButton(const QString& text, QtopiaHome::HomeColor color);
    void setBuddy(QWidget *w);
    QSize sizeHint() const;
    void setColors(QColor bg, QColor fg);
    void paintEvent(QPaintEvent*);
    static void setPaletteFromColor(QPalette *p, QtopiaHome::HomeColor color);
    static void paintButton(QStyleOptionButton *option,
                            QStylePainter *p,
                            QRect r,
                            QString str);

protected:
    void init();

    QWidget *mBuddy;
};

/* toggles text labels when button is checked/unchecked
   (could toggle button colour too if necessary) */
class QTOPIA_EXPORT CheckableHomeActionButton : public HomeActionButton
{
    Q_OBJECT;
public:
    CheckableHomeActionButton(const QString& textUnchecked,
                              const QString& textChecked,
                              const QColor &bgColor);

protected:
    void checkStateSet();
    void nextCheckState();

private:
    void refreshText();

    QString mTextUnchecked;
    QString mTextChecked;
};

//===========================================================================

class QTOPIA_EXPORT HomeFieldButtonBase : public QAbstractButton
{
    Q_OBJECT;
public:
    enum { kLabelPadding = 16 };
    enum { kMinimumVerticalMargin = 5 };

    HomeFieldButtonBase(const QString& label, ColumnSizer& group, bool editing = false);

    void paintEvent(QPaintEvent *pe);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setLabel(const QString&);

    QString label() const {return mLabel;}

    Q_INVOKABLE virtual int columnWidth() {return fontMetrics().width(label());}

protected:
    virtual QSize labelSize() const;
    virtual QSize minimumLabelSize() const;
    virtual void drawLabel(QPainter& p, QRect r) const;

    virtual QSize fieldSize() const = 0;
    virtual QSize minimumFieldSize() const = 0;
    virtual void drawField(QPainter& p, QRect r) const = 0;

    virtual int labelPadding() const;

    bool mEditing;
    bool mShowIcon;
    ColumnSizer& mGroup;
    QString mLabel;

    static QPen mLabelPen;
    static QPen mEditPen;
    static bool mStaticInit;
};

class QTOPIA_EXPORT HomeFieldButton : public HomeFieldButtonBase
{
    Q_OBJECT;
public:
    enum { kMinimumFieldWidth = 30 };

    HomeFieldButton(const QString& label, ColumnSizer& group, bool editing = false);
    HomeFieldButton(const QString& label, const QString &field, ColumnSizer& group, bool editing = false);

    void setField(const QString&);
    void setContents(const QString&, const QString&);

    QString field() const {return mField;}

protected:
    virtual QSize fieldSize() const ;
    virtual QSize minimumFieldSize() const;
    virtual void drawField(QPainter& p, QRect r) const;

    QString mField;
};

class QTOPIA_EXPORT HomeContactButton : public HomeFieldButtonBase
{
    Q_OBJECT;

public:
    enum { kPortraitWidth = 26 };
    enum { kPortraitHeight = 26 };
    enum { kPadding = 1 };
    enum { kSpacing = 4 };
    enum { kMinimumTextWidth = 30 };
    enum { kVerticalMargin = 2 };
    enum { kLabelPadding = 8 };

    HomeContactButton(const QString &label, ColumnSizer& group, const QContact& contact = QContact(), const QString& name = QString(), const QString& subtext = QString());

    void setValues(const QContact& contact, const QString& name, const QString& subtext = QString());

protected slots:
    void contactModelReset();

protected:
    virtual QSize fieldSize() const ;
    virtual QSize minimumFieldSize() const;
    virtual void drawField(QPainter& p, QRect r) const;

    virtual int labelPadding() const;

    QContact mContact;
    QString mName;
    QString mSubtext;
    mutable QPixmap mPortrait;

    static QFont sNameFont;
    static QFont sSubtextFont;
    static QContactModel *sModel;
    static bool sStaticInit;
};

//===========================================================================

/*
    Displays a pixmap (e.g. a contact portrait) within a square with a rounded
    frame border.
*/
class QTOPIA_EXPORT FramedContactWidget : public QWidget
{
    Q_OBJECT
public:
    enum { kFrameWidth = 4 };
    enum { kFrameRoundness = 5 };

    FramedContactWidget(QWidget *parent = 0);
    FramedContactWidget(const QContact& c, QWidget *parent = 0);
    ~FramedContactWidget();

    const QIcon &icon() const;
    QContact contact() const;

    static int frameLineWidth();
    static int frameRoundness();

    static void paintImage(QPainter *p, const QRect& rect, const QPixmap &pixmap);
    static void drawFrame(QPainter *p, const QRect& rect, int frameWidth, const QColor& frameColor);

public slots:
    void setIcon(const QIcon& icon);
    void setContact(const QContact& c);

protected:
    void changeEvent(QEvent *event);
    void showEvent(QShowEvent *event);
    void paintEvent(QPaintEvent *);
    void drawFrame(QPainter *p);
    virtual void recomposePixmap();

private:
    void init();
    void updateFrameColor();

    QIcon m_icon;
    QColor m_frameColor;
    QPixmap m_composed;
    QContact m_contact;
};


//===========================================================================

//XXX Need to turn this into some kind of useful API.
class QTOPIA_EXPORT Shadow : public QWidget
{
    Q_OBJECT;
public:
    // Deliberately limited
    typedef enum {
        Right,
        Top,
        Bottom,
        RightAndBottom,
        OutsideCorner
    } ShadowType;

    Shadow(ShadowType type);
    void setType(ShadowType type);
    QSize sizeHint() const;
    void paintEvent(QPaintEvent *);

protected:
    QPixmap *outerbottomleftcorner();
    QPixmap *outerbottomrightcorner();
    QPixmap *under();
    QPixmap *over();
    QPixmap *innertopleftcorner();
    QPixmap *innertoprightcorner();
    QPixmap *left();
    QPixmap *right();

    ShadowType mType;
    QSize mSize;
};

//===========================================================================

class QTOPIA_EXPORT ContactLabelPainter
{
public:

    enum { kPadding = 10 };
    enum { kSpacing = 4 };
    enum { kIconWidth = 26 };
    enum { kIconHeight = 26 };
    enum { kMinimumVerticalMargin = 5 };


    // The item is painted like this:
    // [kPadding] [portrait pixmap @ kIconWidth x kIconHeight] [kSpacing] [name] [kSpacing] [label, elided to fit remaining space] [kPadding]

    static QSize sizeHint(const QSize& portraitSize,
                          const QString& name,
                          const QFont& nameFont,
                          const QString& label,
                          const QFont& labelFont,
                          const int padding = kPadding,
                          const int spacing = kSpacing,
                          const int verticalMargin = kMinimumVerticalMargin);

    static void paint(QPainter& p,
                      const QRect& rect,
                      const QPixmap& portrait,
                      const QSize& portraitSize,
                      const QColor& presence,
                      const QString& name,
                      const QFont& nameFont,
                      const QString& label,
                      const QFont& labelFont,
                      const int padding = kPadding,
                      const int spacing = kSpacing,
                      const int verticalMargin = kMinimumVerticalMargin);
};

#endif

