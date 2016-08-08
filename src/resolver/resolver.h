#ifndef __RESOLVER_H__
#define __RESOLVER_H__

#include <readtags.h>
#include "common.h"
#include "resolver/snippet.h"
#include "parser/ast.h"

#include "system_resolver.h"
#include "header_sorter.h"

std::set<std::string>
extract_id_to_resolve(std::string code);
std::set<std::string>
extract_id_to_resolve(ast::NodeList nodes);


std::set<std::string>
get_to_resolve(
               ast::NodeList nodes,
               std::set<std::string> known_to_resolve,
               std::set<std::string> known_not_resolve
               );

std::set<std::string>
get_to_resolve(
               std::string code,
               std::set<std::string> known_to_resolve = std::set<std::string>(),
               std::set<std::string> known_not_resolve = std::set<std::string>()
               );




#endif
