import importlib

from mu.protogen import stores_pb2

MODULE_NAME_FORMAT = 'mu.protogen.{}_pb2'
STORE_TYPE_NAME_FORMAT = 'Mu{}Store'


def store_from_name(type_name):
    type_name = type_name.lower()
    # get class from module via introspection
    type_module = importlib.import_module(MODULE_NAME_FORMAT.format(type_name))
    store_class = getattr(type_module, STORE_TYPE_NAME_FORMAT.format(type_name.capitalize()))
    return store_class()


def store_from_enum(store_enum):
    # grab enum name from enum value for introspection
    type_name = stores_pb2.MuStoreType.Name(store_enum).lower()
    return store_from_name(type_name)


def decode_store_info(msg):
    store_info = stores_pb2.MuStoreInfo()
    store_info.ParseFromString(msg)
    return store_info


def decode_store(store_info):
    store = store_from_enum(store_info.type)
    store.ParseFromString(store_info.msg)
    return store


def full_mask(store):
    mask = store.__class__()
    for descriptor, val in store.ListFields():
        if hasattr(val, '__len__'):
            getattr(mask, descriptor.name).extend([1] * len(val))
        else:
            setattr(mask, descriptor.name, 1)
    return mask
