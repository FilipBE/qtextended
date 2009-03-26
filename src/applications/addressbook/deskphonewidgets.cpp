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

#include "deskphonewidgets.h"
#include "qcontactmodel.h"
#include "contactdetails.h"
#include <QMouseEvent>
#include <QCollectivePresence>
#include <QCommServiceManager>


/*
    Contact Header - similar to a groupbox, has a contained
    list widget etc.  The header is the name/company and portrait
    and presence information
*/
ContactHeader::ContactHeader(QWidget *p)
    : QAbstractButton(p)
{
    // we need a contact to figure out the size stuff, so default it
    mHeaderSize = QSize(100, 33);

    mNameFont = font();
    mNameFont.setWeight(80);
    mNameFont.setPointSize((mNameFont.pointSize() * 5) / 4);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ContactHeader::setWidget(QWidget *w)
{
    mChild = w;
    w->setParent(this);
    resizeChild();
    w->show();
}

void ContactHeader::init(QContact c)
{
    mName = c.label();
    mCompany = c.company();
    mContact = c;

    if (mName == mCompany)
        mCompany.clear();

    QSize psize = QSize(ContactLabelPainter::kIconWidth, ContactLabelPainter::kIconHeight);
    mPixmap = c.icon().pixmap(psize);

    // Now we can update our size information
    mHeaderSize = ContactLabelPainter::sizeHint(psize, mName, mNameFont, mCompany, font());
}

void ContactHeader::resizeEvent(QResizeEvent *re)
{
    resizeChild();
    QAbstractButton::resizeEvent(re);
}

void ContactHeader::paintEvent(QPaintEvent*)
{
    // Paint the header
    // Icon on the left (vcenter), Name in the middle (left+vcenter aligned, bigger font), Company on the right (right+baseline aligned)
    QStylePainter p(this);

    QStyleOptionButton option;
    option.initFrom(this);
    option.rect = QRect(0, 0, width(), mHeaderSize.height());
    if (isDown())
        option.state |= QStyle::State_Sunken;

    option.palette.setBrush(QPalette::Button, option.palette.brush(QPalette::Base));
    p.drawPrimitive(QStyle::PE_PanelButtonTool, option);

    // pressed appearance wrangling
    QPoint pressedOffset;
    if (option.state & QStyle::State_Sunken)
        pressedOffset = QPoint(style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal), style()->pixelMetric(QStyle::PM_ButtonShiftVertical));

    QPresenceTypeMap map = QContactModel::contactField(mContact, QContactModel::PresenceStatus).value<QPresenceTypeMap>();
    QColor presence = QtopiaHome::presenceColor(QtopiaHome::chooseBestPresence(map));
    ContactLabelPainter::paint(p, option.rect.translated(pressedOffset), mPixmap, QSize(ContactLabelPainter::kIconWidth, ContactLabelPainter::kIconHeight), presence, mName, mNameFont, mCompany, font());

    // a shadow underneath
    p.drawTiledPixmap(QRect(0, mHeaderSize.height(), width(), 3), *shadow());
}

QSize ContactHeader::sizeHint() const
{
    QSize size;
    if (mChild)
        size = mChild->sizeHint();
    size.rheight() += mHeaderSize.height() + 3;
    size.rwidth() = qMax(mHeaderSize.width(), size.width());

    return size;
}

void ContactHeader::resizeChild()
{
    if (mChild)
        mChild->setGeometry(QRect(0, mHeaderSize.height() + 3, width(), height() - (mHeaderSize.height() + 3)));
}

/* ============================================================ */

