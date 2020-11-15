import sys

from mpxe.protogen import stores_pb2
from mpxe.harness import pm

MODULE_NAME_FORMAT = 'mpxe.protogen.{}_pb2'
STORE_TYPE_NAME_FORMAT = 'Mx{}Store'

def decode_store_info(msg):
    store_info = stores_pb2.MxStoreInfo()
    store_info.ParseFromString(msg)
    return store_info

def decode_store(store_info):
    # grab enum name from enum value for introspection
    type_str = stores_pb2.MxStoreType.Name(store_info.type).lower()
    # get class from module via introspection
    type_module = sys.modules[MODULE_NAME_FORMAT.format(type_str)]
    store = getattr(type_module, STORE_TYPE_NAME_FORMAT.format(type_str.capitalize()))()
    store.ParseFromString(store_info.msg)
    return store
