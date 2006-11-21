//
// bkdisk.exe -- utility to format, read, write and verify
// floppy disks for BK-0010/BK-0011 microcomputers.
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
#define SECTOR_GAP3	0x25	// gap3 size between sectors
#define SECTOR_FILL	0xff	// fill byte for formatted sectors
#define SECTOR_BASE	1	// first sector number on track
#define TRACK_SKEW	0	// format skew to the same sector on the next track

#define TRACK_SIZE          ((128<<SECTOR_SIZE_CODE)*DISK_SECTORS)

///////////////////////////////////////////////////////////////////////////////

DWORD dwRet;

bool CmdRead (HANDLE h_, BYTE cyl_, BYTE head_, BYTE start_, BYTE count_, BYTE size_, PVOID pv_)
{
    FD_READ_WRITE_PARAMS rwp = { FD_OPTION_MFM, head_, cyl_,head_, start_,size_,start_+count_, 0x0a,0xff };
    return !!DeviceIoControl(h_, IOCTL_FDCMD_READ_DATA, &rwp, sizeof(rwp), pv_, count_*(128<<rwp.size), &dwRet, NULL);
}

bool CmdWrite (HANDLE h_, BYTE cyl_, BYTE head_, BYTE start_, BYTE count_, BYTE size_, PVOID pv_)
{
    FD_READ_WRITE_PARAMS rwp = { FD_OPTION_MFM, head_, cyl_,head_, start_,size_,start_+count_, 0x0a,0xff };
    return !!DeviceIoControl(h_, IOCTL_FDCMD_WRITE_DATA, &rwp, sizeof(rwp), pv_, count_*(128<<rwp.size), &dwRet, NULL);
}

bool CmdVerify (HANDLE h_, BYTE cyl_, BYTE head_, BYTE start_, BYTE end_, BYTE size_)
{
    FD_READ_WRITE_PARAMS rwp = { FD_OPTION_MFM, head_, cyl_,head_, start_,size_,end_, 0x0a,0xff };
    return !!DeviceIoControl(h_, IOCTL_FDCMD_VERIFY, &rwp, sizeof(rwp), NULL, 0, &dwRet, NULL);
}

bool CmdFormat (HANDLE h_, PFD_FORMAT_PARAMS pfp_, ULONG ulSize_)
{
    return !!DeviceIoControl(h_, IOCTL_FDCMD_FORMAT_TRACK, pfp_, ulSize_, NULL, 0, &dwRet, NULL);
}

bool SetDataRate (HANDLE h_, BYTE bDataRate_)
{
    return !!DeviceIoControl(h_, IOCTL_FD_SET_DATA_RATE, &bDataRate_, sizeof(bDataRate_), NULL, 0, &dwRet, NULL);
}

///////////////////////////////////////////////////////////////////////////////

HANDLE OpenFloppy (int nDrive_)
{
    char szDevice[32];
    wsprintf(szDevice, "\\\\.\\fdraw%u", nDrive_);

    HANDLE h = CreateFile(szDevice, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    return (h != INVALID_HANDLE_VALUE && SetDataRate(h, DISK_DATARATE)) ? h : NULL;
}

HANDLE OpenImage (LPCSTR pcsz_, bool fWrite_)
{
    HANDLE h = fWrite_ ? CreateFile(pcsz_, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL)
                       : CreateFile(pcsz_, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

    return (h != INVALID_HANDLE_VALUE) ? h : NULL;
}

bool IsFloppy (LPCSTR pcsz_, int *pnDrive_=NULL)
{
    if (! pcsz_[0] || pcsz_[1] != ':' || pcsz_[2])
        return false;

    if (pcsz_[0] >= 'a' && pcsz_[0] <= 'z') {
	if (pnDrive_)
	    *pnDrive_ = pcsz_[0] - 'a';
    } else if (pcsz_[0] >= 'A' && pcsz_[0] <= 'Z') {
	if (pnDrive_)
	    *pnDrive_ = pcsz_[0] - 'A';
    } else
        return false;

    return true;
}

DWORD GetDriverVersion ()
{
    DWORD dwVersion = 0;
    HANDLE h = CreateFile("\\\\.\\fdrawcmd", GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (h != INVALID_HANDLE_VALUE)
    {
        DeviceIoControl(h, IOCTL_FDRAWCMD_GET_VERSION, NULL, 0, &dwVersion, sizeof(dwVersion), &dwRet, NULL);
        CloseHandle(h);
    }

    return dwVersion;
}

void WriteCon (const char* pcsz_, ...)
{
    char sz[1024];

    va_list args;
    va_start(args, pcsz_);
    wvsprintf(sz, pcsz_, args);
    va_end(args);

    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), sz, lstrlen(sz), &dwRet, NULL);
}

const char* LastError ()
{
    static char sz[256];

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
		sz, sizeof(sz), NULL);
    CharToOem(sz, sz);
    return GetLastError() ? sz : "no error";
}

