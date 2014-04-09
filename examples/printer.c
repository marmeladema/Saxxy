#include <saxxy.h>

#include <string.h>
#include <stdio.h>
#include <sys/mman.h>

void token_printer(const saxxy_token *token, void __attribute__ ((unused)) *user_handle) {
	size_t i;
	switch(token->type) {
		case SAXXY_TOKEN_TAG:
			if(token->data.tag.type == SAXXY_TAG_OPEN) {
				printf("tag(%lu): <", token->data.tag.name.len);
			} else {
				printf("tag(%lu): </", token->data.tag.name.len);
			}
			fwrite(token->data.tag.name.ptr, token->data.tag.name.len, 1, stdout);
			fwrite(">", strlen(">"), 1, stdout);
			fwrite("\n", 1, 1, stdout);
			for(i = 0; i < token->data.tag.attributes.count; i++) {
				printf("attribute_name(%lu): ", token->data.tag.attributes.ptr[i].name.len);
				fwrite(token->data.tag.attributes.ptr[i].name.ptr, token->data.tag.attributes.ptr[i].name.len, 1, stdout);
				fwrite("\n", strlen("\n"), 1, stdout);
			
				printf("attribute_value(%lu): ", token->data.tag.attributes.ptr[i].value.len);
				fwrite(token->data.tag.attributes.ptr[i].value.ptr, token->data.tag.attributes.ptr[i].value.len, 1, stdout);
				fwrite("\n", strlen("\n"), 1, stdout);
			}
		break;
		
		case SAXXY_TOKEN_TEXT:
			printf("text(%lu): ", token->data.text.len);
			fwrite(token->data.text.ptr, token->data.text.len, 1, stdout);
			fwrite("\n", 1, 1, stdout);
		break;
		
		case SAXXY_TOKEN_COMMENT:
			fwrite("comment: ", strlen("comment: "), 1, stdout);
			fwrite(token->data.comment.ptr, token->data.comment.len, 1, stdout);
			fwrite("\n", 1, 1, stdout);
		break;
		
		default:
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
	
	parser.data = mmap(NULL, parser.len, PROT_READ, MAP_PRIVATE, fileno(f), 0);
	parser.token_handler = &token_printer;
	saxxy_html_parse(&parser);
	
	return 0;
}

