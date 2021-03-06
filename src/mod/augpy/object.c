/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define MOD_BUILD
#include "object.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include <structmember.h>

struct boximpl_ {
    augpy_box box_;
    int refs_;
    PyObject* pyob_;
};

static void*
castbox_(augpy_box* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, augpy_boxid)) {
        aug_retain(ob);
        return ob;
    }
    return NULL;
}

static void
retainbox_(augpy_box* ob)
{
    struct boximpl_* impl = AUG_PODIMPL(struct boximpl_, box_, ob);
    ++impl->refs_;
}

static void
releasebox_(augpy_box* ob)
{
    struct boximpl_* impl = AUG_PODIMPL(struct boximpl_, box_, ob);
    if (0 == --impl->refs_) {
        Py_DECREF(impl->pyob_);
        free(impl);
    }
}

static PyObject*
unbox_(augpy_box* ob)
{
    struct boximpl_* impl = AUG_PODIMPL(struct boximpl_, box_, ob);
    Py_INCREF(impl->pyob_);
    return impl->pyob_;
}

static const struct augpy_boxvtbl boxvtbl_ = {
    castbox_,
    retainbox_,
    releasebox_,
    unbox_
};

augpy_box*
augpy_createbox(PyObject* pyob)
{
    struct boximpl_* impl = malloc(sizeof(struct boximpl_));
    if (!impl)
        return NULL;

    if (!pyob)
        pyob = Py_None;

    impl->box_.vtbl_ = &boxvtbl_;
    impl->box_.impl_ = NULL;

    impl->refs_ = 1;
    impl->pyob_ = pyob;

    Py_INCREF(impl->pyob_);
    return &impl->box_;
}

struct blobimpl_ {
    aug_blob blob_;
    augpy_box box_;
    int refs_;
    PyObject* pyob_;
};

static void*
castblob_(aug_blob* ob, const char* id)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_blobid)) {
        aug_retain(&impl->blob_);
        return &impl->blob_;
    } else if (AUG_EQUALID(id, augpy_boxid)) {
        aug_retain(&impl->box_);
        return &impl->box_;
    }
    return NULL;
}

static void
retainblob_(aug_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    ++impl->refs_;
}

static void
releaseblob_(aug_blob* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    if (0 == --impl->refs_) {
        Py_DECREF(impl->pyob_);
        free(impl);
    }
}

static const char*
getblobtype_(aug_blob* ob)
{
    return "application/octet-stream";
}

static const void*
getblobdata_(aug_blob* ob, size_t* size)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, blob_, ob);
    const void* data;
    int len;

    if (-1 == PyObject_AsReadBuffer(impl->pyob_, &data, &len)) {
        if (size)
            *size = 0;
        return NULL;
    }

    if (size)
        *size = len;
    return data;
}

static size_t
getblobsize_(aug_blob* ob)
{
    size_t size;
    getblobdata_(ob, &size);
    return size;
}

static const struct aug_blobvtbl blobvtbl_ = {
    castblob_,
    retainblob_,
    releaseblob_,
    getblobtype_,
    getblobdata_,
    getblobsize_
};

static void*
castboxblob_(augpy_box* ob, const char* id)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, box_, ob);
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, augpy_boxid)) {
        aug_retain(&impl->box_);
        return &impl->box_;
    } else if (AUG_EQUALID(id, aug_blobid)) {
        aug_retain(&impl->blob_);
        return &impl->blob_;
    }
    return NULL;
}

static void
retainboxblob_(augpy_box* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, box_, ob);
    ++impl->refs_;
}

static void
releaseboxblob_(augpy_box* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, box_, ob);
    if (0 == --impl->refs_) {
        Py_DECREF(impl->pyob_);
        free(impl);
    }
}

static PyObject*
unboxblob_(augpy_box* ob)
{
    struct blobimpl_* impl = AUG_PODIMPL(struct blobimpl_, box_, ob);
    Py_INCREF(impl->pyob_);
    return impl->pyob_;
}

