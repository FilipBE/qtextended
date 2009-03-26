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
#include "mainwindow.h"
#include "charsetedit.h"
#include "ui_gprefbase.h"
#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qsoftmenubar.h>
#include <qdir.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtreewidget.h>
#include <qheaderview.h>

class ProfileItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT
public:
    ProfileItem(ProfileItem *parent, const QString &name, QIMPenCharSet *s)
        : QTreeWidgetItem(parent), mSet(s), mProf(0)
    {
        setObjectName(name);
        setText( 0, name );
        treeWidget()->expandItem( this );
    }
    ProfileItem(QTreeWidget *parent, const QString &name, QIMPenProfile *p)
        : QTreeWidgetItem(parent), mSet(0), mProf(p)
    {
        setObjectName(name);
        setText( 0, name );
        parent->expandItem( this );
    }
    ~ProfileItem() {}

    ProfileItem *parent()
    {
        return (ProfileItem*)QTreeWidgetItem::parent();
    }

    /* Is this really needed as well as the clicked and returnPressed signals
       from the listview itself?
    void setOpen(bool)
    {
        emit selected(this);
    }
    */

    QIMPenCharSet *set() const { return mSet; }
    QIMPenProfile *profile() const { return mProf; }

signals:
    void selected(QTreeWidgetItem *);

private:
    QIMPenCharSet *mSet;
    QIMPenProfile *mProf;
};

class GeneralPref : public QDialog, public Ui::GeneralPrefBase
{
    public:
        GeneralPref( QWidget *parent = 0, Qt::WFlags f = 0 )
            : QDialog( parent, f )
        {
            setupUi( this );
            setModal( true );
            setWindowTitle( "Handwriting" );
        }
};

/*
   Edit of a single profile.  loadProfiles later moved elsewhere when
   is to handle multiple profiles? (second level of list?)
 */
QIMPenProfileEdit::QIMPenProfileEdit(QWidget *parent, Qt::WFlags f)
    : QDialog(parent, f), cdiag(0), gdiag(0)
{
    setWindowTitle(tr("Handwriting"));
    setObjectName( "hwsettings" );

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);
    lv = new QTreeWidget(this);
    lv->setIndentation( 0 );
    lv->setFrameStyle(QFrame::NoFrame);
    // lv->addColumn("");
    lv->setColumnCount( 1 );
    lv->header()->hide();
    lv->setRootIsDecorated(false);
    vbox->addWidget(lv);

    loadProfiles();
    // lv->resizeColumnToContents( 0 );

    connect(lv, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(editItem(QTreeWidgetItem*)));
    QSoftMenuBar::menuFor( this );
}

QIMPenProfileEdit::~QIMPenProfileEdit()
{
}

// of course, should not be here...
bool QIMPenProfileEdit::loadProfiles()
{
    lv->clear();
    // different profile for Fullscreen to nomral?

    // - was the prev implementation leaking here?
    // profileList.clear();
    while ( profileList.count() )
        delete profileList.takeLast();
    QString path = Qtopia::qtopiaDir() + "etc/qimpen"; // no tr
    QDir dir( path, "*.conf" ); // no tr
    QStringList list = dir.entryList();
    QStringListIterator it( list );
    // XXX  do as tree instead.
    for ( ; it.hasNext(); ) {
        QIMPenProfile *p = new QIMPenProfile( path + "/" + it.next() ); // no tr
        profileList.append( p );
    }
    if (profileList.count() < 1) {
        // QMessage box and quit
        QMessageBox::information(0, tr("No profiles found"),
                tr("<qt>Unable to find any handwriting profiles."),
                QMessageBox::Ok);
        return false;
    }
    // add items in list (and their parts) to view
    for ( int i = 0; i < (int)profileList.count(); i++ ) {
        QIMPenProfile *prof = profileList.at(i);

        ProfileItem *pi = new ProfileItem(lv, prof->name(), prof);

        QStringList cSets = prof->charSets();
        QStringListIterator it( cSets );
        for ( ; it.hasNext(); ) {
            const QString &sit = it.next();
            ProfileItem *si = new ProfileItem(pi, prof->title(sit), prof->charSet(sit));
            Q_UNUSED( si );
            // connect(si, SIGNAL(selected(QTreeWidgetItem*)), this, SLOT(editItem(QTreeWidgetItem*)));
        }
        lv->expandItem( pi );
        // connect(pi, SIGNAL(selected(QTreeWidgetItem*)), this, SLOT(editItem(QTreeWidgetItem*)));
    }

    /*
    QTreeWidgetItem *item = lv->topLevelItem( 0 );
    if ( item != NULL )
        lv->setCurrentItem( item );
        */

    // still interesting to know the 'default'
    return true;
}

