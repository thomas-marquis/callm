#include "../core/errors.h"
#include "../tokenizer/tokenizer.h"
#include <Python.h>

static PyObject *
py_tokenize(PyObject *self, PyObject *args)
{
    const char *filepath;
    const char *input_str;
    if (!PyArg_ParseTuple(args, "ss", &filepath, &input_str))
    {
        return NULL;
    }

    Tokenizer *tokenizer = Tokenizer_new(filepath);
    if (tokenizer == NULL)
    {
        return NULL;
    }

    int *token_ids;
    int token_count;
    CallmStatusCode status = Tokenizer_encode(tokenizer, input_str, &token_ids, &token_count);
    if (status != OK)
    {
        Tokenizer_free(tokenizer);
        return NULL;
    }

    PyObject *py_list = PyList_New(token_count);
    for (int i = 0; i < token_count; i++)
    {
        PyObject *py_token = Py_BuildValue("s", Tokenizer_decode_single(tokenizer, token_ids[i]));
        PyList_SetItem(py_list, i, py_token);
    }
    Tokenizer_free(tokenizer);
    return py_list;
}

static PyMethodDef CaLLMModuleMethods[]
    = { { "tokenize", py_tokenize, METH_VARARGS, "Encode the input text to correspondig token ids" },
        { NULL, NULL, 0, NULL } };

static struct PyModuleDef callm_module_def = { PyModuleDef_HEAD_INIT, "pycallm", NULL, -1, CaLLMModuleMethods };

PyMODINIT_FUNC
PyInit_pycallm(void)
{
    return PyModule_Create(&callm_module_def);
}
