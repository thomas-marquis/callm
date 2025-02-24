#include "pytypedefs.h"
#define PY_SSIZE_T_CLEAN

#include "../core/errors.h"
#include "../core/safetensors.h"
#include "../llm/model.h"
#include "../monitor/probe.h"
#include "../tokenizer/tokenizer.h"
#include <Python.h>

#define MODULE_NAME "pycallm"

#define CALLM_EXC_NAME "CallmError"
#define CALLM_EXC_FULL_NAME "pycallm.CallmError"

#define TOKENIZER_CLS_NAME "Tokenizer"
#define TOKENIZER_CLS_FULL_NAME "pycallm.Tokenizer"

#define MODEL_CLS_NAME "LLamaModel"
#define MODEL_CLS_FULL_NAME "pycallm.LLamaModel"

#define HANDLE_MEMORY_ERR(ptr, msg, goto_label)                                                                        \
    if (ptr == NULL)                                                                                                   \
    {                                                                                                                  \
        PyErr_SetString(PyExc_MemoryError, msg);                                                                       \
        goto goto_label;                                                                                               \
    }

#define HANDLE_RUNTIME_ERR(ptr, msg, goto_label)                                                                       \
    if (ptr == NULL)                                                                                                   \
    {                                                                                                                  \
        PyErr_SetString(PyExc_RuntimeError, msg);                                                                      \
        goto goto_label;                                                                                               \
    }

#define HANDLE_INTERNAL_ERR(ptr, msg, goto_label)                                                                      \
    if (ptr == NULL)                                                                                                   \
    {                                                                                                                  \
        PyErr_SetString(CallmError, msg);                                                                              \
        goto goto_label;                                                                                               \
    }

static PyObject *CallmError;

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
    HANDLE_INTERNAL_ERR(tokenizer, "Failed to create Tokenizer object", error);
    Tokenizer_print(tokenizer);

    self->tokenizer = tokenizer;

    return 0;
error:
    return -1;
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
    HANDLE_MEMORY_ERR(tokenizer, "Null pointer exception: no reference to Tokenizer object found", error);

    int *token_ids;
    int token_count;
    CallmStatusCode status = Tokenizer_encode(tokenizer, input_str, &token_ids, &token_count);
    if (status != OK)
    {
        PyErr_SetString(CallmError, "Failed to tokenize input string");
        goto error;
    }

    PyObject *py_list = PyList_New(token_count);
    for (int i = 0; i < token_count; i++)
    {
        PyObject *py_token = Py_BuildValue("i", token_ids[i]);
        PyList_SetItem(py_list, i, py_token);
    }

    free(token_ids);
    return py_list;

error:
    return NULL;
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
    HANDLE_INTERNAL_ERR(tokenizer, "Null pointer exception: no reference to Tokenizer object found", error);

    char *token = Tokenizer_decode_single(tokenizer, token_id);
    if (token == NULL)
    {
        return NULL;
    }

    PyObject *py_token = Py_BuildValue("s", token);

    free(token);
    return py_token;

error:
    return NULL;
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
    = TOKENIZER_CLS_FULL_NAME,
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

    if (Probe_init("127.0.0.1", 8080) != OK)
    {
        PyErr_SetString(CallmError, "Failed to initialize probe client");
        return -1;
    }

    Safetensors *st = Safetensors_new(safetensors_path);
    HANDLE_INTERNAL_ERR(st, "Failed to create Safetensors object", finally1);

    Config *config = Config_new(config_path);
    HANDLE_INTERNAL_ERR(config, "Failed to create Config object", finally2);

    Model *model = Model_new(st, config);
    HANDLE_INTERNAL_ERR(model, "Failed to create Model object", finally3);

    self->model = model;
    self->safetensors = st;
    self->config = config;

    return 0;
finally3:
    Config_free(config);
finally2:
    Safetensors_free(st);
finally1:
    return -1;
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
    PyObject *result = NULL;

    PyObject *input_list;
    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &input_list))
    {
        PyErr_SetString(PyExc_TypeError, "Invalid input: expected a list of integers");
        return NULL;
    }

    Model *model = self->model;
    HANDLE_MEMORY_ERR(model, "Null pointer exception: no reference to model object found", finally);

    Py_ssize_t list_size = PyList_Size(input_list);
    int *token_ids = (int *) malloc(list_size * sizeof(int));
    HANDLE_MEMORY_ERR(token_ids, "Memory allocation failed for input token ids", finally);

    for (Py_ssize_t i = 0; i < list_size; i++)
    {
        PyObject *item = PyList_GetItem(input_list, i);
        if (!PyLong_Check(item))
        {
            PyErr_SetString(PyExc_TypeError, "List items must be integers");
            goto finally2;
        }
        token_ids[i] = (int) PyLong_AsLong(item);
    }

    Matrix *out = Model_forward(model, token_ids, list_size);
    HANDLE_INTERNAL_ERR(out, "Failed to generate text", finally2);
    Matrix_print(out, 10);

    char output_str[] = "Ceci est un test";
    result = Py_BuildValue("s", output_str);

    Matrix_free(out);

