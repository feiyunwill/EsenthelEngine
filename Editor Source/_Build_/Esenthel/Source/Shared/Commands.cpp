/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
// CS_VERSION_CHECK
/******************************************************************************/
void ClientSendVersion(Connection &conn)
{
   File f; f.writeMem().putByte(CS_VERSION_CHECK).cmpUIntV(ClientServerVersion).putStr(ClientServerString).pos(0); conn.send(f);
}
bool ServerRecvVersion(File &f)
{
   if(f.decUIntV()==ClientServerVersion)if(f.getStr()==ClientServerString)return true;
   return false;
}
void ServerSendVersion(Connection &conn, bool ok)
{
   File f; f.writeMem().putByte(CS_VERSION_CHECK).putBool(ok).pos(0); conn.send(f, -1, false);
}
bool ClientRecvVersion(File &f)
{
   return f.getBool();
}
/******************************************************************************/
// CS_LOGIN
/******************************************************************************/
void ClientSendLogin(Connection &conn, C Str &email, C Str &pass, C Str &license_key, OS_VER os_ver)
{
   File f; f.writeMem().putByte(CS_LOGIN)<<email<<license_key<<PassToMD5(pass); f.putByte(os_ver); f.pos(0); conn.send(f);
}
void ServerRecvLogin(File &f, Str &email, UID &pass, Str &license_key, OS_VER &os_ver)
{
   f>>email>>license_key>>pass;
   os_ver=OS_VER(f.getByte());
}
void ServerSendLogin(Connection &conn, LOGIN_RESULT result, USER_ACCESS access)
{
   File f; f.writeMem().putByte(CS_LOGIN).putByte(result).putByte(access); f.pos(0); conn.send(f, -1, false);
}
void ClientRecvLogin(File &f, LOGIN_RESULT &result, USER_ACCESS &access)
{
   result=LOGIN_RESULT(f.getByte());
   access=USER_ACCESS (f.getByte());
}
/******************************************************************************/
// CS_REGISTER
/******************************************************************************/
void ClientSendRegister(Connection &conn, C Str &email, C Str &pass, C Str &name)
{
   File f; f.writeMem().putByte(CS_REGISTER)<<email<<name<<PassToMD5(pass); f.pos(0); conn.send(f);
}
void ServerRecvRegister(File &f, Str &email, UID &pass, Str &name)
{
   f>>email>>name>>pass;
}
void ServerSendRegister(Connection &conn, REGISTER_RESULT result)
{
   File f; f.writeMem().putByte(CS_REGISTER).putByte(result); f.pos(0); conn.send(f, -1, false);
}
void ClientRecvRegister(File &f, REGISTER_RESULT &result)
{
   result=REGISTER_RESULT(f.getByte());
}
/******************************************************************************/
// CS_FORGOT_PASS
/******************************************************************************/
void ClientSendForgotPass(Connection &conn, C Str &email)
{
   File f; f.writeMem().putByte(CS_FORGOT_PASS)<<email; f.pos(0); conn.send(f);
}
void ServerRecvForgotPass(File &f, Str &email)
{
   f>>email;
}
void ServerSendForgotPass(Connection &conn, FORGOT_PASS_RESULT result)
{
   File f; f.writeMem().putByte(CS_FORGOT_PASS).putByte(result); f.pos(0); conn.send(f, -1, false);
}
void ClientRecvForgotPass(File &f, FORGOT_PASS_RESULT &result)
{
   result=FORGOT_PASS_RESULT(f.getByte());
}
/******************************************************************************/
// CS_CHANGE_PASS
/******************************************************************************/
void ClientSendChangePass(Connection &conn, C Str &email, C Str &new_pass, C Str &old_pass, uint change_pass_key)
{
   File f; f.writeMem().putByte(CS_CHANGE_PASS)<<email<<PassToMD5(new_pass)<<PassToMD5(old_pass)<<change_pass_key; f.pos(0); conn.send(f);
}
void ServerRecvChangePass(File &f, Str &email, UID &new_pass, UID &old_pass, uint &change_pass_key)
{
   f>>email>>new_pass>>old_pass>>change_pass_key;
}
void ServerSendChangePass(Connection &conn, CHANGE_PASS_RESULT result)
{
   File f; f.writeMem().putByte(CS_CHANGE_PASS).putByte(result); f.pos(0); conn.send(f, -1, false);
}
void ClientRecvChangePass(File &f, CHANGE_PASS_RESULT &result)
{
   result=CHANGE_PASS_RESULT(f.getByte());
}
/******************************************************************************/
// CS_LICENSE_KEY
/******************************************************************************/
void ClientSendLicenseKey(Connection &conn, C Str &license_key)
{
   File f; f.writeMem().putByte(CS_LICENSE_KEY)<<license_key; f.pos(0); conn.send(f);
}
void ServerRecvLicenseKey(File &f, Str &license_key)
{
   f>>license_key;
}
/******************************************************************************/
// CS_PROJECTS_LIST
/******************************************************************************/
void ClientSendProjectsListRequest(Connection &conn)
{
   File f; f.writeMem().putByte(CS_PROJECTS_LIST); f.pos(0); conn.send(f, -1, false);
}
void ServerSendProjectsList(Connection &conn, Memx<Project> &projects)
{
   File f; f.writeMem().putByte(CS_PROJECTS_LIST).cmpUIntV(projects.elms()); FREPAO(projects).save(f, true, Project::SAVE_ID_NAME); f.pos(0); conn.send(f, -1, false);
}
bool ClientRecvProjectsList(File &f, Memx<Project> &projects)
{
   projects.setNum(f.decUIntV()); int ver; FREPA(projects)if(projects[i].load(f, ver, true, Project::SAVE_ID_NAME)!=LOAD_OK){projects.clear(); return false;}
   return true;
}
/******************************************************************************/
// CS_PROJECT_OPEN
/******************************************************************************/
void ClientSendProjectOpen(Connection &conn, C UID &proj_id, C Str &proj_name)
{
   File f; f.writeMem().putByte(CS_PROJECT_OPEN)<<proj_name<<proj_id; f.pos(0); conn.send(f, -1, false);
}
void ServerRecvProjectOpen(File &f, UID &proj_id, Str &proj_name)
{
   f>>proj_name>>proj_id;
}
/******************************************************************************/
// CS_PROJECT_DATA
/******************************************************************************/
void ClientSendGetProjectData(Connection &conn) // this asks the Server to send Data for currently opened project
{
   File f; f.writeMem().putByte(CS_PROJECT_DATA); f.pos(0); conn.send(f, -1, false);
}
bool ServerSendProjectData(Connection &conn, Project &project)
{
   File f; f.writeMem().putByte(CS_PROJECT_DATA);
   File data; project.save(data.writeMem(), true); data.pos(0);
   // for a big project of uncompressed "Data" file around 9MB, there were following compression results
   // COMPRESS_LZ4  9 -> 1.6MB 0.075s
   // COMPRESS_LZMA 5 -> 1.1MB 0.388s
   if(Compress(data, f, ServerNetworkCompression, ServerNetworkCompressionLevel)){f.pos(0); conn.send(f, -1, false); return true;}
   return false;
}
bool ClientRecvProjectData(File &f, Project &project)
{
   File decompressed; if(Decompress(f, decompressed, true)){decompressed.pos(0); int ver; return project.load(decompressed, ver, true)==LOAD_OK;}
   return false;
}
/******************************************************************************/
// CS_PROJECT_SETTINGS
/******************************************************************************/
void ClientSendProjectSettings(Connection &conn, Project &project)
{
   File f; f.writeMem().putByte(CS_PROJECT_SETTINGS); project.save(f, true, Project::SAVE_SETTINGS); f.pos(0); conn.send(f, -1, false);
}
bool ServerRecvProjectSettings(File &f, Project &project)
{
   int ver; return project.load(f, ver, true, Project::SAVE_SETTINGS)==LOAD_OK;
}
void ServerWriteProjectSettings(File &f, Project &project)
{
   f.putByte(CS_PROJECT_SETTINGS); project.save(f, true, Project::SAVE_SETTINGS);
}
bool ClientRecvProjectSettings(File &f, Project &project)
{
   int ver; return project.load(f, ver, true, Project::SAVE_SETTINGS)==LOAD_OK;
}
/******************************************************************************/
// CS_NEW_ELM
/******************************************************************************/
void ClientSendNewElm(Connection &conn, Elm &elm)
{
   File f; f.writeMem().putByte(CS_NEW_ELM); elm.compressNew(f); f.pos(0); conn.send(f, -1, false);
}
void ServerRecvNewElm(File &f, Elm &elm)
{
   elm.decompressNew(f);
}
void ServerWriteNewElm(File &f, Elm &elm, C UID &proj_id)
{
   f.putByte(CS_NEW_ELM); elm.compressNew(f); f<<proj_id;
}
void ClientRecvNewElm(File &f, Elm &elm, UID &proj_id)
{
   elm.decompressNew(f); f>>proj_id;
}
/******************************************************************************/
// CS_RENAME_ELM
/******************************************************************************/
void ClientSendRenameElm(Connection &conn, C UID &elm_id, C Str &name, C TimeStamp &name_time)
{
   File f; f.writeMem().putByte(CS_RENAME_ELM)<<elm_id<<name_time<<name; f.pos(0); conn.send(f, -1, false);
}
void ServerRecvRenameElm(File &f, UID &elm_id, Str &name, TimeStamp &name_time)
{
   f>>elm_id>>name_time>>name;
}
void ServerWriteRenameElm(File &f, C UID &elm_id, C Str &name, C TimeStamp &name_time, C UID &proj_id)
{
   f.putByte(CS_RENAME_ELM)<<elm_id<<name_time<<proj_id<<name;
}
void ClientRecvRenameElm(File &f, UID &elm_id, Str &name, TimeStamp &name_time, UID &proj_id)
{
   f>>elm_id>>name_time>>proj_id>>name;
}
/******************************************************************************/
// CS_SET_ELM_PARENT
/******************************************************************************/
void ClientSendSetElmParent(Connection &conn, C UID &elm_id, C UID &parent_id, C TimeStamp &parent_time)
{
   File f; f.writeMem().putByte(CS_SET_ELM_PARENT)<<elm_id<<parent_id<<parent_time; f.pos(0); conn.send(f, -1, false);
}
void ServerRecvSetElmParent(File &f, UID &elm_id, UID &parent_id, TimeStamp &parent_time)
{
   f>>elm_id>>parent_id>>parent_time;
}
void ServerWriteSetElmParent(File &f, C UID &elm_id, C UID &parent_id, C TimeStamp &parent_time, C UID &proj_id)
{
   f.putByte(CS_SET_ELM_PARENT)<<elm_id<<parent_id<<parent_time<<proj_id;
}
void ClientRecvSetElmParent(File &f, UID &elm_id, UID &parent_id, TimeStamp &parent_time, UID &proj_id)
{
   f>>elm_id>>parent_id>>parent_time>>proj_id;
}
/******************************************************************************/
// CS_REMOVE_ELMS
/******************************************************************************/
void ClientSendRemoveElms(Connection &conn, Memc<UID> &elm_ids, bool removed, C TimeStamp &removed_time)
{
   if(elm_ids.elms()){File f; f.writeMem().putByte(CS_REMOVE_ELMS).cmpUIntV(elm_ids.elms())<<removed<<removed_time; FREPA(elm_ids)f<<elm_ids[i]; f.pos(0); conn.send(f, -1, false);}
}
void ServerRecvRemoveElms(File &f, Memc<UID> &elm_ids, bool &removed, TimeStamp &removed_time)
{
   elm_ids.setNum(f.decUIntV()); f>>removed>>removed_time; FREPA(elm_ids)f>>elm_ids[i];
}
void ServerWriteRemoveElms(File &f, Memc<UID> &elm_ids, bool removed, C TimeStamp &removed_time, C UID &proj_id)
{
   f.putByte(CS_REMOVE_ELMS).cmpUIntV(elm_ids.elms())<<removed<<removed_time<<proj_id; FREPA(elm_ids)f<<elm_ids[i];
}
void ClientRecvRemoveElms(File &f, Memc<UID> &elm_ids, bool &removed, TimeStamp &removed_time, UID &proj_id)
{
   elm_ids.setNum(f.decUIntV()); f>>removed>>removed_time>>proj_id; FREPA(elm_ids)f>>elm_ids[i];
}
/******************************************************************************/
// CS_NO_PUBLISH_ELMS
/******************************************************************************/
void ClientSendNoPublishElms(Connection &conn, Memc<UID> &elm_ids, bool no_publish, C TimeStamp &no_publish_time)
{
   if(elm_ids.elms()){File f; f.writeMem().putByte(CS_NO_PUBLISH_ELMS).cmpUIntV(elm_ids.elms())<<no_publish<<no_publish_time; FREPA(elm_ids)f<<elm_ids[i]; f.pos(0); conn.send(f, -1, false);}
}
void ServerRecvNoPublishElms(File &f, Memc<UID> &elm_ids, bool &no_publish, TimeStamp &no_publish_time)
{
   elm_ids.setNum(f.decUIntV()); f>>no_publish>>no_publish_time; FREPA(elm_ids)f>>elm_ids[i];
}
void ServerWriteNoPublishElms(File &f, Memc<UID> &elm_ids, bool no_publish, C TimeStamp &no_publish_time, C UID &proj_id)
{
   f.putByte(CS_NO_PUBLISH_ELMS).cmpUIntV(elm_ids.elms())<<no_publish<<no_publish_time<<proj_id; FREPA(elm_ids)f<<elm_ids[i];
}
void ClientRecvNoPublishElms(File &f, Memc<UID> &elm_ids, bool &no_publish, TimeStamp &no_publish_time, UID &proj_id)
{
   elm_ids.setNum(f.decUIntV()); f>>no_publish>>no_publish_time>>proj_id; FREPA(elm_ids)f>>elm_ids[i];
}
/******************************************************************************/
// CS_GET_ELM_NAMES
/******************************************************************************/
void ClientSendGetElmNames(Connection &conn, Memc<UID> &elm_ids)
{
   if(elm_ids.elms()){File f; f.writeMem().putByte(CS_GET_ELM_NAMES).cmpUIntV(elm_ids.elms()); FREPA(elm_ids)f<<elm_ids[i]; f.pos(0); conn.send(f, -1, false);}
}
void ServerRecvGetElmNames(File &f, Memc<UID> &elm_ids)
{
   elm_ids.setNum(f.decUIntV()); FREPA(elm_ids)f>>elm_ids[i];
}
void ServerSendGetElmNames(Connection &conn, Memc<ElmName> &elm_names, C UID &proj_id)
{
   if(elm_names.elms())
   {
      File f; f.writeMem().putByte(CS_GET_ELM_NAMES);
      File data; data.writeMem().cmpUIntV(elm_names.elms())<<proj_id; FREPA(elm_names)elm_names[i].save(data);
      data.pos(0); if(Compress(data, f, ServerNetworkCompression, ServerNetworkCompressionLevel)){f.pos(0); conn.send(f, -1, false);}
   }
}
void ClientRecvGetElmNames(File &f, Memc<ElmName> &elm_names, UID &proj_id)
{
   File data; if(Decompress(f, data, true)){data.pos(0); elm_names.setNum(data.decUIntV()); data>>proj_id; FREPA(elm_names)elm_names[i].load(data);}
}
/******************************************************************************/
// CS_GET_TEXTURES / CS_SET_TEXTURE
/******************************************************************************/
void ClientSendGetTextures(Connection &conn, Memc<UID> &tex_ids)
{
   if(tex_ids.elms()){File f; f.writeMem().putByte(CS_GET_TEXTURES).cmpUIntV(tex_ids.elms()); FREPA(tex_ids)f<<tex_ids[i]; f.pos(0); conn.send(f, -1, false);}
}
void ServerRecvGetTextures(File &f, Memc<UID> &tex_ids)
{
   tex_ids.setNum(f.decUIntV()); FREPA(tex_ids)f>>tex_ids[i];
}