// Buddy Field things
void ContactBuddyFieldWidget::updateLabel(const QContact& contact)
{
    QString field = definition().value(contact).toString();
    QString provider = definition().provider();

    QCommServiceManager m;

    QSet<QString> providers = m.supports<QCollectivePresence>().toSet();

    bool updated = false;

    foreach (QString service, providers) {
        if (service == provider) {
            QCollectivePresence p(service);
            QCollectivePresenceInfo info = p.peerInfo(field);
            QString value = field;
            if (!mEditing) {
                if (!info.displayName().isEmpty())
                    value += "\n" + info.displayName();
                if (p.pendingPeerSubscriptions().contains(field)) {
                    value += "\n" + tr("Waiting for authorization");
                } else {
                    if (!info.presence().isEmpty() && !info.message().isEmpty())
                        value += "\n" + tr("%1 (%2)", "Away (Feeding the Dog)").arg(info.presence()).arg(info.message());
                    else if (!info.presence().isEmpty())
                        value += "\n" + info.presence();
                    else if (!info.message().isEmpty())
                        value += "\n" + info.message();
                }
            }
            setContents(definition().label(), value);

            // May as well load the avatar here
            if (!mEditing) {
                QImage avatar(info.avatar());
                if (!avatar.isNull())
                    mAvatar = QPixmap::fromImage(avatar.scaled(kAvatarWidth, kAvatarHeight, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                else
                    mAvatar = QPixmap();
            } else
                mAvatar = QPixmap();
            updated = true;
            break;
        }
    }

    if (!updated) {
        mAvatar = QPixmap();
        setContents(definition().label(), field);
    }
}

QSize ContactBuddyFieldWidget::minimumFieldSize() const
{
    QSize r = ContactDefinedFieldWidget::minimumFieldSize();

    // Add our avatar, if we have one
    if (!mEditing && !mAvatar.isNull() && r.height() < kAvatarHeight)
        r.rheight() = kAvatarHeight;

    return r;
}

void ContactBuddyFieldWidget::drawField(QPainter&p, QRect r) const
{
    QStringList lines = mField.split(QLatin1Char('\n'));

    int heightDifference = height() - (2 * kMinimumVerticalMargin) - (fontMetrics().lineSpacing() * lines.count());
    r.setTop(r.top() + kMinimumVerticalMargin + (heightDifference / 2));

    p.setPen(mEditing ? mLabelPen : mEditPen);

    // Fit the avatar in, if we have one
    if (!mEditing && !mAvatar.isNull()) {
        r.setRight(r.right() - kAvatarWidth - kMinimumVerticalMargin); // use the same margin for h as v
        p.drawPixmap(r.topRight(), mAvatar);
    }

    // We draw the first line in the normal color, and the rest in a lighter color
    bool setPen = false;
    QPen oldPen = p.pen();

    foreach (const QString &line, lines) {
        p.drawText(style()->visualRect(layoutDirection(), rect(), r), Qt::AlignLeft | Qt::AlignTop, fontMetrics().elidedText(line, Qt::ElideRight, r.width()));
        r.setTop(r.top() + fontMetrics().lineSpacing());
        if (!setPen) {
            p.setPen(mEditing ? mEditPen : mLabelPen);
            setPen = true;
        }
    }
    p.setPen(oldPen);
}

/* ============================================================ */

// Delegate for contact list view
DeskphoneContactDelegate::DeskphoneContactDelegate(QWidget *parent)
    : QAbstractItemDelegate(parent), mParent(parent)
{
    QFont f = parent->font();

    mNameFont = f;
    mNameFont.setWeight(80);
}

void DeskphoneContactDelegate::getInfo(const QModelIndex& index, QPixmap& pm, QColor& frameColor, QString& mainText, QString& subText) const
{
    const QAbstractItemModel *m = index.model();
    if (m) {
        mainText = m->data(index, Qt::DisplayRole).toString();
        subText = m->data(index.sibling(index.row(), QContactModel::Company), Qt::DisplayRole).toString();
        QIcon i = qvariant_cast<QIcon>(m->data(index, Qt::DecorationRole));

        pm = i.pixmap(ContactLabelPainter::kIconWidth, ContactLabelPainter::kIconHeight);
        QPresenceTypeMap map = m->data(index.sibling(index.row(), QContactModel::PresenceStatus), Qt::DisplayRole).value<QPresenceTypeMap>();
        frameColor = QtopiaHome::presenceColor(QtopiaHome::chooseBestPresence(map));
    } else {
        mainText.clear();
        subText.clear();
        pm = QPixmap();
        frameColor = Qt::transparent;
    }
}

QSize DeskphoneContactDelegate::sizeHint (const QStyleOptionViewItem& , const QModelIndex& index) const
{
    QString name;
    QString company;
    QPixmap pm;
    QColor presence;

    getInfo(index, pm, presence, name, company);

    QSize infoSize = ContactLabelPainter::sizeHint(QSize(ContactLabelPainter::kIconWidth, ContactLabelPainter::kIconHeight), name, mNameFont, company, mParent->font());

    return infoSize;
}

void DeskphoneContactDelegate::paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QString name;
    QString company;
    QPixmap pm;
    QColor presence;

    getInfo(index, pm, presence, name, company);

    QStyleOptionButton boption;
    boption.initFrom(mParent);
    boption.state = option.state;
    boption.rect = option.rect;
    boption.palette.setBrush(QPalette::Button, boption.palette.brush(QPalette::Base));

    QPoint pressedOffset;
    if (option.state & QStyle::State_Sunken)
        pressedOffset = QPoint(mParent->style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal), mParent->style()->pixelMetric(QStyle::PM_ButtonShiftVertical));
#ifdef QTOPIA_HOMEUI_WIDE
    QLinearGradient lg (0,0,0,option.rect.height());
    lg.setColorAt(0,QColor(0x80306690));
    lg.setColorAt(0.5,QColor(0x80303034));
    lg.setColorAt(1,QColor(0x80231f20));
    painter->fillRect(option.rect,QBrush(lg));
    QPen p = painter->pen ();
    if (option.state & QStyle::State_Selected)
        painter->setPen(QPen(QColor(0xf7941d)));
#else
      mParent->style()->drawPrimitive(QStyle::PE_PanelButtonTool, &boption, painter, mParent);
#endif

    ContactLabelPainter::paint(*painter, option.rect.translated(pressedOffset), pm, QSize(ContactLabelPainter::kIconWidth, ContactLabelPainter::kIconHeight), presence, name, mNameFont, company, mParent->font());
}

