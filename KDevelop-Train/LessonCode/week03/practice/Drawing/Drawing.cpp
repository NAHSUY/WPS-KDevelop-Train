/*
 * WPS C++ 绘图实践
 * 基于 Win32 API 实现带状态持久化与重绘的简易绘图程序
 *
 * 编译命令（MinGW）：
 *   g++ drawing.cpp -o drawing.exe -lgdi32 -luser32 -mwindows
 *
 * 编译命令（MSVC）：
 *   cl drawing.cpp /link user32.lib gdi32.lib /SUBSYSTEM:WINDOWS
 *
 * 窗口尺寸改变策略：保留原有笔画，按新客户区重绘（超出部分由 GDI 自然裁剪）
 */
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <windowsx.h> 
#include <vector>
#include <string>

 // ===================== 常量定义 =====================

static const wchar_t* CLASS_NAME = L"WPSDrawClass";
static const wchar_t* WINDOW_TITLE = L"WPS C++ 绘图实践";

// 定时器 ID（用于延迟提交，过滤双击）
static const UINT_PTR TIMER_COMMIT = 1;

// 距离平方采样阈值（约 2 像素），减少过密点
static const int SAMPLE_DIST_SQ = 4;

// ===================== 数据结构 =====================

// 一笔 = 一系列点
using Stroke = std::vector<POINT>;

// ===================== 全局状态 =====================

// 已提交的笔画列表
static std::vector<Stroke> g_strokes;

// 当前正在绘制的笔（尚未提交）
static Stroke g_currentStroke;

// 待提交笔（松键后等待双击超时）
static Stroke g_pendingStroke;

// 是否正在绘制（左键按下状态）
static bool g_isDrawing = false;

// 提示文字缓冲区
static wchar_t g_hintBuffer[256] = L"左键拖动绘制 | C 清空 | ESC 退出";

// ===================== 辅助函数 =====================

// 更新提示文字并触发重绘
static void SetHint(HWND hwnd, const wchar_t* hint)
{
    wcscpy_s(g_hintBuffer, hint);
    InvalidateRect(hwnd, nullptr, TRUE);
}

// 根据已提交笔数 + 是否有待提交笔，更新"已绘制 N 笔"提示
static void UpdateStrokeCountHint(HWND hwnd)
{
    // 待提交笔也计入显示（避免感知延迟）
    int total = (int)g_strokes.size() + (g_pendingStroke.size() >= 2 ? 1 : 0);
    wchar_t buf[256];
    swprintf_s(buf, L"已绘制 %d 笔 | C 清空 | ESC 退出", total);
    SetHint(hwnd, buf);
}

// 正式提交待提交笔到笔画列表
static void CommitPendingStroke(HWND hwnd)
{
    if (g_pendingStroke.size() >= 2) {
        g_strokes.push_back(std::move(g_pendingStroke));
    }
    g_pendingStroke.clear();
    UpdateStrokeCountHint(hwnd);
}

// 丢弃待提交笔（双击时调用）
static void DiscardPendingStroke(HWND hwnd)
{
    KillTimer(hwnd, TIMER_COMMIT);
    g_pendingStroke.clear();
    UpdateStrokeCountHint(hwnd);
    InvalidateRect(hwnd, nullptr, TRUE);
}

// 结束当前一笔（松键或非客户区释放时调用）
static void EndCurrentStroke(HWND hwnd)
{
    if (!g_isDrawing) return;
    g_isDrawing = false;

    if (g_currentStroke.size() >= 2) {
        // 有效笔：放入"待提交"，启动定时器等待双击超时
        // 待提交期间若再次按下则视为双击，丢弃该笔
        g_pendingStroke = std::move(g_currentStroke);
        g_currentStroke.clear();
        UINT dblTime = GetDoubleClickTime();
        SetTimer(hwnd, TIMER_COMMIT, dblTime, nullptr);
        // 提示立即显示含待提交笔的数量（避免感知延迟）
        UpdateStrokeCountHint(hwnd);
    }
    else {
        // 无效笔（点击未移动）：直接清空，更新提示
        g_currentStroke.clear();
        UpdateStrokeCountHint(hwnd);
    }
    InvalidateRect(hwnd, nullptr, TRUE);
}

// ===================== 绘图函数 =====================

// 绘制单笔（折线）
static void DrawStroke(HDC hdc, const Stroke& stroke)
{
    if (stroke.size() < 2) return;
    MoveToEx(hdc, stroke[0].x, stroke[0].y, nullptr);
    for (size_t i = 1; i < stroke.size(); ++i) {
        LineTo(hdc, stroke[i].x, stroke[i].y);
    }
}

// 在 WM_PAINT 中根据保存的数据重绘全部内容
static void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // 设置画笔（黑色，宽度 2）
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    // 绘制已提交笔画
    for (const auto& stroke : g_strokes) {
        DrawStroke(hdc, stroke);
    }

    // 绘制待提交笔（视觉上与已提交笔一致）
    DrawStroke(hdc, g_pendingStroke);

    // 绘制当前正在绘制的笔
    DrawStroke(hdc, g_currentStroke);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    // 绘制提示文字（左上角）
    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.left += 8;
    rc.top += 8;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(80, 80, 200));
    DrawTextW(hdc, g_hintBuffer, -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE);

    EndPaint(hwnd, &ps);
}

