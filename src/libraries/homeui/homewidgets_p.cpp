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

#include "homewidgets_p.h"
#include <QStylePainter>
#include <QStyleOption>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QDialog>
#include <QContact>
#include <QContactModel>

#include <QDebug>

/*
   Gives a dialog the Qt Extended Home popup dialog appearance.  Requires the
   Qt Extended Home style.
*/
void QtopiaHome::setPopupDialogStyle(QDialog *dialog)
{
    dialog->setProperty("QHWindowStyle", "PopupDialog");
}

const QColor &QtopiaHome::standardColor(HomeColor col)
{
    static const QColor green(19, 109, 6);
    static const QColor red(157, 11, 30);
    static const QColor white(Qt::white);

    switch (col) {
    case Green:
        return green;
    case Red:
        return red;
    }

    return white;
}

QColor QtopiaHome::presenceColor(QCollectivePresenceInfo::PresenceType type)
{
    // Colors taken from the lowest point of relevant presence icons
    static const QColor online(0,191,0);
    static const QColor busy(255,241,24);
    static const QColor offline(149,21,17);
    static const QColor away(213,103,0);
    static const QColor none(105,105,105);

    switch(type) {
        case QCollectivePresenceInfo::None:
        default:
            return none;

        case QCollectivePresenceInfo::Hidden:
        case QCollectivePresenceInfo::Offline:
            return offline;

        case QCollectivePresenceInfo::Online:
            return online;

        case QCollectivePresenceInfo::Away:
        case QCollectivePresenceInfo::ExtendedAway:
            return away;

        case QCollectivePresenceInfo::Busy:
            return busy;
    }
}

QCollectivePresenceInfo::PresenceType QtopiaHome::chooseBestPresence(const QMap<QString, QCollectivePresenceInfo::PresenceType> &presences)
{
    // We get the URI in case we know something special about the protocol (e.g. foo doesn't handle away properly)
    // but we don't currently care.

    QCollectivePresenceInfo::PresenceType ret = QCollectivePresenceInfo::None;

    // We actually just care about the most available type at the moment
    foreach(QCollectivePresenceInfo::PresenceType type, presences.values()) {
        switch(type) {
            case QCollectivePresenceInfo::None:
            default:
                // Ignore it
                break;

            case QCollectivePresenceInfo::Online:
                return QCollectivePresenceInfo::Online; // Early out

            case QCollectivePresenceInfo::Away:
                ret = QCollectivePresenceInfo::Away;
                break;

            case QCollectivePresenceInfo::Busy:
                if (ret != QCollectivePresenceInfo::Away)
                    ret = QCollectivePresenceInfo::Busy;
                break;

            case QCollectivePresenceInfo::ExtendedAway:
                if (ret != QCollectivePresenceInfo::Busy && ret != QCollectivePresenceInfo::Away)
                    ret = QCollectivePresenceInfo::ExtendedAway;
                break;

            case QCollectivePresenceInfo::Hidden:
                if (ret != QCollectivePresenceInfo::Busy && ret != QCollectivePresenceInfo::Away && ret != QCollectivePresenceInfo::ExtendedAway)
                    ret = QCollectivePresenceInfo::Hidden;
                break;

            case QCollectivePresenceInfo::Offline:
                if (ret != QCollectivePresenceInfo::Busy && ret != QCollectivePresenceInfo::Away && ret != QCollectivePresenceInfo::ExtendedAway && ret != QCollectivePresenceInfo::Hidden)
                    ret = QCollectivePresenceInfo::Offline;
                break;

        }
    }

    return ret;
}

QCollectivePresenceInfo::PresenceType QtopiaHome::bestPresence(const QContact &contact)
{
    QPresenceTypeMap map = QContactModel::contactField(contact, QContactModel::PresenceStatus).value<QPresenceTypeMap>();
    return QtopiaHome::chooseBestPresence(map);
}

//===========================================================================

ColumnSizer::ColumnSizer()
    : mColumnWidth(-1)
{
}

int ColumnSizer::columnWidth()
{
    if (mColumnWidth == -1)
        updateFields();
    return mColumnWidth;
}

