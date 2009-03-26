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
#include "albumselector.h"
#include "albumribbon.h"
#include "photogallery.h"
#include "qsmoothiconview.h"

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>
#include <QListView>
#include <QScrollBar>
#include <QBoxLayout>
#include <QtopiaItemDelegate>
#include <QSmoothList>
#include <QStackedWidget>

class AlbumIconDelegate : public QAbstractItemDelegate
{
public:
    AlbumIconDelegate(QObject *parent = 0);

    void paint(
            QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    void fillTab(QPainter *painter, const QRect &rect, int radius, const QBrush &brush) const;

    mutable QSize m_sizeHint;
};

AlbumIconDelegate::AlbumIconDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

void AlbumIconDelegate::paint(
        QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const int buttonMargin = QApplication::style()->pixelMetric(QStyle::PM_ButtonMargin, &option);

    painter->save();
    painter->translate(option.rect.topLeft());

    QSize tabSize(option.fontMetrics.height(), option.decorationSize.height());

    QRect tabRect(QPoint(0, 0), tabSize);

    fillTab(painter, tabRect, buttonMargin, option.palette.button());

    QRect decorationRect(QPoint(0, 0), option.decorationSize);
    decorationRect.translate(tabSize.width(), 0);

    QRegion decorationRegion(decorationRect);

    QImage thumbnail = qvariant_cast<QImage>(index.data(Qt::DecorationRole));

    if (!thumbnail.isNull()) {
        painter->save();

        QPoint center = decorationRect.center();

        QSize thumbnailSize = thumbnail.size();

        thumbnailSize.scale(option.decorationSize, Qt::KeepAspectRatioByExpanding);
        decorationRect.setSize(thumbnailSize);
        decorationRect.moveCenter(center);

        painter->setClipRegion(decorationRegion);
        painter->setRenderHint(QPainter::Antialiasing, true);

        painter->drawImage(decorationRect, thumbnail);

        decorationRegion -= decorationRect;

        painter->restore();
    }

    foreach (QRect rect, decorationRegion.rects())
        painter->fillRect(decorationRect, option.palette.button());

    QString text = index.data(Qt::DisplayRole).toString();
    text = option.fontMetrics.elidedText(text, option.textElideMode, tabSize.height() - 2 * buttonMargin);

    QRect textRect(-tabSize.height() + buttonMargin, 0,
                    tabSize.height() - buttonMargin, tabSize.width());

    painter->setPen(option.palette.color(QPalette::ButtonText));
    painter->rotate(-90);
    painter->drawText(textRect, text);
    painter->restore();
}

QSize AlbumIconDelegate::sizeHint(
        const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

    if (!m_sizeHint.isValid()) {
        m_sizeHint = option.decorationSize;

        m_sizeHint.rwidth() += option.fontMetrics.height();
    }

    return m_sizeHint;
}

void AlbumIconDelegate::fillTab(
        QPainter *painter, const QRect &rect, int radius, const QBrush &brush) const
{
    int x1 = rect.x();
    int y1 = rect.y();
    int x2 = x1 + rect.width();
    int y2 = y1 + rect.height();

    QPainterPath path;

    path.moveTo(x2, y2);
    path.lineTo(x2, y1);
    path.arcTo(x1, y1, radius, radius, 90, 90);
    path.arcTo(x1, y2 - radius, radius, radius, 180, 90);
    path.closeSubpath();

    painter->fillPath(path, brush);
}

class AlbumListDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    AlbumListDelegate(QObject *parent = 0);

    void paint(
            QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    mutable QSize m_sizeHint;
};

AlbumListDelegate::AlbumListDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

void AlbumListDelegate::paint(
        QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    const int hMargin = QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent, &option);
    const int buttonMargin = QApplication::style()->pixelMetric(QStyle::PM_ButtonMargin, &option);

    QFont nameFont = option.font;
    nameFont.setBold(true);

    QFontMetrics nameFontMetrics(nameFont);

    if (option.state & QStyle::State_Selected) {
        QRect backgroundRect = option.rect;
        backgroundRect.adjust(0, -nameFontMetrics.lineWidth(), 0, nameFontMetrics.lineWidth());

        painter->fillRect(backgroundRect, option.palette.highlight());
    } else {
        QRect lineRect = option.rect;
        lineRect.translate(0, nameFontMetrics.lineWidth() - nameFontMetrics.lineWidth() / 2);

        painter->setPen(QPen(option.palette.text(), nameFontMetrics.lineWidth(), Qt::DashLine));
        painter->drawLine(lineRect.bottomLeft(), lineRect.bottomRight());
    }

    QRect decorationRect = option.rect;
    decorationRect.setWidth(option.decorationSize.width() + 2 * buttonMargin);

    QImage thumbnail = qvariant_cast<QImage>(index.data(Qt::DecorationRole));

