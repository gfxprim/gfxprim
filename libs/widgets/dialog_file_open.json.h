static const char *dialog_file_open = "\
{ \n\
  \"version\": 1, \n\
  \"rows\": 3, \n\
  \"widgets\": [ \n\
    { \n\
      \"cols\": 2, \n\
      \"halign\": \"fill\", \n\
      \"border\": \"none\", \n\
      \"cfill\": \"0, 1\", \n\
      \"widgets\": [ \n\
	{\"type\": \"label\", \"text\": \"Directory:\"}, \n\
	{\"type\": \"tbox\", \"len\": 75, \"halign\": \"fill\", \"uid\": \"path\"} \n\
      ] \n\
    }, \n\
    { \n\
     \"type\": \"table\", \n\
     \"focused\": true, \n\
     \"align\": \"fill\", \n\
     \"min_rows\": 25, \n\
     \"uid\": \"files\", \n\
     \"col_ops\": \"gp_dialog_files_col_ops\", \n\
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
        { \n\
	  \"type\": \"tbox\", \n\
	  \"len\": 20, \n\
	  \"uid\": \"filter\", \n\
	  \"halign\": \"fill\" \n\
        }, \n\
	{ \n\
	 \"type\": \"checkbox\", \n\
	 \"label\": \"Show Hidden\", \n\
	 \"uid\": \"hidden\" \n\
	}, \n\
	{ \n\
          \"type\": \"button\", \n\
          \"label\": \"Cancel\", \n\
	  \"btype\": \"cancel\", \n\
	  \"uid\": \"cancel\" \n\
        }, \n\
        { \n\
          \"type\": \"button\", \n\
          \"label\": \"Open\", \n\
	  \"btype\": \"open\", \n\
	  \"uid\": \"open\" \n\
        } \n\
      ] \n\
    } \n\
  ] \n\
} \n\
";