bool ClientWriteSetTexture(File &f, C UID &tex_id, File &tex_data)
{
   f.putByte(CS_SET_TEXTURE)<<tex_id; return Compress(tex_data, f, ClientNetworkCompression, ClientNetworkCompressionLevel);
}
void ServerRecvSetTexture(File &f, UID &tex_id, File &cmpr_tex_data)
{
   f>>tex_id; f.copy(cmpr_tex_data);
}

void ServerWriteSetTexture(File &f, C UID &tex_id, File &cmpr_tex_data, C UID &proj_id)
{
   f.putByte(CS_SET_TEXTURE); f<<tex_id<<proj_id; cmpr_tex_data.copy(f); 
}
bool ClientRecvSetTexture(File &f, UID &tex_id, File &tex_data, UID &proj_id)
{
   f>>tex_id>>proj_id; return Decompress(f, tex_data);
}
/******************************************************************************/
// CS_GET_ELM_SHORT / CS_SET_ELM_SHORT
/******************************************************************************/
void ClientSendGetElmShort(Connection &conn, Memc<UID> &elm_ids)
{
   if(elm_ids.elms()){File f; f.writeMem().putByte(CS_GET_ELM_SHORT).cmpUIntV(elm_ids.elms()); FREPA(elm_ids)f<<elm_ids[i]; f.pos(0); conn.send(f, -1, false);}
}
void ServerRecvGetElmShort(File &f, Memc<UID> &elm_ids)
{
   elm_ids.setNum(f.decUIntV()); FREPA(elm_ids)f>>elm_ids[i];
}

