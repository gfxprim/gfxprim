static const char *dialog_err = "\
{ \n\
 \"version\": 1, \n\
 \"widgets\": [ \n\
  {\"type\": \"frame\", \"uid\": \"title\", \n\
   \"widget\": { \n\
    \"rows\": 2, \n\
    \"widgets\": [ \n\
     { \n\
      \"cols\": 2, \n\
      \"widgets\": [ \n\
       {\"type\": \"stock\", \"stock\": \"err\"}, \n\
       {\"type\": \"label\", \"uid\": \"text\"} \n\
      ] \n\
     }, \n\
     {\"type\": \"button\", \"label\": \"OK\", \"on_event\": \"ok\", \"focused\": true} \n\
    ] \n\
   } \n\
  } \n\
 ] \n\
} \n\
";
