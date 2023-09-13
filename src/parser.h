#ifndef PARSER_H
#define PARSER_H

#include "module.h"

void parser_parse(Module* module);
bool parser_should_remove_statement_semicolon(Expr* expression);

#endif // PARSER_H
