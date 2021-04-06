<%doc>
Helps generating enum body content in the format:

    prefix_val0 = 0,
    prefix_val1 = 2,
    prefix_val2 = 5,
    NUM_prefix
    ...

Args:
    data: a dictionary of data in the range (empty values should have a value of None)
    prefix: the prefix to use
</%doc>
<%def name="generate_enum(data, prefix)"> \
  % for key, value in data.items():
    % if value != None:
      ${prefix}_${value} = ${key},
    % endif
  % endfor
  NUM_${prefix}S = ${len(data)} \
</%def>

<%doc>
Helps generating enum body content in the format:

    prefix_val0 type = 0,
    prefix_val1 type = 2,
    prefix_val2 type = 5,
    ...

Args:
    data: a dictionary of data in the range (empty values should have a value of None)
    prefix: the prefix to use
    go_type: the type in go
</%doc>
<%def name="generate_enum_go(data, prefix, go_type)"> \
  % for key, value in data.items():
    % if value != None:
    ${prefix}${''.join(x.title() for x in value.split('_'))} ${go_type} = ${key}
    % endif
  % endfor
</%def>