void ColumnSizer::updateFields()
{
    mColumnWidth = 0;
    // update the shared label width (using metaobjects, urgh)
    foreach(QObject *o, mFields) {
        int labelWidth = 0;
        if (QMetaObject::invokeMethod(o, "columnWidth", Qt::DirectConnection, Q_RETURN_ARG(int, labelWidth)))
            mColumnWidth = qMax(mColumnWidth, labelWidth);
    }
}

void ColumnSizer::addField(QObject* w)
{
    mFields.insert(w);
    mColumnWidth = -1;
    connect(w, SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
}

void ColumnSizer::objectDestroyed()
{
    mColumnWidth = -1;
    mFields.remove(sender());
}


HomeActionButton::HomeActionButton(const QString& text, QColor bg, QColor fg)
    : mBuddy(0)
{
    init();
    setColors(bg, fg);
    setText(text);
}

HomeActionButton::HomeActionButton(QtopiaHome::HomeColor color)
    : mBuddy(0)
{
    QPalette p = palette();
    setPaletteFromColor(&p, color);
    setPalette(p);
    init();
}

HomeActionButton::HomeActionButton(const QString& text, QtopiaHome::HomeColor color)
    : mBuddy(0)
{
    setText(text);
    QPalette p = palette();
    setPaletteFromColor(&p, color);
    setPalette(p);
    init();
}

void HomeActionButton::init()
{
    QFont f = font();
    f.setWeight(80);
    f.setPointSize((int) (f.pointSize() * 0.8f));
    setFont(f);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
}

void HomeActionButton::setBuddy(QWidget *w)
{
    mBuddy = w;
    updateGeometry();
}

QSize HomeActionButton::sizeHint() const
{
    QSize r = fontMetrics().size(0, text());
    return QSize(qMax<int>(r.width() + 8, kMinimumButtonWidth), 
                 qMax<int>(r.height() + 2 * kMinimumVerticalMargin, (mBuddy ? mBuddy->sizeHint().height() : 0)));
}

void HomeActionButton::setColors(QColor bg, QColor fg)
{
    QPalette p = palette();
    p.setColor(QPalette::Button, bg);
    p.setColor(QPalette::Text, fg);
    setPalette(p);
}

void HomeActionButton::setPaletteFromColor(QPalette *p, QtopiaHome::HomeColor color)
{
    p->setColor(QPalette::Button, QtopiaHome::standardColor(color));
    p->setColor(QPalette::Text, Qt::white);
}

void HomeActionButton::paintButton(QStyleOptionButton *option,
                                   QStylePainter *p,
                                   QRect r,
                                   QString str)
{
    p->drawPrimitive(QStyle::PE_PanelButtonTool, *option);
    p->setPen(option->palette.color(QPalette::Text));
    p->drawText(r, Qt::AlignCenter, str);
}

void HomeActionButton::paintEvent(QPaintEvent*)
{
    QStyleOptionButton option;
    option.initFrom(this);
    if (isDown())
        option.state |= QStyle::State_Sunken;

    // pressed appearance wrangling
    QPoint pressedOffset;
    if (option.state & QStyle::State_Sunken)
        pressedOffset = QPoint(style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal), style()->pixelMetric(QStyle::PM_ButtonShiftVertical));

    QRect r(kLabelPadding, 0, width() - (2 * kLabelPadding), height());
    QRect r2 = style()->visualRect(layoutDirection(),
                                   rect(),
                                   r.translated(pressedOffset));
    QStylePainter p(this);
    paintButton(&option, &p, r2, text());
}

//===========================================================================

CheckableHomeActionButton::CheckableHomeActionButton(const QString& textUnchecked, const QString& textChecked, const QColor &bgColor)
    : HomeActionButton(textUnchecked, bgColor, Qt::white),
      mTextUnchecked(textUnchecked),
      mTextChecked(textChecked)
{
    setCheckable(true);
}

void CheckableHomeActionButton::checkStateSet()
{
    HomeActionButton::checkStateSet();
    refreshText();
}

void CheckableHomeActionButton::nextCheckState()
{
    HomeActionButton::nextCheckState();
    refreshText();
}

void CheckableHomeActionButton::refreshText()
{
    setText(isChecked() ? mTextChecked : mTextUnchecked);
    update();
}


//===========================================================================

QPen HomeFieldButtonBase::mLabelPen;
QPen HomeFieldButtonBase::mEditPen;
bool HomeFieldButtonBase::mStaticInit;

