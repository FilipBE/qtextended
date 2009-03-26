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

#include "simfilesystem.h"
#include <qatutils.h>
#include <qdebug.h>

// Known files, their parent directories, and names.
// This list comes from 3GPP TS 11.11 and 3GPP TS 51.011.
typedef struct
{
    const char *fileid;
    const char *parent;
    const char *name;

} SimFileInfo;
static SimFileInfo const knownFiles[] =
{
    {"3F00",        0,          "MF"},

    {"2FE2",        "3F00",     "EFiccid"},
    {"2F05",        "3F00",     "EFelp"},

    {"7F10",        "3F00",     "DFtelecom"},
    {"6F3A",        "7F10",     "EFadn"},
    {"6F3B",        "7F10",     "EFfdn"},
    {"6F3C",        "7F10",     "EFsms"},
    {"6F3D",        "7F10",     "EFccp"},
    {"6F40",        "7F10",     "EFmsisdn"},
    {"6F42",        "7F10",     "EFsmsp"},
    {"6F43",        "7F10",     "EFsmss"},
    {"6F44",        "7F10",     "EFlnd"},
    {"6F47",        "7F10",     "EFsmsr"},
    {"6F49",        "7F10",     "EFsdn"},
    {"6F4A",        "7F10",     "EFext1"},
    {"6F4B",        "7F10",     "EFext2"},
    {"6F4C",        "7F10",     "EFext3"},
    {"6F4D",        "7F10",     "EFbdn"},
    {"6F4E",        "7F10",     "EFext4"},
    {"6F4F",        "7F10",     "EFeccp"},          // 51.011
    {"6F58",        "7F10",     "EFcmi"},           // 51.011

    {"5F50",        "7F10",     "DFgraphics"},
    {"4F20",        "5F50",     "EFimg"},
    {"4F01",        "5F50",     "EFimg1"},          // Usual names of
    {"4F02",        "5F50",     "EFimg2"},          // Image data files.
    {"4F03",        "5F50",     "EFimg3"},
    {"4F04",        "5F50",     "EFimg4"},
    {"4F05",        "5F50",     "EFimg5"},

    {"7F20",        "3F00",     "DFgsm"},
    {"6F05",        "7F20",     "EFlp"},
    {"6F07",        "7F20",     "EFimsi"},
    {"6F20",        "7F20",     "EFkc"},
    {"6F2C",        "7F20",     "EFdck"},           // 51.011
    {"6F30",        "7F20",     "EFplmnsel"},
    {"6F31",        "7F20",     "EFhpplmn"},
    {"6F32",        "7F20",     "EFcnl"},           // 51.011
    {"6F37",        "7F20",     "EFacmmax"},
    {"6F38",        "7F20",     "EFsst"},
    {"6F39",        "7F20",     "EFacm"},
    {"6F3E",        "7F20",     "EFgid1"},
    {"6F3F",        "7F20",     "EFgid2"},
    {"6F41",        "7F20",     "EFpuct"},
    {"6F45",        "7F20",     "EFcbmi"},
    {"6F46",        "7F20",     "EFspn"},
    {"6F48",        "7F20",     "EFcbmid"},
    {"6F50",        "7F20",     "EFcbmir"},
    {"6F51",        "7F20",     "EFnia"},
    {"6F52",        "7F20",     "EFkcgprs"},
    {"6F53",        "7F20",     "EFlocigprs"},
    {"6F54",        "7F20",     "EFsume"},
    {"6F60",        "7F20",     "EFplmnwact"},
    {"6F61",        "7F20",     "EFoplmnwact"},
    {"6F62",        "7F20",     "EFhplmnwact"},
    {"6F63",        "7F20",     "EFcpbcch"},
    {"6F64",        "7F20",     "EFinvscan"},
    {"6F74",        "7F20",     "EFbcch"},
    {"6F78",        "7F20",     "EFacc"},
    {"6F7B",        "7F20",     "EFfplmn"},
    {"6F7E",        "7F20",     "EFloci"},
    {"6FAD",        "7F20",     "EFad"},
    {"6FAE",        "7F20",     "EFphase"},
    {"6FB1",        "7F20",     "EFvgcs"},
    {"6FB2",        "7F20",     "EFvgcss"},
    {"6FB3",        "7F20",     "EFvbs"},
    {"6FB4",        "7F20",     "EFvbss"},
    {"6FB5",        "7F20",     "EFemlpp"},
    {"6FB6",        "7F20",     "EFaaem"},
    {"6FB7",        "7F20",     "EFecc"},
    {"6FC5",        "7F20",     "EFpnn"},           // 51.011
    {"6FC6",        "7F20",     "EFopl"},           // 51.011
    {"6FC7",        "7F20",     "EFmbdn"},          // 51.011
    {"6FC8",        "7F20",     "EFext6"},          // 51.011
    {"6FC9",        "7F20",     "EFmbi"},           // 51.011
    {"6FCA",        "7F20",     "EFmwis"},          // 51.011
    {"6FCB",        "7F20",     "EFcfis"},          // 51.011
    {"6FCC",        "7F20",     "EFext7"},          // 51.011
    {"6FCD",        "7F20",     "EFspdi"},          // 51.011
    {"6FCE",        "7F20",     "EFmmsn"},          // 51.011
    {"6FCF",        "7F20",     "EFext8"},          // 51.011
    {"6FD0",        "7F20",     "EFmmsicp"},        // 51.011
    {"6FD1",        "7F20",     "EFmmsup"},         // 51.011
    {"6FD2",        "7F20",     "EFmmsucp"},        // 51.011

    {"5F30",        "7F20",     "DFiridium"},
    {"5F31",        "7F20",     "DFglobst"},
    {"5F32",        "7F20",     "DFico"},
    {"5F33",        "7F20",     "DFaces"},

    {"5F40",        "7F20",     "DFeia/tia-553"},
    {"4F80",        "5F40",     "EFsid"},           // 51.011
    {"4F81",        "5F40",     "EFgpi"},           // 51.011
    {"4F82",        "5F40",     "EFipc"},           // 51.011
    {"4F83",        "5F40",     "EFcount"},         // 51.011
    {"4F84",        "5F40",     "EFnsid"},          // 51.011
    {"4F85",        "5F40",     "EFpsid"},          // 51.011
    {"4F86",        "5F40",     "EFnetsel"},        // 51.011
    {"4F87",        "5F40",     "EFspl"},           // 51.011
    {"4F88",        "5F40",     "EFmin"},           // 51.011
    {"4F89",        "5F40",     "EFaccolc"},        // 51.011
    {"4F8A",        "5F40",     "EFfc1"},           // 51.011
    {"4F8B",        "5F40",     "EFs-esn"},         // 51.011
    {"4F8C",        "5F40",     "EFcsid"},          // 51.011
    {"4F8D",        "5F40",     "EFreg-thresh"},    // 51.011
    {"4F8E",        "5F40",     "EFccch"},          // 51.011
    {"4F8F",        "5F40",     "EFldcc"},          // 51.011
    {"4F90",        "5F40",     "EFgsm-recon"},     // 51.011
    {"4F91",        "5F40",     "EFamps-2-gsm"},    // 51.011
    {"4F93",        "5F40",     "EFamps-ui"},       // 51.011

    {"5F60",        "7F20",     "DFcts"},

    {"5F70",        "7F20",     "DFsolsa"},
    {"4F30",        "5F70",     "EFsai"},
    {"4F31",        "5F70",     "EFsll"},

    {"5F3C",        "7F20",     "DFmexe"},
    {"4F40",        "5F3C",     "EFmexe-st"},
    {"4F41",        "5F3C",     "EForpk"},
    {"4F42",        "5F3C",     "EFarpk"},
    {"4F43",        "5F3C",     "EFtprpk"},

    {"7F22",        "3F00",     "DFis41"},

    {"7F23",        "3F00",     "DFfp-cts"},

    {0,             0,          0}
};

