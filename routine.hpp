// routine++
// routine++
// Copyright (c) 2012-2019 Henry++

#pragma once

#ifndef _APP_NO_WINXP
#undef PSAPI_VERSION
#define PSAPI_VERSION 1
#endif // _APP_NO_WINXP

#include <windows.h>
#include <wtsapi32.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <shellscalingapi.h>
#include <commctrl.h>
#include <psapi.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <time.h>
#include <lm.h>
#include <process.h>
#include <winhttp.h>
#include <subauth.h>
#include <sddl.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <algorithm>

#include "app.hpp"
#include "ntapi.hpp"
#include "rconfig.hpp"
#include "rstring.hpp"

// libs
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "wtsapi32.lib")

// stdlib typedef
typedef std::vector<rstring> rstringvec;

typedef std::unordered_map<rstring, rstring, rstring::hash, rstring::is_equal> rstringmap1;
typedef std::unordered_map<rstring, rstringmap1, rstring::hash, rstring::is_equal> rstringmap2;

// callback functions
typedef bool (*_R_CALLBACK_HTTP_DOWNLOAD) (DWORD total_written, DWORD total_length, LONG_PTR lpdata);
typedef void (*_R_CALLBACK_OBJECT_CLEANUP) (PVOID pdata);

// memory allocation/cleanup
#ifndef SAFE_DELETE
#define SAFE_DELETE(p) {if(p) {delete (p); (p)=nullptr;}}
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) {if(p) {delete[] (p); (p)=nullptr;}}
#endif

#ifndef SAFE_DELETE_OBJECT
#define SAFE_DELETE_OBJECT(p) {if(p) {DeleteObject (p); (p)=nullptr;}}
#endif

#ifndef SAFE_LOCAL_FREE
#define SAFE_LOCAL_FREE(p) {if(p) {LocalFree (p); (p)=nullptr;}}
#endif

#ifndef SAFE_GLOBAL_FREE
#define SAFE_GLOBAL_FREE(p) {if(p) {GlobalFree (p); (p)=nullptr;}}
#endif

#ifndef SAFE_HEAP_FREE
#define SAFE_HEAP_FREE(h,p) {if(p) {HeapFree (h, 0, p); (p)=nullptr;}}
#endif

/*
	Definitions
*/

#define _R_DEBUG_HEADER L"Date,Function,Code,Description,Version\r\n"
#define _R_DEBUG_BODY L"\"%s()\",\"0x%.8" PRIx32 "\",\"%s\""

#define _R_DEVICE_COUNT 0x1A
#define _R_DEVICE_PREFIX_LENGTH 64

#define _R_STR_MAX_LENGTH (INT_MAX - 1)

/*
	Unit conversion
*/

#define _R_BYTESIZE_KB (1024UL)
#define _R_BYTESIZE_MB (1024UL * _R_BYTESIZE_KB)

#define _R_SECONDSCLOCK_MSEC (1000)
#define _R_SECONDSCLOCK_MIN(mins)(60 * (mins))
#define _R_SECONDSCLOCK_HOUR(hours)((_R_SECONDSCLOCK_MIN (1) * 60) * (hours))
#define _R_SECONDSCLOCK_DAY(days)((_R_SECONDSCLOCK_HOUR (1) * 24) * (days))

/*
	Percentage calculation
*/

#define _R_PERCENT_OF(length, total_length) INT(ceil (double (length) / double (total_length) * 100.0))
#define _R_PERCENT_VAL(percent, total_length) INT(double (total_length) * double (percent) / 100.0)

/*
	Rectangle
*/

#define _R_RECT_WIDTH(lprect) ((lprect)->right - (lprect)->left)
#define _R_RECT_HEIGHT(lprect) ((lprect)->bottom - (lprect)->top)

/*
	Debugging
*/

#define RDBG(a, ...) _r_dbg_print (a, __VA_ARGS__)

void _r_dbg (LPCWSTR fn, DWORD errcode, LPCWSTR desc);
void _r_dbg_print (LPCWSTR text, ...);
void _r_dbg_write (LPCWSTR path, LPCWSTR text);

