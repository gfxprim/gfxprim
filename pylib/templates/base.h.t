%% extends "body.c.h"

%% block pre_body_guard
#ifndef {{ header_guard }}
#define {{ header_guard }}

%% endblock pre_body_guard

%% block post_body_guard
#endif  /* {{ header_guard }} */
%% endblock post_body_guard
