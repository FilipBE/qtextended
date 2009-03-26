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

#ifndef QPHONESTYLE_H
#define QPHONESTYLE_H

#include <qtopiastyle.h>

class QPhoneStylePrivate;

class QTOPIA_EXPORT QPhoneStyle : public QtopiaStyle
{
    Q_OBJECT
public:
    QPhoneStyle();
    virtual ~QPhoneStyle();

    using QtopiaStyle::polish; // Don't hide these symbols!
    void polish(QPalette &pal);
    void polish(QWidget *widget);
    void unpolish(QWidget *widget);
    void unpolish(QApplication *app);
    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
    QSize sizeFromContents ( ContentsType type, const QStyleOption * option, const QSize & contentsSize, const QWidget * widget = 0 ) const;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                        const QWidget *w = 0) const;
    QRect subElementRect(SubElement sr, const QStyleOption *opt,
                        const QWidget *widget) const;
    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const;
    void drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *widget) const;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                        QPainter *p, const QWidget *widget) const;
    int styleHint(StyleHint stylehint, const QStyleOption *opt = 0,
                          const QWidget *widget = 0, QStyleHintReturn* returnData = 0) const;
    void drawItemText(QPainter *painter, const QRect &rect, int alignment, const QPalette &pal,
                          bool enabled, const QString& text, QPalette::ColorRole textRole) const;

    bool event(QEvent *e);

    enum PhoneStyleHint {
        SH_FormLayoutWrapPolicy = QStyle::SH_FormLayoutWrapPolicy, // ### Defined in Qt/4.4
        SH_FormLayoutFieldGrowthPolicy = QStyle::SH_FormLayoutFieldGrowthPolicy,
        SH_FormLayoutFormAlignment = QStyle::SH_FormLayoutFormAlignment,
        SH_FormLayoutLabelAlignment = QStyle::SH_FormLayoutLabelAlignment,

        SH_ExtendedFocusHighlight = 0x10000000+1,   //future QtopiaBase + 1
        SH_PopupShadows,
        SH_HideMenuIcons,
        SH_FullWidthMenu,
        SH_ScrollbarLineStepButtons
    };

    enum {
        PE_ExtendedFocusHighlight = PE_CustomBase+1,
        PE_FilledFocusRect
    };

private:
    QPhoneStylePrivate *d;
};

#endif