rstring _r_dbg_getpath ();

/*
	Format strings, dates, numbers
*/

rstring _r_fmt (LPCWSTR text, ...);

rstring _r_fmt_date (const LPFILETIME ft, DWORD flags = FDTF_DEFAULT); // see SHFormatDateTime flags definition
rstring _r_fmt_date (time_t ut, DWORD flags = FDTF_DEFAULT);

rstring _r_fmt_size64 (LONG64 bytes);
rstring _r_fmt_interval (time_t seconds, INT digits);

/*
	FastLock is a port of FastResourceLock from PH 1.x.
	The code contains no comments because it is a direct port. Please see FastResourceLock.cs in PH
	1.x for details.
	The fast lock is around 7% faster than the critical section when there is no contention, when
	used solely for mutual exclusion. It is also much smaller than the critical section.
	https://github.com/processhacker2/processhacker
*/

#define _R_FASTLOCK_OWNED 0x1
#define _R_FASTLOCK_EXCLUSIVE_WAKING 0x2

#define _R_FASTLOCK_SHARED_OWNERS_SHIFT 2
#define _R_FASTLOCK_SHARED_OWNERS_MASK 0x3ff
#define _R_FASTLOCK_SHARED_OWNERS_INC 0x4

#define _R_FASTLOCK_SHARED_WAITERS_SHIFT 12
#define _R_FASTLOCK_SHARED_WAITERS_MASK 0x3ff
#define _R_FASTLOCK_SHARED_WAITERS_INC 0x1000

#define _R_FASTLOCK_EXCLUSIVE_WAITERS_SHIFT 22
#define _R_FASTLOCK_EXCLUSIVE_WAITERS_MASK 0x3ff
#define _R_FASTLOCK_EXCLUSIVE_WAITERS_INC 0x400000

#define _R_FASTLOCK_EXCLUSIVE_MASK (_R_FASTLOCK_EXCLUSIVE_WAKING | (_R_FASTLOCK_EXCLUSIVE_WAITERS_MASK << _R_FASTLOCK_EXCLUSIVE_WAITERS_SHIFT))

typedef struct _R_FASTLOCK
{
	~_R_FASTLOCK ()
	{
		if (ExclusiveWakeEvent)
		{
			NtClose (ExclusiveWakeEvent);
			ExclusiveWakeEvent = nullptr;
		}

		if (SharedWakeEvent)
		{
			NtClose (SharedWakeEvent);
			SharedWakeEvent = nullptr;
		}

	}

	volatile ULONG Value = 0;

	HANDLE ExclusiveWakeEvent = nullptr;
	HANDLE SharedWakeEvent = nullptr;
} R_FASTLOCK, *P_FASTLOCK;

FORCEINLINE DWORD _r_fastlock_getspincount ()
{
	SYSTEM_INFO si = {0};
	GetNativeSystemInfo (&si);

	return  (si.dwNumberOfProcessors > 1) ? 4000 : 0;
}

FORCEINLINE void _r_fastlock_ensureeventcreated (PHANDLE phandle)
{
	HANDLE handle;

	if (*phandle != nullptr)
		return;

	NtCreateSemaphore (&handle, SEMAPHORE_ALL_ACCESS, nullptr, 0, MAXLONG);

	if (InterlockedCompareExchangePointer (phandle, handle, nullptr) != nullptr)
		NtClose (handle);
}

FORCEINLINE bool _r_fastlock_islocked (const P_FASTLOCK plock)
{
	bool owned;

	// Need two memory barriers because we don't want the compiler re-ordering the following check
	// in either direction.
	MemoryBarrier ();
	owned = (plock->Value & _R_FASTLOCK_OWNED);
	MemoryBarrier ();

	return owned;
}

void _r_fastlock_initialize (P_FASTLOCK plock);

void _r_fastlock_acquireexclusive (P_FASTLOCK plock);
void _r_fastlock_acquireshared (P_FASTLOCK plock);

