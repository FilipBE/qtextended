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
#include "qtopiaresource_p.h"
#include <qtopianamespace.h>
#include <qtopiaservices.h>
#include <qtopialog.h>
#include <QImageIOHandler>
#include <QImageIOPlugin>
#include <QImage>
#include <QPair>
#include <QList>
#include <QCache>

#include <QResource>
#include <private/qresource_p.h>
//#define ENABLE_RESOURCEFILEENGINE

#include <QDebug>
#include <QApplication>
#include <QStyle>
#include <qfsfileengine.h>
#include <QImageReader>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static bool fileExists(const QByteArray &filename)
{
    // assumes that filename is not a resource path.
    if(filename[0]==':')
        return false;
    struct stat statbuf;
    int rv = stat(filename, &statbuf);
    if (rv == 0 && statbuf.st_mode & S_IRUSR)
        return true;
    if(filename.contains('*') || filename.contains('?'))
    {
        QFileInfo fi(filename);
        QDir dir(fi.absolutePath(), fi.fileName());
        if(dir.entryList().count() > 0)
            return true;
    }
    return false;
}

static void expandService(QString& path)
{
    static const QString service(QLatin1String("service/"));
    if ( path.startsWith(service) ) {
        /* 8 == ::strlen("service/") */
        int slash = path.indexOf(QChar('/'),8);
        if ( slash >= 0 ) {
            QString serv = path.mid(8,slash-8);
            QString appname = QtopiaService::app(serv);
            if ( !appname.isEmpty() )
                path.replace(0,slash,appname);
        }
    }
}

QFileResourceFileEngineHandler::QFileResourceFileEngineHandler()
    : QAbstractFileEngineHandler()
{
    QImageReader::supportedImageFormats();
}

QFileResourceFileEngineHandler::~QFileResourceFileEngineHandler()
{
}

void QFileResourceFileEngineHandler::setIconPath(const QStringList& p)
{
    if (!p.isEmpty()) {
        foreach (QString ip, p) {
            QByteArray ipath = ip.toLocal8Bit();
            if (ipath[0] == '/' && fileExists(ipath)) {
                if (!ipath.endsWith('/'))
                    ipath += '/';
                iconpath.append(ipath);
                qLog(Resource) << "Add pic search path" << ipath;
            } else {
                QStringList p = Qtopia::installPaths();
                foreach (QString s, p) {
                    QByteArray t = s.toLocal8Bit() + ipath;
                    if (fileExists(t)) {
                        if (!t.endsWith('/'))
                            t += '/';
                        iconpath.append(t);
                        qLog(Resource) << "Add pic search path" << t;
                    }
                }
            }
        }
    }
    imagedirs.clear();
    sounddirs.clear();
}

class QIODeviceFileEngine : public QAbstractFileEngine {
public:
    QIODeviceFileEngine(QIODevice *d) : dev(d) {}

    bool open(QIODevice::OpenMode openMode) { return dev->open(openMode); }
    bool close() { dev->close(); return true; }
    qint64 size() const { return dev->size(); }
    qint64 pos() const { return dev->pos(); }
    bool seek(qint64 pos) { return dev->seek(pos); }
    bool isSequential() const { return dev->isSequential(); }

    qint64 read(char *data, qint64 maxlen) { return dev->read(data,maxlen); }
    qint64 readLine(char *data, qint64 maxlen) { return dev->readLine(data,maxlen); }
    qint64 write(const char *data, qint64 len) { return dev->write(data,len); }

protected:
    QIODevice* dev; // not owned by QIODeviceFileEngine
};

class QDataFileEngine : public QIODeviceFileEngine {
public:
    QDataFileEngine(QByteArray data) : QIODeviceFileEngine(new QBuffer())
        { ((QBuffer*)dev)->setData(data); }
    ~QDataFileEngine() { delete dev; } // owned by QDataFileEngine
};

QAbstractFileEngine *QFileResourceFileEngineHandler::create(const QString &path) const
{
    if ( path.length() > 0 && path[0] == ':' ) {
        if ( imagedirs.isEmpty() ) {
            imagedirs = iconpath;
            QStringList p = Qtopia::installPaths();
            foreach (QString s, p) {
                appendSearchDirs(imagedirs,s,"pics/");
                appendSearchDirs(sounddirs,s,"sounds/");
            }
        }

#ifdef ENABLE_RESOURCEFILEENGINE
        QAbstractFileEngine * e = findArchivedResourceFile(path);
        if(e)
            return e;
#endif

        QString p = findDiskResourceFile(path);
        if (!p.isNull())
            return new QFSFileEngine(p);
        if (path[0] == ':') {
            if ( path.left(6) == ":data/" )
                return new QDataFileEngine(QByteArray::fromBase64(path.mid(6).toLatin1()));
        }
    }
    return 0;
}

