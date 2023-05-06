static const char *dialog_file_open = "\
{ \n\
 \"info\": {\"version\": 1, \"license\": \"LGPL-2.0-or-later\", \"author\": \"Cyril Hrubis <metan@ucw.cz>\"}, \n\
 \"layout\": { \n\
  \"rows\": 3, \n\
  \"widgets\": [ \n\
    { \n\
      \"cols\": 3, \n\
      \"halign\": \"fill\", \n\
      \"border\": \"none\", \n\
      \"cfill\": \"0, 1, 0\", \n\
      \"widgets\": [ \n\
        {\"type\": \"label\", \"text\": \"Directory:\"}, \n\
        {\"type\": \"tbox\", \"len\": 75, \"halign\": \"fill\", \"uid\": \"path\", \"on_event\": \"path\", \"ttype\": \"path\"}, \n\
	{\"type\": \"button\", \"btype\": \"home\", \"on_event\": \"home\"} \n\
      ] \n\
    }, \n\
    { \n\
     \"type\": \"table\", \n\
     \"focused\": true, \n\
     \"align\": \"fill\", \n\
     \"min_rows\": 25, \n\
     \"uid\": \"files\", \n\
     \"col_ops\": \"file_table\", \n\
     \"header\": [ \n\
      {\"label\": \"File\", \"id\": \"name\", \"min_size\": 20, \"fill\": 1}, \n\
      {\"label\": \"Size\", \"id\": \"size\", \"min_size\": 7}, \n\
      {\"label\": \"Modified\", \"id\": \"mod_time\", \"min_size\": 7} \n\
     ] \n\
    }, \n\
    { \n\
      \"cols\": 5, \n\
      \"border\": \"none\", \n\
      \"halign\": \"fill\", \n\
      \"cfill\": \"0, 8, 0, 0, 0\", \n\
      \"cpadf\": \"0, 0, 1, 1, 0, 0\", \n\
      \"widgets\": [ \n\
        {\"type\": \"label\", \"text\": \"Filter:\"}, \n\
        {\"type\": \"tbox\", \"len\": 20, \"uid\": \"filter\", \"halign\": \"fill\", \"on_event\": \"filter\"}, \n\
        {\"type\": \"checkbox\", \"label\": \"Show Hidden\", \"uid\": \"hidden\"}, \n\
        {\"type\": \"button\", \"label\": \"Cancel\", \"btype\": \"cancel\", \"on_event\": \"cancel\"}, \n\
        {\"type\": \"button\", \"label\": \"Open\", \"btype\": \"open\", \"uid\": \"open\", \"on_event\": \"open\"} \n\
      ] \n\
    } \n\
  ] \n\
 } \n\
} \n\
";
