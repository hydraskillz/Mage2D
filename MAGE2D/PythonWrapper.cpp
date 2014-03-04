#include "MageLib.h"
#if 0
#include <Python.h>

//#pragma comment( lib, "python33.lib" )

using namespace mage;

static GameObject* gOwner;
//---------------------------------------
static PyObject* PyConsolePrintf( PyObject* self, PyObject* args )
{
	const char* s;
	if ( PyArg_ParseTuple( args, "s", &s ) )
	{
		ConsolePrintf( "%s\n", s );
	}
	Py_RETURN_NONE;
}
//---------------------------------------
static PyObject* PyFireEvent( PyObject*, PyObject* args )
{
	const char* eventName;
	if ( PyArg_ParseTuple( args, "s", &eventName ) )
	{
		EventManager::FireEvent( eventName );
	}
	Py_RETURN_NONE;
}
//---------------------------------------
static PyObject* PyGetOwnerName( PyObject*, PyObject* )
{
	return Py_BuildValue( "s", gOwner->Name.c_str() );
}
//---------------------------------------
static PyObject* PyGetIntProperty( PyObject*, PyObject* args )
{
	const char* propName;
	if ( PyArg_ParseTuple( args, "s", &propName ) )
	{
		GameVar& gvar = gOwner->GetProperty( propName );
		int i;
		if ( gvar.GetDataAs( &i ) )
		{
			return Py_BuildValue( "i", i  );
		}
	}
	Py_RETURN_NONE;
}
//---------------------------------------
static PyObject* PyGetStringProperty( PyObject*, PyObject* args )
{
	const char* propName;
	if ( PyArg_ParseTuple( args, "s", &propName ) )
	{
		GameVar& gvar = gOwner->GetProperty( propName );
		std::string s;
		if ( gvar.GetDataAs( &s ) )
		{
			return Py_BuildValue( "s", s.c_str()  );
		}
	}
	Py_RETURN_NONE;
}
//---------------------------------------
static PyObject* PySetIntProperty( PyObject*, PyObject* args )
{
	const char* propName;
	int value;
	if ( PyArg_ParseTuple( args, "si", &propName, &value ) )
	{
		gOwner->SetProperty( propName, StringUtil::ToString( value ) );
	}
	Py_RETURN_NONE;
}
//---------------------------------------
static PyObject* PySetStringProperty( PyObject*, PyObject* args )
{
	const char* propName;
	char* value;
	if ( PyArg_ParseTuple( args, "ss", &propName, &value ) )
	{
		gOwner->SetProperty( propName, value );
	}
	Py_RETURN_NONE;
}
//---------------------------------------
static PyMethodDef mage_methods[] =
{
	{ "ConsolePrint",       PyConsolePrintf,     METH_VARARGS, NULL },
	{ "FireEvent",          PyFireEvent,         METH_VARARGS, NULL },
	{ "GetOwnerName",       PyGetOwnerName,      METH_NOARGS,  NULL },
	{ "GetIntProperty",     PyGetIntProperty,    METH_VARARGS, NULL },
	{ "SetIntProperty",     PySetIntProperty,    METH_VARARGS, NULL },
	{ "GetStringProperty",  PyGetStringProperty, METH_VARARGS, NULL },
	{ "SetStringProperty",  PySetStringProperty, METH_VARARGS, NULL },
	{ NULL, NULL, 0, NULL }
};
//---------------------------------------
static struct PyModuleDef MageModule = 
{
	PyModuleDef_HEAD_INIT,
	"mage",
	NULL,
	-1,
	mage_methods,
	NULL, NULL, NULL, NULL
};
//---------------------------------------
PyMODINIT_FUNC PyInit_mage()
{
	return PyModule_Create( &MageModule );
}
//---------------------------------------
bool PythonWrapper::Initialize()
{
	//wprintf( L"Path: %s\n", Py_GetProgramFullPath() );
	//wprintf( L"Python Path: %s\n", Py_GetPath() );

	// Force using the archived python included with the build
	std::wstring dst( Py_GetPath() );
	dst += L";D:\\Projects\\Mage2D\\bin";
	dst += L"\\python33.zip";
	Py_SetPath( dst.c_str() );

	wprintf( L"Python Path: %s\n", Py_GetPath() );

	// Add mage module to python
	PyImport_AppendInittab( "mage", &PyInit_mage );
	
	Py_Initialize();

	return true;
}
//---------------------------------------
void PythonWrapper::ShutDown()
{
	Py_Finalize();
}
//---------------------------------------
void PythonWrapper::RunScript( const char* script_name )
{
	char* _src;
	unsigned len;
	OpenDataFile( script_name, _src, len );

	PyRun_SimpleString( _src );

	delete[] _src;
}
//---------------------------------------
void PythonWrapper::RunScriptOn( const char* script_name, GameObject* owner )
{
	gOwner = owner;
	RunScript( script_name );
}
//---------------------------------------
#endif