static const struct augpy_boxvtbl boxblobvtbl_ = {
    castboxblob_,
    retainboxblob_,
    releaseboxblob_,
    unboxblob_
};

aug_blob*
augpy_createblob(PyObject* pyob)
{
    struct blobimpl_* impl = malloc(sizeof(struct blobimpl_));
    if (!impl)
        return NULL;

    if (!pyob)
        pyob = Py_None;

    impl->blob_.vtbl_ = &blobvtbl_;
    impl->blob_.impl_ = NULL;

    impl->box_.vtbl_ = &boxblobvtbl_;
    impl->box_.impl_ = NULL;

    impl->refs_ = 1;
    impl->pyob_ = pyob;

    Py_INCREF(impl->pyob_);
    return &impl->blob_;
}

PyObject*
augpy_obtopy(aug_object* ob)
{
    PyObject* py;
    augpy_box* tmp;
    if (ob && (tmp = aug_cast(ob, augpy_boxid))) {
        py = tmp->vtbl_->unbox_(tmp);
        aug_release(tmp);
    } else
        py = NULL;
    return py;
}

/* Implementation note: always reassign members before decrementing reference
   counts. */

static int handles_ = 0;

typedef struct {
    PyObject_HEAD
    char name_[MOD_MAXNAME + 1];
    mod_id id_;
    PyObject* ob_;
} handle_;

static PyMemberDef members_[] = {
    {
        "ob", T_OBJECT_EX, offsetof(handle_, ob_), 0, "TODO"
    },
    { NULL }
};

static int
clear_(handle_* self)
{
    PyObject* tmp;

    tmp = self->ob_;
    self->ob_ = NULL;
    Py_DECREF(tmp);

    return 0;
}

static void
dealloc_(handle_* self)
{
    --handles_;
    mod_writelog(MOD_LOGDEBUG, "deallocated: <augpy.Handle at %p, id=%d>",
                 (void*)self, (int)self->id_);

    clear_(self);
    self->ob_type->tp_free((PyObject*)self);
}

static int
compare_(handle_* lhs, handle_* rhs)
{
    int ret;
    if (lhs->id_ < rhs->id_)
        ret = -1;
    else if (lhs->id_ > rhs->id_)
        ret = 1;
    else
        ret = 0;
    return ret;
}

static PyObject*
repr_(handle_* self)
{
    return PyString_FromFormat("<augpy.Handle at %p, id=%d>",
                               (void*)self, (int)self->id_);
}

static long
hash_(handle_* self)
{
    /* Must not return -1. */

    return -1 == self->id_ ? 0 : self->id_;
}

static PyObject*
str_(handle_* self)
{
    return PyString_FromFormat("%d", (int)self->id_);
}

static int
traverse_(handle_* self, visitproc visit, void* arg)
{
    int ret;

    if (self->ob_) {
        ret = visit(self->ob_, arg);
        if (ret != 0)
            return ret;
    }

    return 0;
}

static int
init_(handle_* self, PyObject* args, PyObject* kwds)
{
    PyObject* ob = NULL, * tmp;

    static char* kwlist[] = { "id", "ob", NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i|O", kwlist, &self->id_,
                                     &ob))
        return -1;

    if (ob) {
        tmp = self->ob_;
        Py_INCREF(ob);
        self->ob_ = ob;
        Py_DECREF(tmp);
    }

    return 0;
}

static PyObject*
new_(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    handle_* self;

    self = (handle_*)type->tp_alloc(type, 0);
    if (self) {

        self->id_ = 0;

        Py_INCREF(Py_None);
        self->ob_ = Py_None;
    }

    ++handles_;
    mod_writelog(MOD_LOGDEBUG, "allocated: <augpy.Handle at %p, id=%d>",
                 (void*)self, (int)self->id_);
    return (PyObject*)self;
}

