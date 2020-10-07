import sys

from . import protogen
from . import pm

def decode_store_info(msg):
    store_info = protogen.stores_pb2.MxStoreInfo()
    store_info.ParseFromString(msg)
    return store_info

def decode_store(store_info):
    type_str = protogen.stores_pb2.MxStoreType.Name(store_info.type).lower()
    type_module = sys.modules['harness.protogen.' + type_str + '_pb2']
    store = getattr(type_module, 'Mx' + type_str.capitalize() + 'Store')()
    store.ParseFromString(store_info.msg)
    return store
