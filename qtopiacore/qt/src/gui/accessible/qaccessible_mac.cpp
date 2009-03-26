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

#include "qaccessible.h"

#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible_mac_p.h"
#include "qhash.h"
#include "qset.h"
#include "qpointer.h"
#include "qapplication.h"
#include "qmainwindow.h"
#include "qtextdocument.h"
#include "qdebug.h"
#include "qabstractslider.h"
#include "qsplitter.h"
#include "qtabwidget.h"
#include "qlistview.h"
#include "qtableview.h"
#include "qdockwidget.h"

#include <private/qt_mac_p.h>
#include <CoreFoundation/CoreFoundation.h>

QT_BEGIN_NAMESPACE

/*****************************************************************************
  Externals
 *****************************************************************************/
extern bool qt_mac_is_macsheet(const QWidget *w); //qwidget_mac.cpp
extern bool qt_mac_is_macdrawer(const QWidget *w); //qwidget_mac.cpp

/*****************************************************************************
  QAccessible Bindings
 *****************************************************************************/
//hardcoded bindings between control info and (known) QWidgets
struct QAccessibleTextBinding {
    int qt;
    CFStringRef mac;
    bool settable;
} text_bindings[][10] = {
    { { QAccessible::MenuItem, kAXMenuItemRole, false },
      { -1, 0, false }
    },
    { { QAccessible::MenuBar, kAXMenuBarRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ScrollBar, kAXScrollBarRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Grip, kAXGrowAreaRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Window, kAXWindowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Dialog, kAXWindowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::AlertMessage, kAXWindowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ToolTip, kAXWindowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::HelpBalloon, kAXWindowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::PopupMenu, kAXMenuRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Application, kAXApplicationRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Pane, kAXGroupRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Grouping, kAXGroupRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Separator, kAXSplitterRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ToolBar, kAXToolbarRole, false },
      { -1, 0, false }
    },
    { { QAccessible::PageTab, kAXRadioButtonRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ButtonMenu, kAXMenuButtonRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ButtonDropDown, kAXPopUpButtonRole, false },
      { -1, 0, false }
    },
    { { QAccessible::SpinBox, kAXIncrementorRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Slider, kAXSliderRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ProgressBar, kAXProgressIndicatorRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ComboBox, kAXPopUpButtonRole, false },
      { -1, 0, false }
    },
    { { QAccessible::RadioButton, kAXRadioButtonRole, false },
      { -1, 0, false }
    },
    { { QAccessible::CheckBox, kAXCheckBoxRole, false },
      { -1, 0, false }
    },
    { { QAccessible::StaticText, kAXStaticTextRole, false },
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
      { QAccessible::Name, kAXDescriptionAttribute, false },
#endif
      { QAccessible::Name, kAXValueAttribute, false },
      { -1, 0, false }
    },
    { { QAccessible::Table, kAXTableRole, false },
      { -1, 0, false }
    },
    { { QAccessible::StatusBar, kAXStaticTextRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Column, kAXColumnRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ColumnHeader, kAXColumnRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Row, kAXRowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::RowHeader, kAXRowRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Cell, kAXTextFieldRole, false },
      { -1, 0, false }
    },
    { { QAccessible::PushButton, kAXButtonRole, false },
      { -1, 0, false }
    },
    { { QAccessible::EditableText, kAXTextFieldRole, true },
      { -1, 0, false }
    },
    { { QAccessible::Link, kAXTextFieldRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Indicator, kAXValueIndicatorRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Splitter, kAXSplitGroupRole, false },
      { -1, 0, false }
    },
    { { QAccessible::List, kAXListRole, false },
      { -1, 0, false }
    },
    { { QAccessible::ListItem, kAXStaticTextRole, false },
      { -1, 0, false }
    },
    { { QAccessible::Cell, kAXStaticTextRole, false },
      { -1, 0, false }
    },
    { { -1, 0, false } }
};

class QAInterface;
static CFStringRef macRole(const QAInterface &interface);

QDebug operator<<(QDebug debug, const QAInterface &interface)
{
    if (interface.isValid() == false)
        debug << "invalid interface";
    else 
        debug << interface.object() << "id" << interface.id() << "role" << hex << interface.role();
    return debug;
}

// The root of the Qt accessible hiearchy.
static QObject *rootObject = 0;

static OSStatus applicationEventHandler(EventHandlerCallRef next_ref, EventRef event, void *data);
static EventHandlerUPP applicationEventHandlerUPP = 0;
static EventTypeSpec application_events[] = {
    { kEventClassAccessibility,  kEventAccessibleGetChildAtPoint },
    { kEventClassAccessibility,  kEventAccessibleGetNamedAttribute }
};

static CFStringRef kObjectQtAccessibility = CFSTR("com.trolltech.qt.accessibility");
static EventHandlerUPP objectCreateEventHandlerUPP = 0;
static EventTypeSpec objectCreateEvents[] = {
    { kEventClassHIObject,  kEventHIObjectConstruct },
    { kEventClassHIObject,  kEventHIObjectInitialize },
    { kEventClassHIObject,  kEventHIObjectDestruct },
    { kEventClassHIObject,  kEventHIObjectPrintDebugInfo }
};

static OSStatus accessibilityEventHandler(EventHandlerCallRef next_ref, EventRef event, void *data);
static EventHandlerUPP accessibilityEventHandlerUPP = 0;
static EventTypeSpec accessibilityEvents[] = {
    { kEventClassAccessibility,  kEventAccessibleGetChildAtPoint },
    { kEventClassAccessibility,  kEventAccessibleGetFocusedChild },
    { kEventClassAccessibility,  kEventAccessibleGetAllAttributeNames },
    { kEventClassAccessibility,  kEventAccessibleGetNamedAttribute },
    { kEventClassAccessibility,  kEventAccessibleSetNamedAttribute },
    { kEventClassAccessibility,  kEventAccessibleIsNamedAttributeSettable },
    { kEventClassAccessibility,  kEventAccessibleGetAllActionNames },
    { kEventClassAccessibility,  kEventAccessiblePerformNamedAction },
    { kEventClassAccessibility,  kEventAccessibleGetNamedActionDescription }
};

static void installAcessibilityEventHandler(HIObjectRef hiObject)
{
    if (!accessibilityEventHandlerUPP)
        accessibilityEventHandlerUPP = NewEventHandlerUPP(accessibilityEventHandler);

    InstallHIObjectEventHandler(hiObject, accessibilityEventHandlerUPP,
                                        GetEventTypeCount(accessibilityEvents),
                                        accessibilityEvents, 0, 0);
}

static OSStatus objectCreateEventHandler(EventHandlerCallRef next_ref, EventRef event, void *data)
{
    Q_UNUSED(data)
    Q_UNUSED(event)
    Q_UNUSED(next_ref)
    return noErr;
}

static void registerQtAccessibilityHIObjectSubclass()
{
    if (!objectCreateEventHandlerUPP)
        objectCreateEventHandlerUPP = NewEventHandlerUPP(objectCreateEventHandler);
    OSStatus err = HIObjectRegisterSubclass(kObjectQtAccessibility, 0, 0, objectCreateEventHandlerUPP,
                                         GetEventTypeCount(objectCreateEvents), objectCreateEvents, 0, 0);
    if (err && err != hiObjectClassExistsErr)
        qWarning("qaccessible_mac internal error: Could not register accessibility HIObject subclass");
}

static void installApplicationEventhandler()
{
    if (!applicationEventHandlerUPP)
        applicationEventHandlerUPP = NewEventHandlerUPP(applicationEventHandler);

    OSStatus err = InstallApplicationEventHandler(applicationEventHandlerUPP,
                            GetEventTypeCount(application_events), application_events,
                            0, 0);

    if (err && err != eventHandlerAlreadyInstalledErr)
        qWarning("qaccessible_mac internal error: Could not install application accessibility event handler");}

static void removeEventhandler(EventHandlerUPP eventHandler)
{
    if (eventHandler) {
        DisposeEventHandlerUPP(eventHandler);
        eventHandler = 0;
    }
}

bool QAInterface::operator==(const QAInterface &other) const
{
    if (isValid() == false || other.isValid() == false)
        return (isValid() && other.isValid());
    
    // walk up the parent chain, comparing child indexes, until we reach
    // an interface that has a QObject.
    QAInterface currentThis = *this;
    QAInterface currentOther = other;
    
    while (currentThis.object() == 0) {
        if (currentOther.object() != 0)
            return false;

        // fail if the child indexes in the two hirearchies don't match.
        if (currentThis.parent().indexOfChild(currentThis) !=
            currentOther.parent().indexOfChild(currentOther))
            return false;

        currentThis = currentThis.parent();
        currentOther = currentOther.parent();
    }
    
    return (currentThis.object() == currentOther.object() && currentThis.id() == currentOther.id());
}

bool QAInterface::operator!=(const QAInterface &other) const
{
    return !operator==(other);
}

uint qHash(const QAInterface &item)
{
    if (item.isValid())
        return qHash(item.object()) + qHash(item.id());
    else
        return qHash(item.cachedObject()) + qHash(item.id());
}

QAInterface QAInterface::navigate(RelationFlag relation, int entry) const
{
        if (!checkValid())
            return QAInterface();

    // On a QAccessibleInterface that handles its own children we can short-circut
    // the navigation if this QAInterface refers to one of the children:
    if (child != 0) {
        // The Ancestor interface will always be the same QAccessibleInterface with
        // a child value of 0.
        if (relation == QAccessible::Ancestor)
            return QAInterface(*this, 0);

        // The child hiearchy is only one level deep, so navigating to a child
        // of a child is not possible.
        if (relation == QAccessible::Child) {
            return QAInterface();
        }
    }
    QAccessibleInterface *child_iface = 0;

    const int status = base.interface->navigate(relation, entry, &child_iface);

    if (status == -1)
        return QAInterface(); // not found;

    // Check if target is a child of this interface.
    if (!child_iface) {
        return QAInterface(*this, status);
    } else {
        // Target is child_iface or a child of that (status decides).
        return QAInterface(child_iface, status);
    }
}

