#define PY_SSIZE_T_CLEAN
#include "../utils/hash_map.h"
#include <Python.h>

typedef struct
{
    PyObject_HEAD;
    HashMap *map;
} CallmHashMapObject;

static PyTypeObject CallmHashMapType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0).tp_name = "callm.HashMap",
    .tp_doc = PyDoc_STR("CaLLM implementation of a hash map."),
    .tp_basicsize = sizeof(CallmHashMapObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
};

static PyModuleDef custommodule = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "callm",
    .m_doc = "Base CaLLM module.",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_callm(void)
{
    PyObject *m;
    if (PyType_Ready(&CallmHashMapType) < 0)
        return NULL;

    m = PyModule_Create(&custommodule);
    if (m == NULL)
        return NULL;

    if (PyModule_AddObjectRef(m, "HashMap", (PyObject *) &CallmHashMapType) < 0)
    {
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