bool QIMPenProfileEdit::saveProfiles()
{
    // Save charsets
    bool ok = true;
    for ( int i = 0; i < (int)profileList.count(); i++ ) {
        QIMPenProfile *prof = profileList.at(i);
        QStringList cSets = prof->charSets();
        QStringList::Iterator it;
        for ( it = cSets.begin(); it != cSets.end(); ++it ) {
            QIMPenCharSet *set = prof->charSet(*it);
            if ( !set->save() ) {
                ok = false;
                break;
            }
        }
    }
    if ( !ok ) {
        if ( QMessageBox::critical( 0, tr( "Out of space" ),
                    tr("<qt>Unable to save information. "
                        "Free up some space "
                        "and try again."
                        "<br>Quit anyway?</qt>"),
                    QMessageBox::Yes|QMessageBox::Escape,
                    QMessageBox::No|QMessageBox::Default )
                != QMessageBox::No ) {
            ok = true;
        }
    } else {
        QtopiaIpcEnvelope e("QPE/Handwriting", "settingsChanged()"); // or, via caching and time-stamps. // no tr
    }
    return ok;
}

void QIMPenProfileEdit::editItem(QTreeWidgetItem *i)
{
    if (!i)
        return;
    // XXX to item which might be a or be
    ProfileItem *pi = (ProfileItem *)i;
    if (pi->set()) {
        if (cdiag == 0) {
            cdiag = new CharSetDlg(this, "chng", true);
        }
        cdiag->setCharSet(pi->set());
        cdiag->setIsFS(((ProfileItem *)pi->parent())->profile()->canIgnoreStroke());

        if (QtopiaApplication::execDialog(cdiag)) {
            // need to save the changes
            pi->set()->save();
            QtopiaIpcEnvelope e("QPE/Handwriting", "settingsChanged()"); // no tr
        } else {
            loadProfiles(); // only way to get combining data as well in set.
        }

    } else if (pi->profile()) {
        if (gdiag == 0) {
            gpb = new GeneralPref( this );
        }
        QIMPenProfile *profile = pi->profile();

        gpb->multiStrokeSlider->setValue(profile->multiStrokeTimeout());
        gpb->ignoreStrokeSlider->setValue(profile->ignoreStrokeTimeout());
        gpb->inputStyle->setChecked(profile->style() == QIMPenProfile::BothCases);
        if (profile->canSelectStyle())
            gpb->inputStyle->show();
        else
            gpb->inputStyle->hide();

        if (profile->canIgnoreStroke()) {
            gpb->ignoreStrokeLabel->show();
            gpb->ignoreStrokeSlider->show();
        } else {
            gpb->ignoreStrokeLabel->hide();
            gpb->ignoreStrokeSlider->hide();
        }

        if (QtopiaApplication::execDialog(gpb)) {
            profile->setMultiStrokeTimeout(gpb->multiStrokeSlider->value());
            profile->setIgnoreStrokeTimeout(gpb->ignoreStrokeSlider->value());
            profile->setStyle(gpb->inputStyle->isChecked() ? QIMPenProfile::BothCases : QIMPenProfile::ToggleCases);
            profile->save();
            QtopiaIpcEnvelope e("QPE/Handwriting", "settingsChanged()"); // no tr
        }
    }
}

#include "mainwindow.moc"
