static const char *dialog_warn = "\
{ \n\
 \"version\": 1, \n\
 \"widgets\": [ \n\
  { \n\
   \"type\": \"frame\", \n\
   \"uid\": \"title\", \n\
   \"widget\": { \n\
    \"rows\": 2, \n\
    \"widgets\": [ \n\
     { \n\
      \"cols\": 2, \n\
      \"widgets\": [ \n\
       { \n\
        \"type\": \"stock\", \n\
        \"stock\": \"warn\" \n\
       }, \n\
       { \n\
        \"type\": \"label\", \n\
        \"uid\": \"text\" \n\
       } \n\
      ] \n\
     }, \n\
     { \n\
      \"type\": \"button\", \n\
      \"label\": \"OK\", \n\
      \"uid\": \"btn_ok\" \n\
     } \n\
    ] \n\
   } \n\
  } \n\
 ] \n\
} \n\
";
