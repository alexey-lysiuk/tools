/*
 * VS1X Tool
 * Copyright (C) 2016-2025  Alexey Lysiuk
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

void FatalExit()
{
	FatalExit(GetLastError());
}

enum class DataType
{
	RAW,
	DECRYPTED
};

enum class Version
{
	NONE,
	VS14,
	VS15,
	VS16,
	VS17,
};

struct ProcessData
{
	Version version = Version::NONE;
	HKEY regKey = nullptr;
	const TCHAR* keyName = nullptr;
	const BYTE* buffer = nullptr;
	DWORD size = 0;
	DataType type = DataType::RAW;
};

typedef std::vector<BYTE> ByteArray;

typedef void (*ProcessFunction)(const ProcessData& data);

SHORT GetProductID(const Version version)
{
	switch (version)
	{
	case Version::VS14:
		return 7078;

	case Version::VS15:
		return 8878;

	case Version::VS16:
		return 9278;

	case Version::VS17:
		return 9678;

	default:
		FatalExit(-1);
	}

	return 0;
}

const TCHAR* GetLicenseKeyName(const Version version)
{
	switch (version)
	{
	case Version::VS14:
		return _T("Licenses\\4D8CFBCB-2F6A-4AD2-BABF-10E28F6F2C8F");

	case Version::VS15:
		return _T("Licenses\\5C505A59-E312-4B89-9508-E162F8150517");

	case Version::VS16:
		return _T("Licenses\\41717607-F34E-432C-A138-A3CFD7E25CDA");

	case Version::VS17:
		return _T("Licenses\\1299B4B9-DFCC-476D-98F0-F65A2B46C96D");

	default:
		FatalExit(-1);
	}

	return _T("");
}

void ProcessLicenseData(const ProcessFunction processFunction, const Version version, const TCHAR* const keyName, const DWORD registryOptions = 0)
{
	HKEY regKey = nullptr;
	REG_API_CALL(RegOpenKeyEx(HKEY_CLASSES_ROOT, keyName, 0, KEY_READ | KEY_WOW64_32KEY | registryOptions, &regKey));

	DWORD encryptedDataSize = 0;
	REG_API_CALL(RegQueryValueEx(regKey, nullptr, nullptr, nullptr, nullptr, &encryptedDataSize));

	ByteArray encryptedData(encryptedDataSize);
	REG_API_CALL(RegQueryValueEx(regKey, nullptr, nullptr, nullptr, &encryptedData[0], &encryptedDataSize));

	CRYPT_INTEGER_BLOB encryptedBlob = { encryptedDataSize, &encryptedData[0] };
	CRYPT_INTEGER_BLOB decryptedBlob = { 0, nullptr };

	ProcessData data;
	data.version = version;
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

void ProcessLicenseData(const ProcessFunction processFunction, const Version version, const DWORD registryOptions = 0)
{
	const TCHAR* const licenseKeyName = GetLicenseKeyName(version);
	ProcessLicenseData(processFunction, version, licenseKeyName, registryOptions);

	HKEY regKey = nullptr;
	REG_API_CALL(RegOpenKeyEx(HKEY_CLASSES_ROOT, licenseKeyName, 0, KEY_READ | KEY_WOW64_32KEY | registryOptions, &regKey));

	for (DWORD subKeyIndex = 0; /* EMPTY */ ; ++subKeyIndex)
	{
		TCHAR subKeyName[256] = {};
		DWORD subKeyNameSize = 256;

		const LONG subKeyResult = RegEnumKeyEx(regKey, subKeyIndex, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr);

		if (ERROR_SUCCESS == subKeyResult)
		{
			TCHAR fullKeyName[256] = {};
			_sntprintf_s(fullKeyName, _countof(fullKeyName), _TRUNCATE, _T("%s\\%s"), licenseKeyName, subKeyName);

			ProcessLicenseData(processFunction, version, fullKeyName, registryOptions);
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


ByteArray CopyLicense(const ProcessData& data)
{
	if (224 != data.size)
	{
		_tprintf(_T("ERROR: Wrong size of decrypted data for key \"%s\"\n"), data.keyName);
		FatalExit(-1);
	}

	const SHORT productID = *reinterpret_cast<const SHORT*>(data.buffer);

	if (GetProductID(data.version) != productID)
	{
		_tprintf(_T("ERROR: Wrong signature of decrypted data for key \"%s\"\n"), data.keyName);
		FatalExit(-1);
	}

	ByteArray result(data.size);
	memcpy_s(&result[0], data.size, data.buffer, data.size);

	return result;
}

void WriteLicense(const HKEY registryKey, ByteArray& buffer)
{
	CRYPT_INTEGER_BLOB decryptedBlob = { DWORD(buffer.size()), &buffer[0] };
	CRYPT_INTEGER_BLOB encryptedBlob = { 0, nullptr };

	if (!CryptProtectData(&decryptedBlob, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN | CRYPTPROTECT_LOCAL_MACHINE, &encryptedBlob))
	{
		_tprintf(_T("ERROR: Failed to encrypt data with error %lu\n"), GetLastError());
		FatalExit();
	}

	REG_API_CALL(RegSetValueEx(registryKey, nullptr, 0, REG_BINARY, encryptedBlob.pbData, encryptedBlob.cbData));

	LocalFree(encryptedBlob.pbData);
}


void ProlongLicense(const ProcessData& data)
{
	ByteArray buffer = CopyLicense(data);

	SYSTEMTIME systemTime = {};
	GetSystemTime(&systemTime);

	FILETIME fileTime = {};

	if (!SystemTimeToFileTime(&systemTime, &fileTime))
	{
		_putts(_T("ERROR: Failed to convert system time to file time"));
		FatalExit();
	}

	ULARGE_INTEGER intTime = { fileTime.dwLowDateTime, fileTime.dwHighDateTime };
	intTime.QuadPart += 29ULL * 24 * 60 * 60 * 10 * 1000 * 1000;

	fileTime.dwHighDateTime = intTime.HighPart;
	fileTime.dwLowDateTime = intTime.LowPart;

	if (!FileTimeToSystemTime(&fileTime, &systemTime))
	{
		_putts(_T("ERROR: Failed to convert file time to system time"));
		FatalExit();
	}

	WORD* year  = reinterpret_cast<WORD*>(&buffer[0xD0]);
	WORD* month = reinterpret_cast<WORD*>(&buffer[0xD2]);
	WORD* day   = reinterpret_cast<WORD*>(&buffer[0xD4]);

	*year  = systemTime.wYear;
	*month = systemTime.wMonth;
	*day   = systemTime.wDay;

	buffer[0xDD] = 1;
	buffer[0xDD] = 1;

	WriteLicense(data.regKey, buffer);
}


void RegisterProduct(const ProcessData& data)
{
	ByteArray buffer = CopyLicense(data);

	memset(&buffer[0xD0], 0xFF, 6);
	buffer[0xC0] = 12;

	WriteLicense(data.regKey, buffer);
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

void UpdateLicence(ProcessFunction processFunction, const Version version)
{
	if (!IsElevated())
	{
		_putts(_T("ERROR: You must have administrative rights to execute this command\n"));
		FatalExit(-1);
	}

	TCHAR communityKey[256] = {};
	_sntprintf_s(communityKey, _countof(communityKey), _TRUNCATE, _T("%s\\%05i"),
		GetLicenseKeyName(version), GetProductID(version));

	ProcessLicenseData(processFunction, version, communityKey, KEY_WRITE);
}

} // unnamed namespace

int _tmain(int argc, TCHAR** argv)
{
	if (argc < 2)
	{
		_putts(_T("ERROR: insufficient command line options"));
		return EXIT_FAILURE;
	}

	enum class Action
	{
		NONE,
		PROLONG,
		REGISTER
	};

	Version version = Version::NONE;
	Action action = Action::NONE;

	for (int i = 1; i < argc; ++i)
	{
		if (_tcscmp(argv[i], _T("--vs14")) == 0)
		{
			version = Version::VS14;
		}
		else if (_tcscmp(argv[i], _T("--vs15")) == 0)
		{
			version = Version::VS15;
		}
		else if (_tcscmp(argv[i], _T("--vs16")) == 0)
		{
			version = Version::VS16;
		}
		else if (_tcscmp(argv[i], _T("--vs17")) == 0)
		{
			version = Version::VS17;
		}
		else if (_tcscmp(argv[i], _T("--prolong")) == 0)
		{
			action = Action::PROLONG;
		}
		else if (_tcscmp(argv[i], _T("--register")) == 0)
		{
			action = Action::REGISTER;
		}
	}

	if (Version::NONE == version)
	{
		_putts(_T("ERROR: no version specified"));
		return EXIT_FAILURE;
	}

	if (Action::PROLONG == action)
	{
		UpdateLicence(ProlongLicense, version);
	}
	else if (Action::REGISTER == action)
	{
		UpdateLicence(RegisterProduct, version);
	}
	else
	{
		ProcessLicenseData(PrintBytes, version);
		ProcessLicenseData(DumpBytes, version);
	}

	return EXIT_SUCCESS;
}
