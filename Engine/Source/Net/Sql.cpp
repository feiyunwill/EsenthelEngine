/******************************************************************************/
#include "stdafx.h"

#include "../../../ThirdPartyLibs/begin.h"

#if SUPPORT_SQLITE
   #include "../../../ThirdPartyLibs/SQLite/sqlite3.h"
#endif

#if SUPPORT_ODBC
   #if !WINDOWS // not needed on Windows platform
      #define _IODBCUNIX_H    // will disable some compilation errors on Unix
      #define UNICODE         // will force SQL command names to be Unicode
      #define ULONG SQL_ULONG // will disable typedef conflict error on Apple
   #endif

   #if MAC
      #include "../../../ThirdPartyLibs/iODBC/sqlext.h"
   #else
      #include <sqlext.h>
   #endif
#endif

#include "../../../ThirdPartyLibs/end.h"

namespace EE{
#define SQLITE_VFS_NAME "Esenthel"
/******************************************************************************/
enum SQL_VAL_TYPE
{
   SVT_RAW,
   SVT_STR,
   SVT_BIN,
   SVT_UID,
};
/******************************************************************************/
SQLValues& SQLValues::New(C Str &name,   Int       value                ) {_values.New().set(name, S+value                               ).type=SVT_RAW; return T;}
SQLValues& SQLValues::New(C Str &name,   Long      value                ) {_values.New().set(name, S+value                               ).type=SVT_RAW; return T;}
SQLValues& SQLValues::New(C Str &name,   Dbl       value                ) {_values.New().set(name, S+value                               ).type=SVT_RAW; return T;}
SQLValues& SQLValues::New(C Str &name, C Str      &value                ) {_values.New().set(name,   value                               ).type=SVT_STR; return T;}
SQLValues& SQLValues::New(C Str &name, C DateTime &value                ) {_values.New().set(name, S+'\''+value.asText(true)+'\''        ).type=SVT_RAW; return T;}
SQLValues& SQLValues::New(C Str &name,  CPtr       value, Int value_size) {_values.New().set(name, TextHexMem( value, value_size , false)).type=SVT_BIN; return T;}
SQLValues& SQLValues::New(C Str &name,   File     &file                 ) {_values.New().set(name, TextHexMem( file              , false)).type=SVT_BIN; return T;}
SQLValues& SQLValues::New(C Str &name, C UID      &value                ) {_values.New().set(name, TextHexMem(&value, SIZE(value), false)).type=SVT_UID; return T;}
/******************************************************************************/
SQL::SQL()
{
  _type=NONE;
  _env=_conn=_statement=_sqlite=null;
  _rows_pos=0;
}
SQL& SQL::del()
{
#if SUPPORT_ODBC
   if(_statement){                      SQLFreeHandle(SQL_HANDLE_STMT, _statement); _statement=null;}
   if(_conn     ){SQLDisconnect(_conn); SQLFreeHandle(SQL_HANDLE_DBC , _conn     ); _conn     =null;}
   if(_env      ){                      SQLFreeHandle(SQL_HANDLE_ENV , _env      ); _env      =null;}
#endif
#if SUPPORT_SQLITE
   if(_sqlite   ){                      sqlite3_close(      (sqlite3*&)_sqlite   ); _sqlite   =null;}
#endif
  _type=NONE;
  _rows  .del();
  _cols  .del();
  _params.del();
   return T;
}
/******************************************************************************/
Bool SQL::connectODBC(C Str &params, Str *messages, Int *error, Int sql_type)
{
#if SUPPORT_ODBC
   if(messages)messages->clear();
   if(error   )*error=0;
   del();

   if(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_env)==SQL_SUCCESS)
   if(SQLSetEnvAttr (_env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0)==SQL_SUCCESS)
   if(SQLAllocHandle(SQL_HANDLE_DBC, _env, &_conn)==SQL_SUCCESS)
   {
      T._type=sql_type;
      SQLWCHAR conn_str[1024];
      if(CChar *p=params())switch(SQLDriverConnect(_conn, null, (SQLWCHAR*)p, SQL_NTS, conn_str, Elms(conn_str), null, SQL_DRIVER_NOPROMPT))
      {
         case SQL_SUCCESS_WITH_INFO:              getStatus(messages, error, false);               break;
         case SQL_INVALID_HANDLE: case SQL_ERROR: getStatus(messages, error, false); del(); return false;
      }
      if(SQLAllocHandle(SQL_HANDLE_STMT, _conn, &_statement)==SQL_SUCCESS)return true;
   }
#else
   if(messages)*messages="ODBC SQL is not supported on this platform";
   if(error   )*error=0;
#endif
   del(); return false;
}
Bool SQL::connectODBC(C Str &server_name, C Str &database, C Str &user, C Str &password, Str *messages, Int *error, Int port, C Str &driver_name, Int sql_type)
{
#if SUPPORT_ODBC
   Str params=S+"DRIVER={"+driver_name+"};SERVER="+server_name+";DATABASE="+database+";";
   switch(_type=sql_type)
   {
      case MSSQL: params+=S+"Trusted_Connection=Yes;UID="+user+";PWD="+password+";"; break;
      case MYSQL: params+=S+"User="+user+";Password="+password+";"; break;
      case PGSQL: params+=S+"Port="+port+";UID="+user+";PWD="+password+";"; break;
   }
#else
   Str params;
#endif
   return connectODBC(params, messages, error, sql_type);
}
Bool SQL::connectMSSQL(C Str &server_name, C Str &database, C Str &user, C Str &password, Str *messages, Int *error                                    ) {return connectODBC(server_name, database, user, password, messages, error,    0, "SQL Server"     , MSSQL);}
Bool SQL::connectMySQL(C Str &server_name, C Str &database, C Str &user, C Str &password, Str *messages, Int *error,           C Str &mysql_driver_name) {return connectODBC(server_name, database, user, password, messages, error,    0, mysql_driver_name, MYSQL);}
Bool SQL::connectPgSQL(C Str &server_name, C Str &database, C Str &user, C Str &password, Str *messages, Int *error, Int port, C Str &pgsql_driver_name) {return connectODBC(server_name, database, user, password, messages, error, port, pgsql_driver_name, PGSQL);}

Bool SQL::dsnConnectMSSQL(C Str &dsn, Str *messages, Int *error) {return connectODBC(S+"DSN="+dsn+";", messages, error, MSSQL);}
Bool SQL::dsnConnectMySQL(C Str &dsn, Str *messages, Int *error) {return connectODBC(S+"DSN="+dsn+";", messages, error, MYSQL);}
Bool SQL::dsnConnectPgSQL(C Str &dsn, Str *messages, Int *error) {return connectODBC(S+"DSN="+dsn+";", messages, error, PGSQL);}