QAElement::QAElement()
:elementRef(0)
{}

QAElement::QAElement(AXUIElementRef elementRef)
:elementRef(elementRef)
{
    if (elementRef != 0) {
        CFRetain(elementRef);
        CFRetain(object());
    }
}

QAElement::QAElement(const QAElement &element)
:elementRef(element.elementRef)
{
    if (elementRef != 0) {
        CFRetain(elementRef);
        CFRetain(object());
    }
}

QAElement::QAElement(HIObjectRef object, int child)
{
    if (object == 0) {
        elementRef = 0; // Create invalid QAElement.
    } else {
        elementRef = AXUIElementCreateWithHIObjectAndIdentifier(object, child);
        CFRetain(object);
    }
}

QAElement::~QAElement()
{
    if (elementRef != 0) {
        CFRelease(object());
        CFRelease(elementRef);
    }
}

void QAElement::operator=(const QAElement &other)
{
    if (*this == other)
        return;

    if (elementRef != 0) {
        CFRelease(object());
        CFRelease(elementRef);
    }

    elementRef = other.elementRef;

    if (elementRef != 0) {
        CFRetain(elementRef);
        CFRetain(object());
    }
}

bool QAElement::operator==(const QAElement &other) const
{
    if (elementRef == 0 || other.elementRef == 0)
        return (elementRef == other.elementRef);

    return CFEqual(elementRef, other.elementRef);
}

uint qHash(QAElement element)
{
    return qHash(element.object()) + qHash(element.id());
}

static QInterfaceFactory *createFactory(const QAInterface &interface);
Q_GLOBAL_STATIC(QAccessibleHierarchyManager, accessibleHierarchyManager);

/*
    Reomves all accessibility info accosiated with the sender object.
*/
void QAccessibleHierarchyManager::objectDestroyed(QObject *object)
{
    HIObjectRef hiObject = qobjectHiobjectHash.value(object);
    delete qobjectElementHash.value(object);
    qobjectElementHash.remove(object);
    hiobjectInterfaceHash.remove(hiObject);
}

/*
    Removes all stored items.
*/
void QAccessibleHierarchyManager::reset()
{
    qDeleteAll(qobjectElementHash);
    qobjectElementHash.clear();
    hiobjectInterfaceHash.clear();
    qobjectHiobjectHash.clear();
}

QAccessibleHierarchyManager *QAccessibleHierarchyManager::instance()
{
    return accessibleHierarchyManager();
}

static bool isItemView(const QAInterface &interface)
{
    QObject *object = interface.object();
    return (interface.role() == QAccessible::List || interface.role() == QAccessible::Table
            || (object && qobject_cast<QAbstractItemView *>(interface.object()))
            || (object && object->objectName() == QLatin1String("qt_scrollarea_viewport")
                && qobject_cast<QAbstractItemView *>(object->parent())));
}

static bool isTabWidget(const QAInterface &interface)
{
    if (QObject *object = interface.object())
        return (object->inherits("QTabWidget") && interface.id() == 0);
    return false;
}

static bool isStandaloneTabBar(const QAInterface &interface)
{
    QObject *object = interface.object();
    if (interface.role() == QAccessible::PageTabList && object)
        return (qobject_cast<QTabWidget *>(object->parent()) == 0);

    return false;
}

static bool isEmbeddedTabBar(const QAInterface &interface)
{
    QObject *object = interface.object();
    if (interface.role() == QAccessible::PageTabList && object)
        return (qobject_cast<QTabWidget *>(object->parent()));

    return false;
}

/*
    Decides if a QAInterface is interesting from an accessibility users point of view.
*/
bool isItInteresting(const QAInterface &interface)
{
    // Mac accessibility does not have an attribute that corresponds to the Invisible/Offscreen
    // state, so we disable the interface here.
    const QAccessible::State state = interface.state();
    if (state & QAccessible::Invisible ||
        state & QAccessible::Offscreen )
        return false;

    const QAccessible::Role role = interface.role();

    if (QObject * const object = interface.object()) {
        const QString className = QLatin1String(object->metaObject()->className());

        // VoiceOver focusing on tool tips can be confusing. The contents of the
        // tool tip is avalible through the description attribute anyway, so
        // we disable accessibility for tool tips.
        if (className == QLatin1String("QTipLabel"))
            return false;

        // Hide TabBars that has a QTabWidget parent (the tab widget handles the accessibility)
        if (isEmbeddedTabBar(interface))
            return false;

         // Hide docked dockwidgets. ### causes infinitie loop in the apple accessibility code.
     /*    if (QDockWidget *dockWidget = qobject_cast<QDockWidget *>(object)) {
            if (dockWidget->isFloating() == false)
                return false;        
         }
    */
    }

    // Client is a generic role returned by plain QWidgets or other
    // widgets that does not have separate QAccessible interface, such
    // as the TabWidget. Return false unless macRole gives the interface
    // a special role.
    if (role == QAccessible::Client && macRole(interface) == kAXUnknownRole)
        return false;

    // Some roles are not interesting:
    if (role == QAccessible::Border ||    // QFrame
        role == QAccessible::Application || // We use the system-provided application element.
        role == QAccessible::MenuItem)      // The system also provides the menu items.
        return false;

    // It is probably better to access the toolbar buttons directly than having
    // to navigate through the toolbar.
    if (role == QAccessible::ToolBar)
        return false;

    return true;
}

QAElement QAccessibleHierarchyManager::registerInterface(QObject *object, int child)
{
    return registerInterface(QAInterface(QAccessible::queryAccessibleInterface(object), child));
}

/*
    Creates a QAXUIelement that corresponds to the given QAInterface.
*/
QAElement QAccessibleHierarchyManager::registerInterface(const QAInterface &interface)
{
    if (interface.isValid() == false)
        return QAElement();
    
    QAInterface objectInterface = interface.objectInterface();

    QObject * qobject = objectInterface.object();
    HIObjectRef hiobject = objectInterface.hiObject();
    if (qobject == 0 || hiobject == 0)
        return QAElement();

    if (qobjectElementHash.contains(qobject) == false) {
        registerInterface(qobject, hiobject, createFactory(interface));
        HIObjectSetAccessibilityIgnored(hiobject, !isItInteresting(interface));
    }

    return QAElement(hiobject, interface.id());
}

void QAccessibleHierarchyManager::registerInterface(QObject * qobject, HIObjectRef hiobject, QInterfaceFactory *interfaceFactory)
{
    if (qobjectElementHash.contains(qobject) == false) {
        qobjectElementHash.insert(qobject, interfaceFactory);
        qobjectHiobjectHash.insert(qobject, hiobject);
        connect(qobject, SIGNAL(destroyed(QObject *)), SLOT(objectDestroyed(QObject *)));
    }

    if (hiobjectInterfaceHash.contains(hiobject) == false) {
        hiobjectInterfaceHash.insert(hiobject, interfaceFactory);
        installAcessibilityEventHandler(hiobject);
    }
}

void QAccessibleHierarchyManager::registerChildren(const QAInterface &interface)
{
    QObject * const object = interface.object();
    if (object == 0)
        return;

    QInterfaceFactory *interfaceFactory = qobjectElementHash.value(object);
    
    if (interfaceFactory == 0)
        return;

    interfaceFactory->registerChildren();
}

QAInterface QAccessibleHierarchyManager::lookup(const AXUIElementRef &element)
{
     if (element == 0)
        return QAInterface();

    HIObjectRef hiObject = AXUIElementGetHIObject(element);

    QInterfaceFactory *factory = hiobjectInterfaceHash.value(hiObject);
    if (factory == 0) {
        return QAInterface();
    }
        
    UInt64 id;
    AXUIElementGetIdentifier(element, &id);
    return factory->interface(id);
}

QAInterface QAccessibleHierarchyManager::lookup(const QAElement &element)
{
    return lookup(element.element());
}

QAElement QAccessibleHierarchyManager::lookup(const QAInterface &interface)
{
    if (interface.isValid() == false)
        return QAElement();

    QInterfaceFactory *factory = qobjectElementHash.value(interface.objectInterface().object());
    if (factory == 0)
        return QAElement();

    return factory->element(interface);
}

QAElement QAccessibleHierarchyManager::lookup(QObject * const object, int id)
{
    QInterfaceFactory *factory = qobjectElementHash.value(object);
    if (factory == 0)
        return QAElement();

    return factory->element(id);
}

/*
    Standard interface mapping, return the stored interface
    or HIObjectRef, and there is an one-to-one mapping between
    the identifier and child.
*/
class QStandardInterfaceFactory : public QInterfaceFactory
{
public:
    QStandardInterfaceFactory(const QAInterface &interface)
    : m_interface(interface), object(interface.hiObject())
    {
        CFRetain(object);
    }
    
    ~QStandardInterfaceFactory()
    {
         CFRelease(object);
    }

    
    QAInterface interface(UInt64 identifier)
    {
        const int child = identifier;
        return QAInterface(m_interface, child);
    }

    QAElement element(int id)
    {
        return QAElement(object, id);
    }

    QAElement element(const QAInterface &interface)
    {
        if (interface.object() == 0)
            return QAElement();
        return QAElement(object, interface.id());
    }

    void registerChildren()
    {
        const int childCount = m_interface.childCount();
        for (int i = 1; i <= childCount; ++i) {
            accessibleHierarchyManager()->registerInterface(m_interface.navigate(QAccessible::Child, i));
        }
    }

private:
    QAInterface m_interface;
    HIObjectRef object;
};

/*
    Interface mapping where that creates one HIObject for each interface child.
*/
class QMultipleHIObjectFactory : public QInterfaceFactory
{
public:
    QMultipleHIObjectFactory(const QAInterface &interface)
    : m_interface(interface)
    {  }
    