SimFileSystem::SimFileSystem( SimRules *rules, SimXmlNode& e )
    : QObject( rules )
{
    this->rules = rules;
    rootItem = new SimFileItem( "3F00", 0 );
    currentItem = rootItem;

    // Create all of the standard directories.
    const SimFileInfo *info = knownFiles;
    SimFileItem *dirItem = 0;
    while ( info->fileid ) {
        QString fileid = info->fileid;
        if ( fileid.startsWith( "7F" ) ) {
            dirItem = new SimFileItem( fileid, rootItem );
        } else if ( fileid.startsWith( "5F" ) ) {
            new SimFileItem( fileid, dirItem );
        }
        ++info;
    }

    // Load the file definitions.
    SimXmlNode *child = e.children;
    while ( child ) {
        if ( child->tag == "file" ) {
            QString name = child->getAttribute( "name" );
            QByteArray data = QAtUtils::fromHex( child->contents );
            QString fileid = resolveFileId( name );
            SimFileItem *parent = findItemParent( fileid );
            if ( parent ) {
                SimFileItem *item;
                item = findItem( fileid.right(4) );
                if ( !item )
                    item = new SimFileItem( fileid.right(4), parent );
                else
                    qDebug() << "File" << name << "defined multiple times";
                item->setContents( data );
                QString size = child->getAttribute( "recordsize" );
                if ( !size.isEmpty() )
                    item->setRecordSize( size.toInt() );
            } else {
                qDebug() << "Could not find parent for" << name;
            }
        } else {
            qDebug() << "Unknown filesystem command <" << child->tag << ">";
        }
        child = child->next;
    }
}

