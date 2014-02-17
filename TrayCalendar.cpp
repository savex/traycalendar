// TrayCalendar.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include "stdafx.h"
#include "TrayCalendar.h"
#include "systemtrayicon.h"

#define MAX_LOADSTRING 100
#define TRAYICON 1
#define WSIZEX 450
#define WSIZEY 192
#define WSCALE 1
#define HK_MINIMIZE 101
#define HK_QUIT 102
#define HK_RESTART 103

#define TM_CLOCK 104
#define TM_REDRAW 105

#define BLACK RGB(0,0,0) //Черный
#define RED RGB(180,0,0) //Красный
#define RED_DARK RGB(180,100,100) //Красный
#define MONTH_COLOR RGB(230,235,230) //Фон названий месяцев
#define TMONTH_COLOR RGB(30,55,30) //Текст названий месяцев
#define W_COLOR RGB(215,220,215) //Фон названий дней недели
#define DAYBG_COLOR RGB(45,90,40) //Фон для текущего дня
#define DAY_COLOR RGB(245,255,245) //Текст текущего дня
#define LINES_COLOR RGB(210,210,210) //Разделительные линии
#define BG_COLOR RGB(255,255,255) //Общий фон
#define TEXT_COLOR RGB(35,80,35) //Обычный текст
#define BG_COLOR_DARK RGB(240,240,202) //Общий фон, темный
#define TEXT_COLOR_DARK RGB(115,140,115) //Обычный текст, серый
#define SELECTED_DAY_COLOR RGB(179,188,179) //Фон выбранного мышкой дня

//My Global
HWND hWnd;
bool bwinState = FALSE;
bool msTrack = FALSE;
bool bRestart = false;
bool bQuit = false;
bool bRepainted=false;
HICON hIcon = NULL;
SYSTEMTIME time;
TIME_ZONE_INFORMATION timezone;
unsigned int iHourZone;
HBRUSH brBg, brCDay, brDay, brCDayBg, brLn, brW, brBlack, brRed, brMonth, brSelected;
HBRUSH brBgD, brDayD;
HFONT sysFont,fntMonth,fntDays, fntTime;
RECT rcDay, rcDays, rcClient, rcExit, rcMini, rcTime, rcWeekLeft, rcWeekRight, rcMonth;
RECT pRcMonth[12];
RECT pDt; //Desktop

//Structures
//Config
struct stCONFIG
{
	UINT HintDelayTime;
	UINT HintHideTime;
	UINT RedrawTimer;
} config;

//Time
struct stTIME
{
	WORD wYear;
	WORD wMonth;
	WORD wDAY;
	WORD wHour;
	WORD wMinute;
};

//Event
struct stEVENT
{
	stTIME schTime; //Время срабатывания
	char note[256]; //Описание
	WORD wSound;    //Звук срабатывания
};

//Each day
struct stDAY
{
	int iDay; //День
	char strDay[3]; //День (строка)
	int iWeekDay; //День недели
	int iMonth; //Месяц
	bool bHoliday; //Праздник?
	char HolidayNote[128]; //Коментарий к празднику
	RECT rcDay; //квадрат текущего дня на экране
	HBRUSH DayBrush; //Браш для заливки фона
	COLORREF bkColor; //Цвет фона
	COLORREF textColor; //Цвет текста
	bool bUpdate; //Необходимо обновить при следующей перерисовке
	bool bSelected; //Над этим днем мышь?
} Day;

//Экран
struct stSCREEN
{
	WORD wSizeX;
	WORD wSizeY;
	WORD wColumns;
	WORD wMonth;
	WORD wBorderX;
	WORD wBorderY;
} Screen;

//Подсказка
struct stHINT
{
	RECT rcHint;
	int xPos;
	int yPos;
	bool bShow;
	bool bInvalidated;
	UINT iDelay;
} hint, dtHint;

int weekDay[] = {6,0,1,2,3,4,5};
int colMonth[] = {31,31,28,31,30,31,30,31,31,30,31,30,31,31,28};
char wNames[7][3] = {("Пн"),("Вт"),("Ср"),("Чт"),("Пт"),("Сб"),("Вс")};
char wNamesBig[7][15] = {(" понедельник"),(" вторник"),(" среда"),(" четверг"),(" пятница"),(" суббота"),(" воскресенье")};
char sMonth[12][25] = {(" января "),(" февраля "),(" марта "),(" апреля "),(" мая "),(" июня "),(" июля "),(" августа "),(" сентября "),(" октября "),(" ноября "),(" декабря ")};
char month[15][25];
int day, xPos, yPos;
stDAY pDayMap[256];
int iStartDay,iStartMonth,iSelectedDay=0;

SystemTrayIcon* ptrayIcon;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

//My forwards
bool CleanUp(HWND hWnd);
bool CleanForRestart(HWND hWnd, HINSTANCE hInstance);
bool DrawLns(HDC hDC);
void CalculateMonth(WORD num);
bool RegistryTrack();
bool RegistryUpdate();
void DrawHint(HDC hDC,WORD xPos,WORD yPos,COLORREF bgColor, COLORREF textColor,char* text);
void DrawHintHolyday(HDC hDC,COLORREF bgColor, COLORREF textColor,char* text);
void DayMapUpdate(HDC hDC);
void DayMapMouseCheck();

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
				
//Чистим за собой
bool CleanUp(HWND hWnd)
{
	//Удаляем браши
	DeleteObject(brBg);
	DeleteObject(brBgD);
	DeleteObject(brCDay);
	DeleteObject(brCDayBg);
	DeleteObject(brDay);
	DeleteObject(brDayD);
	DeleteObject(brLn);
	DeleteObject(brMonth);
	DeleteObject(brBlack);
	DeleteObject(brRed);
	DeleteObject(brW);
	DeleteObject(brSelected);
	ptrayIcon->FDelete();
	delete ptrayIcon;
	ptrayIcon=NULL;
	UnregisterHotKey(hWnd,HK_MINIMIZE);
	UnregisterHotKey(hWnd,HK_RESTART);
	UnregisterHotKey(hWnd,HK_QUIT);
	return true;
}

