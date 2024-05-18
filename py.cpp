#include <Python.h>
#include <fstream>
#include <iostream>

int main() {
    // Initialize Python interpreter
    Py_Initialize();

    // Add the directory containing take_photo.py to the Python system path
    PyRun_SimpleString("import sys\n"
                       "sys.path.append('/home/jeries/Desktop/Project/cleaning-robot/')\n");

    // Read the content of the Python script
    std::ifstream scriptFile("trial.py");
    if (!scriptFile.is_open()) {
        std::cerr << "Failed to open Python script file\n";
        return 1;
    }

    std::string scriptContent((std::istreambuf_iterator<char>(scriptFile)),
                              std::istreambuf_iterator<char>());

    // Close the file
    scriptFile.close();

    // Run Python script
    PyRun_SimpleString(scriptContent.c_str());

    // Finalize Python interpreter
    Py_Finalize();

    return 0;
}
