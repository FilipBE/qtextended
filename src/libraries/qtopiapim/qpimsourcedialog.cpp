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

#include <qpimmodel.h>
#include <qpimsourcemodel.h>
#include <qpimsourcedialog.h>

#include <qtopiaitemdelegate.h>

#include <QListView>
#include <QVBoxLayout>

class QPimSourceDialogData
{
public:
    QPimSourceDialogData() : pimModel(0), view(0), model(0) {}
    QPimModel *pimModel;
    QListView *view;
    QPimSourceModel *model;
};


/*!
  \class QPimSourceDialog
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QPimSourceDialog class provides a dialog for selecting visible PIM sources for
  a PIM model.

  This is useful to allow the user to filter out certain types
  of QPimRecords - for example, to only show QContacts that are
  stored on a SIM card.

  This dialog can be subclassed if further functionality is
  required (e.g. adding sources, syncing between sources, or
  importing/exporting)

  You must specify the PIM model to operate on using \l setPimModel().
  This dialog will query the model for the available PIM sources, and
  present the user with a checkable list.  Once the user accepts the
  dialog, the model's list of visible sources will automatically be
  updated to the dialog's selected sources.

  The window title should be set by the creator.

  \sa QPimSource, QPimSourceModel, QPimModel, {Pim Library}
*/

/*!
  Constructs a QPimSourceDialog with the given \a parent.
*/
QPimSourceDialog::QPimSourceDialog(QWidget *parent)
    : QDialog(parent)
{
    d = new QPimSourceDialogData;
    d->view = new QListView;
    d->view->setSelectionMode(QAbstractItemView::SingleSelection);
    d->view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->view->setWordWrap(true);
    d->view->setFrameStyle(QFrame::NoFrame);
    d->view->setResizeMode(QListView::Adjust);
    d->view->setItemDelegate(new QtopiaItemDelegate());
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(d->view);
    layout->setMargin(0);
    setLayout(layout);

    d->model = new QPimSourceModel(this);
    d->view->setModel(d->model);
}

/*!
  Destroys a QPimSourceDialog.
*/
QPimSourceDialog::~QPimSourceDialog()
{
    delete d;
}

/*!
  Set the PIM model this dialog will operate on to \a m.  The initial
  list of PIM sources and their state will be obtained from
  \l QPimModel::contexts() and \l QPimModel::visibleSources().

  If this dialog is accepted by the user, this model will be
  updated immediately.
*/
void QPimSourceDialog::setPimModel(QPimModel *m)
{
    d->pimModel = m;
    d->model->setContexts(d->pimModel->contexts());
    d->model->setCheckedSources(d->pimModel->visibleSources());
    d->view->selectionModel()->setCurrentIndex(d->model->index(0,0), QItemSelectionModel::ClearAndSelect);
}

/*!
  When the user accepts this dialog, the PIM model supplied
  by \c setPimModel() will be updated so that its visible
  sources are set to the selected list of PIM sources.
*/
void QPimSourceDialog::accept()
{
    if(d->pimModel)
        d->pimModel->setVisibleSources(d->model->checkedSources());
    QDialog::accept();
}
/*!
  Return the QPimSourceModel used by this dialog.
  Mostly useful for subclassing this dialog for extra
  functionality.
*/
QPimSourceModel* QPimSourceDialog::pimSourceModel() const
{
    return d->model;
}

