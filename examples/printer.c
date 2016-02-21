#include <saxxy.h>

#include <string.h>
#include <stdio.h>
#include <sys/mman.h>

bool token_printer(const saxxy_token_t *token, void __attribute__ ((unused)) *user_handle) {
	size_t i, j;
	switch(token->type) {
	case SAXXY_TOKEN_TAG_OPEN:
	case SAXXY_TOKEN_TAG_CLOSE:
		if(token->type == SAXXY_TOKEN_TAG_OPEN) {
			printf("tag(%lu): <", token->data.tag.name.len);
		} else {
			printf("tag(%lu): </", token->data.tag.name.len);
		}
		fwrite(token->data.tag.name.ptr, token->data.tag.name.len, 1, stdout);
		if(token->data.tag.empty) {
			fwrite("/>", strlen("/>"), 1, stdout);
		} else {
			fwrite(">", strlen(">"), 1, stdout);
		}
		fwrite("\n", 1, 1, stdout);
		for(i = 0; i < token->data.tag.attributes.count; i++) {
			if(token->data.tag.attributes.ptr[i].name.ptr) {
				printf("\tattribute_name(%lu): ", token->data.tag.attributes.ptr[i].name.len);
				fwrite(token->data.tag.attributes.ptr[i].name.ptr, token->data.tag.attributes.ptr[i].name.len, 1, stdout);
				fwrite("\n", strlen("\n"), 1, stdout);
			}

			if(token->data.tag.attributes.ptr[i].value.ptr) {
				if(SAXXY_STRCASECMP_STATIC("style", token->data.tag.attributes.ptr[i].name) && token->data.tag.attributes.ptr[i].value.ptr) {
					saxxy_attribute_array_t style_attributes;
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
					saxxy_attribute_array_clean(&style_attributes);
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

	case SAXXY_TOKEN_ENTITY:
		switch(token->data.entity.type) {
		case SAXXY_ENTITY_TYPE_NAME:
			printf("entity_name(%lu): ", token->data.entity.name.len);
			fwrite(token->data.entity.name.ptr, token->data.entity.name.len, 1, stdout);
			fwrite("\n", 1, 1, stdout);
		break;

		case SAXXY_ENTITY_TYPE_DECIMAL:
			printf("entity_decimal(%lu): ", token->data.entity.decimal.len);
			fwrite(token->data.entity.decimal.ptr, token->data.entity.decimal.len, 1, stdout);
			fwrite("\n", 1, 1, stdout);
		break;

		case SAXXY_ENTITY_TYPE_HEXADECIMAL:
			printf("entity_hexadecimal(%lu): ", token->data.entity.hexadecimal.len);
			fwrite(token->data.entity.hexadecimal.ptr, token->data.entity.hexadecimal.len, 1, stdout);
			fwrite("\n", 1, 1, stdout);
		break;

		default:
			abort();
		}
	break;

	default:
		abort();
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

	char *data = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fileno(f), 0);
	saxxy_parser_init(parser, data, len);
	saxxy_parser_set_token_handler(parser, &token_printer, NULL);
	saxxy_html_parse(parser);
	saxxy_parser_free(parser);
	munmap(data, len);
	fclose(f);
	return 0;
}

