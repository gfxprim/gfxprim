static const char *dialog_err = "\
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
        \"stock\": \"err\" \n\
       }, \n\
       { \n\
        \"type\": \"label\", \n\
        \"uid\": \"text\" \n\
       } \n\
      ] \n\
     }, \n\
     {\"type\": \"button\", \"label\": \"OK\", \"uid\": \"btn_ok\", \"focused\": true} \n\
    ] \n\
   } \n\
  } \n\
 ] \n\
} \n\
";