    ~QMultipleHIObjectFactory()
    {
        foreach (HIObjectRef object, objects) {
            CFRelease(object);
        }
    }

    QAInterface interface(UInt64 identifier)
    {
        const int child = identifier;
        return QAInterface(m_interface, child);
    }

    QAElement element(int child)
    {
        if (child == 0)
            return QAElement(m_interface.hiObject(), 0);
        
        if (child > objects.count())
            return QAElement();

        return QAElement(objects.at(child - 1), child);
    }

    void registerChildren()
    {
        const int childCount = m_interface.childCount();
        for (int i = 1; i <= childCount; ++i) {
            HIObjectRef hiobject;
            HIObjectCreate(kObjectQtAccessibility, 0, &hiobject);
            objects.append(hiobject);
              accessibleHierarchyManager()->registerInterface(m_interface.object(), hiobject, this);
            HIObjectSetAccessibilityIgnored(hiobject, !isItInteresting(m_interface.navigate(QAccessible::Child, i)));
        }
    }

private:
    QAInterface m_interface;
    QList<HIObjectRef> objects;
};

class QItemViewInterfaceFactory : public QInterfaceFactory
{
public:
    QItemViewInterfaceFactory(const QAInterface &interface)
    : m_interface(interface), object(interface.hiObject())
    {
        CFRetain(object);
        columnCount = 0;
        if (QTableView * tableView = qobject_cast<QTableView *>(interface.parent().object())) {
            if (tableView->model())
                columnCount = tableView->model()->columnCount();
            if (tableView->verticalHeader())
                ++columnCount;
        }
    }
    
    ~QItemViewInterfaceFactory()
    {
        CFRelease(object);
    }

    QAInterface interface(UInt64 identifier)
    {
        if (identifier == 0)
            return m_interface;

        if (m_interface.role() == QAccessible::List)
            return m_interface.childAt(identifier);
        
        if (m_interface.role() == QAccessible::Table) {
            const int index = identifier;
            if (index == 0)
                return m_interface; // return the item view interface.
           
            const int rowIndex = (index - 1) / (columnCount + 1);
            const int cellIndex = (index - 1)  % (columnCount + 1);
/*
            qDebug() << "index" << index;
            qDebug() << "rowIndex" << rowIndex;
            qDebug() << "cellIndex" << cellIndex;
*/
            const QAInterface rowInterface = m_interface.childAt(rowIndex + 1);

            if ((cellIndex) == 0) // Is it a row?
                return rowInterface;
            else {
                return rowInterface.childAt(cellIndex);
            }
        }

        return QAInterface();
    }

    QAElement element(int id)
    {
        if (id != 0) {
            return QAElement();
        }
        return QAElement(object, 0);
    }

    QAElement element(const QAInterface &interface)
    {
        if (interface.object() && interface.object() == m_interface.object()) {
            return QAElement(object, 0);
        } else if (m_interface.role() == QAccessible::List) {
            if (interface.parent().object() && interface.parent().object() == m_interface.object())
                return QAElement(object, m_interface.indexOfChild(interface));
        } else if (m_interface.role() == QAccessible::Table) {
            QAInterface currentInterface = interface;
            int index = 0;

            while (currentInterface.isValid() && currentInterface.object() == 0) {
                const QAInterface parentInterface = currentInterface.parent();
/*
                qDebug() << "current index" << index;
                qDebug() << "current interface" << interface;

                qDebug() << "parent interface" << parentInterface;
                qDebug() << "grandparent interface" << parentInterface.parent();
                qDebug() << "childCount" << interface.childCount();
                qDebug() << "index of child" << parentInterface.indexOfChild(currentInterface);
*/
                index += ((parentInterface.indexOfChild(currentInterface) - 1) * (currentInterface.childCount() + 1)) + 1;
                currentInterface = parentInterface;
//                qDebug() << "new current interface" << currentInterface;
            }
            if (currentInterface.object() == m_interface.object())
                return QAElement(object, index);


        }
        return QAElement();
    }

    void registerChildren()
    {
        // Item view child interfraces don't have their own qobjects, so there is nothing to register here.
    }

private:
    QAInterface m_interface;
    HIObjectRef object;
    int columnCount; // for table views;
};

static bool managesChildren(const QAInterface &interface)
{
    return (interface.childCount() > 0 && interface.childAt(1).id() > 0);
}

static QInterfaceFactory *createFactory(const QAInterface &interface)
{
    if (isItemView(interface)) {
        return new QItemViewInterfaceFactory(interface);    
    }  if (managesChildren(interface)) {
        return new QMultipleHIObjectFactory(interface);
    }

    return new QStandardInterfaceFactory(interface);
}

QList<QAElement> lookup(const QList<QAInterface> &interfaces)
{
    QList<QAElement> elements;
    foreach (const QAInterface &interface, interfaces)
        if (interface.isValid()) {
            const QAElement element = accessibleHierarchyManager()->lookup(interface);
            if (element.isValid())
                elements.append(element);
        }
    return elements;
}

// Debug output helpers:
/*
static QString nameForEventKind(UInt32 kind)
{
    switch(kind) {
        case kEventAccessibleGetChildAtPoint:       return QString("GetChildAtPoint");      break;
        case kEventAccessibleGetAllAttributeNames:  return QString("GetAllAttributeNames"); break;
        case kEventAccessibleGetNamedAttribute:     return QString("GetNamedAttribute");    break;
        case kEventAccessibleSetNamedAttribute:     return QString("SetNamedAttribute");    break;
        case kEventAccessibleGetAllActionNames:     return QString("GetAllActionNames");    break;
        case kEventAccessibleGetFocusedChild:       return QString("GetFocusedChild");      break;
        default:
            return QString("Unknown accessibility event type: %1").arg(kind);
        break;
    };
}
*/
static bool qt_mac_append_cf_uniq(CFMutableArrayRef array, CFTypeRef value)
{
    if (value == 0)
        return false; 

    CFRange range;
    range.location = 0;
    range.length = CFArrayGetCount(array);
    if(!CFArrayContainsValue(array, range, value)) {
        CFArrayAppendValue(array, value);
        return true;
    }
    return false;
}

static OSStatus setAttributeValue(EventRef event, const QList<QAElement> &elements)
{
    CFMutableArrayRef array = CFArrayCreateMutable(0, 0, &kCFTypeArrayCallBacks);
    foreach (const QAElement &element, elements) {
        if (element.isValid())
            CFArrayAppendValue(array, element.element());
    }

    const OSStatus err = SetEventParameter(event, kEventParamAccessibleAttributeValue, 
                                           typeCFTypeRef, sizeof(array), &array);
    CFRelease(array);
    return err;
}

/*
    Gets the AccessibleObject parameter from an event.
*/
static inline AXUIElementRef getAccessibleObjectParameter(EventRef event)
{
    AXUIElementRef element;
    GetEventParameter(event, kEventParamAccessibleObject, typeCFTypeRef, 0,
                        sizeof(element), 0, &element);
    return element;
}

/*
    The application event handler makes sure that all top-level qt windows are registered
    before any accessibility events are handeled.
*/
static OSStatus applicationEventHandler(EventHandlerCallRef next_ref, EventRef event, void *data)
{
    Q_UNUSED(data);
    
    QAInterface rootInterface(QAccessible::queryAccessibleInterface(rootObject ? rootObject : qApp), 0);
    accessibleHierarchyManager()->registerChildren(rootInterface);

    return CallNextEventHandler(next_ref, event);
}

/*
    Returns the value for element by combining the QAccessibility::Checked and
    QAccessibility::Mixed flags into an int value that the Mac accessibilty
    system understands. This works for check boxes, radio buttons, and the like.
    The return values are:
    0: unchecked
    1: checked
    2: undecided
*/
static int buttonValue(QAInterface element)
{
    const QAccessible::State state = element.state();
    if (state & QAccessible::Mixed)
        return 2;
    else if(state & QAccessible::Checked)
        return 1;
    else
        return 0;
}

static QString getValue(const QAInterface &interface)
{
    const QAccessible::Role role = interface.role();
    if (role == QAccessible::RadioButton || role == QAccessible::CheckBox)
        return QString::number(buttonValue(interface));
    else
        return interface.text(QAccessible::Value);
}

/*
    Translates a QAccessible::Role into a mac accessibility role.
*/
static CFStringRef macRole(const QAInterface &interface)
{
    const QAccessible::Role qtRole = interface.role();

//    qDebug() << "role for" << interface.object() << "interface role" << hex << qtRole;
    
    // Qt accessibility:  QAccessible::Splitter contains QAccessible::Grip.
    // Mac accessibility: AXSplitGroup contains AXSplitter.
    if (qtRole == QAccessible::Grip) {
        const QAInterface parent = interface.parent();
        if (parent.isValid() && parent.role() == QAccessible::Splitter)
            return kAXSplitterRole;
    }

    // Tab widgets and standalone tab bars get the kAXTabGroupRole. Accessibility
    // for tab bars emebedded in a tab widget is handled by the tab widget.
    if (isTabWidget(interface) || isStandaloneTabBar(interface))
        return kAXTabGroupRole;

    if (QObject *object = interface.object()) {
        // ### The interface for an abstract scroll area returns the generic "Client"
        // role, so we have to to an extra detect on the QObject here.
        if (object->inherits("QAbstractScrollArea") && interface.id() == 0)
            return kAXScrollAreaRole;

        if (object->inherits("QDockWidget"))
            return kAXUnknownRole;
    }

    int i = 0;
    int testRole = text_bindings[i][0].qt;
    while (testRole != -1) {
        if (testRole == qtRole)
            return text_bindings[i][0].mac;
        ++i;
        testRole = text_bindings[i][0].qt;
    }
    
//    qDebug() << "got unknown role!" << interface << interface.parent();

    return kAXUnknownRole;
}

