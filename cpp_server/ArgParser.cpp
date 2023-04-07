#include "ArgParser.h"

using namespace std;

bool ArgParser::hasFlag(const string& flag) const {
  return flags.find(flag) != flags.end() || flagDisablers.find(flag) != flagDisablers.end();
}

bool ArgParser::hasOption(const string& flag) const {
  return string_options.find(flag) != string_options.end() || int_options.find(flag) != int_options.end() ||
	string_vector_options.find(flag) != string_vector_options.end() || int_vector_options.find(flag) != int_vector_options.end();
}


bool ArgParser::readFlag(const string& flag, map<bool*, int>& prevPriorities) const
{
  {
	auto flag_itr = flags.find(flag);
	if (flag_itr != flags.end()) {
	  bool* ref = flag_itr->second.ref;
	  auto priority = flag_itr->second.priority;
	  auto priority_itr = prevPriorities.find(ref);
	  if (priority_itr == prevPriorities.end() || priority_itr->second <= priority) {
		*ref = true;
		prevPriorities[ref] = priority;
	  }
	  return true;
	}
  }
  {
	auto flag_itr = flagDisablers.find(flag);
	if (flag_itr != flagDisablers.end()) {
	  bool* ref = flag_itr->second.ref;
	  auto priority = flag_itr->second.priority;
	  auto priority_itr = prevPriorities.find(ref);
	  if (priority_itr == prevPriorities.end() || priority_itr->second <= priority) {
		*ref = false;
		prevPriorities[ref] = priority;
	  }
	  return true;
	}
  }
  return false;
}
bool ArgParser::readOption(const string& flag, const string& option) const
{
  {
	auto itr = string_options.find(flag);
	if (itr != string_options.end()) {
	  auto& ref = *itr->second;
	  ref = option;
	  return true;
	}
  }
  {
	auto itr = string_vector_options.find(flag);
	if (itr != string_vector_options.end()) {
	  auto& ref = *itr->second;
	  ref.push_back(option);
	  return true;
	}
  }
  try {
	{
	  auto itr = int_options.find(flag);
	  if (itr != int_options.end()) {
		auto& ref = *itr->second;
		ref = stoi(option);
		return true;
	  }
	}
	{
	  auto itr = int_vector_options.find(flag);
	  if (itr != int_vector_options.end()) {
		auto& ref = *itr->second;
		ref.push_back(stoi(option));
		return true;
	  }
	}
  }
  catch (invalid_argument e) {}
  catch (range_error e) {}
  return false;
}
#undef has

ArgParser& ArgParser::assign(const string& flag, bool& ref, int priority) {
  if (hasFlag(flag)) {
	throw runtime_error("Duplicated flag: " + flag);
  }
  flags[flag] = { &ref, priority };
  return *this;
}

ArgParser& ArgParser::assignFlagDisabler(const string& flag, bool& ref, int priority) {
  if (hasFlag(flag)) {
	throw runtime_error("Duplicated flag: " + flag);
  }
  flagDisablers[flag] = { &ref, priority };
  return *this;
}

ArgParser& ArgParser::assign(const string& flag, string& ref) {
  if (hasOption(flag)) {
	throw runtime_error("Duplicated option: " + flag);
  }
  string_options[flag] = &ref;
  return *this;
}

ArgParser& ArgParser::assign(const string& flag, int& ref) {
  if (hasOption(flag)) {
	throw runtime_error("Duplicated option: " + flag);
  }
  int_options[flag] = &ref;
  return *this;
}

ArgParser& ArgParser::assign(const string& flag, vector<int>& ref) {
  if (hasOption(flag)) {
	throw runtime_error("Duplicated option: " + flag);
  }
  int_vector_options[flag] = &ref;
  return *this;
}

ArgParser& ArgParser::assign(const string& flag, vector<string>& ref) {
  if (hasOption(flag)) {
	throw runtime_error("Duplicated option: " + flag);
  }
  string_vector_options[flag] = &ref;
  return *this;
}

//TODO: add isalnum checks
ArgParser::parse_result ArgParser::parse(int argc, char** argv) const {
  string program_directory;
  vector<string> non_option_arguments;
  vector<string> warnings;
  if (argc == 0) {
	throw runtime_error("argc == 0");
  }
  program_directory = argv[0];
#ifdef _WIN32
  string::size_type separator_index = program_directory.rfind('\\');
#else
  string::size_type separator_index = program_directory.rfind('/');
#endif // _WIN32
  if (separator_index != string::npos) {
	program_directory.erase(separator_index + 1);
  }
  map<bool*, int> prevPriorities;
  for (int i = 1; i != argc; i++)
  {
	string argument = argv[i];
	if (argument.empty()) {
	  continue;
	}
	if (argument.length() == 1 || argument[0] != '-') {
	  non_option_arguments.push_back(argument);
	  continue;
	}
	if (argument[1] == '-') {
	  if (argument.length() == 2) {
		while (++i != argc) {
		  non_option_arguments.push_back(argument);
		}
		break;
	  }
	  string::size_type equalSignIndex = argument.find_first_of('=', 2);
	  string flag = equalSignIndex == string::npos ? argument : argument.substr(0, equalSignIndex);
	  bool option_found = false;
	  if (i + 1 != argc || equalSignIndex != string::npos) {
		string option = equalSignIndex == string::npos ? argv[++i] : argument.substr(equalSignIndex + 1);
		option_found = readOption(flag, option);
	  }
	  bool flag_found = readFlag(flag, prevPriorities);
	  if (!option_found) {
		i--;
	  }
	  if (!(flag_found || option_found)) {
		throw parse_error(flag);
	  }
	  continue;
	}
	// if ( argument[1] != '-' )
	for (string::size_type argCharI = 1; argCharI != argument.length(); argCharI++) {
	  char flag = argument[argCharI];
	  bool flag_found = readFlag(flag, prevPriorities);
	  if (flag_found && argCharI + 1 != argument.length() && hasFlag(argument[argCharI + 1])) {
		continue;
	  }
	  bool option_found = false;
	  if (argCharI + 1 != argument.length()) {
		option_found = readOption(flag, argument.substr(argCharI + 1));
	  } else if (i + 1 != argc) {
		option_found = readOption(flag, argv[i + 1]);
	  }
	  if (option_found) {
		break;
	  }
	  if (!flag_found) {
		throw parse_error({'-', flag}, argument);
	  }
	}
  }
  return {
	program_directory,
	non_option_arguments,
	warnings,
  };
}
