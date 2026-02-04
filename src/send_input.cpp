#include "send_input.h"
#include <windows.h>

void send_text(const std::wstring &text)
{
    for (wchar_t ch : text)
    {
        INPUT in[2]{};

        in[0].type = INPUT_KEYBOARD;
        in[0].ki.wScan = ch;
        in[0].ki.dwFlags = KEYEVENTF_UNICODE;

        in[1] = in[0];
        in[1].ki.dwFlags |= KEYEVENTF_KEYUP;

        SendInput(2, in, sizeof(INPUT));
    }
}