void ClientSendSetElmShort(Connection &conn, Elm &elm, C Project &proj)
{
   File f; f.writeMem().putByte(CS_SET_ELM_SHORT); elm.compressData(f);
   if(ElmFileInShort(elm.type))
   {
      File data; if(data.readTry(proj.basePath(elm)))data.copy(f);
   }
   f.pos(0); conn.send(f, -1, false);
}
bool ServerRecvSetElmShort(File &f, Elm &elm, File &data)
{
   if(!elm.decompressData(f))return false; return f.copy(data);
}

void ServerWriteSetElmShort(File &f, Elm &elm, C Project &proj)
{
   f.putByte(CS_SET_ELM_SHORT)<<proj.id; elm.compressData(f);
   if(ElmFileInShort(elm.type))
   {
      File data; if(data.readTry(proj.basePath(elm)))data.copy(f);
   }
}
bool ClientRecvSetElmShort(File &f, Elm &elm, File &data, UID &proj_id)
{
   f>>proj_id; if(!elm.decompressData(f))return false; return f.copy(data);
}
/******************************************************************************/
// CS_GET_ELM_LONG / CS_SET_ELM_LONG
/******************************************************************************/
void ClientSendGetElmLong(Connection &conn, Memc<UID> &elm_ids)
{
   if(elm_ids.elms()){File f; f.writeMem().putByte(CS_GET_ELM_LONG).cmpUIntV(elm_ids.elms()); FREPA(elm_ids)f<<elm_ids[i]; f.pos(0); conn.send(f, -1, false);}
}
void ServerRecvGetElmLong(File &f, Memc<UID> &elm_ids)
{
   elm_ids.setNum(f.decUIntV()); FREPA(elm_ids)f>>elm_ids[i];
}