void _r_fastlock_releaseexclusive (P_FASTLOCK plock);
void _r_fastlock_releaseshared (P_FASTLOCK plock);

bool _r_fastlock_tryacquireexclusive (P_FASTLOCK plock);
bool _r_fastlock_tryacquireshared (P_FASTLOCK plock);

/*
	Objects reference
*/

typedef struct _R_OBJECT
{
	PVOID pdata = nullptr;

	_R_CALLBACK_OBJECT_CLEANUP cleanup_callback = nullptr;

	volatile LONG ref_count = 0;
} R_OBJECT, *PR_OBJECT;

PR_OBJECT _r_obj_allocate (PVOID pdata, _R_CALLBACK_OBJECT_CLEANUP cleanup_callback);
PR_OBJECT _r_obj_reference (PR_OBJECT pobj);
void _r_obj_dereference (PR_OBJECT pobj);
void _r_obj_dereferenceex (PR_OBJECT pobj, LONG ref_count);

/*
	System messages
*/

INT _r_msg (HWND hwnd, DWORD flags, LPCWSTR title, LPCWSTR main, LPCWSTR text, ...);
bool _r_msg_taskdialog (const TASKDIALOGCONFIG *ptd, INT *pbutton, INT *pradiobutton, BOOL *pcheckbox); // vista TaskDialogIndirect
HRESULT CALLBACK _r_msg_callback (HWND hwnd, UINT msg, WPARAM, LPARAM lparam, LONG_PTR lpdata);

/*
	Clipboard operations
*/

rstring _r_clipboard_get (HWND hwnd);
void _r_clipboard_set (HWND hwnd, LPCWSTR text, SIZE_T length);

/*
	Filesystem
*/

#define _r_fs_exists(path) (!!RtlDoesFileExists_U ((path)))
#define _r_fs_copy(path_from,path_to,flags) (!!CopyFileEx ((path_from),(path_to),nullptr,nullptr,nullptr,(flags)))
#define _r_fs_move(path_from,path_to,flags) (!!MoveFileEx ((path_from),(path_to),(flags)))

bool _r_fs_delete (LPCWSTR path, bool allowundo);
bool _r_fs_makebackup (LPCWSTR path, time_t timestamp);
bool _r_fs_mkdir (LPCWSTR path);
bool _r_fs_readfile (HANDLE hfile, LPVOID result, DWORD64 size);
void _r_fs_rmdir (LPCWSTR path, bool is_recurse);
bool _r_fs_setpos (HANDLE hfile, LONG64 pos, DWORD method);
LONG64 _r_fs_size (HANDLE hfile);
LONG64 _r_fs_size (LPCWSTR path);

/*
	Paths
*/

rstring _r_path_getdirectory (LPCWSTR path);
LPCWSTR _r_path_getextension (LPCWSTR path);
LPCWSTR _r_path_getfilename (LPCWSTR path);
void _r_path_explore (LPCWSTR path);
rstring _r_path_compact (LPCWSTR path, UINT length);
rstring _r_path_expand (LPCWSTR path);
rstring _r_path_unexpand (LPCWSTR path);
rstring _r_path_makeunique (LPCWSTR path);
rstring _r_path_dospathfromnt (LPCWSTR path);
DWORD _r_path_ntpathfromdos (rstring &path);

/*
	Strings
*/

FORCEINLINE bool _r_str_isempty (LPCWSTR text)
{
	return !text || !*text;
}

bool _r_str_isnumeric (LPCWSTR text);

bool _r_str_alloc (LPWSTR *pbuffer, size_t length, LPCWSTR text);

void _r_str_cat (LPWSTR buffer, size_t length, LPCWSTR text, size_t max_length = _R_STR_MAX_LENGTH);
void _r_str_copy (LPWSTR buffer, size_t length, LPCWSTR text, size_t max_length = _R_STR_MAX_LENGTH);
size_t _r_str_length (LPCWSTR text, size_t max_length = _R_STR_MAX_LENGTH);
void _r_str_printf (LPWSTR buffer, size_t length, LPCWSTR text, ...);
void _r_str_vprintf (LPWSTR buffer, size_t length, LPCWSTR text, va_list args);

