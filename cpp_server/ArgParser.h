#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <list>
#include <stdexcept>

class ArgParser {
public:
  struct parse_result {
	std::string program_directory;
	std::vector<std::string> non_option_arguments;
	std::vector<std::string> warnings;
  };
  class parse_error : public std::runtime_error {
  private:
	friend ArgParser;
	parse_error(const std::string& flag) : std::runtime_error("error parsing flag " + flag) {}
	parse_error(const std::string& flag, const std::string& message) : std::runtime_error("error parsing flag " + flag + ": " + message) {}
  };
private:
  struct flag_target {
	bool* ref;
	int priority;
  };
  std::unordered_map<std::string, flag_target> flags;
  std::unordered_map<std::string, flag_target> flagDisablers;
  std::unordered_map<std::string, std::string*> string_options;
  std::unordered_map<std::string, int*> int_options;
  std::unordered_map<std::string, std::vector<std::string>*> string_vector_options;
  std::unordered_map<std::string, std::vector<int>*> int_vector_options;
  bool hasFlag(const std::string& flag) const;
  bool hasOption(const std::string& flag) const;
  bool hasFlag(char flag) const { return hasFlag({ '-', flag }); }
  bool hasOption(char flag) const { return hasOption({ '-', flag }); }
  bool readFlag(const std::string& flag, std::map<bool*, int>& prevPriorities) const;
  bool readOption(const std::string& flag, const std::string& option) const;
  bool readFlag(char flag, std::map<bool*, int>& prevPriorities) const { return readFlag({ '-', flag }, prevPriorities); }
  bool readOption(char flag, const std::string& option) const { return readOption({ '-', flag }, option); }

public:
  ArgParser() {}
  ArgParser& assign(const std::string& flag, bool& ref, int priority = 0);
  ArgParser& assignFlagDisabler(const std::string& flag, bool& ref, int priority = 0);
  ArgParser& assign(const std::string& flag, std::string& ref);
  ArgParser& assign(const std::string& flag, int& ref);
  ArgParser& assign(const std::string& flag, std::vector<std::string>& ref);
  ArgParser& assign(const std::string& flag, std::vector<int>& ref);
  //TODO:
  template<class T, size_t F>
  ArgParser& assignObjectBuilder(const std::string(&flags)[F], std::vector<T>& object);
  //TODO:
  template<class T, typename MemberT>
  ArgParser& assignToObject(const std::string& flag, MemberT T::*member);
  parse_result parse(int argc, char** argv) const;
};
