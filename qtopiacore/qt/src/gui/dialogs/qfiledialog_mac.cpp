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

#include "qfiledialog.h"

#ifndef QT_NO_FILEDIALOG

/*****************************************************************************
  QFileDialog debug facilities
 *****************************************************************************/
//#define DEBUG_FILEDIALOG_FILTERS

#include <qapplication.h>
#include <private/qapplication_p.h>
#include <private/qfiledialog_p.h>
#include <private/qt_mac_p.h>
#include <qregexp.h>
#include <qbuffer.h>
#include <qdebug.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qdesktopwidget.h>
#include <stdlib.h>

QT_BEGIN_NAMESPACE

/*****************************************************************************
  Externals
 *****************************************************************************/
extern WindowPtr qt_mac_window_for(const QWidget*); //qwidget_mac.cpp
extern QStringList qt_make_filter_list(const QString &filter); //qfiledialog.cpp
extern const char *qt_file_dialog_filter_reg_exp; //qfiledialog.cpp

/*****************************************************************************
  QFileDialog utility functions
 *****************************************************************************/
static UInt8 *str_buffer = 0;
static void cleanup_str_buffer()
{
    if (str_buffer) {
        free(str_buffer);
        str_buffer = 0;
    }
}

// Returns the wildcard part of a filter.
struct qt_mac_filter_name {
    QString description, regxp, filter;
};

static qt_mac_filter_name *qt_mac_extract_filter(const QString &rawFilter)
{
    qt_mac_filter_name *ret = new qt_mac_filter_name;
    ret->filter = rawFilter;
    QString result = rawFilter;
    QRegExp r(QString::fromLatin1(qt_file_dialog_filter_reg_exp));
    int index = r.indexIn(result);
    if (index >= 0) {
        ret->description = r.cap(1).trimmed();
        result = r.cap(2);
    }
    if (ret->description.isEmpty())
        ret->description = result;
    ret->regxp = result.replace(QLatin1Char(' '), QLatin1Char(';'));
    return ret;
}

// Makes a list of filters from ;;-separated text.
static QList<qt_mac_filter_name*> qt_mac_make_filters_list(const QString &filter)
{
#ifdef DEBUG_FILEDIALOG_FILTERS
    qDebug("QFileDialog:%d - Got filter (%s)", __LINE__, filter.latin1());
#endif
    QString f(filter);
    if (f.isEmpty())
        f = QFileDialog::tr("All Files (*)");
    if (f.isEmpty())
        return QList<qt_mac_filter_name*>();
/*
    QString sep(";;");
    int i = f.indexOf(sep, 0);
    if (i == -1) {
        sep = "\n";
        if (f.indexOf(sep, 0) != -1)
            i = f.indexOf(sep, 0);
    }
    QStringList filts = f.split(sep);
*/
    QStringList filts = qt_make_filter_list(f);
    QList<qt_mac_filter_name*> ret;
    for (QStringList::Iterator it = filts.begin(); it != filts.end(); ++it) {
        qt_mac_filter_name *filter = qt_mac_extract_filter((*it));
#ifdef DEBUG_FILEDIALOG_FILTERS
        qDebug("QFileDialog:%d Split out filter (%d) '%s' '%s' [%s]", __LINE__, ret.count(),
               filter->regxp.latin1(), filter->description.latin1(), (*it).latin1());
#endif
        ret.append(filter);
    }
    return ret;
}

struct qt_mac_nav_filter_type {
    int index;
    bool saveDialog;
    QList<qt_mac_filter_name*> *filts;
};

