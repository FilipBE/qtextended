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

#define   INITGUID     /* Interface ID's */

#include <string.h>    /* strcpy */

#include "hxver.h"
#include "hxtypes.h"
#include "hxcom.h"     /* IUnknown */
#include "hxcomm.h"   /* IHXCommonClassFactory */
#include "ihxpckts.h"  /* IHXValues */

#include "qtopiafilesystem.h"
#include "qtopiafileobject.h"

/****************************************************************************
 *  HXCreateInstance                                        ref:  hxplugn.h
 *
 *  This routine creates a new instance of the QtopiaFileSystem class.
 *  It is called when the Helix core application is launched, and whenever
 *  an URL with a protocol associated with this plug-in is opened.
 */
STDAPI
HXCreateInstance(IUnknown** ppExFileSystemObj)
{
	*ppExFileSystemObj = (IUnknown*)(IHXPlugin*)new QtopiaFileSystem();
	if (*ppExFileSystemObj != NULL)
	{
		(*ppExFileSystemObj)->AddRef();
		return HXR_OK;
	}

	return HXR_OUTOFMEMORY;
}

/****************************************************************************
 *  QtopiaFileSystem static variables
 *
 *  These variables are passed to the Helix core to provide information about
 *  this plug-in. They are required to be static in order to remain valid
 *  for the lifetime of the plug-in.
 */
const char* QtopiaFileSystem::zm_pDescription = "Qtopia File System";
const char* QtopiaFileSystem::zm_pCopyright   = "Copyright (C) 2000-2006 Trolltech ASA.  All rights reserved.";
const char* QtopiaFileSystem::zm_pMoreInfoURL = "http://www.trolltech.com";
const char* QtopiaFileSystem::zm_pShortName   = "pn-qtopia";
const char* QtopiaFileSystem::zm_pProtocol    = "qtopia";

/****************************************************************************
 *  QtopiaFileSystem::QtopiaFileSystem
 *
 *  Constructor
 */
QtopiaFileSystem::QtopiaFileSystem(void)
	: m_RefCount      (0),
	  m_pClassFactory (NULL)
{
	m_BasePath[ 0 ] = '\0';
}

/****************************************************************************
 *  QtopiaFileSystem::~QtopiaFileSystem
 *
 *  Destructor. Be sure to release all outstanding references to objects.
 */
QtopiaFileSystem::~QtopiaFileSystem(void)
{
	HX_RELEASE(m_pClassFactory);
}

// IHXFileSystemObject Interface Methods

/****************************************************************************
 *  IHXFileSystemObject::GetFileSystemInfo
 *
 *  This routine returns crucial information required to associate this
 *  plug-in with a given protocol. This information tells the core which
 *  File System plug-in to use for a particular protocol. For example, in the
 *  URL: "file://myfile.txt", the protocol would be "file". This routine is
 *  called when the Helix core application is launched.
 */
STDMETHODIMP
QtopiaFileSystem::GetFileSystemInfo
(
	REF(const char*) pShortName,
	REF(const char*) pProtocol
)
{
	pShortName = zm_pShortName;
	pProtocol  = zm_pProtocol;

	return HXR_OK;
}

/****************************************************************************
 *  IHXFileSystemObject::InitFileSystem
 *
 *  This routine performs any additional initialization steps required for
 *  the file system.  It is called prior to the CreatFile() request. Any
 *  options provided usually refer to mounting options related to the server,
 *  such as base path or authentication preferences.
 */
STDMETHODIMP
QtopiaFileSystem::InitFileSystem(IHXValues*  options )
{
	// Retrieve the platform's base path, if specified
	if (options != NULL )
	{
		IHXBuffer* pPathBuffer = NULL;
		if (options->GetPropertyBuffer("BasePath", pPathBuffer) == HXR_OK)
		{
			if (pPathBuffer->GetBuffer() != NULL)
			{
				strcpy(m_BasePath, (char*)pPathBuffer->GetBuffer());
			}
			pPathBuffer->Release();
		}
	}

	return HXR_OK;
}

/****************************************************************************
 *  IHXFileSystemObject::CreateFile
 *
 *  This routine creates a new File Object which handles all of the file I/O
 *  functionality of this class. This File Object is eventually handed off
 *  to a File Format plug-in which handles file I/O through this File Object.
 *  This method is called called when an URL with a protocol associated with
 *  this plug-in is opened.
 */