HomeFieldButtonBase::HomeFieldButtonBase(const QString& label, ColumnSizer& group, bool editing)
    : QAbstractButton(),
    mEditing(editing),
    mGroup(group),
    mLabel(label)
{
    if (!mStaticInit) {
        mLabelPen = QPen(QColor(3, 101, 147));
        mEditPen = QPen(Qt::black);
        mStaticInit = true;
    }
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
    mGroup.addField(this);
}

void HomeFieldButtonBase::paintEvent(QPaintEvent*)
{
    QStylePainter p(this);
    QStyleOptionButton option;
    option.initFrom(this);
    if (isDown())
        option.state |= QStyle::State_Sunken;

    option.palette.setBrush(QPalette::Button, option.palette.brush(QPalette::Base));

    if (option.state & QStyle::State_Enabled) {
        p.drawPrimitive(QStyle::PE_PanelButtonTool, option);
    } else {
        // just draw a bland rectangle with a thin line at the bottom
        p.fillRect(rect().adjusted(0,0,0,-1), QColor(240,240,240));
        p.fillRect(QRect(0,height()-1, width(), height()), QColor(181,181,181));
    }

    // We have a label & a field
    const int padding = labelPadding();
    QRect labelRect(padding, 0, mGroup.columnWidth(), height());
    QRect fieldRect(padding, 0, width() - padding, height());

    if (!mLabel.isEmpty()) {
        fieldRect.setLeft(fieldRect.left() + mGroup.columnWidth() + padding);
    }

    if (option.state & QStyle::State_Sunken) {
        QPoint pressedOffset(style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal), style()->pixelMetric(QStyle::PM_ButtonShiftVertical));

        labelRect.translate(pressedOffset);
        fieldRect.translate(pressedOffset);
    }

    if (!mLabel.isEmpty()) {
        drawLabel(p, labelRect);
    }

    drawField(p, fieldRect);
}

void HomeFieldButtonBase::setLabel(const QString& label)
{
    mLabel = label;
    mGroup.updateFields();
    updateGeometry();
    update();
}

QSize HomeFieldButtonBase::sizeHint() const
{
    QSize labelHint = labelSize();
    QSize fieldHint = fieldSize();

    return QSize(labelHint.width() + 4 + fieldHint.width(),
                 qMax(labelHint.height(), fieldHint.height()) + 2 * kMinimumVerticalMargin);
}

QSize HomeFieldButtonBase::minimumSizeHint() const
{
    QSize labelHint = minimumLabelSize();
    QSize fieldHint = minimumFieldSize();

    return QSize(labelHint.width() + 4 + fieldHint.width(),
                 qMax(labelHint.height(), fieldHint.height()) + 2 * kMinimumVerticalMargin);
}

QSize HomeFieldButtonBase::minimumLabelSize() const
{
    int labelWidth = mGroup.columnWidth();
    if (labelWidth > 0)
        labelWidth += (2 * labelPadding());

    return QSize(labelWidth, fontMetrics().lineSpacing());
}

QSize HomeFieldButtonBase::labelSize() const
{
    return minimumLabelSize();
}

int HomeFieldButtonBase::labelPadding() const
{
    return kLabelPadding;
}

void HomeFieldButtonBase::drawLabel(QPainter &p, QRect r) const
{
    // Find the difference between the button height and the label text height
    int lineHeight = fontMetrics().lineSpacing();
    int heightDifference = (height() - 2 * kMinimumVerticalMargin) - lineHeight;

    if (heightDifference >= lineHeight) {
        // The field must be significantly taller than the label - move the label down 
        // proportionally to the height difference
        r.setTop(r.top() + kMinimumVerticalMargin + (heightDifference / (lineHeight / 2)));
    } else {
        // The field must be approximately the size of the label - centre the label
        r.setTop(r.top() + kMinimumVerticalMargin + (heightDifference / 2));
    }

    p.setPen(mEditing ? mEditPen : mLabelPen);
    p.drawText(style()->visualRect(layoutDirection(), rect(), r), Qt::AlignLeading | Qt::AlignTop, mLabel);
}

//===========================================================================

HomeFieldButton::HomeFieldButton(const QString& label, ColumnSizer& group, bool editing)
    : HomeFieldButtonBase(label, group, editing)
{
}

