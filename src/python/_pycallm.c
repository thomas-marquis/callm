#include "pytypedefs.h"
#define PY_SSIZE_T_CLEAN

#include "../core/errors.h"
#include "../core/safetensors.h"
#include "../llm/model.h"
#include "../tokenizer/tokenizer.h"
#include <Python.h>

typedef struct
{
    PyObject_HEAD;
    Tokenizer *tokenizer;
} TokenizerObject;

static PyObject *
TokenizerObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    TokenizerObject *self;
    self = (TokenizerObject *) type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->tokenizer = NULL;
    }
    return (PyObject *) self;
}

static int
TokenizerObject_init(TokenizerObject *self, PyObject *args, PyObject *kwds)
{
    const char *filepath;
    if (!PyArg_ParseTuple(args, "s", &filepath))
    {
        return -1;
    }

    Tokenizer *tokenizer = Tokenizer_new(filepath);

    self->tokenizer = tokenizer;
    if (!self->tokenizer)
    {
        PyErr_SetString(PyExc_MemoryError, "Failed to create Tokenizer object");
        return -1;
    }
    return 0;
}

static void
TokenizerObject_free(TokenizerObject *self)
{
    if (self->tokenizer)
    {
        Tokenizer_free(self->tokenizer);
    }
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
TokenizerObject_tokenize(TokenizerObject *self, PyObject *args)
{
    const char *input_str;
    if (!PyArg_ParseTuple(args, "s", &input_str))
    {
        PyErr_SetString(PyExc_TypeError, "Invalid input string");
        return NULL;
    }

    Tokenizer *tokenizer = self->tokenizer;
    if (tokenizer == NULL)
    {
        PyErr_SetString(PyExc_MemoryError, "Null pointer exception: no reference to Tokenizer object found");
        return NULL;
    }

    int *token_ids;
    int token_count;
    CallmStatusCode status = Tokenizer_encode(tokenizer, input_str, &token_ids, &token_count);
    if (status != OK)
    {
        return NULL;
    }

    PyObject *py_list = PyList_New(token_count);
    for (int i = 0; i < token_count; i++)
    {
        PyObject *py_token = Py_BuildValue("i", token_ids[i]);
        PyList_SetItem(py_list, i, py_token);
    }
    return py_list;
}

static PyObject *
TokenizerObject_get_token_by_id(TokenizerObject *self, PyObject *args)
{
    int token_id;
    if (!PyArg_ParseTuple(args, "i", &token_id))
    {
        PyErr_SetString(PyExc_TypeError, "Invalid argument type: expected integer");
        return NULL;
    }

    Tokenizer *tokenizer = self->tokenizer;
    if (tokenizer == NULL)
    {
        PyErr_SetString(PyExc_MemoryError, "Null pointer exception: no reference to Tokenizer object found");
        return NULL;
    }

    char *token = Tokenizer_decode_single(tokenizer, token_id);
    if (token == NULL)
    {
        return NULL;
    }

    PyObject *py_token = Py_BuildValue("s", token);
    return py_token;
}

static PyMethodDef TokenizerObject_methods[] = {
    { "tokenize", (PyCFunction) TokenizerObject_tokenize, METH_VARARGS,
      "Tokenize the given text and return tokens ids as list of int" },
    { "get_token_by_id", (PyCFunction) TokenizerObject_get_token_by_id, METH_VARARGS,
      "Returns the string value corresponding to the provided token id" },
    { NULL }  // Sentinel
};

static PyTypeObject Tokenizer_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)  // init
        .tp_name
    = "pycallm.Tokenizer",
    .tp_basicsize = sizeof(TokenizerObject),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor) TokenizerObject_free,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "Tokenizer object",
    .tp_methods = TokenizerObject_methods,
    .tp_init = (initproc) TokenizerObject_init,
    .tp_new = TokenizerObject_new,
};

typedef struct
{
    PyObject_HEAD;
    Model *model;
    Safetensors *safetensors;
    Config *config;
} LLamaModelObject;

