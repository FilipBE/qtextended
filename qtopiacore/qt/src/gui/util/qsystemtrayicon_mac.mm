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

#define QT_MAC_SYSTEMTRAY_USE_GROWL

@class QNSMenu;

#include "qsystemtrayicon_p.h"
#include <qtemporaryfile.h>
#include <qimagewriter.h>
#include <qapplication.h>
#include <qdebug.h>
#include <qstyle.h>

#include <private/qt_mac_p.h>
#import <AppKit/AppKit.h>

QT_BEGIN_NAMESPACE
extern bool qt_mac_execute_apple_script(const QString &script, AEDesc *ret); //qapplication_mac.cpp
extern void qtsystray_sendActivated(QSystemTrayIcon *i, int r); //qsystemtrayicon.cpp
extern void qt_mac_get_accel(quint32 accel_key, quint32 *modif, quint32 *key); //qmenu_mac.cpp
extern QString qt_mac_no_ampersands(QString str); //qmenu_mac.cpp
QT_END_NAMESPACE

QT_USE_NAMESPACE

@class QNSImageView;

@interface QNSStatusItem : NSObject {
    NSStatusItem *item;
    QSystemTrayIcon *icon;
    QNSImageView *imageCell;
}
-(id)initWithIcon:(QSystemTrayIcon*)icon;
-(void)dealloc;
-(QSystemTrayIcon*)icon;
-(NSStatusItem*)item;
-(QRectF)geometry;
- (void)triggerSelector:(id)sender;
- (void)doubleClickSelector:(id)sender;
@end

@interface QNSImageView : NSImageView {
    BOOL down;
    QNSStatusItem *parent;
}
-(id)initWithParent:(QNSStatusItem*)myParent;
-(QSystemTrayIcon*)icon;
-(void)mouseDown:(NSEvent *)mouseEvent;
-(void)drawRect:(NSRect)rect;
-(void)menuTrackingDone:(NSNotification*)notification;
@end

@interface QNSMenu : NSMenu {
    QMenu *qmenu;
}
-(QMenu*)menu;
-(id)initWithQMenu:(QMenu*)qmenu;
-(void)menuNeedsUpdate:(QNSMenu*)menu;
-(void)selectedAction:(id)item;
@end

QT_BEGIN_NAMESPACE
void qt_mac_trayicon_activate_action(QMenu *menu, QAction *action)
{
    emit menu->triggered(action);
}

NSImage *qt_mac_create_ns_image(const QPixmap &pm)
{
    QMacCocoaAutoReleasePool pool;
    if(CGImageRef image = pm.toMacCGImageRef()) {
        NSRect imageRect = NSMakeRect(0.0, 0.0, CGImageGetWidth(image), CGImageGetHeight(image));
        NSImage *newImage = [[NSImage alloc] initWithSize:imageRect.size];
        [newImage lockFocus];
        {
            CGContextRef imageContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
            CGContextDrawImage(imageContext, *(CGRect*)&imageRect, image);
        }
        [newImage unlockFocus];
        return newImage;
    }
    return 0;
}

class QSystemTrayIconSys
{
public:
    QSystemTrayIconSys(QSystemTrayIcon *icon) {
        QMacCocoaAutoReleasePool pool;
        item = [[QNSStatusItem alloc] initWithIcon:icon];
    }
    ~QSystemTrayIconSys() {
        QMacCocoaAutoReleasePool pool;
        [[[item item] view] setHidden: YES];
        [item release];
    }
    QNSStatusItem *item;
};

void QSystemTrayIconPrivate::install_sys()
{
    Q_Q(QSystemTrayIcon);
    if (!sys) {
        sys = new QSystemTrayIconSys(q);
        updateIcon_sys();
        updateMenu_sys();
        updateToolTip_sys();
    }
}

QRect QSystemTrayIconPrivate::geometry_sys() const
{
    if(sys) {
        const QRectF geom = [sys->item geometry];
        if(!geom.isNull())
            return geom.toRect();
    }
    return QRect();
}