void QFileResourceFileEngineHandler::appendSearchDirs(QList<QByteArray>& dirs,
        const QString& dir, const char *subdir) const
{
    QByteArray t = dir.toLocal8Bit() + subdir;
    if (fileExists(t))
        dirs.append(t);
}

/*!
  \page qtopia_resource_system.html
  \title Qt Extended Resource System

  The Qt Extended resource system allows application programmers to access common
  application resources such as images, icons and sounds without having to
  concern themselves with the exact installation location or file types.  The
  Qt Extended resource system is built on top of the Qt for Embedded Linux resource model.

  Rather than accessing resource files directly from disk, applications should
  use resource syntax inplace of a regular file name.  For example, the
  following lines refer to the same image:

  \code
  QPixmap pix1("/opt/Qtopia/pics/addressbook/email.png");
  QPixmap pix2(":image/addressbook/email");
  QPixmap pix2(":image/service/Contacts/email");
  \endcode

  When Qt Extended detects the use of the special ":" prefix, searches in various
  locations - depending on the resource type - and for various file types
  to locate the actual resource.  In addition to improving the efficiency of
  reference for the programmer, the Qt Extended resource system improves the
  efficiency of access for the system.  Using a special file, known as a
  resource database, Qt Extended can bundle many separate images and icons into a
  single archive that is both quick to access and efficiently shared across
  processes.

  A resource database is created using the Qt for Embedded Linux \c {rcc} tool in binary
  mode.  Any type of Qt Extended supported image and icon can be added to a resource
  database.  A special image type, known as a \i {QRAW} image, is also
  exclusively supported in resource databases.  A \i {QRAW} image is an
  uncompressed raw image that can be mmap'ed directly from disk and efficiently
  displayed on screen with no resident in-memory copies.  As the \i {QRAW} format
  is uncompressed, it is best reserved for small images, very frequently used images,
  or images stored on compressed filesystems.  \i {QRAW} images can be created
  from other image types using the \c {mkqraw} tool included with Qtopia.

  Resource databases are always named \c {qtopia.rdb} and stored in the location
  dictated by the resouces they contain.  The list of search directories
  outlined for each resource type below can be used to determine where a
  resource database be placed.  In general, resource databases are placed in
  either the \c {/opt/Qtopia/pics} or \c {/opt/Qtopia/pics/<application name>}
  directories, but may be located elsewhere in the case of installable software
  or resources.

  Resource databases are only supported in the Qtopia 4.2 series and later.

  The specifics for each resource type are outlined below. Note that
  the \c {<app name>} may also be specified by refering to a service rather
  than the application which might provide the service - for example
  "service/Contacts/" rather than "addressbook/".

  \section1 Images

  When requesting an image, applications use a "filename" of the form
  \c {<path> := :image/[i18n/][<app name>/]<image>}.
  For each search directory
  listed in the $QTOPIA_PATHS environment variable,
  the following sub-locations are tried:

  \c {pics/<QApplication::applicationName()>/qtopia.rdb#<app name>/<image>}

  \c {pics/qtopia.rdb#<QApplication::applicationName()>/<app name>/<image>}

  \c {pics/<app name>/qtopia.rdb#<image>}

  \c {pics/qtopia.rdb#<app name>/<image>}

  \i {i18n only:} \c {pics/<QApplication::applicationName()>/<app name>/i18n/<language>_<locale>/<image>.<image extension>}

  \i {i18n only:} \c {pics/<app name>/i18n/<language>_<locale>/<image>.<image extension>}

  \i {i18n only:} \c {pics/<QApplication::applicationName()>/<app name>/i18n/<language>/<image>.<image extension>}

  \i {i18n only:} \c {pics/<app name>/i18n/<language>/<image>.<image extension>}

  \i {i18n only:} \c {pics/<QApplication::applicationName()>/<app name>/i18n/en_US/<image>.<image extension>}

  \i {i18n only:} \c {pics/<app name>/i18n/en_US/<image>.<image extension>}

  \c {pics/<QApplication::applicationName()>/<app name>/<image>.<image extension>}

  \c {pics/<app name>/<image>.<image extension>}

  In the listing above, \c {<language>} and \c {<locale>} correspond to the current language
  and locale selections.  The supported
  <image extensions> are currently "pic", "svg", "png", "jpg", "mng" and no
  extension.

  For example, in the "addressbook" application

  \code
  // /opt/Qtopia/pics/addressbook/qtopia.rdb#email
  // /opt/Qtopia/pics/qtopia.rdb#addressbook/email
  // /opt/Qtopia/pics/qtopia.rdb#email
  // /opt/Qtopia/pics/addressbook/email.pic
  // /opt/Qtopia/pics/addressbook/email.svg
  // /opt/Qtopia/pics/addressbook/email.png
  // /opt/Qtopia/pics/addressbook/email.jpg
  // /opt/Qtopia/pics/addressbook/email.mng
  // /opt/Qtopia/pics/addressbook/email
  // /opt/Qtopia/pics/email.pic
  // /opt/Qtopia/pics/email.svg
  // /opt/Qtopia/pics/email.png
  // /opt/Qtopia/pics/email.jpg
  // /opt/Qtopia/pics/email.mng
  // /opt/Qtopia/pics/email
  QPixmap pix(":image/email");
  \endcode

  More information on image translation can be found in the \l{Internationalization#image-translation}{Internationalization} guide.

  Themes can override images by specifying an \c{IconPath}. See
  \l{Images and Icons#installing-custom-icons}.

  See \l{Images and Icons} for more information on images in Qtopia.

  \section1 Icons

  When requesting an icon, applications use a "filename" of the form
  \c {<path> := :icon/[i18n/][<app name>/]<icon>}.  For each search directory
  listed in the $QTOPIA_PATHS environment variable,
  the following sub-locations are tried:

  \c {pics/<QApplication::applicationName()>/qtopia.rdb#<app name>/icons/<icon>}

  \c {pics/qtopia.rdb#<QApplication::applicationName()>/<app name>/icons/<icon>}

  \c {pics/<app name>/qtopia.rdb#icons/icon}

  \c {pics/qtopia.rdb#<app name>/icons/icon}

  \c {pics/<QApplication::applicationName()>/<app name>/icons/i18n/<language>_<locale>/icon.<icon extension>}

  \i {i18n only:} \c {pics/<app name>/icons/i18n/<language>_<locale>/icon.<icon extension>}

  \i {i18n only:} \c {pics/<QApplication::applicationName()>/<app name>/icons/i18n/<language>/icon.<icon extension>}

  \i {i18n only:} \c {pics/<app name>/icons/i18n/<language>/icon.<icon extension>}

  \i {i18n only:} \c {pics/<QApplication::applicationName()>/<app name>/icons/i18n/en_US/icon.<icon extension>}

  \i {i18n only:} \c {pics/<app name>/icons/i18n/en_US/icon.<icon extension>}

  \c {pics/<QApplication::applicationName()>/icons/<app name>/icon>.<icon extension>}

  \c {pics/<app name>/icons/<icon>.<icon extension>}

  \i {If none found, search for :image/[i18n/][<app name>/]<icon> as though the icon was requested as an image}

  In the listing above, \c {<language>} and \c {<locale>} correspond to the current
  language and locale.  The supported
  <icon extensions> are currently "pic", "svg", "png", "jpg", "mng" and no extension.

  More information on icon translation can be found in the \l{Internationalization#image-translation}{Internationalization} guide.

  Themes can override images by specifying an \c{IconPath}. See
  \l{Images and Icons#installing-custom-icons}.

  See \l{Images and Icons} for more information on icons in Qtopia.

  \section1 Sounds

  When requesting a sound, applications use a "filename" of the form
  \c {<path> := :sound/<sound>}.  For each search directory listed in the
  $QTOPIA_PATHS environment variable, the following sub-locations are tried:

  \c {sounds/<QApplication::applicationName()>/<sound>.wav}

  \c {sounds/<sound>.wav}
 */

