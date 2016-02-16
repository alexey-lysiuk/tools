
#include <vector>
#include <string>

#include <windows.h>
#include <tchar.h>


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
	_tprintf(_T("ERROR: %s() failed for file %s with error code 0x%08x\n"), failedFunction, filename, GetLastError());
}


static BOOL CALLBACK OnEnumResourceLanguage(HMODULE module, LPCTSTR, LPCTSTR id, LANGID language, LONG_PTR param)
{
	const Resource resource = { id, language };

	ResourceList& resources = *reinterpret_cast<ResourceList*>(param);
	resources.push_back(resource);

	return TRUE;
}

static BOOL CALLBACK OnEnumResourceName(HMODULE module, LPCTSTR type, LPTSTR id, LONG_PTR param)
{
	return EnumResourceLanguages(module, type, id, OnEnumResourceLanguage, param);
}

static void LoadManifests(const TCHAR* const filename, ResourceList& resources)
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

static void UpdateManifest(const TCHAR* const filename)
{
	ResourceList resources;
	LoadManifests(filename, resources);

	for (ResourceList::const_iterator res = resources.begin(), last = resources.end(); res != last; ++res)
	{
		static const char* const COMPAT_BEGIN = "<compatibility";
		static const char* const COMPAT_END = "</compatibility>";

		const size_t beginPos = res->data.find(COMPAT_BEGIN);

		if (std::string::npos == beginPos)
		{
			_tprintf(_T("WARNING: File %s has no compatibility record in manifest\n"), filename);
			continue;
		}

		const size_t endPos = res->data.find(COMPAT_END);

		if (std::string::npos == endPos)
		{
			_tprintf(_T("WARNING: File %s has broken compatibility record in manifest\n"), filename);
			continue;
		}

		std::string manifest = res->data.substr(0, beginPos);
		manifest += res->data.substr(endPos + strlen(COMPAT_END));

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


int _tmain(int argc, TCHAR** argv)
{
	if (argc < 2)
	{
		_tprintf(_T("Usage: %s <file.exe> <...>\n"), argv[0]);
		return 0;
	}

	for (int i = 1; i < argc; ++i)
	{
		UpdateManifest(argv[i]);
	}
}