#if SUPPORT_SQLITE
static SyncLock SQLiteCipherLock;
static Cipher  *SQLiteCipher;
#endif
Bool SQL::connectSQLite(C Str &database_file_name, const_mem_addr Cipher *cipher)
{
#if SUPPORT_SQLITE
   del();
   Bool from_pak=(FExist(database_file_name) && !FExistSystem(database_file_name));
   if(cipher // custom cipher needed
   || from_pak) // file exists only in project data, try to connect in read-only mode using callbacks
   {
      SyncLocker lock(SQLiteCipherLock);
      SQLiteCipher=cipher;
      if(sqlite3_open_v2(UTF8(database_file_name), &(sqlite3*&)_sqlite, from_pak ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE, SQLITE_VFS_NAME)==SQLITE_OK){_type=SQLITE; return true;} // encode to UTF8 and later decode from it, because SQLite callbacks operate on char
   }else
   {
      if(sqlite3_open16(PLATFORM(database_file_name, UnixPath(database_file_name))(), &(sqlite3*&)_sqlite)==SQLITE_OK){_type=SQLITE; return true;}
   }
#endif
   del(); return false;
}
/******************************************************************************/
Bool SQL::command(C Str &command, Str *messages, Int *error)
{
   if(messages)messages->clear();
   if(error   )*error=0;

   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         SQLCloseCursor(_statement); // cursor needs to be closed if there was already a command executed (without closing it, executing new commands wouldn't succeed)
         if(CChar *c=command())if(SQLExecDirect(_statement, (SQLWCHAR*)c, SQL_NTS)!=SQL_SUCCESS)
         {
            getStatus(messages, error);
            return false;
         }
      }return true;
   #endif
      
   #if SUPPORT_SQLITE
      case SQLITE:
      {
        _rows_pos=-1;
        _rows.clear();
        _cols.clear();
         if(_sqlite)
         {
            SQLiteMutexEnter((sqlite3*&)_sqlite);
            sqlite3_stmt *stmt=null;
            Bool ok=(sqlite3_prepare16_v2((sqlite3*&)_sqlite, command(), -1, &stmt, null)==SQLITE_OK);
            if(ok && stmt)
            {
              _cols.setNum(sqlite3_column_count(stmt));
               FREPA(_cols)
               {
                  Col &col=_cols[i];
                  col.name=(CChar*)sqlite3_column_name16(stmt, i);
                  col.type=SDT_UNKNOWN;
               }
               for(; sqlite3_step(stmt)==SQLITE_ROW; )
               {
                  Row &row=_rows.New(); row.cols.setNum(_cols.elms());
                  FREPA(_cols)
                  {
                     Col      & col=   _cols[i];
                     Row::Col &rcol=row.cols[i];
                     switch(sqlite3_column_type(stmt, i))
                     {
                        case SQLITE_INTEGER: rcol.type=SDT_LONG   ; break;
                        case SQLITE_FLOAT  : rcol.type=SDT_DBL    ; break;
                        case SQLITE_TEXT   : rcol.type=SDT_STR    ; break;
                        case SQLITE_BLOB   : rcol.type=SDT_BINARY ; break;
                        default            : rcol.type=SDT_UNKNOWN; break;
                     }
                     if(col.type==SDT_UNKNOWN)col.type=rcol.type;
                     switch(rcol.type)
                     {
                        case SDT_LONG  : rcol.i=        sqlite3_column_int64 (stmt, i); break;
                        case SDT_DBL   : rcol.d=        sqlite3_column_double(stmt, i); break;
                        case SDT_STR   : rcol.s=(CChar*)sqlite3_column_text16(stmt, i); break;
                        case SDT_BINARY:
                        {
                           const Byte *src=(const Byte*)sqlite3_column_blob (stmt, i); // 'sqlite3_column_blob' must be called before 'sqlite3_column_bytes'
                           rcol.b.setNum(               sqlite3_column_bytes(stmt, i)); rcol.b.copyFrom(src);
                        }break;
                     }
                  }
               }
            }
            if(stmt){ok&=(sqlite3_finalize(stmt)==SQLITE_OK); stmt=null;}
            if(!ok)
            {
               if(messages)*messages=(CChar*)sqlite3_errmsg16((sqlite3*&)_sqlite);
               if(error   )*error   =        sqlite3_errcode ((sqlite3*&)_sqlite);
            }
            SQLiteMutexLeave((sqlite3*&)_sqlite);
            return ok;
         }
      }break;
   #endif
   }
   if(messages)*messages="SQL not connected"; return false;
}
/******************************************************************************/
Bool SQL::commandPrepare(C Str &command, Str *messages, Int *error)
{
   if(messages)messages->clear();
   if(error   )*error=0;

   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         SQLCloseCursor(_statement); // cursor needs to be closed if there was already a command executed (without closing it, executing new commands wouldn't succeed)
         CChar *c=command(); if(SQLPrepare(_statement, (SQLWCHAR*)c, SQL_NTS)==SQL_SUCCESS)return true;
         getStatus(messages, error);
      }return false;
   #endif

      case SQLITE: if(messages)*messages="'commandPrepare' is currently not supported on SQLite"; return false;
   }
   if(messages)*messages="SQL not connected"; return false;
}
Bool SQL::commandParamRaw(Int i, CPtr value, Int size, Int c_type, Int sql_type)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         Memc<Byte> &param=_params(i);
         param.setNum(size);
         Copy(param.data(), value, size);
         return SQLBindParameter(_statement, i+1, SQL_PARAM_INPUT, c_type, sql_type, 0, 0, param.data(), 0, null)==SQL_SUCCESS;
      }break;
   #endif
   }
   return false;
}
Bool SQL::commandParamRaw2(Int i, CPtr value, Int size, Int c_type, Int sql_type)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         Memc<Byte> &param=_params(i);
         param.setNum(size+SIZE(SQLLEN));
         Copy(param.data(), value, size);
         SQLLEN &length=*(SQLLEN*)(param.data()+size); length=size;
         return SQLBindParameter(_statement, i+1, SQL_PARAM_INPUT, c_type, sql_type, 0, 0, param.data(), 0, &length)==SQL_SUCCESS;
      }break;
   #endif
   }
   return false;
}
#if SUPPORT_ODBC
SQL& SQL::commandParam(Int i,   Bool      value          ) {commandParamRaw (i, &value  , SIZE  (value)   , SQL_C_BIT    , SQL_BIT     ); return T;}
SQL& SQL::commandParam(Int i,   Int       value          ) {commandParamRaw (i, &value  , SIZE  (value)   , SQL_C_SLONG  , SQL_INTEGER ); return T;}
SQL& SQL::commandParam(Int i,   UInt      value          ) {commandParamRaw (i, &value  , SIZE  (value)   , SQL_C_ULONG  , SQL_INTEGER ); return T;}
SQL& SQL::commandParam(Int i,   Long      value          ) {commandParamRaw (i, &value  , SIZE  (value)   , SQL_C_SBIGINT, SQL_BIGINT  ); return T;}
SQL& SQL::commandParam(Int i,   ULong     value          ) {commandParamRaw (i, &value  , SIZE  (value)   , SQL_C_UBIGINT, SQL_BIGINT  ); return T;}
SQL& SQL::commandParam(Int i,   Flt       value          ) {commandParamRaw (i, &value  , SIZE  (value)   , SQL_C_FLOAT  , SQL_REAL    ); return T;}
SQL& SQL::commandParam(Int i,   Dbl       value          ) {commandParamRaw (i, &value  , SIZE  (value)   , SQL_C_DOUBLE , SQL_DOUBLE  ); return T;}
SQL& SQL::commandParam(Int i, CChar8     *value          ) {commandParamRaw2(i,  value  , Length(value)   , SQL_C_CHAR   , SQL_VARCHAR ); return T;}
SQL& SQL::commandParam(Int i, CChar      *value          ) {commandParamRaw2(i,  value  , Length(value) *2, SQL_C_WCHAR  , SQL_WVARCHAR); return T;}
SQL& SQL::commandParam(Int i, C Str8     &value          ) {commandParamRaw2(i,  value(), value.length()  , SQL_C_CHAR   , SQL_VARCHAR ); return T;}
SQL& SQL::commandParam(Int i, C Str      &value          ) {commandParamRaw2(i,  value(), value.length()*2, SQL_C_WCHAR  , SQL_WVARCHAR); return T;}
SQL& SQL::commandParam(Int i, CPtr        value, Int size) {commandParamRaw2(i,  value  , size            , SQL_C_BINARY , SQL_BINARY  ); return T;}
SQL& SQL::commandParam(Int i, C DateTime &value          )
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         TIMESTAMP_STRUCT ts;
         ts.year    =value.year  ;
         ts.month   =value.month ;
         ts.day     =value.day   ;
         ts.hour    =value.hour  ;
         ts.minute  =value.minute;
         ts.second  =value.second;
         ts.fraction=           0;
         commandParamRaw(i, &ts, SIZE(ts), SQL_C_TIMESTAMP, SQL_TIMESTAMP);
      }break;
   #endif
   }
   return T;
}
SQL& SQL::commandParam(Int i, C DateTimeMs &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         TIMESTAMP_STRUCT ts;
         ts.year    =value.year  ;
         ts.month   =value.month ;
         ts.day     =value.day   ;
         ts.hour    =value.hour  ;
         ts.minute  =value.minute;
         ts.second  =value.     second;
         ts.fraction=value.millisecond*1000000; // 1ms is 1,000,000 units according to - https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
         commandParamRaw(i, &ts, SIZE(ts), SQL_C_TIMESTAMP, SQL_TIMESTAMP);
      }break;
   #endif
   }
   return T;
}
#else
SQL& SQL::commandParam(Int i,   Bool        value          ) {return T;}
SQL& SQL::commandParam(Int i,   Int         value          ) {return T;}
SQL& SQL::commandParam(Int i,   UInt        value          ) {return T;}
SQL& SQL::commandParam(Int i,   Long        value          ) {return T;}
SQL& SQL::commandParam(Int i,   ULong       value          ) {return T;}
SQL& SQL::commandParam(Int i,   Flt         value          ) {return T;}
SQL& SQL::commandParam(Int i,   Dbl         value          ) {return T;}
SQL& SQL::commandParam(Int i, CChar8       *value          ) {return T;}
SQL& SQL::commandParam(Int i, CChar        *value          ) {return T;}
SQL& SQL::commandParam(Int i, C Str8       &value          ) {return T;}
SQL& SQL::commandParam(Int i, C Str        &value          ) {return T;}
SQL& SQL::commandParam(Int i, CPtr          value, Int size) {return T;}
SQL& SQL::commandParam(Int i, C DateTime   &value          ) {return T;}
SQL& SQL::commandParam(Int i, C DateTimeMs &value          ) {return T;}
#endif
SQL& SQL::commandParam(Int i, C UID &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case PGSQL: commandParamRaw(i, &value, SIZE(value), SQL_C_GUID, SQL_GUID); break;

      case MYSQL: commandParamRaw2(i, &value, SIZE(value), SQL_C_BINARY, SQL_BINARY); break; // MySQL does not support UID
   #endif
   }
   return T;
}
SQL& SQL::commandParam(Int i, C MemPtr<Byte> &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         Memc<Byte> &param=_params(i);
         param.setNum(value.elms()+SIZE(SQLLEN));
         value.copyTo(param.data());
         SQLLEN &length=*(SQLLEN*)(param.data()+value.elms()); length=value.elms();
         Bool ok=(SQLBindParameter(_statement, i+1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY, 0, 0, param.data(), 0, &length)==SQL_SUCCESS);
      }break;
   #endif
   }
   return T;
}
Bool SQL::commandExecute(Str *messages, Int *error)
{
   if(messages)messages->clear();
   if(error   )*error=0;

   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         SQLCloseCursor(_statement); // cursor needs to be closed if there was already a command executed (without closing it, executing new commands wouldn't succeed)
         if(SQLExecute(_statement)==SQL_SUCCESS)return true;
         getStatus(messages, error);
      }return false;
   #endif

      case SQLITE: if(messages)*messages="'commandExecute' is currently not supported on SQLite"; return false;
   }
   if(messages)*messages="SQL not connected"; return false;
}
/******************************************************************************/
Bool SQL::getTables(MemPtr<Str> table_names, Str *messages, Int *error)
{
   table_names.clear();
   CChar8 *cmd=null;
   switch(_type)
   {
      default    : if(messages)*messages="SQL not connected"; if(error)*error=0; return false;
      case MSSQL : cmd="SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES"; break; // "INFORMATION_SCHEMA.TABLES" has 4 columns
    //case MSSQL : cmd="SELECT name FROM sys.tables"; break; // "sys.tables" has 27 columns
    //case MSSQL : cmd="SELECT Name FROM SysObjects"; break; // "SysObjects" has 25 columns and returns more elements than it should, probably would require some type filtering?
      case MYSQL : cmd="SHOW TABLES"; break;
      case PGSQL : cmd="SELECT table_name FROM information_schema.tables WHERE table_schema='public'"; break;
      case SQLITE: cmd="SELECT name FROM sqlite_master WHERE type='table'"; break;
   }
   if(command(cmd, messages, error))
   {
      for(; getNextRow(); )getCol(0, table_names.New());
      return true;
   }
   return false;
}
Bool SQL::delTable(C Str &table_name, Str *messages, Int *error)
{
   return command(S+"DROP TABLE "+token(table_name), messages, error);
}
Bool SQL::createTable(C Str &table_name, C MemPtr<SQLColumn> &columns, Str *messages, Int *error)
{
   if(messages)messages->clear();
   if(error   )*error=0;
   if(!columns.elms())
   {
      if(messages)*messages="Can't create an empty table";
      return false;
   }
   Str cmd=S+"CREATE TABLE "+token(table_name)+" (", desc;
   FREPA(columns){if(i)cmd+=", "; if(!colDesc(columns[i], desc, messages))return false; cmd+=desc;}
   cmd+=')';
   return command(cmd, messages, error);
}
Bool SQL::appendTable(C Str &table_name, C MemPtr<SQLColumn> &columns, Str *messages, Int *error)
{
   if(messages)messages->clear();
   if(error   )*error=0;
   if(columns.elms())
   {
      Str cmd=S+"ALTER TABLE "+token(table_name)+" ADD ", desc;
      FREPA(columns){if(i)cmd+=", "; if(!colDesc(columns[i], desc, messages))return false; cmd+=desc;}
      return command(cmd, messages, error);
   }
   return true;
 //if(del_cols.elms())cmd+="DROP COLUMN "; FREPA(del_cols){if(i)cmd+=", "; cmd+=S+'['+del_cols[i]+']';}
 //FREPA(modify_cols)cmd+=S+"ALTER TABLE "+token(table_name)+" ALTER COLUMN "+colDesc(modify_cols[i])+" ;\n";
}
Bool SQL::existsTable(C Str &table_name, Str *messages, Int *error)
{
   switch(_type)
   {
      default    : if(messages)*messages="SQL not connected"; return false;
      case MSSQL : if(command(S+"SELECT * FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME="  +string(table_name), messages, error))return getNextRow(); break;
    //case MYSQL : if(command(S+"SHOW TABLES LIKE "                                          +string(table_name), messages, error))return getNextRow(); break; // this fails for 'table_name' - "\'\\a"
      case MYSQL : return command(S+"SELECT 1 FROM "+token(table_name)+" LIMIT 1", messages, error);
      case PGSQL : if(command(S+"SELECT * FROM INFORMATION_SCHEMA.TABLES WHERE table_name="  +string(table_name), messages, error))return getNextRow(); break;
      case SQLITE: if(command(S+"SELECT name FROM sqlite_master WHERE type='table' AND name="+string(table_name), messages, error))return getNextRow(); break;
   }
   return false;
}
/******************************************************************************/
Bool SQL::delRow(C Str &table_name, C Str &condition, Str *messages, Int *error)
{
   return command(S+"DELETE FROM "+token(table_name)+" WHERE "+condition, messages, error);
}
Bool SQL::newRow(C Str &table_name, C SQLValues &values, Str *messages, Int *error)
{
   Str cmd=S+"INSERT INTO "+token(table_name)+" (";
   FREPA(values._values){if(i)cmd+=", "; cmd+=token(values._values[i].name);}
   cmd+=") VALUES (";
   FREPA(values._values){if(i)cmd+=", "; cmd+=value(values._values[i]);}
   cmd+=")";
   return command(cmd, messages, error);
}
Bool SQL::setRow(C Str &table_name, C Str &condition, C SQLValues &values, Str *messages, Int *error)
{
   Str cmd=S+"UPDATE "+token(table_name)+" SET ";
   FREPA(values._values){if(i)cmd+=", "; cmd+=token(values._values[i].name)+'='+value(values._values[i]);}
   cmd+=" WHERE "; cmd+=condition;
   return command(cmd, messages, error);
}
/******************************************************************************/
Bool SQL::getAllRows(C Str &table_name, Str *messages, Int *error)
{
   return command(S+"SELECT * FROM "+token(table_name), messages, error);
}
Bool SQL::getRows(C Str &table_name, C Str &condition, Str *messages, Int *error)
{
   if(!condition.is())return getAllRows(table_name, messages, error);
   return command(S+"SELECT * FROM "+token(table_name)+" WHERE "+condition, messages, error);
}
Bool SQL::getAllRowsCols(C Str &table_name, C MemPtr<Str> &columns, Str *messages, Int *error)
{
   Str cmd="SELECT ";
   FREPA(columns){if(i)cmd+=", "; cmd+=token(columns[i]);}
   cmd+=S+" FROM "+token(table_name);
   return command(cmd, messages, error);
}
Bool SQL::getRowsCols(C Str &table_name, C Str &condition, C MemPtr<Str> &columns, Str *messages, Int *error)
{
   if(!condition.is())return getAllRowsCols(table_name, columns, messages, error);
   Str cmd="SELECT ";
   FREPA(columns){if(i)cmd+=", "; cmd+=token(columns[i]);}
   cmd+=S+" FROM "+token(table_name)+" WHERE "; cmd+=condition;
   return command(cmd, messages, error);
}
/******************************************************************************/
Int SQL::getAllRowsNum(C Str &table_name, Str *messages, Int *error)
{
   if(command(S+"SELECT COUNT(*) FROM "+token(table_name), messages, error))if(getNextRow())
   {
      Int    rows=0; getCol(0, rows);
      return rows;
   }
   return -1;
}
Int SQL::getRowsNum(C Str &table_name, C Str &condition, Str *messages, Int *error)
{
   if(!condition.is())return getAllRowsNum(table_name, messages, error);
   if(command(S+"SELECT COUNT(*) FROM "+token(table_name)+" WHERE "+condition, messages, error))if(getNextRow())
   {
      Int    rows=0; getCol(0, rows);
      return rows;
   }
   return -1;
}
Int SQL::getUniqueValuesNum(C Str &table_name, C Str &column_name, Str *messages, Int *error)
{
   if(command(S+"SELECT COUNT(DISTINCT "+token(column_name)+") FROM "+token(table_name), messages, error))if(getNextRow())
   {
      Int    rows=0; getCol(0, rows);
      return rows;
   }
   return -1;
}
/******************************************************************************/
Str SQL::string(C Str &s)C
{
   switch(_type)
   {
      case MSSQL:
      {
         Str    out="N'"; FREPA(s){Char c=s[i]; if(c=='\'')out+="''";else if(c=='\n')out+="'+CHAR(10)+N'";else if(c=='\r')out+="'+CHAR(13)+N'";else out+=c;} out+='\''; // "some text" -> "'some text'", "I'm a text" -> "'I''m a text'", (' must be replaced with '')
         return out;
      }
      case PGSQL:
      case SQLITE:
      {
         Str    out='\''; FREPA(s){Char c=s[i]; if(c=='\'')out+="''";else out+=c;} out+='\''; // "some text" -> "'some text'", "I'm a text" -> "'I''m a text'", (' must be replaced with '')
         return out;
      }
      case MYSQL: return S+'\''+Replace(Replace(s, "\\", "\\\\"), "'", "\\'")+'\''; // replace   \ into \\   and   ' into \'
      default   : return s;
   }
}   
/******************************************************************************/
Bool SQL::getNextRow()
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL: return SQLFetch(_statement)==SQL_SUCCESS;
   #endif

      case SQLITE:
      {
         if(InRange(_rows_pos+1, _rows)){_rows_pos++; return true;}
      }return false;

      default: return false;
   }
}
Int SQL::getCols()
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL: {SQLSMALLINT cols=0; SQLNumResultCols(_statement, &cols); return cols;}
   #endif

      case SQLITE: return _cols.elms();

      default: return 0;
   }
}