/* ============================================================ */

// Delegate for contact history view
void ContactHistoryDelegate::getInfo(const QModelIndex& index, QPixmap& pm, QColor& frameColor, QString& mainText, QString& subText) const
{
    const QAbstractItemModel *m = index.model();
    if (m) {
        mainText = m->data(index, Qt::DisplayRole).toString();
        subText = m->data(index, ContactHistoryDelegate::SubLabelRole).toString();
        QIcon i = qvariant_cast<QIcon>(m->data(index, Qt::DecorationRole));

        pm = i.pixmap(ContactLabelPainter::kIconWidth, ContactLabelPainter::kIconHeight);
        QPresenceTypeMap map = m->data(index.sibling(index.row(), QContactModel::PresenceStatus), Qt::DisplayRole).value<QPresenceTypeMap>();
        frameColor = QtopiaHome::presenceColor(QtopiaHome::chooseBestPresence(map));
    } else {
        mainText.clear();
        subText.clear();
        pm = QPixmap();
        frameColor = Qt::transparent;
    }
}

/* ============================================================ */

QStringList ContactMiscFieldWidget::typeLabels()
{
    static QStringList labels(QStringList() << tr("Birthday") << tr("Anniversary") << tr("Spouse") << tr("Children") << tr("Webpage") << tr("Gender"));
    return labels;
}

QString ContactMiscFieldWidget::typeLabel(MiscType type)
{
    switch(type) {
        case Birthday: return tr("Birthday");
        case Anniversary: return tr("Anniversary");
        case Spouse: return tr("Spouse");
        case Children: return tr("Children");
        case Webpage: return tr("Webpage");
        case Gender: return tr("Gender");
        default: return QString();
    }
}

ContactMiscFieldWidget::MiscType ContactMiscFieldWidget::type(QString type)
{
    switch(typeLabels().indexOf(type)) {
        case 0: return Birthday;
        case 1: return Anniversary;
        case 2: return Spouse;
        case 3: return Children;
        case 4: return Webpage;
        case 5: return Gender;
        default: return Invalid;
    }
}

/* ============================================================ */

/* button for the "choose a portrait" button. */
ContactPortraitButton::ContactPortraitButton()
{

}

void ContactPortraitButton::paintEvent(QPaintEvent *)
{
    QStylePainter p(this);

    QStyleOptionButton option;
    option.initFrom(this);
    if (isDown())
        option.state |= QStyle::State_Sunken;

    p.drawControl(QStyle::CE_PushButtonBevel, option);

    // pressed appearance wrangling
    QPoint pressedOffset;
    if (option.state & QStyle::State_Sunken)
        pressedOffset = QPoint(style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal), style()->pixelMetric(QStyle::PM_ButtonShiftVertical));

    // ### cache!
    QRect iconRect = QStyle::alignedRect(layoutDirection(), Qt::AlignCenter, QSize(48,48), rect());
    FramedContactWidget::paintImage(&p, iconRect, icon().pixmap(QSize(48,48)));
}

QSize ContactPortraitButton::sizeHint() const
{
    return QSize(62,62);
}

