#include <saxxy.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <iconv.h>

struct saxxy_parser_s {
	saxxy_token_handler_t token_handler;
	void *user_handle;
	const char *data;
	size_t len;
	bool converted;

	saxxy_attribute_array_t attributes;
};

inline static bool check_add_overflow(size_t a, size_t b, size_t *res) {
#ifdef __builtin_add_overflow
	return __builtin_add_overflow(a, b, res);
#else
	*res = a + b;
	return *res < a;
#endif
}

bool saxxy_attribute_array_store(saxxy_attribute_array_t *attributes, saxxy_attribute_t attribute) {
	if(!attribute.name.ptr && !attribute.value.ptr) {
		return true;
	}

	if(attributes->count >= attributes->size) {
		while(attributes->count >= attributes->size) {
			if(attributes->size == 0) {
				attributes->size = 4;
			} else {
				attributes->size *= 2;
			}
		}
		void *tmp = realloc(attributes->ptr, sizeof(saxxy_attribute_t) * attributes->size);
		if(!tmp) {
			return false;
		}
		attributes->ptr = (saxxy_attribute_t *)tmp;
		memset(attributes->ptr + attributes->count, 0, sizeof(saxxy_attribute_t) * (attributes->size - attributes->count));
	}

	attributes->ptr[attributes->count] = attribute;
	attributes->count++;

	return true;
}

void saxxy_attribute_array_clean(saxxy_attribute_array_t *attributes) {
	if(!attributes) {
		return;
	}

	if(attributes->ptr) {
		free(attributes->ptr);
		attributes->ptr = NULL;
	}
	attributes->count = 0;
	attributes->size = 0;
}

size_t saxxy_attribute_parse(saxxy_parser_t *parser, size_t off, saxxy_attribute_array_t *attributes) {
	size_t s = off;
	saxxy_attribute_t attribute;

	attributes->count = 0;

	while(off < parser->len && parser->data[off] != '>') {
		while(isspace(parser->data[off]) || parser->data[off] == '/') {
			off++;
			if(off >= parser->len || parser->data[off] == '>') {
				return off-s;
			}
		}

		memset(&attribute, 0, sizeof(attribute));

		attribute.name.ptr = parser->data+off;
		while(!isspace(parser->data[off]) && parser->data[off] != '=') {
			off++;
			attribute.name.len++;
			if(off >= parser->len || parser->data[off] == '>') {
				saxxy_attribute_array_store(attributes, attribute);
				return off-s;
			}
		}

		while(isspace(parser->data[off])) {
			off++;
			if(off >= parser->len || parser->data[off] == '>') {
				saxxy_attribute_array_store(attributes, attribute);
				return off-s;
			}
		}

		if(off < parser->len && parser->data[off] == '=') {
			off++;

			if(off >= parser->len || parser->data[off] == '>') {
				saxxy_attribute_array_store(attributes, attribute);
				return off-s;
			}

			while(isspace(parser->data[off])) {
				off++;
				if(off >= parser->len || parser->data[off] == '>') {
					saxxy_attribute_array_store(attributes, attribute);
					return off-s;
				}
			}

			if(off+1 < parser->len && parser->data[off] == '\'') {
				off++;
				attribute.value.ptr = parser->data+off;
				char *c = memchr(parser->data+off, '\'', parser->len-off);
				if(c) {
					attribute.value.len = c - (parser->data+off);
					off++;
				} else {
					attribute.value.len = parser->len - off;
				}
				off += attribute.value.len;
			} else
			if(off+1 < parser->len && parser->data[off] == '"') {
				off++;
				attribute.value.ptr = parser->data+off;
				char *c = memchr(parser->data+off, '"', parser->len-off);
				if(c) {
					attribute.value.len = c - (parser->data+off);
					off++;
				} else {
					attribute.value.len = parser->len - off;
				}
				off += attribute.value.len;
			} else {
				attribute.value.ptr = parser->data+off;
				while(!isspace(parser->data[off])) {
					off++;
					attribute.value.len++;
					if(off >= parser->len || parser->data[off] == '>') {
						break;
					}
				}
			}
		}
		saxxy_attribute_array_store(attributes, attribute);
	}
	return off-s;
}

