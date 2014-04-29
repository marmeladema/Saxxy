#ifndef _SAXXY_H_
#define _SAXXY_H_

#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>

#define SAXXY_ROUNDUP_DIV(x, y) (1 + (((x) - 1) / (y)))

#define SAXXY_STRLEN_STATIC(s) 						(sizeof(s)-1)

#define SAXXY_STRCMP_STATIC(s, s2) 					((SAXXY_STRLEN_STATIC(s) == (s2).len) && (strncmp((s), (s2).ptr, (s2).len) == 0))
#define SAXXY_STRCMP(s, s2) 						(((s).len == (s2).len) && (strncmp((s).ptr, (s2).ptr, (s2).len) == 0))

#define SAXXY_STRCASECMP_STATIC(s, s2) 				((SAXXY_STRLEN_STATIC(s) == (s2).len) && (strncasecmp((s), (s2).ptr, (s2).len) == 0))
#define SAXXY_STRCASECMP(s, s2) 					(((s).len == (s2).len) && (strncasecmp((s).ptr, (s2).ptr, (s2).len) == 0))

#define SAXXY_TAG_MATCH_STATIC(s, t) 				(SAXXY_STRCASECMP_STATIC(s, (t).name))
#define SAXXY_TAG_MATCH(s, t) 						(SAXXY_STRCASECMP(s, (t).name))

#define SAXXY_TAG_OPEN_MATCH_STATIC(s, t) 			((t).type == SAXXY_TAG_OPEN && (SAXXY_TAG_MATCH_STATIC(s, (t))))
#define SAXXY_TAG_OPEN_MATCH(s, t) 					((t).type == SAXXY_TAG_OPEN && (SAXXY_TAG_MATCH(s, (t))))

#define SAXXY_TAG_CLOSE_MATCH_STATIC(s, t) 			((t).type == SAXXY_TAG_CLOSE && (SAXXY_TAG_MATCH_STATIC(s, (t))))
#define SAXXY_TAG_CLOSE_MATCH(s, t) 				((t).type == SAXXY_TAG_CLOSE && (SAXXY_TAG_MATCH(s, (t))))

#define SAXXY_TOKEN_TAG_MATCH_STATIC(s, t) 			((t).type == SAXXY_TOKEN_TAG && (SAXXY_TAG_MATCH_STATIC(s, (t).data.tag)))
#define SAXXY_TOKEN_TAG_MATCH(s, t) 				((t).type == SAXXY_TOKEN_TAG && (SAXXY_TAG_MATCH(s, (t).data.tag)))

#define SAXXY_TOKEN_TAG_OPEN_MATCH_STATIC(s, t) 	((t).type == SAXXY_TOKEN_TAG && (SAXXY_TAG_OPEN_MATCH_STATIC(s, (t).data.tag)))
#define SAXXY_TOKEN_TAG_OPEN_MATCH(s, t)		 	((t).type == SAXXY_TOKEN_TAG && (SAXXY_TAG_OPEN_MATCH(s, (t).data.tag)))

#define SAXXY_TOKEN_TAG_CLOSE_MATCH_STATIC(s, t) 	((t).type == SAXXY_TOKEN_TAG && (SAXXY_TAG_CLOSE_MATCH_STATIC(s, (t).data.tag)))
#define SAXXY_TOKEN_TAG_CLOSE_MATCH(s, t) 			((t).type == SAXXY_TOKEN_TAG && (SAXXY_TAG_CLOSE_MATCH(s, (t).data.tag)))

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

typedef enum saxxy_tag_type {
	SAXXY_TAG_OPEN,
	SAXXY_TAG_CLOSE,
} saxxy_tag_type;

typedef struct saxxy_tag {
	saxxy_tag_type type;
	saxxy_string ns;
	saxxy_string name;
	saxxy_attribute_array attributes;
	bool self_closing;
} saxxy_tag;

typedef enum saxxy_token_type {
	SAXXY_TOKEN_TAG,
	SAXXY_TOKEN_COMMENT,
	SAXXY_TOKEN_TEXT,
	SAXXY_TOKEN_EOF
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
	
	saxxy_tag current_tag;
	saxxy_string current_comment;
	bool inside_raw_element;
	saxxy_string raw_element;
} saxxy_parser;

bool saxxy_html_parse(saxxy_parser *parser);

void saxxy_parser_clean(saxxy_parser *parser);

#endif
