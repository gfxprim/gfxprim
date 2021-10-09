static const char *dialog_input = "\
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
       {\"type\": \"stock\", \"uid\": \"stock\", \"stock\": \"question\"}, \n\
       {\"type\": \"tbox\", \"uid\": \"input\", \"len\": 15} \n\
      ] \n\
     }, \n\
     {\"cols\": 2, \n\
      \"halign\": \"fill\", \n\
      \"cpadf\": \"1, 1, 1\", \n\
      \"cfill\": \"0, 0\", \n\
      \"border\": \"none\", \n\
      \"uniform\": true, \n\
      \"widgets\": [ \n\
       {\"type\": \"button\", \"halign\": \"fill\", \"label\": \"Cancel\", \"btype\": \"cancel\", \"uid\": \"btn_cancel\"}, \n\
       {\"type\": \"button\", \"halign\": \"fill\", \"label\": \"OK\", \"btype\": \"ok\", \"uid\": \"btn_ok\"} \n\
      ] \n\
     } \n\
    ] \n\
   } \n\
  } \n\
 ] \n\
} \n\
";
