/******************************************************************************

   Use 'SQL' class to access SQL database.

   SQL supports:
      -Microsoft SQL (MSSQL)
      -MySQL
      -PostgreSQL
      -SQLite

   MSSQL      supports all SQL_DATA_TYPE enums
   MySQL      supports all SQL_DATA_TYPE enums, however SDT_UID  will be converted internally to BINARY(16)
   PostgreSQL supports all SQL_DATA_TYPE enums, however SDT_BYTE will be converted internally to SDT_SHORT
   SQLite     supports all SQL_DATA_TYPE enums, however internally they will be converted to one of: SDT_LONG, SDT_DBL, SDT_STR, SDT_BINARY. SQLite also does not use 'str_len' or 'binary_size' for string and binary limits (all strings and binary values can be of unlimited length)

/******************************************************************************/
enum SQL_DATA_TYPE : Byte // possible type of an Column Data in a SQL Table
{
   SDT_UNKNOWN  ,
   SDT_BOOL     , // bool
   SDT_BYTE     , //  8-bit integer
   SDT_SHORT    , // 16-bit integer
   SDT_INT      , // 32-bit integer
   SDT_LONG     , // 64-bit integer
   SDT_FLT      , // 32-bit real
   SDT_DBL      , // 64-bit real
   SDT_STR      , // 16-bit string
   SDT_STR8     , //  8-bit string
   SDT_UID      , // unique id
   SDT_DATE_TIME, // date   time
   SDT_BINARY   , // binary data
};
/******************************************************************************/
struct SQLColumn // SQL Column definition, use this for creating new tables in a database
{
   enum MODE : Byte
   {
      DEFAULT     , // default mode for most of the columns
      UNIQUE      , // requires that this column must have unique data, this can be set to custom number of columns
      PRIMARY     , // requires that this column must have unique data, this can be set only up to 1 columns
      PRIMARY_AUTO, // requires that this column must have unique data, this can be set only up to 1 columns, this is a special type of PRIMARY mode, which automatically sets the value to a unique index, type of this column must be only of BYTE, SHORT, INT, LONG type
   };

   Str           name       ; // column name
   SQL_DATA_TYPE type       ; // column type
   Bool          allow_nulls; // if allow this column to have a null value
   MODE          mode       ; // column mode
   Str           default_val; // default value (optional, can be left empty), this can be set to: string "abc..", number "5", binary data "0x00112233..", SQL function "GetDate()" - gets current DateTime, "NewID()" - generates random UID
   union
   {
      Int    str_len ; // maximum number of characters allowed in a string type (STR,STR8), giving a value<=0 will make the length be as long as possible (but slower)
      Int binary_size; // maximum number of bytes      allowed in a binary type (BINARY  ), giving a value<=0 will make the size   be as big  as possible (but slower)
   };

   SQLColumn& set(C Str &name, SQL_DATA_TYPE type, Int str_len_bin_size=16) {T.name=name; T.type=type; T.str_len=T.binary_size=str_len_bin_size; return T;}

