/*
  pygame physics - Pygame physics module

  Copyright (C) 2008 Zhang Fan

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define PHYSICS_RECTSHAPE_INTERNAL

#include "physicsmod.h"
#include "pgphysics.h"

static PyVector2* _get_vertices (PyShape *shape, Py_ssize_t *count);
static int _get_aabbox (PyShape *shape, AABBox *box);
static int _update (PyShape *shape, PyBody *body);

static PyObject *_rectshape_new (PyTypeObject *type, PyObject *args,
    PyObject *kwds);
static int _rectshape_init (PyRectShape *self, PyObject *args, PyObject *kwds);

static PyObject* _rectshape_getrect (PyRectShape *self, void *closure);

/**
 * Getters/Setters
 */
static PyGetSetDef _rectshape_getsets[] =
{
    { "rect",  (getter) _rectshape_getrect, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL }
};

PyTypeObject PyRectShape_Type =
{
    TYPE_HEAD(NULL, 0)
    "physics.RectShape",        /* tp_name */
    sizeof (PyRectShape),       /* tp_basicsize */
    0,                          /* tp_itemsize */
    0,                          /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_compare */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    "",
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    0,                          /* tp_methods */
    0,                          /* tp_members */
    _rectshape_getsets,         /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)_rectshape_init,  /* tp_init */
    0,                          /* tp_alloc */
    _rectshape_new,             /* tp_new */
    0,                          /* tp_free */
    0,                          /* tp_is_gc */
    0,                          /* tp_bases */
    0,                          /* tp_mro */
    0,                          /* tp_cache */
    0,                          /* tp_subclasses */
    0,                          /* tp_weaklist */
    0,                          /* tp_del */
#if PY_VERSION_HEX >= 0x02060000
    0                           /* tp_version_tag */
#endif
};

static PyVector2*
_get_vertices (PyShape *shape, Py_ssize_t *count)
{
    PyRectShape *r = (PyRectShape*)shape;
    PyVector2* vertices;
    if (!r || !count)
        return NULL;
    
    vertices = PyMem_New (PyVector2, 4);
    if (!vertices)
        return NULL;

    /* Return in CCW order starting from the bottomleft. */
    vertices[0] = r->bottomleft;
    vertices[1] = r->bottomright;
    vertices[2] = r->topright;
    vertices[3] = r->topleft;

    *count = 4;
    return vertices;
}

static int
_get_aabbox (PyShape *shape, AABBox* box)
{
    PyRectShape *r = (PyRectShape*)shape;
    
    if (!shape || !box)
    {
        PyErr_SetString (PyExc_TypeError, "arguments must not be NULL");
        return 0;
    }

    box->top = r->box.top;
    box->left = r->box.left;
    box->bottom = r->box.bottom;
    box->right = r->box.right;
    return 1;
}

static int
_update (PyShape *shape, PyBody *body)
{
    PyRectShape *r = (PyRectShape*)shape;
    PyVector2 gp[4], tl, tr, bl, br;
    
    if (!shape || !body)
        return 0;

    PyVector2_Set (tl, r->topleft.real, r->topleft.imag);
    PyVector2_Set (tr, r->topright.real, r->topright.imag);
    PyVector2_Set (bl, r->bottomleft.real, r->bottomleft.imag);
    PyVector2_Set (br, r->bottomright.real, r->bottomright.imag);

    /* Update the aabbox. */
    PyBody_GetGlobalPos (body, bl, gp[0]);
    PyBody_GetGlobalPos (body, br, gp[1]);
    PyBody_GetGlobalPos (body, tr, gp[2]);
    PyBody_GetGlobalPos (body, tl, gp[3]);

    AABBox_Reset (&(r->box));
    AABBox_ExpandTo (&(r->box), &(gp[0]));
    AABBox_ExpandTo (&(r->box), &(gp[1]));
    AABBox_ExpandTo (&(r->box), &(gp[2]));
    AABBox_ExpandTo (&(r->box), &(gp[3]));
    return 1;
}

static PyObject *_rectshape_new (PyTypeObject *type, PyObject *args,
    PyObject *kwds)
{
    PyRectShape *shape = (PyRectShape*) type->tp_alloc (type, 0);
    if (!shape)
        return NULL;

    shape->shape.get_aabbox = _get_aabbox;
    shape->shape.get_vertices = _get_vertices;
    shape->shape.update = _update;
    shape->shape.type = RECT;

    PyVector2_Set (shape->bottomleft, 0, 0);
    PyVector2_Set (shape->bottomright, 0, 0);
    PyVector2_Set (shape->topleft, 0, 0);
    PyVector2_Set (shape->topright, 0, 0);

    AABBox_Reset (&(shape->box));
    
    return (PyObject*) shape;
}

static int
_rectshape_init (PyRectShape *self, PyObject *args, PyObject *kwds)
{
    PyObject *tuple;
    AABBox box;

    if (PyShape_Type.tp_init ((PyObject*)self, args, kwds) < 0)
        return -1;
    
    if (!PyArg_ParseTuple (args, "O", &tuple))
        return -1;

    if (!AABBox_FromRect (tuple, &box))
        return -1;
    
    PyVector2_Set (self->topleft, box.left, box.top);
    PyVector2_Set (self->topright, box.right, box.top);
    PyVector2_Set (self->bottomleft, box.left, box.bottom);
    PyVector2_Set (self->bottomright, box.right, box.bottom);

    return 0;
}

/* Getters/Setters */
static PyObject*
_rectshape_getrect (PyRectShape *self, void *closure)
{
    AABBox box;
    
    box.top = self->topleft.imag;
    box.left = self->topleft.real;
    box.right = self->bottomright.real;
    box.bottom = self->bottomright.imag;
    
    return AABBox_AsFRect (&box);
}

/* C API */
PyObject*
PyRectShape_New (AABBox box)
{
    /* TODO: is anything correctly initialised? */
    PyRectShape *shape = (PyRectShape*)PyRectShape_Type.tp_new
        (&PyRectShape_Type, NULL, NULL);
    if (!shape)
        return NULL;

    PyVector2_Set (shape->topleft, box.left, box.top);
    PyVector2_Set (shape->topright, box.right, box.top);
    PyVector2_Set (shape->bottomleft, box.left, box.bottom);
    PyVector2_Set (shape->bottomright, box.right, box.bottom);
    return (PyObject*) shape;
}

void
rectshape_export_capi (void **capi)
{
    capi[PHYSICS_RECTSHAPE_FIRSTSLOT + 0] = &PyRectShape_Type;
    capi[PHYSICS_RECTSHAPE_FIRSTSLOT + 1] = PyRectShape_New;
}
