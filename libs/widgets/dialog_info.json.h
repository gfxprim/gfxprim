static const char *dialog_info = "\
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
        {\"type\": \"stock\", \"stock\": \"info\"}, \n\
        {\"type\": \"label\", \"uid\": \"text\"} \n\
       ] \n\
      }, \n\
      {\"type\": \"button\", \"label\": \"OK\", \"on_event\": \"ok\", \"focused\": true} \n\
     ] \n\
    } \n\
   } \n\
  ] \n\
 } \n\
} \n\
";