Bool SQL::getColDesc(Int i, Str &name, SQL_DATA_TYPE &type, Int &size)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         SQLWCHAR temp[1024]; SQLSMALLINT t; SQLULEN s; if(SQLDescribeCol(_statement, i+1, temp, Elms(temp), null, &t, &s, null, null)==SQL_SUCCESS)
         {
            name=(Char*)temp;
            switch(t)
            {
               case SQL_CHAR          : size=s             ; type=SDT_STR8     ; break;
               case SQL_VARCHAR       : size=s             ; type=SDT_STR8     ; break;
               case SQL_LONGVARCHAR   : size=-1            ; type=SDT_STR8     ; break;
               case SQL_WCHAR         : size=s*2           ; type=SDT_STR      ; break;
               case SQL_WVARCHAR      : size=s*2           ; type=SDT_STR      ; break;
               case SQL_WLONGVARCHAR  : size=-1            ; type=SDT_STR      ; break;
               case SQL_BIT           : size=1             ; type=SDT_BOOL     ; break;
               case SQL_TINYINT       : size=1             ; type=SDT_BYTE     ; break;
               case SQL_SMALLINT      : size=2             ; type=SDT_SHORT    ; break;
               case SQL_INTEGER       : size=4             ; type=SDT_INT      ; break;
               case SQL_BIGINT        : size=8             ; type=SDT_LONG     ; break;
               case SQL_REAL          : size=4             ; type=SDT_FLT      ; break; // SQL_REAL  is C++ float
               case SQL_FLOAT         : size=8             ; type=SDT_DBL      ; break; // SQL_FLOAT is C++ double
               case SQL_DOUBLE        : size=8             ; type=SDT_DBL      ; break; // encountered in MySQL
               case SQL_GUID          : size=16            ; type=SDT_UID      ; break;
               case SQL_BINARY        : size=s             ; type=SDT_BINARY   ; break;
               case SQL_VARBINARY     : size=s             ; type=SDT_BINARY   ; break;
               case SQL_LONGVARBINARY : size=-1            ; type=SDT_BINARY   ; break;
               case SQL_TYPE_TIMESTAMP: size=SIZE(DateTime); type=SDT_DATE_TIME; break;
               default                : size=s             ; type=SDT_UNKNOWN  ; break;
            }
            return true;
         }
      }break;
   #endif

      case SQLITE:
      {
         if(InRange(i, _cols))
         {
            Col &col=_cols[i];
                   name=col.name;
            switch(type=col.type)
            {
               default: size=-1; break;

               case SDT_INT :
               case SDT_LONG: size=SIZE(Long); break;

               case SDT_FLT:
               case SDT_DBL: size=SIZE(Dbl); break;
            }
            return true;
         }
      }break;
   }
   name.clear(); type=SDT_UNKNOWN; size=0;
   return false;
}
/******************************************************************************/
Bool SQL::getCol(Int i, Bool &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL: if(SQLGetData(_statement, i+1, SQL_C_BIT, &value, SIZE(value), null)==SQL_SUCCESS)return true; break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : value=        (col.i!=0); return true;
            case SDT_DBL   : value=        (col.d!=0); return true;
            case SDT_STR   : value=TextBool(col.s   ); return true;
            case SDT_BINARY: value=false; REPA(col.b)if(col.b[i]){value=true; break;} return true;
         }
      }break;
   }
   value=false; return false;
}
Bool SQL::getCol(Int i, Byte &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL: if(SQLGetData(_statement, i+1, SQL_C_UTINYINT, &value, SIZE(value), null)==SQL_SUCCESS)return true; break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : value=         col.i ; return true;
            case SDT_DBL   : value=  RoundU(col.d); return true;
            case SDT_STR   : value=TextUInt(col.s); return true;
            case SDT_BINARY: value=(col.b.elms() ? col.b[0] : 0); return true;
         }
      }break;
   }
   value=0; return false;
}
Bool SQL::getCol(Int i, Int &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL: if(SQLGetData(_statement, i+1, SQL_C_SLONG, &value, SIZE(value), null)==SQL_SUCCESS)return true; break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : value=        col.i ; return true;
            case SDT_DBL   : value=  Round(col.d); return true;
            case SDT_STR   : value=TextInt(col.s); return true;
            case SDT_BINARY: value=0; FREP(Min(col.b.elms(), SIZEI(value)))value|=(col.b[i]<<(i*8)); return true;
         }
      }break;
   }
   value=0; return false;
}
Bool SQL::getCol(Int i, UInt &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL: if(SQLGetData(_statement, i+1, SQL_C_ULONG, &value, SIZE(value), null)==SQL_SUCCESS)return true; break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : value=         col.i ; return true;
            case SDT_DBL   : value=  RoundU(col.d); return true;
            case SDT_STR   : value=TextUInt(col.s); return true;
            case SDT_BINARY: value=0; FREP(Min(col.b.elms(), SIZEI(value)))value|=(col.b[i]<<(i*8)); return true;
         }
      }break;
   }
   value=0; return false;
}
Bool SQL::getCol(Int i, Long &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL: if(SQLGetData(_statement, i+1, SQL_C_SBIGINT, &value, SIZE(value), null)==SQL_SUCCESS)return true; break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : value=         col.i ; return true;
            case SDT_DBL   : value=         col.d ; return true;
            case SDT_STR   : value=TextLong(col.s); return true;
            case SDT_BINARY: value=0; FREP(Min(col.b.elms(), SIZEI(value)))value|=(ULong(col.b[i])<<(i*8)); return true;
         }
      }break;
   }
   value=0; return false;
}
Bool SQL::getCol(Int i, ULong &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL: if(SQLGetData(_statement, i+1, SQL_C_UBIGINT, &value, SIZE(value), null)==SQL_SUCCESS)return true; break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : value=          col.i ; return true;
            case SDT_DBL   : value=          col.d ; return true;
            case SDT_STR   : value=TextULong(col.s); return true;
            case SDT_BINARY: value=0; FREP(Min(col.b.elms(), SIZEI(value)))value|=(ULong(col.b[i])<<(i*8)); return true;
         }
      }break;
   }
   value=0; return false;
}
Bool SQL::getCol(Int i, Flt &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL: if(SQLGetData(_statement, i+1, SQL_C_FLOAT, &value, SIZE(value), null)==SQL_SUCCESS)return true; break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : value=        col.i ; return true;
            case SDT_DBL   : value=        col.d ; return true;
            case SDT_STR   : value=TextFlt(col.s); return true;
            case SDT_BINARY: value=0; FREP(Min(col.b.elms(), SIZEI(value)))(U32&)value|=(col.b[i]<<(i*8)); return true;
         }
      }break;
   }
   value=0; return false;
}
Bool SQL::getCol(Int i, Dbl &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL: if(SQLGetData(_statement, i+1, SQL_C_DOUBLE, &value, SIZE(value), null)==SQL_SUCCESS)return true; break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : value=        col.i ; return true;
            case SDT_DBL   : value=        col.d ; return true;
            case SDT_STR   : value=TextDbl(col.s); return true;
            case SDT_BINARY: value=0; FREP(Min(col.b.elms(), SIZEI(value)))(ULong&)value|=(ULong(col.b[i])<<(i*8)); return true;
         }
      }break;
   }
   value=0; return false;
}
Bool SQL::getCol(Int i, UID &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL: if(SQLGetData(_statement, i+1, (_type==MYSQL) ? SQL_C_BINARY : SQL_C_GUID, &value, SIZE(value), null)==SQL_SUCCESS)return true; break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : value.set    (col.i, 0); return true;
            case SDT_DBL   : value.set    (col.d, 0); return true;
            case SDT_STR   : value.fromHex(col.s   ); return true;
            case SDT_BINARY: value.zero(); REP(Min(col.b.elms(), Elms(value.b)))value.b[i]=col.b[i]; return true;
         }
      }break;
   }
   value.zero(); return false;
}
Bool SQL::getCol(Int i, Char *value, Int max_length)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL: if(SQLGetData(_statement, i+1, SQL_C_WCHAR, value, max_length, null)==SQL_SUCCESS)
      {
         if(_type==PGSQL)ReplaceSelf(value, '\r', '\0'); // PostgreSql will add '\r' characters to new lines
         return true;
      }break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : Set(value, TextInt   (col.i                           ), max_length); return true;
            case SDT_DBL   : Set(value, TextDbl   (col.d                           ), max_length); return true;
            case SDT_STR   : Set(value,            col.s                            , max_length); return true;
            case SDT_BINARY: Set(value, TextHexMem(col.b.data(), col.b.elms(), true), max_length); return true;
         }
      }break;
   }
   if(value && max_length>0)value[0]='\0'; return false;
}
Bool SQL::getCol(Int i, Char8 *value, Int max_length)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL: if(SQLGetData(_statement, i+1, SQL_C_CHAR, value, max_length, null)==SQL_SUCCESS)
      {
         if(_type==PGSQL)ReplaceSelf(value, '\r', '\0'); // PostgreSql will add '\r' characters to new lines
         return true;
      }break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : Set(value, TextInt   (col.i                           ), max_length); return true;
            case SDT_DBL   : Set(value, TextDbl   (col.d                           ), max_length); return true;
            case SDT_STR   : Set(value,            col.s                            , max_length); return true;
            case SDT_BINARY: Set(value, TextHexMem(col.b.data(), col.b.elms(), true), max_length); return true;
         }
      }break;
   }
   if(value && max_length>0)value[0]='\0'; return false;
}
Bool SQL::getCol(Int i, Str &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         Char temp[1024]; SQLLEN len=0;
         switch(SQLGetData(_statement, i+1, SQL_C_WCHAR, temp, SIZE(temp), &len))
         {
            case SQL_SUCCESS:
            {
               SQLSMALLINT type; if(_type==MYSQL && len==1 && (temp[0]==0 || temp[0]==1) && SQLDescribeCol(_statement, i+1, null, 0, null, &type, null, null, null)==SQL_SUCCESS_WITH_INFO)if(type==SQL_BIT){value=(temp[0] ? "1" : "0"); return true;} // MySQL returns incorrectly 0 or 1 for SQL_BIT instead of "0" or "1"
               if(len>0) // 'len' can be -1 for null
               {
                  value=temp;
                  if(_type==PGSQL)value.replace('\r', '\0'); // PostgreSql will add '\r' characters to new lines
               }else value.clear();
            }return true;

            case SQL_SUCCESS_WITH_INFO:
            {
               if(len>0)
               {
                  len/=2;
                  value.clear().reserve(len); // make room for remaining data, call this before "value=temp"
                  value=temp                ; // copy what was already received
                  value.reserve        (len); // make room for remaining data, call this after  "value=temp" (in case that would somehow make the buffer smaller)
                  if(SQLGetData(_statement, i+1, SQL_C_WCHAR, (Ptr)(value()+value.length()), (value._d.elms()-value.length())*2, null)==SQL_SUCCESS)
                  {
                     value._length=Length(value());
                     if(_type==PGSQL)value.replace('\r', '\0'); // PostgreSql will add '\r' characters to new lines
                     return true;
                  }
                  value._length=0;
               }
            }break;
         }
      }break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : value=           col.i                            ; return true;
            case SDT_DBL   : value=           col.d                            ; return true;
            case SDT_STR   : value=           col.s                            ; return true;
            case SDT_BINARY: value=TextHexMem(col.b.data(), col.b.elms(), true); return true;
         }
      }break;
   }
   value.clear(); return false;
}
Bool SQL::getCol(Int i, Str8 &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         Char8 temp[1024]; SQLLEN len=0;
         switch(SQLGetData(_statement, i+1, SQL_C_CHAR, temp, SIZE(temp), &len))
         {
            case SQL_SUCCESS:
            {
               SQLSMALLINT type; if(_type==MYSQL && len==1 && (temp[0]==0 || temp[0]==1) && SQLDescribeCol(_statement, i+1, null, 0, null, &type, null, null, null)==SQL_SUCCESS_WITH_INFO)if(type==SQL_BIT){value=(temp[0] ? "1" : "0"); return true;} // MySQL returns incorrectly 0 or 1 for SQL_BIT instead of "0" or "1"
               if(len>0) // 'len' can be -1 for null
               {
                  value=temp;
                  if(_type==PGSQL)value.replace('\r', '\0'); // PostgreSql will add '\r' characters to new lines
               }else value.clear();
            }return true;

            case SQL_SUCCESS_WITH_INFO:
            {
               if(len>0)
               {
                  value.clear().reserve(len); // make room for remaining data, call this before "value=temp"
                  value=temp                ; // copy what was already received
                  value.reserve        (len); // make room for remaining data, call this after  "value=temp" (in case that would somehow make the buffer smaller)
                  if(SQLGetData(_statement, i+1, SQL_C_CHAR, (Ptr)(value()+value.length()), value._d.elms()-value.length(), null)==SQL_SUCCESS)
                  {
                     value._length=Length(value());
                     if(_type==PGSQL)value.replace('\r', '\0'); // PostgreSql will add '\r' characters to new lines
                     return true;
                  }
                  value._length=0;
               }
            }break;
         }
      }break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : value=           col.i                            ; return true;
            case SDT_DBL   : value=           col.d                            ; return true;
            case SDT_STR   : value=           col.s                            ; return true;
            case SDT_BINARY: value=TextHexMem(col.b.data(), col.b.elms(), true); return true;
         }
      }break;
   }
   value.clear(); return false;
}
Bool SQL::getCol(Int i, MemPtr<Byte> value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         Byte temp[1024]; SQLLEN len=0;
         switch(SQLGetData(_statement, i+1, SQL_C_BINARY, temp, SIZE(temp), &len))
         {
            case SQL_SUCCESS: if(len>0)value.setNum(len).copyFrom(temp);else value.clear(); return true; // 'len' can be -1 for null
            case SQL_SUCCESS_WITH_INFO:
            {
               if(len>0)
               {
                  Memt<Byte> full; full.setNum(len);
                  CopyFast(full.data(), temp, SIZE(temp)); // copy what was already received
                  if(SQLGetData(_statement, i+1, SQL_C_BINARY, full.data()+SIZE(temp), full.elms()-SIZE(temp), null)==SQL_SUCCESS) // get the remaining part
                  {
                     value.setNum(full.elms()).copyFrom(full.data()); // copy everything to destination
                     return true;
                  }
               }
            }break;
         }
      }break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : value.setNum(SIZE(col.i)                    ).copyFrom((Byte*)&col.i  ); return true;
            case SDT_DBL   : value.setNum(SIZE(col.d)                    ).copyFrom((Byte*)&col.d  ); return true;
            case SDT_STR   : value.setNum(  (1+col.s.length())*SIZE(Char)).copyFrom((Byte*) col.s()); return true;
            case SDT_BINARY: value=col.b; return true;
         }
      }break;
   }
   value.clear(); return false;
}
Bool SQL::getCol(Int i, File &file)
{
   Memt<Byte> value; if(getCol(i, value))return file.put(value.data(), value.elms());
   return false;
}
Bool SQL::getCol(Int i, DateTime &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         TIMESTAMP_STRUCT ts; if(SQLGetData(_statement, i+1, SQL_C_TIMESTAMP, &ts, SIZE(ts), null)==SQL_SUCCESS)
         {
            value.year  =ts.year  ;
            value.month =ts.month ;
            value.day   =ts.day   ;
            value.hour  =ts.hour  ;
            value.minute=ts.minute;
            value.second=ts.second;
            return true;
         }
      }break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_STR   : value.fromText(col.s); return true;
            case SDT_BINARY:
            {
               value.zero();
               if(col.b.elms()>=1)value.second=col.b[0];
               if(col.b.elms()>=2)value.minute=col.b[1];
               if(col.b.elms()>=3)value.hour  =col.b[2];
               if(col.b.elms()>=4)value.day   =col.b[3];
               if(col.b.elms()>=5)value.month =col.b[4];
               if(col.b.elms()>=6){Int ofs=5; FREP(Min(col.b.elms()-ofs, SIZEI(value.year)))value.year|=(col.b[ofs+i]<<(i*8));}
            }return true;
         }
      }break;
   }
   value.zero(); return false;
}
Bool SQL::getCol(Int i, DateTimeMs &value)
{
   switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         TIMESTAMP_STRUCT ts; if(SQLGetData(_statement, i+1, SQL_C_TIMESTAMP, &ts, SIZE(ts), null)==SQL_SUCCESS)
         {
            value.year       =ts.year  ;
            value.month      =ts.month ;
            value.day        =ts.day   ;
            value.hour       =ts.hour  ;
            value.minute     =ts.minute;
            value.     second=ts.second;
            value.millisecond=ts.fraction/1000000; // 1ms is 1,000,000 units according to - https://docs.microsoft.com/en-us/sql/odbc/reference/appendixes/c-data-types
            return true;
         }
      }break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         { // TODO: 'millisecond' is ignored here
            case SDT_STR   : value.millisecond=0; value.fromText(col.s); return true;
            case SDT_BINARY:
            {
               value.zero();
               if(col.b.elms()>=1)value.second=col.b[0];
               if(col.b.elms()>=2)value.minute=col.b[1];
               if(col.b.elms()>=3)value.hour  =col.b[2];
               if(col.b.elms()>=4)value.day   =col.b[3];
               if(col.b.elms()>=5)value.month =col.b[4];
               if(col.b.elms()>=6){Int ofs=5; FREP(Min(col.b.elms()-ofs, SIZEI(value.year)))value.year|=(col.b[ofs+i]<<(i*8));}
            }return true;
         }
      }break;
   }
   value.zero(); return false;
}
Bool SQL::getCol(Int i, Ptr value, Int &size)
{
   if(value)switch(_type)
   {
   #if SUPPORT_ODBC
      case MSSQL:
      case MYSQL:
      case PGSQL:
      {
         SQLLEN len=0; if(SQLGetData(_statement, i+1, SQL_C_BINARY, value, size, &len)==SQL_SUCCESS)
         {
            size=len;
            return true;
         }
      }break;
   #endif

      case SQLITE: if(InRange(i, _cols) && InRange(_rows_pos, _rows))
      {
       C Row::Col &col=_rows[_rows_pos].cols[i];
         switch(   col.type)
         {
            case SDT_LONG  : MIN(size, SIZEI(col.i)                     ); CopyFast(value, &col.i       , size); return true;
            case SDT_DBL   : MIN(size, SIZEI(col.d)                     ); CopyFast(value, &col.d       , size); return true;
            case SDT_STR   : MIN(size,    (1+col.s.length())*SIZEI(Char)); CopyFast(value,  col.s     (), size); return true;
            case SDT_BINARY: MIN(size,       col.b.elms  ()             ); CopyFast(value,  col.b.data(), size); return true;
         }
      }break;
   }
   size=0; return false;
}
/******************************************************************************/
Str SQL::value(C SQLValues::Value &value)C
{
   switch(value.type)
   {
      default: return value.value; // SVT_RAW

      case SVT_STR: return string(value.value);

      case SVT_BIN: bin: switch(_type)
      {
         case MSSQL : return S+"0x"+value.value;                           // 0x     0x01     0x0102
         case MYSQL : return value.value.is() ? S+"0x"+value.value : "''"; // ''     0x01     0x0102
         case PGSQL : return S+"E'\\\\x"+value.value+'\'';                 // E'\\x' E'\\x01' E'\\x0102'
         case SQLITE: return S+"x'"+value.value+'\'';                      // x''    x'01'    x'0102'
      }break;

      case SVT_UID: switch(_type)
      {
         default   : goto bin;
         case PGSQL:
         {
            Str temp; temp.reserve(32+4+2);
                                         temp='\''; // order of characters was specified to match the output in 'getCol(Int i, UID &value)'
            for(Int i= 8-2; i>= 0; i-=2){temp+=value.value[i]; temp+=value.value[i+1];} temp+='-';
            for(Int i=12-2; i>= 8; i-=2){temp+=value.value[i]; temp+=value.value[i+1];} temp+='-';
            for(Int i=16-2; i>=12; i-=2){temp+=value.value[i]; temp+=value.value[i+1];} temp+='-';
            for(Int i=16  ; i< 20; i+=2){temp+=value.value[i]; temp+=value.value[i+1];} temp+='-';
            for(Int i=20  ; i< 32; i+=2){temp+=value.value[i]; temp+=value.value[i+1];} temp+='\'';
            return temp;
         }
      }break;
   }
   return S;
}
Str SQL::token(C Str &token)C
{
   switch(_type)
   {
      case MSSQL: return S+'['+Replace(token, "]", "]]")+']';

      case MYSQL :
      case SQLITE: return S+'`'+Replace(token, "`", "``")+'`';

      case PGSQL: return S+'"'+Replace(token, "\"", "\"\"")+'"';

      default: return token;
   }
}
Bool SQL::colDesc(C SQLColumn &col, Str &desc, Str *messages)
{
   desc=token(col.name)+' ';
   switch(_type)
   {
      default: if(messages)*messages="SQL not connected"; return false;

      case MSSQL:
      {
         switch(col.type)
         {
            case SDT_BOOL     : desc+="BIT "             ; break;
            case SDT_BYTE     : desc+="TINYINT "         ; break;
            case SDT_SHORT    : desc+="SMALLINT "        ; break;
            case SDT_INT      : desc+="INT "             ; break;
            case SDT_LONG     : desc+="BIGINT "          ; break;
            case SDT_FLT      : desc+="REAL "            ; break;
            case SDT_DBL      : desc+="FLOAT "           ; break;
            case SDT_UID      : desc+="UNIQUEIDENTIFIER "; break;
            case SDT_DATE_TIME: desc+="DATETIME "        ; break;
            case SDT_STR      : desc+=((col.   str_len <=0) ?  "NVARCHAR(MAX) " : S+"NVARCHAR("+col.   str_len +") "); break;
            case SDT_STR8     : desc+=((col.   str_len <=0) ?   "VARCHAR(MAX) " : S+ "VARCHAR("+col.   str_len +") "); break;
            case SDT_BINARY   : desc+=((col.binary_size<=0) ? "VARBINARY(MAX) " : S+  "BINARY("+col.binary_size+") "); break;
            default           : if(messages)*messages=S+"Can't create table with column \""+col.name+"\" of UNKNOWN type"; return false;
         }
         switch(col.mode)
         {
            case SQLColumn::UNIQUE      : desc+="UNIQUE "; break;
            case SQLColumn::PRIMARY     : desc+="PRIMARY KEY "; break;
            case SQLColumn::PRIMARY_AUTO: desc+="PRIMARY KEY IDENTITY(0,1) "; break;
         }
      }break;

      case MYSQL:
      {
         switch(col.type)
         {
            case SDT_BOOL     : desc+="BIT "       ; break;
            case SDT_BYTE     : desc+="TINYINT "   ; break;
            case SDT_SHORT    : desc+="SMALLINT "  ; break;
            case SDT_INT      : desc+="INT "       ; break;
            case SDT_LONG     : desc+="BIGINT "    ; break;
            case SDT_FLT      : desc+="FLOAT "     ; break;
            case SDT_DBL      : desc+="DOUBLE "    ; break;
            case SDT_UID      : desc+="BINARY(16) "; break;
            case SDT_DATE_TIME: desc+="DATETIME "  ; break;
            case SDT_STR      : desc+=((col.   str_len <=0) ? "LONGTEXT CHARACTER SET utf8 " : S+"VARCHAR("+col.   str_len +") CHARACTER SET utf8 "); break;
            case SDT_STR8     : desc+=((col.   str_len <=0) ? "LONGTEXT "                    : S+"VARCHAR("+col.   str_len +") "                   ); break;
            case SDT_BINARY   : desc+=((col.binary_size<=0) ? "LONGBLOB "                    : S+ "BINARY("+col.binary_size+") "                   ); break;
            default           : if(messages)*messages=S+"Can't create table with column \""+col.name+"\" of UNKNOWN type"; return false;
         }
         if(col.mode==SQLColumn::PRIMARY_AUTO)desc+="AUTO_INCREMENT ";
      }break;

      case PGSQL:
      {
         if(col.mode==SQLColumn::PRIMARY_AUTO)switch(col.type)
         {
            case SDT_BYTE : desc+=S+"smallserial PRIMARY KEY "; break; // PostgreSQL doesn't have a 'byte' type
            case SDT_SHORT: desc+=S+"smallserial PRIMARY KEY "; break;
            case SDT_INT  : desc+=S+     "serial PRIMARY KEY "; break;
            case SDT_LONG : desc+=S+  "bigserial PRIMARY KEY "; break;
            default       : if(messages)*messages=S+"Can't create table with column \""+col.name+"\" of invalid type for PRIMARY_AUTO"; return false;
         }else
         switch(col.type)
         {
            case SDT_BOOL     : desc+="boolean "         ; break;
            case SDT_BYTE     : desc+="smallint "        ; break; // PostgreSQL doesn't have a 'byte' type
            case SDT_SHORT    : desc+="smallint "        ; break;
            case SDT_INT      : desc+="integer "         ; break;
            case SDT_LONG     : desc+="bigint "          ; break;
            case SDT_FLT      : desc+="real "            ; break;
            case SDT_DBL      : desc+="double precision "; break;
            case SDT_UID      : desc+="uuid "            ; break;
            case SDT_DATE_TIME: desc+="timestamp "       ; break;
            case SDT_STR      : desc+=((col.str_len<=0) ? "text " : S+"varchar("+col.str_len+") "); break;
            case SDT_STR8     : desc+=((col.str_len<=0) ? "text " : S+"varchar("+col.str_len+") "); break;
            case SDT_BINARY   : desc+="bytea "; break;
            default           : if(messages)*messages=S+"Can't create table with column \""+col.name+"\" of UNKNOWN type"; return false;
         }
         switch(col.mode)
         {
            case SQLColumn::PRIMARY: desc+=S+"PRIMARY KEY "; break;
            case SQLColumn::UNIQUE : desc+=S+"UNIQUE "; break;
         }
      }break;

      case SQLITE:
      {
         if(col.mode==SQLColumn::PRIMARY_AUTO)desc+="INTEGER PRIMARY KEY ";else // PRIMARY_AUTO must use INTEGER
         switch(col.type)
         {
            case SDT_BOOL     : desc+="BIT "       ; break;
            case SDT_BYTE     : desc+="TINYINT "   ; break;
            case SDT_SHORT    : desc+="SMALLINT "  ; break;
            case SDT_INT      : desc+="INT "       ; break;
            case SDT_LONG     : desc+="BIGINT "    ; break;
            case SDT_FLT      : desc+="FLOAT "     ; break;
            case SDT_DBL      : desc+="DOUBLE "    ; break;
            case SDT_UID      : desc+="BINARY(16) "; break;
            case SDT_DATE_TIME: desc+="DATETIME "  ; break;
            case SDT_STR      : desc+=((col.   str_len <=0) ? "TEXT " : S+"NVARCHAR("+col.   str_len +") "); break;
            case SDT_STR8     : desc+=((col.   str_len <=0) ? "TEXT " : S+ "VARCHAR("+col.   str_len +") "); break;
            case SDT_BINARY   : desc+=((col.binary_size<=0) ? "BLOB " : S+  "BINARY("+col.binary_size+") "); break;
            default           : if(messages)*messages=S+"Can't create table with column \""+col.name+"\" of UNKNOWN type"; return false;
         }
         switch(col.mode)
         {
            case SQLColumn::UNIQUE : desc+="UNIQUE "; break;
            case SQLColumn::PRIMARY: desc+="PRIMARY KEY "; break;
         }
      }break;
   }
   desc+=(col.allow_nulls ? "NULL " : "NOT NULL ");
   if(col.default_val.is())
   {
      if(col.type==SDT_STR || col.type==SDT_STR8)
      {
         desc+="DEFAULT "; desc+=string(col.default_val);
      }else
      {
         desc+="DEFAULT "; desc+=col.default_val;
      }
   }
   if(_type==MYSQL)
   {
      if(col.mode==SQLColumn::PRIMARY || col.mode==SQLColumn::PRIMARY_AUTO)desc+=S+", PRIMARY KEY ("+token(col.name          )+")";
      if(col.mode!=SQLColumn::DEFAULT                                     )desc+=S+", UNIQUE INDEX "+token(col.name+"_UNIQUE")+" ("+token(col.name)+" ASC)";
   }
   return true;
}
#if SUPPORT_ODBC
void SQL::getStatus(Str *messages, Int *error, Bool statement)
{
   if(messages || error)
   {
      if(messages)messages->clear();
      if(error   )*error=0;
   #if DEBUG
      SQLWCHAR text[1024], state[1024]; SQLINTEGER sql_error;
      if(SQLGetDiagRec(statement ? SQL_HANDLE_STMT : SQL_HANDLE_DBC, statement ? T._statement : T._conn, 1, state, &sql_error, text, Elms(text), null)==SQL_SUCCESS){if(messages)*messages=(Char*)text; if(error)*error=sql_error;}
   #else
      SQLWCHAR text[1024]; SQLINTEGER sql_error;
      if(SQLGetDiagRec(statement ? SQL_HANDLE_STMT : SQL_HANDLE_DBC, statement ? T._statement : T._conn, 1, null, &sql_error, text, Elms(text), null)==SQL_SUCCESS){if(messages)*messages=(Char*)text; if(error)*error=sql_error;}
   #endif
   }
}
#endif
/******************************************************************************/
// SQLite VFS
/******************************************************************************/
#if SUPPORT_SQLITE