void ClientWriteSetElmLong(File &elm_file, File &data_file, Elm &elm, Project &proj, bool &compress)
{
   elm_file.writeMem().putByte(CS_SET_ELM_LONG); elm.compressData(elm_file);
   if(ElmHasFile(elm.type))
   {
      if(ElmSendBoth(elm.type))
      {
         File data; data.readTry(proj.editPath(elm)); elm_file.cmpUIntV(data.size()); data.copy(elm_file); // if sending both, then store edit in the 'elm_file', because we will keep 'elm_file' uncompressed, and only 'data_file' compressed for 'ElmCompressable'
                 if(data.readTry(proj.gamePath(elm)))data.copy(data_file);
      }else
      {
         File data; if(data.readTry(proj.basePath(elm)))data.copy(data_file);
      }
      compress=ElmCompressable(elm.type); // actual compression will be done in secondary thread
   }
}
bool ServerRecvSetElmLong(File &f, Elm &elm, File &data, File &extra)
{
   if(!elm.decompressData(f))return false;
   if(ElmHasFile(elm.type))
   {
      if(ElmSendBoth(elm.type))
      {
         f.copy(data, f.decUIntV());
         f.copy(extra);
      }else
      {
         f.copy(data);
      }
   }
   return true;
}

void ServerWriteSetElmLong(File &f, Elm &elm, C Project &proj)
{
   f.putByte(CS_SET_ELM_LONG)<<proj.id; elm.compressData(f);
   if(ElmHasFile(elm.type))
   {
      if(ElmSendBoth(elm.type))
      {
         File data; data.readTry(proj.editPath(elm)); f.cmpUIntV(data.size()); data.copy(f);
                 if(data.readTry(proj.gamePath(elm)))data.copy(f);
      }else
      {
         File data; if(data.readTry(proj.basePath(elm)))data.copy(f);
      }
   }
}
bool ClientRecvSetElmLong(File &f, Elm &elm, File &data, File &extra, UID &proj_id)
{
   f>>proj_id; if(!elm.decompressData(f))return false;
   if(ElmHasFile(elm.type))
   {
      if(ElmSendBoth(elm.type))
      {
         f.copy(data, f.decUIntV());
         if(ElmCompressable(elm.type) ? !Decompress(f, extra) : !f.copy(extra))return false;
      }else
      {
         if(ElmCompressable(elm.type) ? !Decompress(f, data) : !f.copy(data))return false;
      }
   }
   return true;
}
/******************************************************************************/
// CS_SET_ELM_FULL
/******************************************************************************/
void ClientWriteSetElmFull(File &elm_file, File &data_file, Elm &elm, Project &proj, bool &compress)
{
   elm_file.writeMem().putByte(CS_SET_ELM_FULL); elm.save(elm_file, true, false);
   if(ElmHasFile(elm.type))
   {
      if(ElmSendBoth(elm.type))
      {
         File data; data.readTry(proj.editPath(elm)); elm_file.cmpUIntV(data.size()); data.copy(elm_file);
                 if(data.readTry(proj.gamePath(elm)))data.copy(data_file);
      }else
      {
         File data; if(data.readTry(proj.basePath(elm)))data.copy(data_file);
      }
      compress=ElmCompressable(elm.type); // actual compression will be done in secondary thread
   }
}
bool ServerRecvSetElmFull(File &f, Elm &elm, File &data, File &extra)
{
   if(!elm.load(f, true, false))return false;
   if(ElmHasFile(elm.type))
   {
      if(ElmSendBoth(elm.type))
      {
         f.copy(data, f.decUIntV());
         f.copy(extra);
      }else
      {
         f.copy(data);
      }
   }
   return true;
}
void ServerWriteSetElmFull(File &f, Elm &elm, C Project &proj)
{
   f.putByte(CS_SET_ELM_FULL)<<proj.id; elm.save(f, true, false);
   if(ElmHasFile(elm.type))
   {
      if(ElmSendBoth(elm.type))
      {
         File data; data.readTry(proj.editPath(elm)); f.cmpUIntV(data.size()); data.copy(f);
                 if(data.readTry(proj.gamePath(elm)))data.copy(f);
      }else
      {
         File data; if(data.readTry(proj.basePath(elm)))data.copy(f);
      }
   }
}
bool ClientRecvSetElmFull(File &f, Elm &elm, File &data, File &extra, UID &proj_id)
{
   f>>proj_id; if(!elm.load(f, true, false))return false;
   if(ElmHasFile(elm.type))
   {
      if(ElmSendBoth(elm.type))
      {
         f.copy(data, f.decUIntV());
         if(ElmCompressable(elm.type) ? !Decompress(f, extra) : !f.copy(extra))return false;
      }else
      {
         if(ElmCompressable(elm.type) ? !Decompress(f, data) : !f.copy(data))return false;
      }
   }
   return true;
}
/******************************************************************************/
// CS_GET_WORLD_VER
/******************************************************************************/
void ClientSendGetWorldVer(Connection &conn, C UID &world_id)
{
   File f; f.writeMem().putByte(CS_GET_WORLD_VER)<<world_id; f.pos(0); conn.send(f, -1, false);
}
void ServerRecvGetWorldVer(File &f, UID &world_id)
{
   f>>world_id;
}
void ServerSendGetWorldVer(Connection &conn, WorldVer *world_ver, C UID &world_id, C UID &proj_id)
{
   File f; f.writeMem().putByte(CS_GET_WORLD_VER)<<world_id<<proj_id; f.putBool(world_ver!=null);
   if(world_ver)
   {
      File temp; world_ver->save(temp.writeMem(), true); temp.pos(0); Compress(temp, f, ServerNetworkCompression, ServerNetworkCompressionLevel);
   }
   f.pos(0); conn.send(f, -1, false);
}
void ClientRecvGetWorldVer(File &f, WorldVer &world_ver, UID &world_id, UID &proj_id)
{
   f>>world_id>>proj_id;
   if(f.getBool())
   {
      File temp; if(Decompress(f, temp, true)){temp.pos(0); world_ver.load(temp, true);}
   }
}
/******************************************************************************/
// CS_GET_WORLD_AREAS
/******************************************************************************/
void ClientSendGetWorldAreas(Connection &conn, C UID &world_id, Memc<AreaSync> &areas)
{
   if(areas.elms())
   {
      File f; f.writeMem().putByte(CS_GET_WORLD_AREAS)<<world_id; f.cmpUIntV(areas.elms()); FREPA(areas)f<<areas[i].xy<<areas[i].flag; f.pos(0); conn.send(f, -1, false);
   }
}
void ServerRecvGetWorldAreas(File &f, UID &world_id, Memc<AreaSync> &areas)
{
   f>>world_id; areas.setNum(f.decUIntV()); FREPA(areas)f>>areas[i].xy>>areas[i].flag;
}
/******************************************************************************/
// CS_SET_WORLD_AREA
/******************************************************************************/
void WriteSetWorldArea(File &f, C UID &world_id, C VecI2 &area_xy, byte area_sync_flag, C AreaVer &ver, C Heightmap *hm, C Memc<ObjData> &objs, C Str &edit_path)
{
   f<<world_id<<area_xy<<area_sync_flag;
   if(area_sync_flag&(AREA_SYNC_HEIGHT|AREA_SYNC_MTRL|AREA_SYNC_COLOR))
   {
      int res=(hm ? hm->resolution() : 0);
      f.cmpUIntV(res);
      if(area_sync_flag&AREA_SYNC_HEIGHT)
      {
         f<<ver.hm_height_time;
         REPD(y, res)
         REPD(x, res)f.putFlt(hm->height(x, y));
      }
      if(area_sync_flag&AREA_SYNC_MTRL)
      {
         f<<ver.hm_mtrl_time;
         f.cmpUIntV(hm ? hm->materials() : 0);
         if(hm)FREP(hm->materials())f<<hm->material(i).id();
         REPD(y, res)
         REPD(x, res){VecB4 m, i; hm->getMaterial(x, y, m, i); f<<m<<i;}
      }
      if(area_sync_flag&AREA_SYNC_COLOR)
      {
         f<<ver.hm_color_time;
         if(res)
         {
            f.putBool(hm->hasColor());
            if(hm->hasColor())
            {
               REPD(y, res)
               REPD(x, res){Color c=hm->color(x, y); f<<c.r<<c.g<<c.b;}
            }
         }
      }
   }
   if(area_sync_flag&AREA_SYNC_REMOVED)f<<ver.hm_removed_time;
   if(area_sync_flag&AREA_SYNC_OBJ    )
   {
      f<<ver.obj_ver;
      objs.save(f, edit_path);
   }
}
bool RecvSetWorldArea(File &f, UID &world_id, VecI2 &area_xy, byte &area_sync_flag, AreaVer &ver, Heightmap &hm, Memc<ObjData> &objs, C Str &game_path, C Str &edit_path)
{
   f>>world_id>>area_xy>>area_sync_flag;
   if(area_sync_flag&(AREA_SYNC_HEIGHT|AREA_SYNC_MTRL|AREA_SYNC_COLOR))
   {
      int res=f.decUIntV(); hm.create(res, 0, null, false, null, null, null, null, null, null, null, null);
      if(area_sync_flag&AREA_SYNC_HEIGHT)
      {
         f>>ver.hm_height_time;
         REPD(y, res)
         REPD(x, res)hm.height(x, y, f.getFlt());
      }
      if(area_sync_flag&AREA_SYNC_MTRL)
      {
         f>>ver.hm_mtrl_time;
         Mems<MaterialPtr> mtrls; mtrls.setNum(f.decUIntV());
         FREPA(mtrls){UID mtrl_id; f>>mtrl_id; if(mtrl_id.valid())mtrls[i]=game_path+EncodeFileName(mtrl_id);}
         REPD(y, res)
         REPD(x, res)
         {
            VecB4 m, i; f>>m>>i;
            hm.setMaterial(x, y, InRange(m.x, mtrls) ? mtrls[m.x] : MaterialPtr(), InRange(m.y, mtrls) ? mtrls[m.y] : MaterialPtr(), InRange(m.z, mtrls) ? mtrls[m.z] : MaterialPtr(), InRange(m.w, mtrls) ? mtrls[m.w] : MaterialPtr(), i);
         }
      }
      if(area_sync_flag&AREA_SYNC_COLOR)
      {
         f>>ver.hm_color_time;
         if(res)
            if(f.getBool())
               REPD(y, res)
               REPD(x, res){Color c; f>>c.r>>c.g>>c.b; c.a=255; hm.color(x, y, c);}
      }
   }
   if(area_sync_flag&AREA_SYNC_REMOVED)f>>ver.hm_removed_time;
   if(area_sync_flag&AREA_SYNC_OBJ    )
   {
      f>>ver.obj_ver;
      if(!objs.load(f, edit_path))return false;
   }
   return true;
}

