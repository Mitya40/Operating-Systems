#include <windows.h>
#include <commdlg.h>
#include <vector>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct BitmapData {
    int width;
    int height;
    std::vector<BYTE> pixelData;
    BITMAPINFO bmpInfo;
};

BitmapData cachedFigure;
COLORREF backgroundColor = RGB(240, 240, 240);

bool LoadBitmapData(const wchar_t* filePath, BitmapData& bmpData) {
    FILE* file;
    if (_wfopen_s(&file, filePath, L"rb") != 0) return false;

    BITMAPFILEHEADER fileHeader;
    fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);

    BITMAPINFOHEADER infoHeader;
    fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, file);

    if (infoHeader.biCompression != BI_RGB) {
        fclose(file);
        return false;
    }

    bmpData.width = infoHeader.biWidth;
    bmpData.height = abs(infoHeader.biHeight);
    bmpData.pixelData.resize(((bmpData.width * 3 + 3) & ~3) * bmpData.height);

    bmpData.bmpInfo.bmiHeader = infoHeader;
    bmpData.bmpInfo.bmiHeader.biHeight = bmpData.height;
    bmpData.bmpInfo.bmiHeader.biCompression = BI_RGB;

    fseek(file, fileHeader.bfOffBits, SEEK_SET);
    fread(bmpData.pixelData.data(), 1, bmpData.pixelData.size(), file);
    fclose(file);

    backgroundColor = RGB(
        bmpData.pixelData[2],
        bmpData.pixelData[1],
        bmpData.pixelData[0]
    );

    return true;
}

void OpenBitmapFile(HWND hwnd) {
    OPENFILENAME ofn;
    wchar_t fileName[MAX_PATH] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Bitmap Files\0*.bmp\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        if (LoadBitmapData(fileName, cachedFigure)) {
            InvalidateRect(hwnd, nullptr, TRUE);
        }
    }
}

void DrawCheckerboardBackground(HDC hdc, const RECT& clientRect) {
    int squareSize = 20;
    for (int y = 0; y < clientRect.bottom; y += squareSize) {
        for (int x = 0; x < clientRect.right; x += squareSize) {
            COLORREF squareColor = ((x / squareSize + y / squareSize) % 2 == 0) ? RGB(200, 200, 200) : RGB(240, 240, 240);
            HBRUSH brush = CreateSolidBrush(squareColor);
            RECT squareRect = { x, y, x + squareSize, y + squareSize };
            FillRect(hdc, &squareRect, brush);
            DeleteObject(brush);
        }
    }
}

void DrawFigure(HDC hdc, const RECT& clientRect) {
    if (cachedFigure.pixelData.empty()) return;

    int offsetX = (clientRect.right - clientRect.left - cachedFigure.width) / 2;

    int offsetY = clientRect.bottom - cachedFigure.height;

    for (int y = cachedFigure.height - 1; y >= 0; --y) {
        for (int x = 0; x < cachedFigure.width; ++x) {

            BYTE* pixel = &cachedFigure.pixelData[(y * cachedFigure.width + x) * 3];
            COLORREF pixelColor = RGB(pixel[2], pixel[1], pixel[0]);

            if (pixelColor == backgroundColor) {
                continue;
            }

            SetPixel(hdc, offsetX + x, offsetY + (cachedFigure.height - 1 - y), pixelColor);
        }
    }
}



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"SampleWindowClass";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Bitmap Loader",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND button;

    switch (uMsg) {
    case WM_CREATE: {
        button = CreateWindow(
            L"BUTTON", L"Load BMP",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 10, 100, 30,
            hwnd, (HMENU)1, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
        );
        break;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            OpenBitmapFile(hwnd);
        }
        break;
    case WM_SIZE:
        InvalidateRect(hwnd, nullptr, TRUE);
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        DrawCheckerboardBackground(hdc, clientRect);

        DrawFigure(hdc, clientRect);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