size_t _r_str_hash (LPCWSTR text);

INT _r_str_compare (LPCWSTR str1, LPCWSTR str2, size_t length = INVALID_SIZE_T);
INT _r_str_compare_logical (LPCWSTR str1, LPCWSTR str2);
INT _r_str_compare_unicode (LPWSTR str1, LPWSTR str2, bool is_ignorecase);

rstring _r_str_fromguid (const GUID &lpguid);
rstring _r_str_fromsid (const PSID lpsid);

size_t _r_str_find (LPCWSTR text, size_t length, WCHAR char_find, size_t start_pos = 0);
size_t _r_str_reversefind (LPCWSTR text, size_t length, WCHAR char_find, size_t start_pos = 0);

bool _r_str_match (LPCWSTR text, LPCWSTR pattern);
void _r_str_replace (LPWSTR text, WCHAR char_from, WCHAR char_to);
void _r_str_trim (rstring& text, LPCWSTR trim);

#define _r_str_lower RtlDowncaseUnicodeChar
#define _r_str_upper RtlUpcaseUnicodeChar

void _r_str_tolower (LPWSTR text);
void _r_str_toupper (LPWSTR text);

rstring _r_str_extract (LPCWSTR text, size_t length, size_t start_pos, size_t extract_length = INVALID_SIZE_T);
rstring& _r_str_extract_ref (rstring& text, size_t start_pos, size_t extract_length = INVALID_SIZE_T);

bool _r_str_multibyte2widechar (UINT cp, LPCSTR in_text, LPWSTR out_text, size_t length);

void _r_str_split (LPCWSTR text, size_t length, WCHAR delimiter, rstringvec& rvc);
bool _r_str_unserialize (LPCWSTR text, WCHAR delimeter, WCHAR key_delimeter, rstringmap1 * lpresult);

INT _r_str_versioncompare (LPCWSTR v1, LPCWSTR v2);

/*
	System information
*/

bool _r_sys_isadmin ();
rstring _r_sys_getsessioninfo (WTS_INFO_CLASS info);
rstring _r_sys_getusernamesid (LPCWSTR domain, LPCWSTR username);
bool _r_sys_iswow64 ();
bool _r_sys_setprivilege (LPCWSTR privileges[], size_t count, bool is_enable);
bool _r_sys_uacstate ();
bool _r_sys_validversion (DWORD major, DWORD minor, DWORD build = 0, BYTE condition = VER_GREATER_EQUAL);

FORCEINLINE DWORD _r_sys_gettickcount ()
{
#ifdef _WIN64

	return (DWORD)((USER_SHARED_DATA->TickCountQuad * USER_SHARED_DATA->TickCountMultiplier) >> 24);

#else

	ULARGE_INTEGER tickCount;

	while (true)
	{
		tickCount.HighPart = (DWORD)USER_SHARED_DATA->TickCount.High1Time;
		tickCount.LowPart = USER_SHARED_DATA->TickCount.LowPart;

		if (tickCount.HighPart == (DWORD)USER_SHARED_DATA->TickCount.High2Time)
			break;

		YieldProcessor ();
	}

	return (DWORD)((UInt32x32To64 (tickCount.LowPart, USER_SHARED_DATA->TickCountMultiplier) >> 24) +
				   UInt32x32To64 ((tickCount.HighPart << 8) & 0xffffffff, USER_SHARED_DATA->TickCountMultiplier));

#endif
}