///////////////////////////////////////////////////////////////////////////////

bool ReadTrack (HANDLE h_, BYTE cyl_, BYTE head_, PBYTE pb_)
{
    return CmdRead(h_, cyl_, head_, SECTOR_BASE, DISK_SECTORS, SECTOR_SIZE_CODE, pb_);
}

bool WriteTrack (HANDLE h_, BYTE cyl_, BYTE head_, PBYTE pb_)
{
    return CmdWrite(h_, cyl_, head_, SECTOR_BASE, DISK_SECTORS, SECTOR_SIZE_CODE, pb_);
}

bool VerifyTrack (HANDLE h_, BYTE cyl_, BYTE head_)
{
    return CmdVerify(h_, cyl_, head_, SECTOR_BASE, SECTOR_BASE+DISK_SECTORS-1, SECTOR_SIZE_CODE);
}

bool FormatTrack (HANDLE h_, BYTE cyl_, BYTE head_)
{
    BYTE abFormat[sizeof(FD_FORMAT_PARAMS) + sizeof(FD_ID_HEADER)*DISK_SECTORS];

    PFD_FORMAT_PARAMS pfp = (PFD_FORMAT_PARAMS)abFormat;
    pfp->flags = FD_OPTION_MFM;
    pfp->phead = head_;
    pfp->size = SECTOR_SIZE_CODE;
    pfp->sectors = DISK_SECTORS;
    pfp->gap = SECTOR_GAP3;
    pfp->fill = SECTOR_FILL;

    PFD_ID_HEADER ph = pfp->Header;

    for (BYTE s = 0 ; s < pfp->sectors ; s++, ph++)
    {
        ph->cyl = cyl_;
        ph->head = head_;
        ph->sector = SECTOR_BASE + ((s + cyl_*(pfp->sectors - TRACK_SKEW)) % pfp->sectors);
        ph->size = pfp->size;
    }

    return CmdFormat(h_, pfp, (PBYTE)ph - abFormat);
}

///////////////////////////////////////////////////////////////////////////////

void Usage ()
{
    WriteCon("BK-0010 Disk Utility by Serge Vakulenko <vak@cronyx.ru>\n"
             "Usage:\n"
             "\tbkdisk A: image.bkd [-81]\n"
             "\tbkdisk image.bkd A: [-81] [-f] [-v]\n"
             "\tbkdisk --format A: [-81] [-v]\n"
             "\tbkdisk --verify A: [-81]\n"
             "Options:\n"
             "\t-81\t81 tracks on disk (default 80 tracks)\n"
             "\t-f\tdo not format before write\n"
             "\t-v\tdo not verify after write\n"
    );

    exit(EXIT_FAILURE);
}

