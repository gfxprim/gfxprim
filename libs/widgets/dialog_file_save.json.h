static const char *dialog_file_save = "\
{ \n\
 \"info\": {\"version\": 1, \"license\": \"LGPL-2.0-or-later\", \"author\": \"Cyril Hrubis <metan@ucw.cz>\"}, \n\
 \"layout\": { \n\
  \"rows\": 3, \n\
  \"widgets\": [ \n\
    { \n\
      \"cols\": 3, \n\
      \"halign\": \"fill\", \n\
      \"border\": \"none\", \n\
      \"cfill\": \"1, 0, 0\", \n\
      \"widgets\": [ \n\
	{\"type\": \"tbox\", \"len\": 75, \"halign\": \"fill\", \"uid\": \"path\", \"ttype\": \"path\", \"on_event\": \"path\"}, \n\
	{\"type\": \"button\", \"btype\": \"home\", \"on_event\": \"home\"}, \n\
	{\"type\": \"button\", \"btype\": \"new_dir\", \"on_event\": \"new_dir\"} \n\
      ] \n\
    }, \n\
    {\"type\": \"table\", \"align\": \"fill\", \"min_rows\": 25, \"uid\": \"files\", \n\
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
        {\"type\": \"label\", \"text\": \"Filename:\"}, \n\
        {\"type\": \"tbox\", \"len\": 20, \"uid\": \"filename\", \"halign\": \"fill\", \"focused\": true, \"ttype\": \"filename\", \"on_event\": \"filename\"}, \n\
	{\"type\": \"checkbox\", \"label\": \"Show Hidden\", \"uid\": \"hidden\"}, \n\
	{\"type\": \"button\", \"label\": \"Cancel\", \"btype\": \"cancel\", \"on_event\": \"cancel\"}, \n\
        {\"type\": \"button\", \"label\": \"Save\", \"btype\": \"save\", \"uid\": \"save\", \"on_event\": \"save\"} \n\
      ] \n\
    } \n\
  ] \n\
 } \n\
} \n\
";