/*
    Translates a QAccessible::Role and an attribute name into a QAccessible::Text, taking into
    account execptions listed in text_bindings.
*/
static int textForRoleAndAttribute(QAccessible::Role role, CFStringRef attribute)
{
     // Search for exception, return it if found.
    int testRole = text_bindings[0][0].qt;
    int i = 0;
    while (testRole != -1) {
        if (testRole == role) {
            int j = 1;
            int qtRole = text_bindings[i][j].qt;
            CFStringRef testAttribute = text_bindings[i][j].mac;
            while (qtRole != -1) {
                if (CFStringCompare(attribute, testAttribute, 0) == kCFCompareEqualTo) {
                    return (QAccessible::Text)qtRole;
                }
                ++j;
                testAttribute = text_bindings[i][j].mac;
                qtRole = text_bindings[i][j].qt;
            }
            break;
        }
        ++i;
        testRole = text_bindings[i][0].qt;
    }

    // Return default mappping
    if (CFStringCompare(attribute, kAXTitleAttribute, 0) == kCFCompareEqualTo)
        return QAccessible::Name;
    else if (CFStringCompare(attribute, kAXValueAttribute, 0) == kCFCompareEqualTo)
        return QAccessible::Value;
    else if (CFStringCompare(attribute, kAXHelpAttribute, 0) == kCFCompareEqualTo)
        return QAccessible::Help;
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    else if (CFStringCompare(attribute, kAXDescriptionAttribute, 0) == kCFCompareEqualTo)
        return QAccessible::Description;
#endif
    else
        return -1;
}

/*
    Returns the subrole string constant for the interface if it has one,
    else returns an empty string.
*/
static QCFString subrole(const QAInterface &interface)
{
    const QAInterface parent = interface.parent();
    if (parent.isValid() == false)
        return QCFString();

    if (parent.role() == QAccessible::ScrollBar) {
        QCFString subrole;
        switch(interface.id()) {
            case 1: subrole = kAXDecrementArrowSubrole; break;
            case 2: subrole = kAXDecrementPageSubrole; break;
            case 4: subrole = kAXIncrementPageSubrole; break;
            case 5: subrole = kAXIncrementArrowSubrole; break;
            default:
            break;
        }
        return subrole;
    }
    return QCFString();
}

// Gets the scroll bar orientation by asking the QAbstractSlider object directly.
static Qt::Orientation scrollBarOrientation(const QAInterface &scrollBar)
{
    QObject *const object = scrollBar.object();
    if (QAbstractSlider * const sliderObject = qobject_cast<QAbstractSlider * const>(object))
        return sliderObject->orientation();
    
    return Qt::Vertical; // D'oh! The interface wasn't a scroll bar.
}

static QAInterface scrollAreaGetScrollBarInterface(const QAInterface &scrollArea, Qt::Orientation orientation)
{
    if (macRole(scrollArea) != kAXScrollAreaRole)
        return QAInterface();

    // Child 1 is the contents widget, 2 and 3 are the scroll bar containers wich contains possible scroll bars.
    for (int i = 2; i <= 3; ++i) {
        QAInterface scrollBarContainer = scrollArea.childAt(i);
        for (int i = 1; i <= scrollBarContainer.childCount(); ++i) {
            QAInterface scrollBar = scrollBarContainer.childAt(i);
            if (scrollBar.isValid() &&
                scrollBar.role() == QAccessible::ScrollBar &&
                scrollBarOrientation(scrollBar) == orientation)
                return scrollBar;
        }
    }
    
    return QAInterface();
}

static bool scrollAreaHasScrollBar(const QAInterface &scrollArea, Qt::Orientation orientation)
{
    return scrollAreaGetScrollBarInterface(scrollArea, orientation).isValid();
}

static QAElement scrollAreaGetScrollBar(const QAInterface &scrollArea, Qt::Orientation orientation)
{
    return accessibleHierarchyManager()->lookup(scrollAreaGetScrollBarInterface(scrollArea, orientation));
}

static QAElement scrollAreaGetContents(const QAInterface &scrollArea)
{
    // Child 1 is the contents widget,
    return accessibleHierarchyManager()->lookup(scrollArea.navigate(QAccessible::Child, 1));
}

static QAElement tabWidgetGetContents(const QAInterface &interface)
{   
    // A kAXTabGroup has a kAXContents attribute, which consists of the
    // ui elements for the current tab page. Get the current tab page
    // from the QStackedWidget, where the current visible page can
    // be found at index 1.
    QAInterface stackedWidget = interface.childAt(1);
    accessibleHierarchyManager()->registerChildren(stackedWidget);
    QAInterface tabPageInterface = stackedWidget.childAt(1);
    return accessibleHierarchyManager()->lookup(tabPageInterface);
}

static QList<QAElement> tabBarGetTabs(const QAInterface &interface)
{
    // Get the tabs by searching for children with the "PageTab" role.
    // This filters out the left/right navigation buttons.
    accessibleHierarchyManager()->registerChildren(interface);
    QList<QAElement> tabs;
    const int numChildren = interface.childCount();
    for (int i = 1; i < numChildren + 1; ++i) {
        QAInterface child = interface.navigate(QAccessible::Child, i);
        if (child.isValid() && child.role() == QAccessible::PageTab) {
            tabs.append(accessibleHierarchyManager()->lookup(child));
        }
    }
    return tabs;
}

static QList<QAElement> tabWidgetGetTabs(const QAInterface &interface)
{
    // Each QTabWidget has two children, a QStackedWidget and a QTabBar.
    // Get the tabs from the QTabBar.
    return tabBarGetTabs(interface.childAt(2));
}

static QList<QAElement> tabWidgetGetChildren(const QAInterface &interface)
{
    // The children for a kAXTabGroup should consist of the tabs and the
    // contents of the current open tab page.
    QList<QAElement> children = tabWidgetGetTabs(interface);
    children += tabWidgetGetContents(interface);
    return children;
}

/*
    Returns the label (buddy) interface for interface, or 0 if it has none.
*/
/*
static QAInterface findLabel(const QAInterface &interface)
{
    return interface.navigate(QAccessible::Label, 1);
}
*/
/*
    Returns a list of interfaces this interface labels, or an empty list if it doesn't label any.
*/
/*
static QList<QAInterface> findLabelled(const QAInterface &interface)
{
    QList<QAInterface> interfaceList;

    int count = 1;
    const QAInterface labelled = interface.navigate(QAccessible::Labelled, count);
    while (labelled.isValid()) {
        interfaceList.append(labelled);
        ++count;
    }
    return interfaceList;
}
*/
/*
    Tests if the given QAInterface has data for a mac attribute.
*/
static bool supportsAttribute(CFStringRef attribute, const QAInterface &interface)
{
    const int text = textForRoleAndAttribute(interface.role(), attribute);

    // Special case: Static texts don't have a title.
    if (interface.role() == QAccessible::StaticText && attribute == kAXTitleAttribute)
        return false;

    // Return true if we the attribute matched a QAccessible::Role and we get text for that role from the interface.
    if (text != -1) {
        if (text == QAccessible::Value) // Special case for Value, see getValue()
            return !getValue(interface).isEmpty();
        else
            return !interface.text((QAccessible::Text)text).isEmpty();
    }

    if (CFStringCompare(attribute, kAXChildrenAttribute,  0) == kCFCompareEqualTo) {
        if (interface.childCount() > 0)
            return true;
    }

    if (CFStringCompare(attribute, kAXSubroleAttribute,  0) == kCFCompareEqualTo) {
        return (subrole(interface) != QCFString());
    }

    return false;
}

static void appendIfSupported(CFMutableArrayRef array, CFStringRef attribute, const QAInterface &interface)
{
    if (supportsAttribute(attribute, interface))
        qt_mac_append_cf_uniq(array, attribute);
}

