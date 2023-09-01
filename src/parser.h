#ifndef PARSER_H
#define PARSER_H

#include "module.h"
#include "token.h"

Module parser_parse(Span source, TokenList token_list);

#endif // PARSER_H
