#include <windows.h>
#include <winternl.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "ntdll.lib")

/* --- NT API declarations not in winternl.h --- */

// typedef enum _FSINFOCLASS {
//     FileFsDeviceInformation = 4
// } FS_INFORMATION_CLASS_EX;

typedef struct _FILE_FS_DEVICE_INFORMATION {
    ULONG DeviceType;
    ULONG Characteristics;
} FILE_FS_DEVICE_INFORMATION;

typedef struct _FILE_NAME_INFORMATION {
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAME_INFORMATION;

#ifndef FILE_DEVICE_NAMED_PIPE
#define FILE_DEVICE_NAMED_PIPE 0x0011
#endif

#ifndef FILE_DEVICE_CONSOLE
#define FILE_DEVICE_CONSOLE 0x0050
#endif

/* CTL_CODE(FILE_DEVICE_CONSOLE, 5, METHOD_OUT_DIRECT, FILE_ANY_ACCESS) */
#define IOCTL_CONDRV_ISSUE_USER_IO 0x500016

#define CONSOLE_OP_GET_MODE 0x1000001

NTSTATUS NTAPI NtDeviceIoControlFile(
    HANDLE FileHandle,
    HANDLE Event,
    PIO_APC_ROUTINE ApcRoutine,
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    ULONG IoControlCode,
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PVOID OutputBuffer,
    ULONG OutputBufferLength
);

// NTSTATUS NTAPI NtQueryVolumeInformationFile(
//     HANDLE FileHandle,
//     PIO_STATUS_BLOCK IoStatusBlock,
//     PVOID FsInformation,
//     ULONG Length,
//     FS_INFORMATION_CLASS_EX FsInformationClass
// );

NTSTATUS NTAPI NtQueryInformationFile(
    HANDLE FileHandle,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass
);

/* --- Console driver request structures --- */

typedef struct {
    ULONG Operation;
    ULONG Size;
    DWORD Data;
} CONSOLE_GET_MODE_MSG;

typedef struct {
    ULONG Size;
    const void *Pointer;
} CONDRV_INPUT_BUFFER;

typedef struct {
    ULONG Size;
    void *Pointer;
} CONDRV_OUTPUT_BUFFER;

typedef struct {
    HANDLE Handle;
    ULONG InputBuffersLength;
    ULONG OutputBuffersLength;
    CONDRV_INPUT_BUFFER InputBuffers[1];
    CONDRV_OUTPUT_BUFFER OutputBuffers[1];
} CONDRV_REQUEST;

/* --- Helpers --- */

static HANDLE get_console_handle(void)
{
    /*
     * PEB->ProcessParameters->ConsoleHandle
     * MSVC's winternl.h has a stripped RTL_USER_PROCESS_PARAMETERS that
     * doesn't include ConsoleHandle. It's at offset 0x10 in the real struct.
     */
    PRTL_USER_PROCESS_PARAMETERS params =
        NtCurrentTeb()->ProcessEnvironmentBlock->ProcessParameters;
    return *(HANDLE *)((BYTE *)params + 0x10);
}

static BOOL is_cygwin_pty(HANDLE handle)
{
    IO_STATUS_BLOCK iosb;
    FILE_FS_DEVICE_INFORMATION device_info;
    NTSTATUS status;

    /* Check if it's a named pipe */
    status = NtQueryVolumeInformationFile(
        handle, &iosb,
        &device_info, sizeof(device_info),
        FileFsDeviceInformation);
    if (status != 0)
        return FALSE;
    if (device_info.DeviceType != FILE_DEVICE_NAMED_PIPE)
        return FALSE;

    /* Get the pipe name */
    BYTE buf[sizeof(FILE_NAME_INFORMATION) + MAX_PATH * sizeof(WCHAR)];
    memset(buf, 0, sizeof(buf));

    status = NtQueryInformationFile(
        handle, &iosb,
        buf, sizeof(buf),
        (FILE_INFORMATION_CLASS)9 /* FileNameInformation */);
    if (status != 0)
        return FALSE;

    FILE_NAME_INFORMATION *name_info = (FILE_NAME_INFORMATION *)buf;
    WCHAR *name = name_info->FileName;
    ULONG len = name_info->FileNameLength / sizeof(WCHAR);

    /* Match \msys-*-pty* or \cygwin-*-pty* */
    BOOL has_prefix = (len >= 6  && wcsncmp(name, L"\\msys-", 6) == 0) ||
                      (len >= 8  && wcsncmp(name, L"\\cygwin-", 8) == 0);

    BOOL has_pty = FALSE;
    for (ULONG i = 0; i + 3 < len; i++) {
        if (name[i]   == L'-' && name[i+1] == L'p' &&
            name[i+2] == L't' && name[i+3] == L'y') {
            has_pty = TRUE;
            break;
        }
    }

    return has_prefix && has_pty;
}

static BOOL is_tty(HANDLE file_handle)
{
    CONSOLE_GET_MODE_MSG msg;
    msg.Operation = CONSOLE_OP_GET_MODE;
    msg.Size = sizeof(DWORD);
    msg.Data = 0;

    CONDRV_REQUEST req;
    req.Handle = file_handle;
    req.InputBuffersLength = 1;
    req.OutputBuffersLength = 1;
    req.InputBuffers[0].Size = sizeof(msg);
    req.InputBuffers[0].Pointer = &msg;
    req.OutputBuffers[0].Size = sizeof(DWORD);
    req.OutputBuffers[0].Pointer = &msg.Data;

    IO_STATUS_BLOCK iosb;
    NTSTATUS status = NtDeviceIoControlFile(
        get_console_handle(),
        NULL, NULL, NULL,
        &iosb,
        IOCTL_CONDRV_ISSUE_USER_IO,
        &req, sizeof(req),
        NULL, 0);

    if (status == 0) /* STATUS_SUCCESS */
        return TRUE;
    if (status == (NTSTATUS)0xC0000008) /* STATUS_INVALID_HANDLE */
        return is_cygwin_pty(file_handle);
    return FALSE;
}

int main(void)
{
    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (is_tty(stdout_handle))
        printf("tty\n");
    else
        printf("not tty\n");
    return 0;
}