HomeFieldButton::HomeFieldButton(const QString& label, const QString &field, ColumnSizer& group, bool editing)
    : HomeFieldButtonBase(label, group, editing),
      mField(field)
{
}

void HomeFieldButton::setField(const QString& field)
{
    mField = field;
    updateGeometry();
    update();
}

void HomeFieldButton::setContents(const QString& label, const QString& field)
{
    mField = field;
    setLabel(label);
}

QSize HomeFieldButton::fieldSize() const
{
    return fontMetrics().size(0, mField);
}

QSize HomeFieldButton::minimumFieldSize() const
{
    // We can elide our field text.. a lot..
    return QSize(kMinimumFieldWidth, fontMetrics().lineSpacing());
}

void HomeFieldButton::drawField(QPainter& p, QRect r) const
{
    QStringList lines = mField.split(QLatin1Char('\n'));

    int heightDifference = height() - (2 * kMinimumVerticalMargin) - (fontMetrics().lineSpacing() * lines.count());
    r.setTop(r.top() + kMinimumVerticalMargin + (heightDifference / 2));

    p.setPen(mEditing ? mLabelPen : mEditPen);

    // Eliding multi line text, how annoying
    foreach (const QString &line, lines) {
        p.drawText(style()->visualRect(layoutDirection(), rect(), r), Qt::AlignLeft | Qt::AlignTop, fontMetrics().elidedText(line, Qt::ElideRight, r.width()));
        r.setTop(r.top() + fontMetrics().lineSpacing());
    }
}

//===========================================================================

QFont HomeContactButton::sNameFont;
QFont HomeContactButton::sSubtextFont;
QContactModel *HomeContactButton::sModel;
bool HomeContactButton::sStaticInit;

HomeContactButton::HomeContactButton(const QString& label, ColumnSizer& group, const QContact& contact, const QString& name, const QString& subtext)
    : HomeFieldButtonBase(label, group, false),
      mContact(contact),
      mName(name),
      mSubtext(subtext)
{
    if (!sStaticInit) {
        sNameFont = font();
        sSubtextFont = font();
        sSubtextFont.setPointSize(sSubtextFont.pointSize() - 2);
        sModel = new QContactModel;
        sStaticInit = true;
    }

    connect(sModel, SIGNAL(modelReset()), this, SLOT(contactModelReset()));
}

void HomeContactButton::setValues(const QContact& contact, const QString& name, const QString& subtext)
{
    mContact = contact;
    mName = name;
    mSubtext = subtext;
    mPortrait = QPixmap();
    update();
}

int HomeContactButton::labelPadding() const
{
    return kLabelPadding;
}

QSize HomeContactButton::fieldSize() const
{
    return ContactLabelPainter::sizeHint(mPortrait.size(), mName, sNameFont, mSubtext, sSubtextFont, kPadding, kSpacing, kVerticalMargin);
}

QSize HomeContactButton::minimumFieldSize() const
{
    // Find size with no text
    QSize hint(ContactLabelPainter::sizeHint(mPortrait.size(), QString(), sNameFont, QString(), sSubtextFont, kPadding, kSpacing, kVerticalMargin));

    // Allow for a minimum amount of text
    return QSize(hint.width() + kMinimumTextWidth, hint.height());
}

void HomeContactButton::drawField(QPainter& p, QRect r) const
{
    if (mPortrait.isNull()) {
        mPortrait = mContact.portrait().scaled(kPortraitWidth, kPortraitHeight);
    }

    QColor presenceColor = QtopiaHome::presenceColor(QtopiaHome::bestPresence(mContact));

    ContactLabelPainter::paint(p, r, mPortrait, mPortrait.size(), presenceColor, mName, sNameFont, mSubtext, sSubtextFont, kPadding, kSpacing, kVerticalMargin);
}

void HomeContactButton::contactModelReset()
{
    repaint();
}

//===========================================================================

FramedContactWidget::FramedContactWidget(QWidget *parent)
    : QWidget(parent),
    m_frameColor(0,0,0)
{
    init();
}

FramedContactWidget::FramedContactWidget(const QContact &contact, QWidget *parent)
    : QWidget(parent),
      m_icon(contact.icon()),
      m_contact(contact)
{
    updateFrameColor();
    init();
}

FramedContactWidget::~FramedContactWidget()
{
}