#ifdef ENABLE_RESOURCEFILEENGINE
QAbstractFileEngine *QFileResourceFileEngineHandler::findArchivedResourceFile(const QString &path) const
{
    if ( path.left(7 /* ::strlen(":image/") */)==":image/" ) {

        QString p1 = path.mid(7 /* ::strlen(:image/") */);
        return findArchivedImage(p1);

    } else if ( path.left(6 /* ::strlen(":icon/") */)==":icon/" ) {

        QString p1 = path.mid(6 /* ::strlen(":icon/") */);
        return findArchivedIcon(p1);

    }

    return 0;
}

/*! Returns an archive identifier if succeeds, empty string if not */
QString QFileResourceFileEngineHandler::loadArchive(const QString &archive) const
{
    static int nextId = 0;

    QMap<QString, QString>::ConstIterator iter =
        m_registeredArchives.find(archive);
    if(iter != m_registeredArchives.end())
        return *iter;

    QString myId = QString("/Qtopia/%1").arg(nextId++);
    if(!QResource::registerResource(archive, myId))
        myId = QString();

    m_registeredArchives.insert(archive, myId);

    return myId;
}

QAbstractFileEngine *QFileResourceFileEngineHandler::findArchivedImage(const QString &_path) const
{
    QString path = _path;

    if(path.startsWith("i18n/"))
        path.remove(0, 5 /* ::strlen("i18n/") */);

    QString myApp = QApplication::applicationName();
    QString app;
    QString image;
    {
        int index = path.indexOf('/');
        if(index != -1) {
            app = path.left(index);
            image = path.mid(index + 1);
        } else {
            image = path;
        }
    }

    QList<QPair<QString, QString > > searchNames;
    if(app.isEmpty()) {
        searchNames.append(qMakePair(myApp + "/qtopia.rdb", image));
        searchNames.append(qMakePair(QString("qtopia.rdb"),
                                     myApp + "/" + image));
        searchNames.append(qMakePair(QString("qtopia.rdb"), image));
    } else {
        searchNames.append(qMakePair(myApp + "/qtopia.rdb", app + "/" + image));
        searchNames.append(qMakePair(QString("qtopia.rdb"),
                                     myApp + "/" + app + "/" + image));
        searchNames.append(qMakePair(app + "/qtopia.rdb", image));
        searchNames.append(qMakePair(QString("qtopia.rdb"), app + "/" + image));
    }

    foreach (QString searchBase, imagedirs) {
        typedef QPair<QString, QString> SearchName;
        foreach (SearchName searchName, searchNames) {
            QString id = loadArchive(searchBase + "/" + searchName.first);
            if(!id.isEmpty()) {
                QString resName = ":" + id + "/" + searchName.second;
                QResource resource(resName);
                if(resource.isValid()) {
                    qLog(Resource) << "Archived Image Resource " << _path << "->" << resName;
                    return new QResourceFileEngine(resName);
                }
            }
        }
    }

    return 0;
}