void QSystemTrayIconPrivate::remove_sys()
{
    delete sys;
    sys = 0;
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
    if(sys && !icon.isNull()) {
        QMacCocoaAutoReleasePool pool;
        const short scale = GetMBarHeight()-4;
        NSImage *nsimage = qt_mac_create_ns_image(icon.pixmap(QSize(scale, scale)));
        [(NSImageView*)[[sys->item item] view] setImage: nsimage];
        [nsimage release];
    }
}

void QSystemTrayIconPrivate::updateMenu_sys()
{
    if(sys) {
        QMacCocoaAutoReleasePool pool;
        if(menu && !menu->isEmpty()) {
            [[sys->item item] setHighlightMode:YES];
        } else {
            [[sys->item item] setHighlightMode:NO];
        }
    }
}

void QSystemTrayIconPrivate::updateToolTip_sys()
{
    if(sys) {
        QMacCocoaAutoReleasePool pool;
        QCFString string(toolTip);
        [(NSImageView*)[[sys->item item] view] setToolTip:(NSString*)static_cast<CFStringRef>(string)];
    }
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
    return true;
}

void QSystemTrayIconPrivate::showMessage_sys(const QString &title, const QString &message, QSystemTrayIcon::MessageIcon icon, int)
{

    if(sys) {
#ifdef QT_MAC_SYSTEMTRAY_USE_GROWL
        // Make sure that we have Growl installed on the machine we are running on.
        QCFType<CFURLRef> cfurl;
        OSStatus status = LSGetApplicationForInfo(kLSUnknownType, kLSUnknownCreator,
                                                  CFSTR("growlTicket"), kLSRolesAll, 0, &cfurl);
        if (status == kLSApplicationNotFoundErr)
            return;
        QCFType<CFBundleRef> bundle = CFBundleCreate(0, cfurl);

        if (CFStringCompare(CFBundleGetIdentifier(bundle), CFSTR("com.Growl.GrowlHelperApp"),
                    kCFCompareCaseInsensitive |  kCFCompareBackwards) != kCFCompareEqualTo)
            return;
        QPixmap notificationIconPixmap;
        if(icon == QSystemTrayIcon::Information)
            notificationIconPixmap = QApplication::style()->standardPixmap(QStyle::SP_MessageBoxInformation);
        else if(icon == QSystemTrayIcon::Warning)
            notificationIconPixmap = QApplication::style()->standardPixmap(QStyle::SP_MessageBoxWarning);
        else if(icon == QSystemTrayIcon::Critical)
            notificationIconPixmap = QApplication::style()->standardPixmap(QStyle::SP_MessageBoxCritical);
        QTemporaryFile notificationIconFile;
        QString notificationType(QLatin1String("Notification")), notificationIcon, notificationApp(QApplication::applicationName());
        if(notificationApp.isEmpty())
            notificationApp = QLatin1String("Application");
        if(!notificationIconPixmap.isNull() && notificationIconFile.open()) {
            QImageWriter writer(&notificationIconFile, "PNG");
            if(writer.write(notificationIconPixmap.toImage()))
                notificationIcon = QLatin1String("image from location \"file://") + notificationIconFile.fileName() + QLatin1String("\"");
        }
        const QString script(QLatin1String(
            "tell application \"GrowlHelperApp\"\n"
            "-- Make a list of all the notification types (all)\n"
            "set the allNotificationsList to {\"") + notificationType + QLatin1String("\"}\n"

            "-- Make a list of the notifications (enabled)\n"
            "set the enabledNotificationsList to {\"") + notificationType + QLatin1String("\"}\n"

            "-- Register our script with growl.\n"
            "register as application \"") + notificationApp + QLatin1String("\" all notifications allNotificationsList default notifications enabledNotificationsList\n"

            "--	Send a Notification...\n") +
            QLatin1String("notify with name \"") + notificationType +
            QLatin1String("\" title \"") + title +
            QLatin1String("\" description \"") + message +
            QLatin1String("\" application name \"") + notificationApp +
            QLatin1String("\" ")  + notificationIcon +
            QLatin1String("\nend tell"));
        qt_mac_execute_apple_script(script, 0);
#elif 0
        Q_Q(QSystemTrayIcon);
        NSView *v = [[sys->item item] view];
        NSWindow *w = [v window];
        w = [[sys->item item] window];
        qDebug() << w << v;
        QPoint p(qRound([w frame].origin.x), qRound([w frame].origin.y));
        qDebug() << p;
        QBalloonTip::showBalloon(icon, message, title, q, QPoint(0, 0), msecs);
#else
        Q_UNUSED(icon);
        Q_UNUSED(title);
        Q_UNUSED(message);
#endif
    }
}
QT_END_NAMESPACE