   SQLColumn() {type=SDT_UNKNOWN; allow_nulls=false; mode=DEFAULT; str_len=binary_size=16;}
};
/******************************************************************************/
struct SQLValues // SQL Row values, use this for creating new rows in a table
{
   SQLValues& New(C Str &name,   Int       value);
   SQLValues& New(C Str &name,   Long      value);
   SQLValues& New(C Str &name,   Dbl       value);
   SQLValues& New(C Str &name, C Str      &value);
   SQLValues& New(C Str &name, C UID      &value);
   SQLValues& New(C Str &name, C DateTime &value);
   SQLValues& New(C Str &name,  CPtr       value, Int size); // add new value from binary memory
   SQLValues& New(C Str &name,   File     &file           ); // add new value from binary data of the file from its current position to the end of the file

#if !EE_PRIVATE
private:
#endif
   struct Value {Byte type; Str name, value;   Value() {type=0;}   Value& set(C Str &name, C Str &value) {T.name=name; T.value=value; return T;}};
   Memc<Value> _values;
};
/******************************************************************************/
struct SQL
{
   // connect
#if EE_PRIVATE
   Bool connectODBC(C Str &params, Str *messages, Int *error, Int sql_type);
   Bool connectODBC(C Str &server_name, C Str &database, C Str &user, C Str &password, Str *messages, Int *error, Int port, C Str &driver_name, Int sql_type);
#endif
   Bool connectMSSQL (C Str &server_name, C Str &database, C Str &user=S, C Str &password=S, Str *messages=null, Int *error=null                                                                           ); // connect to Microsoft SQL 'server_name' and 'database', 'messages'=optional pointer to custom string which will receive any messages, 'error'=optional pointer to error code, false on fail
   Bool connectMySQL (C Str &server_name, C Str &database, C Str &user=S, C Str &password=S, Str *messages=null, Int *error=null,                C Str &mysql_driver_name="MySQL ODBC 5.3 Unicode Driver"  ); // connect to         MySQL 'server_name' and 'database', 'messages'=optional pointer to custom string which will receive any messages, 'error'=optional pointer to error code, false on fail
   Bool connectPgSQL (C Str &server_name, C Str &database, C Str &user=S, C Str &password=S, Str *messages=null, Int *error=null, Int port=5432, C Str &pgsql_driver_name="PostgreSQL ODBC Driver(UNICODE)"); // connect to    PostgreSQL 'server_name' and 'database', 'messages'=optional pointer to custom string which will receive any messages, 'error'=optional pointer to error code, false on fail
   Bool connectSQLite(C Str &database_file_name, const_mem_addr Cipher *cipher=null                                                                                                                        ); // connect to a SQLite database, 'database_file_name'=path to the database file (SQLite stores database information in regular files), if the specified file does not exist, then it will be automatically created, 'cipher' must point to object in constant memory address (only pointer is stored through which the object can be later accessed), false on fail

   // connect using DSN, DSN is a name of the configuration containing all the required information, including 'server_name', 'database', 'user' and 'password', you can read more about it here - https://support.quadrahosting.com/kb-article-23-0,3,15.html
   Bool dsnConnectMSSQL(C Str &dsn, Str *messages=null, Int *error=null); // connect to Microsoft SQL using a 'dsn', 'messages'=optional pointer to custom string which will receive any messages, 'error'=optional pointer to error code, false on fail, please check here how to setup DSN - https://msdn.microsoft.com/en-us/library/windows/desktop/dn170503(v=vs.85).aspx
   Bool dsnConnectMySQL(C Str &dsn, Str *messages=null, Int *error=null); // connect to         MySQL using a 'dsn', 'messages'=optional pointer to custom string which will receive any messages, 'error'=optional pointer to error code, false on fail, please check here how to setup DSN - http://dev.mysql.com/doc/connector-odbc/en/connector-odbc-configuration-dsn-windows.html
   Bool dsnConnectPgSQL(C Str &dsn, Str *messages=null, Int *error=null); // connect to    PostgreSQL using a 'dsn', 'messages'=optional pointer to custom string which will receive any messages, 'error'=optional pointer to error code, false on fail

   // commands
      // tables
      Bool    getTables(MemPtr<Str> table_names,                               Str *messages=null, Int *error=null); // get    a list of all table names   in the database, false on fail
      Bool    delTable (C Str      &table_name                               , Str *messages=null, Int *error=null); // delete a 'table_name' table from      the database, false on fail
      Bool createTable (C Str      &table_name , C MemPtr<SQLColumn> &columns, Str *messages=null, Int *error=null); // create a 'table_name' table in        the database, false on fail
      Bool appendTable (C Str      &table_name , C MemPtr<SQLColumn> &columns, Str *messages=null, Int *error=null); // append a 'table_name' table in        the database, false on fail, this method works by adding new 'columns' to an existing table
      Bool existsTable (C Str      &table_name                               , Str *messages=null, Int *error=null); // check if 'table_name' table exists in the database, false on fail