struct SQLiteFile : sqlite3_file // 'sqlite3_file' base must be first
{
   File file;

   SQLiteFile() {Zero(SCAST(sqlite3_file, T));}
};

static int SQLiteClose(sqlite3_file *pFile)
{
   SQLiteFile &f=*(SQLiteFile*)pFile;
   int rc=(f.file.flush() ? SQLITE_OK : SQLITE_IOERR_WRITE);
   f.file.del(); DTOR(f); return rc;
}
static int SQLiteRead(sqlite3_file *pFile, void *zBuf, int iAmt, sqlite_int64 iOfst)
{
   SQLiteFile &f=*(SQLiteFile*)pFile;
   if(!f.file.pos(iOfst))return SQLITE_IOERR_SEEK; // TODO: this could trigger data loss if flush failed, however how to handle this error?
   Int read=f.file.getReturnSize(zBuf, iAmt);
   if( read>=iAmt              )return SQLITE_OK; // read all
   if( read<=0 && !f.file.end())return SQLITE_IOERR_READ; // read nothing but still have some left (not at the end)
   // partial read
   Zero((Byte*)zBuf+read, iAmt-read); // zero remaining data (this is required by SQLite)
   return SQLITE_IOERR_SHORT_READ;
}
static int SQLiteWrite(sqlite3_file *pFile, const void *zBuf, int iAmt, sqlite_int64 iOfst)
{
   SQLiteFile &f=*(SQLiteFile*)pFile;
   if(!f.file.pos(iOfst))return SQLITE_IOERR_SEEK; // TODO: this could trigger data loss if flush failed, however how to handle this error?
   return f.file.put(zBuf, iAmt) ? SQLITE_OK : SQLITE_IOERR_WRITE;
}
static int SQLiteTruncate(sqlite3_file *pFile, sqlite_int64 size)
{
   SQLiteFile &f=*(SQLiteFile*)pFile;
   return f.file.size(size) ? SQLITE_OK : SQLITE_IOERR_TRUNCATE;
}
static int SQLiteSync(sqlite3_file *pFile, int flags)
{
   SQLiteFile &f=*(SQLiteFile*)pFile;
   if(!f.file.flush())return SQLITE_IOERR_WRITE;
   return f.file.sync() ? SQLITE_OK : SQLITE_IOERR_FSYNC;
}
static int SQLiteFileSize(sqlite3_file *pFile, sqlite_int64 *pSize)
{
   SQLiteFile &f=*(SQLiteFile*)pFile;
   *pSize=f.file.size();
   return SQLITE_OK;
}
static int SQLiteLock                 (sqlite3_file *pFile, int eLock) {return SQLITE_OK;}
static int SQLiteUnlock               (sqlite3_file *pFile, int eLock) {return SQLITE_OK;}
static int SQLiteCheckReservedLock    (sqlite3_file *pFile, int *pResOut) {*pResOut=0; return SQLITE_OK;}
static int SQLiteFileControl          (sqlite3_file *pFile, int op, void *pArg) {return SQLITE_NOTFOUND;}
static int SQLiteSectorSize           (sqlite3_file *pFile) {return 4096;} // use some default based on SQLite SQLITE_DEFAULT_SECTOR_SIZE
static int SQLiteDeviceCharacteristics(sqlite3_file *pFile) {return (WINDOWS ? SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN : 0);}