@implementation NSStatusItem (Qt)
@end

@implementation QNSImageView
-(id)initWithParent:(QNSStatusItem*)myParent {
    self = [super init];
    parent = myParent;
    down = NO;
    return self;
}

-(QSystemTrayIcon*)icon {
    return [parent icon];
}

-(void)menuTrackingDone:(NSNotification*)notification
{
    Q_UNUSED(notification);
    down = NO;
    if([self icon]->contextMenu())
        [self icon]->contextMenu()->hide();
    [self setNeedsDisplay:YES];
}

-(void)mouseDown:(NSEvent *)mouseEvent {
    int clickCount = [mouseEvent clickCount];
    down = !down;
    if(!down && [self icon]->contextMenu())
        [self icon]->contextMenu()->hide();
    [self setNeedsDisplay:YES];

    if (down)
        [parent triggerSelector:self];
    else if ((clickCount%2))
        [parent doubleClickSelector:self];
    while (down) {
        mouseEvent = [[self window] nextEventMatchingMask: NSLeftMouseDownMask | NSLeftMouseUpMask | NSLeftMouseDraggedMask];
        switch ([mouseEvent type]) {
            case NSLeftMouseDown:
            case NSLeftMouseUp:
                [self menuTrackingDone:nil];
                break;
            case NSLeftMouseDragged:
            default:
                /* Ignore any other kind of event. */
                break;
        }
    };
}


-(void)drawRect:(NSRect)rect {
    [[parent item] drawStatusBarBackgroundInRect:rect withHighlight:down];
    [super drawRect:rect];
}
@end

@implementation QNSStatusItem
-(id)initWithIcon:(QSystemTrayIcon*)i {
    self = [super init];
    if(self) {
        icon = i;
        item = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSSquareStatusItemLength] retain];
        imageCell = [[QNSImageView alloc] initWithParent:self];
        [item setView: imageCell];
    }
    return self;
}
-(void)dealloc {
    [[NSStatusBar systemStatusBar] removeStatusItem:item];
    [imageCell release];
    [item release];
    [super dealloc];

}

-(QSystemTrayIcon*)icon {
    return icon;
}

-(NSStatusItem*)item {
    return item;
}
-(QRectF)geometry {
    if(NSWindow *window = [[item view] window]) {
        NSRect screenRect = [[window screen] frame];
        NSRect windowRect = [window frame];
        return QRectF(windowRect.origin.x, screenRect.size.height-windowRect.origin.y-windowRect.size.height, windowRect.size.width, windowRect.size.height);
    }
    return QRectF();
}
- (void)triggerSelector:(id)sender {
    Q_UNUSED(sender);
    if(!icon)
        return;
    qtsystray_sendActivated(icon, QSystemTrayIcon::Trigger);
    if (icon->contextMenu()) {
#if 0
        const QRectF geom = [self geometry];
        if(!geom.isNull()) {
            [[NSNotificationCenter defaultCenter] addObserver:imageCell
                                                  selector:@selector(menuTrackingDone:)
                                                  name:nil
                                                  object:self];
            icon->contextMenu()->exec(geom.topLeft().toPoint(), 0);
            [imageCell menuTrackingDone:nil];
        } else 
#endif
        {
            NSMenu *m = [[QNSMenu alloc] initWithQMenu:icon->contextMenu()];
            [m setAutoenablesItems: NO];
            [[NSNotificationCenter defaultCenter] addObserver:imageCell
                                                  selector:@selector(menuTrackingDone:)
                                                  name:NSMenuDidEndTrackingNotification
                                                  object:m];
            [item popUpStatusItemMenu: m];
            [m release];
        }
    }
}
- (void)doubleClickSelector:(id)sender {
    Q_UNUSED(sender);
    if(!icon)
        return;
    qtsystray_sendActivated(icon, QSystemTrayIcon::DoubleClick);
}
@end