      // rows
      Bool delRow(C Str &table_name, C Str &condition,                      Str *messages=null, Int *error=null); // delete existing rows in 'table_name' table which meet the 'condition'                        , false on fail, 'condition' is a custom string specifying the condition, for example: "id=0" will delete all rows which have 'id' column equal to 0
      Bool newRow(C Str &table_name,                   C SQLValues &values, Str *messages=null, Int *error=null); // create a new    row  in 'table_name' table                            with given row 'values', false on fail
      Bool setRow(C Str &table_name, C Str &condition, C SQLValues &values, Str *messages=null, Int *error=null); // modify existing rows in 'table_name' table which meet the 'condition' by setting its 'values', false on fail, 'condition' is a custom string specifying the condition, for example: "id=0" will modify all rows which have 'id' column equal to 0

      Bool getAllRows    (C Str &table_name,                                           Str *messages=null, Int *error=null); // get all rows of a 'table_name' table                           , if the method succeeds you can access the rows using 'getNextRow' method, returned rows will have information about all their     column data, false on fail
      Bool getRows       (C Str &table_name, C Str &condition,                         Str *messages=null, Int *error=null); // get     rows of a 'table_name' table which meet the 'condition', if the method succeeds you can access the rows using 'getNextRow' method, returned rows will have information about all their     column data, false on fail, 'condition' is a custom string specifying the condition, for example: "id=0" will return all rows which have 'id' column equal to 0
      Bool getAllRowsCols(C Str &table_name,                   C MemPtr<Str> &columns, Str *messages=null, Int *error=null); // get all rows of a 'table_name' table                           , if the method succeeds you can access the rows using 'getNextRow' method, returned rows will have information only about selected column data, false on fail                                                                                                                                       'columns'=name of columns to return in the result, this method may be faster than 'getAllRows' because it returns only selected columns (unlike all columns in 'getAllRows')
      Bool getRowsCols   (C Str &table_name, C Str &condition, C MemPtr<Str> &columns, Str *messages=null, Int *error=null); // get     rows of a 'table_name' table which meet the 'condition', if the method succeeds you can access the rows using 'getNextRow' method, returned rows will have information only about selected column data, false on fail, 'condition' is a custom string specifying the condition, for example: "id=0" will return all rows which have 'id' column equal to 0, 'columns'=name of columns to return in the result, this method may be faster than 'getRows'    because it returns only selected columns (unlike all columns in 'getRows')

      Int getAllRowsNum(C Str &table_name,                   Str *messages=null, Int *error=null); // get number of all rows of a 'table_name' table                           , -1 on fail
      Int getRowsNum   (C Str &table_name, C Str &condition, Str *messages=null, Int *error=null); // get number of     rows of a 'table_name' table which meet the 'condition', -1 on fail, 'condition' is a custom string specifying the condition, for example: "id=0" will return all rows which have 'id' column equal to 0

      Int getUniqueValuesNum(C Str &table_name, C Str &column_name, Str *messages=null, Int *error=null); // get number of unique values in 'column_name' column of 'table_name' table, -1 on fail

      // custom
      Bool command(C Str &command, Str *messages=null, Int *error=null); // 'command'=custom sql command to be executed, 'messages'=optional pointer to custom string which will receive any messages, 'error'=optional pointer to error code, false on fail

      // custom with parameters passed by method calls !! these methods are supported only on MSSQL, MySQL and PostgreSQL !!
      Bool commandPrepare(C Str &command, Str *messages=null, Int *error=null); // 'command'=custom sql command to be executed with parameters replaced with "?", 'messages'=optional pointer to custom string which will receive any messages, 'error'=optional pointer to error code, unlike the 'command' method above, this method only prepares execution of specified command, after calling this method you should call 'commandParam' for each parameter, and 'commandExecute' at the end. This method returns false on fail. For example: if(commandPrepare("insert into players(player_name, race, gender) values (?, ?, ?)"))commandParam(0, "John").commandParam(1, "Elf").commandParam(2, "Male").commandExecute(); Another example: if(commandPrepare("update players set player_name=? where player_id=?"))commandParam(0, "John").commandParam(1, "123").commandExecute();