void ClientWriteSetWorldArea(File &elm, File &data, C UID &world_id, C VecI2 &area_xy, byte area_sync_flag, C AreaVer &ver, C Heightmap *hm)
{
   elm.putByte(CS_SET_WORLD_AREA);
   WriteSetWorldArea(data, world_id, area_xy, area_sync_flag, ver, hm, Memc<ObjData>(), S);
}
bool ServerRecvSetWorldArea(File &f, UID &world_id, VecI2 &area_xy, byte &area_sync_flag, AreaVer &ver, Heightmap &hm, C Str &game_path, C Str &edit_path)
{
   File data; if(Decompress(f, data, true))
   {
      Memc<ObjData> objs;
      data.pos(0); return RecvSetWorldArea(data, world_id, area_xy, area_sync_flag, ver, hm, objs, game_path, edit_path);
   }
   return false;
}

void ServerSendSetWorldArea(Connection &conn, C UID &world_id, C VecI2 &area_xy, byte area_sync_flag, C AreaVer &ver, C Heightmap *hm, Memc<ObjData> &objs, C UID &proj_id, C Str &edit_path)
{
   File f; f.writeMem().putByte(CS_SET_WORLD_AREA);
   File data; data.writeMem()<<proj_id; WriteSetWorldArea(data, world_id, area_xy, area_sync_flag, ver, hm, objs, edit_path);
   data.pos(0); if(Compress(data, f, ServerNetworkCompression, ServerNetworkCompressionLevel)){f.pos(0); conn.send(f, -1, false);}
}
bool ClientRecvSetWorldArea(File &f, UID &world_id, VecI2 &area_xy, byte &area_sync_flag, AreaVer &ver, Heightmap &hm, Memc<ObjData> &objs, UID &proj_id, C Str &game_path, C Str &edit_path)
{
   File data; if(Decompress(f, data, true))
   {
      data.pos(0); data>>proj_id; return RecvSetWorldArea(data, world_id, area_xy, area_sync_flag, ver, hm, objs, game_path, edit_path);
   }
   return false;
}
/******************************************************************************/
// CS_SET_WORLD_OBJS
/******************************************************************************/
void ClientWriteSetWorldObjs(File &elm, File &data, C UID &world_id, C VecI2 &area_xy, Memc<ObjData> &objs, C Str &edit_path)
{
   elm.putByte(CS_SET_WORLD_OBJS);
   data.cmpIntV(area_xy.x).cmpIntV(area_xy.y)<<world_id;
   objs.save(data, edit_path);
}
bool ServerRecvSetWorldObjs(File &f, UID &world_id, VecI2 &area_xy, Memc<ObjData> &objs, C Str &edit_path)
{
   File data; if(Decompress(f, data, true))
   {
      data.pos(0); data.decIntV(area_xy.x).decIntV(area_xy.y)>>world_id;
      return objs.load(data, edit_path);
   }
   return false;
}

