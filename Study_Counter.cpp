#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <windows.h>
#include <commctrl.h>
#include <map>

using namespace std;
using namespace std::chrono;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void UpdateTimeDisplay(HWND hwnd);
void UpdateRecordsDisplay(HWND hwnd);

typedef struct {
    int day, month, year;
} Date;

steady_clock::time_point startTime;
duration<double> totalStudyTime = duration<double>::zero();
bool studying = false;
map<Date, double> studyRecords;

HWND hStartStopButton, hQuitButton, hTimeDisplay, hCalendar, hRecordsDisplay;

bool operator<(const Date& a, const Date& b) {
    return tie(a.year, a.month, a.day) < tie(b.year, b.month, b.day);
}

Date GetSelectedDate() {
    SYSTEMTIME st;
    SendMessage(hCalendar, MCM_GETCURSEL, 0, (LPARAM)&st);
    return {st.wDay, st.wMonth, st.wYear};
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "StudyCounterClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindow("StudyCounterClass", "Study Counter", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, 400, 400,
                             NULL, NULL, hInstance, NULL);
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hStartStopButton = CreateWindow("BUTTON", "Start", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                            20, 20, 100, 30, hwnd, (HMENU)1, NULL, NULL);
            hQuitButton = CreateWindow("BUTTON", "Quit", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                       150, 20, 100, 30, hwnd, (HMENU)2, NULL, NULL);
            hTimeDisplay = CreateWindow("STATIC", "Total Time: 0.00s", WS_VISIBLE | WS_CHILD,
                                        20, 60, 230, 30, hwnd, NULL, NULL, NULL);
            hCalendar = CreateWindowEx(0, MONTHCAL_CLASS, NULL, WS_BORDER | WS_VISIBLE | WS_CHILD,
                                       20, 100, 200, 200, hwnd, NULL, NULL, NULL);
            hRecordsDisplay = CreateWindow("STATIC", "Study Records:", WS_VISIBLE | WS_CHILD,
                                           250, 100, 120, 200, hwnd, NULL, NULL, NULL);
            SetTimer(hwnd, 1, 100, NULL);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                if (!studying) {
                    startTime = steady_clock::now();
                    studying = true;
                    SetWindowText(hStartStopButton, "Stop");
                } else {
                    auto endTime = steady_clock::now();
                    totalStudyTime += duration_cast<duration<double>>(endTime - startTime);
                    studying = false;
                    SetWindowText(hStartStopButton, "Start");
                    Date selectedDate = GetSelectedDate();
                    studyRecords[selectedDate] += totalStudyTime.count();
                    UpdateRecordsDisplay(hwnd);
                }
            } else if (LOWORD(wParam) == 2) {
                PostQuitMessage(0);
            }
            break;

        case WM_TIMER:
            if (studying) {
                UpdateTimeDisplay(hwnd);
            }
            break;

        case WM_CLOSE:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void UpdateTimeDisplay(HWND hwnd) {
    double elapsed = totalStudyTime.count();
    if (studying) {
        elapsed += duration_cast<duration<double>>(steady_clock::now() - startTime).count();
    }
    char buffer[50];
    sprintf(buffer, "Total Time: %.2fs", elapsed);
    SetWindowText(hTimeDisplay, buffer);
}

void UpdateRecordsDisplay(HWND hwnd) {
    string recordsText = "Study Records:\n";
    for (const auto& record : studyRecords) {
        char buffer[100];
        sprintf(buffer, "%02d/%02d/%04d: %.2fs\n", record.first.day, record.first.month, record.first.year, record.second);
        recordsText += buffer;
    }
    SetWindowText(hRecordsDisplay, recordsText.c_str());
}