static PyObject *
LLamaModelObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    LLamaModelObject *self;
    self = (LLamaModelObject *) type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->model = NULL;
    }
    return (PyObject *) self;
}

static int
LLamaModelObject_init(LLamaModelObject *self, PyObject *args, PyObject *kwds)
{
    const char *safetensors_path;
    const char *config_path;
    if (!PyArg_ParseTuple(args, "ss", &safetensors_path, &config_path))
    {
        return -1;
    }

    Safetensors *st = Safetensors_new(safetensors_path);
    Config *config = Config_new(config_path);
    Model *model = Model_new(st, config);

    self->model = model;
    if (!self->model)
    {
        PyErr_SetString(PyExc_MemoryError, "Failed to create Model object");
        return -1;
    }
    self->safetensors = st;
    self->config = config;
    return 0;
}

static void
LLamaModelObject_free(LLamaModelObject *self)
{
    if (self->safetensors)
    {
        Safetensors_free(self->safetensors);
    }
    if (self->config)
    {
        Config_free(self->config);
    }
    if (self->model)
    {
        Model_free(self->model);
    }
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
LLamaModelObject_generate(LLamaModelObject *self, PyObject *args)
{
    PyObject *input_list;
    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &input_list))
    {
        PyErr_SetString(PyExc_TypeError, "Invalid input: expected a list of integers");
        return NULL;
    }

    Model *model = self->model;
    if (model == NULL)
    {
        PyErr_SetString(PyExc_MemoryError, "Null pointer exception: no reference to model object found");
        return NULL;
    }

    Py_ssize_t list_size = PyList_Size(input_list);
    int *token_ids = (int *) malloc(list_size * sizeof(int));
    if (token_ids == NULL)
    {
        PyErr_SetString(PyExc_MemoryError, "Memory allocation failed for input token ids");
        return NULL;
    }

    for (Py_ssize_t i = 0; i < list_size; i++)
    {
        PyObject *item = PyList_GetItem(input_list, i);
        if (!PyLong_Check(item))
        {
            free(token_ids);
            PyErr_SetString(PyExc_TypeError, "List items must be integers");
            return NULL;
        }
        token_ids[i] = (int) PyLong_AsLong(item);
    }

    char output_str[] = "Ceci est un test";
    // CallmStatusCode status = Model_forward(model, token_ids, list_size, &output_str);
    // free(token_ids);
    // if (status != OK)
    // {
    //     PyErr_SetString(PyExc_RuntimeError, "Model generation failed");
    //     return NULL;
    // }

    PyObject *result = Py_BuildValue("s", output_str);
    // free(output_str);
    return result;
}

static PyMethodDef LLamaModelObject_methods[] = {
    { "generate", (PyCFunction) LLamaModelObject_generate, METH_VARARGS, "Generate text with LLM" },
    { NULL }  // Sentinel
};

static PyTypeObject LLamaModel_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)  // init
        .tp_name
    = "pycallm.LLamaModel",
    .tp_basicsize = sizeof(LLamaModelObject),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor) LLamaModelObject_free,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "LLama model instance",
    .tp_methods = LLamaModelObject_methods,
    .tp_init = (initproc) LLamaModelObject_init,
    .tp_new = LLamaModelObject_new,
};

static PyMethodDef CaLLMModule_methods[] = {
    { NULL }  // Sentinel
};

static struct PyModuleDef callm_module_def = { PyModuleDef_HEAD_INIT, "pycallm", NULL, -1, CaLLMModule_methods };

PyMODINIT_FUNC
PyInit_pycallm(void)
{
    PyObject *m;
    if (PyType_Ready(&Tokenizer_Type) < 0 || PyType_Ready(&LLamaModel_Type) < 0)
        return NULL;

    m = PyModule_Create(&callm_module_def);
    if (m == NULL)
        return NULL;

    Py_INCREF(&Tokenizer_Type);
    Py_INCREF(&LLamaModel_Type);
    PyModule_AddObject(m, "Tokenizer", (PyObject *) &Tokenizer_Type);
    PyModule_AddObject(m, "LLamaModel", (PyObject *) &LLamaModel_Type);
    return m;
}
