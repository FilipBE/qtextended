/**********************************************************************
** Author: Qt Extended
**
** Licensees holding a valid license agreement for the use of the
** Helix DNA code base may use this file in accordance with that license.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@qtextended.org if any conditions of this licensing are
** not clear to you.
**
**/

/****************************************************************************
 *
 *  This is an example of a File Object which is used in conjunction with the
 *  File System plug-in to provide low level access to files. The File System
 *  plug-in passes this object to the Helix core, which eventually hands it off
 *  to the appropriate File Format plug-in. This File Format plug-in is what
 *  actually calls the file access methods defined by this object. Thus, this
 *  File Object acts as an abstract interface which File Format objects use
 *  to access the contents of a file.
 *
 *  In order the File Object to achieve the proper functionality required by
 *  a File System plug-in, several interfaces must be implemented. The
 *  IHXFileObject interface handles all of the file access routines and is
 *  the core of this example. In order to obtain the requested URL, the
 *  IHXRequestHandler is also needed. To determine if the file actually
 *  exists, the IHXFileExists interface is also implemented.
 *
 *  This example makes use of the standard C file access routines such as
 *  fread, fwrite, and fseek to handle file access with a file on a local
 *  file system such as your hard drive.
 */

#include "hlxclib/string.h"   /* strcpy, etc. */

#include "hxtypes.h"
#include "hxcom.h"     /* IUnknown */
#include "hxcomm.h"   /* IHXCommonClassFactory */
#include "ihxpckts.h"  /* IHXValues, IHXBuffers */

#include "qtopiafileobject.h"

#include <QtCore/qstring.h>
#include <qcontent.h>

/****************************************************************************
 *  QtopiaFileObject::QtopiaFileObject
 *
 *  Constructor
 */
QtopiaFileObject::QtopiaFileObject
	(IHXCommonClassFactory* pClassFactory, char* pBasePath)
	: m_RefCount       (0),
	  m_pClassFactory  (pClassFactory),
	  m_pFileResponse  (NULL),
	  m_pFile          (NULL),
	  m_pFilename      (NULL),
	  m_pRequest       (NULL),
	  m_pBasePath      (NULL)
{
	// Signify that we need to keep a reference to this object
	if (m_pClassFactory != NULL)
	{
		m_pClassFactory->AddRef();
	}

    if (pBasePath)
    {
        m_pBasePath = new char[strlen(pBasePath) + 1];

        if (m_pBasePath)
        {
            strcpy(m_pBasePath, pBasePath);
        }
    }
}

/****************************************************************************
 *  QtopiaFileObject::~QtopiaFileObject
 *
 *  Destructor. It is essential to call the Close() routine before destroying
 *  this object.
 */
QtopiaFileObject::~QtopiaFileObject(void)
{
    delete [] m_pBasePath;
    m_pBasePath = 0;

	Close();
}

/****************************************************************************
 *  QtopiaFileObject::OpenFile                         
 *
 *  This routine opens a file according to the access mode given. It is
 *  called while initializing the File Object.
 */
STDMETHODIMP
QtopiaFileObject::OpenFile(UINT32 fileAccessMode)
{
	HX_RESULT   result    = HXR_OK;

	// Only use HX_FILE_READ
	if( fileAccessMode & HX_FILE_READ ) {
		m_pFile = new QFile( m_pFilename );
		if( !m_pFile->open( QIODevice::ReadOnly ) ) {
			result = HXR_FAILED;
		}
	} else {
		result = HXR_INVALID_PARAMETER;
	}

	return result;
}

/****************************************************************************
 *  QtopiaFileObject::ConvertToPlatformPath            
 *
 *  This routine converts the given file path to a platform specific file
 *  path based upon the naming conventions of that platform. The platform
 *  specific path name is required to properly open the file.
 */
