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


#include "qmemoryfile_p.h"
#include "qfile.h"

/*****************************************************************************
  QMemoryFile member functions
 *****************************************************************************/

/*
  \class QMemoryFile
    \inpublicgroup QtBaseModule

  \intern
  \brief The QMemoryFile class provides a means to map a file info memory or reference
  to a block or memory by name.

  Warning: This class is not part of the Qt API and subject to change.
             This file file may change from version to version without
             notice, or even be removed.


  Currently this class supports read only access on all platforms; and read plus write
  accces provided on unix and windows based plaforms.  The size of mapped file can not be changed.

  \ingroup io
*/


/*! \enum QMemoryFile::Flags
  This enum specifies the possible types of connections made to the file to
  be memory mapped.

  The currently defined values are :

  \value Write  Allow write access to file.
  \value Shared Allow file to be shared with other proceses.
  \value Create Create file named.
 */

/*!
  \fn QMemoryFileData * QMemoryFile::openData (const QString &fileName, int flags,
                                         uint size )
  Initializes the memory map.  If sucessful wil return a pointer to the memory file
  data.  Otherwise returns null.
*/

/*!
  \fn void QMemoryFile::closeData(QMemoryFileData *memoryFile)
  Closes the memory file and frees any memory used.
*/

/*
  Construct a memory mapped to an actual file or named memory block.
  If \a fileName is not preceeded by an \\ then it is treated as a real
     file name.
  Otherwise the characters after the \\ are used as the name for the memory block required.
    nb: You will need to escape the \'s so it you need to use \\\\ within a literal string
  A value for \a size should always be provided, though if the size is not provided it will be determined if possible.

 If a file is being mapped to be written to, then the \a flags of subsequent QMemoryFiles
 mapped to the same file should include QMemoryFile::Write.

 Example:
 \code
 // Open a maping to file text.dat
 QMemoryFile memoryFile("text.dat", QMemoryFile:Read, 20);
 char *data = memoryFile.data();
 int sum = 0;
 for (int i = 0; i < 20; i++){
   sum = sum + data[i];
 }
 \endcode

 Example for creating named memory block:
 \code

 QMemoryFile block1("\\\\block1", QMemoryFile::Create | QMemoryFile::Write, 20);
 char *dataBlock = block.data();
 for (int i = 0; i < 19; i++){
   dataBlock[i] = i +'a';
 }
 dataBlock[20] = '\0';
 \endcode


*/
QMemoryFile::QMemoryFile(const QString &fileName, int flags, uint size)
{
    block = NULL;
    length = 0;
    if (flags == -1)
      flags = QMemoryFile::Shared; // read only shared file mapping

    this->flags = flags;
    d = openData(fileName, flags, size);
}

/*
  Destructs the memory mapped file
*/
QMemoryFile::~QMemoryFile()
{
    closeData(d);
}

/*!
  \fn uint QMemoryFile::size()

  Returns the length of the block of memory that this QMemoryFile is associated with.
*/

/*!
 \fn char* QMemoryFile::data()

 Returns a pointer to memory that this QMemoryFile is associated with.
*/


/*!
  \fn bool QMemoryFile::isShared()

  Returns true if the memory block is shared.
*/


/*!
 \fn bool QMemoryFile::isWritable()

 Returns true if the memory block is writable.
*/

