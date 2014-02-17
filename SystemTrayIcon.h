#include <shellapi.h>
#pragma once

class SystemTrayIcon
{
BOOL fIsPlaced;
NOTIFYICONDATA nid;
void SetTip (LPSTR lpszTip);

public:

SystemTrayIcon (): fIsPlaced (FALSE) {}
~SystemTrayIcon ()
{
	if (fIsPlaced) FDelete ();
}

BOOL FAdd (HWND hwnd, UINT uID, HICON hicon, LPSTR lpszTip, UINT msg);
BOOL FChangeIcon (HICON hicon);
BOOL FChangeTip (LPSTR lpszTip);
BOOL FDelete ();

};
