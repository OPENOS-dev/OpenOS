/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __MESSAGE_CLASS_HEADER_DEFINE_HEADER__
#define __MESSAGE_CLASS_HEADER_DEFINE_HEADER__

#include "command.h"
#include "message_interface.h"

ICeiMessage *create_message(ICeiMessage::MESSAGE_TYPE type, void *v);
ICeiMessage *create_message(ICeiMessage::MESSAGE_TYPE type, long long v);
ICeiMessage *create_message(ICeiMessage::MESSAGE_TYPE type, CStreamCmd *v);
ICeiMessage *create_message(ICeiMessage::MESSAGE_TYPE type, CSenseCmd *v);
ICeiMessage *create_message(ICeiMessage::MESSAGE_TYPE type, IUnknown *v);
#endif