#include "stdafx.h"
#include "sysutils.h"
#include <TlHelp32.h>

NtUnmapViewOfSectionFunc NtUnmapViewOfSection;

/*
* Mappe un fichier en m�moire, quelque soit le r�sultat c'est � l'appelant
* de fermer les handles ouvert avec unmap_file.
*
* R�f�rences :
*  - version simple https://msdn.microsoft.com/fr-fr/library/windows/desktop/aa366551(v=vs.85).aspx
*  - version ++ https://www.sysnative.com/forums/programming/21860-mapviewoffile-example-win32.html
*
* @param swFilePath : chemin vers le fichier � mapper en m�moire
* @param mFile : pointeur vers une structure de type MAPPEDFILE qui contient les handles n�cessaire au mapping,
*                cette structure doit �tre vide
* @return TRUE si l'op�ration c'est bien pass�
*         FALSE si l'op�ration c'est mal pass�
*/
BOOL MapFileFromName(LPCWSTR swFilePath, PMAPPEDFILE mFile)
{
	BOOL bSuccess = FALSE;

	memset(mFile, 0, sizeof(MAPPEDFILE));

	mFile->hFile = CreateFile(swFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (mFile->hFile != INVALID_HANDLE_VALUE)
	{
		mFile->dwFileSize = GetFileSize(mFile->hFile, NULL);
		if (mFile->dwFileSize != 0)
		{
			mFile->hMapFile = CreateFileMapping(mFile->hFile, NULL, PAGE_READONLY | SEC_COMMIT, 0, mFile->dwFileSize, NULL);
			if (mFile->hMapFile != NULL)
			{
				mFile->lpView = MapViewOfFile(mFile->hMapFile, FILE_MAP_READ, 0, 0, 0);
				if (mFile->lpView != NULL)
					bSuccess = TRUE;
				else
					fprintf(stderr, "[-] MapViewOfFile\n");
			}
			else
				fprintf(stderr, "[-] Error when CreateFileMapping\n");
		}
		else
			fprintf(stderr, "[-] Can't GetFileSize\n");
	}
	else
		fprintf(stderr, "[-] Can't OpenFile\n");

	return bSuccess;
}

/*
* "D�mappe" un fichier de la m�moire
* @param mFile : un pointeur vers une structure contenant les handles du fichier � "d�mapper"
* @return TRUE si l'op�ration c'est bien pass�e
*         FALSE s'il y a eu une erreur lors de la fermeture d'un handle
*/
BOOL UnmapFile(PMAPPEDFILE mFile)
{
	BOOL bSuccess = FALSE;
	if (mFile->lpView != NULL)
		bSuccess &= UnmapViewOfFile(mFile->lpView);
	if (mFile->hMapFile != NULL)
		bSuccess &= CloseHandle(mFile->hMapFile);
	if (mFile->hFile != NULL)
		bSuccess &= CloseHandle(mFile->hFile);
	return bSuccess;
}

/*
* R�soud les adresses des fonctions de ntdll.dll
* @return -1 si la dll n'as pas �t� trouv�e
*/
DWORD ResolveNativeAPI()
{
	DWORD dwReturnCode = 0;
	HMODULE hModNtdll = LoadLibrary(L"ntdll.dll");
	if (hModNtdll)
	{
		NtUnmapViewOfSection = (NtUnmapViewOfSectionFunc)GetProcAddress(hModNtdll, "NtUnmapViewOfSection");
	}
	else
		dwReturnCode = -1;

	return dwReturnCode;
}