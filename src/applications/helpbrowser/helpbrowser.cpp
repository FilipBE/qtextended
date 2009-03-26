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

#include "helpbrowser.h"
#include "helppreprocessor.h"

#include <qtopiaapplication.h>
#include <qtopialog.h>

#include <qcontent.h>
#include <qcontentset.h>

#include <qtopianamespace.h>
#include <qsoftmenubar.h>

#include <QAction>
#include <QMenu>
#include <QFile>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QScrollBar>
#include <QFileInfo>
#include <QStyle>
#include <QImageReader>

#define HOMEPAGE QUrl( "index.html" )


MagicTextBrowser::MagicTextBrowser( QWidget* parent )
    : QTextBrowser( parent )
{ }

QVariant MagicTextBrowser::loadResource(int type, const QUrl &name)
{
    if (type == QTextDocument::ImageResource) {
        QString filename(name.toLocalFile());
        // QUrl drops the ':' from the start of the name.
        if (filename.startsWith(QLatin1String("image/"))) {
            filename.prepend(":");
            QFileInfo fi(filename);
            QImageReader reader(filename);
            if (fi.suffix() == "svg" || fi.suffix() == "pic") {
                // We'll force a sensible size, otherwise we could get
                // anything.
                int size = style()->pixelMetric(QStyle::PM_ListViewIconSize);
                reader.setScaledSize(QSize(size, size));
            }
            QImage img = reader.read();
            QPixmap pm = QPixmap::fromImage(img);
            return QVariant(pm);
        } else if (filename.startsWith(QLatin1String("icon/"))) {
            filename.prepend(":");
            QFileInfo fi(filename);
            if (fi.exists()) {
                int size = style()->pixelMetric(QStyle::PM_SmallIconSize);
                QIcon icon(filename);
                return QVariant(icon.pixmap(size, size));
            }
        }
        QVariant r = QTextBrowser::loadResource(type, name);
        if (!r.isValid()) {
            // Avoid "file-16.png" ugle, once.
            QUrl unknown(filename.startsWith(":icon")
                    ? ":icon/qpe/UnknownDocument" : ":image/qpe/UnknownDocument");
            if (name != unknown)
                return loadResource(QTextDocument::ImageResource, unknown);
        }
        return r;
    } else if (type == QTextDocument::HtmlResource) {
        QString filename(name.toLocalFile());
        HelpPreProcessor hpp(filename);
        QString result = hpp.text();
        return QVariant(result);
    }

    return QTextBrowser::loadResource(type, name);
}

HelpBrowser::~HelpBrowser()
{
}

HelpBrowser::HelpBrowser( QWidget* parent, Qt::WFlags f )
    : QMainWindow( parent, f )
{
    init();
}

