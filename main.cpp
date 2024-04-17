#include <python.h>
#include <iostream>


int main(int argc,char*argv[])
{
    Py_Initialize()
    PyObject *name, *load_module,*func,*callfunc,*args;
    name=PyUnicode_FromString((char*)"script");
    load_module=PyImport_Import(name);
    func = PyObject_GetAttrString(load_module,(char*)"fun1");
    callfunc=PyObject_CallObject(func,NULL);
    double fun1_out=PyFloat_AsDouble(callfunc);

    func = PyObject_GetAttrString(load_module,(char*)"func2");
    args = PyTuple_Pack(1,PyFloat_FromDouble(4.0))
    callfunc=PyObject_CallObject(func,NULL);
    double fun2_out=PyFloat_AsDouble(callfunc);
    
    py_Finalize();
    std::cout <<fun1_out<<std::endl;
    std::cout <<fun2_out<<std::endl;


  
}