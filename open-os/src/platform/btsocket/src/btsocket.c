/* Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <Python.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>

#include "bluetooth.h"
#include "l2cap.h"
#include "rfcomm.h"
#include "hci.h"
#include "sco.h"

#if PY_MAJOR_VERSION < 3
  #define MOD_ERROR_VAL
  #define MOD_SUCCESS_VAL(val)
  #define MOD_INIT(name) void init##name(void)
  #define MOD_DEF(ob, name, doc, methods) \
          ob = Py_InitModule3(name, methods, doc);
#else
  #define PyInt_AS_LONG PyLong_AS_LONG
  #define PyInt_Check PyLong_Check
  #define MOD_ERROR_VAL NULL
  #define MOD_SUCCESS_VAL(val) val
  #define MOD_INIT(name) PyMODINIT_FUNC PyInit_##name(void)
  #define MOD_DEF(ob, name, doc, methods) \
          static struct PyModuleDef moduledef = { \
            PyModuleDef_HEAD_INIT, name, doc, -1, methods, }; \
          ob = PyModule_Create(&moduledef);
#endif

static PyObject *_btsocket_error;
static PyObject *_btsocket_timeout;

union bt_sockaddr {
  struct sockaddr_l2 l2;
  struct sockaddr_rc rc;
  struct sockaddr_hci hci;
  struct sockaddr_sco sco;
};

static int _get_fileno(PyObject *socket) {
  PyObject *fileno_object;
  long fileno;

  fileno_object = PyObject_CallMethod(socket, "fileno", "", NULL);
  if (!fileno_object)
    return -1;

  if (!PyInt_Check(fileno_object))
    return -1;

  /* PyInt are longer than your average int, truncate back to an int */
  fileno = PyInt_AS_LONG(fileno_object);
  Py_DECREF(fileno_object);
  return (int)fileno;
}

static int _get_timeout(PyObject *socket, double *timeout) {
  PyObject *timeout_object;

  timeout_object = PyObject_CallMethod(socket, "gettimeout", "", NULL);
  if (!timeout_object)
    return 0;

  if (!PyFloat_Check(timeout_object))
    return 0;

  *timeout = PyFloat_AS_DOUBLE(timeout_object);
  Py_DECREF(timeout_object);
  return 1;
}

static int _setbdaddr(const char *straddr, bdaddr_t *bdaddr) {
  unsigned int b0, b1, b2, b3, b4, b5;
  int n;

  n = sscanf(straddr, "%X:%X:%X:%X:%X:%X", &b5, &b4, &b3, &b2, &b1, &b0);
  if (n == 6) {
    bdaddr->b[0] = b0;
    bdaddr->b[1] = b1;
    bdaddr->b[2] = b2;
    bdaddr->b[3] = b3;
    bdaddr->b[4] = b4;
    bdaddr->b[5] = b5;
    return 1;
  } else {
    PyErr_SetString(_btsocket_error, "bad bluetooth address for bind()");
    return 0;
  }
}

static PyObject *parsesocketargs(PyObject *self, PyObject *args,
                                 union bt_sockaddr *addr, size_t *addrlen) {
  PyObject *socket_object, *tuple;
  int proto;

  if (!PyArg_ParseTuple(args, "OiO:_btsocket.parsesocketargs",
                        &socket_object, &proto, &tuple))
    return NULL;

  memset(addr, 0, sizeof (union bt_sockaddr));

  switch (proto) {
    case BTPROTO_L2CAP: {
      char *straddr;

      addr->l2.l2_family = AF_BLUETOOTH;

      if (!PyArg_ParseTuple(tuple, "si:_btsocket.parsesocketargs",
                            &straddr, &addr->l2.l2_psm))
        return NULL;
      if (!_setbdaddr(straddr, &addr->l2.l2_bdaddr))
        return NULL;

      *addrlen = sizeof (struct sockaddr_l2);
      break;
    }
    case BTPROTO_RFCOMM: {
      char *straddr;

      addr->rc.rc_family = AF_BLUETOOTH;

      if (!PyArg_ParseTuple(tuple, "si:_btsocket.parsesocketargs",
                            &straddr, &addr->rc.rc_channel))
        return NULL;
      if (!_setbdaddr(straddr, &addr->rc.rc_bdaddr))
        return NULL;

      *addrlen = sizeof (struct sockaddr_rc);
      break;
    }
    case BTPROTO_HCI: {
      addr->hci.hci_family = AF_BLUETOOTH;

      if (!PyArg_ParseTuple(tuple, "HH:_btsocket.parsesocketargs",
                            &addr->hci.hci_dev, &addr->hci.hci_channel))
        return NULL;

      *addrlen = sizeof (struct sockaddr_hci);
      break;
    }
    case BTPROTO_SCO: {
      char *straddr;

      addr->sco.sco_family = AF_BLUETOOTH;

      if (!PyArg_ParseTuple(tuple, "s:_btsocket.parsesocketargs",
                            &straddr))
        return NULL;
      if (!_setbdaddr(straddr, &addr->sco.sco_bdaddr))
        return NULL;

      *addrlen = sizeof (struct sockaddr_sco);
      break;
    }
    default:
      PyErr_SetString(_btsocket_error, "unknown protocol");
      return NULL;
  }

  return socket_object;
}

