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

#ifndef QT_NO_PRINTDIALOG

#include <private/qt_mac_p.h>

#include <qhash.h>
#include <qprintdialog.h>
#include <private/qapplication_p.h>
#include <private/qabstractprintdialog_p.h>
#include <private/qprintengine_mac_p.h>

QT_BEGIN_NAMESPACE

class QPrintDialogPrivate : public QAbstractPrintDialogPrivate
{
    Q_DECLARE_PUBLIC(QPrintDialog)
public:
    QPrintDialogPrivate() : ep(0), upp(0), sheetBlocks(false), acceptStatus(false) { }
    ~QPrintDialogPrivate() {
        if (upp) {
            DisposePMSheetDoneUPP(upp);
            upp = 0;
        }
        QHash<PMPrintSession, QPrintDialogPrivate *>::iterator it = sheetCallbackMap.begin();
        while (it != sheetCallbackMap.end()) {
            if (it.value() == this) {
                it = sheetCallbackMap.erase(it);
            } else {
                ++it;
            }
        }
    }

    inline void _q_printToFileChanged(int) {}
    inline void _q_rbPrintRangeToggled(bool) {}
    inline void _q_printerChanged(int) {}
#ifndef QT_NO_MESSAGEBOX
    inline void _q_checkFields() {}
#endif
    inline void _q_chbPrintLastFirstToggled(bool) {}
    inline void _q_paperSizeChanged(int) {}
    inline void _q_btnBrowseClicked() {}
    inline void _q_btnPropertiesClicked() {}
    static void printDialogSheetDoneCallback(PMPrintSession printSession, WindowRef /*documentWindow*/, Boolean accepted) {
        QPrintDialogPrivate *priv = sheetCallbackMap.value(printSession);
        if (!priv) {
            qWarning("%s:%d: QPrintDialog::exec: Could not retrieve data structure, "
                     "you most likely now have an infinite loop", __FILE__, __LINE__);
            return;
        }
        priv->sheetBlocks = false;
        priv->acceptStatus = accepted;
    }

    QMacPrintEnginePrivate *ep;
    PMSheetDoneUPP upp;
    bool sheetBlocks;
    Boolean acceptStatus;
    static QHash<PMPrintSession, QPrintDialogPrivate *> sheetCallbackMap;
};

QHash<PMPrintSession, QPrintDialogPrivate *> QPrintDialogPrivate::sheetCallbackMap;

QPrintDialog::QPrintDialog(QPrinter *printer, QWidget *parent)
    : QAbstractPrintDialog(*(new QPrintDialogPrivate), printer, parent)
{
    Q_D(QPrintDialog);
    d->ep = static_cast<QMacPrintEngine *>(printer->paintEngine())->d_func();
}

QPrintDialog::~QPrintDialog()
{
    //nothing
}

int QPrintDialog::exec()
{
    Q_D(QPrintDialog);
    QMacBlockingFunction func;

    // If someone is reusing a QPrinter object, the end released all our old
    // information. In this case, we must reinitialize.
    if (d->ep->session == 0)
        d->ep->initialize();

    // Carbon's documentation lies.
    // It seems the only way that Carbon lets you use all is if the minimum
    // for the page range is 1. This _kind of_ makes sense if you think about
    // it. However, calling _q_setFirstPage or _q_setLastPage always enforces the range.
    PMSetPageRange(d->ep->settings, minPage(), maxPage());
    if (printRange() == PageRange) {
        PMSetFirstPage(d->ep->settings, fromPage(), false);
        PMSetLastPage(d->ep->settings, toPage(), false);
    }
    { //simulate modality
        // First, see if we should use a sheet.
        QWidget *parent = parentWidget();
        if (parent && parent->isVisible()) {
            WindowRef windowRef = qt_mac_window_for(parent);
            WindowClass wclass;
            GetWindowClass(windowRef, &wclass);
            if (!isOptionEnabled(QAbstractPrintDialog::DontUseSheet)
                && (wclass == kDocumentWindowClass || wclass == kFloatingWindowClass
                    || wclass == kMovableModalWindowClass)) {
                // Yes, we can use a sheet
                if (!d->upp)
                    d->upp = NewPMSheetDoneUPP(QPrintDialogPrivate::printDialogSheetDoneCallback);
                d->sheetCallbackMap.insert(d->ep->session, d);
                PMSessionUseSheets(d->ep->session, windowRef, d->upp);
                d->sheetBlocks = true;
            }
        }
	QWidget modal_widg(0, Qt::Window);
        modal_widg.setObjectName(QLatin1String(__FILE__ "__modal_dlg"));
        modal_widg.createWinId();
	QApplicationPrivate::enterModal(&modal_widg);
    QApplicationPrivate::native_modal_dialog_active = true;
        PMSessionPrintDialog(d->ep->session, d->ep->settings, d->ep->format, &d->acceptStatus);
        while (d->sheetBlocks) {
            qApp->processEvents(QEventLoop::WaitForMoreEvents);
        }
	QApplicationPrivate::leaveModal(&modal_widg);
    QApplicationPrivate::native_modal_dialog_active = false;
    }
    if (d->acceptStatus) {
        UInt32 frompage, topage;
        PMGetFirstPage(d->ep->settings, &frompage);
        PMGetLastPage(d->ep->settings, &topage);
        topage = qMin(UInt32(INT_MAX), topage);
        setFromTo(frompage, topage);

        // OK, I need to map these values back let's see
        // If from is 1 and to is INT_MAX, then print it all
        // (Apologies to the folks with more than INT_MAX pages)
        // ...that's a joke.
        if (fromPage() == 1 && toPage() == INT_MAX) {
            setPrintRange(AllPages);
            setFromTo(0,0);
        } else {
            setPrintRange(PageRange); // In a way a lie, but it shouldn't hurt.
            // Carbon hands us back a very large number here even for ALL, set it to max
            // in that case to follow the behavior of the other print dialogs.
            if (maxPage() < toPage())
                setFromTo(fromPage(), maxPage());
        }
        // Keep us in sync with file output
        PMDestinationType dest;
        PMSessionGetDestinationType(d->ep->session, d->ep->settings, &dest);
        if (dest == kPMDestinationFile) {
            QCFType<CFURLRef> file;
            PMSessionCopyDestinationLocation(d->ep->session, d->ep->settings, &file);
            UInt8 localFile[2048];  // Assuming there's a POSIX file system here.
            CFURLGetFileSystemRepresentation(file, true, localFile, sizeof(localFile));
            d->ep->outputFilename
                = QString::fromUtf8(reinterpret_cast<const char *>(localFile));
        } else {
            // Keep output format.
            QPrinter::OutputFormat format;
            format = d->printer->outputFormat();
            d->printer->setOutputFileName(QString());
            d->printer->setOutputFormat(format);

            QCFType<CFURLRef> location;
            OSStatus status = PMSessionCopyDestinationLocation(d->ep->session, d->ep->settings, &location);
            // If location is NULL, it means default destination.
            if (status == noErr && location) {
                QString locName = QCFString::toQString(CFURLGetString(location));
                d->printer->setPrinterName(locName);
            } else {
                d->printer->setPrinterName(QString());
            }
        }
    }
    return d->acceptStatus ? Accepted : Rejected;
}

QT_END_NAMESPACE

#include "moc_qprintdialog.cpp"

#endif // QT_NO_PRINTDIALOG