/*
    Returns the names of the attributes the give QAInterface supports.
*/
static OSStatus getAllAttributeNames(EventRef event, const QAInterface &interface, EventHandlerCallRef next_ref)
{
    // Call system event handler.
    OSStatus err = CallNextEventHandler(next_ref, event);
    if(err != noErr && err != eventNotHandledErr)
        return err;
    CFMutableArrayRef attrs = 0;
    GetEventParameter(event, kEventParamAccessibleAttributeNames, typeCFMutableArrayRef, 0,
                      sizeof(attrs), 0, &attrs);

    if (!attrs)
        return eventNotHandledErr;

    // Append attribute names that are always supported.
    qt_mac_append_cf_uniq(attrs, kAXPositionAttribute);
    qt_mac_append_cf_uniq(attrs, kAXSizeAttribute);
    qt_mac_append_cf_uniq(attrs, kAXRoleAttribute);
    qt_mac_append_cf_uniq(attrs, kAXEnabledAttribute);
    qt_mac_append_cf_uniq(attrs, kAXWindowAttribute);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    qt_mac_append_cf_uniq(attrs, kAXTopLevelUIElementAttribute);
#endif

    // Append these names if the QInterafceItem returns any data for them.
    appendIfSupported(attrs, kAXTitleAttribute, interface);
    appendIfSupported(attrs, kAXValueAttribute, interface);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    appendIfSupported(attrs, kAXDescriptionAttribute, interface);
    appendIfSupported(attrs, kAXLinkedUIElementsAttribute, interface);
#endif
    appendIfSupported(attrs, kAXHelpAttribute, interface);
    appendIfSupported(attrs, kAXTitleUIElementAttribute, interface);
    appendIfSupported(attrs, kAXChildrenAttribute, interface);
    appendIfSupported(attrs, kAXSubroleAttribute, interface);

    // Append attribute names based on the interaface role.
    switch (interface.role())  {
        case QAccessible::Window:
            qt_mac_append_cf_uniq(attrs, kAXMainAttribute);
            qt_mac_append_cf_uniq(attrs, kAXMinimizedAttribute);
            qt_mac_append_cf_uniq(attrs, kAXCloseButtonAttribute);
            qt_mac_append_cf_uniq(attrs, kAXZoomButtonAttribute);
            qt_mac_append_cf_uniq(attrs, kAXMinimizeButtonAttribute);
            qt_mac_append_cf_uniq(attrs, kAXToolbarButtonAttribute);
            qt_mac_append_cf_uniq(attrs, kAXGrowAreaAttribute);
        break;
        case QAccessible::RadioButton:
        case QAccessible::CheckBox:
            qt_mac_append_cf_uniq(attrs, kAXMinValueAttribute);
            qt_mac_append_cf_uniq(attrs, kAXMaxValueAttribute);
        break;
        case QAccessible::ScrollBar:
            qt_mac_append_cf_uniq(attrs, kAXOrientationAttribute);
        break;
        case QAccessible::Splitter:
            qt_mac_append_cf_uniq(attrs, kAXSplittersAttribute);
        break;
        case QAccessible::Table:
            qt_mac_append_cf_uniq(attrs, kAXRowsAttribute);
            qt_mac_append_cf_uniq(attrs, kAXVisibleRowsAttribute);
            qt_mac_append_cf_uniq(attrs, kAXSelectedRowsAttribute);
        break;
        default:
        break;
    }

    // Append attribute names based on the mac accessibility role.
    const QCFString mac_role = macRole(interface);
    if (mac_role == kAXSplitterRole) {
        qt_mac_append_cf_uniq(attrs, kAXPreviousContentsAttribute);
        qt_mac_append_cf_uniq(attrs, kAXNextContentsAttribute);
        qt_mac_append_cf_uniq(attrs, kAXOrientationAttribute);
    } else if (mac_role == kAXScrollAreaRole) {
        if (scrollAreaHasScrollBar(interface, Qt::Horizontal))
            qt_mac_append_cf_uniq(attrs, kAXHorizontalScrollBarAttribute);
        if (scrollAreaHasScrollBar(interface, Qt::Vertical))
            qt_mac_append_cf_uniq(attrs, kAXVerticalScrollBarAttribute);
        qt_mac_append_cf_uniq(attrs, kAXContentsAttribute);
    } else if (mac_role == kAXTabGroupRole) {
        qt_mac_append_cf_uniq(attrs, kAXTabsAttribute);
        
        // Only tab widgets can have the contents attribute, there is no way of getting
        // the contents from a QTabBar.
        if (isTabWidget(interface)) 
            qt_mac_append_cf_uniq(attrs, kAXContentsAttribute);
    }

    return noErr;
}

static void handleStringAttribute(EventRef event, QAccessible::Text text, const QAInterface &interface)
{
    QString str = interface.text(text);
    if (str.isEmpty())
        return;

    // Remove any html markup from the text string, or VoiceOver will read the html tags.
    static QTextDocument document;
    document.setHtml(str);
    str = document.toPlainText();

    CFStringRef cfstr = QCFString::toCFStringRef(str);
    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef, sizeof(cfstr), &cfstr);
}

/*
    Handles the parent attribute for a interface.
    There are basically three cases here:
    1. interface is a HIView and has only HIView children.
    2. interface is a HIView but has children that is not a HIView
    3. interface is not a HIView.
*/
static OSStatus handleChildrenAttribute(EventHandlerCallRef next_ref, EventRef event, QAInterface &interface)
{
   // Add the children for this interface to the global QAccessibelHierachyManager.
    accessibleHierarchyManager()->registerChildren(interface);

    if (isTabWidget(interface)) {
        QList<QAElement> children = tabWidgetGetChildren(interface);
        const int childCount = children.count();

        CFMutableArrayRef array = 0;
        array = CFArrayCreateMutable(0, 0, &kCFTypeArrayCallBacks);
        for (int i = 0; i < childCount; ++i)  {
            qt_mac_append_cf_uniq(array, children.at(i).element());
        }

        OSStatus err;
        err = SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFArrayRef, sizeof(array), &array);
        if (err != noErr)
            qWarning("Qt:Internal error (%s:%d)", __FILE__, __LINE__);

        return noErr;
    }

    const QList<QAElement> children = lookup(interface.children());
    const int childCount = children.count();

    OSStatus err = eventNotHandledErr;
    if (interface.isHIView())
        err = CallNextEventHandler(next_ref, event);

    CFMutableArrayRef array = 0;
    int arraySize = 0;
    if (err == noErr) {
        CFTypeRef obj = 0;
        err = GetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, NULL , sizeof(obj), NULL, &obj);
        if (err == noErr && obj != 0) {
            array = (CFMutableArrayRef)obj;
            arraySize = CFArrayGetCount(array);
        }
    }

    if (array) {
        CFArrayRemoveAllValues(array);
        for (int i = 0; i < childCount; ++i)  {
            qt_mac_append_cf_uniq(array, children.at(i).element());
        }
    } else {
        array = CFArrayCreateMutable(0, 0, &kCFTypeArrayCallBacks);
        for (int i = 0; i < childCount; ++i)  {
            qt_mac_append_cf_uniq(array, children.at(i).element());
        }

        err = SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFArrayRef, sizeof(array), &array);
        if (err != noErr)
            qWarning("Qt:Internal error (%s:%d)", __FILE__, __LINE__);
    }

    return noErr;
}

/*

*/
static OSStatus handleParentAttribute(EventHandlerCallRef next_ref, EventRef event, const QAInterface &interface)
{
    OSStatus err = eventNotHandledErr;
    if (interface.isHIView()) {
         err = CallNextEventHandler(next_ref, event);
    }
    if (err == noErr)
        return err;

    const QAInterface parentInterface  = interface.navigate(QAccessible::Ancestor, 1);
    const QAElement parentElement = accessibleHierarchyManager()->lookup(parentInterface);

    if (parentElement.isValid() == false)
        return eventNotHandledErr;

    AXUIElementRef elementRef = parentElement.element();
    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof(elementRef), &elementRef);
    return noErr;
}

struct IsWindowTest
{
    static inline bool test(const QAInterface &interface)
    {
        return (interface.role() == QAccessible::Window);
    }
};

struct IsWindowAndNotDrawerOrSheetTest
{
    static inline bool test(const QAInterface &interface)
    {
        QWidget * const widget = qobject_cast<QWidget*>(interface.object());
        return (interface.role() == QAccessible::Window &&
                widget && widget->isWindow() &&
                !qt_mac_is_macdrawer(widget) &&
                !qt_mac_is_macsheet(widget));
    }
};

/*
    Navigates up the iterfaces ancestor hierachy until a QAccessibleInterface that
    passes the Test is found. If we reach a interface that is a HIView we stop the
    search and call AXUIElementCopyAttributeValue.
*/
template <typename TestType>
OSStatus navigateAncestors(EventHandlerCallRef next_ref, EventRef event, const QAInterface &interface, CFStringRef attribute)
{
    if (interface.isHIView())
        return CallNextEventHandler(next_ref, event);

    QAInterface current = interface;
    QAElement element;
    while (current.isValid()) {
        if (TestType::test(interface)) {
            element = accessibleHierarchyManager()->lookup(current);
            break;
        }

        // If we reach an InterfaceItem that is a HiView we can hand of the search to
        // the system event handler. This is the common case.
        if (current.isHIView()) {
            CFTypeRef value = 0;
            const QAElement currentElement = accessibleHierarchyManager()->lookup(current);
            AXError err = AXUIElementCopyAttributeValue(currentElement.element(), attribute, &value);
            AXUIElementRef newElement = (AXUIElementRef)value;

            if (err == noErr)
                element = QAElement(newElement);

            if (newElement != 0)
                CFRelease(newElement);
            break;
        }

        QAInterface next = current.parent();
        if (next.isValid() == false)
            break;
        if (next == current)
            break;
        current = next;
    }

    if (element.isValid() == false)
        return eventNotHandledErr;

    
    AXUIElementRef elementRef = element.element();
    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef,
                                      sizeof(elementRef), &elementRef);
    return noErr;
}

/*
    Returns the top-level window for an interface, which is the closest ancestor interface that
    has the Window role, but is not a sheet or a drawer.
*/
static OSStatus handleWindowAttribute(EventHandlerCallRef next_ref, EventRef event, const QAInterface &interface)
{
    return navigateAncestors<IsWindowAndNotDrawerOrSheetTest>(next_ref, event, interface, kAXWindowAttribute);
}

/*
    Returns the top-level window for an interface, which is the closest ancestor interface that
    has the Window role. (Can also be a sheet or a drawer)
*/
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
static OSStatus handleTopLevelUIElementAttribute(EventHandlerCallRef next_ref, EventRef event, const QAInterface &interface)
{
    return navigateAncestors<IsWindowTest>(next_ref, event, interface, kAXTopLevelUIElementAttribute);
}
#endif

/*
    Returns the tab buttons for an interface.
*/
static OSStatus handleTabsAttribute(EventHandlerCallRef next_ref, EventRef event, QAInterface &interface)
{
    Q_UNUSED(next_ref);
    if (isTabWidget(interface))
        return setAttributeValue(event, tabWidgetGetTabs(interface));
    else
        return setAttributeValue(event, tabBarGetTabs(interface));
}

static OSStatus handlePositionAttribute(EventHandlerCallRef, EventRef event, const QAInterface &interface)
{
    QPoint qpoint(interface.rect().topLeft());
    HIPoint point;
    point.x = qpoint.x();
    point.y = qpoint.y();
    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeHIPoint, sizeof(point), &point);
    return noErr;
}

static OSStatus handleSizeAttribute(EventHandlerCallRef, EventRef event, const QAInterface &interface)
{
    QSize qSize(interface.rect().size());
    HISize size;
    size.width = qSize.width();
    size.height = qSize.height();
    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeHISize, sizeof(size), &size);
    return noErr;
}

static OSStatus handleSubroleAttribute(EventHandlerCallRef, EventRef event, const QAInterface &interface)
{
    const QCFString role = subrole(interface);
    CFStringRef rolestr = (CFStringRef)role;
    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof(rolestr), &rolestr);
    return noErr;
}