STDMETHODIMP
QtopiaFileObject::ConvertToPlatformPath
(
	REF(char*)  pFilePathPlatform,
	const char* pFilePath
)
{
    HX_RESULT res = HXR_OUTOFMEMORY;

    pFilePathPlatform =  0;

    if (m_pBasePath && pFilePath)
    {
        // Create new string
        pFilePathPlatform =
            new char[ strlen(m_pBasePath) + strlen(pFilePath) + 2 ];
    }


	static const char* protocol = "qtopia";
    UINT32 length = strlen(protocol) + 1; // Add 1 for the colon
    char* pProtocolString = new char[length + 1];

    if (pFilePathPlatform && pProtocolString && m_pBasePath)
    {
        // Prepend base path, if any
        if (strlen(m_pBasePath) > 0)
        {
            strcpy(pFilePathPlatform, m_pBasePath);
            strcat(pFilePathPlatform, OS_PATH_SEPARATOR_STR);
            strcat(pFilePathPlatform, pFilePath);
        }
        else
        {
            strcpy(pFilePathPlatform, pFilePath);
        }

        // Strip protocol string, if any
        strcpy(pProtocolString, protocol);
        strcat(pProtocolString, ":");
        if (strnicmp(pFilePathPlatform, pProtocolString, length) == 0)
        {
            //copy the rest of the string back onto itself.
            memmove( (void*) pFilePathPlatform,
                     (void*) &pFilePathPlatform[length],
                     (strlen( &pFilePathPlatform[length] )+1)*sizeof(char)
                     );

            if ((pFilePathPlatform[0] == '/') &&
                (pFilePathPlatform[1] == '/'))
            {
                // "qtopia://" forms

                // Find next '/'
                const char* pNext = strchr(pFilePathPlatform + 2, '/');

                if (pNext)
                {
                    // "qtopia://host/path" or "qtopia:///path" form.
                    // everything after the second '/'
                    memmove( (void*) pFilePathPlatform,
                             (void*) pNext,
                             (strlen(pNext)+1)*sizeof(char)
                             );
                    pNext = 0;
                    res = HXR_OK;
                }
                else
                {
                    // Forms: qtopia://c:\file.ra
                    //        qtopia://file.ra
                    memmove( (void*) pFilePathPlatform,
                             (void*) (pFilePathPlatform+2),
                             (strlen(pFilePathPlatform+2)+1)*sizeof(char)
                             );
                    res = HXR_OK;
                }
            }
            else
            {
                res = HXR_OK;
            }

            if (HXR_OK == res)
            {
                // Replace path slashes with platform specific path separators
                // and watch for the parameter delimiter
                char* pCur = pFilePathPlatform;
                for (; *pCur && (*pCur != '?'); pCur++)
                {
                    if (*pCur == '/')
                    {
                        *pCur = OS_PATH_SEPARATOR_CHAR;
                    }
                }

                /*
                 * Strip off the parameters
                 */
                if (*pCur == '?')
                {
                    *pCur = '\0';
                }
            }
        }
        else
        {
			if (NULL == strstr(pFilePathPlatform,"//"))
				res = HXR_OK; // allow path/file w/o file://
			else
	            res = HXR_INVALID_PROTOCOL;
        }
    }

    delete [] pProtocolString;
    pProtocolString = 0;

    if (res != HXR_OK)
    {
        delete [] pFilePathPlatform;
        pFilePathPlatform = 0;
    }

    return res;
}

// IHXFileObject Interface Methods

/****************************************************************************
 *  IHXFileObject::Init                                     ref:  hxfiles.h
 *
 *  This routine associates this File Object with a File Response object
 *  which is notified when file operations (read, write, seek, etc.) are
 *  complete. This method also checks the validity of the file by actually
 *  opening it.
 */