SimFileSystem::~SimFileSystem()
{
    delete rootItem;
}

void SimFileSystem::crsm( const QString& args )
{
    // Extract the arguments to the command.
    uint posn = 0;
    uint command = QAtUtils::parseNumber( args, posn );
    QString fileid =
        QString::number( QAtUtils::parseNumber( args, posn ), 16 ).toUpper();
    uint p1 = QAtUtils::parseNumber( args, posn );
    uint p2 = QAtUtils::parseNumber( args, posn );
    uint p3 = QAtUtils::parseNumber( args, posn );
    QString data = QAtUtils::fromHex( args.mid( (int)posn ) );

    // Determine how to execute the command.
    bool ok = true;
    SimFileItem *item;
    QString response;
    int sw1 = 0;
    int sw2 = 0;
    int offset, length;
    QByteArray contents;
    switch ( command ) {

        case 176:       // READ BINARY
        {
            offset = (int)((p1 << 8) + p2);
            length = (int)p3;
            item = findItemRelative( fileid );
            if ( item ) {
                contents = item->contents();
                if ( ( offset + length ) > contents.size() ) {
                    sw1 = 0x94;
                    sw2 = 0x02;
                } else {
                    sw1 = 0x9F;
                    sw2 = length;
                    response =
                        QAtUtils::toHex( contents.mid( offset, length ) );
                }
            } else {
                sw1 = 0x94;
                sw2 = 0x04;
            }
        }
        break;

        case 178:       // READ RECORD
        {
            offset = (int)(p1 - 1);
            length = (int)p3;
            item = findItemRelative( fileid );
            if ( p2 == 0x04 && item ) { // Only absolute reads are supported.
                offset *= item->recordSize();
                contents = item->contents();
                if ( ( offset + length ) > contents.size() ) {
                    sw1 = 0x94;
                    sw2 = 0x02;
                } else {
                    sw1 = 0x9F;
                    sw2 = length;
                    response =
                        QAtUtils::toHex( contents.mid( offset, length ) );
                }
            } else {
                sw1 = 0x94;
                sw2 = 0x08;
            }
        }
        break;

        case 192:       // GET RESPONSE
        {
            if ( !findItemRelative( fileid ) ) {
                sw1 = 0x94;
                sw2 = 0x04;
                break;
            }
        }
        // Fall through to the next case.

        case 242:       // STATUS
        {
            // Format the status response according to GSM 51.011, 9.2.1.
            char status[15];
            status[0] = 0x00;           // RFU
            status[1] = 0x00;
            int size = currentItem->contents().size();
            status[2] = (char)(size >> 8);
            status[3] = (char)size;
            status[4] = fileid.left(2).toInt(0, 16);
            status[5] = fileid.right(2).toInt(0, 16);
            if ( currentItem == rootItem ) {
                status[6] = 0x01;
            } if ( currentItem->isDirectory() ) {
                status[6] = 0x02;
            } else {
                status[6] = 0x04;
            }
            status[7] = 0x00;           // RFU
            status[8] = 0x00;
            status[9] = 0x00;
            status[10] = 0x00;
            status[11] = 0x00;
            status[12] = 2;             // Size of data that follows.
            if ( currentItem->isDirectory() ) {
                status[13] = 0x00;
                status[14] = 0x00;
            } else if ( currentItem->recordSize() > 1 ) {
                status[13] = 0x01;
                status[14] = (char)( currentItem->recordSize() );
            } else {
                status[13] = 0x00;
                status[14] = 0x00;
            }
            sw1 = 0x90;
            sw2 = 0x00;
            response = QAtUtils::toHex( QByteArray( status, 15 ) );
        }
        break;

        case 214:       // UPDATE BINARY
        {
            // Not implemented yet.
            sw1 = 0x94;
            sw2 = 0x08;
        }
        break;

        case 220:       // UPDATE RECORD
        {
            // Not implemented yet.
            sw1 = 0x94;
            sw2 = 0x08;
        }
        break;

        default: ok = false; break;
    }

    // Send the response information.
    if ( sw1 != 0 ) {
        QString resp;
        resp = "+CRSM: " + QString::number(sw1) + "," + QString::number(sw2);
        if ( !response.isEmpty() )
            resp += "," + response;
        rules->respond( resp );
    }
    if ( ok )
        rules->respond( "OK" );
    else
        rules->respond( "ERROR" );
}