static OSStatus handleOrientationAttribute(EventHandlerCallRef next_ref, EventRef event, const QAInterface &interface)
{
    QObject *const object = interface.object();
    Qt::Orientation orientation;
    if (interface.role() == QAccessible::ScrollBar) {
        orientation  = scrollBarOrientation(interface);
    } else if (QSplitterHandle * const splitter = qobject_cast<QSplitterHandle * const>(object)) {
        // Qt reports the layout orientation, but we want the splitter handle orientation.
        orientation = (splitter->orientation() == Qt::Horizontal) ? Qt::Vertical : Qt::Horizontal;
    } else {
        return CallNextEventHandler(next_ref, event);
    }
    const CFStringRef orientationString = (orientation == Qt::Vertical)
        ? kAXVerticalOrientationValue : kAXHorizontalOrientationValue;
    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef, sizeof(orientationString), &orientationString);
    return noErr;
}

/*
    Figures out the next or previous contents for a splitter.
*/
static OSStatus handleSplitterContentsAttribute(EventHandlerCallRef next_ref, EventRef event, const QAInterface &interface, QCFString nextOrPrev)
{
    if (interface.isValid() == false || interface.role() != QAccessible::Grip)
        return eventNotHandledErr;

    const QAInterface parent = interface.parent();
    if (parent.isValid() == false)
        return CallNextEventHandler(next_ref, event);

    if (parent.role() != QAccessible::Splitter)
        return CallNextEventHandler(next_ref, event);

    const QSplitter * const splitter = qobject_cast<const QSplitter * const>(parent.object());
    if (splitter == 0)
        return CallNextEventHandler(next_ref, event);

    QWidget * const splitterHandle = qobject_cast<QWidget * const>(interface.object());
    const int splitterHandleIndex = splitter->indexOf(splitterHandle);
    const int widgetIndex = (nextOrPrev == QCFString(kAXPreviousContentsAttribute)) ? splitterHandleIndex - 1 : splitterHandleIndex;
    const QAElement contentsElement = accessibleHierarchyManager()->lookup(splitter->widget(widgetIndex), 0);
    return setAttributeValue(event, QList<QAElement>() << contentsElement);
}

/*
    Creates a list of all splitter handles the splitter contains.
*/
static OSStatus handleSplittersAttribute(EventHandlerCallRef next_ref, EventRef event, QAInterface &interface)
{
    const QSplitter * const splitter = qobject_cast<const QSplitter * const>(interface.object());
    if (splitter == 0)
        return CallNextEventHandler(next_ref, event);

    accessibleHierarchyManager()->registerChildren(interface);

    QList<QAElement> handles;
    const int visibleSplitterCount = splitter->count() -1; // skip first handle, it's always invisible.
    for (int i = 0; i < visibleSplitterCount; ++i)
        handles.append(accessibleHierarchyManager()->lookup(splitter->handle(i + 1), 0));
    
    return setAttributeValue(event, handles);
}

// This handler gets the scroll bars for a scroll area
static OSStatus handleScrollBarAttribute(EventHandlerCallRef next_ref, EventRef event, QAInterface &scrollArea, Qt::Orientation orientation)
{
    QAElement scrollBar = scrollAreaGetScrollBar(scrollArea, orientation);
    if (scrollBar.isValid() == false)
        return CallNextEventHandler(next_ref, event);

    AXUIElementRef elementRef = scrollBar.element();
    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof(elementRef), &elementRef);
    return noErr;
}

// This handler gets the contents for a scroll area or tab widget.
static OSStatus handleContentsAttribute(EventHandlerCallRef next_ref, EventRef event, QAInterface &interface)
{
    const QCFString mac_role = macRole(interface);

    QAElement contents;

    if (mac_role == kAXTabGroupRole) {
        contents = tabWidgetGetContents(interface);
    } else {
        contents = scrollAreaGetContents(interface);
        if (contents.isValid() == false)
            return CallNextEventHandler(next_ref, event);
    }

    return setAttributeValue(event, QList<QAElement>() << contents);
}

static OSStatus handleRowsAttribute(EventHandlerCallRef, EventRef event, QAInterface &tableView)
{
    QList<QAElement> rows = lookup(tableView.children());

    // kill the first row which is the horizontal header.
    rows.removeAt(0);
 
    return setAttributeValue(event, rows);
}

static OSStatus handleVisibleRowsAttribute(EventHandlerCallRef, EventRef event, QAInterface &tableView)
{
    QList<QAElement> visibleRows;

    QList<QAInterface> rows = tableView.children();
    // kill the first row which is the horizontal header.
    rows.removeAt(0);
    
    foreach (const QAInterface &interface, rows)
        if ((interface.state() & QAccessible::Invisible) == false)
            visibleRows.append(accessibleHierarchyManager()->lookup(interface));

    return setAttributeValue(event, visibleRows);
}

static OSStatus handleSelectedRowsAttribute(EventHandlerCallRef, EventRef event, QAInterface &tableView)
{
    QList<QAElement> selectedRows;
    foreach (const QAInterface &interface, tableView.children())
        if ((interface.state() & QAccessible::Selected))
            selectedRows.append(accessibleHierarchyManager()->lookup(interface));

    return setAttributeValue(event, selectedRows);
}

static OSStatus getNamedAttribute(EventHandlerCallRef next_ref, EventRef event, QAInterface &interface)
{
    CFStringRef var;
    GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, 0,
                              sizeof(var), 0, &var);

    if (CFStringCompare(var, kAXChildrenAttribute, 0) == kCFCompareEqualTo) {
        return handleChildrenAttribute(next_ref, event, interface);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    } else if(CFStringCompare(var, kAXTopLevelUIElementAttribute, 0) == kCFCompareEqualTo) {
        return handleTopLevelUIElementAttribute(next_ref, event, interface);
#endif
    } else if(CFStringCompare(var, kAXWindowAttribute, 0) == kCFCompareEqualTo) {
        return handleWindowAttribute(next_ref, event, interface);
    } else if(CFStringCompare(var, kAXParentAttribute, 0) == kCFCompareEqualTo) {
        return handleParentAttribute(next_ref, event, interface);
    } else if (CFStringCompare(var, kAXPositionAttribute, 0) == kCFCompareEqualTo) {
        return handlePositionAttribute(next_ref, event, interface);
    } else if (CFStringCompare(var, kAXSizeAttribute, 0) == kCFCompareEqualTo) {
        return handleSizeAttribute(next_ref, event, interface);
    } else  if (CFStringCompare(var, kAXRoleAttribute, 0) == kCFCompareEqualTo) {
        CFStringRef role = macRole(interface);
// ###
//        QWidget * const widget = qobject_cast<QWidget *>(interface.object());
//        if (role == kAXUnknownRole && widget && widget->isWindow())
//            role = kAXWindowRole;

        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef,
                          sizeof(role), &role);

    } else if (CFStringCompare(var, kAXEnabledAttribute, 0) == kCFCompareEqualTo) {
        Boolean val =  !((interface.state() & QAccessible::Unavailable))
                     && !((interface.state() & QAccessible::Invisible));
        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                          sizeof(val), &val);
    } else if (CFStringCompare(var, kAXExpandedAttribute, 0) == kCFCompareEqualTo) {
        Boolean val = (interface.state() & QAccessible::Expanded);
        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                          sizeof(val), &val);
    } else if (CFStringCompare(var, kAXSelectedAttribute, 0) == kCFCompareEqualTo) {
        Boolean val = (interface.state() & QAccessible::Selection);
        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                          sizeof(val), &val);
    } else if (CFStringCompare(var, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
        Boolean val = (interface.state() & QAccessible::Focus);
        SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                          sizeof(val), &val);
    } else if (CFStringCompare(var, kAXSelectedChildrenAttribute, 0) == kCFCompareEqualTo) {
        const int cc = interface.childCount();
        QList<QAElement> selected;
        for (int i = 1; i <= cc; ++i) {
            const QAInterface child_iface = interface.navigate(QAccessible::Child, i);
            if (child_iface.isValid() && child_iface.state() & QAccessible::Selected)
                selected.append(accessibleHierarchyManager()->lookup(child_iface));
        }
        
        return setAttributeValue(event, selected);

      } else if (CFStringCompare(var, kAXCloseButtonAttribute, 0) == kCFCompareEqualTo) {
        if(interface.object() && interface.object()->isWidgetType()) {
            Boolean val = true; //do we want to add a WState for this?
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                              sizeof(val), &val);
        }
    } else if (CFStringCompare(var, kAXZoomButtonAttribute, 0) == kCFCompareEqualTo) {
        if(interface.object() && interface.object()->isWidgetType()) {
            QWidget *widget = (QWidget*)interface.object();
            Boolean val = (widget->windowFlags() & Qt::WindowMaximizeButtonHint);
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                              sizeof(val), &val);
        }
    } else if (CFStringCompare(var, kAXMinimizeButtonAttribute, 0) == kCFCompareEqualTo) {
        if(interface.object() && interface.object()->isWidgetType()) {
            QWidget *widget = (QWidget*)interface.object();
            Boolean val = (widget->windowFlags() & Qt::WindowMinimizeButtonHint);
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                              sizeof(val), &val);
        }
    } else if (CFStringCompare(var, kAXToolbarButtonAttribute, 0) == kCFCompareEqualTo) {
        if(interface.object() && interface.object()->isWidgetType()) {
            QWidget *widget = (QWidget*)interface.object();
            Boolean val = qobject_cast<QMainWindow *>(widget) != 0;
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                              sizeof(val), &val);
        }
    } else if (CFStringCompare(var, kAXGrowAreaAttribute, 0) == kCFCompareEqualTo) {
        if(interface.object() && interface.object()->isWidgetType()) {
            Boolean val = true; //do we want to add a WState for this?
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                              sizeof(val), &val);
        }
    } else if (CFStringCompare(var, kAXMinimizedAttribute, 0) == kCFCompareEqualTo) {
        if (interface.object() && interface.object()->isWidgetType()) {
            QWidget *widget = (QWidget*)interface.object();
            Boolean val = (widget->windowState() & Qt::WindowMinimized);
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeBoolean,
                              sizeof(val), &val);
        }
    } else if (CFStringCompare(var, kAXSubroleAttribute, 0) == kCFCompareEqualTo) {
        return handleSubroleAttribute(next_ref, event, interface);
    } else if (CFStringCompare(var, kAXRoleDescriptionAttribute, 0) == kCFCompareEqualTo) {
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
        if (HICopyAccessibilityRoleDescription) {
            const CFStringRef roleDescription = HICopyAccessibilityRoleDescription(macRole(interface), 0);
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFStringRef,
                          sizeof(roleDescription), &roleDescription);
        } else