STDMETHODIMP
QtopiaFileObject::Init
(
	UINT32            fileAccessMode,
	IHXFileResponse* pFileResponse
)
{	/*
	 * Associate this File Object with a File Response object for completion
	 * notification.
	 */
	if (pFileResponse != NULL)
	{
		// Release any previous File Response objects
		HX_RELEASE(m_pFileResponse);
		m_pFileResponse = pFileResponse;
		m_pFileResponse->AddRef();
	}
	else
	{
		return HXR_INVALID_PARAMETER;
	}

	/*
	 * Open the file and notify File Response when complete
	 */
	if (m_pFile != NULL) // File is already open
	{
		// Only use HX_FILE_READ
		if ( fileAccessMode & HX_FILE_READ )
		{
		    // reset to start of file
			m_pFile->reset();

		    // notify that file is ready
		    m_pFileResponse->InitDone(HXR_OK);
		    return HXR_OK;
	    }
	    else // Access mode has changed
	    {
			delete m_pFile;
			m_pFile = NULL;
	    }
	}
	
	HX_RESULT fileOpenResult = OpenFile(fileAccessMode);
	m_pFileResponse->InitDone(fileOpenResult);

	return fileOpenResult;
}

/****************************************************************************
 *  IHXFileObject::GetFilename                              ref:  hxfiles.h
 *
 *  This routine returns the name of the requested file (without any path
 *  information). This method may be called by the File Format plug-in if the
 *  short name of the file is required.
 */
STDMETHODIMP
QtopiaFileObject::GetFilename(REF(const char*) pFileName)
{
	pFileName = NULL;
	HX_RESULT   result = HXR_OK;

	// Find the separator character before the file name
	pFileName = ::strrchr(m_pFilename, OS_PATH_SEPARATOR_CHAR);

	if (pFileName != NULL) // Found
	{
		// File name starts after the separator charactor
		pFileName++;
	}
	else // Not found
	{
		pFileName = m_pFilename;
	}

	return result;
}

/****************************************************************************
 *  IHXFileObject::Read                                     ref:  hxfiles.h
 *
 *  This routine reads a block of data of the specified length from the file.
 *  When reading has completed, the caller is asynchronously notified via the
 *  File Response object associated with this File Object. This method is 
 *  called by the File Format plug-in when it needs to read from the file.
 */
STDMETHODIMP
QtopiaFileObject::Read(UINT32 byteCount)
{
	HX_RESULT result = HXR_OK;

	// Create a buffer object to store the data which is to be read
	IHXBuffer* pBuffer;
	m_pClassFactory->CreateInstance(CLSID_IHXBuffer, (void**)&pBuffer);
	if (pBuffer != NULL)
	{
		result =  pBuffer->SetSize(byteCount);
		if (result != HXR_OK)
		{
			return result;
		}
		
		// Read from the file directly into the buffer object
		UINT32 actualCount = m_pFile->read( (char*)pBuffer->GetBuffer(), byteCount );
		pBuffer->SetSize(actualCount);

		// Notify the caller that the read is done
		HX_RESULT readResult = actualCount > 0 ? HXR_OK : HXR_FAILED;
		m_pFileResponse->ReadDone(readResult, pBuffer);
		
		// Release the buffer object since we are done with it
		pBuffer->Release();
	}
	else
	{
		result = HXR_OUTOFMEMORY;
	}
	
	return result;
}

/****************************************************************************
 *  IHXFileObject::Write                                    ref:  hxfiles.h
 *
 *  This routine writes a block of data to the file. When writing has
 *  completed, the caller is asynchronously notified via the File Response
 *  object associated with this File Object. This method called by the File
 *  Format plug-in when it needs to write to the file.
 */
STDMETHODIMP
QtopiaFileObject::Write(IHXBuffer* pDataToWrite)
{
	return HXR_NOTIMPL;
}

/****************************************************************************
 *  IHXFileObject::Seek                                    ref:  hxfiles.h
 *
 *  This routine moves to a given position in the file. The move can be
 *  either from the beginning of the file (absolute), or relative to the
 *  current file position. When seeking has completed, the caller is
 *  asynchronously notified via the File Response object associated with this
 *  File Object. This method called by the File Format plug-in when it needs
 *  to seek to a location within the file.
 */
