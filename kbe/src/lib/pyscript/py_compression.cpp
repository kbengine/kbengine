// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "py_compression.h"
#include "resmgr/resmgr.h"

namespace KBEngine{ 
namespace script{

bool PyCompression::isInit = false;

//-------------------------------------------------------------------------------------
bool PyCompression::initialize(void)
{
	if (isInit)
		return true;

	return isInit;
}

//-------------------------------------------------------------------------------------
void PyCompression::finalise(void)
{
}

//-------------------------------------------------------------------------------------
bool PyCompression::zipCompressDirectory(const std::string& sourceDir, const std::string& outfile)
{
	std::string outpath = "", zipfile = outfile;

	std::vector<std::string> tmpvec;
	KBEngine::strutil::kbe_splits(zipfile, ".", tmpvec);
	outpath = tmpvec[0];
	zipfile = tmpvec[1];

	if (zipfile.size() > 0)
	{
		PyObject* pyModule = PyImport_ImportModule("zipfile");
		if (pyModule)
		{
			PyObject* zipModule = PyObject_GetAttrString(pyModule, "ZipFile");
			PyObject* zipTypeOBJ = PyObject_GetAttrString(pyModule, "ZIP_DEFLATED");
			Py_DECREF(pyModule);

			if (!zipModule || !zipTypeOBJ)
			{
				Py_XDECREF(zipTypeOBJ);
				Py_XDECREF(zipModule);
				goto _PYZIPMODULE_ERROR;
			}

			int ziptype = PyLong_AsLong(zipTypeOBJ);
			Py_XDECREF(zipTypeOBJ);

			PyObject* pyargs = PyTuple_New(3);
			PyTuple_SET_ITEM(pyargs, 0, PyUnicode_FromString((outpath + "." + zipfile).c_str()));
			PyTuple_SET_ITEM(pyargs, 1, PyUnicode_FromString("w"));
			PyTuple_SET_ITEM(pyargs, 2, PyLong_FromLong(ziptype));

			PyObject* zipObject = PyObject_CallObject(zipModule, pyargs);
			Py_DECREF(pyargs);
			Py_XDECREF(zipModule);

			if (zipObject == NULL)
			{
				SCRIPT_ERROR_CHECK();
				goto _PYZIPMODULE_ERROR;
			}

			wchar_t* wpath = strutil::char2wchar(sourceDir.c_str());
			std::vector<std::wstring> results;
			Resmgr::getSingleton().listPathRes(wpath, L"*", results);

			std::vector<std::wstring>::iterator iter = results.begin();
			for (; iter != results.end(); ++iter)
			{
				std::wstring wstrpath = (*iter);
				strutil::kbe_replace(wstrpath, wpath, L"");
				PyObject* pyResult = PyObject_CallMethod(zipObject, const_cast<char*>("write"),
					const_cast<char*>("u#u#"), (*iter).c_str(), (*iter).size(), wstrpath.c_str(), wstrpath.size());

				if (pyResult != NULL)
					Py_DECREF(pyResult);
				else
					SCRIPT_ERROR_CHECK();
			}

			PyObject* pyResult = PyObject_CallMethod(zipObject, const_cast<char*>("close"),
				const_cast<char*>(""));

			if (pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();

			free(wpath);
			Py_DECREF(zipObject);
			return true;
		}
		else
		{
		_PYZIPMODULE_ERROR:
			SCRIPT_ERROR_CHECK();
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------
bool PyCompression::tarCompressDirectory(const std::string& sourceDir, const std::string& outfile)
{
	std::string outpath = "", gzfile = outfile;

	std::vector<std::string> tmpvec;
	KBEngine::strutil::kbe_splits(gzfile, ".", tmpvec);
	outpath = tmpvec[0];
	gzfile = tmpvec[1];

	if (gzfile.size() > 0)
	{
		PyObject* pyModule = PyImport_ImportModule("tarfile");
		if (pyModule)
		{
			PyObject* pyOpen = PyObject_GetAttrString(pyModule, "open");
			Py_DECREF(pyModule);

			if (!pyOpen)
			{
				goto _PYTARMODULE_ERROR;
			}

			PyObject* pyargs = PyTuple_New(2);
			PyTuple_SET_ITEM(pyargs, 0, PyUnicode_FromString((outpath + "." + gzfile).c_str()));
			PyTuple_SET_ITEM(pyargs, 1, PyUnicode_FromString("w:gz"));

			PyObject* tarObject = PyObject_CallObject(pyOpen, pyargs);
			Py_DECREF(pyargs);
			Py_XDECREF(pyOpen);

			if (tarObject == NULL)
			{
				SCRIPT_ERROR_CHECK();
				goto _PYTARMODULE_ERROR;
			}

			wchar_t* wpath = strutil::char2wchar(sourceDir.c_str());
			std::vector<std::wstring> results;
			Resmgr::getSingleton().listPathRes(wpath, L"*", results);

			std::vector<std::wstring>::iterator iter = results.begin();
			for (; iter != results.end(); ++iter)
			{
				std::wstring wstrpath = (*iter);
				strutil::kbe_replace(wstrpath, wpath, L"");
				PyObject* pyResult = PyObject_CallMethod(tarObject, const_cast<char*>("add"),
					const_cast<char*>("u#u#"), (*iter).c_str(), (*iter).size(), wstrpath.c_str(), wstrpath.size());

				if (pyResult != NULL)
					Py_DECREF(pyResult);
				else
					SCRIPT_ERROR_CHECK();
			}

			PyObject* pyResult = PyObject_CallMethod(tarObject, const_cast<char*>("close"),
				const_cast<char*>(""));

			if (pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();

			free(wpath);
			Py_DECREF(tarObject);
			return true;
		}
		else
		{
		_PYTARMODULE_ERROR:
			SCRIPT_ERROR_CHECK();
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------

}
}
