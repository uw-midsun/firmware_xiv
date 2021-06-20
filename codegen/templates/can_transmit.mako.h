<% from data import parse_can_frames %> \
<% from constants import NUM_FIELDS %> \
#pragma once

#include <stddef.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_pack.h"

<% can_frames = parse_can_frames(options.filename) %> 
% for id, frame in can_frames.items():

#define CAN_TRANSMIT_${frame.msg_name}( \
    % if frame.is_critical:
      ack_ptr \
    % endif
    % for i, field in enumerate(frame.fields): 
      % if i > 0 or frame.is_critical:
        , \
      % endif
      ${field}_${frame.ftype} \
    % endfor 
    ) \
    ({ \
    CanMessage msg = { 0 }; \
    CAN_PACK_${frame.msg_name}(&msg \
    % for field in frame.fields: 
      , (${field}_${frame.ftype}) \
    % endfor
    ); \
    StatusCode status = can_transmit(&msg \
    % if frame.is_critical:
      , (ack_ptr) \
    % else:
      , NULL \
    % endif
    ); \
    status; \
    })
% endfor