    if (!thumbnail.isNull()) {
        QSize thumbnailSize = thumbnail.size();
        thumbnailSize.scale(option.decorationSize, Qt::KeepAspectRatio);

        QRect thumbnailRect = decorationRect;
        thumbnailRect.setSize(thumbnailSize);
        thumbnailRect.moveCenter(decorationRect.center());

        painter->setRenderHint(QPainter::Antialiasing, true);

        painter->drawImage(thumbnailRect, thumbnail);
    }

    QString picsText = tr("%n pic(s)", "", index.data(AlbumModel::CountRole).toInt());

    QFont picsFont = option.font;
    picsFont.setItalic(true);

    QSize picsSize = QFontMetrics(picsFont).size(0, picsText);
    QRect picsRect(
            option.rect.right() - picsSize.width() - hMargin,
            option.rect.top(),
            picsSize.width(),
            option.rect.height());

    painter->setFont(picsFont);
    painter->setPen(option.palette.color(QPalette::BrightText));
    painter->drawText(picsRect, Qt::AlignLeft | Qt::AlignVCenter, picsText);

    QRect nameRect = option.rect;
    nameRect.adjust(decorationRect.width(), 0, -picsRect.width() - hMargin, 0);

    QString nameText = nameFontMetrics.elidedText(
            index.data(Qt::DisplayRole).toString(), option.textElideMode, nameRect.width());

    painter->setFont(nameFont);
    painter->setPen(option.palette.color(QPalette::Text));
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);

    painter->restore();
}

QSize AlbumListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

    if (!m_sizeHint.isValid()) {
        const int hMargin = QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent, &option);
        const int buttonMargin = QApplication::style()->pixelMetric(QStyle::PM_ButtonMargin, &option);

        m_sizeHint = option.decorationSize;
        m_sizeHint.rheight() += buttonMargin * 2;
        m_sizeHint.rwidth() += buttonMargin * 2;

        QFont font = option.font;
        font.setBold(true);

        QSize nameSize = QFontMetrics(font).size(0, QLatin1String("XXXX"));
        m_sizeHint.rwidth() += nameSize.width() + hMargin;

        font = option.font;
        font.setItalic(true);

        QSize picsSize = QFontMetrics(font).size(0, tr("0 pics"));
        m_sizeHint.rwidth() += picsSize.width() + hMargin;
    }
    return m_sizeHint;
}

AlbumSelector::AlbumSelector(AlbumModel::SortMode sort, QWidget *parent)
    : QWidget(parent)
    , m_iconView(0)
    , m_listView(0)
{
    m_albumModel = new AlbumModel(parent);

    connect(m_albumModel, SIGNAL(yearRangeChanged(int,int)), this, SLOT(yearRangeChanged(int,int)));

    m_ribbon = new AlbumRibbon;
    m_ribbon->setAutoFillBackground(true);

    QLinearGradient gradient(0.0, 0.0, 0.0, 1.0);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient.setColorAt(0.0, Qt::white);
    gradient.setColorAt(0.5, PhotoGallery::lightGrey.darker(110));
    gradient.setColorAt(1.0, PhotoGallery::lightGrey);

    QPalette palette = m_ribbon->palette();
    palette.setBrush(QPalette::Window, QBrush(gradient));
    palette.setColor(QPalette::WindowText, PhotoGallery::blue);
    palette.setColor(QPalette::BrightText, PhotoGallery::orange);
    m_ribbon->setPalette(palette);

    QFont font = m_ribbon->font();
    font.setBold(true);
    m_ribbon->setFont(font);

    connect(m_ribbon, SIGNAL(valueChanged(int)),
            this, SLOT(ribbonValueChanged(int)));

    m_stack = new QStackedWidget;

    QBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_ribbon);
    layout->addWidget(m_stack);
    setLayout(layout);

    switch (sort) {
    case AlbumModel::SortByDate:
        sortByDate();
        break;
    case AlbumModel::SortByName:
        sortByName();
        break;
    }
}

AlbumSelector::~AlbumSelector()
{
}

AlbumModel::SortMode AlbumSelector::sortMode() const
{
    return m_albumModel->sortMode();
}

void AlbumSelector::setSortMode(AlbumModel::SortMode mode)
{
    if (mode != m_albumModel->sortMode()) {
        switch (mode) {
        case AlbumModel::SortByDate:
            sortByDate();
            break;
        case AlbumModel::SortByName:
            sortByName();
            break;
        }
    }
}

void AlbumSelector::albumSelected(const QModelIndex &index)
{
    emit albumSelected(
            index.data(Qt::DisplayRole).toString(),
            index.data(AlbumModel::CategoryIdRole).toString());
}

