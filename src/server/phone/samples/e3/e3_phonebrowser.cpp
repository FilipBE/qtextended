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

#include "e3_phonebrowser.h"
#include "qtopiaserverapplication.h"
#include "launcherview.h"
#include <QCategoryManager>
#include <QContent>
#include <QContentFilter>
#include <QKeyEvent>
#include "lazycontentstack.h"
#include <QSoftMenuBar>

#include <QAction>
#include <QDebug>
#include <QHash>
#include <QItemDelegate>
#include <QListView>
#include <QMenu>
#include <QPainter>
#include <QStackedWidget>
#include <QVBoxLayout>

QTOPIA_REPLACE_WIDGET(QAbstractBrowserScreen, E3BrowserScreen);

class E3BrowserDelegate : public QAbstractItemDelegate
{
Q_OBJECT
public:
    E3BrowserDelegate(QObject *parent);

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


E3BrowserDelegate::E3BrowserDelegate(QObject *parent)
: QAbstractItemDelegate(parent)
{
}

QSize E3BrowserDelegate::sizeHint(const QStyleOptionViewItem &,
                               const QModelIndex &) const
{
    return QSize(64, 64);
}

void E3BrowserDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    QFontMetrics met(option.font);

    QRect iconRect = option.rect;
    iconRect.adjust(2, 2, -2, -2); // Margin
    iconRect.adjust(2 + met.height() / 2, 0,
                    -1 * (2 + met.height() / 2), -1 * (met.height() + 4)); // Text
    QRect textRect(option.rect.left(), iconRect.bottom() + 4, option.rect.width() - 4, met.height());

    QPixmap pix =
        qvariant_cast<QIcon>(index.data(Qt::DecorationRole)).pixmap(iconRect.size());

    if(option.state & QStyle::State_Selected) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(option.palette.highlight());
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawRoundRect(option.rect, 600 / option.rect.width(),
                                            600 / option.rect.height());
        painter->restore();
    }

    int x = iconRect.x() + (iconRect.width() - pix.width()) / 2;
    int y = iconRect.y() + (iconRect.height() - pix.height()) / 2;
    painter->drawPixmap(x, y, pix);

    QString text = index.data(Qt::DisplayRole).toString();
    painter->setFont(option.font);
    text = met.elidedText(text, Qt::ElideRight, textRect.width());

    if(option.state & QStyle::State_Selected)
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    else
        painter->setPen(option.palette.color(QPalette::Text));

    painter->drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter, text);
}

class E3BrowserScreenStack : public LazyContentStack
{
Q_OBJECT
public:
    E3BrowserScreenStack(QWidget *parent);

    QString currentName() const;

    void back() { LazyContentStack::back(); }
    void setMode(QListView::ViewMode);

protected:
    virtual QObject* createView(const QString &);
    virtual void raiseView(const QString &, bool reset);
    virtual QObject* currentViewObject();

private:
    QListView::ViewMode m_mode;
    QStackedWidget *stack;
    QHash<QString, int> stackContents;

    E3BrowserDelegate *m_bd;
    QItemDelegate *m_id;
};

E3BrowserScreenStack::E3BrowserScreenStack(QWidget *parent)
: LazyContentStack(NoFlags, parent), m_mode(QListView::IconMode), stack(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);
    stack = new QStackedWidget(this);
    layout->addWidget(stack);

    m_id = new QItemDelegate(this);
    m_bd = new E3BrowserDelegate(this);
}

void E3BrowserScreenStack::setMode(QListView::ViewMode mode)
{
    m_mode = mode;
    for(int ii = 0; ii < stack->count(); ++ii) {
        LauncherView *v = static_cast<LauncherView *>(stack->widget(ii));

        v->setViewMode(mode);
        if(mode == QListView::ListMode) {
            v->setItemDelegate(m_id);
            v->setColumns(1);
            v->setFont(QApplication::font());
        } else {
            v->setItemDelegate(m_bd);
            v->setColumns(3);
            QFont fn(QApplication::font());
            fn.setPointSize(4);
            v->setFont(fn);
        }
    }
}

QObject* E3BrowserScreenStack::createView(const QString &name)
{
    LauncherView *view = 0;
    if("Main" == name) {

        view = new LauncherView(stack);
        view->showCategory(QContentFilter::category(QLatin1String("MainApplications")));

    } else if(name.startsWith("Folder/")) {

        QString category = name.mid(7);
        view = new LauncherView(stack);
        view->showCategory(QContentFilter(QCategoryFilter(category)));
    }

    if(view) {
        view->setViewMode(m_mode);
        if(QListView::IconMode == m_mode) {
            view->setItemDelegate(m_bd);
            view->setColumns(3);
            QFont fn(QApplication::font());
            fn.setPointSize(4);
            view->setFont(fn);
        }
        stackContents[name] = stack->addWidget(view);
    }

    return view;
}

QObject* E3BrowserScreenStack::currentViewObject()
{
    return stack->currentWidget();
}

void E3BrowserScreenStack::raiseView(const QString &view, bool)
{
    stack->setCurrentIndex(stackContents[view]);
}

E3BrowserScreen::E3BrowserScreen(QWidget *parent, Qt::WFlags flags)
: QAbstractBrowserScreen(parent, flags)
{
    setWindowTitle(tr("Menu"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);

    m_stack = new E3BrowserScreenStack(this);
    layout->addWidget(m_stack);

    QMenu *menu = QSoftMenuBar::menuFor(this);
    listAction = new QAction(tr("List View"), this);
    gridAction = new QAction(tr("Grid View"), this);
    gridAction->setEnabled(false);
    menu->addAction(listAction);
    menu->addAction(gridAction);
    QObject::connect(listAction, SIGNAL(triggered()), this, SLOT(listMode()));
    QObject::connect(gridAction, SIGNAL(triggered()), this, SLOT(gridMode()));
}

void E3BrowserScreen::listMode()
{
    gridAction->setEnabled(true);
    listAction->setEnabled(false);
    m_stack->setMode(QListView::ListMode);
}

void E3BrowserScreen::gridMode()
{
    gridAction->setEnabled(false);
    listAction->setEnabled(true);
    m_stack->setMode(QListView::IconMode);
}

QString E3BrowserScreen::currentView() const
{
    return m_stack->currentView();
}

bool E3BrowserScreen::viewAvailable(const QString &name) const
{
    return "Main" == name || name.startsWith("Folder/");
}

void E3BrowserScreen::resetToView(const QString &name)
{
    m_stack->resetToView(name);
}

void E3BrowserScreen::moveToView(const QString &name)
{
    m_stack->showView(name);
}

void E3BrowserScreen::closeEvent(QCloseEvent *e)
{
    m_stack->back();

    if(m_stack->isDone()) {
        e->accept();
    } else {
        e->ignore();
    }
}

#include "e3_phonebrowser.moc"