/*
     <app name>/qtopia.rdb/icons/<icon size>/<icon>
     qtopia.rdb/<app name>/icons/<icon size>/<icon>
 */
QAbstractFileEngine *QFileResourceFileEngineHandler::findArchivedIcon(const QString &_path) const
{
    QString path = _path;

    if(path.startsWith("i18n/"))
        path.remove(0, 5 /* ::strlen("i18n/") */);

    QString myApp = QApplication::applicationName();
    QString app;
    QString icon;
    {
        int index = path.indexOf('/');
        if(index != -1) {
            app = path.left(index);
            icon = path.mid(index + 1);
        } else {
            icon = path;
        }
    }

    QList<QPair<QString, QString > > searchNames;
    if(app.isEmpty()) {
        searchNames.append(qMakePair(myApp + "/qtopia.rdb",
                                     QString("icons/") + icon));
        searchNames.append(qMakePair(QString("qtopia.rdb"),
                                     myApp + "/icons/" + icon));
        searchNames.append(qMakePair(QString("qtopia.rdb"),
                                     QString("icons/") + icon));
    } else {
        searchNames.append(qMakePair(myApp + "/qtopia.rdb",
                                     app + "/icons/" + icon));
        searchNames.append(qMakePair(QString("qtopia.rdb"),
                                     myApp + "/" + app + "/icons/" + icon));
        searchNames.append(qMakePair(app + "/" + "qtopia.rdb",
                                     QString("icons/") + icon));
        searchNames.append(qMakePair(QString("qtopia.rdb"),
                                     app + "/icons/" + icon));
    }

    foreach (QString searchBase, imagedirs) {
        typedef QPair<QString, QString> SearchName;
        foreach (SearchName searchName, searchNames) {
            QString id = loadArchive(searchBase + "/" + searchName.first);
            if(!id.isEmpty()) {
                QString resName = ":" + id + "/" + searchName.second;
                QResource resource(resName);
                if(resource.isValid()) {
                    qLog(Resource) << "Archived Icon Resource " << _path << "->" << resName;
                    return new QResourceFileEngine(resName);
                }
            }
        }
    }

    return findArchivedImage(path);
}
#endif

