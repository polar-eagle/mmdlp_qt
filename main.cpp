#include <QtWidgets/QApplication>
#include "mmdlp_gui.h"
#include "ConfigManager.h"
#include <windows.h>
#include "projector_gui.h"
#include <QScreen>
#include <QPointer>

HHOOK g_keyboardHook = nullptr;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT *kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            switch (kbdStruct->vkCode)
            {
            case VK_LWIN:
            case VK_RWIN:
            case VK_TAB:
            case VK_ESCAPE:
            case VK_MENU:
                return 1;
            default:
                break;
            }
        }
    }
    return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}
void disableSystemKeys()
{
    if (!g_keyboardHook)
    {
        g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);
        if (!g_keyboardHook)
        {
            qWarning() << "Failed to set keyboard hook.";
        }
    }
}
void enableSystemKeys()
{
    if (g_keyboardHook)
    {
        UnhookWindowsHookEx(g_keyboardHook);
        g_keyboardHook = nullptr;
    }
}
void setScreen(QWidget *widget, QScreen *screen)
{
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - widget->width()) / 2;
    int y = (screenGeometry.height() - widget->height()) / 2;

    widget->move(screenGeometry.x() + x, screenGeometry.y() + y);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ProjectorGui *projectorGui = new ProjectorGui();
    MmdlpGui *mmdlpGui = new MmdlpGui(nullptr, projectorGui);
    mmdlpGui->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    projectorGui->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    disableSystemKeys();
    QList<QScreen *> screens = a.screens();
    if (screens.size() > 1)
    {
        setScreen(mmdlpGui, screens[0]);
        setScreen(projectorGui, screens[1]);
    }
    else
    {
        QMessageBox::critical(nullptr, "Error", "Screen number is less than 2");
    }
    mmdlpGui->show();
    projectorGui->show();
    // w.showFullScreen();
    return a.exec();
}