finally2:
    free(token_ids);
    // free(output_str);
finally:
    return result;
}

static PyObject *
LLamaModelObject_embed(LLamaModelObject *self, PyObject *args)
{
    PyObject *result_list = NULL;
    Matrix *cos = NULL;
    Matrix *sin = NULL;

    PyObject *input_list;
    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &input_list))
    {
        PyErr_SetString(PyExc_TypeError, "Invalid input: expected a list of integers");
        return NULL;
    }

    Model *model = self->model;
    HANDLE_MEMORY_ERR(model, "Null pointer exception: no reference to model object found", finally);

    Py_ssize_t list_size = PyList_Size(input_list);
    int *token_ids = (int *) malloc(list_size * sizeof(int));
    HANDLE_MEMORY_ERR(token_ids, "Memory allocation failed for input token ids", finally);

    for (Py_ssize_t i = 0; i < list_size; i++)
    {
        PyObject *item = PyList_GetItem(input_list, i);
        if (!PyLong_Check(item))
        {
            PyErr_SetString(PyExc_TypeError, "List items must be integers");
            goto finally2;
        }
        token_ids[i] = (int) PyLong_AsLong(item);
    }

    Matrix *embeds_in = Model_embed_inputs(model, token_ids, list_size, &cos, &sin);
    HANDLE_INTERNAL_ERR(embeds_in, "Failed to embed input tokens", finally2);

    result_list = PyList_New(list_size);
    HANDLE_MEMORY_ERR(result_list, "Memory allocation failed for embedding list", finally3);

    int vector_size = embeds_in->c;
    for (Py_ssize_t i = 0; i < embeds_in->r; i++)
    {
        PyObject *vector = PyList_New(vector_size);
        HANDLE_MEMORY_ERR(vector, "Memory allocation failed for embedding vector", finally3);

        for (int j = 0; j < vector_size; j++)
        {
            PyObject *value = PyLong_FromLong(embeds_in->data[i * vector_size + j]);
            HANDLE_RUNTIME_ERR(value, "Failed to convert embedding value to Python integer", finally3);
            PyList_SetItem(vector, j, value);
        }

        PyList_SetItem(result_list, i, vector);
    }

finally3:
    Matrix_free(embeds_in);
finally2:
    free(token_ids);
finally:
    Matrix_free(cos);
    Matrix_free(sin);

    return result_list;
}

static PyMethodDef LLamaModelObject_methods[] = {
    { "generate", (PyCFunction) LLamaModelObject_generate, METH_VARARGS, "Generate text with LLM" },
    { "embed", (PyCFunction) LLamaModelObject_embed, METH_VARARGS,
      "Get embedding vectors for a given set of token ids. Returs list[list[int]]" },
    { NULL }  // Sentinel
};

static PyTypeObject LLamaModel_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)  // init
        .tp_name
    = MODEL_CLS_FULL_NAME,
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

static struct PyModuleDef callm_module_def = { PyModuleDef_HEAD_INIT, MODULE_NAME, NULL, -1, CaLLMModule_methods };

PyMODINIT_FUNC
PyInit_pycallm(void)
{
    PyObject *m;
    if (PyType_Ready(&Tokenizer_Type) < 0 || PyType_Ready(&LLamaModel_Type) < 0)
        return NULL;

    m = PyModule_Create(&callm_module_def);
    if (m == NULL)
        return NULL;

    CallmError = PyErr_NewException(CALLM_EXC_FULL_NAME, NULL, NULL);
    if (CallmError == NULL)
        return NULL;

    Py_INCREF(CallmError);
    Py_INCREF(&Tokenizer_Type);
    Py_INCREF(&LLamaModel_Type);
    PyModule_AddObject(m, TOKENIZER_CLS_NAME, (PyObject *) &Tokenizer_Type);
    PyModule_AddObject(m, MODEL_CLS_NAME, (PyObject *) &LLamaModel_Type);
    PyModule_AddObject(m, CALLM_EXC_NAME, CallmError);

    return m;
}
