from ..protogen import stores_pb2

def write_proj(proj, store_type, msg, mask):
    update = stores_pb2.MxStoreUpdate()
    update.key = 0
    update.type = store_type
    update.msg = msg.SerializeToString()
    update.mask = mask.SerializeToString()
    proj.write(update.SerializeToString())
