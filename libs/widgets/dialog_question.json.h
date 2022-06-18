static const char *dialog_question = "\
{ \n\
 \"info\": {\"version\": 1, \"license\": \"LGPL-2.0-or-later\", \"author\": \"Cyril Hrubis <metan@ucw.cz>\"}, \n\
 \"layout\": { \n\
  \"widgets\": [ \n\
   {\"type\": \"frame\", \"uid\": \"title\", \n\
    \"widget\": { \n\
     \"rows\": 2, \n\
     \"widgets\": [ \n\
      { \n\
       \"cols\": 2, \n\
       \"widgets\": [ \n\
        {\"type\": \"stock\", \"stock\": \"question\"}, \n\
        {\"type\": \"label\", \"uid\": \"text\"} \n\
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
         \"on_event\": \"no\" \n\
        }, \n\
        { \n\
         \"type\": \"button\", \n\
         \"halign\": \"fill\", \n\
         \"label\": \"Yes\", \n\
         \"btype\": \"ok\", \n\
         \"on_event\": \"yes\", \n\
         \"focused\": true \n\
        } \n\
       ] \n\
      } \n\
     ] \n\
    } \n\
   } \n\
  ] \n\
 } \n\
} \n\
";
