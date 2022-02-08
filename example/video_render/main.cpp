#include <iostream>

#include "main_window.h"

int main(void) {
    MainWindow main_window;
    main_window.Init();
    main_window.CreateDuiWindow();
    main_window.Show();
    return 0;
}