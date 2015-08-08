#include <windows.h>

//! \file
//! \brief Sets the focus to the window passed in as argument
//!
//! Sets the focus to the window passed in as argument
//! Windows specific but can probably be tweaked to do somethign similar on Linux

int main( int argc, char *argv[])
{
    HWND TargetWindow = FindWindow( NULL, argv[1] );
    /*
    SetWindowPos(
        TargetWindow,
        HWND_TOP,
        0,
        0,
        60,
        40,
        SWP_NOSIZE | SWP_NOMOVE
    );
    */
    SetForegroundWindow(
        TargetWindow
    );

}
