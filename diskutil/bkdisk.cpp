//
// bkdisk.exe -- utility to format, read, write and verify
// floppy disks for BK-0010/BK-0011 microcomputers.
//
// Copyright (C) 2006 Serge Vakulenko
//
// Based on sources of Demo Disk Utility by Simon Owen <simon@simonowen.com>
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <stdlib.h>

#include "fdrawcmd.h"		// from http://simonowen.com/fdrawcmd/fdrawcmd.h

int DISK_TRACKS	= 80;		// tracks per side
#define DISK_SIDES	2	// sides per disk
#define DISK_SECTORS	10	// sectors per track
#define DISK_DATARATE	2	// 2 is double-density
#define SECTOR_SIZE_CODE 2	// 0=128, 1=256, 2=512, 3=1024, ...
#define SECTOR_GAP3	0x2e	// gap3 size between sectors for format
#define SECTOR_FILL	0xff	// fill byte for formatted sectors
#define SECTOR_BASE	1	// first sector number on track

#define TRACK_SIZE	((128<<SECTOR_SIZE_CODE)*DISK_SECTORS)

///////////////////////////////////////////////////////////////////////////////

DWORD dwRet;

bool CmdRead (HANDLE h, BYTE cyl, BYTE head, BYTE start,
	BYTE count, BYTE size, PVOID pv)
{
	FD_READ_WRITE_PARAMS rwp = { FD_OPTION_MFM, head, cyl, head,
		start, size, start+count, 0x0a, 0xff };

	return DeviceIoControl(h, IOCTL_FDCMD_READ_DATA, &rwp, sizeof(rwp),
		pv, count * (128 << rwp.size), &dwRet, NULL) != 0;
}

bool CmdWrite (HANDLE h, BYTE cyl, BYTE head, BYTE start,
	BYTE count, BYTE size, PVOID pv)
{
	FD_READ_WRITE_PARAMS rwp = { FD_OPTION_MFM, head, cyl, head,
		start, size, start+count, 0x0a, 0xff };

	return DeviceIoControl(h, IOCTL_FDCMD_WRITE_DATA, &rwp, sizeof(rwp),
		pv, count * (128 << rwp.size), &dwRet, NULL) != 0;
}

bool CmdVerify (HANDLE h, BYTE cyl, BYTE head, BYTE start,
	BYTE end, BYTE size)
{
	FD_READ_WRITE_PARAMS rwp = { FD_OPTION_MFM, head, cyl, head,
		start, size, end, 0x0a, 0xff };

	return DeviceIoControl(h, IOCTL_FDCMD_VERIFY, &rwp, sizeof(rwp),
		NULL, 0, &dwRet, NULL) != 0;
}

bool CmdFormat (HANDLE h, PFD_FORMAT_PARAMS pfp, ULONG ulSize)
{
	return DeviceIoControl(h, IOCTL_FDCMD_FORMAT_TRACK, pfp,
		ulSize, NULL, 0, &dwRet, NULL) != 0;
}

bool SetDataRate (HANDLE h, BYTE bDataRate)
{
	return DeviceIoControl(h, IOCTL_FD_SET_DATA_RATE, &bDataRate,
		sizeof(bDataRate), NULL, 0, &dwRet, NULL) != 0;
}

///////////////////////////////////////////////////////////////////////////////

HANDLE OpenFloppy (int nDrive)
{
	char szDevice[32];
	HANDLE h;

	wsprintf(szDevice, "\\\\.\\fdraw%u", nDrive);
	h = CreateFile(szDevice, GENERIC_READ|GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE)
		return NULL;
	if (! SetDataRate(h, DISK_DATARATE))
		return NULL;
	return h;
}

HANDLE OpenImage (LPCSTR pcsz, bool fWrite)
{
	HANDLE h;

	if (fWrite)
		h = CreateFile(pcsz, GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, 0, NULL);
	else
		h = CreateFile(pcsz, GENERIC_READ, 0, NULL,
			OPEN_EXISTING, 0, NULL);

	return (h != INVALID_HANDLE_VALUE) ? h : NULL;
}

bool IsFloppy (LPCSTR pcsz, int *pnDrive=NULL)
{
	if (! pcsz[0] || pcsz[1] != ':' || pcsz[2])
		return false;

	if (pcsz[0] >= 'a' && pcsz[0] <= 'z') {
		if (pnDrive)
			*pnDrive = pcsz[0] - 'a';
	} else if (pcsz[0] >= 'A' && pcsz[0] <= 'Z') {
		if (pnDrive)
			*pnDrive = pcsz[0] - 'A';
	} else
		return false;

	return true;
}