class QSystemTrayIconQMenu : public QMenu
{
public:
    void doAboutToShow() { emit aboutToShow(); }
private:
    QSystemTrayIconQMenu();
};

@implementation QNSMenu
-(id)initWithQMenu:(QMenu*)qm {
    self = [super init];
    if(self) {
        self->qmenu = qm;
        [self setDelegate:self];
    }
    return self;
}
-(QMenu*)menu { 
    return qmenu; 
}
-(void)menuNeedsUpdate:(QNSMenu*)menu {
    emit static_cast<QSystemTrayIconQMenu*>(menu->qmenu)->doAboutToShow();
    for(int i = [menu numberOfItems]-1; i >= 0; --i)
        [menu removeItemAtIndex:i];
    QList<QAction*> actions = menu->qmenu->actions();;
    for(int i = 0; i < actions.size(); ++i) {
        const QAction *action = actions[i];
        if(!action->isVisible())
            continue;

        NSMenuItem *item = 0;
        bool needRelease = false;
        if(action->isSeparator()) {
            item = [NSMenuItem separatorItem];
        } else {
            item = [[NSMenuItem alloc] init];
            needRelease = true;
            QString text = action->text();
            QKeySequence accel = action->shortcut();
            {
                int st = text.lastIndexOf(QLatin1Char('\t'));
                if(st != -1) {
                    accel = QKeySequence(text.right(text.length()-(st+1)));
                    text.remove(st, text.length()-st);
                }
            }
            if(accel.count() > 1)
                text += QLatin1String(" (****)"); //just to denote a multi stroke shortcut

            [item setTitle:(NSString*)QCFString::toCFStringRef(qt_mac_no_ampersands(text))];
            [item setEnabled:action->isEnabled()];
            [item setState:action->isChecked() ? NSOnState : NSOffState];
            [item setToolTip:(NSString*)QCFString::toCFStringRef(action->toolTip())];
            const QIcon icon = action->icon();
            if(!icon.isNull()) {
                const short scale = GetMBarHeight()-4;
                NSImage *nsimage = qt_mac_create_ns_image(icon.pixmap(QSize(scale, scale)));
                [item setImage: nsimage];
                [nsimage release];
            }
            if(action->menu()) {
                QNSMenu *sub = [[QNSMenu alloc] initWithQMenu:action->menu()];
                [item setSubmenu:sub];
            } else {
                [item setAction:@selector(selectedAction:)];
                [item setTarget:self];
            }
            if(!accel.isEmpty()) {
                quint32 modifier, key;
                qt_mac_get_accel(accel[0], &modifier, &key);
                [item setKeyEquivalentModifierMask:modifier];
                [item setKeyEquivalent:(NSString*)QCFString::toCFStringRef(QString((QChar*)&key, 2))];
            }
        }
        if(item)
            [menu addItem:item];
        if (needRelease)
            [item release];
    }
}
-(void)selectedAction:(id)a {
    const int activated = [self indexOfItem:a];
    QAction *action = 0;
    QList<QAction*> actions = qmenu->actions();
    for(int i = 0, cnt = 0; i < actions.size(); ++i) {
        if(actions.at(i)->isVisible() && (cnt++) == activated) {
            action = actions.at(i);
            break;
        }
    }
    if(action) {
        action->activate(QAction::Trigger);
        qt_mac_trayicon_activate_action(qmenu, action);
    }
}
@end


/* Done here because this is the only .mm for now! -Sam */
QMacCocoaAutoReleasePool::QMacCocoaAutoReleasePool()
{
    NSApplicationLoad();
    pool = (void*)[[NSAutoreleasePool alloc] init];
}

QMacCocoaAutoReleasePool::~QMacCocoaAutoReleasePool()
{
    [(NSAutoreleasePool*)pool release];
}

