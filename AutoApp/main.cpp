#ifndef AUTO_APP_MAIN_CPP
#define AUTO_APP_MAIN_CPP

#include <windows.h>
#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <vector>
#include <shlwapi.h>
#include <tchar.h>

#pragma comment(lib, "shlwapi.lib")

#define ID_BUTTON_START 101
#define ID_BUTTON_STOP 102

HINSTANCE hInst;
HWND hWnd;
HWND hButtonStart;
HWND hButtonStop;
bool running = false;

void ClickCenter() {
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    SetCursorPos(cursorPos.x, cursorPos.y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void RunApp(const std::wstring& appPath) {
    while (running) {
        // Запускаем приложение
        ShellExecute(NULL, L"open", appPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Кликаем 10 раз по центру окна
        for (int i = 0; i < 10; ++i) {
            ClickCenter();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Закрываем приложение (можно использовать Alt+F4)
        keybd_event(VK_MENU, 0, 0, 0);
        keybd_event(VK_F4, 0, 0, 0);
        keybd_event(VK_F4, 0, KEYEVENTF_KEYUP, 0);
        keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Ждем 3 часа
        std::this_thread::sleep_for(std::chrono::hours(3));
    }
}

std::wstring FindAppPath(const std::wstring& appName) {
    HKEY hKey;
    std::wstring path;
    std::vector<std::wstring> regPaths = {
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
        L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    };

    for (const auto& regPath : regPaths) {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD index = 0;
            TCHAR subKeyName[256];
            DWORD subKeyNameSize = sizeof(subKeyName) / sizeof(subKeyName[0]);

            while (RegEnumKeyEx(hKey, index, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                HKEY hSubKey;
                if (RegOpenKeyEx(hKey, subKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    TCHAR displayName[256];
                    DWORD displayNameSize = sizeof(displayName);
                    if (RegQueryValueEx(hSubKey, L"DisplayName", NULL, NULL, (LPBYTE)displayName, &displayNameSize) == ERROR_SUCCESS) {
                        if (_wcsicmp(displayName, appName.c_str()) == 0) {
                            TCHAR installLocation[256];
                            DWORD installLocationSize = sizeof(installLocation);
                            if (RegQueryValueEx(hSubKey, L"InstallLocation", NULL, NULL, (LPBYTE)installLocation, &installLocationSize) == ERROR_SUCCESS) {
                                path = installLocation;
                                path += L"\\";
                                path += appName + L".exe";
                                RegCloseKey(hSubKey);
                                RegCloseKey(hKey);
                                return path;
                            }
                        }
                    }
                    RegCloseKey(hSubKey);
                }
                subKeyNameSize = sizeof(subKeyName) / sizeof(subKeyName[0]);
                index++;
            }
            RegCloseKey(hKey);
        }
    }
    return L"";
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        hButtonStart = CreateWindow(L"BUTTON", L"Включить", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 50, 100, 30, hWnd, (HMENU)ID_BUTTON_START, hInst, NULL);
        hButtonStop = CreateWindow(L"BUTTON", L"Выключить", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            50, 100, 100, 30, hWnd, (HMENU)ID_BUTTON_STOP, hInst, NULL);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BUTTON_START) {
            if (!running) {
                running = true;
                std::wstring appPath = FindAppPath(L"Banana");
                if (!appPath.empty()) {
                    std::thread(RunApp, appPath).detach();
                }
                else {
                    MessageBox(hWnd, L"Приложение не найдено", L"Ошибка", MB_OK | MB_ICONERROR);
                    running = false;
                }
            }
        }
        else if (LOWORD(wParam) == ID_BUTTON_STOP) {
            running = false;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    hInst = hInstance;

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"AutoAppClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, L"Call to RegisterClassEx failed!", L"Windows Desktop Guided Tour", NULL);
        return 1;
    }

    hWnd = CreateWindow(
        L"AutoAppClass",
        L"Автоматизация приложения",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        300, 200,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd) {
        MessageBox(NULL, L"Call to CreateWindow failed!", L"Windows Desktop Guided Tour", NULL);
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

#endif // AUTO_APP_MAIN_CPP