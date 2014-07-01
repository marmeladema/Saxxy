#include <saxxy.h>

#include <string.h>
#include <stdio.h>
#include <sys/mman.h>

void token_printer(const saxxy_token *token, void __attribute__ ((unused)) *user_handle) {
	size_t i, j;
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
				if(token->data.tag.attributes.ptr[i].name.ptr) {
					printf("\tattribute_name(%lu): ", token->data.tag.attributes.ptr[i].name.len);
					fwrite(token->data.tag.attributes.ptr[i].name.ptr, token->data.tag.attributes.ptr[i].name.len, 1, stdout);
					fwrite("\n", strlen("\n"), 1, stdout);
				}
				
				if(token->data.tag.attributes.ptr[i].value.ptr) {
					if(SAXXY_STRCASECMP_STATIC("style", token->data.tag.attributes.ptr[i].name) && token->data.tag.attributes.ptr[i].value.ptr) {
						saxxy_attribute_array style_attributes;
						memset(&style_attributes, 0, sizeof(style_attributes));
						saxxy_style_parse(&style_attributes, token->data.tag.attributes.ptr[i].value);
						for(j = 0; j < style_attributes.count; j++) {
							if(style_attributes.ptr[j].name.ptr) {
								printf("\t\tstyle_name(%lu): ", style_attributes.ptr[j].name.len);
								fwrite(style_attributes.ptr[j].name.ptr, style_attributes.ptr[j].name.len, 1, stdout);
								fwrite("\n", strlen("\n"), 1, stdout);
							}
							if(style_attributes.ptr[j].value.ptr) {
								printf("\t\tstyle_value(%lu): ", style_attributes.ptr[j].value.len);
								fwrite(style_attributes.ptr[j].value.ptr, style_attributes.ptr[j].value.len, 1, stdout);
								fwrite("\n", strlen("\n"), 1, stdout);
							}
						}
					} else {
						printf("\tattribute_value(%lu): ", token->data.tag.attributes.ptr[i].value.len);
						fwrite(token->data.tag.attributes.ptr[i].value.ptr, token->data.tag.attributes.ptr[i].value.len, 1, stdout);
						fwrite("\n", strlen("\n"), 1, stdout);
					}
				}
			}
		break;

		case SAXXY_TOKEN_TEXT:
			printf("text(%lu): ", token->data.text.len);
			fwrite(token->data.text.ptr, token->data.text.len, 1, stdout);
			fwrite("\n", 1, 1, stdout);
		break;

		case SAXXY_TOKEN_COMMENT:
			printf("comment(%lu): ", token->data.comment.len);
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
	saxxy_parser_clean(&parser);
	fclose(f);
	return 0;
}