static Boolean qt_mac_nav_filter(AEDesc *theItem, void *info,
                                             void *myd, NavFilterModes)
{
    qt_mac_nav_filter_type *t = (qt_mac_nav_filter_type *)myd;
    if (!t || !t->filts || t->index >= t->filts->count())
        return true;

    NavFileOrFolderInfo *theInfo = (NavFileOrFolderInfo *)info;
    QString file;
    qt_mac_filter_name *fn = t->filts->at(t->index);
    if (!fn)
        return true;
    if (theItem->descriptorType == typeFSRef) {
        FSRef ref;
        AEGetDescData(theItem, &ref, sizeof(ref));
        if (!str_buffer) {
            qAddPostRoutine(cleanup_str_buffer);
            str_buffer = (UInt8 *)malloc(1024);
        }
        FSRefMakePath(&ref, str_buffer, 1024);
        file = QString::fromUtf8((const char *)str_buffer);
        int slsh = file.lastIndexOf(QLatin1Char('/'));
        if (slsh != -1)
            file = file.right(file.length() - slsh - 1);
    }
    QStringList reg = fn->regxp.split(QLatin1String(";"));
    for (QStringList::Iterator it = reg.begin(); it != reg.end(); ++it) {
        QRegExp rg(*it, Qt::CaseInsensitive, QRegExp::Wildcard);
#ifdef DEBUG_FILEDIALOG_FILTERS
        qDebug("QFileDialog:%d, asked to filter.. %s (%s)", __LINE__,
               file.latin1(), (*it).latin1());
#endif
        if (rg.exactMatch(file))
            return true;
    }
    return (theInfo->isFolder && !file.endsWith(QLatin1String(".app")));
}

//filter UPP stuff
static NavObjectFilterUPP mac_navFilterUPP = 0;
static void cleanup_navFilterUPP()
{
    DisposeNavObjectFilterUPP(mac_navFilterUPP);
    mac_navFilterUPP = 0;
}
static const NavObjectFilterUPP make_navFilterUPP()
{
    if (mac_navFilterUPP)
        return mac_navFilterUPP;
    qAddPostRoutine(cleanup_navFilterUPP);
    return mac_navFilterUPP = NewNavObjectFilterUPP(qt_mac_nav_filter);
}
//event UPP stuff
static NavEventUPP mac_navProcUPP = 0;
static void cleanup_navProcUPP()
{
    DisposeNavEventUPP(mac_navProcUPP);
    mac_navProcUPP = 0;
}
static bool g_nav_blocking=true;
static void qt_mac_filedialog_event_proc(const NavEventCallbackMessage msg,
                                                     NavCBRecPtr p, NavCallBackUserData myd)
{
    switch(msg) {
    case kNavCBPopupMenuSelect: {
        qt_mac_nav_filter_type *t = (qt_mac_nav_filter_type *)myd;
        NavMenuItemSpec *s = (NavMenuItemSpec*)p->eventData.eventDataParms.param;
        t->index = s->menuType;
        if (t->saveDialog) {
            QString base = QCFString::toQString(NavDialogGetSaveFileName(p->context));
            QFileInfo fi(base);
            base = fi.completeBaseName();
            qt_mac_filter_name *fn = t->filts->at(t->index);
            QStringList reg = fn->regxp.split(QLatin1String(";"), QString::SkipEmptyParts); 
            QString r = reg.first();
            r  = r.right(r.length()-1);      // Strip the *
            base += r;                        //"." + QString::number(s->menuType);
            NavDialogSetSaveFileName(p->context, QCFString::toCFStringRef(base));
        }
#ifdef DEBUG_FILEDIALOG_FILTERS
        qDebug("QFileDialog:%d - Selected a filter: %ld", __LINE__, s->menuType);
#endif
        break; }
    case kNavCBStart:
        g_nav_blocking=true;
        break;
    case kNavCBUserAction:
        g_nav_blocking=false;
        break;
    }
}
static const NavEventUPP make_navProcUPP()
{
    if (mac_navProcUPP)
        return mac_navProcUPP;
    qAddPostRoutine(cleanup_navProcUPP);
    return mac_navProcUPP = NewNavEventUPP(qt_mac_filedialog_event_proc);
}

extern OSErr qt_mac_create_fsref(const QString &, FSRef *); // qglobal.cpp

