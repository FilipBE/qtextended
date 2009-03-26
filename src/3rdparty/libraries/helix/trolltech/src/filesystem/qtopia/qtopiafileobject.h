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

#ifndef QTOPIAFILEOBJECT_H
#define QTOPIAFILEOBJECT_H

#include <QtCore/qfile.h>

#include "hxfiles.h"  /* IHXFileObject, IHXRequestHandler, etc. */

#define OS_PATH_SEPARATOR_CHAR '/'
#define OS_PATH_SEPARATOR_STR  "/"

/****************************************************************************
 *
 *  QtopiaFileObject Class
 *
 *  This class inherits the interfaces required to create a File Object,
 *  which is used by the File System plug-in to handle file I/O. This class
 *  implements the IHXFileObject interface which handles the actual low
 *  level file access. The IHXRequestHandler interface is used to obtain
 *  the requested URL; while the IHXFileExists interface determines if the
 *  requested file actually exists. Since we are using COM, this class also
 *  inherits COM's IUnknown interface to handle reference counting and
 *  interface query.
 */
class QtopiaFileObject :  public IHXFileObject,
                              public IHXRequestHandler,
                              public IHXFileExists,
			      public IHXFileStat,
			      public IHXGetFileFromSamePool,
			      public IHXFileMimeMapper
{
	public:

	QtopiaFileObject
		(
		IHXCommonClassFactory* pClassFactory,
		char*                   pBasePath
		);
	~QtopiaFileObject(void);


	/************************************************************************
	 *  IHXFileObject Interface Methods                     ref:  hxfiles.h
	 */
	STDMETHOD(Init       ) (THIS_ UINT32 access,IHXFileResponse* pFileResp);
	STDMETHOD(GetFilename) (THIS_ REF(const char*) pFileName);
	STDMETHOD(Read       ) (THIS_ UINT32 byteCount);
	STDMETHOD(Write      ) (THIS_ IHXBuffer* pDataToWrite);
	STDMETHOD(Seek       ) (THIS_ UINT32 offset, BOOL bRelative);
	STDMETHOD(Advise     ) (THIS_ UINT32 useage);
	STDMETHOD(Close      ) (THIS);


	/************************************************************************
	 *  IHXRequestHandler Interface Methods                 ref:  hxfiles.h
	 */
	STDMETHOD(SetRequest) (THIS_     IHXRequest*  pRequest);
	STDMETHOD(GetRequest) (THIS_ REF(IHXRequest*) pRequest);


	/************************************************************************
	 *  IHXFileExists Interface Methods                     ref:  hxfiles.h
	 */
	STDMETHOD(DoesExist)
		(THIS_ 
		 const char*             pFilePath,
		 IHXFileExistsResponse* pFileResponse
		);


	/************************************************************************
	 *  IUnknown COM Interface Methods                          ref:  hxcom.h
	 */
	STDMETHOD (QueryInterface ) (THIS_ REFIID ID, void** ppInterfaceObj);
	STDMETHOD_(UINT32, AddRef ) (THIS);
	STDMETHOD_(UINT32, Release) (THIS);

	/************************************************************************
	 *  IHXFileStat Interface Methods                       ref:  hxfiles.h
	 */
	STDMETHOD(Stat)
		(THIS_
		 IHXFileStatResponse* pFileStatResponse
		);

	/************************************************************************
	 *  IHXGetFileFromSamePool Interface Methods            ref:  hxfiles.h
	 */
	STDMETHOD(GetFileObjectFromPool) 
				(THIS_
				IHXGetFileFromSamePoolResponse* response
				);

    /************************************************************************
     *	Method:
     *	    IHXFileMimeMapper::FindMimeType
     *	Purpose:
     */
    STDMETHOD(FindMimeType) (THIS_
			    const char*		    /*IN*/  pURL, 
			    IHXFileMimeMapperResponse* /*IN*/  pMimeMapperResponse
			    );

	private:
	/****** Private Class Variables ****************************************/
	INT32                   m_RefCount;       // Object's reference count
	IHXCommonClassFactory* m_pClassFactory;  // Creates common Helix classes
	IHXFileResponse*       m_pFileResponse;  // Provides completion notif.
	QFile*                   m_pFile;          // Actual file pointer
	char*			m_pFilename;      // Object's copy of file name
	IHXRequest*            m_pRequest;       // Used to get requested URL
	char*			m_pBasePath; // Platform's root path

	/****** Private Class Methods ******************************************/
	STDMETHOD(OpenFile)  (THIS_ UINT32 fileMode);
	STDMETHOD(ConvertToPlatformPath)
		(THIS_ 
		 REF(char*)  pFilePathPlatform,
		 const char* pFilePath
		);
};

#endif  /* ifndef QTOPIAFILEOBJECT_H */

