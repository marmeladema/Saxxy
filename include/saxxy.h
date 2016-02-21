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

typedef struct saxxy_string_s {
	const char *ptr;
	size_t len;
} saxxy_string_t;

typedef struct saxxy_attribute_s {
	saxxy_string_t ns;
	saxxy_string_t name;
	saxxy_string_t value;
} saxxy_attribute_t;

typedef struct saxxy_attribute_array_s {
	saxxy_attribute_t *ptr;
	size_t count;
	size_t size;
} saxxy_attribute_array_t;

typedef struct saxxy_tag_s {
	saxxy_string_t ns;
	saxxy_string_t name;
	saxxy_attribute_array_t attributes;
	bool empty;
} saxxy_tag_t;

typedef enum saxxy_entity_type_e {
	SAXXY_ENTITY_TYPE_NAME,
	SAXXY_ENTITY_TYPE_DECIMAL,
	SAXXY_ENTITY_TYPE_HEXADECIMAL,
} saxxy_entity_type_t;

typedef struct saxxy_entity_s {
	saxxy_entity_type_t type;
	union {
		saxxy_string_t name;
		saxxy_string_t decimal;
		saxxy_string_t hexadecimal;
	};
} saxxy_entity_t;

typedef enum saxxy_token_type_e {
	SAXXY_TOKEN_UNKNOWN = 0,
	SAXXY_TOKEN_TAG_OPEN,
	SAXXY_TOKEN_TAG_CLOSE,
	SAXXY_TOKEN_COMMENT,
	SAXXY_TOKEN_TEXT,
	SAXXY_TOKEN_ENTITY,
	SAXXY_TOKEN_EOF
} saxxy_token_type_t;

typedef union saxxy_token_data_u {
	saxxy_tag_t tag;
	saxxy_string_t comment;
	saxxy_string_t text;
	saxxy_entity_t entity;
} saxxy_token_data_t ;

typedef struct saxxy_token_s {
	saxxy_token_data_t data;
	saxxy_token_type_t type;
} saxxy_token_t;

typedef bool (*saxxy_token_handler_t)(const saxxy_token_t *token, void *user_handle);

typedef struct saxxy_parser_s saxxy_parser_t;

size_t saxxy_attribute_parse(saxxy_parser_t *parser, size_t off, saxxy_attribute_array_t *attributes);
void saxxy_attribute_array_clean(saxxy_attribute_array_t *attributes);
size_t saxxy_comment_parse(saxxy_parser_t *parser, size_t off, saxxy_token_t *token);
size_t saxxy_entity_parse(saxxy_parser_t *parser, size_t off, saxxy_token_t *token);
bool saxxy_html_parse(saxxy_parser_t *parser);

saxxy_parser_t *saxxy_parser_new();
void saxxy_parser_init(saxxy_parser_t *parser, const char *data, size_t len);
void saxxy_parser_set_token_handler(saxxy_parser_t *parser, saxxy_token_handler_t token_handler, void *user_handle);
void saxxy_parser_clean(saxxy_parser_t *parser);
void saxxy_parser_free(saxxy_parser_t *parser);

void saxxy_style_parse(saxxy_attribute_array_t *attributes, saxxy_string_t style);

#endif