QStringList qt_mac_get_open_file_names(const QFileDialogArgs &args, QString *pwd,
                                       QString *selectedFilter)
{
    QWidget *parent = args.parent;
    OSErr err;
    QStringList retstrl;

    NavDialogCreationOptions options;
    NavGetDefaultDialogCreationOptions(&options);
    options.modality = kWindowModalityAppModal;
    options.optionFlags |= kNavSupportPackages;
    if (args.options & QFileDialog::DontConfirmOverwrite)
        options.optionFlags |= kNavDontConfirmReplacement;
    if (args.mode != QFileDialog::ExistingFiles)
        options.optionFlags &= ~kNavAllowMultipleFiles;

    if (!args.caption.isEmpty())
        options.windowTitle = QCFString::toCFStringRef(args.caption);

    static const int w = 450, h = 350;
    options.location.h = options.location.v = -1;
    if (parent && parent->isVisible()) {
        WindowClass wclass;
        GetWindowClass(qt_mac_window_for(parent), &wclass);
        if (!(args.options & QFileDialog::DontUseSheet) && (wclass == kDocumentWindowClass ||
                                                            wclass == kFloatingWindowClass || wclass == kMovableModalWindowClass)) {
            options.modality = kWindowModalityWindowModal;
            options.parentWindow = qt_mac_window_for(parent);
        } else {
            parent = parent->window();
            QString s = parent->windowTitle();
            options.clientName = QCFString::toCFStringRef(s);
            options.location.h = (parent->x() + (parent->width() / 2)) - (w / 2);
            options.location.v = (parent->y() + (parent->height() / 2)) - (h / 2);

            QRect r = QApplication::desktop()->screenGeometry(
                QApplication::desktop()->screenNumber(parent));
            const int border = 10;
            if (options.location.h + w > r.right())
                options.location.h -= (options.location.h + w) - r.right() + border;
            if (options.location.v + h > r.bottom())
                options.location.v -= (options.location.v + h) - r.bottom() + border;
            if (options.location.h < r.left())
                options.location.h = r.left() + border;
            if (options.location.v < r.top())
                options.location.v = r.top() + border;
        }
#if 0
    } else if (QWidget *p = qApp->mainWidget()) {
        static int last_screen = -1;
        int scr = QApplication::desktop()->screenNumber(p);
        if (last_screen != scr) {
            QRect r = QApplication::desktop()->screenGeometry(scr);
            options.location.h = (r.x() + (r.width() / 2)) - (w / 2);
            options.location.v = (r.y() + (r.height() / 2)) - (h / 2);
        }
#endif
    }

    QList<qt_mac_filter_name*> filts = qt_mac_make_filters_list(args.filter);
    qt_mac_nav_filter_type t;
    t.saveDialog = false;
    t.index = 0;
    t.filts = &filts;
    if (filts.count() > 1) {
        int i = 0;
        CFStringRef *arr = static_cast<CFStringRef *>(malloc(sizeof(CFStringRef) * filts.count()));
        for (QList<qt_mac_filter_name*>::const_iterator it = filts.constBegin();
             it != filts.constEnd(); ++it)
            arr[i++] = QCFString::toCFStringRef((*it)->description);
        options.popupExtension = CFArrayCreate(0, reinterpret_cast<const void **>(arr), filts.count(), 0);
    }

    NavDialogRef dlg;
    if (args.mode == QFileDialog::DirectoryOnly ||
        args.mode == QFileDialog::Directory) {
        if (NavCreateChooseFolderDialog(&options, make_navProcUPP(), 0, 0, &dlg)) {
            qDebug("Shouldn't happen %s:%d", __FILE__, __LINE__);
            return retstrl;
        }
    } else {
        if (NavCreateGetFileDialog(&options, 0, make_navProcUPP(), 0,
                                  make_navFilterUPP(), (void *) (filts.isEmpty() ? 0 : &t),
                                  &dlg)) {
            qDebug("Shouldn't happen %s:%d", __FILE__, __LINE__);
            return retstrl;
        }
    }
    if (pwd && !pwd->isEmpty()) {
        FSRef fsref;
        if (qt_mac_create_fsref(*pwd, &fsref) == noErr) {
            AEDesc desc;
            if (AECreateDesc(typeFSRef, &fsref, sizeof(FSRef), &desc) == noErr)
                NavCustomControl(dlg, kNavCtlSetLocation, (void*)&desc);
        }
    }

    QWidget modal_widg(parent, Qt::Sheet);
    if (options.modality == kWindowModalityWindowModal || options.modality == kWindowModalityAppModal) {
        //simulate modality prepare:
        modal_widg.createWinId();
        QApplicationPrivate::enterModal(&modal_widg);
        QApplicationPrivate::native_modal_dialog_active = true;
    }
    
    NavDialogRun(dlg);

    if (selectedFilter) {
        NavMenuItemSpec navSpec;
        bzero(&navSpec, sizeof(NavMenuItemSpec));
        qt_mac_filter_name *sel_filt_name = qt_mac_make_filters_list(*selectedFilter).at(0);
        for (int i = 0; i < filts.count(); ++i) {
            const qt_mac_filter_name *filter = filts.at(i);
            if (sel_filt_name->description == filter->description
                    && sel_filt_name->regxp == filter->regxp
                    && sel_filt_name->filter == filter->filter) {
                navSpec.menuType = i;
                break;
            }
        }
        NavCustomControl(dlg, kNavCtlSelectCustomType, &navSpec);
    }

    if (options.modality == kWindowModalityWindowModal || options.modality == kWindowModalityAppModal) {
        //simulate modality prepare:
        while (g_nav_blocking)
            qApp->processEvents(QEventLoop::WaitForMoreEvents);
        QApplicationPrivate::leaveModal(&modal_widg);
        QApplicationPrivate::native_modal_dialog_active = false;
    }
    
    if (!(NavDialogGetUserAction(dlg) &
          (kNavUserActionOpen | kNavUserActionChoose | kNavUserActionNewFolder))) {
        NavDialogDispose(dlg);
        return retstrl;
    }
    NavReplyRecord ret;
    NavDialogGetReply(dlg, &ret);
    NavDialogDispose(dlg);

    long count;
    err = AECountItems(&(ret.selection), &count);
    if (!ret.validRecord || err != noErr || !count) {
        NavDisposeReply(&ret);
        return retstrl;
    }

    for (long index = 1; index <= count; index++) {
        FSRef ref;
        err = AEGetNthPtr(&(ret.selection), index, typeFSRef, 0, 0, &ref, sizeof(ref), 0);
        if (err != noErr)
            break;

        if (!str_buffer) {
            qAddPostRoutine(cleanup_str_buffer);
            str_buffer = (UInt8 *)malloc(1024);
        }
        FSRefMakePath(&ref, str_buffer, 1024);
        retstrl.append(QString::fromUtf8((const char *)str_buffer));
    }
    NavDisposeReply(&ret);
    if (selectedFilter)
        *selectedFilter = filts.at(t.index)->filter;
    while (!filts.isEmpty())
        delete filts.takeFirst();
    return retstrl;
}

