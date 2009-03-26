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

#ifndef QTOPIAFILESYSTEM_H
#define QTOPIAFILESYSTEM_H

#include "hxplugn.h"  /* IHXPlugin */
#include "hxfiles.h"  /* IHXFileSystemObject */

class QtopiaFileSystem :  public IHXFileSystemObject,
                              public IHXPlugin
{
	public:
	 QtopiaFileSystem(void);
	~QtopiaFileSystem(void);


	/************************************************************************
	 *  IHXFileSystemObject Interface Methods               ref:  hxfiles.h
	 */
	STDMETHOD(GetFileSystemInfo)
		(THIS_
		  REF(const char*) pShortName,
		  REF(const char*) pProtocol
		);
	STDMETHOD(InitFileSystem) (THIS_ IHXValues*     pOptions);
	STDMETHOD(CreateFile    ) (THIS_ IUnknown**      ppFileObject);
	STDMETHOD(CreateDir     ) (THIS_ IUnknown**	 ppDirObject);


	/************************************************************************
	 *  IHXPlugin Interface Methods                         ref:  hxplugn.h
	 */
	STDMETHOD(GetPluginInfo)
		(THIS_ 
		  REF(BOOL)        bLoadMultiple,
		  REF(const char*) pDescription,
		  REF(const char*) pCopyright,
		  REF(const char*) pMoreInfoURL,
		  REF(UINT32)      versionNumber
		);
	STDMETHOD(InitPlugin) (THIS_ IUnknown* pHXCore);


	/************************************************************************
	 *  IUnknown COM Interface Methods                          ref:  hxcom.h
	 */
	STDMETHOD (QueryInterface ) (THIS_ REFIID ID, void** ppInterfaceObj);
	STDMETHOD_(UINT32, AddRef ) (THIS);
	STDMETHOD_(UINT32, Release) (THIS);


	private:
	/****** Private Class Variables ****************************************/
	INT32                    m_RefCount;       // Object's reference count
	IHXCommonClassFactory*  m_pClassFactory;  // Creates common Helix classes
	char                     m_BasePath[1024]; // Platform's root path

	/****** Private Static Class Variables *********************************/
	static const char*      zm_pDescription;
	static const char*      zm_pCopyright;
	static const char*      zm_pMoreInfoURL;
	static const char*	zm_pShortName;
	static const char*	zm_pProtocol;
};

#endif  /* ifndef QTOPIAFILESYSTEM_H */

