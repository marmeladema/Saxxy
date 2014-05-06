#include <saxxy.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <iconv.h>

bool saxxy_attribute_array_store(saxxy_attribute_array *attributes, saxxy_attribute attribute) {
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
		void *tmp = realloc(attributes->ptr, sizeof(saxxy_attribute) * attributes->size);
		if(!tmp) {
			return false;
		}
		attributes->ptr = (saxxy_attribute *)tmp;
		memset(attributes->ptr + attributes->count, 0, sizeof(saxxy_attribute) * (attributes->size - attributes->count));
	}
	
	attributes->ptr[attributes->count] = attribute;
	attributes->count++;

	return true;
}

void saxxy_attribute_array_clean(saxxy_attribute_array *attributes) {
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

size_t saxxy_attribute_parse(saxxy_parser *parser, size_t off) {
	size_t s = off;
	parser->current_tag.attributes.count = 0;
	saxxy_attribute attribute;

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
				saxxy_attribute_array_store(&parser->current_tag.attributes, attribute);
				return off-s;
			}
		}

		while(isspace(parser->data[off])) {
			off++;
			if(off >= parser->len || parser->data[off] == '>') {
				saxxy_attribute_array_store(&parser->current_tag.attributes, attribute);
				return off-s;
			}
		}

		if(off+1 < parser->len && parser->data[off] == '=') {
			off++;

			while(isspace(parser->data[off])) {
				off++;
				if(off >= parser->len || parser->data[off] == '>') {
					saxxy_attribute_array_store(&parser->current_tag.attributes, attribute);
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
		saxxy_attribute_array_store(&parser->current_tag.attributes, attribute);
	}
	return off-s;
}

size_t saxxy_tag_parse(saxxy_parser *parser, size_t off) {
	if(off+3 > parser->len || parser->data[off] != '<') {
		return 0;
	}
	size_t s = off, l;

	off++;

	parser->current_tag.type = SAXXY_TAG_OPEN;
	parser->current_tag.name.ptr = parser->data + off;
	if(parser->data[off] == '/') {
		off++;
		parser->current_tag.type = SAXXY_TAG_CLOSE;
		parser->current_tag.name.ptr = parser->data + off;
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
		parser->current_tag.name.len = (parser->data+off)-parser->current_tag.name.ptr;

		l = saxxy_attribute_parse(parser, off);
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
			parser->current_tag.self_closing = true;
		} else {
			parser->current_tag.self_closing = false;
		}
		return off-s+1;
		if(parser->data[off] == '>') {
			return off-s;
		}
	}
	return 0;
}

size_t saxxy_comment_parse(saxxy_parser *parser, size_t off) {
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
	parser->current_comment.ptr = parser->data+off;
	parser->current_comment.len = 0;
	if(normal_mode) {
		while(parser->data[off] != '>' || parser->data[off-1] != '-' || parser->data[off-2] != '-') {
			off++;
			parser->current_comment.len++;
			if(off >= parser->len) {
				return 0;
			}
		}
		if(off > s+4) {
			parser->current_comment.len -= 2;
		}
	} else {
		while(parser->data[off] != '>') {
			off++;
			if(off >= parser->len) {
				return 0;
			}
		}
		parser->current_comment.len = off-s-2;
		if(parser->data[s+1] == '?' && parser->current_comment.ptr[parser->current_comment.len-1] == '?') {
			parser->current_comment.len--;
		}
	}

	return off-s+1;
}

