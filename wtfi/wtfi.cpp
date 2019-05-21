/*
 * Windows 10 Fix for Intel OpenGL driver (WTFI)
 * Copyright (C) 2016, 2017  Alexey Lysiuk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <vector>
#include <string>

#include <windows.h>
#include <tchar.h>
#include <conio.h>

#pragma warning(disable: 4192)
#import <msxml6.dll>


namespace
{

struct Resource
{
	LPCTSTR     id;
	LANGID      lang;
	std::string data;
};

typedef std::vector<Resource> ResourceList;


class LibraryModule
{
public:
	explicit LibraryModule(const TCHAR* const filename)
	: m_module(LoadLibraryEx(filename, NULL, DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE))
	{
	}

	~LibraryModule()
	{
		if (NULL != m_module)
		{
			FreeLibrary(m_module);
		}
	}

	HMODULE operator*() const { return m_module; }

private:
	HMODULE m_module;
};


static void PrintError(const TCHAR* const failedFunction, const TCHAR* const filename)
{
	_tprintf(_T("ERROR: %s() failed for file %s with code 0x%08x\n"), failedFunction, filename, GetLastError());
}


BOOL CALLBACK OnEnumResourceLanguage(HMODULE module, LPCTSTR, LPCTSTR id, LANGID language, LONG_PTR param)
{
	const Resource resource = { id, language };

	ResourceList& resources = *reinterpret_cast<ResourceList*>(param);
	resources.push_back(resource);

	return TRUE;
}

BOOL CALLBACK OnEnumResourceName(HMODULE module, LPCTSTR type, LPTSTR id, LONG_PTR param)
{
	return EnumResourceLanguages(module, type, id, OnEnumResourceLanguage, param);
}

void LoadManifests(const TCHAR* const filename, ResourceList& resources)
{
	LibraryModule module(filename);

	if (NULL == *module)
	{
		PrintError(_T("LoadLibraryEx"), filename);
		return;
	}

	if (!EnumResourceNames(*module, RT_MANIFEST, OnEnumResourceName, reinterpret_cast<LONG_PTR>(&resources)))
	{
		PrintError(_T("EnumResourceNames"), filename);
		return;
	}

	if (resources.empty())
	{
		_tprintf(_T("WARNING: File %s has no manifest resources\n"), filename);
		return;
	}

	for (ResourceList::iterator res = resources.begin(), last = resources.end(); res != last; ++res)
	{
		const HRSRC resHandle = FindResource(*module, res->id, RT_MANIFEST);
		if (NULL == resHandle)
		{
			PrintError(_T("FindResource"), filename);
			continue;
		}

		const DWORD size = SizeofResource(*module, resHandle);
		if (0 == size)
		{
			PrintError(_T("SizeofResource"), filename);
			continue;
		}

		const HGLOBAL ptrHandle = LoadResource(*module, resHandle);
		if (NULL ==  ptrHandle)
		{
			PrintError(_T("LoadResource"), filename);
			continue;
		}

		const void* ptr = LockResource(ptrHandle);
		if (NULL == ptr)
		{
			PrintError(_T("LockResource"), filename);
			continue;
		}

		res->data.resize(size);
		memcpy(&res->data[0], ptr, size);
	}
}

MSXML2::IXMLDOMNodePtr FindXMLNode(MSXML2::IXMLDOMNodePtr parent, const char* const name)
{
	if (NULL == parent)
	{
		return NULL;
	}

	MSXML2::IXMLDOMNodeListPtr nodeList = parent->childNodes;

	for (long i = 0, count = nodeList->length; i < count; ++i)
	{
		MSXML2::IXMLDOMNodePtr childNode = nodeList->item[i];

		if (NULL != childNode && 0 == strcmp(childNode->nodeName, name))
		{
			return childNode;
		}
	}

	return NULL;
}

bool GetFixedManifest(const TCHAR* const filename, const std::string& input, std::string& output)
{
	MSXML2::IXMLDOMDocumentPtr dom;

	if (FAILED(dom.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER))) 
	{
		PrintError(_T("IXMLDOMDocument::CreateInstance"), filename);
		return false;
	}

	try
	{
		dom->async = VARIANT_FALSE;
		dom->validateOnParse = VARIANT_FALSE;
		dom->resolveExternals = VARIANT_FALSE;

		if (VARIANT_TRUE == dom->loadXML(input.c_str()))
		{
			MSXML2::IXMLDOMNodePtr assemblyNode = FindXMLNode(dom, "assembly");

			if (NULL == assemblyNode)
			{
				_tprintf(_T("ERROR: Broken manifest in file %s \n"), filename);
				return false;
			}

			MSXML2::IXMLDOMNodePtr compatNode = FindXMLNode(assemblyNode, "compatibility");

			if (NULL == compatNode)
			{
				_tprintf(_T("WARNING: File %s has no compatibility record in manifest\n"), filename);
				return false;
			}

			assemblyNode->removeChild(compatNode);
			output = (LPCSTR)dom->xml;
		}
		else
		{
			_tprintf(_T("ERROR: Failed to parse manifest from file %s with message \"%s\"\n"), filename, dom->parseError->Getreason().GetBSTR());
			return false;
		}
	}
	catch(_com_error errorObject)
	{
		_tprintf(_T("ERROR: Failed to parse manifest from file %s with code 0x%08x\n"), filename, errorObject.Error());
		return false;
	}

	return true;
}

void UpdateManifest(const TCHAR* const filename)
{
	ResourceList resources;
	LoadManifests(filename, resources);

	for (ResourceList::const_iterator res = resources.begin(), last = resources.end(); res != last; ++res)
	{
		std::string manifest;
		
		if (!GetFixedManifest(filename, res->data, manifest))
		{
			continue;
		}

		const HANDLE handle = BeginUpdateResource(filename, FALSE);

		if (NULL == handle)
		{
			PrintError(_T("BeginUpdateResource"), filename);
			continue;
		}

		if (!UpdateResource(handle, RT_MANIFEST, res->id, res->lang, &manifest[0], manifest.size()))
		{
			PrintError(_T("UpdateResource"), filename);
			continue;
		}

		if (!EndUpdateResource(handle, FALSE))
		{
			PrintError(_T("EndUpdateResource"), filename);
			continue;
		}

		_tprintf(_T("File %s was successfully updated\n"), filename);
	}
}


void EnterVisualMode()
{
	TCHAR filename[1024] = _T("gzdoom.exe");

	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof ofn;
	ofn.lpstrFilter = _T("Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof filename;
	ofn.lpstrTitle = _T("Select executable file to patch manifest");
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if (GetOpenFileName(&ofn))
	{
		UpdateManifest(ofn.lpstrFile);
	}
}

} // unnamed namespace


int _tmain(int argc, TCHAR** argv)
{
	if (SUCCEEDED(CoInitialize(NULL)))
	{
		if (argc < 2)
		{
			_tprintf(_T("Usage: %s <file.exe> <...>\n\nEntering visual mode...\n"), argv[0]);

			EnterVisualMode();
		}
		else
		{
			for (int i = 1; i < argc; ++i)
			{
				UpdateManifest(argv[i]);
			}
		}

		CoUninitialize();
	}
	else
	{
		_tprintf(_T("ERROR: CoInitialize() failed with code 0x%08x\n"), GetLastError());
	}

	_putts(_T("\nPress any key to continue...\n"));
	_getch();
}