QString QFileResourceFileEngineHandler::findDiskResourceFile(const QString &path) const
{
    // Caching makes sense.  We often look for the same small number of files.
    static QCache<QString,QString> fileCache(25);

    if (QString *cFile = fileCache.object(path))
        return *cFile;

    static const QString image(QLatin1String(":image/"));
    static const QString icon(QLatin1String(":icon/"));
    static const QString sound(QLatin1String(":sound/"));

    QString r;
    if (path.startsWith(image)) {

        QString p1 = path.mid(7 /* ::strlen(:image/") */);
        expandService(p1);
        r = findDiskImage(p1, QString());

    } else if (path.startsWith(icon)) {

        QString p1 = path.mid(6 /* ::strlen(":icon/") */);
        expandService(p1);
        static const QString icons(QLatin1String("icons/"));
        r = findDiskImage(p1, icons);
        if ( r.isEmpty() )
            r = findDiskImage(p1, QString());

    } else if (path.startsWith(sound)) {

        QString p1 = path.mid(7 /* ::strlen(":sound/") */);
        r = findDiskSound(p1);

    } else {

        qLog(Resource) << "Unsupported resource" << path;
    }

    fileCache.insert(path, new QString(r));

    return r;
}

/* _path does NOT include the ":image/" prefix; _subdir is either empty or MUST include trailing "/" */
QString QFileResourceFileEngineHandler::findDiskImage(const QString &_path, const QString& _subdir) const
{
    static QList<QByteArray> commonFormats;
    static QList<QByteArray> otherFormats;
    static const char sep = '/';
    static const QByteArray i18nDir("i18n/");

    if (!commonFormats.count()) {
        //XXX Only 3 letter extensions supported for common formats.
#ifndef QT_NO_PICTURE
        commonFormats.append("pic");
#endif
        commonFormats.append("svg");
        commonFormats.append("png");
        commonFormats.append("jpg");
        commonFormats.append("mng");

        // Get the rest of the formats Qt supports.
        QList<QByteArray> suppFormats = QImageReader::supportedImageFormats();
        foreach (QByteArray format, suppFormats) {
            if (!commonFormats.contains(format)) {
                otherFormats.append(format);
            }
        }
    }

    QByteArray path = _path.toLocal8Bit();
    QByteArray subDir = _subdir.toLocal8Bit();
    QList<QByteArray> searchNames; // List of names to search for

    bool i18n = false;
    if(path.startsWith(i18nDir)) {
        path.remove(0, 5 /* ::strlen("i18n/") */);
        i18n = true;
    }

    QByteArray myApp = QApplication::applicationName().toLocal8Bit() + sep;
    QByteArray app;
    QByteArray image;
    bool knownExtn = false;

    {
        int slash = path.indexOf(sep);
        if(slash != -1) {
            app = path.left(slash + 1);
            image = path.mid(slash + 1);
        } else {
            image = path;
        }
        int dot = image.lastIndexOf('.');
        if ( dot >= 0 ) {
            slash = image.lastIndexOf(sep);
            if (slash < 0 || dot > slash) {
                QByteArray img_extn = image.mid(dot+1);
                if (commonFormats.contains(img_extn)
                    || otherFormats.contains(img_extn)) {
                    knownExtn = true;
                }
            }
        }
    }

    if(i18n) {
        QStringList langs = Qtopia::languageList();
        langs.append(QLatin1String("en_US"));

        foreach(QString lang, langs) {
            if(app.isEmpty()) {
                searchNames.append(myApp + subDir + i18nDir + lang.toLatin1() + sep + image);
                searchNames.append(subDir + i18nDir + lang.toLatin1() + sep + image);
            } else {
                searchNames.append(myApp + app + subDir + i18nDir + lang.toLatin1() + sep + image);
                searchNames.append(app + subDir + i18nDir + lang.toLatin1() + sep + image);
            }
        }
    }

    QByteArray tmpStr;
    tmpStr.reserve(app.size() + subDir.size() + image.size());
    if (app.size() > 1)  // i.e. not just '/'
        tmpStr += app;
    tmpStr += subDir;
    tmpStr += image;
    searchNames.append(myApp + tmpStr);
    searchNames.append(tmpStr);

    foreach (QByteArray searchBase, imagedirs) {
        foreach (QByteArray searchName, searchNames) {
            QByteArray r;
            r.reserve(searchBase.size() + searchName.size() + 5); // +5 for extn below
            r += searchBase;
            r += searchName;
            if (!knownExtn) {
                r += "....";
                int ext = r.length()-3;
                // Try our common formats first.
                foreach (QByteArray extn, commonFormats) {
                    r[ext]=extn[0]; r[ext+1]=extn[1]; r[ext+2]=extn[2];
                    if (fileExists(r)) {
                        qLog(Resource) << extn << "Image Resource" << path << "->" << r;
                        return QString::fromLocal8Bit(r);
                    }
                }
                r.truncate(ext);
                // Then anything else Qt supports
                foreach (QByteArray extn, otherFormats) {
                    QByteArray fn = r + extn;
                    if (fileExists(fn)) {
                        qLog(Resource) << extn << "Image Resource" << path << "->" << fn;
                        return QString::fromLocal8Bit(fn);
                    }
                }
            } else {
                // File has an extension we know
                if (fileExists(r)) {
                    qLog(Resource) << "Found Image Resource" << path << "->" << r;
                    return QString::fromLocal8Bit(r);
                }
            }
        }
    }

    qLog(Resource) << "No resource" << path;
    qLog(Resource) << "  Tried directories:" << imagedirs;
    qLog(Resource) << "  Tried files:" << searchNames;

    return QString();
}

