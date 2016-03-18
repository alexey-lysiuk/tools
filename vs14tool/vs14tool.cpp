
#include <cstdlib>
#include <vector>

#include <windows.h>
#include <tchar.h>

#define REG_API_CALL(CALL)                                                        \
	{                                                                             \
		LONG result = CALL;                                                       \
		if (ERROR_SUCCESS != result)                                              \
		{                                                                         \
			_tprintf(_T("ERROR: %s failed with error %li\n"), _T(#CALL), result); \
			if (IsDebuggerPresent()) __debugbreak();                              \
			exit(result);                                                         \
		}                                                                         \
	}

namespace
{

void FatalExit(const int code)
{
	if (IsDebuggerPresent())
	{
		__debugbreak();
	}

	exit(code);
}

enum class DataType
{
	RAW,
	DECRYPTED
};

struct ProcessData
{
	HKEY regKey = nullptr;
	const TCHAR* keyName = nullptr;
	const BYTE* buffer = nullptr;
	DWORD size = 0;
	DataType type = DataType::RAW;
};

typedef void (*ProcessFunction)(const ProcessData& data);

const TCHAR LICENSE_KEY_NAME[] = _T("Licenses\\4D8CFBCB-2F6A-4AD2-BABF-10E28F6F2C8F");

void ProcessLicenseData(ProcessFunction processFunction, const TCHAR* const keyName, const DWORD registryOptions = 0)
{
	HKEY regKey = nullptr;
	REG_API_CALL(RegOpenKeyEx(HKEY_CLASSES_ROOT, keyName, 0, KEY_READ | KEY_WOW64_32KEY | registryOptions, &regKey));

	DWORD encryptedDataSize = 0;
	REG_API_CALL(RegQueryValueEx(regKey, nullptr, nullptr, nullptr, nullptr, &encryptedDataSize));

	std::vector<BYTE> encryptedData(encryptedDataSize);
	REG_API_CALL(RegQueryValueEx(regKey, nullptr, nullptr, nullptr, &encryptedData[0], &encryptedDataSize));

	CRYPT_INTEGER_BLOB encryptedBlob = { encryptedDataSize, &encryptedData[0] };
	CRYPT_INTEGER_BLOB decryptedBlob = { 0, nullptr };

	ProcessData data;
	data.regKey = regKey;
	data.keyName = keyName;

	if (CryptUnprotectData(&encryptedBlob, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &decryptedBlob))
	{
		data.buffer = decryptedBlob.pbData;
		data.size = decryptedBlob.cbData;
		data.type = DataType::DECRYPTED;
	}
	else
	{
		data.buffer = &encryptedData[0];
		data.size = encryptedDataSize;
		data.type = DataType::RAW;
	}

	processFunction(data);

	if (nullptr != decryptedBlob.pbData)
	{
		LocalFree(decryptedBlob.pbData);
	}

	REG_API_CALL(RegCloseKey(regKey));
}

void ProcessLicenseData(ProcessFunction processFunction, const DWORD registryOptions = 0)
{
	ProcessLicenseData(processFunction, LICENSE_KEY_NAME, registryOptions);

	HKEY regKey = nullptr;
	REG_API_CALL(RegOpenKeyEx(HKEY_CLASSES_ROOT, LICENSE_KEY_NAME, 0, KEY_READ | KEY_WOW64_32KEY | registryOptions, &regKey));

	for (DWORD subKeyIndex = 0; /* EMPTY */ ; ++subKeyIndex)
	{
		TCHAR subKeyName[256] = {};
		DWORD subKeyNameSize = 256;

		const LONG subKeyResult = RegEnumKeyEx(regKey, subKeyIndex, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr);

		if (ERROR_SUCCESS == subKeyResult)
		{
			TCHAR fullKeyName[256] = {};
			_sntprintf_s(fullKeyName, _countof(fullKeyName), _TRUNCATE, _T("%s\\%s"), LICENSE_KEY_NAME, subKeyName);

			ProcessLicenseData(processFunction, fullKeyName, registryOptions);
		}
		else if (ERROR_NO_MORE_ITEMS == subKeyResult)
		{
			break;
		}
		else
		{
			_tprintf(_T("ERROR: RegEnumKeyEx() failed with error %li\n"), subKeyResult);
			FatalExit(subKeyResult);
		}
	}

	REG_API_CALL(RegCloseKey(regKey));
}


void PrintBytes(const ProcessData& data)
{
	_tprintf(_T("\n%s  %s\n\n"), data.keyName, (DataType::DECRYPTED == data.type ? _T("[DECRYPTED]") : _T("[RAW]")));

	static const DWORD BYTES_PER_ROW = 16;

	for (DWORD y = 0; y < data.size / BYTES_PER_ROW; ++y)
	{
		_tprintf(_T("%04X:  "), y * BYTES_PER_ROW);

		for (DWORD x = 0; x < BYTES_PER_ROW; ++x)
		{
			_tprintf(_T("%02X "), data.buffer[y * BYTES_PER_ROW + x]);
		}

		_tprintf(_T("  "));

		for (DWORD x = 0; x < BYTES_PER_ROW; ++x)
		{
			const BYTE byte = data.buffer[y * BYTES_PER_ROW + x];
			_tprintf(_T("%c"), byte < 0x20 || byte > 0x7F ? _T('.') : TCHAR(byte));
		}

		_tprintf(_T("\n"));
	}
}

void DumpBytes(const ProcessData& data)
{
	const size_t fileNameSize = _tcslen(data.keyName) + 4; // for extention

	TCHAR* const fileName = static_cast<TCHAR*>(alloca((fileNameSize + 1) * sizeof(TCHAR)));
	_tcscpy_s(fileName, fileNameSize + 1, data.keyName);
	_tcscat_s(fileName, fileNameSize + 1, (DataType::DECRYPTED == data.type ? _T(".dec") : _T(".raw")));

	for (size_t i = 0; i < fileNameSize; ++i)
	{
		if ('\\' == fileName[i])
		{
			fileName[i] = '.';
		}
	}

	const auto CallFailed = [fileName](const TCHAR* const function)
	{
		_tprintf(_T("ERROR: %s() failed for file \"%s\" with error %i\n"), function, fileName, errno);
		FatalExit(errno);
	};

	FILE* file = nullptr;
	_tfopen_s(&file, fileName, _T("wb"));

	if (nullptr == file)
	{
		CallFailed(_T("fopen"));
	}

	if (1 != fwrite(data.buffer, data.size, 1, file))
	{
		CallFailed(_T("fwrite"));
	}

	if (0 != fclose(file))
	{
		CallFailed(_T("fclose"));
	}
}


void ProlongLicense(const ProcessData& data)
{
	if (224 != data.size)
	{
		_tprintf(_T("ERROR: Wrong size of decrypted data for key \"%s\"\n"), data.keyName);
		return;
	}

	if (   0xA6 != data.buffer[0]
		|| 0x1B != data.buffer[1])
	{
		_tprintf(_T("ERROR: Wrong signature of decrypted data for key \"%s\"\n"), data.keyName);
		return;
	}

	std::vector<BYTE> buffer(data.size);
	memcpy_s(&buffer[0], data.size, data.buffer, data.size);

	SYSTEMTIME systemTime = {};
	GetSystemTime(&systemTime);

	FILETIME fileTime = {};

	if (!SystemTimeToFileTime(&systemTime, &fileTime))
	{
		_putts(_T("ERROR: Failed to convert system time to file time"));
		return;
	}

	ULARGE_INTEGER intTime = { fileTime.dwLowDateTime, fileTime.dwHighDateTime };
	intTime.QuadPart += 29ULL * 24 * 60 * 60 * 10 * 1000 * 1000;

	fileTime.dwHighDateTime = intTime.HighPart;
	fileTime.dwLowDateTime = intTime.LowPart;

	if (!FileTimeToSystemTime(&fileTime, &systemTime))
	{
		_putts(_T("ERROR: Failed to convert file time to system time"));
		return;
	}

	WORD* year  = reinterpret_cast<WORD*>(&buffer[0xD0]);
	WORD* month = reinterpret_cast<WORD*>(&buffer[0xD2]);
	WORD* day   = reinterpret_cast<WORD*>(&buffer[0xD4]);

	*year  = systemTime.wYear;
	*month = systemTime.wMonth;
	*day   = systemTime.wDay;

	buffer[0xDD] = 1;
	buffer[0xDD] = 1;

	CRYPT_INTEGER_BLOB decryptedBlob = { data.size, &buffer[0] };
	CRYPT_INTEGER_BLOB encryptedBlob = { 0, nullptr };

	if (!CryptProtectData(&decryptedBlob, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN | CRYPTPROTECT_LOCAL_MACHINE, &encryptedBlob))
	{
		_tprintf(_T("ERROR: Failed to encrypt data with error %lu\n"), GetLastError());
		return;
	}

	REG_API_CALL(RegSetValueEx(data.regKey, nullptr, 0, REG_BINARY, encryptedBlob.pbData, encryptedBlob.cbData));

	LocalFree(encryptedBlob.pbData);
}

BOOL IsElevated()
{
	BOOL result = false;
	HANDLE token = nullptr;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
	{
		TOKEN_ELEVATION elevation;
		DWORD size = sizeof(TOKEN_ELEVATION);

		if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size))
		{
			result = elevation.TokenIsElevated;
		}

		CloseHandle(token);
	}

	return result;
}

void ProlongLicense()
{
	if (!IsElevated())
	{
		_putts(_T("ERROR: You must have administrative rights to execute this command\n"));
		return;
	}

	TCHAR communityKey[256] = {};
	_sntprintf_s(communityKey, _countof(communityKey), _TRUNCATE, _T("%s\\07078"), LICENSE_KEY_NAME);

	ProcessLicenseData(ProlongLicense, communityKey, KEY_WRITE);
}

} // unnamed namespace

int _tmain(int argc, TCHAR** argv)
{
	if (argc > 1 && _tcscmp(argv[1], _T("--prolong")) == 0)
	{
		ProlongLicense();
		return EXIT_SUCCESS;
	}

	ProcessLicenseData(PrintBytes);
	ProcessLicenseData(DumpBytes);
}