static const sqlite3_io_methods SQLiteIO=
{
   1,                           /* iVersion */
   SQLiteClose,                 /* xClose */
   SQLiteRead,                  /* xRead */
   SQLiteWrite,                 /* xWrite */
   SQLiteTruncate,              /* xTruncate */
   SQLiteSync,                  /* xSync */
   SQLiteFileSize,              /* xFileSize */
   SQLiteLock,                  /* xLock */
   SQLiteUnlock,                /* xUnlock */
   SQLiteCheckReservedLock,     /* xCheckReservedLock */
   SQLiteFileControl,           /* xFileControl */
   SQLiteSectorSize,            /* xSectorSize */
   SQLiteDeviceCharacteristics, /* xDeviceCharacteristics */
};
static int SQLiteOpen(sqlite3_vfs *pVfs, const char *zName, sqlite3_file *pFile, int flags, int *pOutFlags)
{
   if(pOutFlags)*pOutFlags=flags;
   SQLiteFile &f=*(SQLiteFile*)pFile;
   CTOR(f);
   f.pMethods=&SQLiteIO;
   Str name=FromUTF8(zName);
   if((flags&SQLITE_OPEN_READONLY) ? f.file.  readTry(name, SQLiteCipher)
                                   : f.file.appendTry(name, SQLiteCipher))return SQLITE_OK;
   return SQLITE_CANTOPEN; // 'SQLiteClose' will be called upon 'SQLITE_CANTOPEN'
}
static int SQLiteDelete(sqlite3_vfs *pVfs, const char *zPath, int dirSync)
{
   Str name=FromUTF8(zPath);
   return (!FExistSystem(name) || FDelFile(name)) ? SQLITE_OK : SQLITE_IOERR_DELETE;
}
static int SQLiteAccess(sqlite3_vfs *pVfs, const char *zPath, int flags, int *pResOut)
{
   FileInfo fi; Bool exists=fi.get(FromUTF8(zPath));
   switch(flags)
   {
      case SQLITE_ACCESS_EXISTS   : *pResOut=(exists &&  fi.size>0                    ); break; // SQLite requires check for size>0
      case SQLITE_ACCESS_READ     : *pResOut= exists                                   ; break;
      case SQLITE_ACCESS_READWRITE: *pResOut=(exists && !(fi.attrib&FATTRIB_READ_ONLY)); break;
      default                     : return SQLITE_ERROR;
   }
   return SQLITE_OK;
}
static int SQLiteFullPathname(sqlite3_vfs *pVfs, const char *zPath, int nPathOut, char *zPathOut)
{
   Set(zPathOut, zPath, nPathOut); // just copy
   return SQLITE_OK;
}

