import sys

from mpxe.protogen import stores_pb2
from mpxe.harness import pm

def decode_store_info(msg):
    store_info = stores_pb2.MxStoreInfo()
    store_info.ParseFromString(msg)
    return store_info

def decode_store(store_info):
    # grab enum name from enum value for introspection
    type_str = stores_pb2.MxStoreType.Name(store_info.type).lower()
    # get class from module via introspection
    type_module = sys.modules['mpxe.protogen.' + type_str + '_pb2']
    store = getattr(type_module, 'Mx' + type_str.capitalize() + 'Store')()
    store.ParseFromString(store_info.msg)
    return store
