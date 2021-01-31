static const char *dialog_file_open = "\
{ \
  \"version\": 1, \
  \"rows\": 3, \
  \"widgets\": [ \
    { \
      \"cols\": 2, \
      \"halign\": \"fill\", \
      \"border\": \"none\", \
      \"cfill\": \"0, 1\", \
      \"widgets\": [ \
	{ \
	  \"type\": \"label\", \
	  \"text\": \"Directory:\" \
	}, \
	{ \
	  \"type\": \"tbox\", \
	  \"len\": 75, \
	  \"halign\": \"fill\", \
	  \"uid\": \"path\" \
	} \
      ] \
    }, \
    { \
    }, \
    { \
      \"cols\": 5, \
      \"border\": \"none\", \
      \"halign\": \"fill\", \
      \"cfill\": \"0, 8, 0, 0, 0\", \
      \"cpadf\": \"0, 0, 1, 1, 0, 0\", \
      \"widgets\": [ \
        { \
          \"type\": \"label\", \
          \"text\": \"Filter:\" \
        }, \
        { \
	  \"type\": \"tbox\", \
	  \"len\": 20, \
	  \"uid\": \"filter\", \
	  \"halign\": \"fill\" \
        }, \
	{ \
	 \"type\": \"checkbox\", \
	 \"label\": \"Show Hidden\", \
	 \"uid\": \"hidden\" \
	}, \
	{ \
          \"type\": \"button\", \
          \"label\": \"Cancel\", \
	  \"btype\": \"cancel\", \
	  \"uid\": \"cancel\" \
        }, \
        { \
          \"type\": \"button\", \
          \"label\": \"Open\", \
	  \"btype\": \"open\", \
	  \"uid\": \"open\" \
        } \
      ] \
    } \
  ] \
} \
";