DWORD GetDriverVersion ()
{
	DWORD dwVersion = 0;
	HANDLE h;

	h = CreateFile("\\\\.\\fdrawcmd", GENERIC_READ|GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, 0, NULL);
	if (h != INVALID_HANDLE_VALUE) {
		DeviceIoControl(h, IOCTL_FDRAWCMD_GET_VERSION, NULL, 0,
			&dwVersion, sizeof(dwVersion), &dwRet, NULL);
		CloseHandle(h);
	}
	return dwVersion;
}

void WriteCon (const char* pcsz, ...)
{
	char sz[1024];

	va_list args;
	va_start(args, pcsz);
	wvsprintf(sz, pcsz, args);
	va_end(args);

	WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), sz, lstrlen(sz),
		&dwRet, NULL);
}

const char* LastError ()
{
	static char sz[256];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
		sz, sizeof(sz), NULL);
	CharToOem(sz, sz);
	return GetLastError() ? sz : "no error";
}

///////////////////////////////////////////////////////////////////////////////

bool ReadTrack (HANDLE h, BYTE cyl, BYTE head, PBYTE pb)
{
	WriteCon("\rReading track %u/%u side %u", cyl, DISK_TRACKS, head);
	return CmdRead(h, cyl, head, SECTOR_BASE, DISK_SECTORS,
		SECTOR_SIZE_CODE, pb);
}

bool WriteTrack (HANDLE h, BYTE cyl, BYTE head, PBYTE pb)
{
	WriteCon("\rWriting    track %u/%u side %u", cyl, DISK_TRACKS, head);
	return CmdWrite(h, cyl, head, SECTOR_BASE, DISK_SECTORS,
		SECTOR_SIZE_CODE, pb);
}

bool VerifyTrack (HANDLE h, BYTE cyl, BYTE head)
{
	WriteCon("\rVerifying  track %u/%u side %u", cyl, DISK_TRACKS, head);
	return CmdVerify(h, cyl, head, SECTOR_BASE,
		SECTOR_BASE + DISK_SECTORS - 1, SECTOR_SIZE_CODE);
}

bool FormatTrack (HANDLE h, BYTE cyl, BYTE head)
{
	BYTE abFormat [sizeof(FD_FORMAT_PARAMS) + sizeof(FD_ID_HEADER)*DISK_SECTORS];

	PFD_FORMAT_PARAMS pfp = (PFD_FORMAT_PARAMS)abFormat;
	pfp->flags = FD_OPTION_MFM;
	pfp->phead = head;
	pfp->size = SECTOR_SIZE_CODE;
	pfp->sectors = DISK_SECTORS;
	pfp->gap = SECTOR_GAP3;
	pfp->fill = SECTOR_FILL;

	PFD_ID_HEADER ph = pfp->Header;

	for (BYTE s = 0; s < pfp->sectors; s++, ph++) {
		ph->cyl = cyl;
		ph->head = head;
		ph->sector = SECTOR_BASE + (s % DISK_SECTORS);
		ph->size = pfp->size;
	}
	WriteCon("\rFormatting track %u/%u side %u", cyl, DISK_TRACKS, head);
	return CmdFormat(h, pfp, (PBYTE)ph - abFormat);
}

///////////////////////////////////////////////////////////////////////////////

void Usage ()
{
	WriteCon("BK-0010 Disk Utility by Serge Vakulenko <vak@cronyx.ru>\n"
		"Based on driver fdrawcmd.sys from http://simonowen.com/fdrawcmd/\n"
		"Usage:\n"
		"\tbkdisk image.bkd A: [-f] [-v]\n"
		"\tbkdisk A: image.bkd [-81]\n"
		"\tbkdisk --format A: [-81] [-v]\n"
		"\tbkdisk --verify A: [-81]\n"
		"Options:\n"
		"\t-81\t81 tracks on disk (default 80 tracks)\n"
		"\t-f\tdo not format before write\n"
		"\t-v\tdo not verify after write\n"
	);
	exit(EXIT_FAILURE);
}

enum {
	cmdNone=0, cmdRead, cmdWrite, cmdFormat, cmdVerify
};