void FramedContactWidget::setIcon(const QIcon &icon)
{
    m_icon = icon;
    if (isVisible()) {
        recomposePixmap();
        update();
    }
}

const QIcon &FramedContactWidget::icon() const
{
    return m_icon;
}

void FramedContactWidget::setContact(const QContact& contact)
{
    m_contact = contact;
    m_icon = contact.icon();

    updateFrameColor();
    if (isVisible()) {
        recomposePixmap();
        update();
    }
}

void FramedContactWidget::updateFrameColor()
{
    QPresenceTypeMap map = QContactModel::contactField(m_contact, QContactModel::PresenceStatus).value<QPresenceTypeMap>();
    m_frameColor = QtopiaHome::presenceColor(QtopiaHome::chooseBestPresence(map));
}

QContact FramedContactWidget::contact() const
{
    return m_contact;
}

void FramedContactWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::EnabledChange && isVisible()) {
        recomposePixmap();
        update();
    }
}

void FramedContactWidget::showEvent(QShowEvent *event)
{
    int sideLength = -1;
    QSizePolicy policy = sizePolicy();
    if (policy.horizontalPolicy() & QSizePolicy::GrowFlag
            && policy.verticalPolicy() & QSizePolicy::GrowFlag) {
        sideLength = qMax(width(), height());
    } else if (policy.horizontalPolicy() & QSizePolicy::ShrinkFlag
            && policy.verticalPolicy() & QSizePolicy::ShrinkFlag) {
        sideLength = qMin(width(), height());
    }

    if (sideLength > 0) {
        setMinimumSize(sideLength, sideLength);
        setMaximumSize(sideLength, sideLength);
    }

    recomposePixmap();
    QWidget::showEvent(event);
}

void FramedContactWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.drawPixmap((width()-m_composed.width())/2, (height()-m_composed.height())/2, m_composed);
}

void FramedContactWidget::paintImage(QPainter* p, const QRect& rect, const QPixmap& pixmap)
{
    QSize srcSize = pixmap.size();
    QRect sourceRect;

    if (srcSize.width() > srcSize.height())
        sourceRect = QRect((srcSize.width() - srcSize.height()) / 2,0, srcSize.height(), srcSize.height());
    else
        sourceRect = QRect(0, (srcSize.height() - srcSize.width()) / 2, srcSize.width(), srcSize.width());

    // Now assume that we actually have something to paint...
    if (srcSize.width() > 0 && srcSize.height() > 0) {
        // Create a clippath for the portrait
        QPainterPath pp;
        pp.addRoundedRect(rect, kFrameRoundness, kFrameRoundness);

        // And draw
        p->setRenderHint(QPainter::SmoothPixmapTransform, true);
        p->setRenderHint(QPainter::Antialiasing, true);
        p->setClipPath(pp);
        if (pixmap.hasAlphaChannel()) {
            QLinearGradient lg(0,0,0, rect.height());
            QColor bgcol(45, 97, 141); // ### arbitrary
            lg.setColorAt(0, bgcol.lighter(140));
            lg.setColorAt(1, bgcol.darker(140));
            p->fillRect(rect, lg);
        }

        p->drawPixmap(rect, pixmap, sourceRect);

        // and clear the clip
        p->setClipPath(pp, Qt::NoClip);
    }
}

void FramedContactWidget::recomposePixmap()
{
    // We enlarge the pixmap so the short edge is
    // equal to adjustedRect
    // and then crop the center

    QRect adjustedRect = rect().adjusted(kFrameWidth, kFrameWidth, -kFrameWidth, -kFrameWidth);

    // New pixmap.
    m_composed = QPixmap(size());
    m_composed.fill(Qt::transparent);

    QPainter ip(&m_composed);
    paintImage(&ip, adjustedRect, m_icon.pixmap(adjustedRect.size()));

    // darken image if widget is disabled
    if (!isEnabled())
        ip.fillRect(adjustedRect, QColor(0, 0, 0, 100));

    drawFrame(&ip, adjustedRect, kFrameWidth, m_frameColor);
}

void FramedContactWidget::drawFrame(QPainter *p, const QRect& rect, int frameWidth, const QColor& frameColor)
{
    p->setRenderHint(QPainter::Antialiasing);
    QPen pen = p->pen();
    pen.setWidth(frameWidth);
    pen.setColor(frameColor);
    p->setPen(pen);

    p->setBrush(Qt::NoBrush);
    p->drawRoundedRect(rect, kFrameRoundness, kFrameRoundness);
}

