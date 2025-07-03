#include <stdio.h>
#include <yaml.h>

int main() {
    FILE* file = fopen("data/scenes/ode_test.yaml", "r");
    if (!file) {
        printf("Failed to open file\n");
        return 1;
    }
    
    yaml_parser_t parser;
    yaml_event_t event;
    
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, file);
    
    int done = 0;
    int expecting_value = 0;
    
    while (!done) {
        if (!yaml_parser_parse(&parser, &event)) {
            printf("Parse error\n");
            break;
        }
        
        switch (event.type) {
            case YAML_STREAM_END_EVENT:
                done = 1;
                break;
                
            case YAML_MAPPING_START_EVENT:
                printf("MAPPING START\n");
                expecting_value = 0;
                break;
                
            case YAML_MAPPING_END_EVENT:
                printf("MAPPING END\n");
                break;
                
            case YAML_SEQUENCE_START_EVENT:
                printf("SEQUENCE START\n");
                break;
                
            case YAML_SEQUENCE_END_EVENT:
                printf("SEQUENCE END\n");
                break;
                
            case YAML_SCALAR_EVENT:
                if (!expecting_value) {
                    printf("KEY: %s\n", event.data.scalar.value);
                    expecting_value = 1;
                } else {
                    printf("VALUE: %s\n", event.data.scalar.value);
                    expecting_value = 0;
                }
                break;
                
            default:
                break;
        }
        
        yaml_event_delete(&event);
    }
    
    yaml_parser_delete(&parser);
    fclose(file);
    
    return 0;
}