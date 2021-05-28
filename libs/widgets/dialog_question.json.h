static const char *dialog_question = "\
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
        \"stock\": \"question\" \n\
       }, \n\
       { \n\
        \"type\": \"label\", \n\
        \"uid\": \"text\" \n\
       } \n\
      ] \n\
     }, \n\
     { \n\
      \"cols\": 2, \n\
      \"halign\": \"fill\", \n\
      \"cpadf\": \"1, 1, 1\", \n\
      \"cfill\": \"0, 0\", \n\
      \"border\": \"none\", \n\
      \"uniform\": true, \n\
      \"widgets\": [ \n\
       { \n\
        \"type\": \"button\", \n\
	\"halign\": \"fill\", \n\
        \"label\": \"No\", \n\
	\"btype\": \"cancel\", \n\
        \"uid\": \"btn_no\" \n\
       }, \n\
       { \n\
        \"type\": \"button\", \n\
	\"halign\": \"fill\", \n\
        \"label\": \"Yes\", \n\
	\"btype\": \"ok\", \n\
        \"uid\": \"btn_yes\" \n\
       } \n\
      ] \n\
     } \n\
    ] \n\
   } \n\
  } \n\
 ] \n\
} \n\
";
