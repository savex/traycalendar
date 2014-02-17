#include "StdAfx.h"
#include ".\systemtrayicon.h"

void SystemTrayIcon::SetTip (LPSTR lpszTip)
{
if (lpszTip)
	lstrcpyn (nid.szTip, lpszTip, sizeof (nid.szTip));
else
	nid.szTip[0] = '\0';
}

BOOL SystemTrayIcon::FAdd (HWND hwnd, UINT uID, HICON hicon, LPSTR lpszTip, UINT msg)
{
if (fIsPlaced) return FALSE;

if (msg) {
	nid.cbSize = sizeof (NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = uID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = msg;
	nid.hIcon = hicon;
	SetTip (lpszTip);

	fIsPlaced = Shell_NotifyIcon (NIM_ADD, &nid);
}
else
	fIsPlaced = FALSE;
if (hicon)
	DestroyIcon (hicon);

return fIsPlaced;
}

BOOL SystemTrayIcon::FChangeIcon (HICON icon)
{
if (!fIsPlaced) return FALSE;

HICON oldIcon = nid.hIcon;

nid.hIcon = icon;
nid.uFlags = NIF_ICON;

BOOL res = Shell_NotifyIcon(NIM_MODIFY, &nid);

if (!res)
nid.hIcon = oldIcon;

return res;
}

BOOL SystemTrayIcon::FChangeTip (LPSTR lpszTip)
{
if (!fIsPlaced)
return FALSE;

char oldTip[sizeof (nid.szTip)];
if (nid.szTip)
lstrcpyn (oldTip, nid.szTip, sizeof (nid.szTip));
else
oldTip[0] = '\0';

SetTip (lpszTip);
nid.uFlags = NIF_TIP;

BOOL res = Shell_NotifyIcon(NIM_MODIFY, &nid);

if (!res)
SetTip (oldTip);

return res;
}

BOOL SystemTrayIcon::FDelete ()
{
if (!fIsPlaced)
return FALSE;
fIsPlaced =!Shell_NotifyIcon(NIM_DELETE, &nid);

return fIsPlaced;
}

