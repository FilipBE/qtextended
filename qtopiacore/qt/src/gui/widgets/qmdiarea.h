/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QMDIAREA_H
#define QMDIAREA_H

#include <QtGui/qabstractscrollarea.h>
#include <QtGui/qtabwidget.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_MDIAREA

class QMdiSubWindow;

class QMdiAreaPrivate;
class Q_GUI_EXPORT QMdiArea : public QAbstractScrollArea
{
    Q_OBJECT
    Q_ENUMS(ViewMode)
    Q_PROPERTY(QBrush background READ background WRITE setBackground)
    Q_PROPERTY(WindowOrder activationOrder READ activationOrder WRITE setActivationOrder)
    Q_PROPERTY(ViewMode viewMode READ viewMode WRITE setViewMode)
#ifndef QT_NO_TABWIDGET
    Q_PROPERTY(QTabWidget::TabShape tabShape READ tabShape WRITE setTabShape)
    Q_PROPERTY(QTabWidget::TabPosition tabPosition READ tabPosition WRITE setTabPosition)
#endif
    Q_ENUMS(WindowOrder)
public:
    enum AreaOption {
        DontMaximizeSubWindowOnActivation = 0x1
    };
    Q_DECLARE_FLAGS(AreaOptions, AreaOption)

    enum WindowOrder {
        CreationOrder,
        StackingOrder,
        ActivationHistoryOrder
    };

    enum ViewMode {
        SubWindowView,
        TabbedView
    };

    QMdiArea(QWidget *parent = 0);
    ~QMdiArea();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QMdiSubWindow *currentSubWindow() const;
    QMdiSubWindow *activeSubWindow() const;
    QList<QMdiSubWindow *> subWindowList(WindowOrder order = CreationOrder) const;

    QMdiSubWindow *addSubWindow(QWidget *widget, Qt::WindowFlags flags = 0);
    void removeSubWindow(QWidget *widget);

    QBrush background() const;
    void setBackground(const QBrush &background);

    WindowOrder activationOrder() const;
    void setActivationOrder(WindowOrder order);

    void setOption(AreaOption option, bool on = true);
    bool testOption(AreaOption opton) const;

    void setViewMode(ViewMode mode);
    ViewMode viewMode() const;

#ifndef QT_NO_TABWIDGET
    void setTabShape(QTabWidget::TabShape shape);
    QTabWidget::TabShape tabShape() const;

    void setTabPosition(QTabWidget::TabPosition position);
    QTabWidget::TabPosition tabPosition() const;
#endif

Q_SIGNALS:
    void subWindowActivated(QMdiSubWindow *);

public Q_SLOTS:
    void setActiveSubWindow(QMdiSubWindow *window);
    void tileSubWindows();
    void cascadeSubWindows();
    void closeActiveSubWindow();
    void closeAllSubWindows();
    void activateNextSubWindow();
    void activatePreviousSubWindow();

protected Q_SLOTS:
    void setupViewport(QWidget *viewport);

protected:
    bool event(QEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
    void paintEvent(QPaintEvent *paintEvent);
    void childEvent(QChildEvent *childEvent);
    void resizeEvent(QResizeEvent *resizeEvent);
    void timerEvent(QTimerEvent *timerEvent);
    void showEvent(QShowEvent *showEvent);
    bool viewportEvent(QEvent *event);
    void scrollContentsBy(int dx, int dy);

private:
    Q_DISABLE_COPY(QMdiArea)
    Q_DECLARE_PRIVATE(QMdiArea)
    Q_PRIVATE_SLOT(d_func(), void _q_deactivateAllWindows())
    Q_PRIVATE_SLOT(d_func(), void _q_processWindowStateChanged(Qt::WindowStates, Qt::WindowStates))
    Q_PRIVATE_SLOT(d_func(), void _q_currentTabChanged(int index))
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMdiArea::AreaOptions)

QT_END_NAMESPACE

QT_END_HEADER

#endif // QT_NO_MDIAREA
#endif // QMDIAREA_H
