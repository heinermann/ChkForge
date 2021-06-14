#pragma once

#include <MappingCoreLib/MapFile.h>
#include <memory>

class SCMDStringList {
public:
  virtual void blah_00() {};
  virtual void blah_04() {};
  virtual void blah_08() {};
  virtual int FindString_RawIndex(const char*);

  // Retrives stringtable[stringID].
  // stringID == 0 : return blank string
  // No valid string at given stringID : return NULL
  // Else : return SCMD2String
  //
  // Returned memory is managed by SCMDraft2, and shouldn't be freed manually.
  virtual const char* GetString(int strid);
  virtual void blah_14() {};
  virtual int AddSCMD2String(const char* scmd2text, int SectionName, char AlwaysCreate);

  // Delete 1 reference from given section to string. String with no reference are deleted automatically.
  // a3 is unknown or unused. Set it to 0
  virtual int Dereference(__int16 stringindex, int SectionName, int unkown_arg);

  // Dereference & Add in one function.
  virtual int DerefAndAddString(const char* Text, int oldStringIndex, int SectionName);
  virtual char SetSCMD2Text(const char* scmd2text, int stringID);
  virtual void blah_28() {};

  // Get reserved space for string table.
  // Returned size = (used stringtable slot) + (unused stringtable slot num)
  // Stringtable slot 0(empty string) is assumed as being used.
  virtual int GetTotalStringNum();
  virtual void blah_30() {};
  virtual void blah_34() {};
  virtual void blah_38() {};

  // Backup string table temporarilly. Mainly used for trigger editor.
  // If anything goes wrong, you can call RestoreBackup to revert changes made to string table
  // If all goes right, call ClearBackup.
  virtual char BackupStrings();

  // Counterpart of StringTable_BackupStrings 
  virtual char RestoreBackup();

  // Counterpart of StringTable_BackupStrings
  virtual char ClearBackup();

  void setTrackingMap(std::shared_ptr<MapFile> map);

private:
  std::shared_ptr<MapFile> current_map_file = nullptr;
  Strings::StringBackup current_map_string_backup = { nullptr };
};