FORCEINLINE DWORD64 _r_sys_gettickcount64 ()
{
	ULARGE_INTEGER tickCount;

#ifdef _WIN64

	tickCount.QuadPart = USER_SHARED_DATA->TickCountQuad;

#else

	while (true)
	{
		tickCount.HighPart = (DWORD)USER_SHARED_DATA->TickCount.High1Time;
		tickCount.LowPart = USER_SHARED_DATA->TickCount.LowPart;

		if (tickCount.HighPart == (DWORD)USER_SHARED_DATA->TickCount.High2Time)
			break;

		YieldProcessor ();
	}

#endif

	return (UInt32x32To64 (tickCount.LowPart, USER_SHARED_DATA->TickCountMultiplier) >> 24) +
		(UInt32x32To64 (tickCount.HighPart, USER_SHARED_DATA->TickCountMultiplier) << 8);
}

/*
	Unixtime
*/

time_t _r_unixtime_now ();
void _r_unixtime_to_filetime (time_t ut, const LPFILETIME pft);
void _r_unixtime_to_systemtime (time_t ut, const LPSYSTEMTIME pst);
time_t _r_unixtime_from_filetime (const FILETIME *pft);
time_t _r_unixtime_from_systemtime (const LPSYSTEMTIME pst);

/*
	Device context (Draw/Calculation etc...)
*/

void _r_dc_enablenonclientscaling (HWND hwnd);
INT _r_dc_getdpivalue (HWND hwnd, INT new_value = 0);
COLORREF _r_dc_getcolorbrightness (COLORREF clr);
COLORREF _r_dc_getcolorshade (COLORREF clr, INT percent);
void _r_dc_fillrect (HDC hdc, const LPRECT lprc, COLORREF clr);
LONG _r_dc_fontwidth (HDC hdc, LPCWSTR text, size_t length);

FORCEINLINE INT _r_dc_getdpi (HWND hwnd, INT scale)
{
	return MulDiv (scale, _r_dc_getdpivalue (hwnd), USER_DEFAULT_SCREEN_DPI);
}

FORCEINLINE INT _r_dc_fontheighttosize (HWND hwnd, INT height)
{
	return MulDiv (-height, 72, _r_dc_getdpivalue (hwnd));
}

FORCEINLINE INT _r_dc_fontsizetoheight (HWND hwnd, INT size)
{
	return -MulDiv (size, _r_dc_getdpivalue (hwnd), 72);
}

/*
	Window management
*/

void _r_wnd_addstyle (HWND hwnd, INT ctrl_id, LONG_PTR mask, LONG_PTR stateMask, INT index);
void _r_wnd_adjustwindowrect (HWND hwnd, LPRECT lprect);
void _r_wnd_centerwindowrect (LPRECT lprect, const LPRECT lpparent);
void _r_wnd_center (HWND hwnd, HWND hparent);
void _r_wnd_changemessagefilter (HWND hwnd, UINT msg, DWORD action);
void _r_wnd_toggle (HWND hwnd, bool is_show);
void _r_wnd_top (HWND hwnd, bool is_enable);
bool _r_wnd_undercursor (HWND hwnd);
bool _r_wnd_isfullscreenmode ();
void _r_wnd_resize (HDWP *hdefer, HWND hwnd, HWND hwnd_after, INT left, INT right, INT width, INT height, UINT flags);

#ifndef _APP_NO_DARKTHEME
bool _r_wnd_isdarkmessage (LPARAM lparam);
bool _r_wnd_isdarktheme ();
void _r_wnd_setdarkframe (HWND hwnd, BOOL is_enable);
void _r_wnd_setdarktheme (HWND hwnd);
#endif // _APP_NO_DARKTHEME

/*
	Inernet access (WinHTTP)
*/

HINTERNET _r_inet_createsession (LPCWSTR useragent, LPCWSTR proxy_addr);
DWORD _r_inet_openurl (HINTERNET hsession, LPCWSTR url, LPCWSTR proxy_addr, LPHINTERNET pconnect, LPHINTERNET prequest, PDWORD ptotallength);
bool _r_inet_readrequest (HINTERNET hrequest, LPSTR buffer, DWORD length, PDWORD preaded, PDWORD ptotalreaded);
DWORD _r_inet_parseurl (LPCWSTR url, INT *scheme_ptr, LPWSTR host_ptr, LPWORD port_ptr, LPWSTR path_ptr, LPWSTR user_ptr, LPWSTR pass_ptr);
DWORD _r_inet_downloadurl (HINTERNET hsession, LPCWSTR proxy_addr, LPCWSTR url, LPVOID buffer, bool is_filepath, _R_CALLBACK_HTTP_DOWNLOAD _callback, LONG_PTR lpdata);
#define _r_inet_close(h) if(h){WinHttpCloseHandle(h);h=nullptr;}

