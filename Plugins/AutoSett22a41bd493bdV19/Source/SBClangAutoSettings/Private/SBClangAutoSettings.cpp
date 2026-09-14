// Copyright Sam Bonifacio. All Rights Reserved.

#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, SBClangAutoSettings)

// Clang-built Windows modules import wrappers that are absent from MSVC-built launcher-engine Core binaries.
#if defined(PLATFORM_WINDOWS) && PLATFORM_WINDOWS && defined(__clang__)

#include "Microsoft/WindowsHWrapper.h"

namespace Windows
{
	SBCLANGAUTOSETTINGS_API ::HMODULE WINAPI LoadLibraryW(::LPCTSTR lpFileName)
	{
		return ::LoadLibraryW(lpFileName);
	}

	SBCLANGAUTOSETTINGS_API ::BOOL WINAPI FreeLibrary(::HMODULE hModule)
	{
		return ::FreeLibrary(hModule);
	}

	SBCLANGAUTOSETTINGS_API void WINAPI InitializeCriticalSection(::LPCRITICAL_SECTION lpCriticalSection)
	{
		::InitializeCriticalSection(lpCriticalSection);
	}

	SBCLANGAUTOSETTINGS_API ::BOOL WINAPI InitializeCriticalSectionAndSpinCount(::LPCRITICAL_SECTION lpCriticalSection, ::DWORD dwSpinCount)
	{
		return ::InitializeCriticalSectionAndSpinCount(lpCriticalSection, dwSpinCount);
	}

	SBCLANGAUTOSETTINGS_API ::DWORD WINAPI SetCriticalSectionSpinCount(::LPCRITICAL_SECTION lpCriticalSection, ::DWORD dwSpinCount)
	{
		return ::SetCriticalSectionSpinCount(lpCriticalSection, dwSpinCount);
	}

	SBCLANGAUTOSETTINGS_API ::BOOL WINAPI TryEnterCriticalSection(::LPCRITICAL_SECTION lpCriticalSection)
	{
		return ::TryEnterCriticalSection(lpCriticalSection);
	}

	SBCLANGAUTOSETTINGS_API void WINAPI EnterCriticalSection(::LPCRITICAL_SECTION lpCriticalSection)
	{
		::EnterCriticalSection(lpCriticalSection);
	}

	SBCLANGAUTOSETTINGS_API void WINAPI LeaveCriticalSection(::LPCRITICAL_SECTION lpCriticalSection)
	{
		::LeaveCriticalSection(lpCriticalSection);
	}

	SBCLANGAUTOSETTINGS_API void WINAPI DeleteCriticalSection(::LPCRITICAL_SECTION lpCriticalSection)
	{
		::DeleteCriticalSection(lpCriticalSection);
	}

	SBCLANGAUTOSETTINGS_API void WINAPI InitializeSRWLock(::PSRWLOCK SRWLock)
	{
		::InitializeSRWLock(SRWLock);
	}

	SBCLANGAUTOSETTINGS_API void WINAPI AcquireSRWLockShared(::PSRWLOCK SRWLock)
	{
		::AcquireSRWLockShared(SRWLock);
	}

	SBCLANGAUTOSETTINGS_API ::BOOLEAN WINAPI TryAcquireSRWLockShared(::PSRWLOCK SRWLock)
	{
		return ::TryAcquireSRWLockShared(SRWLock);
	}

	SBCLANGAUTOSETTINGS_API ::BOOLEAN WINAPI TryAcquireSRWLockExclusive(::PSRWLOCK SRWLock)
	{
		return ::TryAcquireSRWLockExclusive(SRWLock);
	}

	SBCLANGAUTOSETTINGS_API void WINAPI ReleaseSRWLockShared(::PSRWLOCK SRWLock)
	{
		::ReleaseSRWLockShared(SRWLock);
	}

	SBCLANGAUTOSETTINGS_API void WINAPI AcquireSRWLockExclusive(::PSRWLOCK SRWLock)
	{
		::AcquireSRWLockExclusive(SRWLock);
	}

