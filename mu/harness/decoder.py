import importlib

from mu.protogen import stores_pb2

MODULE_NAME_FORMAT = 'mu.protogen.{}_pb2'
STORE_TYPE_NAME_FORMAT = 'Mu{}Store'


def decode_store_info(msg):
    store_info = stores_pb2.MuStoreInfo()
    store_info.ParseFromString(msg)
    return store_info


def decode_store(store_info):
    # grab enum name from enum value for introspection
    type_str = stores_pb2.MuStoreType.Name(store_info.type).lower()
    # get class from module via introspection
    type_module = importlib.import_module(MODULE_NAME_FORMAT.format(type_str))
    store = getattr(type_module, STORE_TYPE_NAME_FORMAT.format(type_str.capitalize()))()
    store.ParseFromString(store_info.msg)
    return store