// ===================== 窗口过程 =====================

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        // -------- 鼠标：左键按下 --------
    case WM_LBUTTONDOWN:
    {
        // 若有待提交笔，说明这是双击的第二次按下 → 丢弃待提交笔
        if (!g_pendingStroke.empty()) {
            DiscardPendingStroke(hwnd);
        }

        g_isDrawing = true;
        g_currentStroke.clear();
        g_currentStroke.reserve(512); // 预留容量，减少扩容拷贝

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        g_currentStroke.push_back(pt);

        SetHint(hwnd, L"绘制中... 松开左键结束");
        SetCapture(hwnd); // 捕获鼠标，确保拖出窗口仍能收到消息
        break;
    }

    // -------- 鼠标：移动 --------
    case WM_MOUSEMOVE:
    {
        if (!g_isDrawing) break;

        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        // 距离采样：仅当新点与上一点距离平方 >= 阈值时才记录
        if (!g_currentStroke.empty()) {
            const POINT& last = g_currentStroke.back();
            long dx = pt.x - last.x;
            long dy = pt.y - last.y;
            if (dx * dx + dy * dy < SAMPLE_DIST_SQ) break;
        }

        g_currentStroke.push_back(pt);
        // 仅重绘（不清背景），减少闪烁；提示文字已在 WM_LBUTTONDOWN 更新
        InvalidateRect(hwnd, nullptr, FALSE);
        break;
    }

    // -------- 鼠标：左键松开（客户区） --------
    case WM_LBUTTONUP:
    {
        ReleaseCapture();
        EndCurrentStroke(hwnd);
        break;
    }

    // -------- 双击：丢弃当前笔，过滤双击产生的笔画 --------
    case WM_LBUTTONDBLCLK:
    {
        // 丢弃因双击第一次按下产生的当前笔
        g_isDrawing = false;
        g_currentStroke.clear();
        // 待提交笔已在 WM_LBUTTONDOWN 中处理（丢弃）
        UpdateStrokeCountHint(hwnd);
        InvalidateRect(hwnd, nullptr, TRUE);
        break;
    }

    // -------- 非客户区鼠标释放（如拖到标题栏/边框后松键） --------
    case WM_NCLBUTTONUP:
    {
        if (g_isDrawing) {
            ReleaseCapture();
            EndCurrentStroke(hwnd);
        }
        // 交给 DefWindowProc 处理非客户区逻辑（如 resize）
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    // -------- 定时器：双击超时，正式提交待提交笔 --------
    case WM_TIMER:
    {
        if (wParam == TIMER_COMMIT) {
            KillTimer(hwnd, TIMER_COMMIT);
            CommitPendingStroke(hwnd);
            InvalidateRect(hwnd, nullptr, TRUE);
        }
        break;
    }

    // -------- 键盘 --------
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case 'C':
        case 'c':
        {
            // 清空所有笔画（包括待提交、当前笔）
            KillTimer(hwnd, TIMER_COMMIT);
            g_strokes.clear();
            g_pendingStroke.clear();
            g_currentStroke.clear();
            g_isDrawing = false;
            SetHint(hwnd, L"已清空 | 左键拖动绘制 | ESC 退出");
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
        }
        case VK_ESCAPE:
        {
            int ret = MessageBoxW(hwnd,
                L"是否退出？",
                L"退出确认",
                MB_YESNO | MB_ICONQUESTION);
            if (ret == IDYES) {
                DestroyWindow(hwnd);
            }
            break;
        }
        }
        break;
    }

    // -------- 绘制 --------
    case WM_PAINT:
    {
        OnPaint(hwnd);
        break;
    }

    // -------- 窗口尺寸改变 --------
    // 策略：保留原有笔画，按新客户区重绘（超出部分由 GDI 自然裁剪，不删除数据）
    case WM_SIZE:
    {
        InvalidateRect(hwnd, nullptr, TRUE);
        break;
    }

    // -------- 关闭按钮 --------
    case WM_CLOSE:
    {
        DestroyWindow(hwnd);
        break;
    }

    // -------- 销毁窗口：释放资源，结束消息循环 --------
    case WM_DESTROY:
    {
        // 释放动态分配的点序列内存
        g_strokes.clear();
        g_strokes.shrink_to_fit();
        g_pendingStroke.clear();
        g_pendingStroke.shrink_to_fit();
        g_currentStroke.clear();
        g_currentStroke.shrink_to_fit();

        KillTimer(hwnd, TIMER_COMMIT);
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ===================== 程序入口 =====================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    // 注册窗口类（CS_DBLCLKS：接收双击消息）
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_CROSS); // 十字光标，更适合绘图
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(nullptr, L"窗口类注册失败", L"错误", MB_ICONERROR);
        return 1;
    }

    // 创建窗口（800×600）
    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd) {
        MessageBoxW(nullptr, L"窗口创建失败", L"错误", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 消息循环
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}