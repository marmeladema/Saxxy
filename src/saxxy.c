#include <saxxy.h>

size_t saxxy_attribute_parse(saxxy_parser *parser, size_t off) {
	size_t s = off;
	parser->current_tag.n_attributes = 0;
	while(off < parser->len && parser->data[off] != '>') {
		while(parser->data[off] == ' ' || parser->data[off] == '\t' || parser->data[off] == '\r' || parser->data[off] == '\n') {
			off++;
			if(off >= parser->len || parser->data[off] == '>') {
				return off-s;
			}
		}
		saxxy_string name;
		name.ptr = parser->data+off;
		name.len = 0;
		while(parser->data[off] != ' ' && parser->data[off] != '\t' && parser->data[off] != '\r' && parser->data[off] != '\n' && parser->data[off] != '=') {
			off++;
			name.len++;
			if(off >= parser->len || parser->data[off] == '>') {
				return off-s;
			}
		}
		
		if(parser->current_tag.n_attributes >= parser->a_attributes) {
			if(parser->a_attributes == 0) {
				parser->a_attributes = 4;
			} else {
				parser->a_attributes *= 2;
			}
			TK_REALLOC(parser->current_tag.attributes, saxxy_attribute, parser->a_attributes);
		}
		memset(parser->current_tag.attributes + parser->current_tag.n_attributes, 0, sizeof(saxxy_attribute));
		parser->current_tag.n_attributes++;
		fwrite("name: ", strlen("name: "), 1, stdout);
		fwrite(name.ptr, name.len, 1, stdout);
		fwrite("\n", 1, 1, stdout);
		
		while(parser->data[off] == ' ' || parser->data[off] == '\t' || parser->data[off] == '\r' || parser->data[off] == '\n') {
			off++;
			if(off >= parser->len || parser->data[off] == '>') {
				return off-s;
			}
		}
		
		if(off+1 < parser->len && parser->data[off] == '=') {
			off++;
			html_string value;
			value.len = 0;
			while(parser->data[off] == ' ' || parser->data[off] == '\t' || parser->data[off] == '\r' || parser->data[off] == '\n') {
				off++;
				if(off >= parser->len || parser->data[off] == '>') {
					return off-s;
				}
			}
			
			if(off+1 < parser->len && parser->data[off] == '\'') {
				off++;
				value.ptr = parser->data+off;
				uint8_t *c = memchr(parser->data+off, '\'', parser->len-off);
				if(c) {
					value.len = c - (parser->data+off);
					off++;
				} else {
					value.len = parser->len - off;
				}
				off += value.len;
			} else
			if(off+1 < parser->len && parser->data[off] == '"') {
				off++;
				value.ptr = parser->data+off;
				uint8_t *c = memchr(parser->data+off, '"', parser->len-off);
				if(c) {
					value.len = c - (parser->data+off);
					off++;
				} else {
					value.len = parser->len - off;
				}
				off += value.len;
			} else {
				value.ptr = parser->data+off;
				while(parser->data[off] != ' ' && parser->data[off] != '\t' && parser->data[off] != '\r' && parser->data[off] != '\n') {
					off++;
					value.len++;
					if(off >= parser->len || parser->data[off] == '>') {
						break;
					}
				}
			}
			
			fwrite("value: ", strlen("value: "), 1, stdout);
			fwrite(value.ptr, value.len, 1, stdout);
			fwrite("\n", 1, 1, stdout);
		}
	}
	return off-s;
}

size_t saxxy_tag_parse(saxxy_parser *parser, size_t off) {
	if(off+3 >= parser->len || parser->data[off] != '<') {
		return 0;
	}
	size_t s = off, l;
	saxxy_token token;
	
	off++;
#if 0
	if(parser->data[off] == '?') {
		
	}
	else
	if(parser->data[off] == '!') {
		
	}
	else
#endif
	token.type = HTML_TOKEN_START_TAG;
	if(parser->data[off] == '/') {
		token.type = HTML_TOKEN_END_TAG;
		off++;
		if(off >= parser->len) {
			return 0;
		}
	}
	if(('a' <= parser->data[off] && parser->data[off] <= 'z') || ('A' <= parser->data[off] && parser->data[off] <= 'Z')) {
		parser->current_tag.name.ptr = parser->data + off;
		// while('a' <= data[off] && data[off] <= 'z' && 'A' <= data[off] && data[off] <= 'Z' && '0' <= data[off] && data[off] <= '9') {
		while(parser->data[off] != ' ' && parser->data[off] != '\t' && parser->data[off] != '\r' && parser->data[off] != '\n' && parser->data[off] != '>') {
			off++;
			if(off >= parser->len) {
				return 0;
			}
		}
		parser->current_tag.name.len = off-s;
		
		l = attribute_parse(parser, off);
		if(off + l >= parser->len) {
			return parser->len - s;
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
		}
		
		token.data.tag = parser->current_tag;
		if(parser->token_handler) {
			parser->token_handler(&token, parser->user_handle);
		}
		return off-s+1;
		if(parser->data[off] == '>') {
			return off-s;
		}
	}
	return 0;
}

void saxxy_html_parse(saxxy_parser *parser) {
	size_t level = 0, s = 0, i = 0, l = 0;
	
	if(!parser) {
		return;
	}
	
	while(i < parser->len) {
		while(parser->data[i] != '<') {
			i++;
		}
		l = tag_parse(parser, i);
		if(l > 0) {
			fwrite(parser->data+i, l, 1, stdout);
			fwrite("\n", 1, 1, stdout);
			i += l;
		}
		i++;
	}
}