STDMETHODIMP
QtopiaFileObject::Seek
(
	UINT32 offset,
	BOOL   bIsRelative
)
{
	HX_RESULT result = HXR_OK;

	qint64 pos = offset;

	if( bIsRelative ) {
		pos += m_pFile->pos();
	}

	if( !m_pFile->seek( pos ) ) {
		result = HXR_FAILED;
	}

	m_pFileResponse->SeekDone( result );

	return result;
}

/****************************************************************************
 *  IHXFileObject::Advise                                   ref:  hxfiles.h
 *
 *  This routine is passed information about the intended usage of this
 *  object. The useage will indicate whether sequential or random requests
 *  for information will be made. This may be useful, for example, in
 *  developing a caching scheme.
 */
STDMETHODIMP
QtopiaFileObject::Advise(UINT32 /* useage */)
{
	return HXR_UNEXPECTED;
}

/****************************************************************************
 *  IHXFileObject::Close                                    ref:  hxfiles.h
 *
 *  This routine closes the file resource and releases all resources
 *  associated with the object. This routine is crucial and must be called
 *  before the File Object is destroyed.
 */
STDMETHODIMP
QtopiaFileObject::Close(void)
{
	if (m_pFile != NULL) 
	{
		delete m_pFile;
		m_pFile = NULL;
	}

	HX_RELEASE(m_pClassFactory);
	HX_RELEASE(m_pRequest);
	HX_VECTOR_DELETE(m_pFilename);

	/*
	 * Store this in temp so that if calling CloseDone
	 * causes our descructor to get called we will
	 * have pCallCloseDone on the stack to safely release.
	 */
	IHXFileResponse* pCallCloseDone = m_pFileResponse;
	if (m_pFileResponse != NULL)
	{
	    m_pFileResponse = NULL;
	    pCallCloseDone->CloseDone(HXR_OK);
	    pCallCloseDone->Release();
	}

	return HXR_OK;
}

// IHXRequestHandler Interface Methods

/****************************************************************************
 *  IHXRequestHandler::SetRequest                           ref:  hxfiles.h
 *
 *  This routine associates this File Object with the file Request object
 *  passed to it from the Helix core. This Request object is primarily used to
 *  obtain the requested URL. This method is called just after the File
 *  Object is created.
 */
STDMETHODIMP
QtopiaFileObject::SetRequest(IHXRequest* pRequest)
{
	// Release any previous request objects
	HX_RELEASE(m_pRequest);
    
    // Store a reference to the object
	m_pRequest = pRequest;
	m_pRequest->AddRef();

	const char* pURL;

	if (m_pRequest->GetURL(pURL) != HXR_OK)
	{
	    return HXR_FAIL;
	}

	if (pURL)
	{
	    char* pPtr;

	    delete[] m_pFilename;

	    ConvertToPlatformPath(m_pFilename, pURL);

	    /*
	     * Strip off the parameters
	     */

	    pPtr = ::strchr(m_pFilename, '?');

	    if (pPtr)
	    {
		    *pPtr = '\0';
	    }
	}

	return HXR_OK;
}

/****************************************************************************
 *  IHXRequestHandler::GetRequest                           ref:  hxfiles.h
 *
 *  This routine retrieves the Request object associated with this File
 *  Object. It is called just after the SetRequest() method.
 */
STDMETHODIMP
QtopiaFileObject::GetRequest(REF(IHXRequest*) pRequest)
{
	pRequest = m_pRequest;
	if (pRequest != NULL)
	{
		pRequest->AddRef();
	}

	return HXR_OK;
}