#endif
        {
            // Just use Qt::Description on 10.3
            handleStringAttribute(event, QAccessible::Description, interface);
        }
    } else if (CFStringCompare(var, kAXTitleAttribute, 0) == kCFCompareEqualTo) {
        const QAccessible::Role role = interface.role();
        const QAccessible::Text text = (QAccessible::Text)textForRoleAndAttribute(role, var);
        handleStringAttribute(event, text, interface);
    } else if (CFStringCompare(var, kAXValueAttribute, 0) == kCFCompareEqualTo) {
        const QAccessible::Role role = interface.role();
        const QAccessible::Text text = (QAccessible::Text)textForRoleAndAttribute(role, var);
        if (role == QAccessible::CheckBox || role == QAccessible::RadioButton) {
            int value = buttonValue(interface);
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeUInt32, sizeof(value), &value);
        } else {
            handleStringAttribute(event, text, interface);
        }
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    } else if (CFStringCompare(var, kAXDescriptionAttribute, 0) == kCFCompareEqualTo) {
        const QAccessible::Role role = interface.role();
        const QAccessible::Text text = (QAccessible::Text)textForRoleAndAttribute(role, var);
        handleStringAttribute(event, text, interface);
    } else if (CFStringCompare(var, kAXLinkedUIElementsAttribute, 0) == kCFCompareEqualTo) {
        return CallNextEventHandler(next_ref, event);
#endif
    } else if (CFStringCompare(var, kAXHelpAttribute, 0) == kCFCompareEqualTo) {
        const QAccessible::Role role = interface.role();
        const QAccessible::Text text = (QAccessible::Text)textForRoleAndAttribute(role, var);
        handleStringAttribute(event, text, interface);
    } else if (CFStringCompare(var, kAXTitleUIElementAttribute, 0) == kCFCompareEqualTo) {
        return CallNextEventHandler(next_ref, event);
    } else if (CFStringCompare(var, kAXTabsAttribute, 0) == kCFCompareEqualTo) {
        return handleTabsAttribute(next_ref, event, interface);
    } else if (CFStringCompare(var, kAXMinValueAttribute, 0) == kCFCompareEqualTo) {
        if (interface.role() == QAccessible::RadioButton || interface.role() == QAccessible::CheckBox) {
            uint value = 0;
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeUInt32, sizeof(value), &value);
        } else {
            return CallNextEventHandler(next_ref, event);
        }
    } else if (CFStringCompare(var, kAXMaxValueAttribute, 0) == kCFCompareEqualTo) {
        if (interface.role() == QAccessible::RadioButton || interface.role() == QAccessible::CheckBox) {
            uint value = 2;
            SetEventParameter(event, kEventParamAccessibleAttributeValue, typeUInt32, sizeof(value), &value);
        } else {
            return CallNextEventHandler(next_ref, event);
        }
    } else if (CFStringCompare(var, kAXOrientationAttribute, 0) == kCFCompareEqualTo) {
        return handleOrientationAttribute(next_ref, event, interface);
    } else if (CFStringCompare(var, kAXPreviousContentsAttribute, 0) == kCFCompareEqualTo) {
        return handleSplitterContentsAttribute(next_ref, event, interface, kAXPreviousContentsAttribute);
    } else if (CFStringCompare(var, kAXNextContentsAttribute, 0) == kCFCompareEqualTo) {
        return handleSplitterContentsAttribute(next_ref, event, interface, kAXNextContentsAttribute);
    } else if (CFStringCompare(var, kAXSplittersAttribute, 0) == kCFCompareEqualTo) {
        return handleSplittersAttribute(next_ref, event, interface);
    } else if (CFStringCompare(var, kAXHorizontalScrollBarAttribute, 0) == kCFCompareEqualTo) {
        return handleScrollBarAttribute(next_ref, event, interface, Qt::Horizontal);
    } else if (CFStringCompare(var, kAXVerticalScrollBarAttribute, 0) == kCFCompareEqualTo) {
        return handleScrollBarAttribute(next_ref, event, interface, Qt::Vertical);
    } else if (CFStringCompare(var, kAXContentsAttribute, 0) == kCFCompareEqualTo) {
        return handleContentsAttribute(next_ref, event, interface);
    } else if (CFStringCompare(var, kAXRowsAttribute, 0) == kCFCompareEqualTo) {
        return handleRowsAttribute(next_ref, event, interface);
    } else if (CFStringCompare(var, kAXVisibleRowsAttribute, 0) == kCFCompareEqualTo) {
        return handleVisibleRowsAttribute(next_ref, event, interface);
    } else if (CFStringCompare(var, kAXSelectedRowsAttribute, 0) == kCFCompareEqualTo) {
        return handleSelectedRowsAttribute(next_ref, event, interface);
    } else {
        return CallNextEventHandler(next_ref, event);
    }
    return noErr;
}

static OSStatus isNamedAttributeSettable(EventRef event, const QAInterface &interface)
{
    CFStringRef var;
    GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, 0,
                      sizeof(var), 0, &var);
    Boolean settable = false;
    if (CFStringCompare(var, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
        settable = true;
    } else {
        for (int r = 0; text_bindings[r][0].qt != -1; r++) {
            if(interface.role() == (QAccessible::Role)text_bindings[r][0].qt) {
                for (int a = 1; text_bindings[r][a].qt != -1; a++) {
                    if (CFStringCompare(var, text_bindings[r][a].mac, 0) == kCFCompareEqualTo) {
                        settable = text_bindings[r][a].settable;
                        break;
                    }
                }
            }
        }
    }
    SetEventParameter(event, kEventParamAccessibleAttributeSettable, typeBoolean,
                      sizeof(settable), &settable);
    return noErr;
}

static OSStatus getChildAtPoint(EventHandlerCallRef next_ref, EventRef event, QAInterface &interface)
{
    Q_UNUSED(next_ref);
    if (interface.isValid() == false)
        return eventNotHandledErr;

    // Add the children for this interface to the global QAccessibelHierachyManager.
    accessibleHierarchyManager()->registerChildren(interface);

    Point where;
    GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, 0, sizeof(where), 0, &where);
    const QAInterface childInterface = interface.childAt(where.h, where.v);

    if (childInterface.isValid() == false || childInterface == interface)
        return eventNotHandledErr;

    const QAElement element = accessibleHierarchyManager()->lookup(childInterface);
    if (element.isValid() == false)
        return eventNotHandledErr;

    AXUIElementRef elementRef = element.element();
    CFRetain(elementRef);
    SetEventParameter(event, kEventParamAccessibleChild, typeCFTypeRef,
                                  sizeof(elementRef), &elementRef);

    return noErr;
}

/*
    Returns a list of actions the given interface supports.
    Currently implemented by getting the interface role and deciding based on that.
*/
static QList<QAccessible::Action> supportedPredefinedActions(const QAInterface &interface)
{
    QList<QAccessible::Action> actions;
    switch (interface.role()) {
        default:
            // Most things can be pressed.
            actions.append(QAccessible::Press);
        break;
    }

    return actions;
}

/*
    Translates a predefined QAccessible::Action to a Mac action constant.
    Returns an empty string if the Qt Action has no mac equivalent.
*/
static QCFString translateAction(const QAccessible::Action action)
{
    switch (action) {
        case QAccessible::Press:
            return kAXPressAction;
        break;
        case QAccessible::Increase:
            return kAXIncrementAction;
        break;
        case QAccessible::Decrease:
            return kAXDecrementAction;
        break;
        case QAccessible::Accept:
            return kAXConfirmAction;
        break;
        case QAccessible::Select:
            return kAXPickAction;
        break;
        case QAccessible::Cancel:
            return kAXCancelAction;
        break;
        default:
            return QCFString();
        break;
    }
}

/*
    Translates between a Mac action constant and a QAccessible::Action.
    Returns QAccessible::Default action if there is no Qt predefined equivalent.
*/
static QAccessible::Action translateAction(const CFStringRef actionName)
{
    if(CFStringCompare(actionName, kAXPressAction, 0) == kCFCompareEqualTo) {
        return QAccessible::Press;
    } else if(CFStringCompare(actionName, kAXIncrementAction, 0) == kCFCompareEqualTo) {
        return QAccessible::Increase;
    } else if(CFStringCompare(actionName, kAXDecrementAction, 0) == kCFCompareEqualTo) {
        return QAccessible::Decrease;
    } else if(CFStringCompare(actionName, kAXConfirmAction, 0) == kCFCompareEqualTo) {
        return QAccessible::Accept;
    } else if(CFStringCompare(actionName, kAXPickAction, 0) == kCFCompareEqualTo) {
        return QAccessible::Select;
    } else if(CFStringCompare(actionName, kAXCancelAction, 0) == kCFCompareEqualTo) {
        return QAccessible::Cancel;
    } else {
        return QAccessible::DefaultAction;
    }
}

