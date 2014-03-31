#ifndef _SAXXY_H_
#define _SAXXY_H_

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

typedef struct saxxy_tag {
	saxxy_string ns;
	saxxy_string name;
	size_t n_attributes;		/**< Count of attributes */
	saxxy_attribute *attributes;	/**< Array of attribute data */
	bool self_closing;
} saxxy_tag;

typedef enum saxxy_token_type {
	HTML_TOKEN_DOCTYPE,
	HTML_TOKEN_START_TAG,
	HTML_TOKEN_END_TAG,
	HTML_TOKEN_COMMENT,
	HTML_TOKEN_CHARACTER,
	HTML_TOKEN_EOF
} saxxy_token_type;

typedef struct saxxy_token {
	saxxy_token_type type;
	union {
		saxxy_tag tag;
		saxxy_string comment;
	} data;
} saxxy_token;

typedef void (*saxxy_token_handler)(const saxxy_token *token, void *user_handle);

typedef struct saxxy_parser {
	saxxy_token_handler token_handler;
	void *user_handle;
	saxxy_tag current_tag;
	size_t a_attributes;
	const uint8_t *data;
	size_t len;
} html_parser;

#endif