void ServerWriteSetWorldObjs(File &f, C UID &world_id, C VecI2 &area_xy, Memc<ObjData> &objs, C Str &edit_path, C UID &proj_id)
{
   f.putByte(CS_SET_WORLD_OBJS).cmpIntV(area_xy.x).cmpIntV(area_xy.y)<<world_id<<proj_id;
   objs.save(f, edit_path);
}
bool ClientRecvSetWorldObjs(File &f, UID &world_id, VecI2 &area_xy, Memc<ObjData> &objs, C Str &edit_path, UID &proj_id)
{
   f.decIntV(area_xy.x).decIntV(area_xy.y)>>world_id>>proj_id;
   return objs.load(f, edit_path);
}
/******************************************************************************/
// CS_GET_WORLD_WAYPOINTS
/******************************************************************************/
void ClientSendGetWorldWaypoints(Connection &conn, C UID &world_id, Memc<UID> &waypoints)
{
   if(waypoints.elms()){File f; f.writeMem().putByte(CS_GET_WORLD_WAYPOINTS)<<world_id; f.cmpUIntV(waypoints.elms()); FREPA(waypoints)f<<waypoints[i]; f.pos(0); conn.send(f, -1, false);}
}
void ServerRecvGetWorldWaypoints(File &f, UID &world_id, Memc<UID> &waypoints)
{
   f>>world_id; waypoints.setNum(f.decUIntV()); FREPA(waypoints)f>>waypoints[i];
}
/******************************************************************************/
// CS_SET_WORLD_WAYPOINT
/******************************************************************************/
void ClientSendSetWorldWaypoint(Connection &conn, C UID &world_id, C UID &waypoint_id, C Version &waypoint_ver, EditWaypoint &waypoint)
{
   File f; f.writeMem().putByte(CS_SET_WORLD_WAYPOINT)<<world_id<<waypoint_id<<waypoint_ver; waypoint.save(f); f.pos(0); conn.send(f, -1, false);
}
bool ServerRecvSetWorldWaypoint(File &f, UID &world_id, UID &waypoint_id, Version &waypoint_ver, EditWaypoint &waypoint)
{
   f>>world_id>>waypoint_id>>waypoint_ver; return waypoint.load(f);
}

void ServerWriteSetWorldWaypoint(File &f, C UID &world_id, C UID &waypoint_id, C Version &waypoint_ver, EditWaypoint &waypoint, C UID &proj_id)
{
   f.putByte(CS_SET_WORLD_WAYPOINT)<<world_id<<waypoint_id<<waypoint_ver<<proj_id; waypoint.save(f);
}
bool ClientRecvSetWorldWaypoint(File &f, UID &world_id, UID &waypoint_id, Version &waypoint_ver, EditWaypoint &waypoint, UID &proj_id)
{
   f>>world_id>>waypoint_id>>waypoint_ver>>proj_id; return waypoint.load(f);
}
/******************************************************************************/
// CS_GET_WORLD_LAKES
/******************************************************************************/
void ClientSendGetWorldLakes(Connection &conn, C UID &world_id, Memc<UID> &lakes)
{
   if(lakes.elms()){File f; f.writeMem().putByte(CS_GET_WORLD_LAKES)<<world_id; f.cmpUIntV(lakes.elms()); FREPA(lakes)f<<lakes[i]; f.pos(0); conn.send(f, -1, false);}
}
void ServerRecvGetWorldLakes(File &f, UID &world_id, Memc<UID> &lakes)
{
   f>>world_id; lakes.setNum(f.decUIntV()); FREPA(lakes)f>>lakes[i];
}
/******************************************************************************/
// CS_SET_WORLD_LAKE
/******************************************************************************/
void ClientSendSetWorldLake(Connection &conn, C UID &world_id, C UID &lake_id, C Version &lake_ver, Lake &lake)
{
   File f; f.writeMem().putByte(CS_SET_WORLD_LAKE)<<world_id<<lake_id<<lake_ver; lake.save(f); f.pos(0); conn.send(f, -1, false);
}
bool ServerRecvSetWorldLake(File &f, UID &world_id, UID &lake_id, Version &lake_ver, Lake &lake)
{
   f>>world_id>>lake_id>>lake_ver; return lake.load(f);
}