size_t saxxy_tag_parse(saxxy_parser_t *parser, size_t off, saxxy_token_t *token) {
	if(off+3 > parser->len || parser->data[off] != '<') {
		return 0;
	}
	size_t s = off, l;

	off++;

	token->type = SAXXY_TOKEN_TAG_OPEN;
	token->data.tag.empty = false;
	token->data.tag.name.ptr = parser->data + off;
	if(parser->data[off] == '/') {
		off++;
		token->type = SAXXY_TOKEN_TAG_CLOSE;
		token->data.tag.name.ptr = parser->data + off;
		if(off >= parser->len) {
			return 0;
		}
	}
	if(('a' <= parser->data[off] && parser->data[off] <= 'z') || ('A' <= parser->data[off] && parser->data[off] <= 'Z')) {
		while(!isspace(parser->data[off]) && parser->data[off] != '/' && parser->data[off] != '>') {
			off++;
			if(off >= parser->len) {
				return 0;
			}
		}
		token->data.tag.name.len = (parser->data+off)-token->data.tag.name.ptr;

		l = saxxy_attribute_parse(parser, off, &(token->data.tag.attributes));
		if(off + l >= parser->len) {
			return 0;
			// return parser->len - s;
		}
		off += l;

		while(parser->data[off] != '>') {
			off++;
			if(off >= parser->len) {
				return parser->len-s;
			}
		}

		if(parser->data[off - 1] == '/') {
			token->data.tag.empty = true;
		}
		return off-s+1;
		if(parser->data[off] == '>') {
			return off-s;
		}
	}
	return 0;
}

size_t saxxy_comment_parse(saxxy_parser_t *parser, size_t off, saxxy_token_t *token) {
	if(off+3 > parser->len || parser->data[off] != '<' || (parser->data[off+1] != '!' && parser->data[off+1] != '?')) {
		return 0;
	}
	size_t s = off;
	off += 1;

	bool normal_mode = false;
	if(off+2 < parser->len && parser->data[off] == '!' && parser->data[off+1] == '-' && parser->data[off+2] == '-') {
		normal_mode = true;
		off += 2;
	}
	off += 1;

	token->type = SAXXY_TOKEN_COMMENT;
	token->data.comment.ptr = parser->data+off;
	token->data.comment.len = 0;
	if(normal_mode) {
		while(parser->data[off] != '>' || parser->data[off-1] != '-' || parser->data[off-2] != '-') {
			off++;
			token->data.comment.len++;
			if(off >= parser->len) {
				return 0;
			}
		}
		if(off > s+4) {
			token->data.comment.len -= 2;
		}
	} else {
		while(parser->data[off] != '>') {
			off++;
			if(off >= parser->len) {
				return 0;
			}
		}

		token->data.comment.len = off-s-2;
		if(parser->data[s+1] == '?' && token->data.comment.ptr[token->data.comment.len-1] == '?') {
			token->data.comment.len--;
		}
	}

	return off-s+1;
}

#define __ENTITY_PARSE_VALUE(isentitychar, entityvalue)\
	while(off < parser->len) {\
		if(isentitychar(parser->data[off])) {\
			token->data.entity.entityvalue.len ++;\
		} else if(token->data.entity.entityvalue.len > 0) {\
			if(parser->data[off] == ';') {\
				if(check_add_overflow(off, 1, &off) || off >= parser->len) {\
					return 0;\
				} else {\
					return off+1-start;\
				}\
			} else {\
				return off-start;\
			}\
		} else {\
			return 0;\
		}\
		if(check_add_overflow(off, 1, &off)) {\
			return 0;\
		}\
	}

size_t saxxy_entity_parse(saxxy_parser_t *parser, size_t off, saxxy_token_t *token) {
	if(off >= parser->len || parser->data[off] != '&') {
		return 0;
	}

	size_t start = off;

	token->type = SAXXY_TOKEN_ENTITY;
	token->data.entity.type = SAXXY_ENTITY_TYPE_NAME;
	if(check_add_overflow(off, 1, &off) || off >= parser->len) {
		return 0;
	} else if(parser->data[off] == '#') {
		token->data.entity.type = SAXXY_ENTITY_TYPE_DECIMAL;
		if(check_add_overflow(off, 1, &off) || off >= parser->len) {
			return 0;
		} else if(parser->data[off] == 'x') {
			token->data.entity.type = SAXXY_ENTITY_TYPE_HEXADECIMAL;
			if(check_add_overflow(off, 1, &off) || off >= parser->len) {
				return 0;
			}
		}
	}

	token->data.entity.name.ptr = parser->data + off;
	token->data.entity.name.len = 0;
	if(token->data.entity.type == SAXXY_ENTITY_TYPE_NAME) {
		__ENTITY_PARSE_VALUE(isalnum, name);
	} else if(token->data.entity.type == SAXXY_ENTITY_TYPE_DECIMAL) {
		__ENTITY_PARSE_VALUE(isdigit, decimal);
	} else if(token->data.entity.type == SAXXY_ENTITY_TYPE_HEXADECIMAL) {
		__ENTITY_PARSE_VALUE(isxdigit, hexadecimal);
	}
	return 0;
}