/****************************************************************************
 *  IHXFileExists::DoesExist                                ref:  hxfiles.h
 *
 *  This routine determines if the given file exists, and notifies the File
 *  Response object. It is called by the Helix server after the File Object has
 *  been created to determine if the requested file does actually exist. If
 *  it does the File Object is handed off to the File Format object.
 */
STDMETHODIMP
QtopiaFileObject::DoesExist
(
	const char*             pFilePath,
	IHXFileExistsResponse* pFileResponse
)
{
	BOOL  bDoesExist = FALSE;
	char* pFilePathPlatform   = NULL;

	// Convert file path to platform specific file path
	HX_RESULT result = ConvertToPlatformPath(pFilePathPlatform,pFilePath);

	// Determine if the file can be opened
	if (result == HXR_OK)
	{
		if ( QFile::exists( pFilePathPlatform ) )
		{
			bDoesExist = TRUE;
		}
	}

	// Notify the caller if the file exists
	pFileResponse->DoesExistDone(bDoesExist);

	if (pFilePathPlatform != NULL)
	{
		delete[] pFilePathPlatform;
	}

	return HXR_OK;
}

// IUnknown COM Interface Methods

/****************************************************************************
 *  IUnknown::AddRef                                            ref:  hxcom.h
 *
 *  This routine increases the object reference count in a thread safe
 *  manner. The reference count is used to manage the lifetime of an object.
 *  This method must be explicitly called by the user whenever a new
 *  reference to an object is used.
 */
STDMETHODIMP_(UINT32)
QtopiaFileObject::AddRef(void)
{
	return InterlockedIncrement(&m_RefCount);
}

/****************************************************************************
 *  IUnknown::Release                                           ref:  hxcom.h
 *
 *  This routine decreases the object reference count in a thread safe
 *  manner, and deletes the object if no more references to it exist. It must
 *  be called explicitly by the user whenever an object is no longer needed.
 */
STDMETHODIMP_(UINT32)
QtopiaFileObject::Release(void)
{
	if (InterlockedDecrement(&m_RefCount) > 0)
	{
		return m_RefCount;
	}

	delete this;
	return 0;
}

/****************************************************************************
 *  IUnknown::QueryInterface                                    ref:  hxcom.h
 *
 *  This routine indicates which interfaces this object supports. If a given
 *  interface is supported, the object's reference count is incremented, and
 *  a reference to that interface is returned. Otherwise a NULL object and
 *  error code are returned. This method is called by other objects to
 *  discover the functionality of this object.
 */
STDMETHODIMP
QtopiaFileObject::QueryInterface
(
	REFIID interfaceID,
	void** ppInterfaceObj
)
{
	// By definition all COM objects support the IUnknown interface
	if (IsEqualIID(interfaceID, IID_IUnknown))
	{
		AddRef();
		*ppInterfaceObj = (IUnknown*)(IHXFileObject*)this;
		return HXR_OK;
	}

	// IHXFileObject interface is supported
	else if (IsEqualIID(interfaceID, IID_IHXFileObject))
	{
		AddRef();
		*ppInterfaceObj = (IHXFileObject*)this;
		return HXR_OK;
	}

	// IHXDirHandler interface is supported
	else if(IsEqualIID(interfaceID, IID_IHXDirHandler))
	{
		AddRef();
		*ppInterfaceObj = (IHXDirHandler*)this;
		return HXR_OK;
	}
	// IHXRequestHandler interface is supported
	else if (IsEqualIID(interfaceID, IID_IHXRequestHandler))
	{
		AddRef();
		*ppInterfaceObj = (IHXRequestHandler*)this;
		return HXR_OK;
	}

	// IHXFileExists interface is supported
	else if (IsEqualIID(interfaceID, IID_IHXFileExists))
	{
		AddRef();
		*ppInterfaceObj = (IHXFileExists*)this;
		return HXR_OK;
	}
	else if (IsEqualIID(interfaceID, IID_IHXFileStat))
	{
		AddRef();
		*ppInterfaceObj = (IHXFileStat*)this;
		return HXR_OK;
	}
	else if (IsEqualIID(interfaceID, IID_IHXGetFileFromSamePool))
	{
	    AddRef();
	    *ppInterfaceObj = (IHXGetFileFromSamePool*)this;
	    return HXR_OK;
	}
    else if (IsEqualIID(interfaceID, IID_IHXFileMimeMapper))
    {
		AddRef();
		*ppInterfaceObj = (IHXFileMimeMapper*)this;
		return HXR_OK;
    }

	// No other interfaces are supported
	*ppInterfaceObj = NULL;
	return HXR_NOINTERFACE;
}