void ServerWriteSetWorldLake(File &f, C UID &world_id, C UID &lake_id, C Version &lake_ver, Lake &lake, C UID &proj_id)
{
   f.putByte(CS_SET_WORLD_LAKE)<<world_id<<lake_id<<lake_ver<<proj_id; lake.save(f);
}
bool ClientRecvSetWorldLake(File &f, UID &world_id, UID &lake_id, Version &lake_ver, Lake &lake, UID &proj_id)
{
   f>>world_id>>lake_id>>lake_ver>>proj_id; return lake.load(f);
}
/******************************************************************************/
// CS_GET_WORLD_RIVERS
/******************************************************************************/
void ClientSendGetWorldRivers(Connection &conn, C UID &world_id, Memc<UID> &rivers)
{
   if(rivers.elms()){File f; f.writeMem().putByte(CS_GET_WORLD_RIVERS)<<world_id; f.cmpUIntV(rivers.elms()); FREPA(rivers)f<<rivers[i]; f.pos(0); conn.send(f, -1, false);}
}
void ServerRecvGetWorldRivers(File &f, UID &world_id, Memc<UID> &rivers)
{
   f>>world_id; rivers.setNum(f.decUIntV()); FREPA(rivers)f>>rivers[i];
}
/******************************************************************************/
// CS_SET_WORLD_RIVER
/******************************************************************************/
void ClientSendSetWorldRiver(Connection &conn, C UID &world_id, C UID &river_id, C Version &river_ver, River &river)
{
   File f; f.writeMem().putByte(CS_SET_WORLD_RIVER)<<world_id<<river_id<<river_ver; river.save(f); f.pos(0); conn.send(f, -1, false);
}
bool ServerRecvSetWorldRiver(File &f, UID &world_id, UID &river_id, Version &river_ver, River &river)
{
   f>>world_id>>river_id>>river_ver; return river.load(f);
}

void ServerWriteSetWorldRiver(File &f, C UID &world_id, C UID &river_id, C Version &river_ver, River &river, C UID &proj_id)
{
   f.putByte(CS_SET_WORLD_RIVER)<<world_id<<river_id<<river_ver<<proj_id; river.save(f);
}
bool ClientRecvSetWorldRiver(File &f, UID &world_id, UID &river_id, Version &river_ver, River &river, UID &proj_id)
{
   f>>world_id>>river_id>>river_ver>>proj_id; return river.load(f);
}
/******************************************************************************/
// CS_GET_MINI_MAP_VER
/******************************************************************************/
void ClientSendGetMiniMapVer(Connection &conn, C UID &mini_map_id)
{
   File f; f.writeMem().putByte(CS_GET_MINI_MAP_VER)<<mini_map_id; f.pos(0); conn.send(f, -1, false);
}
void ServerRecvGetMiniMapVer(File &f, UID &mini_map_id)
{
   f>>mini_map_id;
}
void ServerSendGetMiniMapVer(Connection &conn, MiniMapVer *mini_map_ver, C UID &mini_map_id, C UID &proj_id)
{
   File f; f.writeMem().putByte(CS_GET_MINI_MAP_VER)<<mini_map_id<<proj_id; f.putBool(mini_map_ver!=null);
   if(mini_map_ver)
   {
      File temp; mini_map_ver->save(temp.writeMem(), true); temp.pos(0); Compress(temp, f, ServerNetworkCompression, ServerNetworkCompressionLevel);
   }
   f.pos(0); conn.send(f, -1, false);
}
void ClientRecvGetMiniMapVer(File &f, MiniMapVer &mini_map_ver, UID &mini_map_id, UID &proj_id)
{
   f>>mini_map_id>>proj_id;
   if(f.getBool())
   {
      File temp; if(Decompress(f, temp, true)){temp.pos(0); mini_map_ver.load(temp, true);}
   }
}
/******************************************************************************/
// CS_GET_MINI_MAP_IMAGES
/******************************************************************************/
void ClientSendGetMiniMapImages(Connection &conn, C UID &mini_map_id, Memc<VecI2> &images)
{
   File f; f.writeMem().putByte(CS_GET_MINI_MAP_IMAGES)<<mini_map_id;
   f.cmpUIntV(images.elms()); FREPA(images)f.cmpIntV(images[i].x).cmpIntV(images[i].y); f.pos(0); conn.send(f, -1, false);
}
void ServerRecvGetMiniMapImages(File &f, UID &mini_map_id, Memc<VecI2> &images)
{
   f>>mini_map_id; images.setNum(f.decUIntV()); FREPA(images)f.decIntV(images[i].x).decIntV(images[i].y);
}
/******************************************************************************/
// CS_SET_MINI_MAP_SETTINGS
/******************************************************************************/
void ClientSendSetMiniMapSettings(Connection &conn, C UID &mini_map_id, C Game::MiniMap::Settings &settings, C TimeStamp &settings_time)
{
   File f; f.writeMem().putByte(CS_SET_MINI_MAP_SETTINGS)<<mini_map_id<<settings_time; settings.save(f); f.pos(0); conn.send(f, -1, false);
}
bool ServerRecvSetMiniMapSettings(File &f, UID &mini_map_id, Game::MiniMap::Settings &settings, TimeStamp &settings_time)
{
   f>>mini_map_id>>settings_time; return settings.load(f);
}
void ServerWriteSetMiniMapSettings(File &f, C UID &mini_map_id, C Game::MiniMap::Settings &settings, C TimeStamp &settings_time, C UID &proj_id)
{
   f.putByte(CS_SET_MINI_MAP_SETTINGS)<<mini_map_id<<proj_id<<settings_time; settings.save(f);
}
bool ClientRecvSetMiniMapSettings(File &f, UID &mini_map_id, Game::MiniMap::Settings &settings, TimeStamp &settings_time, UID &proj_id)
{
   f>>mini_map_id>>proj_id>>settings_time; return settings.load(f);
}
/******************************************************************************/
// CS_SET_MINI_MAP_IMAGE
/******************************************************************************/
void ServerSendSetMiniMapImage(Connection &conn, C UID &mini_map_id, C VecI2 &image_xy, C TimeStamp &image_time, File &cmpr_image_data, C UID &proj_id)
{
   File f; f.writeMem().putByte(CS_SET_MINI_MAP_IMAGE)<<mini_map_id<<proj_id<<image_xy<<image_time; cmpr_image_data.copy(f);
   f.pos(0); conn.send(f, -1, false);
}
bool ClientRecvSetMiniMapImage(File &f, UID &mini_map_id, VecI2 &image_xy, TimeStamp &image_time, File &image_data, UID &proj_id)
{
   f>>mini_map_id>>proj_id>>image_xy>>image_time; Decompress(f, image_data); return true; // 'f' can be empty if it doesn't exist, so don't return error on decompress fail
}
void ClientWriteSetMiniMapImage(File &elm, File &data, C UID &mini_map_id, C VecI2 &image_xy, C TimeStamp &image_time, File &image_data)
{
   elm.putByte(CS_SET_MINI_MAP_IMAGE)<<mini_map_id<<image_xy<<image_time; elm.putBool(image_data.is());
   image_data.copy(data); // 'data' will be compressed
}
bool ServerRecvSetMiniMapImage(File &f, UID &mini_map_id, VecI2 &image_xy, TimeStamp &image_time, bool &image_is, File &cmpr_image_data)
{
   f>>mini_map_id>>image_xy>>image_time>>image_is; return f.copy(cmpr_image_data); // don't decompress image data on the server
}
/******************************************************************************/
// CS_GET_CODE_VER
/******************************************************************************/
void ClientSendGetCodeVer(Connection &conn, Memc<ElmTypeVer> &elms)
{
   elms.sort(ElmTypeVer::Compare); // sort on the client so server can access using binary search
   File f; f.writeMem().putByte(CS_GET_CODE_VER); elms.save(f); f.pos(0); conn.send(f, -1, false);
}
bool ServerRecvGetCodeVer(File &f, Memc<ElmTypeVer> &elms)
{
   return elms.load(f);
}
/******************************************************************************/
// CS_SET_CODE_DATA
/******************************************************************************/
void ServerSendSetCodeData(Connection &conn, C Memc<ElmCodeData> &elms, C UID &proj_id) // use compression because source code can be big
{
   File data; data.writeMem()<<proj_id; elms.save(data); data.pos(0);
   File f; f.writeMem().putByte(CS_SET_CODE_DATA); Compress(data, f, ServerNetworkCompression, ServerNetworkCompressionLevel); f.pos(0); conn.send(f, -1, false);
}
bool ClientRecvSetCodeData(File &f, Memc<ElmCodeData> &elms, UID &proj_id)
{
   File data; if(Decompress(f, data, true)){data.pos(0); data>>proj_id; return elms.load(data);}
   return false;
}
void ClientSendSetCodeData(Connection &conn, C Memc<ElmCodeData> &elms) // use compression because source code can be big
{
   File data; data.writeMem(); elms.save(data); data.pos(0);
   File f; f.writeMem().putByte(CS_SET_CODE_DATA); Compress(data, f, ServerNetworkCompression, 9); f.pos(0); conn.send(f, -1, false);
}
bool ServerRecvSetCodeData(File &f, Memc<ElmCodeData> &elms)
{
   File data; if(Decompress(f, data, true)){data.pos(0); return elms.load(data);}
   return false;
}
/******************************************************************************/
// CS_CODE_SYNC_STATUS
/******************************************************************************/
void ServerSendCodeSyncStatus(Connection &conn, C Memc<ElmCodeBase> &elms, bool resync, C UID &proj_id) // use compression because source code can be big
{
   File data; data.writeMem()<<resync<<proj_id; elms.save(data); data.pos(0);
   File f; f.writeMem().putByte(CS_CODE_SYNC_STATUS); Compress(data, f, ServerNetworkCompression, ServerNetworkCompressionLevel); f.pos(0); conn.send(f, -1, false);
}
bool ClientRecvCodeSyncStatus(File &f, Memc<ElmCodeBase> &elms, bool &resync, UID &proj_id)
{
   File data; if(Decompress(f, data, true)){data.pos(0); data>>resync>>proj_id; return elms.load(data);}
   return false;
}
/******************************************************************************/