	SBCLANGAUTOSETTINGS_API void WINAPI ReleaseSRWLockExclusive(::PSRWLOCK SRWLock)
	{
		::ReleaseSRWLockExclusive(SRWLock);
	}

	SBCLANGAUTOSETTINGS_API ::BOOL WINAPI ConnectNamedPipe(::HANDLE hNamedPipe, ::LPOVERLAPPED lpOverlapped)
	{
		return ::ConnectNamedPipe(hNamedPipe, lpOverlapped);
	}

	SBCLANGAUTOSETTINGS_API ::BOOL WINAPI GetOverlappedResult(::HANDLE hFile, ::LPOVERLAPPED lpOverlapped, ::LPDWORD lpNumberOfBytesTransferred, ::BOOL bWait)
	{
		return ::GetOverlappedResult(hFile, lpOverlapped, lpNumberOfBytesTransferred, bWait);
	}

	SBCLANGAUTOSETTINGS_API ::BOOL WINAPI WriteFile(::HANDLE hFile, ::LPCVOID lpBuffer, ::DWORD nNumberOfBytesToWrite, ::LPDWORD lpNumberOfBytesWritten, ::LPOVERLAPPED lpOverlapped)
	{
		return ::WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	}

	SBCLANGAUTOSETTINGS_API ::BOOL WINAPI ReadFile(::HANDLE hFile, ::LPVOID lpBuffer, ::DWORD nNumberOfBytesToRead, ::LPDWORD lpNumberOfBytesRead, ::LPOVERLAPPED lpOverlapped)
	{
		return ::ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	}

	SBCLANGAUTOSETTINGS_API ::BOOL WINAPI QueryPerformanceCounter(::LARGE_INTEGER* Cycles)
	{
		return ::QueryPerformanceCounter(Cycles);
	}

	SBCLANGAUTOSETTINGS_API ::DWORD WINAPI GetCurrentThreadId()
	{
		return ::GetCurrentThreadId();
	}

	SBCLANGAUTOSETTINGS_API ::DWORD WINAPI TlsAlloc()
	{
		return ::TlsAlloc();
	}

	SBCLANGAUTOSETTINGS_API ::LPVOID WINAPI TlsGetValue(::DWORD dwTlsIndex)
	{
		return ::TlsGetValue(dwTlsIndex);
	}

	SBCLANGAUTOSETTINGS_API ::BOOL WINAPI TlsSetValue(::DWORD dwTlsIndex, ::LPVOID lpTlsValue)
	{
		return ::TlsSetValue(dwTlsIndex, lpTlsValue);
	}

	SBCLANGAUTOSETTINGS_API ::BOOL WINAPI TlsFree(::DWORD dwTlsIndex)
	{
		return ::TlsFree(dwTlsIndex);
	}

	SBCLANGAUTOSETTINGS_API ::DWORD WINAPI FlsAlloc(::PFLS_CALLBACK_FUNCTION lpCallback)
	{
		return ::FlsAlloc(lpCallback);
	}

	SBCLANGAUTOSETTINGS_API ::LPVOID WINAPI FlsGetValue(::DWORD dwFlsIndex)
	{
		return ::FlsGetValue(dwFlsIndex);
	}

	SBCLANGAUTOSETTINGS_API ::BOOL WINAPI FlsSetValue(::DWORD dwFlsIndex, ::LPVOID lpFlsValue)
	{
		return ::FlsSetValue(dwFlsIndex, lpFlsValue);
	}

	SBCLANGAUTOSETTINGS_API ::BOOL WINAPI FlsFree(::DWORD dwTlsIndex)
	{
		return ::FlsFree(dwTlsIndex);
	}

	SBCLANGAUTOSETTINGS_API ::BOOL WINAPI IsProcessorFeaturePresent(::DWORD ProcessorFeature)
	{
		return ::IsProcessorFeaturePresent(ProcessorFeature);
	}
} // namespace Windows

#endif