void FramedContactWidget::drawFrame(QPainter *p)
{
    p->save();
    QRect adjustedRect = rect().adjusted(kFrameWidth, kFrameWidth, -kFrameWidth, -kFrameWidth);
    drawFrame(p, adjustedRect, kFrameWidth, m_frameColor);
    p->restore();
}

void FramedContactWidget::init()
{
    QPalette pal(palette());
    pal.setBrush(QPalette::Window, Qt::transparent);
    setAttribute(Qt::WA_NoSystemBackground);
    setPalette(pal);
}

int FramedContactWidget::frameLineWidth()
{
    return kFrameWidth;
}

int FramedContactWidget::frameRoundness()
{
    return kFrameRoundness;
}

//===========================================================================

Shadow::Shadow(ShadowType type) : QWidget()
{
    // Hmm.. workaround for setType check of mType
    mType = (type == Right) ? Top : Right;
    setType(type);
}

void Shadow::setType(ShadowType type)
{
    if (mType != type) {
        mType = type;
        switch (type) {
            case Right:
                setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));
                setMaximumHeight(QWIDGETSIZE_MAX);
                setMaximumWidth(4);
                mSize = QSize(4, 16);
                break;
            case Top:
                setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
                setMaximumHeight(3);
                setMaximumWidth(QWIDGETSIZE_MAX);
                mSize = QSize(16, 3);
                break;
            case Bottom:
                setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
                setMaximumHeight(12);
                setMaximumWidth(QWIDGETSIZE_MAX);
                mSize = QSize(16, 12);
                break;
            case RightAndBottom:
                setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
                mSize = QSize(16, 12);
                setMaximumHeight(QWIDGETSIZE_MAX);
                setMaximumWidth(QWIDGETSIZE_MAX);
                break;
            case OutsideCorner:
                setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
                setMaximumWidth(4);
                setMaximumHeight(QWIDGETSIZE_MAX);
                mSize = QSize(4, 12);
                break;
        }
        updateGeometry();
    }
}

QSize Shadow::sizeHint() const
{
    return mSize;
}

void Shadow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    bool rtl = layoutDirection() == Qt::RightToLeft;

    switch (mType) {
        case Right:
            p.drawTiledPixmap(QRect(rtl ? width() - 4 : 0, 0, rtl ? width() : 4, height()), rtl ? *left() : *right());
            break;

        case Top:
            p.drawTiledPixmap(QRect(0, height() - 3, width(), height()), *over());
            break;

        case Bottom:
            p.drawTiledPixmap(QRect(0, 0, width(), 12), *under());
            break;

        case RightAndBottom:
            // First the corner
            p.drawPixmap(QPoint(rtl ? width() - 4 : 0, 0), rtl ? *innertoprightcorner() : *innertopleftcorner());
            // then the top
            p.drawTiledPixmap(QRect(rtl ? 0 : 4, 0, width() - 4, 12), *under());
            // then the side
            p.drawTiledPixmap(QRect(rtl ? width() - 4 : 0, 12, 4, height() - 12), rtl ? *left() : *right());
            break;

        case OutsideCorner:
            p.drawPixmap(QPoint(rtl ? width() - 4 : 0, 0), rtl ? *outerbottomleftcorner() : *outerbottomrightcorner());
            break;

    }
}

QPixmap *Shadow::outerbottomleftcorner()
{
    static QPixmap p(":image/shadow_outerbl");
    return &p;
}

QPixmap *Shadow::outerbottomrightcorner()
{
    static QPixmap p(":image/shadow_outerbr");
    return &p;
}

QPixmap *Shadow::under()
{
    static QPixmap p(":image/shadow_under");
    return &p;
}

QPixmap *Shadow::over()
{
    static QPixmap p(":image/shadow_over");
    return &p;
}

QPixmap *Shadow::innertopleftcorner()
{
    static QPixmap p(":image/shadow_innertl");
    return &p;
}

QPixmap *Shadow::innertoprightcorner()
{
    static QPixmap p(":image/shadow_innertr");
    return &p;
}