/******************************************************************************/
   void ElmName::set(Elm  &elm) {id=elm.id; time=elm.name_time; name=elm.name;}
   bool ElmName::save(File &f  ) {f<<id<<time<<name; return f.ok();}
   bool ElmName::load(File &f  ) {f>>id>>time>>name; return f.ok();}
   void AreaSync::set(uint flag, C VecI2 &xy) {T.flag=flag; T.xy=xy;}
   void ElmTypeVer::set(C UID &id, ELM_TYPE type, C Version &ver) {T.id=id; T.type=type; T.ver=ver;}
   void ElmTypeVer::set(C Elm &elm                              ) {set(elm.id, elm.type, elm.data ? elm.data->ver : Version());}
   bool ElmTypeVer::save(File &f)C {f<<id<<type<<ver; return f.ok();}
   bool ElmTypeVer::load(File &f)  {f>>id>>type>>ver; return f.ok();}
   int ElmTypeVer::Compare(C ElmTypeVer &a, C ElmTypeVer &b ) {return ::Compare(a.id, b.id);}
   int ElmTypeVer::Compare(C ElmTypeVer &a, C UID        &id) {return ::Compare(a.id,   id);}
   void ElmCodeData::set(Elm &elm, Code *code)
   {
      ::ElmTypeVer::set(elm);
      switch(type)
      {
         case ELM_APP : if(ElmApp *elm_app=elm.appData())app=*elm_app; break;
         case ELM_CODE: if(code)T.code=*code; break;
      }
   }
   bool ElmCodeData::save(File &f)C
   {
      ::ElmTypeVer::save(f);
      if(type==ELM_CODE)code.save(f);
      if(type==ELM_APP )app .save(f);
      return f.ok();
   }
   bool ElmCodeData::load(File &f)
   {
      if(::ElmTypeVer::load(f))
      {
         if(type==ELM_CODE)if(!code.load(f))return false;
         if(type==ELM_APP )if(!app .load(f))return false;
         if(f.ok())return true;
      }
      return false;
   }
   void ElmCodeBase::set(C UID &id, C Str &data) {T.id=id; T.data=data;}
   bool ElmCodeBase::save(File &f)C {f<<id<<data; return f.ok();}
   bool ElmCodeBase::load(File &f)  {f>>id>>data; return f.ok();}
/******************************************************************************/
