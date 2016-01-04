#ifndef __UTIL_H__
#define __UTIL_H__

#include "common.h"
#include <pugixml.hpp>


void trim(std::string& s);

/*******************************
 ** Dom Related
 *******************************/

pugi::xml_node get_previous_ast_element(pugi::xml_node node);
pugi::xml_node get_parent_ast_element(pugi::xml_node node);
pugi::xml_node get_function_call(pugi::xml_node node);

std::string get_text_content(pugi::xml_node node);
std::string get_text_content_except_tag(pugi::xml_node, std::string name);

pugi::xml_node lub(pugi::xml_node n1, pugi::xml_node n2);

/*******************************
 ** File utils
 *******************************/

std::vector<std::string> get_files(const std::string &folder);
void get_files(const std::string &folder, std::vector<std::string>& vs);
void get_files_by_extension(const std::string& folder, std::vector<std::string>& vs, const std::string& s);
void get_files_by_extension(const std::string& folder, std::vector<std::string>& vs, const std::vector<std::string>& extension);


/*******************************
 * read/write
 *******************************/
void write(const std::string& file, const std::string& content);
void append(const std::string& file, const std::string& content);
std::string read(const std::string& file);
// folders
void remove_folder(const std::string& folder);
void create_folder(const std::string& folder);

/*******************************
 ** Srcml Utils
 *******************************/

void file2xml(const std::string& filename, pugi::xml_document& doc);
void string2xml(const std::string& code, pugi::xml_document& doc);

/*******************************
 ** string
 *******************************/

bool ends_with(const std::string &s, const std::string &pattern);
bool starts_with(const std::string &s, const std::string &pattern);
std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

/*
 * This is the split function that use any white space as delimiter
 * Use completely different implementation from the other two
 */
std::vector<std::string> split(const std::string &s);
void remove(std::string& s, const std::string& pattern);

/*******************************
 ** thread
 *******************************/
std::string exec(const char* cmd, int *status, int timeout=0);
// with input
std::string exec(const char* cmd, const char* input, int *status, unsigned int timeout=0);

#endif