void AlbumSelector::sortByName()
{
    if (!m_listView) {
        int iconSize = style()->pixelMetric(QStyle::PM_ListViewIconSize);

        m_listView = new QSmoothList;
        m_listView->setModel(m_albumModel);
        m_listView->setItemDelegate(new AlbumListDelegate(m_listView));
        m_listView->setIconSize(QSize((iconSize * 4) / 3, iconSize));
        m_listView->setSelectionMode(QSmoothList::SingleSelection);

        QLinearGradient gradient(0.0, 0.0, 0.0, 1.0);
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        gradient.setColorAt(0.0, PhotoGallery::lightGrey);
        gradient.setColorAt(0.1, PhotoGallery::lightGrey.darker(150));
        gradient.setColorAt(0.9, PhotoGallery::lightGrey.darker(180));
        gradient.setColorAt(1.0, PhotoGallery::lightGrey.darker(300));

        QPalette palette = m_listView->palette();
        palette.setColor(QPalette::Base, Qt::black);
        palette.setColor(QPalette::Button, Qt::white);
        palette.setColor(QPalette::ButtonText, Qt::black);
        palette.setColor(QPalette::Text, PhotoGallery::blue);
        palette.setColor(QPalette::BrightText, PhotoGallery::orange);
        palette.setBrush(QPalette::Highlight, QBrush(gradient));
        m_listView->setPalette(palette);

        m_stack->addWidget(m_listView);

        connect(m_listView, SIGNAL(activated(QModelIndex)),
                this, SLOT(albumSelected(QModelIndex)));
        connect(m_listView, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(currentChanged(QModelIndex)));
    }

    m_albumModel->setSortMode(AlbumModel::SortByName);

    m_ribbon->setType(AlbumRibbon::CharacterRibbon);
    m_ribbon->setMinimumSpacing(0);
    m_ribbon->setRange(QLatin1Char('A').unicode(), QLatin1Char('Z').unicode());

    if (m_iconView)
        m_listView->setCurrentIndex(m_iconView->currentIndex());

    m_stack->setCurrentWidget(m_listView);

    emit sortModeChanged(AlbumModel::SortByName);
}

void AlbumSelector::sortByDate()
{
    if (!m_iconView) {
        m_iconView = new QSmoothIconView;
        m_iconView->setModel(m_albumModel);
        m_iconView->setScrollbarEnabled(false);
        m_iconView->setItemDelegate(new AlbumIconDelegate(m_iconView));
        m_iconView->setSelectionMode(QSmoothIconView::SingleSelection);
        m_iconView->setFixedRowCount( 2 );
        m_iconView->setSpacing(6);

        QLinearGradient gradient(0.0, 0.0, 0.0, 1.0);
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        gradient.setColorAt(0.0, PhotoGallery::lightGrey);
        gradient.setColorAt(0.1, PhotoGallery::lightGrey.darker(150));
        gradient.setColorAt(0.9, PhotoGallery::lightGrey.darker(180));
        gradient.setColorAt(1.0, PhotoGallery::lightGrey.darker(300));

        QPalette palette = m_iconView->palette();
        palette.setColor(QPalette::Base, Qt::black);
        palette.setColor(QPalette::Button, Qt::white);
        palette.setColor(QPalette::ButtonText, Qt::black);
        palette.setColor(QPalette::Text, PhotoGallery::blue);
        palette.setColor(QPalette::BrightText, PhotoGallery::orange);
        palette.setBrush(QPalette::Highlight, QBrush(gradient));
        m_iconView->setPalette(palette);

        connect(m_iconView, SIGNAL(activated(QModelIndex)),
                this, SLOT(albumSelected(QModelIndex)));
        connect(m_iconView, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(currentChanged(QModelIndex)));

        m_stack->addWidget(m_iconView);
    }

    m_albumModel->setSortMode(AlbumModel::SortByDate);

    m_ribbon->setType(AlbumRibbon::YearRibbon);
    m_ribbon->setMinimumSpacing(8);
    m_ribbon->setRange(m_albumModel->minimumYear(), m_albumModel->maximumYear());

    if (m_listView)
        m_iconView->setCurrentIndex(m_listView->currentIndex());

    m_stack->setCurrentWidget(m_iconView);

    emit sortModeChanged(AlbumModel::SortByDate);
}

void AlbumSelector::ribbonValueChanged(int value)
{
    switch (m_albumModel->sortMode()) {
    case AlbumModel::SortByDate:
        m_iconView->scrollTo(m_albumModel->indexForYear(value));
        break;
    case AlbumModel::SortByName:
        m_listView->scrollTo(m_albumModel->indexForCharacter(QChar(value)));
        break;
    }
}

void AlbumSelector::yearRangeChanged(int minimum, int maximum)
{
    if (m_ribbon->type() == AlbumRibbon::YearRibbon)
        m_ribbon->setRange(minimum, maximum);
}

void AlbumSelector::currentChanged(const QModelIndex &index)
{
    switch (m_albumModel->sortMode()) {
    case AlbumModel::SortByDate:
        m_ribbon->setValue(index.data(AlbumModel::YearRole).toInt());
        break;
    case AlbumModel::SortByName:
        {
            QString name = index.data(Qt::DisplayRole).toString();

            m_ribbon->setValue(!name.isEmpty()
                    ? name.at(0).unicode()
                    : QLatin1Char('A').unicode());
        }
        break;
    }
}

#include "albumselector.moc"