   #if EE_PRIVATE
      Bool commandParamRaw (Int i, CPtr value, Int size, Int c_type, Int sql_type);
      Bool commandParamRaw2(Int i, CPtr value, Int size, Int c_type, Int sql_type);
   #endif
      SQL& commandParam(Int i,   Bool          value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i,   Int           value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i,   UInt          value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i,   Long          value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i,   ULong         value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i,   Flt           value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i,   Dbl           value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i, CChar          *value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i, CChar8         *value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i, C Str          &value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i, C Str8         &value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i, C UID          &value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i, C DateTime     &value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i, C DateTimeMs   &value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i, C MemPtr<Byte> &value          ); // set i-th parameter as 'value'
      SQL& commandParam(Int i, CPtr            value, Int size); // get i-th parameter as 'value'

      Bool commandExecute(Str *messages=null, Int *error=null); // execute prepared command, 'messages'=optional pointer to custom string which will receive any messages, 'error'=optional pointer to error code, false on fail

   // get
   Str string(C Str &s)C; // return 's' string in SQL friendly format, this is needed for writing custom SQL conditions, this converts "John's" string into:
                          //    MSSQL  : "N'John''s'"
                          //    MySQL  : "'John\'s'"
                          //    PgSQL  : "'John''s'"
                          //    SQLite : "'John''s'"

   Bool getNextRow(); // get next row, call this in a loop after calling sql command to process all returned rows, false on fail

   Int getCols(); // get number of columns in returned result, you can optionally call this after sql commands

   Bool getColDesc(Int i, Str &name, SQL_DATA_TYPE &type, Int &size); // get i-th column description, 'name'=column name, 'type'=column data type, 'size'=maximum column data size (in bytes)

   // reading column data must be done in sequential order !! (you can read data from column 0, 1, 2, ..; but NOT from 2, 0, 1, ..;) 
   Bool getCol(Int i, Bool        &value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i, Byte        &value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i,  Int        &value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i, UInt        &value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i, Long        &value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i, ULong       &value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i, Flt         &value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i, Dbl         &value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i, Str         &value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i, Str8        &value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i, UID         &value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i, DateTime    &value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i, DateTimeMs  &value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i, MemPtr<Byte> value                 ); // get i-th column data as 'value', false on fail
   Bool getCol(Int i, File        &file                  ); // get i-th column data into file , false on fail, 'file'=file object which will have appended binary data from the column, 'file' should be already opened for writing, column binary data will be written into 'file' from the file's current position, during writing 'file' position will be adjusted by the size of written data
   Bool getCol(Int i, Char        *value, Int  max_length); // get i-th column data as array  of 'max_length' characters, false on fail
   Bool getCol(Int i, Char8       *value, Int  max_length); // get i-th column data as array  of 'max_length' characters, false on fail
   Bool getCol(Int i, Ptr          value, Int &size      ); // get i-th column data as memory of 'size'       size      , before calling this method you should set 'size' to maximum capacity of the value buffer, after this method returns, 'size' will be equal to the actual amount of data written to the buffer, false on fail

   SQL&   del(); // delete manually
  ~SQL() {del();}
   SQL();

private:
   struct Row
   {
      struct Col
      {
         Long          i;
         Dbl           d;
         Str           s;
         Mems<Byte>    b;
         SQL_DATA_TYPE type;
      };
      Mems<Col> cols;
   };
   struct Col
   {
      Str           name;
      SQL_DATA_TYPE type;
   };
#if EE_PRIVATE
   enum SQL_TYPE : Byte
   {
      NONE  ,
      MSSQL ,
      MYSQL ,
      PGSQL ,
      SQLITE,
   };
#endif
   Byte      _type;
   Ptr       _env, _conn, _statement, _sqlite;
   Int       _rows_pos;
   Memc<Row> _rows;
   Memc<Col> _cols;
   Memc< Memc<Byte> > _params;
#if EE_PRIVATE
   Str  value    (C SQLValues::Value &value)C;
   Str  token    (C Str &token)C;
   Bool colDesc  (C SQLColumn &col, Str &desc, Str *messages);
   void getStatus(Str *messages, Int *error, Bool statement=true);
#endif
   NO_COPY_CONSTRUCTOR(SQL);
};
/******************************************************************************/
