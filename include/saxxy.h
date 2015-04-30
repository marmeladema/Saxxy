#ifndef _SAXXY_H_
#define _SAXXY_H_

#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>

#define SAXXY_ROUNDUP_DIV(x, y) 					(1 + (((x) - 1) / (y)))

#define SAXXY_STRLEN_STATIC(s) 						(sizeof(s)-1)

#define SAXXY_STRCMP_STATIC(s, s2) 					((s2).ptr && (SAXXY_STRLEN_STATIC(s) == (s2).len) && (strncmp((s), (s2).ptr, (s2).len) == 0))
#define SAXXY_STRCMP(s, s2) 						((s).ptr && (s2).ptr && ((s).len == (s2).len) && (strncmp((s).ptr, (s2).ptr, (s2).len) == 0))

#define SAXXY_STRCASECMP_STATIC(s, s2) 				((s2).ptr && (SAXXY_STRLEN_STATIC(s) == (s2).len) && (strncasecmp((s), (s2).ptr, (s2).len) == 0))
#define SAXXY_STRCASECMP(s, s2) 					((s).ptr && (s2).ptr && ((s).len == (s2).len) && (strncasecmp((s).ptr, (s2).ptr, (s2).len) == 0))

#define SAXXY_TAG_MATCH_STATIC(s, t) 				(SAXXY_STRCASECMP_STATIC(s, (t).name))
#define SAXXY_TAG_MATCH(s, t) 						(SAXXY_STRCASECMP(s, (t).name))
/*
#define SAXXY_TAG_OPEN_MATCH_STATIC(s, t) 			((t).type == SAXXY_TAG_OPEN && (SAXXY_TAG_MATCH_STATIC(s, (t))))
#define SAXXY_TAG_OPEN_MATCH(s, t) 					((t).type == SAXXY_TAG_OPEN && (SAXXY_TAG_MATCH(s, (t))))

#define SAXXY_TAG_CLOSE_MATCH_STATIC(s, t) 			((t).type == SAXXY_TAG_CLOSE && (SAXXY_TAG_MATCH_STATIC(s, (t))))
#define SAXXY_TAG_CLOSE_MATCH(s, t) 				((t).type == SAXXY_TAG_CLOSE && (SAXXY_TAG_MATCH(s, (t))))

#define SAXXY_TOKEN_TAG_MATCH_STATIC(s, t) 			((t).type == SAXXY_TOKEN_TAG && (SAXXY_TAG_MATCH_STATIC(s, (t).data.tag)))
#define SAXXY_TOKEN_TAG_MATCH(s, t) 				((t).type == SAXXY_TOKEN_TAG && (SAXXY_TAG_MATCH(s, (t).data.tag)))
*/
#define SAXXY_TOKEN_TAG_OPEN_MATCH_STATIC(s, t) 	(((t).type&SAXXY_TOKEN_TAG_OPEN) && (SAXXY_TAG_MATCH_STATIC(s, (t).data.tag)))
#define SAXXY_TOKEN_TAG_OPEN_MATCH(s, t)		 	(((t).type&SAXXY_TOKEN_TAG_OPEN) && (SAXXY_TAG_MATCH(s, (t).data.tag)))

#define SAXXY_TOKEN_TAG_CLOSE_MATCH_STATIC(s, t) 	(((t).type&SAXXY_TOKEN_TAG_CLOSE) && (SAXXY_TAG_MATCH_STATIC(s, (t).data.tag)))
#define SAXXY_TOKEN_TAG_CLOSE_MATCH(s, t) 			(((t).type&SAXXY_TOKEN_TAG_CLOSE) && (SAXXY_TAG_MATCH(s, (t).data.tag)))

typedef struct saxxy_string {
	const char *ptr;
	size_t len;
} saxxy_string;

typedef struct saxxy_attribute {
	saxxy_string ns;
	saxxy_string name;
	saxxy_string value;
} saxxy_attribute;

typedef struct saxxy_attribute_array {
	saxxy_attribute *ptr;
	size_t count;
	size_t size;
} saxxy_attribute_array;

typedef struct saxxy_tag {
	saxxy_string ns;
	saxxy_string name;
	saxxy_attribute_array attributes;
} saxxy_tag;

typedef enum saxxy_token_type {
	SAXXY_TOKEN_TAG_OPEN = 1,
	SAXXY_TOKEN_TAG_CLOSE = 1<<1,
	SAXXY_TOKEN_TAG_OPEN_CLOSE = (1 | 1<<1),
	SAXXY_TOKEN_COMMENT = 1<<2,
	SAXXY_TOKEN_TEXT = 1<<3,
	SAXXY_TOKEN_EOF = 1<<4
} saxxy_token_type;

typedef struct saxxy_token {
	saxxy_token_type type;
	union {
		saxxy_tag tag;
		saxxy_string comment;
		saxxy_string text;
	} data;
} saxxy_token;

typedef void (*saxxy_token_handler)(const saxxy_token *token, void __attribute__ ((unused)) *user_handle);

typedef struct saxxy_parser {
	saxxy_token_handler token_handler;
	void *user_handle;
	const char *data;
	size_t len;
	bool converted;

	saxxy_attribute_array attributes;
} saxxy_parser;

size_t saxxy_attribute_parse(saxxy_parser *parser, size_t off, saxxy_attribute_array *attributes);
void saxxy_attribute_array_clean(saxxy_attribute_array *attributes);
size_t saxxy_comment_parse(saxxy_parser *parser, size_t off, saxxy_token *token);
bool saxxy_html_parse(saxxy_parser *parser);
void saxxy_parser_clean(saxxy_parser *parser);

void saxxy_style_parse(saxxy_attribute_array *attributes, saxxy_string style);

#endif