QPixmap *Shadow::left()
{
    static QPixmap p(":image/shadow_left");
    return &p;
}

QPixmap *Shadow::right()
{
    static QPixmap p(":image/shadow_right");
    return &p;
}


QSize ContactLabelPainter::sizeHint(const QSize& portraitSize,
                                    const QString& name,
                                    const QFont& nameFont,
                                    const QString& label,
                                    const QFont& labelFont,
                                    const int padding,
                                    const int spacing,
                                    const int verticalMargin)
{
    QFontMetrics nfm(nameFont);
    QFontMetrics lfm(labelFont);

    int height = qMax(portraitSize.height(), qMax(nfm.height(), lfm.height())) + 2 * verticalMargin;
    int width = portraitSize.width() + 2 * (padding + spacing);

    if (!name.isEmpty())
        width += nfm.size(0, name).width();
    if (!label.isEmpty())
        width += lfm.size(0, label).width();

    return QSize(width, height);
}

void ContactLabelPainter::paint(QPainter& p,
                                const QRect& rect,
                                const QPixmap& portrait,
                                const QSize& portraitSize,
                                const QColor& presence,
                                const QString& name,
                                const QFont& nameFont,
                                const QString& label,
                                const QFont& labelFont,
                                const int padding,
                                const int spacing,
                                const int verticalMargin)
{
    Q_UNUSED(verticalMargin)

    Qt::LayoutDirection layoutDirection = p.layoutDirection();
    bool rtl = layoutDirection == Qt::RightToLeft;

    QFontMetrics nfm(nameFont);
    QFontMetrics lfm(labelFont);
    QRect portraitRect =
#ifdef QTOPIA_HOMEUI_WIDE
    QRect (rect.right() - padding - portraitSize.width(), rect.top() + (rect.height() - portraitSize.height()) / 2, portraitSize.width(), portraitSize.height());
#else
    QRect (rect.left() + padding, rect.top() + (rect.height() -portraitSize.height()) / 2, portraitSize.width(), portraitSize.height());
#endif
    // Portrait/Message type Icon
    QRect actualPortraitRect = QStyle::visualRect(layoutDirection, rect, portraitRect).adjusted(1,1,-2,-2);
    FramedContactWidget::paintImage(&p, actualPortraitRect, portrait);
    FramedContactWidget::drawFrame(&p, actualPortraitRect, 2, presence); // We use a smaller frame for this

    // Find the rect for the two text elements
    int textWidth = rect.width() - portraitRect.width() - spacing - (2 * + padding);

    // Vertically center the larger font, and align both by baseline
    int maxTextHeight = qMax(nfm.height() - nfm.descent(), lfm.height() - lfm.descent());
    int maxTextDescent = qMax(nfm.descent(), lfm.descent());
    int textMargin = (rect.height() - maxTextHeight) / 2;
    QRect textRect =
#ifdef QTOPIA_HOMEUI_WIDE
    QRect (spacing, rect.top() + textMargin, textWidth, maxTextHeight + maxTextDescent);
#else
    QRect (portraitRect.right() + spacing, rect.top() + textMargin, textWidth, maxTextHeight + maxTextDescent);
#endif
    Qt::TextElideMode elideMode(rtl ? Qt::ElideLeft : Qt::ElideRight);

    // Name
    QRect nameRect(textRect);
    nameRect.setBottom(nameRect.bottom() - (maxTextDescent - nfm.descent()));

    p.setFont(nameFont);
    p.setPen(
#ifdef QTOPIA_HOMEUI_WIDE
        Qt::white
#else
        Qt::black
#endif
    );
    p.drawText(QStyle::visualRect(layoutDirection, rect, nameRect), Qt::AlignLeading | Qt::AlignBottom, nfm.elidedText(name, elideMode, nameRect.width()), &nameRect);

    // Label - use the remaining textRect after painting the name, and adding spacing
    QRect labelRect(textRect);
    labelRect.setBottom(labelRect.bottom() - (maxTextDescent - lfm.descent()));
    labelRect.setLeft(nameRect.right() + spacing);

    p.setFont(labelFont);
    p.drawText(QStyle::visualRect(layoutDirection, rect, labelRect), Qt::AlignTrailing | Qt::AlignBottom, lfm.elidedText(label, elideMode, labelRect.width()));
}

#include "homewidgets_p.moc"