bool saxxy_html_parse(saxxy_parser *parser) {
	size_t s = 0, i = 0, l = 0;
	saxxy_token text_token, token;
	char *encoding = NULL, *from = NULL, *to = NULL, *to_orig = NULL;
	size_t from_len, to_len, to_len_orig;

	if(!parser) {
		return false;
	}

	if(parser->len > 4 && parser->data[0] == '\x00' && parser->data[1] == '\x00' && parser->data[2] == '\xFE' && parser->data[3] == '\xFF') {
		encoding = "UTF32BE";
		from = (char *)parser->data;
		from_len = parser->len;
		to_len = SAXXY_ROUNDUP_DIV(parser->len-2, 4)*6+3;
		to_orig = to = calloc(to_len, sizeof(char));
		if(!to) {
			return false;
		}
	}
	else if(parser->len > 4 && parser->data[0] == '\xFF' && parser->data[1] == '\xFE' && parser->data[2] == '\x00' && parser->data[3] == '\x00') {
		encoding = "UTF32LE";
		from = (char *)parser->data;
		from_len = parser->len;
		to_len = SAXXY_ROUNDUP_DIV(parser->len-2, 4)*6+3;
		to_orig = to = calloc(to_len, sizeof(char));
		if(!to) {
			return false;
		}
	}
	else if(parser->len > 2 && parser->data[0] == '\xFE' && parser->data[1] == '\xFF') {
		encoding = "UTF16BE";
		from = (char *)parser->data;
		from_len = parser->len;
		to_len = SAXXY_ROUNDUP_DIV(parser->len-2, 2)*6+3;
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
	if(parser->len > 3 && parser->data[0] == '\xEF' && parser->data[1] == '\xBB' && parser->data[2] == '\xBF') {
		s = i = 3;
	}

	while(i < parser->len) {
		while(parser->data[i] != '<') {
			i++;
			if(i >= parser->len) {
				break;
			}
		}
		if(i+2 >= parser->len) {
			 break;
		}
		l = 0;
		if(!parser->inside_raw_element && (parser->data[i+1] == '!' || parser->data[i+1] == '?')) {
			l = saxxy_comment_parse(parser, i);
			token.type = SAXXY_TOKEN_COMMENT;
			token.data.comment = parser->current_comment;
		} else if(isalpha(parser->data[i+1]) != 0 || parser->data[i+1] == '/') {
			l = saxxy_tag_parse(parser, i);
			token.type = SAXXY_TOKEN_TAG;
			token.data.tag = parser->current_tag;
		}

		if(l > 0 && parser->inside_raw_element) {
			if(!SAXXY_TOKEN_TAG_CLOSE_MATCH(parser->raw_element, token)) {
				l = 0;
			} else {
				parser->inside_raw_element = false;
			}
		}
		if(l > 0) {
			if(i > s) {
				text_token.type = SAXXY_TOKEN_TEXT;
				text_token.data.text.ptr = parser->data + s;
				text_token.data.text.len = i - s;
				if(parser->token_handler) {
					parser->token_handler(&text_token, parser->user_handle);
				}
			}
			if(SAXXY_TOKEN_TAG_OPEN_MATCH_STATIC("script", token) || SAXXY_TOKEN_TAG_OPEN_MATCH_STATIC("style", token)) {
				parser->inside_raw_element = true;
				parser->raw_element = token.data.tag.name;
			}

			if(parser->token_handler) {
				parser->token_handler(&token, parser->user_handle);
			}
			i += l;
			s = i;
		} else {
			i++;
		}
	}

	if(parser->len > s) {
		text_token.type = SAXXY_TOKEN_TEXT;
		text_token.data.text.ptr = parser->data + s;
		text_token.data.text.len = parser->len - s;
		if(parser->token_handler) {
			parser->token_handler(&text_token, parser->user_handle);
		}
	}

	return true;
}

void saxxy_parser_clean(saxxy_parser *parser) {
	if(!parser) {
		return;
	}

	if(parser->converted) {
		free((void *)parser->data);
	}
	parser->data = NULL;
	parser->len = 0;

	saxxy_attribute_array_clean(&parser->current_tag.attributes);
}

void saxxy_style_parse(saxxy_attribute_array *attributes, saxxy_string style) {
	size_t off = 0;
	saxxy_attribute attribute;
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

		if(off+1 < style.len && style.ptr[off] == ':') {
			off++;

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