bool saxxy_html_parse(saxxy_parser_t *parser) {
	size_t s = 0, i = 0, l = 0;
	saxxy_token_t token, text;
	char *encoding = NULL, *from = NULL, *to = NULL, *to_orig = NULL;
	size_t from_len, to_len, to_len_orig;
	bool inside_raw_element = false;
	saxxy_string_t raw_element;

	if(!parser) {
		return false;
	}

	memset(&token, 0, sizeof(token));
	memset(&text, 0, sizeof(text));
	memset(&raw_element, 0, sizeof(raw_element));

	if(parser->len > 4 && parser->data[0] == '\x00' && parser->data[1] == '\x00' && parser->data[2] == '\xFE' && parser->data[3] == '\xFF') {
		encoding = "UTF32BE";
		from = (char *)parser->data;
		from_len = parser->len;
		to_len_orig = to_len = SAXXY_ROUNDUP_DIV(parser->len-2, 4)*6+3;
		to_orig = to = calloc(to_len, sizeof(char));
		if(!to) {
			return false;
		}
	}
	else if(parser->len > 4 && parser->data[0] == '\xFF' && parser->data[1] == '\xFE' && parser->data[2] == '\x00' && parser->data[3] == '\x00') {
		encoding = "UTF32LE";
		from = (char *)parser->data;
		from_len = parser->len;
		to_len_orig = to_len = SAXXY_ROUNDUP_DIV(parser->len-2, 4)*6+3;
		to_orig = to = calloc(to_len, sizeof(char));
		if(!to) {
			return false;
		}
	}
	else if(parser->len > 2 && parser->data[0] == '\xFE' && parser->data[1] == '\xFF') {
		encoding = "UTF16BE";
		from = (char *)parser->data;
		from_len = parser->len;
		to_len_orig = to_len = SAXXY_ROUNDUP_DIV(parser->len-2, 2)*6+3;
		to_orig = to = calloc(to_len, sizeof(char));
		if(!to) {
			return false;
		}
	}
	else if(parser->len > 2 && parser->data[0] == '\xFF' && parser->data[1] == '\xFE') {
		encoding = "UTF16LE";
		from = (char *)parser->data;
		from_len = parser->len;
		to_len_orig = to_len = SAXXY_ROUNDUP_DIV(parser->len-2, 2)*6+3;
		to_orig = to = calloc(to_len, sizeof(char));
		if(!to) {
			return false;
		}
	}

	if(encoding) {
		iconv_t cd = iconv_open("UTF8", encoding);
		if(cd == (iconv_t) -1) {
			free(to);
			return false;
		}
		errno = 0;
		if(iconv(cd, &from, &from_len, &to, &to_len) == (size_t) -1) {
			perror("iconv");
			free(to_orig);
			return false;
		}
		iconv_close(cd);

		parser->data = to_orig;
		parser->len = to_len_orig-to_len;
		parser->converted = true;
	}

	// Skip UTF-8 BOM
	if(parser->len >= 3 && parser->data[0] == '\xEF' && parser->data[1] == '\xBB' && parser->data[2] == '\xBF') {
		s = i = 3;
	}

	while(i < parser->len) {
		l = 0;
		if(parser->data[i] == '<') {
			if(!inside_raw_element && (parser->data[i+1] == '!' || parser->data[i+1] == '?')) {
				l = saxxy_comment_parse(parser, i, &token);
			} else if(isalpha(parser->data[i+1]) || parser->data[i+1] == '/') {
				token.data.tag.attributes = parser->attributes;
				l = saxxy_tag_parse(parser, i, &token);
				if(l > 0) {
					parser->attributes = token.data.tag.attributes;
					if(inside_raw_element) {
						if(SAXXY_TOKEN_TAG_CLOSE_MATCH(raw_element, token)) {
							inside_raw_element = false;
						} else {
							l = 0;
						}
					}
				}
			}
		} else if(parser->data[i] == '&' && !inside_raw_element) {
			l = saxxy_entity_parse(parser, i, &token);
		}
		if(l > 0) {
			if(i > s) {
				text.type = SAXXY_TOKEN_TEXT;
				text.data.text.ptr = parser->data + s;
				text.data.text.len = i - s;
				if(parser->token_handler) {
					if(!parser->token_handler(&text, parser->user_handle)) {
						return false;
					}
				}
			}
			if(SAXXY_TOKEN_TAG_OPEN_MATCH_STATIC("title", token)
			   || SAXXY_TOKEN_TAG_OPEN_MATCH_STATIC("textarea", token)
			   || SAXXY_TOKEN_TAG_OPEN_MATCH_STATIC("script", token)
			   || SAXXY_TOKEN_TAG_OPEN_MATCH_STATIC("style", token)) {
				inside_raw_element = true;
				raw_element = token.data.tag.name;
			}

			if(parser->token_handler) {
				if(!parser->token_handler(&token, parser->user_handle)) {
					return false;
				}
			}
			s = i+l;
		} else {
			l = 1;
		}
		if(check_add_overflow(i, l, &i)) {
			break;
		}
	}

	if(parser->len > s) {
		text.type = SAXXY_TOKEN_TEXT;
		text.data.text.ptr = parser->data + s;
		text.data.text.len = parser->len - s;
		if(parser->token_handler) {
			if(!parser->token_handler(&text, parser->user_handle)) {
				return false;
			}
		}
	}

	return true;
}