static PyObject*
getid_(handle_* self, void *closure)
{
    return Py_BuildValue("i", self->id_);
}

static PyGetSetDef getset_[] = {
    {
        "id", (getter)getid_, NULL, "TODO", NULL
    },
    { NULL }  /* Sentinel */
};

static PyTypeObject pytype_ = {
    PyObject_HEAD_INIT(NULL)
    0,                       /*ob_size*/
    "augpy.Handle",          /*tp_name*/
    sizeof(handle_),         /*tp_basicsize*/
    0,                       /*tp_itemsize*/
    (destructor)dealloc_,    /*tp_dealloc*/
    0,                       /*tp_print*/
    0,                       /*tp_getattr*/
    0,                       /*tp_setattr*/
    (cmpfunc)compare_,       /*tp_compare*/
    (reprfunc)repr_,         /*tp_repr*/
    0,                       /*tp_as_number*/
    0,                       /*tp_as_sequence*/
    0,                       /*tp_as_mapping*/
    (hashfunc)hash_,         /*tp_hash */
    0,                       /*tp_call*/
    (reprfunc)str_,          /*tp_str*/
    0,                       /*tp_getattro*/
    0,                       /*tp_setattro*/
    0,                       /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE
    | Py_TPFLAGS_HAVE_GC,    /*tp_flags*/
    "TODO",                  /* tp_doc */
    (traverseproc)traverse_, /* tp_traverse */
    (inquiry)clear_,         /* tp_clear */
    0,                       /* tp_richcompare */
    0,                       /* tp_weaklistoffset */
    0,                       /* tp_iter */
    0,                       /* tp_iternext */
    0,                       /* tp_methods */
    members_,                /* tp_members */
    getset_,                 /* tp_getset */
    0,                       /* tp_base */
    0,                       /* tp_dict */
    0,                       /* tp_descr_get */
    0,                       /* tp_descr_set */
    0,                       /* tp_dictoffset */
    (initproc)init_,         /* tp_init */
    0,                       /* tp_alloc */
    new_,                    /* tp_new */
};

PyTypeObject*
augpy_createtype(void)
{
    if (-1 == PyType_Ready(&pytype_))
        return NULL;

    Py_INCREF(&pytype_);
    return &pytype_;
}

PyObject*
augpy_createhandle(PyTypeObject* type, mod_id id, PyObject* ob)
{
    handle_* self = PyObject_GC_New(handle_, type);
    if (!self)
        return NULL;

    self->id_ = id;

    if (!ob)
        ob = Py_None;
    Py_INCREF(ob);
    self->ob_ = ob;

    ++handles_;
    mod_writelog(MOD_LOGDEBUG, "allocated: <augpy.Handle at %p, id=%d>",
                 (void*)self, (int)self->id_);
    return (PyObject*)self;
}

augpy_box*
augpy_boxhandle(PyTypeObject* type, mod_id id, PyObject* ob)
{
    PyObject* pyob = augpy_createhandle(type, id, ob);
    augpy_box* box = NULL;
    if (pyob) {
        box = augpy_createbox(pyob);
        Py_DECREF(pyob);
    }
    return box;
}

void
augpy_setid(PyObject* self, mod_id id)
{
    handle_* x = (handle_*)self;
    x->id_ = id;
}

mod_id
augpy_getid(PyObject* self)
{
    handle_* x = (handle_*)self;
    return x->id_;
}

void
augpy_setob(PyObject* self, PyObject* ob)
{
    handle_* x = (handle_*)self;
    PyObject* tmp = x->ob_;
    Py_INCREF(ob);
    x->ob_ = ob;
    Py_DECREF(tmp);
}

PyObject*
augpy_getob(PyObject* self)
{
    handle_* x = (handle_*)self;
    Py_INCREF(x->ob_);
    return x->ob_;
}

int
augpy_handles(void)
{
    return handles_;
}
