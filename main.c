#include <assert.h>
#include <string.h>
#include <Windows.h>
#include <shellapi.h>

#define TRAY_MESSAGE (WM_USER + 1)

char g_LastButtonText[128] = "Last button text";
int textPosX;
int textPosY;
HMENU g_TrayContextMenu;
enum TrayContextMenu
{
    ID_TRAY_OPEN = 0x1234,
    ID_TRAY_EXIT
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));

            RECT textRect = {
                .left = textPosX,
                .top = textPosY,
                .right = textPosX + 128,
                .bottom = textPosY + 32
            };
            DrawText(hdc, g_LastButtonText, strlen(g_LastButtonText), &textRect, DT_LEFT);

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);
            switch (wmId)
            {
                case BN_CLICKED:
                {
                    GetWindowText((HWND)lParam, g_LastButtonText, 128);
                    InvalidateRect(hwnd, NULL, 0);
                    break;
                }
                case ID_TRAY_OPEN:
                {
                    ShowWindow(hwnd, SW_SHOW);
                    SetForegroundWindow(hwnd);
                    break;
                }
                case ID_TRAY_EXIT:
                {
                    DestroyWindow(hwnd);
                    break;
                }
                default:
                    return DefWindowProc(hwnd, uMsg, wParam, lParam);
            }
            break;
        }
        case TRAY_MESSAGE:
        {
            switch(lParam)
            {
                case WM_LBUTTONDBLCLK:
                {
                    ShowWindow(hwnd, SW_SHOW);
                    SetForegroundWindow(hwnd);
                    break;
                }
                case WM_RBUTTONUP:
                {
                    POINT curPoint;
                    GetCursorPos(&curPoint);
                    UINT clicked = TrackPopupMenu(g_TrayContextMenu, 0, curPoint.x, curPoint.y, 0, hwnd, NULL);
                    break;
                }
                default:
                {
                    return DefWindowProc(hwnd, uMsg, wParam, lParam);
                }
            };
            break;
        }
        case WM_CLOSE:
        {
            ShowWindow(hwnd, SW_HIDE);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = "Test Win32UI class";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,          // Optional window styles.
        wc.lpszClassName,
        "Win32 UI test",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, // Position
        400, 400,
        NULL,       // Parent window
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );
    assert(hwnd);

    int posX = 10;
    int posY = 10;

    DWORD buttonStyle = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON;
    HWND btn = CreateWindow("BUTTON", "Set A", buttonStyle, posX, posY, 128, 32, hwnd, NULL, hInstance, NULL);
    assert(btn);

    posY += 32 + 10;

    HWND btn2 = CreateWindow("BUTTON", "Set B", buttonStyle, posX, posY, 128, 32, hwnd, NULL, hInstance, NULL);
    assert(btn);

    posY += 32 + 10;

    textPosX = posX;
    textPosY = posY;

    ShowWindow(hwnd, nCmdShow);

    //----------
    // Tray icon
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uVersion = NOTIFYICON_VERSION;
    nid.uCallbackMessage = TRAY_MESSAGE;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy(nid.szTip, "Tray Icon");
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

    Shell_NotifyIcon(NIM_ADD, &nid);

    g_TrayContextMenu = CreatePopupMenu();
    AppendMenu(g_TrayContextMenu, MF_STRING,    ID_TRAY_OPEN, "Open");
    AppendMenu(g_TrayContextMenu, MF_SEPARATOR, 0,            NULL  );
    AppendMenu(g_TrayContextMenu, MF_STRING,    ID_TRAY_EXIT, "Exit");

    MSG msg = {};
    while (1)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD))
        {
            switch (msg.message)
            {
                case WM_QUIT:
                    return 0;
                default:
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                    break;
            }
        }
    }

    return 0;
}