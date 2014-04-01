#ifndef _SAXXY_H_
#define _SAXXY_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct saxxy_string {
	const uint8_t *ptr;
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
		saxxy_string character;
	} data;
} saxxy_token;

typedef void (*saxxy_token_handler)(const saxxy_token *token, void *user_handle);

typedef struct saxxy_parser {
	saxxy_token_handler token_handler;
	void *user_handle;
	saxxy_tag current_tag;
	saxxy_string current_comment;
	const uint8_t *data;
	size_t len;
} saxxy_parser;

void saxxy_html_parse(saxxy_parser *parser);

#endif
