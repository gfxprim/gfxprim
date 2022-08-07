static const char *dialog_app_info = "\
{ \n\
 \"info\": {\"version\": 1, \"license\": \"LGPL-2.0-or-later\", \"author\": \"Cyril Hrubis <metan@ucw.cz>\"}, \n\
 \"layout\": { \n\
  \"widgets\": [ \n\
   {\"type\": \"frame\", \"uid\": \"title\", \n\
    \"widget\": { \n\
     \"rows\": 2, \n\
     \"widgets\": [ \n\
      { \n\
       \"rows\": 4, \n\
       \"uid\": \"app_info\", \n\
       \"widgets\": [ \n\
	{\"type\": \"label\", \"uid\": \"app_name\", \"tattr\": \"bold\"}, \n\
	{\"type\": \"label\", \"uid\": \"app_version\"}, \n\
	{\"type\": \"label\", \"uid\": \"app_desc\"}, \n\
	{\"type\": \"label\", \"uid\": \"app_url\"} \n\
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