static void  *SQLiteDlOpen (sqlite3_vfs *pVfs, const char *zPath) {return null;}
static void   SQLiteDlError(sqlite3_vfs *pVfs, int nByte, char *zErrMsg) {Set(zErrMsg, "Loadable extensions are not supported", nByte);}
static void (*SQLiteDlSym  (sqlite3_vfs *pVfs, void *pH, const char *z))(void) {return null;}
static void   SQLiteDlClose(sqlite3_vfs *pVfs, void *pHandle) {}
/******************************************************************************/
static struct SQLiteInitializer // thanks to this class, we don't need to call a function such as 'InitSQL' (now removed), if we would call it from other CPP file, then we would always link with this CPP file, and forced inclusion of SQLite libraries which would increase the executable file size even if the app would not use it
{
   static sqlite3_vfs SQLiteVFS;

   SQLiteInitializer()
   {
      sqlite3_initialize();
      if(sqlite3_vfs *default_vfs=sqlite3_vfs_find(null))
      {
         SQLiteVFS.xRandomness      =default_vfs->xRandomness;
         SQLiteVFS.xSleep           =default_vfs->xSleep;
         SQLiteVFS.xCurrentTime     =default_vfs->xCurrentTime;
         SQLiteVFS.xGetLastError    =null;
         SQLiteVFS.xCurrentTimeInt64=default_vfs->xCurrentTimeInt64;
         sqlite3_vfs_register(&SQLiteVFS, false);
      }
   }
  ~SQLiteInitializer()
   {
      sqlite3_vfs_unregister(&SQLiteVFS);
    //sqlite3_shutdown(); don't call this in case there are still open SQL connections, as the SQLite header says: "All open [database connections] must be closed and all other SQLite resources must be deallocated prior to invoking 'sqlite3_shutdown'
   }
}SQLiteInit; // this object is needed to force calling constructor at the start of the Application

sqlite3_vfs SQLiteInitializer::SQLiteVFS=
{
   2,                  /* iVersion */
   SIZE(SQLiteFile),   /* szOsFile */
   MAX_UTF_PATH,       /* mxPathname */
   null,               /* pNext */
   SQLITE_VFS_NAME,    /* zName */
   null,               /* pAppData */
   SQLiteOpen,         /* xOpen */
   SQLiteDelete,       /* xDelete */
   SQLiteAccess,       /* xAccess */
   SQLiteFullPathname, /* xFullPathname */
   SQLiteDlOpen,       /* xDlOpen */
   SQLiteDlError,      /* xDlError */
   SQLiteDlSym,        /* xDlSym */
   SQLiteDlClose,      /* xDlClose */
};
#endif
/******************************************************************************/
}
/******************************************************************************/