SimFileItem *SimFileSystem::findItem( const QString& fileid ) const
{
    return rootItem->findItem( fileid );
}

SimFileItem *SimFileSystem::findItemParent( const QString& fileid ) const
{
    QString parent = fileid.left( fileid.length() - 4 );
    if ( parent.isEmpty() )
        return rootItem;
    else
        return findItem( parent.right(4) );
}

SimFileItem *SimFileSystem::findItemRelative( const QString& fileid )
{
    SimFileItem *item;
    if ( fileid == "3F00" ) {
        // We can always find the root directory no matter where we are.
        item = rootItem;
    } else if ( currentItem->fileid() == fileid ) {
        // We can always find where we currently are.
        item = currentItem;
    } else if ( currentItem->parentDir() &&
                currentItem->parentDir()->fileid() == fileid ) {
        // We can always find our parent directory.
        item = currentItem->parentDir();
    } else {
        // Search the current item's immediate children.
        QList<SimFileItem *> children = currentItem->children();
        item = 0;
        foreach( SimFileItem *temp, children ) {
            if ( temp->fileid() == fileid ) {
                item = temp;
                break;
            }
        }
        if ( !item )
            return 0;
    }
    currentItem = item;
    return item;
}

QString SimFileSystem::resolveFileId( const QString& _fileid ) const
{
    QString fileid = _fileid;

    // Convert alphabetic names into their numeric equivalents.
    const SimFileInfo *info = knownFiles;
    while ( info->fileid ) {
        if ( fileid == info->name ) {
            fileid = info->fileid;
            break;
        }
        ++info;
    }

    // Determine if the fileid is already a full path.
    if ( fileid.startsWith( "3F" ) ||       // MF root directory.
         fileid.startsWith( "7F" ) ||       // First-level DF directory.
         fileid.startsWith( "2F" ) )        // First-level EF file.
        return fileid;

    // Extract the first component and search for a path back to the root.
    QString newId = fileid;
    QString first = fileid.left(4);
    for(;;) {
        info = knownFiles;
        while ( info->fileid ) {
            if ( first == info->fileid && info->parent ) {
                first = info->parent;
                newId = first + newId;
                if ( first.startsWith( "3F" ) || first.startsWith( "7F" ) )
                    return newId;
                break;
            }
            ++info;
        }
        if ( !info->fileid ) {
            // We could not find a suitable parent directory, so bail out.
            return newId;
        }
    }
}

SimFileItem::SimFileItem( const QString& fileid, SimFileItem *parentDir )
    : QObject( parentDir )
{
    _fileid = fileid;
    _parentDir = parentDir;
    _recordSize = 1;
    _isDirectory = false;
    if ( parentDir ) {
        parentDir->_isDirectory = true;
        parentDir->_children.append( this );
    }
}

SimFileItem::~SimFileItem()
{
}

SimFileItem *SimFileItem::findItem( const QString& fileid )
{
    if ( fileid == _fileid )
        return this;
    foreach ( SimFileItem *item, _children ) {
        SimFileItem *temp = item->findItem( fileid );
        if ( temp )
            return temp;
    }
    return 0;
}