QString qt_mac_get_save_file_name(const QFileDialogArgs &args, QString *pwd,
                                  QString *selectedFilter)
{
    QWidget *parent = args.parent;
    OSErr err;
    QString retstr;
    NavDialogCreationOptions options;
    NavGetDefaultDialogCreationOptions(&options);
    static const int w = 450, h = 350;
    if (args.options & QFileDialog::DontConfirmOverwrite)
        options.optionFlags |= kNavDontConfirmReplacement;
    options.modality = kWindowModalityAppModal;
    options.location.h = options.location.v = -1;
    if (!args.directory.isEmpty())
        options.saveFileName = QCFString::toCFStringRef(args.selection);
    if (!args.caption.isEmpty())
        options.windowTitle = QCFString::toCFStringRef(args.caption);
    if (parent && parent->isVisible()) {
        WindowClass wclass;
        GetWindowClass(qt_mac_window_for(parent), &wclass);
        if (!(args.options & QFileDialog::DontUseSheet) && (wclass == kDocumentWindowClass ||
                                                            wclass == kFloatingWindowClass || wclass == kMovableModalWindowClass)) {
            options.modality = kWindowModalityWindowModal;
            options.parentWindow = qt_mac_window_for(parent);

            // The parent needs to be active for the sheet to get keyboard focus.
            if (!parent->isActiveWindow())
                parent->activateWindow();
        } else {
            parent = parent->window();
            QString s = parent->windowTitle();
            options.clientName = CFStringCreateWithCharacters(0, (UniChar *)s.unicode(), s.length());
            options.location.h = (parent->x() + (parent->width() / 2)) - (w / 2);
            options.location.v = (parent->y() + (parent->height() / 2)) - (h / 2);

            QRect r = QApplication::desktop()->screenGeometry(
                QApplication::desktop()->screenNumber(parent));
            if (options.location.h + w > r.right())
                options.location.h -= (options.location.h + w) - r.right() + 10;
            if (options.location.v + h > r.bottom())
                options.location.v -= (options.location.v + h) - r.bottom() + 10;
        }
#if 0
    } else if (QWidget *p = qApp->mainWidget()) {
        static int last_screen = -1;
        int scr = QApplication::desktop()->screenNumber(p);
        if (last_screen != scr) {
            QRect r = QApplication::desktop()->screenGeometry(scr);
            options.location.h = (r.x() + (r.width() / 2)) - (w / 2);
            options.location.v = (r.y() + (r.height() / 2)) - (h / 2);
        }
#endif
    }

    QList<qt_mac_filter_name*> filts = qt_mac_make_filters_list(args.filter);
    qt_mac_nav_filter_type t;
    t.saveDialog = true;
    t.index = 0;
    t.filts = &filts;
    if (filts.count() > 1) {
        int i = 0;
        CFStringRef *arr = static_cast<CFStringRef *>(malloc(sizeof(CFStringRef) * filts.count()));
        for (QList<qt_mac_filter_name*>::const_iterator it = filts.constBegin();
             it != filts.constEnd(); ++it)
            arr[i++] = QCFString::toCFStringRef((*it)->description);
        options.popupExtension = CFArrayCreate(0, reinterpret_cast<const void **>(arr), filts.count(), 0);
    }

    NavDialogRef dlg;
    if (NavCreatePutFileDialog(&options, 'cute', kNavGenericSignature, make_navProcUPP(),
                               static_cast<void *>(filts.isEmpty() ? 0 : &t), &dlg)) {
        qDebug("Shouldn't happen %s:%d", __FILE__, __LINE__);
        return retstr;
    }
    if (pwd && !pwd->isEmpty()) {
        FSRef fsref;
        if (qt_mac_create_fsref(*pwd, &fsref) == noErr) {
            AEDesc desc;
            if (AECreateDesc(typeFSRef, &fsref, sizeof(FSRef), &desc) == noErr)
                NavCustomControl(dlg, kNavCtlSetLocation, (void*)&desc);
        }
    }
    NavDialogRun(dlg);
    
    if (selectedFilter) {
        NavMenuItemSpec navSpec;
        bzero(&navSpec, sizeof(NavMenuItemSpec));
        qt_mac_filter_name *sel_filt_name = qt_mac_make_filters_list(*selectedFilter).at(0);
        for (int i = 0; i < filts.count(); ++i) {
            const qt_mac_filter_name *filter = filts.at(i);
            if (sel_filt_name->description == filter->description
                    && sel_filt_name->regxp == filter->regxp
                    && sel_filt_name->filter == filter->filter) {
                navSpec.menuType = i;
                break;
            }
        }
        NavCustomControl(dlg, kNavCtlSelectCustomType, &navSpec);
    }
    if (options.modality == kWindowModalityWindowModal) { //simulate modality
        QWidget modal_widg(parent, Qt::Sheet);
        modal_widg.createWinId();
        QApplicationPrivate::native_modal_dialog_active = true;
        QApplicationPrivate::enterModal(&modal_widg);
        while (g_nav_blocking)
            qApp->processEvents(QEventLoop::WaitForMoreEvents);
        QApplicationPrivate::leaveModal(&modal_widg);
        QApplicationPrivate::native_modal_dialog_active = false;
    }

    if (NavDialogGetUserAction(dlg) != kNavUserActionSaveAs) {
        NavDialogDispose(dlg);
        return retstr;
    }
    NavReplyRecord ret;
    NavDialogGetReply(dlg, &ret);
    NavDialogDispose(dlg);

    long count;
    err = AECountItems(&(ret.selection), &count);
    if (!ret.validRecord || err != noErr || !count) {
        NavDisposeReply(&ret);
        return retstr;
    }

    AEKeyword        keyword;
    DescType    type;
    Size        size;
    FSRef ref;
    err = AEGetNthPtr(&(ret.selection), 1, typeFSRef, &keyword,
                      &type, &ref, sizeof(ref), &size);
    if (err == noErr) {
        if (!str_buffer) {
            qAddPostRoutine(cleanup_str_buffer);
            str_buffer = (UInt8 *)malloc(1024);
        }
        FSRefMakePath(&ref, str_buffer, 1024);
        retstr = QString::fromUtf8((const char *)str_buffer);
        //now filename
        CFStringGetCString(ret.saveFileName, (char *)str_buffer, 1024, kCFStringEncodingUTF8);
        retstr += QLatin1String("/") + QString::fromUtf8((const char *)str_buffer);
    }
    NavDisposeReply(&ret);
    if (selectedFilter)
        *selectedFilter = filts.at(t.index)->filter;
    while (!filts.isEmpty())
        delete filts.takeFirst();
    return retstr;
}

QT_END_NAMESPACE

#endif // QT_NO_FILEDIALOG