saxxy_parser_t *saxxy_parser_new() {
	saxxy_parser_t *parser = calloc(1, sizeof(*parser));
	return parser;
}

void saxxy_parser_init(saxxy_parser_t *parser, const char *data, size_t len) {
	memset(parser, 0, sizeof(*parser));
	parser->data = data;
	parser->len = len;
}

void saxxy_parser_set_token_handler(saxxy_parser_t *parser, saxxy_token_handler_t token_handler, void *user_handle) {
	parser->token_handler = token_handler;
	parser->user_handle = user_handle;
}

void saxxy_parser_clean(saxxy_parser_t *parser) {
	if(!parser) {
		return;
	}

	if(parser->converted) {
		free((void *)parser->data);
	}
	parser->data = NULL;
	parser->len = 0;

	saxxy_attribute_array_clean(&(parser->attributes));
}

void saxxy_parser_free(saxxy_parser_t *parser) {
	saxxy_parser_clean(parser);
	free(parser);
}

void saxxy_style_parse(saxxy_attribute_array_t *attributes, saxxy_string_t style) {
	size_t off = 0;
	saxxy_attribute_t attribute;
	while(off < style.len) {
		while(isspace(style.ptr[off])) {
			off++;
			if(off >= style.len) {
				return;
			}
		}

		memset(&attribute, 0, sizeof(attribute));
		attribute.name.ptr = style.ptr+off;
		while(!isspace(style.ptr[off]) && style.ptr[off] != ':' && style.ptr[off] != ';') {
			off++;
			attribute.name.len++;
			if(off >= style.len) {
				saxxy_attribute_array_store(attributes, attribute);
				return;
			}
		}

		while(isspace(style.ptr[off])) {
			off++;
			if(off >= style.len) {
				saxxy_attribute_array_store(attributes, attribute);
				return;
			}
		}

		if(off < style.len && style.ptr[off] == ':') {
			off++;

			if(off >= style.len) {
				saxxy_attribute_array_store(attributes, attribute);
				return;
			}

			while(isspace(style.ptr[off])) {
				off++;
				if(off >= style.len) {
					saxxy_attribute_array_store(attributes, attribute);
					return;
				}
			}

			attribute.value.ptr = style.ptr+off;
			while(style.ptr[off] != ';') {
				off++;
				attribute.value.len++;
				if(off >= style.len) {
					saxxy_attribute_array_store(attributes, attribute);
					return;
				}
			}
		}

		saxxy_attribute_array_store(attributes, attribute);

		while(style.ptr[off] == ';') {
			off++;
			if(off >= style.len) {
				return;
			}
		}
	}
}
