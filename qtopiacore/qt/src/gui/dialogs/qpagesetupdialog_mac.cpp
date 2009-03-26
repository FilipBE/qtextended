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
#include "qpagesetupdialog.h"

#include <qhash.h>
#include <private/qapplication_p.h>
#include <private/qprintengine_mac_p.h>
#include <private/qabstractpagesetupdialog_p.h>

#ifndef QT_NO_PRINTDIALOG

QT_BEGIN_NAMESPACE

class QPageSetupDialogPrivate : public QAbstractPageSetupDialogPrivate
{
    Q_DECLARE_PUBLIC(QPageSetupDialog)
public:
    QPageSetupDialogPrivate() : ep(0), upp(0), sheetBlocks(false), acceptStatus(false) {}
    ~QPageSetupDialogPrivate() {
        if (upp) {
            DisposePMSheetDoneUPP(upp);
            upp = 0;
        }
        QHash<PMPrintSession, QPageSetupDialogPrivate *>::iterator it = sheetCallbackMap.begin();
        while (it != sheetCallbackMap.end()) {
            if (it.value() == this) {
                it = sheetCallbackMap.erase(it);
            } else {
                ++it;
            }
        }
    }
    static void pageSetupDialogSheetDoneCallback(PMPrintSession printSession, WindowRef /*documentWindow*/, Boolean accepted) {
        QPageSetupDialogPrivate *priv = sheetCallbackMap.value(printSession);
        if (!priv) {
            qWarning("%s:%d: QPageSetupDialog::exec: Could not retrieve data structure, "
                     "you most likely now have an infinite modal loop", __FILE__, __LINE__);
            return;
        }
        priv->sheetBlocks = false;
        priv->acceptStatus = accepted;
    }
    QMacPrintEnginePrivate *ep;
    PMSheetDoneUPP upp;
    bool sheetBlocks;
    Boolean acceptStatus;
    static QHash<PMPrintSession, QPageSetupDialogPrivate*> sheetCallbackMap;
};

QHash<PMPrintSession, QPageSetupDialogPrivate*> QPageSetupDialogPrivate::sheetCallbackMap;

QPageSetupDialog::QPageSetupDialog(QPrinter *printer, QWidget *parent)
    : QAbstractPageSetupDialog(*(new QPageSetupDialogPrivate), printer, parent)
{
    Q_D(QPageSetupDialog);
    QMacPrintEngine *engine = static_cast<QMacPrintEngine *>(d->printer->paintEngine());
    d->ep = static_cast<QMacPrintEnginePrivate *>(engine->d_ptr);
}

int QPageSetupDialog::exec()
{
    Q_D(QPageSetupDialog);

    if (d->printer->outputFormat() != QPrinter::NativeFormat)
        return Rejected;

    QMacPrintEngine *engine = static_cast<QMacPrintEngine *>(d->printer->paintEngine());
    QMacPrintEnginePrivate *ep = static_cast<QMacPrintEnginePrivate *>(engine->d_ptr);

    // If someone is reusing a QPrinter object, the end released all our old
    // information. In this case, we must reinitialize.
    if (ep->session == 0)
        ep->initialize();

    { //simulate modality
        // First, see if we should use a sheet.
        QWidget *parent = parentWidget();
        if (parent && parent->isVisible()) {
            WindowRef windowRef = qt_mac_window_for(parent);
            WindowClass wclass;
            GetWindowClass(windowRef, &wclass);
            if (!isOptionEnabled(QPageSetupDialog::DontUseSheet)
                    && (wclass == kDocumentWindowClass || wclass == kFloatingWindowClass
                        || wclass == kMovableModalWindowClass)) {
                // Yes, we can use a sheet
                if (!d->upp)
                    d->upp = NewPMSheetDoneUPP(QPageSetupDialogPrivate::pageSetupDialogSheetDoneCallback);
                d->sheetCallbackMap.insert(d->ep->session, d);
                PMSessionUseSheets(d->ep->session, qt_mac_window_for(parentWidget()), d->upp);
                d->sheetBlocks = true;
            }
        }
	QWidget modal_widg(0, Qt::Window);
        modal_widg.setObjectName(QLatin1String(__FILE__ "__modal_dlg"));
        modal_widg.createWinId();
	QApplicationPrivate::enterModal(&modal_widg);
        QApplicationPrivate::native_modal_dialog_active = true;
        PMSessionPageSetupDialog(ep->session, ep->format, &d->acceptStatus);
        while (d->sheetBlocks) {
            qApp->processEvents(QEventLoop::WaitForMoreEvents);
        }
	QApplicationPrivate::leaveModal(&modal_widg);
        QApplicationPrivate::native_modal_dialog_active = false;
        
        // if the margins have changed, we have to use the margins from the new 
        // PMFormat object
        if (d->acceptStatus == Accepted) {
            PMPaper paper;
            PMPaperMargins margins;
            PMGetPageFormatPaper(ep->format, &paper);
            PMPaperGetMargins(paper, &margins);
            ep->leftMargin = margins.left;
            ep->topMargin = margins.top;
            ep->rightMargin = margins.right;
            ep->bottomMargin = margins.bottom;
        }
    }
    return d->acceptStatus ? Accepted : Rejected;
}

QT_END_NAMESPACE

#endif QT_NO_PRINTDIALOG

