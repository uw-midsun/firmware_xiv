<% from data import parse_can_frames %> \
<% from constants import NUM_FIELDS %> \
#pragma once

#include "can_msg_defs.h"
#include "can_unpack_impl.h"

<% can_frames = parse_can_frames(options.filename) %> 
% for id, frame in can_frames.items():

#define CAN_UNPACK_${frame.msg_name}(msg_ptr \
    % for field in frame.fields: 
      , ${field}_${frame.ftype}_ptr \
    % endfor 
    ) \
    can_unpack_impl_${frame.ftype}((msg_ptr), ${frame.dlc} \
    % for field in frame.fields:
      , (${field}_${frame.ftype}_ptr) \
    % endfor
    % for _ in range(0, NUM_FIELDS[frame.ftype] - len(frame.fields)):
      , CAN_UNPACK_IMPL_EMPTY \
    % endfor
    )
% endfor