STDMETHODIMP
QtopiaFileSystem::CreateFile(IUnknown** ppFileObject)
{
	QtopiaFileObject* pFileObj;

	// Create a new File Object which implements the file I/O methods
	pFileObj = new QtopiaFileObject(m_pClassFactory, m_BasePath);
	if (pFileObj != NULL)
	{	/*
		 * Pass the File Object off to the Helix core, and eventually to the
		 * associated File Format plug-in.
		 */
		pFileObj->QueryInterface(IID_IUnknown, (void**)ppFileObject);
		if (pFileObj != NULL)
		{
			return HXR_OK;
		}
		return HXR_UNEXPECTED;
		/*
		 * Note that the Helix core obtains ownership of this File Object and
		 * is therefore responsible for releasing it.
		 */
	}

	return HXR_OUTOFMEMORY;
}

/****************************************************************************
 *  IHXFileSystemObject::CreateDir
 *
 *  This routine is analagous to CreatFile, except directories instead of
 *  files are of concern. It is not implemented in this example.
 */
STDMETHODIMP
QtopiaFileSystem::CreateDir(IUnknown** /* ppDirectoryObject */)
{
	return HXR_NOTIMPL;
}

// IHXPlugin Interface Methods

/****************************************************************************
 *  IHXPlugin::GetPluginInfo                                ref:  hxplugn.h
 *
 *  This routine returns descriptive information about the plug-in, most
 *  of which is used in the About box of the user interface. It is called
 *  when the Helix core application is launched.
 */
STDMETHODIMP
QtopiaFileSystem::GetPluginInfo
(
	REF(BOOL)        bLoadMultiple,
	REF(const char*) pDescription,
	REF(const char*) pCopyright,
	REF(const char*) pMoreInfoURL,
	REF(UINT32)      versionNumber
)
{
	bLoadMultiple = TRUE;
	pDescription  = zm_pDescription;
	pCopyright    = zm_pCopyright;
	pMoreInfoURL  = zm_pMoreInfoURL;
	versionNumber = 0;

	return HXR_OK;
}

/****************************************************************************
 *  IHXPlugin::InitPlugin                                   ref:  hxplugn.h
 *
 *  This routine performs initialization steps such as determining if
 *  required interfaces are available. It is called when the Helix core
 *  application is launched, and whenever an URL with a protocol associated
 *  with this plug-in is opened.
 */
STDMETHODIMP
QtopiaFileSystem::InitPlugin(IUnknown* pHXCore)
{
	/*
	 * Store a reference to the IHXCommonClassFactory interface which is
	 * used to create commonly used Helix objects such as IHXPacket,
	 * IHXValues, and IHXBuffers.
	 */
	if (pHXCore->QueryInterface(IID_IHXCommonClassFactory,
                                           (void**)&m_pClassFactory) != HXR_OK)
	{
		return HXR_NOINTERFACE;
	}

	/*
	 * Note that QueryInterface() takes care of adding a reference to the
	 * interface for us. You are however responsible for releasing the
	 * reference to the interface when you are done using it by calling
	 * the Release() routine.
	 */

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
QtopiaFileSystem::AddRef(void)
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
QtopiaFileSystem::Release(void)
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
QtopiaFileSystem::QueryInterface
(
	REFIID interfaceID,
	void** ppInterfaceObj
)
{
	// By definition all COM objects support the IUnknown interface
	if (IsEqualIID(interfaceID, IID_IUnknown))
	{
		AddRef();
		*ppInterfaceObj = (IUnknown*)(IHXPlugin*)this;
		return HXR_OK;
	}

	// IHXPlugin interface is supported
	else if (IsEqualIID(interfaceID, IID_IHXPlugin))
	{
		AddRef();
		*ppInterfaceObj = (IHXPlugin*)this;
		return HXR_OK;
	}

	// IHXFileSystemObject interface is supported
	else if (IsEqualIID(interfaceID, IID_IHXFileSystemObject))
	{
		AddRef();
		*ppInterfaceObj = (IHXFileSystemObject*)this;
		return HXR_OK;
	}

	// No other interfaces are supported
	*ppInterfaceObj = NULL;
	return HXR_NOINTERFACE;
}
