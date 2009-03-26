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

#include <QAction>
#include <QSoftMenuBar>
#include <QKeyEvent>


#include "qmediamenutest.h"

CustomMenuItem::CustomMenuItem(QIcon* icon, QString text, QMediaList* data)
    :QAbstractMediaMenuItem(icon,text,data)
{
}

bool CustomMenuItem::execute()
{
    qWarning("CustomMenuItem::execute()");
    setLevel(0);
    emit dataChanged();
    return false;
}

QSize CustomMenuItem::size()
{
    return QSize(20,20);
}

void CustomMenuItem::paint(QPainter* painter, const QStyleOptionViewItem& option)
{
    static const int iconSize(qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize));

    QFont main(option.font);
    main.setWeight(QFont::Bold);
    painter->save();
    QRect textRect(option.rect);
    QRect iconRect(option.rect);
    QRect headerRect(textRect);
    QFontMetrics fm(main);

    QPoint drawOffset(iconRect.left() + ((iconRect.width() - iconSize)/2), iconRect.top() + ((iconRect.height() - iconSize) / 2));
    painter->drawPixmap(drawOffset, icon()->pixmap(QSize(iconSize, iconSize)));

    painter->setFont(main);
    painter->drawText(headerRect, Qt::AlignLeading, fm.elidedText(text(), option.textElideMode, headerRect.width()));

    painter->restore();
}

QMediaMenuTest::QMediaMenuTest( QWidget *parent, Qt::WindowFlags flags )
    : QWidget( parent )
{
    setWindowFlags( flags );

    QVBoxLayout* layout = new QVBoxLayout;

    QMediaPlaylist playlist(QContentFilter(QContent::Document));
    medialist = new QMediaList(playlist);

    mainmenu = new QMediaMenu();

    layout->addWidget(mainmenu);
    setLayout(layout);

    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );
    QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Back );


    // MAINMENU
    QMediaMenuItem* a = new QMediaMenuItem(new QIcon(":icon/mediaplayer/black/music"), QString("Music"), 0);
    mainmenu->add(a);

    a->add(new QMediaMenuItem(QMediaList::Artist, medialist));
    a->add(new QMediaMenuItem(QMediaList::Album, medialist));
    a->add(new QMediaMenuItem(QMediaList::Genre, medialist));
    a->add(new QMediaMenuItem(QMediaList::Title, medialist));

    QMediaMenuItem* b = new QMediaMenuItem(new QIcon(":icon/mediaplayer/black/videos"), QString("Videos"), medialist);
    mainmenu->add(b);

    QMediaMenuItem* c = (QMediaMenuItem*) new CustomMenuItem(new QIcon(":icon/mediaplayer/black/videos"), QString("Custom"), 0);
    mainmenu->add(c);

    mainmenu->resetMenu();
}

void QMediaMenuTest::keyPressEvent( QKeyEvent *event )
{
    if ( event->key() == Qt::Key_Left ) {
        event->accept();

        mainmenu->prev();
    } else
        event->ignore();
}