/*
	Registry
*/

PBYTE _r_reg_querybinary (HKEY hkey, LPCWSTR value);
DWORD _r_reg_querydword (HKEY hkey, LPCWSTR value);
DWORD64 _r_reg_querydword64 (HKEY hkey, LPCWSTR value);
rstring _r_reg_querystring (HKEY hkey, LPCWSTR value);
DWORD _r_reg_querysubkeylength (HKEY hkey);
time_t _r_reg_querytimestamp (HKEY hkey);

/*
	Other
*/

HANDLE _r_createthread (_beginthreadex_proc_type proc, void *args, bool is_suspended, INT priority = THREAD_PRIORITY_NORMAL);
HICON _r_loadicon (HINSTANCE hinst, LPCWSTR name, INT size);
bool _r_parseini (LPCWSTR path, rstringmap2& pmap, rstringvec* psections);
DWORD _r_rand (DWORD min_number, DWORD max_number);
bool _r_run (LPCWSTR filename, LPCWSTR cmdline, LPCWSTR dir = nullptr, WORD show_state = SW_SHOWDEFAULT, DWORD flags = 0);
void _r_sleep (LONG64 milliseconds);

/*
	System tray
*/

bool _r_tray_create (HWND hwnd, UINT uid, UINT code, HICON hicon, LPCWSTR tooltip, bool is_hidden);
bool _r_tray_popup (HWND hwnd, UINT uid, DWORD icon_id, LPCWSTR title, LPCWSTR text);
bool _r_tray_setinfo (HWND hwnd, UINT uid, HICON hicon, LPCWSTR tooltip);
bool _r_tray_toggle (HWND hwnd, UINT uid, bool is_show);
bool _r_tray_destroy (HWND hwnd, UINT uid);

/*
	Control: common
*/

INT _r_ctrl_isradiobuttonchecked (HWND hwnd, INT start_id, INT end_id);

rstring _r_ctrl_gettext (HWND hwnd, INT ctrl_id);
void _r_ctrl_settext (HWND hwnd, INT ctrl_id, LPCWSTR text, ...);

void _r_ctrl_setbuttonmargins (HWND hwnd, INT ctrl_id);
void _r_ctrl_settabletext (HDC hdc, HWND hwnd, INT ctrl_id1, LPCWSTR text1, INT ctrl_id2, LPCWSTR text2);

HWND _r_ctrl_createtip (HWND hparent);
void _r_ctrl_settip (HWND htip, HWND hparent, INT ctrl_id, LPCWSTR text);
void _r_ctrl_settipstyle (HWND htip);
void _r_ctrl_showtip (HWND hwnd, INT ctrl_id, INT icon_id, LPCWSTR title, LPCWSTR text);

FORCEINLINE bool _r_ctrl_isenabled (HWND hwnd, INT ctrl_id)
{
	return !!IsWindowEnabled (GetDlgItem (hwnd, ctrl_id));
}

FORCEINLINE void _r_ctrl_enable (HWND hwnd, INT ctrl_id, bool is_enable)
{
	EnableWindow (GetDlgItem (hwnd, ctrl_id), is_enable);
}

/*
	Control: tab
*/

INT _r_tab_additem (HWND hwnd, INT ctrl_id, INT index, LPCWSTR text, INT image = INVALID_INT, LPARAM lparam = 0);
INT _r_tab_setitem (HWND hwnd, INT ctrl_id, INT index, LPCWSTR text, INT image = INVALID_INT, LPARAM lparam = 0);

/*
	Control: listview
*/

