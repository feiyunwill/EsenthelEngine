/******************************************************************************

   This tutorial presents the basics of SQL usage.

   Esenthel Engine supports 3 SQL engines:
      MS SQL
      MySQL
      SQLite

   This tutorial by default uses SQLite.
   
   If you wish to use MS SQL, please first Install and Configure it:

      1. Installing MS SQL Server
         In order for this tutorial to work, you must have installed MS SQL Server on your computer.
         You can obtain a free version of the SQL Server by downloading Express edition.
            http://www.microsoft.com/express/Database/InstallOptions.aspx
         It is recommended to download "Database with Management Tools" version.

      2. Creating a Database
         After you have downloaded and installed the SQL Server,
         please launch "SQL Server Management Studio" program from Windows Start Menu.
         In the popup window connect to your server:
            Default options should be set to:
               Server type   : Database Engine
               Server name   : LocalHost\SQLExpress
               Authentication: Windows Authorization
         After you have connected to the server, right click on the "Databases" element in the Object Explorer tree view,
         and select "new database".
         Enter "test_db" as your database name, press ok and the database should appear in the "Databases" folder.

      3. Using the SQL
         Now that we have installed the SQL server, and created a database, we're good to go, and we can use SQL functions.

/******************************************************************************/
enum SQL_TYPE
{
   SQL_MSSQL ,
   SQL_SQLITE,
}
SQL_TYPE   sql_type=SQL_SQLITE;
SQL        sql    ; // let's create a global SQL object that will handle sql connection to our game database
Memc<Str>  players; // list of player names that we will use to draw them on the screen
/******************************************************************************/
void SQLTests()
{
   Str messages;

   // connect to server/database
   switch(sql_type)
   {
      case SQL_MSSQL : if(!sql.connectMSSQL ("LocalHost\\SQLExpress", "test_db", "", "", &messages))Exit(S+"Can't connect to Server!\nPlease read instructions in the tutorial codes.\nGiven Error:\n"+messages); break;
      case SQL_SQLITE: if(!sql.connectSQLite("sqlite.db"))Exit("Can't connect to Server!"); break;
   }

   Str table_name="players"; // define a name for the table that we'll use in this tutorial

   // create a sample table
   {
      if(!sql.existsTable(table_name)) // if it wasn't yet created
      {
         Memc<SQLColumn> columns; // define a list of columns for the table
         columns.New().set("id"        , SDT_INT      ).mode=SQLColumn.PRIMARY_AUTO; // set a new column named "id"         of INT       type, and set it to a special mode of PRIMARY_AUTO, which will give us the primary key, with automatic setting of its value
         columns.New().set("name"      , SDT_STR  , 32);                             // set a new column named "name"       of STR       type, and maximum length of 32 characters
         columns.New().set("e_mail"    , SDT_STR  , 32);                             // set a new column named "e_mail"     of STR       type, and maximum length of 32 characters
         columns.New().set("clan"      , SDT_STR      ).default_val="default";       // set a new column named "group"      of STR       type, and set its default value to "default" string
         columns.New().set("registered", SDT_DATE_TIME);                             // set a new column named "registered" of DATE_TIME type
         if(!sql.createTable(table_name, columns, &messages))Exit(S+"Can't create table:\n"+messages);

         // after calling 'createTable' you can switch to the "SQL Server Management Studio" application window,
         // expand your "test_db" database, expand "Tables" folder, and notice that there should be a new table that we've just created
         // (sometimes you may need to right-click on the "Tables" and select "Refresh" to see the changes)
      }
   }

   // add players
   {
      if(!sql.newRow(table_name, SQLValues().New("name", "PlayerName").New("e_mail", "address@domain.com").New("registered", DateTime().getUTC()), &messages))Exit(S+"Can't create player:\n"+messages);
      if(!sql.newRow(table_name, SQLValues().New("name", "Player #2" ).New("e_mail",     "xxx@domain.com").New("registered", DateTime().getUTC()), &messages))Exit(S+"Can't create player:\n"+messages);
   }

   // read data from the SQL database
   {
      if(!sql.getAllRows(table_name, &messages))Exit(S+"Can't get rows from table:\n"+messages);

      // optionally you can get the information about columns from returned rows
      int  row_columns=sql.getCols();
      FREP(row_columns)
      {
         Str           col_name;
         SQL_DATA_TYPE col_type;
         int           col_size;
         if(sql.getColDesc(i, col_name, col_type, col_size))
         {
            int z=0; // here you can investigate the values of the name, type, size column parameters when running in debug mode
         }
      }

      // now let's process all returned rows
      for(;sql.getNextRow();)
      {
         // !! you can read data from columns only in sequential order, read from 0th, 1st, 2nd, .. column, but not from 2nd, 1st, 0th !!
         uint     id; sql.getCol(0, id  ); // we know that the 0-th column is the ID
         Str    name; sql.getCol(1, name); // we know that the 1-st column is the name
                                           // let's skip columns 2 and 3
         DateTime dt; sql.getCol(4, dt  ); // we know that the 4-th column is the date of registration

         players.New()=name; // store the name in the global container, so we can draw it later
      }
   }

   // adjust data in SQL
   {
      // modify 'clan' column to "Ghosts" for all players that have 'id' equal to 1
      if(!sql.setRow(table_name, "id=1", SQLValues().New("clan", "Ghosts"), &messages))Exit(S+"Can't modify clan for player:\n"+messages);

      // modify 'clan' column to "Zombies" for all players that have 'e_mail' equal to "address@domain.com"
      // please note that for comparing strings we must use 'SQL.string' method
      if(!sql.setRow(table_name, S+"e_mail="+sql.string("address@domain.com"), SQLValues().New("clan", "Zombies"), &messages))Exit(S+"Can't modify clan for player:\n"+messages);
   }
   
   // after performing all operations visit the "SQL Server Management Studio" application window again
   // and notice the changes that have been made in the table
   // (you may need to right-click on the table and select "Refresh" to see the changes)
}
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
bool Init()
{
   SQLTests();
   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);

   // draw list of players
   D.text(0, 0.9, "Players:");
   FREPA(players)D.text(0, 0.7-i*0.1, players[i]);
}
/******************************************************************************/
