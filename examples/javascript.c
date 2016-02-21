#include <saxxy.h>

#include <string.h>
#include <stdio.h>
#include <sys/mman.h>

bool script_handler(const saxxy_token_t *token, void *user_handle) {
	bool *inside_script = (bool *)user_handle;

	switch(token->type) {
		case SAXXY_TOKEN_TAG_OPEN:
		case SAXXY_TOKEN_TAG_CLOSE:
			if(strlen("script") == token->data.tag.name.len && strncasecmp("script", token->data.tag.name.ptr, token->data.tag.name.len) == 0) {
				if(token->type == SAXXY_TOKEN_TAG_OPEN) {
					*inside_script = true;
				} else {
					*inside_script = false;
				}
			}
		break;

		case SAXXY_TOKEN_TEXT:
			if(inside_script && *inside_script) {
				fwrite(token->data.text.ptr, token->data.text.len, 1, stdout);
			}
		break;

		default:
		break;
	}

	return true;
}

int main(int argc, char *argv[]) {
	saxxy_parser_t *parser = saxxy_parser_new();

	if(argc < 2) {
		printf("Usage: %s <file>\n", argv[0]);
		return 1;
	}

	FILE *f = fopen(argv[1], "r");
	if(!f) {
		perror("fopen");
		return 1;
	}

	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);

	bool inside_script = false;
	char *data = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fileno(f), 0);
	saxxy_parser_init(parser, data, len);
	saxxy_parser_set_token_handler(parser, &script_handler, &inside_script);
	saxxy_html_parse(parser);
	saxxy_parser_free(parser);
	munmap(data, len);
	fclose(f);
	return 0;
}
