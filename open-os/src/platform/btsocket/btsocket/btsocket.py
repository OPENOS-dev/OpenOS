# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
import socket

from . import _btsocket
from . import constants

class BluetoothSocket(socket.socket):
    """Socket wrapper class for Bluetooth sockets.

    Wraps the Python socket class to implement the necessary pieces to be able
    to bind to the HCI monitor and control sockets as well as receive messages
    with ancillary data from them.

    The bind() and connect() functions match the behavior of those of the
    built-in socket methods and accept a tuple defining the address, the
    contents of which depend on the protocol of the socket.

    For BTPROTO_HCI this is:
      @param dev: Device index, or HCI_DEV_NONE.
      @param channel: Channel, e.g. HCI_CHANNEL_RAW.

    For BTPROTO_L2CAP this is:
      @param address: Address of device in string format.
      @param psm: PSM of L2CAP service.

    For BTPROTO_RFCOMM this is:
      @param address: Address of device in string format.
      @param cid: Channel ID of RFCOMM service.

    For BTPROTO_SCO this is:
      @param address: Address of device in string format.

    """

    def __init__(self,
                 family=constants.AF_BLUETOOTH,
                 type=socket.SOCK_RAW,
                 proto=constants.BTPROTO_HCI):
        super(BluetoothSocket, self).__init__(family, type, proto)

    def bind(self, *args):
        """Bind the socket to a local address."""
        if self.family == constants.AF_BLUETOOTH:
            return _btsocket.bind(self, self.proto, *args)
        else:
            return super(BluetoothSocket, self).bind(*args)

    def connect(self, *args):
        """Connect the socket to a remote address."""
        if self.family == constants.AF_BLUETOOTH:
            return _btsocket.connect(self, self.proto, *args)
        else:
            return super(BluetoothSocket, self).connect(*args)

    def recvmsg(self, bufsize, ancbufsize=0, flags=0):
        """Receive normal data and ancillary data from the socket.

        @param bufsize size in bytes of buffer for normal data.
        @param ancbufsize size in bytes of internal buffer for ancillary data.
        @param flags same meaning as socket.recv()

        @return tuple of (data, ancdata, msg_flags, address),
          @ancdata is zero or more tuples of (cmsg_level, cmsg_type, cmsg_data).

        """
        buffer = bytearray(bufsize)
        (nbytes, ancdata, msg_flags, address) = \
                self.recvmsg_into((buffer,), ancbufsize, flags)
        return (bytes(buffer), ancdata, msg_flags, address)

    def recvmsg_into(self, buffers, ancbufsize=0, flags=0):
        """Receive normal data and ancillary data from the socket.

        @param buffers iterable of bytearray objects filled with read chunks.
        @param ancbufsize size in bytes of internal buffer for ancillary data.
        @param flags same meaning as socket.recv()

        @return tuple of (nbytes, ancdata, msg_flags, address),
          @ancdata is zero or more tuples of (cmsg_level, cmsg_type, cmsg_data).

        """
        return _btsocket.recvmsg(self, buffers, ancbufsize, flags)


def create_le_gatt_server_socket():
    """Create a socket for incoming BLE connection.

    The function creates an LE socket, then does bind, listen and accept.
    The accept call blocks until it receives an incoming connection.

    Note, that resulting socket will be standard Python socket,
    not BluetoothSocket. It is already connected, ready for recv() and
    send() calls, and there is no need for further setup.

    Before calling this function, the machine must be set up (with appropriate
    HCI commands) to be advertising, be powered, and have LE turned on.

    @return tuple of (sock, source_address),
      @sock is standard Python socket object, which is able to receive and send.
      @source_address is string containing BDADDR of the connected remote.

    """
    file_descriptor, source_address = _btsocket.listen_and_accept()
    sock = socket.fromfd(file_descriptor, constants.AF_BLUETOOTH,
                         socket.SOCK_SEQPACKET, constants.BTPROTO_L2CAP)
    return sock, source_address