/************************************************************************
 * Method:
 *	IHXFileObject::Stat
 * Purpose:
 *	Collects information about the file that is returned to the
 *	caller in an IHXStat object
 */
STDMETHODIMP 
QtopiaFileObject::Stat(IHXFileStatResponse* pFileStatResponse)
{
    HX_RESULT result = HXR_OK;

    if( m_pFile )
    {
        pFileStatResponse->StatDone(HXR_OK,
                                    m_pFile->size(),
                                    0,
                                    0,
                                    0,
                                    0);
    }
    else
    {
        result = HXR_FAIL;
        pFileStatResponse->StatDone(HXR_FAIL, 0, 0, 0, 0, 0);
    }

    return result;
}

/************************************************************************
 *	Method:
 *	    IHXFileObject::GetFileObjectFromPool
 *	Purpose:
 *      To get another FileObject from the same pool. 
 */
STDMETHODIMP 
QtopiaFileObject::GetFileObjectFromPool (
    IHXGetFileFromSamePoolResponse* response
)
{
    HX_RESULT lReturnVal = HXR_OUTOFMEMORY;
    QtopiaFileObject* pFileObject = 0;
    IUnknown* pUnknown = 0;

	if( m_pFilename )
	{
		delete[] m_pBasePath;
		m_pBasePath = new char[strlen(m_pFilename) + 1];

		if(m_pBasePath)
		{
	    	// Chop off the current path to create a base path.
    		strcpy(m_pBasePath,m_pFilename);
    		char* pLastSeparator = strrchr(m_pBasePath,OS_PATH_SEPARATOR_CHAR);
    		*pLastSeparator = '\0';

    		pFileObject = new QtopiaFileObject(m_pClassFactory,m_pBasePath);

    		if (!pFileObject)
    		{
	    		return HXR_OUTOFMEMORY;
    		}

    		lReturnVal = pFileObject->QueryInterface(IID_IUnknown, (void**)&pUnknown);
    
    		response->FileObjectReady(lReturnVal == HXR_OK ? HXR_OK : HXR_FAILED, pUnknown);
    		HX_RELEASE(pUnknown);
		}
	}

    return lReturnVal;
}

/************************************************************************
 *	Method:
 *	    IHXFileMimeMapper::FindMimeType
 *	Purpose:
 */
STDMETHODIMP
QtopiaFileObject::FindMimeType
(
    const char*		    /*IN*/  pURL, 
    IHXFileMimeMapperResponse* /*IN*/  pMimeMapperResponse
)
{
    HX_RESULT result = HXR_OK;

	// Strip protocol
	QString file( pURL );
	if( file.startsWith( "qtopia:" ) ) {
        // URI with qtopia scheme

        file = file.mid(7);

        // handle empty authority
        if (file.startsWith("///"))
            file = file.mid(2);
	}

	pMimeMapperResponse->AddRef();
	QContent content( file );
	if( content.isValid() ) {
		QString type = content.type();
		result = pMimeMapperResponse->MimeTypeFound( type.isNull() ? HXR_FAILED : HXR_OK, type.toLatin1().data() );
	} else {
		result = pMimeMapperResponse->MimeTypeFound( HXR_FAILED, NULL );
	}
	pMimeMapperResponse->Release();

    return result;
}