/*!
  \internal
 */
QString QFileResourceFileEngineHandler::findDiskSound(const QString &path) const
{
    QByteArray myApp = QApplication::applicationName().toLatin1();
    QByteArray p1 = path.toLocal8Bit();
    p1 += ".wav";
    foreach (QByteArray s, sounddirs) {
        QByteArray r = s + "/" + myApp + "/" + p1; 
        if (fileExists(r)) {
            qLog(Resource) << "WAV Sound Resource" << path << "->" << r;
            return QString::fromLocal8Bit(r);
        }

        r = s + p1;
        if (fileExists(r)) {
            qLog(Resource) << "WAV Sound Resource" << path << "->" << r;
            return QString::fromLocal8Bit(r);
        }
    }

    qLog(Resource) << "No resource" << path;

    return QString();
}

#ifdef ENABLE_RESOURCEFILEENGINE
// declare QRawImageIOPlugin
class QRawImageIOPlugin : public QImageIOPlugin
{
public:
    virtual Capabilities capabilities(QIODevice *device,
                                      const QByteArray &format) const;
    virtual QStringList keys() const;
    virtual QImageIOHandler *create(QIODevice *device,
                                    const QByteArray &format) const;
};

// declare QRawImageIOHandler
class QRawImageIOHandler : public QImageIOHandler
{
public:
    QRawImageIOHandler(QIODevice *);

    virtual bool canRead() const;
    virtual bool read(QImage * image);
private:
    struct RawImage {
        const char qraw[4];
        int format;
        int width;
        int height;
        const uchar data[0];
    };
    const RawImage * m_data;
};

// device QRawImageIOHandler
QRawImageIOHandler::QRawImageIOHandler(QIODevice *device)
: m_data(0)
{
    // We only support QRAW images in resource archives
    QFile *file = qobject_cast<QFile *>(device);
    if(!file) return;

    QResource resource(file->fileName());
    if(!resource.isValid()) return;

    m_data = reinterpret_cast<const RawImage *>(resource.data());
}

bool QRawImageIOHandler::canRead() const
{
    return (m_data != 0);
}

bool QRawImageIOHandler::read(QImage * image)
{
    Q_ASSERT(image);
    if(!m_data)
        return false;
    *image = QImage(m_data->data, m_data->width,
                    m_data->height, (QImage::Format)m_data->format);
    return true;
}

// define QRawImageIOPlugin
QObject * qt_plugin_instance_qraw_image()
{
    return new QRawImageIOPlugin();
}
Q_IMPORT_PLUGIN(qraw_image);

QRawImageIOPlugin::Capabilities
QRawImageIOPlugin::capabilities(QIODevice *device,
                                const QByteArray &format) const
{
    if(!device) return 0;

    device->seek(0);
    QByteArray type = device->read(4 /* ::strlen("QRAW") */);

    return ("QRAW" == type)?CanRead:(QRawImageIOPlugin::Capabilities)(0);
}

QStringList QRawImageIOPlugin::keys() const
{
    QStringList rv;
    rv << "qraw";
    return rv;
}

QImageIOHandler *QRawImageIOPlugin::create(QIODevice *device,
                                           const QByteArray &format) const
{
    device->seek(4);
    return new QRawImageIOHandler(device);
}
#endif