int main (int argc_, char *argv_[])
{
    enum { cmdNone=0, cmdRead, cmdWrite, cmdFormat, cmdVerify };
    int nCommand = cmdNone, nDrive = -1;
    bool fFormat = true, fVerify = true;
    LPCSTR pcszFile = NULL;
    HANDLE h, hfile = 0;

    for (int i = 1 ; i < argc_ ; i++)
    {
        if (!lstrcmpi(argv_[i], "--format") && nCommand == cmdNone)
            nCommand = cmdFormat;
        else if (!lstrcmpi(argv_[i], "--verify") && nCommand == cmdNone)
            nCommand = cmdVerify;
        else if (!lstrcmpi(argv_[i], "-81"))
            DISK_TRACKS = 81;
        else if (!lstrcmpi(argv_[i], "-f"))
            fFormat = false;
        else if (!lstrcmpi(argv_[i], "-v"))
            fVerify = false;
        else if (argv_[i][0] == '/')
            Usage();
        else if (nDrive == -1 && IsFloppy(argv_[i], &nDrive) && nCommand == cmdNone)
            nCommand = pcszFile ? cmdWrite : cmdRead;
        else if (!pcszFile && !IsFloppy(argv_[i]) && (pcszFile = argv_[i]) && nCommand == cmdNone)
            nCommand = (nDrive == -1) ? cmdWrite : cmdRead;
    }

    if (nCommand == cmdNone || nDrive == -1)    // require a command and a drive
        Usage();
    else if (!pcszFile && (nCommand == cmdRead || nCommand == cmdWrite))    // read/write require image file
        Usage();

    DWORD dwVersion = GetDriverVersion();

    if (!dwVersion)
        WriteCon("fdrawcmd.sys is not installed, see: http://simonowen.com/fdrawcmd/\n");
    else if (HIWORD(dwVersion) != HIWORD(FDRAWCMD_VERSION))
        WriteCon("The installed fdrawcmd.sys is not compatible with this utility.\n");
    else if (!(h = OpenFloppy(nDrive)))
        WriteCon("Failed to open floppy: %s\n", LastError());
    else if (!DeviceIoControl(h, IOCTL_FD_RESET, NULL, 0, NULL, 0, &dwRet, NULL))
        WriteCon("Failed to initialise controller: %s\n", LastError());
    else if (pcszFile && !(hfile = OpenImage(pcszFile, (nCommand == cmdRead))))
        WriteCon("Failed to open image: %s\n", LastError());
    else if (nCommand == cmdWrite && GetFileSize(hfile, NULL) % TRACK_SIZE != 0)
        WriteCon("Image file is wrong size (should be %lu bytes)\n", TRACK_SIZE*DISK_TRACKS*DISK_SIDES);
    else
    {
        PBYTE pbTrack = (PBYTE)VirtualAlloc(NULL, TRACK_SIZE, MEM_COMMIT, PAGE_READWRITE);

        for (BYTE cyl = 0 ; cyl < DISK_TRACKS ; cyl++)
        {
            for (BYTE head = 0 ; head < DISK_SIDES ; head++)
            {
                if (!DeviceIoControl(h, IOCTL_FDCMD_SEEK, &cyl, sizeof(cyl), NULL, 0, &dwRet, NULL))
                    return false;

                switch (nCommand)
                {
                    case cmdFormat:
                        WriteCon("\rFormatting track %u/%u side %u", cyl, DISK_TRACKS, head);
                        if (FormatTrack(h, cyl, head) && (!fVerify || VerifyTrack(h, cyl, head)))
                            break;

                        WriteCon("\n\nFormat failed: %s\n", LastError());
                        return EXIT_FAILURE;

                    case cmdRead:
                        WriteCon("\rReading track %u/%u side %u", cyl, DISK_TRACKS, head);
                        if (ReadTrack(h, cyl, head, pbTrack) && WriteFile(hfile, pbTrack, TRACK_SIZE, &dwRet, NULL))
                            break;

                        WriteCon("\n\nRead failed: %s\n", LastError());
                        return EXIT_FAILURE;

                    case cmdWrite:
                        WriteCon("\rWriting track %u/%u side %u", cyl, DISK_TRACKS, head);
                        if (ReadFile(hfile, pbTrack, TRACK_SIZE, &dwRet, NULL) &&
                            (!fFormat || FormatTrack(h, cyl, head)) &&
                             WriteTrack(h, cyl, head, pbTrack) &&
                            (!fVerify || VerifyTrack(h, cyl, head)))
                            break;

                        WriteCon("\n\nWrite failed: %s\n", LastError());
                        return EXIT_FAILURE;

                    case cmdVerify:
                        WriteCon("\rVerifying track %u/%u side %u", cyl, DISK_TRACKS, head);
                        if (VerifyTrack(h, cyl, head))
                            break;

                        WriteCon("\n\nVerify failed: %s\n", LastError());
                        return EXIT_FAILURE;
                }
            }
        }

        if (pbTrack)
            VirtualFree(pbTrack, 0, MEM_RELEASE);

        switch (nCommand)
        {
            case cmdFormat: WriteCon("\rFormat complete.           \n");    break;
            case cmdRead:   WriteCon("\rImage created successfully.\n");    break;
            case cmdWrite:  WriteCon("\rImage written successfully.\n");    break;
            case cmdVerify: WriteCon("\rDisk verified successfully.\n");    break;
        }

        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