//Чистим переменные и удаляем классы и окна для перезапуска
bool CleanForRestart(HWND hWnd, HINSTANCE hInstance)
{
	DeleteObject(brBg);
	DeleteObject(brBgD);
	DeleteObject(brCDay);
	DeleteObject(brCDayBg);
	DeleteObject(brDay);
	DeleteObject(brDayD);
	DeleteObject(brLn);
	DeleteObject(brMonth);
	DeleteObject(brBlack);
	DeleteObject(brRed);
	DeleteObject(brW);
	DeleteObject(brSelected);
	UnregisterHotKey(hWnd,HK_MINIMIZE);
	UnregisterHotKey(hWnd,HK_RESTART);
	UnregisterHotKey(hWnd,HK_QUIT);

	ptrayIcon->FDelete();
	delete ptrayIcon;
	ptrayIcon=NULL;
	UnregisterClass(szWindowClass,hInstance);
	DestroyWindow(hWnd);
	return true;
}

//Рисуем линии месяца по количеству столбцов
bool DrawLns(HDC hDC)
{
	RECT tm;
	tm.left=rcDays.left+5; //горизонтальные
	tm.top=rcDays.top+20;
	tm.right=rcDays.right-5;
	tm.bottom=tm.top+2;
	for(int i=1;i<7;i++) {
		FillRect(hDC,&tm,brLn);
		tm.top+=22;
		tm.bottom+=22;
	}
	
	tm.left=rcDays.left+20; //вертикальные
	tm.top=rcDays.top+5;
	tm.right=tm.left+2;
	tm.bottom=rcDays.bottom-5;
	for(int i=1;i<Screen.wColumns;i++) {
		FillRect(hDC,&tm,brLn);
		tm.left+=22;
		tm.right+=22;
	}

	return true;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
		MSG msg;
		HACCEL hAccelTable;

	do {
		GetLocalTime(&time);
		GetTimeZoneInformation(&timezone);
		iHourZone=abs(timezone.Bias+timezone.DaylightBias)/60;


		// Initialize global strings
		LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadString(hInstance, IDC_TRAYCALENDAR, szWindowClass, MAX_LOADSTRING);
		MyRegisterClass(hInstance);

		// Perform application initialization:
		if (!InitInstance (hInstance, nCmdShow)) 
		{
			return FALSE;
		}

		hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_TRAYCALENDAR);

		bRestart=false;
		// Main message loop:
		while (GetMessage(&msg, NULL, 0, 0)) 
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (bRestart) {
				RegistryUpdate();
				CleanForRestart(hWnd,hInstance);
				break;
			}
		}
	} while (!bQuit);

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= 0;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_TRAY1);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0; //(LPCTSTR)IDC_TRAYCALENDAR;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TRAY2);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
	
	//Очистим структуры
	ZeroMemory(&Day,sizeof(Day));
	ZeroMemory(&Screen,sizeof(Screen));

	//Заполним структуры
	Day.iDay=time.wDay;
	Day.iMonth=time.wMonth;
	Day.iWeekDay=time.wDayOfWeek;

	config.HintDelayTime=1000;
	config.HintHideTime=5000;
	config.RedrawTimer=50;
 
	if ((time.wYear%4)==0) {
 		//Если год делится на 4 без остатка, то он высокосный
  	   colMonth[2]=29; 
 	   colMonth[14]=29;
    }

	Screen.wSizeY=40+20*7+2*6+20; //Title+Days+TimeDisplay

	//Работаем с реестром
	RegistryTrack();
    
	//Калькулируем количество столбцов и размер экрана
	CalculateMonth(Screen.wMonth);
	Screen.wBorderX=5;
	Screen.wBorderY=35;

	//Создаем окно
   GetClientRect(GetDesktopWindow(),&pDt);

   hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUPWINDOW | WS_SYSMENU,
	   pDt.right-Screen.wSizeX-Screen.wBorderX , pDt.bottom-Screen.wSizeY-Screen.wBorderY, 
	   Screen.wSizeX, Screen.wSizeY, NULL, NULL, hInstance, NULL);
   SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, 0);
   UpdateWindow(hWnd);
   if (bRestart) {
	    ShowWindow(hWnd, SW_SHOW);
		SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		SetTimer(hWnd,TM_CLOCK,300,NULL);
		SetTimer(hWnd,TM_REDRAW,config.RedrawTimer,NULL);
   }

   //Setting tray icon
   hIcon=LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAY2));
   ptrayIcon=new SystemTrayIcon();
   ptrayIcon->FAdd(hWnd,TRAYICON,hIcon,"Календарь",WM_USER);
   RedrawWindow(hWnd, NULL, NULL, NULL);

   //Регистрируем горячую клавишу
   RegisterHotKey(hWnd,HK_MINIMIZE,MOD_WIN,0x5A); //<Win>+'Z'
   RegisterHotKey(hWnd,HK_RESTART,MOD_WIN,0x57); //<Win>+'W'
   RegisterHotKey(hWnd,HK_QUIT,MOD_WIN,0x51); //<Win>+'Q'

   //Создаем браши и все остальное
   brBg=CreateSolidBrush(BG_COLOR);
   brBgD=CreateSolidBrush(BG_COLOR_DARK);
   brW=CreateSolidBrush(W_COLOR);
   brCDayBg=CreateSolidBrush(DAYBG_COLOR);
   brCDay=CreateSolidBrush(DAY_COLOR);
   brDay=CreateSolidBrush(TEXT_COLOR);
   brDayD=CreateSolidBrush(TEXT_COLOR_DARK);
   brLn=CreateSolidBrush(LINES_COLOR);
   brBlack=CreateSolidBrush(BLACK);
   brRed=CreateSolidBrush(RED);
   brMonth=CreateSolidBrush(MONTH_COLOR);
   brSelected=CreateSolidBrush(SELECTED_DAY_COLOR);
   //Создадим шрифт
   sysFont=(HFONT)GetStockObject(SYSTEM_FIXED_FONT);

   LOGFONT lFnt;
   ZeroMemory(&lFnt,sizeof(lFnt));
   lFnt.lfHeight=-10;
   lFnt.lfWeight=700;
   lFnt.lfCharSet=204;
   lFnt.lfOutPrecision=3;
   lFnt.lfClipPrecision=2;
   lFnt.lfQuality=ANTIALIASED_QUALITY;
   lFnt.lfPitchAndFamily=34;
   strcpy_s(lFnt.lfFaceName,sizeof(lFnt.lfFaceName),"Tahoma");
   fntMonth=CreateFontIndirect(&lFnt);

   ZeroMemory(&lFnt,sizeof(lFnt));
   lFnt.lfHeight=-9;
   lFnt.lfWeight=400;
   lFnt.lfCharSet=204;
   lFnt.lfOutPrecision=3;
   lFnt.lfClipPrecision=2;
   lFnt.lfQuality=ANTIALIASED_QUALITY;
   lFnt.lfPitchAndFamily=34;
   strcpy_s(lFnt.lfFaceName,sizeof(lFnt.lfFaceName),"Verdana");
   fntTime=CreateFontIndirect(&lFnt);

   //Month
   strcpy_s(month[0],25,"Декабрь/Грудень");
   strcpy_s(month[1],25,"Январь/Сiчень");
   strcpy_s(month[2],25,"Февраль/Лютий");
   strcpy_s(month[3],25,"Март/Березень");
   strcpy_s(month[4],25,"Апрель/Квiтень");
   strcpy_s(month[5],25,"Май/Травень");
   strcpy_s(month[6],25,"Июнь/Червень");
   strcpy_s(month[7],25,"Июль/Липень");
   strcpy_s(month[8],25,"Август/Серпень");
   strcpy_s(month[9],25,"Сентябрь/Вересень");
   strcpy_s(month[10],25,"Октябрь/Жовтень");
   strcpy_s(month[11],25,"Ноябрь/Листопад");
   strcpy_s(month[12],25,"Декабрь/Грудень");
   strcpy_s(month[13],25,"Январь/Сiчень");
   strcpy_s(month[14],25,"Февраль/Лютий");

   //Регионы вывода
   GetClientRect(hWnd,&rcClient);
   SetRect(&rcExit,Screen.wSizeX-20,0,Screen.wSizeX,20); //Выход
   SetRect(&rcMini,Screen.wSizeX-20,Screen.wSizeY-20,Screen.wSizeX,Screen.wSizeY); //Минимизация
   SetRect(&rcTime,20,Screen.wSizeY-20,Screen.wSizeX-20,Screen.wSizeY);
   SetRect(&rcWeekLeft,0,40,20,Screen.wSizeY-20);
   SetRect(&rcWeekRight,Screen.wSizeX-20,40,Screen.wSizeX,Screen.wSizeY-20);
   SetRect(&rcMonth,20,0,Screen.wSizeX-20,40);
   SetRect(&rcDays,20,40,Screen.wSizeX-20,Screen.wSizeY-20);

   //Очистим переменные коотдинат мыши
   xPos=yPos=0;

   //Очистим переменные подсказки
   ZeroMemory(&hint,sizeof(stHINT));
   ZeroMemory(&dtHint,sizeof(stHINT));


   return TRUE;
}
bool DrawMonths(HDC hDC)
{
	HGDIOBJ oldBrush, oldFnt;
	COLORREF oldBkCol,oldTextCol;
	
	//Заливаем Фон
	FillRect(hDC,&rcClient,brBg);
	FillRect(hDC,&rcWeekLeft,brW);
	FillRect(hDC,&rcWeekRight,brW);
	FillRect(hDC,&rcMonth,brMonth);
	FillRect(hDC,&rcTime,brBgD);


	//Сохраняем старые значения
	oldBrush=SelectObject(hDC,brCDay);
	oldFnt=SelectObject(hDC,fntMonth);
	oldBkCol=SetBkColor(hDC,MONTH_COLOR);
	oldTextCol=SetTextColor(hDC,TMONTH_COLOR);

	//Разделительные линии
	DrawLns(hDC);
	//Рисуем названия дней недели
	SetBkColor(hDC,W_COLOR);
	SetTextColor(hDC,BLACK);
	RECT wL,wR;
	memcpy(&wL,&rcWeekLeft,sizeof(RECT));
	memcpy(&wR,&rcWeekRight,sizeof(RECT));
	wL.bottom=wL.top+20;
	wR.bottom=wR.top+20;
	for(int i=0;i<7;i++) {
		if (i==5) SetTextColor(hDC,RED);
		DrawText(hDC,wNames[i],(int)strlen(wNames[i]),&wL,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		DrawText(hDC,wNames[i],(int)strlen(wNames[i]),&wR,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		wL.top+=22;wL.bottom+=22;
		wR.top+=22;wR.bottom+=22;
	}
	
	//Рисуем карту дней
	RECT rcCurDay;
	int iCurDay = pDayMap->iDay;
	int iCurMonth = pDayMap->iMonth;
	int iCurWeekDay = pDayMap->iWeekDay;
	if (iCurMonth%2>0) {
		SetBkColor(hDC,BG_COLOR);
		SetTextColor(hDC,TEXT_COLOR_DARK);
	}
	else {
		SetBkColor(hDC,BG_COLOR_DARK);
		SetTextColor(hDC,TEXT_COLOR_DARK);
	}
	rcCurDay.top=rcDays.top;
	rcCurDay.left=rcDays.left;
	rcCurDay.bottom=rcCurDay.top+20;
	rcCurDay.right=rcCurDay.left+20;

	for (int j=0;j<Screen.wColumns;j++) {
		iCurWeekDay=0;
		do {
			int tmp=j*7+iCurWeekDay; //индекс обрабатываемого дня в карте
			//Рисуем день
			if (pDayMap[tmp].iMonth%2>0) { //нечетный месяц
				SetBkColor(hDC,BG_COLOR);
				SetTextColor(hDC,TEXT_COLOR_DARK);
				FillRect(hDC,&rcCurDay,brBg);
				pDayMap[tmp].DayBrush=brBg; //Запомним браш
			}
			else { //четный месяц
				SetBkColor(hDC,MONTH_COLOR);
				SetTextColor(hDC,TEXT_COLOR_DARK);
				FillRect(hDC,&rcCurDay,brMonth);
				pDayMap[tmp].DayBrush=brMonth; //Запомним браш
			}
			if ((iCurWeekDay==5) || (iCurWeekDay==6)) SetTextColor(hDC,RED_DARK);

			if (pDayMap[tmp].iMonth==time.wMonth) {
				SetTextColor(hDC,TEXT_COLOR);
				if ((iCurWeekDay==5) || (iCurWeekDay==6)) SetTextColor(hDC,RED);
			}
			//Также запомним цвета, использованные для рисования этого дня
			pDayMap[tmp].bkColor=GetBkColor(hDC);
			pDayMap[tmp].textColor=GetTextColor(hDC);

			if ((time.wDay==pDayMap[tmp].iDay) && (time.wMonth==pDayMap[tmp].iMonth)) {
				SetBkColor(hDC,DAYBG_COLOR);
				SetTextColor(hDC,DAY_COLOR);
				FillRect(hDC,&rcCurDay,brCDayBg);
				DrawText(hDC,pDayMap[tmp].strDay,(int)strlen(pDayMap[tmp].strDay),&rcCurDay,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
			else {
				DrawText(hDC,pDayMap[tmp].strDay,(int)strlen(pDayMap[tmp].strDay),&rcCurDay,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
			//Запомним рамку соответствующего дня в карте дней
			memcpy(&pDayMap[tmp].rcDay,&rcCurDay,sizeof(RECT)); 
			//Перейдем к следующему дню в столбце
			rcCurDay.top+=22;
			rcCurDay.bottom=rcCurDay.top+20;

			//Следующий день недели
			iCurWeekDay++;
		} while (iCurWeekDay<7);
		//Следующий столбец
		rcCurDay.top=rcDays.top;
		rcCurDay.left+=22;
		rcCurDay.bottom=rcCurDay.top+20;
		rcCurDay.right=rcCurDay.left+20;
	}
	
	//Рисуем названия месяцев
	int i=time.wMonth-Screen.wMonth/2;
	if (i<1) i+=12;
	rcMonth.bottom=20;
	FillRect(hDC,&rcMonth,brMonth);
	rcMonth.bottom=40;
	rcMonth.top=20;
	FillRect(hDC,&rcMonth,brBg);
	rcMonth.top=0;
	for (WORD j=0;j<Screen.wMonth;j++) {
		if ((i%2)>0) { //нечетный месяц
			FillRect(hDC,pRcMonth+j,brBg);
			SetBkColor(hDC,BG_COLOR);
			SetTextColor(hDC,TMONTH_COLOR);
			DrawText(hDC,month[i],(int)strlen(month[i]),pRcMonth+j,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		else { //четный
			FillRect(hDC,pRcMonth+j,brMonth);
			SetBkColor(hDC,MONTH_COLOR);
			SetTextColor(hDC,TMONTH_COLOR);
			DrawText(hDC,month[i],(int)strlen(month[i]),pRcMonth+j,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		i++;
		if (i>12) i%=12;
	}

	//Востанавливаем старые значения
	SelectObject(hDC,oldBrush);
	SelectObject(hDC,oldFnt);
	SetBkColor(hDC,oldBkCol);
	SetTextColor(hDC,oldTextCol);

	//All is Ok!
	return true;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent,i;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	//получаем сообщения из трея
	case WM_USER:
		switch (lParam)
		{
		case WM_LBUTTONDOWN:

			break;
		case WM_LBUTTONDBLCLK:

			break;
		case WM_RBUTTONDOWN:
			bwinState=!bwinState;
			if (bwinState) {
				ShowWindow(hWnd, SW_SHOW);
				SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	            SetTimer(hWnd,TM_CLOCK,300,NULL);
				SetTimer(hWnd,TM_REDRAW,config.RedrawTimer,NULL);
			}
			else {
				ShowWindow(hWnd, SW_HIDE);
				KillTimer(hWnd,TM_CLOCK);
				KillTimer(hWnd,TM_REDRAW);
			}
			break;
		}
		break;
	//команды меню
	case WM_LBUTTONDOWN:
		xPos = (int) lParam & 0x0000ffff; 
		yPos = (int) (lParam & 0xffff0000) >> 16;

		if ((xPos>rcExit.left) && (xPos<rcExit.right))
			if ((yPos>rcExit.top) && (yPos<rcExit.bottom))  {
				KillTimer(hWnd,TM_CLOCK);
				KillTimer(hWnd,TM_REDRAW);
				CleanUp(hWnd);
				PostQuitMessage(0);
				bQuit=true;
		}
		if ((xPos>rcMini.left) && (xPos<rcMini.right))
			if ((yPos>rcMini.top) && (yPos<rcMini.bottom))  {
				bwinState=FALSE;
				ShowWindow(hWnd, SW_HIDE);
				KillTimer(hWnd,TM_CLOCK);
				KillTimer(hWnd,TM_REDRAW);
		}
		break;
	case WM_MOUSEMOVE:
		xPos = (int) lParam & 0x0000ffff; 
		yPos = (int) (lParam & 0xffff0000) >> 16;
		COLORREF oldText, oldBk;
		HGDIOBJ oldFont;
		POINT ptMouse;

		ptMouse.x = xPos;
		ptMouse.y = yPos;

		hdc=GetDC(hWnd);
		oldText=SetTextColor(hdc,DAYBG_COLOR);
		oldBk=SetBkColor(hdc,BG_COLOR);
		oldFont=SelectObject(hdc,fntMonth);
		if (msTrack && ((xPos>WSIZEX) || (yPos>WSIZEY) || (xPos<0) || (yPos<0))) {
            FillRect(hdc,&rcExit,brBg);
            FillRect(hdc,&rcMini,brBg);
			ReleaseCapture();
			msTrack=FALSE;
		}
		if ((xPos>rcExit.left) && (xPos<rcExit.right))
			if ((yPos>rcExit.top) && (yPos<rcExit.bottom))  {
				char str[2]="X";
				DrawText(hdc,str,1,&rcExit,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				if (!msTrack) {
					SetCapture(hWnd);
					msTrack=TRUE;
				}
			}
			else FillRect(hdc,&rcExit,brBg);
		else FillRect(hdc,&rcExit,brBg);

		if ((xPos>rcMini.left) && (xPos<rcMini.right))
			if ((yPos>rcMini.top) && (yPos<rcMini.bottom))  {
				char str[2]="C";
				DrawText(hdc,str,1,&rcMini,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				if (!msTrack) {
					SetCapture(hWnd);
					msTrack=TRUE;
				}
			}
			else FillRect(hdc,&rcMini,brBg);
		else FillRect(hdc,&rcMini,brBg);

		SetTextColor(hdc,oldText);
		SetBkColor(hdc,oldBk);
		SelectObject(hdc,oldFont);

		//Проверка на прохождение мышки по карте дней
		DayMapMouseCheck();

		//Обрабатываем подсказки
		if (PtInRect(&rcMonth,ptMouse)) hint.bShow=true;
		else hint.bShow=false;

		ReleaseDC(hWnd,hdc);

		break;
	case WM_KEYUP:
		i=(int)(lParam>>16)&0x000000ff;
		
		switch (wParam) {
			case 0x21: //PageUp
				Screen.wMonth++;
				if (Screen.wMonth>7) Screen.wMonth=7;
				break;
			case 0x22: //PageDown
				Screen.wMonth--;
				if (Screen.wMonth<3) Screen.wMonth=3;
				break;
		}
		break;
	case WM_CHAR:
//		i=(int)(lParam>>16)&0x000000ff;
		break;
	case WM_HOTKEY:
		switch (wParam) {
			case HK_MINIMIZE:
				bwinState=!bwinState;
				if (bwinState) {
					ShowWindow(hWnd, SW_SHOW);
					SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
					SetTimer(hWnd,TM_CLOCK,300,NULL);
					SetTimer(hWnd,TM_REDRAW,config.RedrawTimer,NULL);
				}
				else {
					ShowWindow(hWnd, SW_HIDE);
					KillTimer(hWnd,TM_CLOCK);
					KillTimer(hWnd,TM_REDRAW);
				}
				break;
			case HK_RESTART:
				KillTimer(hWnd,TM_CLOCK);
				KillTimer(hWnd,TM_REDRAW);
				bRestart=true;
				break;
			case HK_QUIT:
				KillTimer(hWnd,TM_CLOCK);
				KillTimer(hWnd,TM_REDRAW);
				CleanUp(hWnd);
				PostQuitMessage(0);
				bQuit=true;
				break;
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_CCMINIMIZE:
			break;
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			KillTimer(hWnd,TM_CLOCK);
			KillTimer(hWnd,TM_REDRAW);
			CleanUp(hWnd);
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		//Собственно календарь
		DrawMonths(hdc);
		
		EndPaint(hWnd, &ps);
		bRepainted=true;
		break;
	case WM_TIMER:
		//Обработка таймера
		switch (wParam) {
			case TM_CLOCK:
				if (bwinState) {
					GetLocalTime(&time);
					hdc=GetDC(hWnd);
					char strTime[10];
					char strHour[3], strMin[3], strSec[3];
					int l;
					_itoa_s(time.wHour,strHour,3,10);
					_itoa_s(time.wMinute,strMin,3,10);
					_itoa_s(time.wSecond,strSec,3,10);

					if ((int)strlen(strMin)==1) {
						strMin[1]=strMin[0];
						strMin[0]='0';
						strMin[2]=0;
					}
					if ((int)strlen(strSec)==1) {
						strSec[1]=strSec[0];
						strSec[0]='0';
						strSec[2]=0;
					}
					strcpy_s(strTime,10,strHour);
					l=(int)strlen(strTime);
					strTime[l++]=':';strTime[l]=0;
					strcpy_s(strTime+l,10-l,strMin);
					l=(int)strlen(strTime);
					strTime[l++]=':';strTime[l]=0;
					strcpy_s(strTime+l,10-l,strSec);

					char str[128];
					char strDate[30];
					_itoa_s(time.wDay,strDate,30,10);
					strcpy_s(strDate+strlen(strDate),30-strlen(strDate),sMonth[time.wMonth-1]);
					_itoa_s(time.wYear,strDate+strlen(strDate),30-strlen(strDate),10);
					strcpy_s(strDate+strlen(strDate),30-strlen(strDate)," года,");
					strcpy_s(str,30,strDate);
					strcpy_s(str+strlen(str),128-strlen(str),wNamesBig[weekDay[time.wDayOfWeek]]);
					int u=(int)strlen(str);
					str[u]=',';
					str[u+1]=' ';
					str[u+2]=0;
					strcat_s(str,128,strTime);
					SelectObject(hdc,fntTime);
					SetBkColor(hdc,W_COLOR);
					SetTextColor(hdc,BLACK);
					FillRect(hdc,&rcTime,brW);
					DrawText(hdc,str,(int)strlen(str),&rcTime,DT_CENTER | DT_VCENTER | DT_SINGLELINE);

						//Нарисуем цифру которая соответствует количеству месяцев
					_itoa_s(Screen.wMonth,str,128,10);
					SetBkColor(hdc,BG_COLOR);
					SetTextColor(hdc,BLACK);
					SelectObject(hdc,fntMonth);
					RECT rcMonthNum;
					rcMonthNum.top=20;
					rcMonthNum.bottom=40;
					rcMonthNum.left=Screen.wSizeX-20;
					rcMonthNum.right=Screen.wSizeX;
					DrawText(hdc,str,(int)strlen(str),&rcMonthNum,DT_CENTER | DT_VCENTER | DT_SINGLELINE);

					//Отладочная информация
/*
					_itoa_s(dtHint.iDelay,str,128,10);
					DrawText(hdc,str,(int)strlen(str),&rcTime,DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
*/
					ReleaseDC(hWnd,hdc);
				}
				break;
			case TM_REDRAW:
				hdc=GetDC(hWnd);
				//Обновляем дни по флагам в карте дней
				DayMapUpdate(hdc);
				
				//Подсказки о праздниках, а также другие...
				if (hint.bShow) DrawHint(hdc,xPos+15,yPos+10,BG_COLOR,BLACK,"Подсказка!");
				if (dtHint.bShow) DrawHintHolyday(hdc,BG_COLOR,RED,"Праздник. Описание праздника.");

				//Считаем время
				if (hint.bShow && hint.iDelay<=config.HintHideTime) 
					hint.iDelay+=config.RedrawTimer;
				if (dtHint.bShow && dtHint.iDelay<=config.HintHideTime) 
					dtHint.iDelay+=config.RedrawTimer;

				ReleaseDC(hWnd,hdc);
				break;
		}
		break;
	case WM_DESTROY:
		if (!bRestart) {
			CleanUp(hWnd);
			PostQuitMessage(0);
			bQuit=true;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void CalculateMonth(WORD num)
{
	int ii,jj,kk;
	int mm[12]; //Массив с количеством дней в месяцах на текущей карте дней :-)

	kk=(int)num/2; //Количество месяцев в каждую сторону (влево и вправо)
	ii=(Day.iMonth-kk)-1;
	if (ii<1) ii+=12;
	iStartMonth=ii; //Первый месяц на карте дней
	for(jj=0;jj<=(num+1);jj++) {
		mm[jj]=colMonth[ii++];
		if (ii>12) ii-=12;
	}
	//Текущий месяц из общего количества - прошлый от текущего
	kk=(int)(num/2); //3 -> 1; 5 -> 2; 7 -> 3;

//Вычислим количество недель/столбцов
	//Перемещаемся в начало карты
	ii=Day.iDay-weekDay[Day.iWeekDay]; //День который был в начале недели
	do {
		do ii-=7; while (ii>1); //Вычитаем пока число больше 1, т.е. перемещаемся в начало месяца
		ii=mm[kk--]+ii;
	} while (kk>=0);

	//Сохраним значение начала первого месяца на карте. Позже будем заполнять карту дней.
	iStartDay=ii;
	jj=1;
	//В начале. Тек. месяц 0-й из num. Считаем столбцы.
	kk=0;
	do {
		ii+=7;
		if (ii>mm[kk]) ii-=mm[kk++];
		if (kk>num) break; //Если мы добрались до последнего месяца, выход
		else jj++;
	} while (jj<64); //Это фальшивка, чтобы из цыкла не выскакивало

	Screen.wColumns=jj; //Запоминаем количество столбцов
	Screen.wSizeX=2*20+(jj*22-2);

//Заполняем карту дней в соответствии с количеством посчитанных столбцов
	//Создаем карту дней
	ZeroMemory(pDayMap,sizeof(stDAY)*128);
	//Стартовый день.
	ii=iStartDay;
	int curMonth=iStartMonth;
	for (jj=0;jj<Screen.wColumns;jj++) {
		kk=0;
		do {
			if (ii>colMonth[curMonth]) {
				ii-=colMonth[curMonth++];
				if (curMonth>12) curMonth-=12;
			}
			if (ii>0) {
				pDayMap[jj*7+kk].iDay=ii;
				_itoa_s(ii,pDayMap[jj*7+kk].strDay,3,10);
				pDayMap[jj*7+kk].iMonth=curMonth;
				pDayMap[jj*7+kk].bHoliday=false;
				pDayMap[jj*7+kk].iWeekDay=kk+1;
			}
			else {
				pDayMap[jj*7+kk].iDay=0;
			}
			kk++;ii++;
		} while (kk<7);
	}
	//Теперь по количеству столбцов и карте дней вычислим где надо рисовать названия месяцев
	ZeroMemory(pRcMonth,sizeof(RECT)*12);
	kk=(WORD)Day.iMonth-(WORD)Screen.wMonth/2;
	if (kk<1) kk+=12;
	for (ii=0;ii<Screen.wMonth;ii++) {
		if (kk%2>0) {
			pRcMonth[ii].top=0;
			pRcMonth[ii].bottom=20;
		}
		else {
			pRcMonth[ii].top=20;
			pRcMonth[ii].bottom=40;
		}
		kk++;
		if (kk>12) kk-=12;
	}
	kk=0;
	for (ii=0;ii<(Screen.wColumns*7);ii++) {
		int tmp = (int)(ii/7)*22; //Координата текущего столбца
		if ((pDayMap[ii].iDay==1) && (((ii/7)>0) && ((ii/7)<Screen.wColumns))) {
			if ((ii%7)>0) pRcMonth[kk++].right=20+tmp+20;
			else pRcMonth[kk++].right=20+tmp;
			pRcMonth[kk].left=20+tmp;
		}
		if (ii==0) pRcMonth[kk].left=20;
		if (ii==Screen.wColumns*7-1) pRcMonth[kk].right=20+tmp+20;
	}

}

//Обработка значений в реестре, создание, обновление и т.д.
bool RegistryTrack()
{
	long err_code;
	unsigned long lRegSubKeyType,lRegSubKeyLenght,dwDisp;
	DWORD dwData=0;
	char strRegSubKeyName[] = {("MonthNumberToDisplay")};
	char strRegKeyName[] = {("Software\\TrayCalendar\\")};
	HKEY hkRegKey;

	//Сначала проверяем есть ли такой ключ в реестре,
	//потом если нету - создаем со значением по умолчанию,
	//если есть читаем значение и заносим в переменную программы

	//Проверяем
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,strRegKeyName,0,KEY_QUERY_VALUE|KEY_READ,&hkRegKey)) {
		//Ключика в реестре нету
		//Готовим данные для реестра
		Screen.wMonth=3; //Всего 3 месяца на экране по умолчанию
		dwData = Screen.wMonth;
		//Создаем, записываем значение и выходим из функции
		err_code=RegCreateKeyEx(HKEY_LOCAL_MACHINE,strRegKeyName,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkRegKey,&dwDisp);
		if (err_code) return false;
		err_code=RegSetValueEx(hkRegKey,strRegSubKeyName,0,REG_DWORD,(const BYTE *)&dwData,sizeof(dwData));
		RegCloseKey(hkRegKey);
		if (err_code) return false;
		else return true;
	}
	else {
		//Есть, читаем и заносим
		RegQueryValueEx(hkRegKey,strRegSubKeyName,0,&lRegSubKeyType,(LPBYTE)&dwData,&lRegSubKeyLenght);
		RegCloseKey(hkRegKey);
		Screen.wMonth=(WORD)dwData;
	}

	return true;

}

//Сохраняем значение в реестр
bool RegistryUpdate()
{
	long err_code;
	DWORD dwData=0;
	char strRegSubKeyName[] = {("MonthNumberToDisplay")};
	char strRegKeyName[] = {("Software\\TrayCalendar\\")};
	HKEY hkRegKey;

	//Сначала проверяем есть ли такой ключ в реестре,
	//потом если нету - выходим с ошибкой,
	//если есть заносим туда последнее значение

	//Проверяем
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,strRegKeyName,0,KEY_QUERY_VALUE|KEY_WRITE,&hkRegKey))
		return false;
	else {
		//Есть, заносим
		//Готовим данные для реестра
		dwData = Screen.wMonth;
		err_code=RegSetValueEx(hkRegKey,strRegSubKeyName,0,REG_DWORD,(const BYTE *)&dwData,sizeof(dwData));
		RegCloseKey(hkRegKey);
	}

	return true;
}

//Выводит в окне программы строку с текстом
void DrawHint(HDC hDC,WORD xPos,WORD yPos,COLORREF bgColor, COLORREF textColor,char* text)
{
	SIZE Size;
	int Border=3;
	HBRUSH bgBrush,frBrush;
	COLORREF oldBgColor,oldTxColor;

	if (hint.bShow) {
		//Координаты не изменились, ничего рисовать не надо
		if (xPos!=hint.xPos || yPos!=hint.yPos) {
			//Мышка двинулась, очищаем старое и пока не остановиться ничего рисовать не будем
			if (!hint.bInvalidated) InvalidateRect(hWnd, &hint.rcHint, FALSE);
			hint.xPos=xPos;
			hint.yPos=yPos;
			hint.bInvalidated=true;
			hint.iDelay=0;
			return;
		}
		else {
			//Если прошла секунда с момента остановки мыши - можно рисовать
			if (hint.iDelay<config.HintDelayTime || hint.iDelay>config.HintHideTime) {
				if (!hint.bInvalidated) InvalidateRect(hWnd, &hint.rcHint, FALSE);
				hint.bInvalidated=true;
				return;
			}

			//Координаты старые и новые равны - мышка не двигается, рисуем
			bgBrush=CreateSolidBrush(bgColor);
			frBrush=CreateSolidBrush(textColor);

			//Длинна строки символов
			GetTextExtentPoint32(hDC,text,(int)strlen(text),&Size);

			//Регион вывода подсказки
			hint.rcHint.top=yPos;
			hint.rcHint.bottom=hint.rcHint.top+Size.cy+Border;
			hint.rcHint.left=xPos;
			hint.rcHint.right=hint.rcHint.left+Size.cx+Border+3;

			//Запоминаем старое
			oldBgColor=SetBkColor(hDC,bgColor);
			oldTxColor=SetTextColor(hDC,textColor);
			
			//Рисуем подсказку
			FillRect(hDC,&hint.rcHint,bgBrush);
			DrawText(hDC,text,(int)strlen(text),&hint.rcHint,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			FrameRect(hDC,&hint.rcHint,frBrush);
			
			//Сохраним координату подсказки, чтобы потом можно было узнать двигалась она или нет
			hint.bInvalidated=false;
			hint.xPos=xPos;
			hint.yPos=yPos;

			//Восстанавливаем старое
			SetBkColor(hDC,oldBgColor);
			SetTextColor(hDC,oldTxColor);
			DeleteObject(bgBrush);
			DeleteObject(frBrush);
		}
	}
	else if ((hint.xPos>0) || (hint.yPos>0)) {
		InvalidateRect(hWnd, &hint.rcHint, TRUE);
	}

	return;
}

//Выводит подсказку о празднике в клиентское окно
void DrawHintHolyday(HDC hDC,COLORREF bgColor, COLORREF textColor,char* text)
{
	SIZE Size;
	int Border=3;
	HBRUSH bgBrush,frBrush;
	COLORREF oldBgColor,oldTxColor;
	
	if (dtHint.bShow) {
		//Координаты не изменились, ничего рисовать не надо
		if (xPos!=dtHint.xPos || yPos!=dtHint.yPos) {
			//Мышка двинулась, очищаем старое и пока не остановиться ничего рисовать не будем
			if (!dtHint.bInvalidated) InvalidateRect(hWnd, &dtHint.rcHint, FALSE);
			dtHint.xPos=xPos;
			dtHint.yPos=yPos;
			dtHint.bInvalidated=true;
			dtHint.iDelay=0;
			return;
		}
		else {
			//Если прошла секунда с момента остановки мыши - можно рисовать
			if (dtHint.iDelay<config.HintDelayTime || dtHint.iDelay>config.HintHideTime) {
				if (!dtHint.bInvalidated) InvalidateRect(hWnd, &dtHint.rcHint, FALSE);
				dtHint.bInvalidated=true;
				return;
			}

			//Координаты старые и новые равны - мышка не двигается, рисуем
			bgBrush=CreateSolidBrush(bgColor);
			frBrush=CreateSolidBrush(textColor);

			//Длинна строки символов
			GetTextExtentPoint32(hDC,text,(int)strlen(text),&Size);

			//Регион вывода подсказки
			//Если текущий день понедельник-четверг, то подсказка внизу
			//пятница-воскресенье - вверху
			dtHint.rcHint.right=rcDays.right;
			dtHint.rcHint.left=rcDays.left;
			
			if (iSelectedDay%7>4) {
				dtHint.rcHint.top=rcDays.top;
				dtHint.rcHint.bottom=dtHint.rcHint.top+Size.cy+Border;
			}
			else {
				dtHint.rcHint.bottom=rcDays.bottom;
				dtHint.rcHint.top=dtHint.rcHint.bottom-Size.cy-Border;
			}

			//Запоминаем старое
			oldBgColor=SetBkColor(hDC,bgColor);
			oldTxColor=SetTextColor(hDC,textColor);
			
			//Рисуем подсказку
			FillRect(hDC,&dtHint.rcHint,bgBrush);
			DrawText(hDC,text,(int)strlen(text),&dtHint.rcHint,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			FrameRect(hDC,&dtHint.rcHint,frBrush);
			
			//Сохраним координату подсказки, чтобы потом можно было узнать двигалась она или нет
			dtHint.bInvalidated=false;
			dtHint.xPos=xPos;
			dtHint.yPos=yPos;

			//Восстанавливаем старое
			SetBkColor(hDC,oldBgColor);
			SetTextColor(hDC,oldTxColor);
			DeleteObject(bgBrush);
			DeleteObject(frBrush);
		}
	}
	else if ((dtHint.xPos>0) || (dtHint.yPos>0)) {
		InvalidateRect(hWnd, &dtHint.rcHint, TRUE);
	}

	return;
}


//Обновляет карту дней на экране
void DayMapUpdate(HDC hDC)
{
	
	HGDIOBJ oldFnt = SelectObject(hDC,fntMonth);
	//Рисуем карту дней
	int iCurDay = pDayMap->iDay;
	int iCurMonth = pDayMap->iMonth;
	int iCurWeekDay = pDayMap->iWeekDay;
	if (iCurMonth%2>0) {
		SetBkColor(hDC,BG_COLOR);
		SetTextColor(hDC,TEXT_COLOR_DARK);
	}
	else {
		SetBkColor(hDC,BG_COLOR_DARK);
		SetTextColor(hDC,TEXT_COLOR_DARK);
	}

	for (int j=0;j<Screen.wColumns;j++) {
		iCurWeekDay=0;
		do {
			int tmp=j*7+iCurWeekDay; //индекс обрабатываемого дня в карте
			
			if (!pDayMap[tmp].bUpdate) {
				iCurWeekDay++;
				continue;
			}
			else pDayMap[tmp].bUpdate=false;

			if (pDayMap[tmp].bSelected) {

				if ((time.wDay==pDayMap[tmp].iDay) && (time.wMonth==pDayMap[tmp].iMonth)) {
					SetBkColor(hDC,DAY_COLOR);
					SetTextColor(hDC,DAYBG_COLOR);
					FillRect(hDC,&pDayMap[tmp].rcDay,brCDay);
					DrawText(hDC,pDayMap[tmp].strDay,(int)strlen(pDayMap[tmp].strDay),&pDayMap[tmp].rcDay,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				}
				else {
					SetBkColor(hDC,SELECTED_DAY_COLOR);
					SetTextColor(hDC,BG_COLOR);
					FillRect(hDC,&pDayMap[tmp].rcDay,brSelected);
					DrawText(hDC,pDayMap[tmp].strDay,(int)strlen(pDayMap[tmp].strDay),&pDayMap[tmp].rcDay,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				}
			}
			else {
				FillRect(hDC,&pDayMap[tmp].rcDay,pDayMap[tmp].DayBrush);

				if ((time.wDay==pDayMap[tmp].iDay) && (time.wMonth==pDayMap[tmp].iMonth)) {
					SetBkColor(hDC,DAYBG_COLOR);
					SetTextColor(hDC,DAY_COLOR);
					FillRect(hDC,&pDayMap[tmp].rcDay,brCDayBg);
					DrawText(hDC,pDayMap[tmp].strDay,(int)strlen(pDayMap[tmp].strDay),&pDayMap[tmp].rcDay,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				}
				else {
					SetBkColor(hDC,pDayMap[tmp].bkColor);
					SetTextColor(hDC,pDayMap[tmp].textColor);
					FillRect(hDC,&pDayMap[tmp].rcDay,pDayMap[tmp].DayBrush);
					DrawText(hDC,pDayMap[tmp].strDay,(int)strlen(pDayMap[tmp].strDay),&pDayMap[tmp].rcDay,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				}
			}
			//Следующий день недели
			iCurWeekDay++;
		} while (iCurWeekDay<7);
	}

	SelectObject(hDC,oldFnt);
	return;
}

//Проверка координат мышки и обновление карты соответствующим образом
void DayMapMouseCheck()
{
	POINT ptMouse = {xPos,yPos};
	//Мышь покинула пределы дня или нет?
	if (iSelectedDay!=-1 && !PtInRect(&pDayMap[iSelectedDay].rcDay,ptMouse)) {
		pDayMap[iSelectedDay].bSelected=false;
		pDayMap[iSelectedDay].bUpdate=true;
		iSelectedDay=-1;
		dtHint.bShow=false;
	}

	for(int j=0;j<Screen.wColumns*7;j++) {
		if (PtInRect(&pDayMap[j].rcDay,ptMouse)) {
			//Отмечаем новое положение мышки
			if (!pDayMap[j].bSelected) {
				pDayMap[j].bUpdate=true;
				pDayMap[j].bSelected=true;
				iSelectedDay=j;

				//Подсказка о празднике
				dtHint.bShow=true;
			}
		}
	}
	return;
}