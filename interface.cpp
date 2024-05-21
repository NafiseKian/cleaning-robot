#include <Python.h>

int main() {
    // Initialize Python interpreter
    Py_Initialize();

    // Import the Python module
    PyObject* pModule = PyImport_ImportModule("trial.py");

    if (pModule != NULL) {
        // Call a function from the Python module
        PyObject* pFunc = PyObject_GetAttrString(pModule, "detect_objects");
        
        if (pFunc && PyCallable_Check(pFunc)) {
            // Call the Python function with arguments
            PyObject* pArgs = PyTuple_New(0); // You can pass arguments if your function requires
            
            PyObject* pResult = PyObject_CallObject(pFunc, pArgs);
            
            // Do something with the result if needed
            
            // Clean up
            Py_XDECREF(pArgs);
            Py_XDECREF(pResult);
            Py_XDECREF(pFunc);
        } else {
            if (PyErr_Occurred()) PyErr_Print();
            fprintf(stderr, "Cannot find function\n");
        }
        
        // Clean up
        Py_DECREF(pModule);
    } else {
        PyErr_Print();
        fprintf(stderr, "Failed to load Python module\n");
    }

    // Finalize Python interpreter
    Py_Finalize();

    return 0;
}