void HelpBrowser::init()
{
    QWidget *box = new QWidget(this);
    QBoxLayout *boxLayout = new QVBoxLayout( box );
    boxLayout->setMargin(0);
    boxLayout->setSpacing(0);
    browser = new MagicTextBrowser( box );
    boxLayout->addWidget(browser);

#ifdef DEBUG
    location = new QLabel( box );
    boxLayout->addWidget(location);
#endif

    navigationBar = new NavigationBar(this);

    // Ensure that when either navigationBar's left or right arrow button is clicked,
    // we pick up the emitted signal and handle it by moving forwards or backwards
    // in the document hierarchy.
    connect(navigationBar,SIGNAL(forwards()),browser,SLOT(forward()));
    connect(navigationBar,SIGNAL(backwards()),browser,SLOT(backward()));

    // Ensure that when the browser's document queue changes condition at either end
    // (i.e. it has a 'backwards' document when previously it had none, or it no
    // longer has a 'forwards' document, etc), navigationBar's left or right
    // arrow button is enabled or disabled appropriately.
    connect(browser,SIGNAL(backwardAvailable(bool)),navigationBar,(SLOT(setBackwardsEnabled(bool))));
    connect(browser,SIGNAL(forwardAvailable(bool)),navigationBar,(SLOT(setForwardsEnabled(bool))));
    // When the browser's document queue changes condition, the previous and next
    // documents will also undergo changes. When this happens, navigationBar will
    // need to update its previous and next document titles.
    connect(browser,SIGNAL(historyChanged()), this,SLOT(updateLabels()));

    boxLayout->addWidget(navigationBar);

    QStringList helpPath = Qtopia::helpPaths();
    helpPath.append("pics");
    browser->setSearchPaths(helpPath);
    connect( browser, SIGNAL(sourceChanged(QUrl)),
             this, SLOT(textChanged(QUrl)) );

    setCentralWidget( box );

    // Hook onto the application channel to process Help service messages.
    new HelpService( this );

    backAction = new QAction(QIcon(":icon/i18n/previous"), tr("Back"), this);
    backAction->setWhatsThis(tr("Move backward one page."));
    backAction->setVisible( false );
    connect( backAction, SIGNAL(triggered()), browser, SLOT(backward()) );
    connect( browser, SIGNAL(backwardAvailable(bool)), backAction, SLOT(setVisible(bool)) );

    forwardAction = new QAction(QIcon(":icon/i18n/next"), tr("Forward"), this );
    forwardAction->setWhatsThis( tr( "Move forward one page." ) );
    forwardAction->setVisible( false );
    connect( forwardAction, SIGNAL(triggered()), browser, SLOT(forward()) );
    connect( browser, SIGNAL(forwardAvailable(bool)), forwardAction, SLOT(setVisible(bool)) );

    QAction *homeAction = new QAction(QIcon(":icon/home"), tr("Home"), this );
    homeAction->setWhatsThis( tr( "Go to the home page." ) );
    connect( homeAction, SIGNAL(triggered()), this, SLOT(goHome()) );

    contextMenu = QSoftMenuBar::menuFor(this);

    contextMenu->addAction( homeAction );
    contextMenu->addAction( backAction );
    contextMenu->addAction( forwardAction );

    setFocusProxy( browser );
    browser->setFrameStyle( QFrame::NoFrame );

    browser->installEventFilter( this );
    browser->ensurePolished();
    browser->setSource( HOMEPAGE );

    QSoftMenuBar::setLabel(browser, Qt::Key_Back, "close", tr("Close"));
}

void HelpBrowser::updateLabels()
{
    navigationBar->labelsChanged(browser->historyTitle(-1),
                                 browser->historyTitle(+1));
}

void HelpBrowser::setDocument( const QString &doc )
{
    if ( !doc.isEmpty() ) {
        browser->clearHistory();

        browser->setSource( doc );
        QtopiaApplication::instance()->showMainWidget();
    }
}

void HelpBrowser::goHome()
{
    browser->setSource( HOMEPAGE );
}

// Private slot to handle browser's sourceChanged() signal. Ensures the browser's
// title and the debug location's filename are up-to-date.
void HelpBrowser::textChanged(QUrl url)
{
    if ( browser->documentTitle().isNull() )
        setWindowTitle( tr("Help Browser") );
    else
        setWindowTitle( browser->documentTitle() ) ;
#ifdef DEBUG
    location->setText( url.toString() );
#else
    Q_UNUSED(url);
#endif
}

bool HelpBrowser::eventFilter( QObject*obj, QEvent* e )
{
    Q_UNUSED(obj);

    switch( e->type() ) {
    case QEvent::KeyPress:
        {
            QKeyEvent *ke = (QKeyEvent*)e;

            // The left and right hardware keys cause the NavigationBar keys to be
            // triggered, which in turn are connected to the slots to navigate through
            // the document hierarchy.
            if ( ke->key() == Qt::Key_Left ) {
                // Cause the left key in the NavigationBar to be 'pressed'.
                navigationBar->triggerBackwards();
            } else if ( ke->key() == Qt::Key_Right ) {
                // Cause the right key in the NavigationBar to be 'pressed'.
                navigationBar->triggerForwards();
            }
        }
        break;
    default:
        break;
    }

    // Allow the parent to handle it.
    return false;
}

void HelpBrowser::closeEvent( QCloseEvent* e )
{
    e->accept();
}

/*!
    \service HelpService Help
    \brief Provides the Qt Extended Help service.

    The \i Help service enables applications to display context-sensitive help.
*/

/*!
    \internal
*/
HelpService::~HelpService()
{
}

/*!
    Display \a doc within the help browser.

    This slot corresponds to the QCop service message
    \c{Help::setDocument(QString)}.
*/
void HelpService::setDocument( const QString& doc )
{
    parent->setDocument( doc );
}