INT _r_listview_addcolumn (HWND hwnd, INT ctrl_id, INT column_id, LPCWSTR title, INT width, INT fmt);
INT _r_listview_addgroup (HWND hwnd, INT ctrl_id, INT group_id, LPCWSTR title, UINT align, UINT state);
INT _r_listview_additem (HWND hwnd, INT ctrl_id, INT item, INT subitem, LPCWSTR text, INT image = INVALID_INT, INT group_id = INVALID_INT, LPARAM lparam = 0);

void _r_listview_deleteallcolumns (HWND hwnd, INT ctrl_id);
void _r_listview_deleteallgroups (HWND hwnd, INT ctrl_id);
void _r_listview_deleteallitems (HWND hwnd, INT ctrl_id);

INT _r_listview_getcolumncount (HWND hwnd, INT ctrl_id);
rstring _r_listview_getcolumntext (HWND hwnd, INT ctrl_id, INT column_id);
INT _r_listview_getcolumnwidth (HWND hwnd, INT ctrl_id, INT column_id);

INT _r_listview_getitemcount (HWND hwnd, INT ctrl_id, bool list_checked = false);
LPARAM _r_listview_getitemlparam (HWND hwnd, INT ctrl_id, INT item);
rstring _r_listview_getitemtext (HWND hwnd, INT ctrl_id, INT item, INT subitem);

bool _r_listview_isitemchecked (HWND hwnd, INT ctrl_id, INT item);
bool _r_listview_isitemvisible (HWND hwnd, INT ctrl_id, INT item);

void _r_listview_redraw (HWND hwnd, INT ctrl_id, INT start_id = INVALID_INT, INT end_id = INVALID_INT);

void _r_listview_setstyle (HWND hwnd, INT ctrl_id, DWORD exstyle);
void _r_listview_setcolumn (HWND hwnd, INT ctrl_id, INT column_id, LPCWSTR text, INT width);
void _r_listview_setcolumnsortindex (HWND hwnd, INT ctrl_id, INT column_id, INT arrow);
void _r_listview_setitem (HWND hwnd, INT ctrl_id, INT item, INT subitem, LPCWSTR text, INT image = INVALID_INT, INT group_id = INVALID_INT, LPARAM lparam = 0);
void _r_listview_setitemcheck (HWND hwnd, INT ctrl_id, INT item, bool state);
void _r_listview_setgroup (HWND hwnd, INT ctrl_id, INT group_id, LPCWSTR title, UINT state, UINT state_mask);

/*
	Control: treeview
*/

HTREEITEM _r_treeview_additem (HWND hwnd, INT ctrl_id, LPCWSTR text, HTREEITEM hparent = nullptr, INT image = INVALID_INT, LPARAM lparam = 0);
LPARAM _r_treeview_getlparam (HWND hwnd, INT ctrl_id, HTREEITEM item);
void _r_treeview_setitem (HWND hwnd, INT ctrl_id, HTREEITEM hitem, LPCWSTR text, INT image = INVALID_INT, LPARAM lparam = 0);
void _r_treeview_setstyle (HWND hwnd, INT ctrl_id, DWORD exstyle, INT height);

/*
	Control: statusbar
*/

void _r_status_settext (HWND hwnd, INT ctrl_id, INT idx, LPCWSTR text);
void _r_status_setstyle (HWND hwnd, INT ctrl_id, INT height);

/*
	Control: toolbar
*/

void _r_toolbar_addbutton (HWND hwnd, INT ctrl_id, UINT command_id, INT style, INT_PTR text = 0, INT state = 0, INT image = I_IMAGENONE);
INT _r_toolbar_getwidth (HWND hwnd, INT ctrl_id);
void _r_toolbar_setbutton (HWND hwnd, INT ctrl_id, UINT command_id, LPCWSTR text, INT style, INT state = 0, INT image = I_IMAGENONE);
void _r_toolbar_setstyle (HWND hwnd, INT ctrl_id, DWORD exstyle);

/*
	Control: progress bar
*/

void _r_progress_setmarquee (HWND hwnd, INT ctrl_id, BOOL is_enable);

