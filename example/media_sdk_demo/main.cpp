#include <iostream>

#include "main_window.h"
// #include "dump_writer.h"

int main(void) {
    // DumpWriter::GenerateDumpFile();
    MainWindow main_window;
    main_window.Init();
    main_window.CreateDuiWindow();
    main_window.Show();

    return 0;
}