bool DoCommand (HANDLE h, HANDLE hfile,
	int nCommand, bool fVerify, bool fFormat)
{
	PBYTE pbTrack = (PBYTE)VirtualAlloc(NULL, TRACK_SIZE,
		MEM_COMMIT, PAGE_READWRITE);

	WriteCon("Geometry %d cylinders %d heads %d sectors per track\n",
		DISK_TRACKS, DISK_SIDES, DISK_SECTORS);
	for (BYTE cyl = 0; cyl < DISK_TRACKS; cyl++) {
		for (BYTE head = 0; head < DISK_SIDES; head++) {
			if (! DeviceIoControl(h, IOCTL_FDCMD_SEEK,
			    &cyl, sizeof(cyl), NULL, 0, &dwRet, NULL))
				return false;

			switch (nCommand) {
			case cmdFormat:
				if (! FormatTrack(h, cyl, head)) {
format_failed:				WriteCon("\n\nFormat failed: %s\n",
						LastError());
					return false;
				}
				if (fVerify && ! VerifyTrack(h, cyl, head))
					goto verify_failed;
				break;

			case cmdRead:
				if (! ReadTrack(h, cyl, head, pbTrack)) {
					WriteCon("\n\nRead failed: %s\n", LastError());
					return false;
				}
				WriteFile(hfile, pbTrack, TRACK_SIZE,
					&dwRet, NULL);
				break;

			case cmdWrite:
				if (fFormat && ! FormatTrack(h, cyl, head))
					goto format_failed;
				if (ReadFile(hfile, pbTrack, TRACK_SIZE,
				    &dwRet, NULL) &&
				    ! WriteTrack(h, cyl, head, pbTrack)) {
					WriteCon("\n\nWrite failed: %s\n", LastError());
					return false;
				}
				if (fVerify && ! VerifyTrack(h, cyl, head))
					goto verify_failed;
				break;

			case cmdVerify:
				if (! VerifyTrack(h, cyl, head)) {
verify_failed:				WriteCon("\n\nVerify failed: %s\n", LastError());
					return false;
				}
				break;

			}
		}
	}

	if (pbTrack)
		VirtualFree(pbTrack, 0, MEM_RELEASE);

	switch (nCommand) {
	case cmdFormat:
		WriteCon("\rFormat complete              \n");
		break;
	case cmdRead:
		WriteCon("\rImage created successfully   \n");
		break;
	case cmdWrite:
		WriteCon("\rImage written successfully   \n");
		break;
	case cmdVerify:
		WriteCon("\rDisk verified successfully   \n");
		break;
	}
	return true;
}

int main (int argc, char *argv[])
{
	int nCommand = cmdNone, nDrive = -1;
	bool fFormat = true, fVerify = true;
	LPCSTR pcszFile = NULL;
	HANDLE h, hfile = 0;

	for (int i = 1; i < argc; i++) {
		if (! lstrcmpi(argv[i], "--format") && nCommand == cmdNone)
			nCommand = cmdFormat;
		else if (! lstrcmpi(argv[i], "--verify") &&
		    nCommand == cmdNone)
			nCommand = cmdVerify;
		else if (! lstrcmpi(argv[i], "-81"))
			DISK_TRACKS = 81;
		else if (! lstrcmpi(argv[i], "-f"))
			fFormat = false;
		else if (! lstrcmpi(argv[i], "-v"))
			fVerify = false;
		else if (argv[i][0] == '/')
			Usage();
		else if (nDrive == -1 && IsFloppy(argv[i], &nDrive) &&
		    nCommand == cmdNone)
			nCommand = pcszFile ? cmdWrite : cmdRead;
		else if (! pcszFile && ! IsFloppy(argv[i]) &&
		    (pcszFile = argv[i]) && nCommand == cmdNone)
			nCommand = (nDrive == -1) ? cmdWrite : cmdRead;
	}

	if (nCommand == cmdNone || nDrive == -1) {
		// require a command and a drive
		Usage();
	} else if (!pcszFile && (nCommand == cmdRead || nCommand == cmdWrite)) {
		// read/write require image file
		Usage();
	}
	DWORD dwVersion = GetDriverVersion();

	if (!dwVersion)
		WriteCon("fdrawcmd.sys is not installed, see: http://simonowen.com/fdrawcmd/\n");
	else if (HIWORD(dwVersion) != HIWORD(FDRAWCMD_VERSION))
		WriteCon("The installed fdrawcmd.sys is not compatible with this utility.\n");
	else if (! (h = OpenFloppy(nDrive)))
		WriteCon("Failed to open floppy: %s\n", LastError());
	else if (! DeviceIoControl(h, IOCTL_FD_RESET, NULL, 0, NULL,
	    0, &dwRet, NULL))
		WriteCon("Failed to initialise controller: %s\n", LastError());
	else if (pcszFile &&
	    ! (hfile = OpenImage(pcszFile, (nCommand == cmdRead))))
		WriteCon("Failed to open image: %s\n", LastError());
	else {
		switch (nCommand) {
		case cmdFormat:
			WriteCon("Format floppy %c:\n", nDrive + 'A');
			break;
		case cmdRead:
			WriteCon("Read floppy %c: to file \"%s\"\n",
				nDrive + 'A', pcszFile);
			break;
		case cmdWrite:
			WriteCon("Write file \"%s\" to floppy %c:\n",
				pcszFile, nDrive + 'A');
			if (GetFileSize(hfile, NULL) >
			    (unsigned) DISK_TRACKS * DISK_SIDES * TRACK_SIZE)
				DISK_TRACKS = 81;
			break;
		case cmdVerify:
			WriteCon("Verify floppy %c:\n", nDrive + 'A');
			break;
		}
		if (DoCommand (h, hfile, nCommand, fVerify, fFormat))
			return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}
