# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: scores.proto

from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='scores.proto',
  package='',
  syntax='proto2',
  serialized_options=None,
  create_key=_descriptor._internal_create_key,
  serialized_pb=b'\n\x0cscores.proto\"\x1f\n\rScoresMessage\x12\x0e\n\x06scores\x18\x01 \x03(\x05'
)




_SCORESMESSAGE = _descriptor.Descriptor(
  name='ScoresMessage',
  full_name='ScoresMessage',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  create_key=_descriptor._internal_create_key,
  fields=[
    _descriptor.FieldDescriptor(
      name='scores', full_name='ScoresMessage.scores', index=0,
      number=1, type=5, cpp_type=1, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR,  create_key=_descriptor._internal_create_key),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto2',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=16,
  serialized_end=47,
)

DESCRIPTOR.message_types_by_name['ScoresMessage'] = _SCORESMESSAGE
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

ScoresMessage = _reflection.GeneratedProtocolMessageType('ScoresMessage', (_message.Message,), {
  'DESCRIPTOR' : _SCORESMESSAGE,
  '__module__' : 'scores_pb2'
  # @@protoc_insertion_point(class_scope:ScoresMessage)
  })
_sym_db.RegisterMessage(ScoresMessage)


# @@protoc_insertion_point(module_scope)
