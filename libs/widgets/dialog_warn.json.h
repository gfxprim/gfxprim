static const char *dialog_warn = "\
{ \
 \"version\": 1, \
 \"widgets\": [ \
  { \
   \"type\": \"frame\", \
   \"label\": \"Warning\", \
   \"widget\": { \
    \"rows\": 2, \
    \"widgets\": [ \
     { \
      \"cols\": 2, \
      \"frame\": \"none\", \
      \"widgets\": [ \
       { \
        \"type\": \"stock\", \
        \"stock\": \"warn\" \
       }, \
       { \
        \"type\": \"label\", \
        \"uid\": \"text\" \
       } \
      ] \
     }, \
     { \
      \"type\": \"button\", \
      \"label\": \"OK\", \
      \"uid\": \"btn_ok\" \
     } \
    ] \
   } \
  } \
 ] \
} \
";
