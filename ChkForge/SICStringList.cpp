#include "SICStringList.h"

#include <MappingCoreLib/MapFile.h>
#include <memory>
#include <sstream>
#include <cctype>


void SCMDStringList::setTrackingMap(std::shared_ptr<MapFile> map) {
	current_map_file = map;
	current_map_string_backup = {};
	string_keeper.clear();
}


int SCMDStringList::FindString_RawIndex(const char* str) {
  RawString target = RawString(str);
  return current_map_file->findString(target);
}


const char* SCMDStringList::GetString(int strid) {
	if (string_keeper.count(strid) != 0) {
		return string_keeper.at(strid).str;
	}

	auto str = current_map_file->getString<RawString>(strid);
	if (str) {
		return string_keeper.emplace(strid, Chk::ScStr{str->c_str()}).first->second.str;
	}
	return "";
}


static int ParseXDigit(int ch) {
	if ('0' <= ch && ch <= '9') return ch - '0';
	else if ('a' <= ch && ch <= 'f') return ch - 'a' + 10;
	else return ch - 'A' + 10;
}


std::string ConvertString_SCMD2ToRaw(const char* scmd2text) {
	std::ostringstream result;

	for (const char* p = scmd2text; *p != '\0'; p++) {
		if (strncmp(p, "\\<", 2) == 0) {
			result << '<';
			p += 1;
		}

		else if (strncmp(p, "\\>", 2) == 0) {
			result << '>';
			p += 1;
		}

		else if (strncmp(p, "<R>", 3) == 0) {
			result << '\x12';
			p += 2;
		}

		else if (strncmp(p, "<C>", 3) == 0) {
			result << '\x13';
			p += 2;
		}

		else if (p[0] == '<' && isxdigit(p[1]) && p[2] == '>') {
			result << static_cast<char>(ParseXDigit(p[1]));
			p += 2;
		}

		else if (p[0] == '<' && isxdigit(p[1]) && isxdigit(p[2]) && p[3] == '>') {
			result << static_cast<char>(ParseXDigit(p[1]) << 4 | ParseXDigit(p[2]));
			p += 3;
		}

		else if (p[0] == '\r') continue; //ignore
		else result << *p;
	}

	return result.str();
}


int SCMDStringList::AddSCMD2String(const char* scmd2text, int SectionName, char AlwaysCreate) {
	std::string newString = ConvertString_SCMD2ToRaw(scmd2text);
	return current_map_file->addString(RawString(newString));
}


int SCMDStringList::Dereference(__int16 stringindex, int SectionName, int unknown_arg) {
	return 0;
}


int SCMDStringList::DerefAndAddString(const char* Text, int oldStringIndex, int SectionName) {
	Dereference(oldStringIndex, SectionName, 0);
	return AddSCMD2String(Text, SectionName, 1);
}


char SCMDStringList::SetSCMD2Text(const char* scmd2text, int stringID) {
	std::string newString = ConvertString_SCMD2ToRaw(scmd2text);
	string_keeper.erase(stringID);
	current_map_file->replaceString(stringID, RawString(newString));
	return 1;
}


int SCMDStringList::GetTotalStringNum() {
	return current_map_file->getCapacity();
}


char SCMDStringList::BackupStrings() {
	current_map_string_backup = current_map_file->strings;
	return 1;
}


char SCMDStringList::RestoreBackup() {
	current_map_file->strings = current_map_string_backup;
	string_keeper.clear();
	return 1;
}


char SCMDStringList::ClearBackup() {
	current_map_string_backup = {};
	return 1;
}