/*
    Copies the translated names all supported actions for an interface into the kEventParamAccessibleActionNames
    event parameter.
*/
static OSStatus getAllActionNames(EventHandlerCallRef next_ref, EventRef event, const QAInterface &interface)
{
    Q_UNUSED(next_ref);

    CFMutableArrayRef actions = 0;
    GetEventParameter(event, kEventParamAccessibleActionNames, typeCFMutableArrayRef, 0,
                      sizeof(actions), 0, &actions);

    // Add supported predefined actions.
    const QList<QAccessible::Action> predefinedActions = supportedPredefinedActions(interface);
    for (int i = 0; i < predefinedActions.count(); ++i) {
        const QCFString action = translateAction(predefinedActions.at(i));
        if (action != QCFString())
            qt_mac_append_cf_uniq(actions, action);
    }

    // Add user actions
    const int actionCount = interface.userActionCount();
    for (int i = 0; i < actionCount; ++i) {
        const QString actionName = interface.actionText(i, QAccessible::Name);
        qt_mac_append_cf_uniq(actions, QCFString::toCFStringRef(actionName));
    }

    return noErr;
}

/*
    Handles the perforNamedAction event.
*/
static OSStatus performNamedAction(EventHandlerCallRef next_ref, EventRef event, const QAInterface& interface)
{
    Q_UNUSED(next_ref);

    CFStringRef act;
    GetEventParameter(event, kEventParamAccessibleActionName, typeCFStringRef, 0,
                      sizeof(act), 0, &act);

    const QAccessible::Action action = translateAction(act);

    // Perform built-in action
    if (action != QAccessible::DefaultAction) {
        interface.doAction(action, QVariantList());
        return noErr;
    }

    // Search for user-defined actions and perform it if found.
    const int actCount = interface.userActionCount();
    const QString qAct = QCFString::toQString(act);
    for(int i = 0; i < actCount; i++) {
        if(interface.actionText(i, QAccessible::Name) == qAct) {
            interface.doAction(i, QVariantList());
            break;
        }
    }
    return noErr;
}

static OSStatus setNamedAttribute(EventHandlerCallRef next_ref, EventRef event, const QAInterface &interface)
{
    Q_UNUSED(next_ref);
    Q_UNUSED(event);

    CFStringRef var;
    GetEventParameter(event, kEventParamAccessibleAttributeName, typeCFStringRef, 0,
                      sizeof(var), 0, &var);
    if(CFStringCompare(var, kAXFocusedAttribute, 0) == kCFCompareEqualTo) {
        CFTypeRef val;
        if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, 0,
                             sizeof(val), 0, &val) == noErr) {
            if(CFGetTypeID(val) == CFBooleanGetTypeID() &&
               CFEqual(static_cast<CFBooleanRef>(val), kCFBooleanTrue)) {
                interface.doAction(QAccessible::SetFocus);
            }
        }
    } else {
        bool found = false;
        for(int r = 0; text_bindings[r][0].qt != -1; r++) {
            if(interface.role() == (QAccessible::Role)text_bindings[r][0].qt) {
                for(int a = 1; text_bindings[r][a].qt != -1; a++) {
                    if(CFStringCompare(var, text_bindings[r][a].mac, 0) == kCFCompareEqualTo) {
                        if(!text_bindings[r][a].settable) {
                        } else {
                            CFTypeRef val;
                            if(GetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef, 0,
                                                 sizeof(val), 0, &val) == noErr) {
                                if(CFGetTypeID(val) == CFStringGetTypeID())
                                    interface.setText((QAccessible::Text)text_bindings[r][a].qt,
                                                   QCFString::toQString(static_cast<CFStringRef>(val)));

                            }
                        }
                        found = true;
                        break;
                    }
                }
                break;
            }
        }
    }
    return noErr;
}

/*
    This is the main accessibility event handler.
*/
static OSStatus accessibilityEventHandler(EventHandlerCallRef next_ref, EventRef event, void *data)
{
    Q_UNUSED(data)

    // Return if this event is not a AccessibleGetNamedAttribute event.
    const UInt32 eclass = GetEventClass(event);
    if (eclass != kEventClassAccessibility)
        return eventNotHandledErr;

    // Get the AXUIElementRef and QAInterface pointer
    AXUIElementRef element = 0;
    GetEventParameter(event, kEventParamAccessibleObject, typeCFTypeRef, 0, sizeof(element), 0, &element);
    QAInterface interface = accessibleHierarchyManager()->lookup(element);
    if (interface.isValid() == false)
        return eventNotHandledErr;

    const UInt32 ekind = GetEventKind(event);
    OSStatus status = noErr;
    switch (ekind) {
        case kEventAccessibleGetAllAttributeNames:
             status = getAllAttributeNames(event, interface, next_ref);
        break;
        case kEventAccessibleGetNamedAttribute:
             status = getNamedAttribute(next_ref, event, interface);
        break;
        case kEventAccessibleIsNamedAttributeSettable:
             status = isNamedAttributeSettable(event, interface);
        break;
        case kEventAccessibleGetChildAtPoint:
            status = getChildAtPoint(next_ref, event, interface);
        break;
        case kEventAccessibleGetAllActionNames:
            status = getAllActionNames(next_ref, event, interface);
        break;
        case kEventAccessibleGetFocusedChild:
            status = CallNextEventHandler(next_ref, event);
        break;
        case kEventAccessibleSetNamedAttribute:
            status = setNamedAttribute(next_ref, event, interface);
        break;
        case kEventAccessiblePerformNamedAction:
            status = performNamedAction(next_ref, event, interface);
        break;
        default:
            status = CallNextEventHandler(next_ref, event);
        break;
    };
    return status;
}

void QAccessible::initialize()
{
    registerQtAccessibilityHIObjectSubclass();
    installApplicationEventhandler();
}

// Sets thre root object for the application
void QAccessible::setRootObject(QObject *object)
{
    // Call installed root object handler if we have one
    if (rootObjectHandler) {
        rootObjectHandler(object);
        return;
    }

    rootObject = object;
}

void QAccessible::cleanup()
{
    accessibleHierarchyManager()->reset();
    removeEventhandler(applicationEventHandlerUPP);
    removeEventhandler(objectCreateEventHandlerUPP);
    removeEventhandler(accessibilityEventHandlerUPP);
}

void QAccessible::updateAccessibility(QObject *object, int child, Event reason)
{
    // Call installed update handler if we have one.
    if (updateHandler) {
        updateHandler(object, child, reason);
        return;
    }

    // Return if the mac accessibility is not enabled.
    if(!AXAPIEnabled())
        return;

     // Work around crash, disable accessiblity for focus frames.
     if (qstrcmp(object->metaObject()->className(), "QFocusFrame") == 0)
        return;

//    qDebug() << "updateAccessibility" << object << child << hex << reason;

    if (reason == ObjectShow) {
        QAInterface interface = QAInterface(QAccessible::queryAccessibleInterface(object), child);
        accessibleHierarchyManager()->registerInterface(interface);
    }

    const QAElement element = accessibleHierarchyManager()->lookup(object, child);
    if (element.isValid() == false)
        return;

    CFStringRef notification = 0;
    if(object && object->isWidgetType() && reason == ObjectCreated) {
        notification = kAXWindowCreatedNotification;
    } else if(reason == ValueChanged) {
        notification = kAXValueChangedNotification;
    } else if(reason == MenuStart) {
        notification = kAXMenuOpenedNotification;
    } else if(reason == MenuEnd) {
        notification = kAXMenuClosedNotification;
    } else if(reason == LocationChanged) {
        notification = kAXWindowMovedNotification;
    } else if(reason == ObjectShow || reason == ObjectHide ) {
        // When a widget is deleted we get a ObjectHide before the destroyed(QObject *)
        // signal is emitted (which makes sense). However, at this point we are in the
        // middle of the QWidget destructor which means that we have to be careful when
        // using the widget pointer. Since we can't control what the accessibilty interfaces
        // does when navigate() is called below we ignore the hide update in this case.
        // (the widget will be deleted soon anyway.)
        extern QWidgetPrivate * qt_widget_private(QWidget *);
        if (QWidget *widget = qobject_cast<QWidget*>(object)) {
            if (qt_widget_private(widget)->data.in_destructor)
                return;

            // Check widget parent as well, special case for preventing crash
            // when the viewport() of an abstract scroll area is hidden when
            // the QWidget destructor hides all its children.
            QWidget *parentWidget = widget->parentWidget();
            if (parentWidget && qt_widget_private(parentWidget)->data.in_destructor)
                return;
        }

        // There is no equivalent Mac notification for ObjectShow/Hide, so we call HIObjectSetAccessibilityIgnored
        // and isItIntersting which will mark the HIObject accociated with the element as ignored if the
        // QAccessible::Invisible state bit is set.
        QAInterface interface = accessibleHierarchyManager()->lookup(element);
        if (interface.isValid()) {
            HIObjectSetAccessibilityIgnored(element.object(), !isItInteresting(interface));
        }

        // If the interface manages its own children, also check if we should ignore those.
        if (isItemView(interface) == false && managesChildren(interface)) {
            for (int i = 1; i <= interface.childCount(); ++i) {
                QAInterface childInterface = interface.navigate(QAccessible::Child, i);
                if (childInterface.isValid() && childInterface.isHIView() == false) {
                    const QAElement element = accessibleHierarchyManager()->lookup(childInterface);
                    if (element.isValid()) {
                        HIObjectSetAccessibilityIgnored(element.object(), !isItInteresting(childInterface));
                    }
                }
            }
        }

    } else if(reason == Focus) {
        if(object && object->isWidgetType()) {
            QWidget *w = static_cast<QWidget*>(object);
            if(w->isWindow())
                notification = kAXFocusedWindowChangedNotification;
            else
                notification = kAXFocusedUIElementChangedNotification;
        }
    }

    if (!notification)
        return;

    AXNotificationHIObjectNotify(notification, element.object(), element.id());
}

QT_END_NAMESPACE

#endif // QT_NO_ACCESSIBILITY