static PyObject *_btsocket_bind(PyObject *self, PyObject *args) {
  PyObject *socket_object;
  union bt_sockaddr addr;
  size_t addrlen;
  int fd, result;

  socket_object = parsesocketargs(self, args, &addr, &addrlen);
  if (!socket_object)
    return NULL;

  fd = _get_fileno(socket_object);
  if (fd < 0)
    return NULL;

  Py_BEGIN_ALLOW_THREADS;
  result = bind(fd, (struct sockaddr *)&addr, addrlen);
  Py_END_ALLOW_THREADS;

  if (result < 0)
    return PyErr_SetFromErrno(_btsocket_error);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_btsocket_connect(PyObject *self, PyObject *args) {
  PyObject *socket_object;
  union bt_sockaddr addr;
  size_t addrlen;
  int fd, timeout = 0, did_timeout = 0, result;
  double timeout_secs;

  socket_object = parsesocketargs(self, args, &addr, &addrlen);
  if (!socket_object)
    return NULL;

  fd = _get_fileno(socket_object);
  if (fd < 0)
    return NULL;

  timeout = _get_timeout(socket_object, &timeout_secs);

  Py_BEGIN_ALLOW_THREADS;
  result = connect(fd, (struct sockaddr *)&addr, addrlen);

  if (timeout) {
    struct pollfd pollfd;
    int timeout_ms;
    int n;

    pollfd.fd = fd;
    pollfd.events = POLLIN;

    /* timeout limits are set by Python */
    timeout_ms = (int)(timeout_secs * 1000);
    n = poll(&pollfd, 1, timeout_ms);
    if (n == 0) {
      did_timeout = 1;
    } else if (n < 0) {
      result = n;
    } else {
      /* result from connect() is EINPROGRESS, get the real error */
      socklen_t resultlen = sizeof result;
      (void)getsockopt(fd, SOL_SOCKET, SO_ERROR, &result, &resultlen);
      if (result == EISCONN)
        result = 0;
      else {
        errno = result;
        result = -1;
      }
    }
  }
  Py_END_ALLOW_THREADS;

  if (did_timeout) {
    PyErr_SetString(_btsocket_timeout, "timed out");
    return NULL;
  }
  if (result < 0) {
    PyErr_SetFromErrno(_btsocket_error);
    return NULL;
  }

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *_btsocket_recvmsg(PyObject *self, PyObject *args) {
  PyObject *socket_object, *buffers, *iterator, *cmsg_list = NULL, *addrval;
  PyObject *retval = NULL;
  Py_ssize_t controllen = 0, nbuffers, buf_index = 0;
  int flags = 0, fd, i, timeout = 0, did_timeout = 0;
  double timeout_secs;
  struct iovec *iovs = NULL;
  Py_buffer *bufs = NULL;
  void *controlbuf = NULL;
  struct sockaddr_hci addr;
  struct msghdr msg = {0};
  ssize_t len;
  struct cmsghdr *cmsgh;

  /* Parse arguments, allocating an iovec array matching the incoming buffers
     list and a matching PyBuffer for each one that we can fetch the incoming
     buffer into for receiving. */
  if (!PyArg_ParseTuple(args, "OO|ni:_btsocket.recvmsg_into",
                        &socket_object, &buffers, &controllen, &flags))
    return NULL;

  fd = _get_fileno(socket_object);
  if (fd < 0)
    return NULL;

  timeout = _get_timeout(socket_object, &timeout_secs);

  iterator = PySequence_Fast(buffers, ("recvmsg_into() argument 1 must be an "
                                   "iterable"));
  if (!iterator)
    return NULL;

  nbuffers = PySequence_Fast_GET_SIZE(iterator);
  if (nbuffers > INT_MAX) {
    PyErr_SetString(_btsocket_error, "recvmsg_into() argument 1 is too long");
    goto finally;
  }

  if (nbuffers > 0) {
    iovs = PyMem_New(struct iovec, nbuffers);
    bufs = PyMem_New(Py_buffer, nbuffers);

    if (!iovs || !bufs) {
      PyErr_NoMemory();
      goto finally;
    }
  }

  for (buf_index = 0; buf_index < nbuffers; ++buf_index) {
    if (!PyArg_Parse(PySequence_Fast_GET_ITEM(iterator, buf_index),
                     ("w*;recvmsg_into() argument 1 must be an iterable "
                      "of single-segment read-write buffers"),
                     &bufs[buf_index]))
      goto finally;

    iovs[buf_index].iov_base = bufs[buf_index].buf;
    iovs[buf_index].iov_len = bufs[buf_index].len;
  }

  /* Allocate a control buffer large enough to receive ancillary data. */
  if (controllen < 0 || controllen > INT_MAX) {
    PyErr_SetString(_btsocket_error, "recvmsg_into() argument 2 invalid");
    goto finally;
  }

  if (controllen > 0) {
    controlbuf = PyMem_Malloc(controllen);
    if (!controlbuf) {
      PyErr_NoMemory();
      goto finally;
    }
  }

  /* Receive data on the socket. */
  Py_BEGIN_ALLOW_THREADS;
  msg.msg_name = (struct sockaddr *)&addr;
  msg.msg_namelen = sizeof addr;
  msg.msg_iov = iovs;
  msg.msg_iovlen = nbuffers;
  msg.msg_control = controlbuf;
  msg.msg_controllen = controllen;

  if (timeout) {
    struct pollfd pollfd;
    int timeout_ms;
    int n;

    pollfd.fd = fd;
    pollfd.events = POLLIN;

    /* timeout limits are set by Python */
    timeout_ms = (int)(timeout_secs * 1000);
    n = poll(&pollfd, 1, timeout_ms);
    if (n <= 0)
      did_timeout = 1;
  }

  if (!did_timeout)
    len = recvmsg(fd, &msg, flags);
  Py_END_ALLOW_THREADS;

  if (did_timeout) {
    PyErr_SetString(_btsocket_timeout, "timed out");
    goto finally;
  }
  if (len < 0) {
    PyErr_SetFromErrno(_btsocket_error);
    goto finally;
  }

  /* Parse control message data into a list we pass in the return value. */
  cmsg_list = PyList_New(0);
  if (!cmsg_list)
    goto finally;

  for (cmsgh = CMSG_FIRSTHDR(&msg); cmsgh != NULL;
       cmsgh = CMSG_NXTHDR(&msg, cmsgh)) {
    size_t cmsgdatalen;
    PyObject *bytes, *tuple;
    int tmp;

    /* cmsg_len includes the length of the 'struct cmsghdr' and any padding
       the kernel sees fit to add. CMSG_LEN(0) gives the value that cmsg_len
       would have if there were 0 bytes of additional data. Thus by doing
       cmsg_len - CMSG_LEN(0) we get the actual length of data.

       Never let kernel engineers design APIs. */
    cmsgdatalen = cmsgh->cmsg_len - CMSG_LEN(0);
    /* bytes is refcounted, if NULL the Py_BuildValue will fail and tuple will
       be NULL too - which we catch. If it succeeds, the refcount will be 0
       until Py_BuildValue succeeds. So there's no need to explicitly free or
       decref bytes. */
    bytes = PyBytes_FromStringAndSize((char *)CMSG_DATA(cmsgh), cmsgdatalen);
    tuple = Py_BuildValue("iiN",
                          (int)cmsgh->cmsg_level, (int)cmsgh->cmsg_type, bytes);
    if (tuple == NULL)
      goto finally;

    tmp = PyList_Append(cmsg_list, tuple);
    Py_DECREF(tuple);
    if (tmp != 0)
      goto finally;
  }

  /* Build the rest of the return value. */
  addrval = Py_BuildValue("ii", addr.hci_dev, addr.hci_channel);
  if (!addrval)
    goto finally;

  retval = Py_BuildValue("NOiN",
                         PyLong_FromSsize_t(len),
                         cmsg_list,
                         (int)msg.msg_flags,
                         addrval);

  /* Clean up in success cases as well as error. */
finally:
  Py_XDECREF(cmsg_list);
  PyMem_Free(controlbuf);
  /* We can abort out of the allocation loop only, so buffers will only be
     allocated up to buf_index not nbuffers. */
  for (i = 0; i < buf_index; ++i)
    PyBuffer_Release(&bufs[i]);
  PyMem_Free(bufs);
  PyMem_Free(iovs);
  Py_DECREF(iterator);
  return retval;
}

static PyObject *_btsocket_listen_and_accept() {
  int sk, nsk, result;
  struct sockaddr_l2 srcaddr, addr;
  socklen_t optlen;
  struct bt_security btsec;
  bdaddr_t *ba;
  char str[18];

  Py_BEGIN_ALLOW_THREADS;
  sk = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
  Py_END_ALLOW_THREADS;
  if (sk < 0)
    goto fail;

  /* Set up source address */
  memset(&srcaddr, 0, sizeof(srcaddr));
  srcaddr.l2_family = AF_BLUETOOTH;
  srcaddr.l2_cid = htobs(ATT_CID);
  srcaddr.l2_bdaddr_type = BDADDR_LE_PUBLIC;

  Py_BEGIN_ALLOW_THREADS;
  result = bind(sk, (struct sockaddr *) &srcaddr, sizeof(srcaddr));
  Py_END_ALLOW_THREADS;

  if (result < 0)
    goto fail;

  /* Set the security level */
  memset(&btsec, 0, sizeof(btsec));
  btsec.level = BT_SECURITY_LOW;

  Py_BEGIN_ALLOW_THREADS;
  result = setsockopt(sk, SOL_BLUETOOTH, BT_SECURITY, &btsec, sizeof(btsec));
  Py_END_ALLOW_THREADS;

  if (result != 0)
    goto fail;

  Py_BEGIN_ALLOW_THREADS;
  result = listen(sk, 10);
  Py_END_ALLOW_THREADS;

  if (result < 0)
    goto fail;

  memset(&addr, 0, sizeof(addr));
  optlen = sizeof(addr);

  Py_BEGIN_ALLOW_THREADS;
  nsk = accept(sk, (struct sockaddr *) &addr, &optlen);
  Py_END_ALLOW_THREADS;

  if (nsk < 0)
    goto fail;

  ba = &addr.l2_bdaddr;
  sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
          ba->b[5], ba->b[4], ba->b[3], ba->b[2], ba->b[1], ba->b[0]);

  close(sk);
  return Py_BuildValue("(i,s)", nsk, str);

fail:
  if (sk >= 0)
    close(sk);

  return PyErr_SetFromErrno(_btsocket_error);
}

static PyMethodDef _btsocket_methods[] = {
  { "bind", _btsocket_bind, METH_VARARGS,
    "Bind a Bluetooth socket to a device and channel" },
  { "connect", _btsocket_connect, METH_VARARGS,
    "Connect a Bluetooth socket to a remote address" },
  { "recvmsg", _btsocket_recvmsg, METH_VARARGS,
    "Receive normal and ancillary data from a Bluetooth socket" },
  { "listen_and_accept", _btsocket_listen_and_accept, METH_NOARGS,
    "Create a socket for incoming BLE connection" },

  { NULL, NULL, 0, NULL }
};

MOD_INIT(_btsocket)
{
  PyObject *m;

  MOD_DEF(m, "_btsocket", "Bluetooth socket API", _btsocket_methods);
  if (!m)
    return MOD_ERROR_VAL;

  _btsocket_error = PyErr_NewException("btsocket.error",
                                       PyExc_OSError, NULL);
  Py_INCREF(_btsocket_error);
  PyModule_AddObject(m, "error", _btsocket_error);

  _btsocket_timeout = PyErr_NewException("btsocket.timeout",
                                         PyExc_OSError, NULL);
  Py_INCREF(_btsocket_timeout);
  PyModule_AddObject(m, "timeout", _btsocket_timeout);

  return MOD_SUCCESS_VAL(m);
}
