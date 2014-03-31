#include <saxxy.h>

#include <string.h>
#include <stdio.h>
#include <sys/mman.h>

void script_handler(const saxxy_token *token, void *user_handle) {
	bool *inside_script = (bool *)user_handle;

	switch(token->type) {
		case SAXXY_TOKEN_TAG:
			if(strlen("script") == token->data.tag.name.len && strncasecmp("script", token->data.tag.name.ptr, token->data.tag.name.len) == 0) {
				if(token->data.tag.type == SAXXY_TAG_OPEN && !token->data.tag.self_closing) {
					*inside_script = true;
				} else {
					*inside_script = false;
				}
			}
		break;
		
		case SAXXY_TOKEN_CHARACTER:
			if(inside_script && *inside_script) {
				fwrite(token->data.character.ptr, token->data.character.len, 1, stdout);
			}
		break;
	}
}

int main(int argc, char *argv[]) {
	saxxy_parser parser;
	memset(&parser, 0, sizeof(parser));
	
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
	parser.len = ftell(f);
	
	bool inside_script = false;
	parser.data = mmap(NULL, parser.len, PROT_READ, MAP_PRIVATE, fileno(f), 0);
	parser.token_handler = &script_handler;
	parser.user_handle = &inside_script;
	saxxy_html_parse(&parser);
	
	return 0;
}
