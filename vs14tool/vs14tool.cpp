
#include <cstdlib>
#include <vector>

#include <windows.h>
#include <tchar.h>

#define REG_API_CALL(CALL)                                                        \
	{                                                                             \
		LONG result = CALL;                                                       \
		if (ERROR_SUCCESS != result)                                              \
		{                                                                         \
			_tprintf(_T("ERROR: %s failed with error %i\n"), _T(#CALL), result);  \
			if (IsDebuggerPresent()) __debugbreak();                              \
			exit(result);                                                         \
		}                                                                         \
	}

static const TCHAR LICENSE_KEY_NAME[] = _T("Licenses\\4D8CFBCB-2F6A-4AD2-BABF-10E28F6F2C8F");

void PrintBytes(const BYTE* const buffer, const DWORD size)
{
	static const DWORD BYTES_PER_ROW = 16;

	for (DWORD y = 0; y < size / BYTES_PER_ROW; ++y)
	{
		_tprintf(_T("%04X:  "), y * BYTES_PER_ROW);

		for (DWORD x = 0; x < BYTES_PER_ROW; ++x)
		{
			_tprintf(_T("%02X "), buffer[y * BYTES_PER_ROW + x]);
		}

		_tprintf(_T("  "));

		for (DWORD x = 0; x < BYTES_PER_ROW; ++x)
		{
			const BYTE byte = buffer[y * BYTES_PER_ROW + x];
			_tprintf(_T("%c"), byte < 0x20 || byte > 0x7F ? _T('.') : TCHAR(byte));
		}

		_tprintf(_T("\n"));
	}
}

void PrintLicenseData(const TCHAR* const keyName)
{
	HKEY regKey = nullptr;
	REG_API_CALL(RegOpenKeyEx(HKEY_CLASSES_ROOT, keyName, 0, KEY_READ | KEY_WOW64_32KEY, &regKey));

	DWORD encryptedDataSize = 0;
	REG_API_CALL(RegQueryValueEx(regKey, nullptr, nullptr, nullptr, nullptr, &encryptedDataSize));

	std::vector<BYTE> encryptedData(encryptedDataSize);
	REG_API_CALL(RegQueryValueEx(regKey, nullptr, nullptr, nullptr, &encryptedData[0], &encryptedDataSize));

	CRYPT_INTEGER_BLOB encryptedBlob = { encryptedDataSize, &encryptedData[0] };
	CRYPT_INTEGER_BLOB decryptedBlob = { 0, nullptr };

	if (CryptUnprotectData(&encryptedBlob, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &decryptedBlob))
	{
		_tprintf(_T("\n%s  [DECRYPTED]\n\n"), keyName);
		PrintBytes(decryptedBlob.pbData, decryptedBlob.cbData);
	}
	else
	{
		_tprintf(_T("\n%s  [RAW]\n\n"), keyName);
		PrintBytes(&encryptedData[0], encryptedDataSize);
	}

	REG_API_CALL(RegCloseKey(regKey));

	LocalFree(decryptedBlob.pbData);
}

void PrintLicenseData()
{
	PrintLicenseData(LICENSE_KEY_NAME);

	HKEY regKey = nullptr;
	REG_API_CALL(RegOpenKeyEx(HKEY_CLASSES_ROOT, LICENSE_KEY_NAME, 0, KEY_READ | KEY_WOW64_32KEY, &regKey));

	for (DWORD subKeyIndex = 0; /* EMPTY */ ; ++subKeyIndex)
	{
		TCHAR subKeyName[256] = {};
		DWORD subKeyNameSize = 256;

		const LONG subKeyResult = RegEnumKeyEx(regKey, subKeyIndex, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr);

		if (ERROR_SUCCESS == subKeyResult)
		{
			TCHAR fullKeyName[256] = {};
			_sntprintf_s(fullKeyName, _countof(fullKeyName), _TRUNCATE, _T("%s\\%s"), LICENSE_KEY_NAME, subKeyName);

			PrintLicenseData(fullKeyName);
		}
		else if (ERROR_NO_MORE_ITEMS == subKeyResult)
		{
			break;
		}
		else
		{
			_tprintf(_T("ERROR: RegEnumKeyEx() failed with error %i\n"), subKeyResult);

			if (IsDebuggerPresent()) __debugbreak(); 
			exit(subKeyResult);
		}
	}

	REG_API_CALL(RegCloseKey(regKey));
}

int _tmain(int argc, TCHAR** argv)
{
	PrintLicenseData();
}

/*
int _tmain(int argc, TCHAR** argv)
{
	HKEY licenseKey = nullptr;

	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("Licenses\\4D8CFBCB-2F6A-4AD2-BABF-10E28F6F2C8F\\07078"), 0, KEY_READ | KEY_WRITE | KEY_WOW64_32KEY, &licenseKey))
	{
		// TODO: report error
		return EXIT_FAILURE;
	}

	DWORD encryptedDataSize = 0;

	if (ERROR_SUCCESS != RegQueryValueEx(licenseKey, nullptr, nullptr, nullptr, nullptr, &encryptedDataSize))
	{
		// TODO: report error
		return EXIT_FAILURE;
	}

	std::vector<BYTE> encryptedData;
	encryptedData.resize(encryptedDataSize);

	if (ERROR_SUCCESS != RegQueryValueEx(licenseKey, nullptr, nullptr, nullptr, &encryptedData[0], &encryptedDataSize))
	{
		// TODO: report error
		return EXIT_FAILURE;
	}

	//RegCloseKey(licenseKey);

	CRYPT_INTEGER_BLOB encryptedBlob = { encryptedDataSize, &encryptedData[0] };
	CRYPT_INTEGER_BLOB decryptedBlob = { 0, nullptr };

	if (!CryptUnprotectData(&encryptedBlob, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &decryptedBlob))
	{
		// TODO: report error
		return EXIT_FAILURE;
	}

	if (240 == decryptedBlob.cbData)
	{
		// TODO: report error
		return EXIT_FAILURE;
	}

	BYTE* decryptedData = decryptedBlob.pbData;

	//if (    '%' != decryptedData[0]
	//	|| '\0' != decryptedData[1]
	//	|| '\0' != decryptedData[2]
	//	|| '\0' != decryptedData[3])
	//{
	//	// TODO: report error
	//	return EXIT_FAILURE;
	//}

	//WORD* year  = reinterpret_cast<WORD*>(&decryptedData[0x12C]);
	//WORD* month = reinterpret_cast<WORD*>(&decryptedData[0x12E]);
	//WORD* day   = reinterpret_cast<WORD*>(&decryptedData[0x130]);

	//SYSTEMTIME currentTime;
	//GetSystemTime(&currentTime);

	//*year  = currentTime.wYear;
	//*month = currentTime.wMonth;
	//*day   = currentTime.wDay;

	if (   0xA6 != decryptedData[0]
		|| 0x1B != decryptedData[1])
	{
		// TODO: report error
		return EXIT_FAILURE;
	}

	WORD* year  = reinterpret_cast<WORD*>(&decryptedData[0xD0]);
	WORD* month = reinterpret_cast<WORD*>(&decryptedData[0xD2]);
	WORD* day   = reinterpret_cast<WORD*>(&decryptedData[0xD4]);

	SYSTEMTIME currentTime;
	GetSystemTime(&currentTime);

	*year = 2016; //currentTime.wYear;
	*month = 4; //currentTime.wMonth;
	*day = 9; // currentTime.wDay;

	//decryptedData[0xD0] = 0xFFFF;
	decryptedData[0xDD] = 1;
	decryptedData[0xDD] = 1;

	encryptedBlob = { 0, nullptr };

	if (!CryptProtectData(&decryptedBlob, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN | CRYPTPROTECT_LOCAL_MACHINE, &encryptedBlob))
	{
		// TODO: report error
		return EXIT_FAILURE;
	}

	LONG rez = RegSetValueEx(licenseKey, nullptr, 0, REG_BINARY, encryptedBlob.pbData, encryptedBlob.cbData);

	if (ERROR_SUCCESS != rez)
	{
		// TODO: report error
		return EXIT_FAILURE;
	}

	RegCloseKey(licenseKey);

	LocalFree(decryptedBlob.pbData);
	LocalFree(encryptedBlob.pbData);
}
*/
