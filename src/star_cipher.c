#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>

#define STAR_DIM 50
#define STAR_BRIGHT 255
#define STAR_BRIGHT_THRESHOLD 200

// Hash function for password-based randomization
void simple_hash(const char* input, uint32_t* hash_output) {
    uint32_t hash = 5381;
    int c;
    const char* str = input;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    for(int i = 0; i < 8; i++) {
        hash_output[i] = hash ^ (hash >> (i * 4));
        hash = hash * 1103515245 + 12345;
    }
}

// Simple PRNG for shuffling
uint32_t prng_state;
void seed_prng(uint32_t seed) {
    prng_state = seed;
}

uint32_t next_rand() {
    prng_state = prng_state * 1103515245 + 12345;
    return (prng_state / 65536) % 32768;
}

void shuffle_array(int* array, int size) {
    for(int i = size - 1; i > 0; i--) {
        int j = next_rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

// Generate star position combinations
void generate_combinations(int star_count, int positions[][4], int* total_combos) {
    int combo_index = 0;
    
    if(star_count == 1) {
        for(int i = 1; i <= 9; i++) {
            positions[combo_index][0] = i;
            positions[combo_index][1] = 0;
            positions[combo_index][2] = 0;
            positions[combo_index][3] = 0;
            combo_index++;
        }
        *total_combos = 9;
    }
    else if(star_count == 2) {
        for(int i = 1; i <= 9; i++) {
            for(int j = i + 1; j <= 9; j++) {
                positions[combo_index][0] = i;
                positions[combo_index][1] = j;
                positions[combo_index][2] = 0;
                positions[combo_index][3] = 0;
                combo_index++;
            }
        }
        *total_combos = 36;
    }
    else if(star_count == 3) {
        for(int i = 1; i <= 9 && combo_index < 30; i++) {
            for(int j = i + 1; j <= 9 && combo_index < 30; j++) {
                for(int k = j + 1; k <= 9 && combo_index < 30; k++) {
                    positions[combo_index][0] = i;
                    positions[combo_index][1] = j;
                    positions[combo_index][2] = k;
                    positions[combo_index][3] = 0;
                    combo_index++;
                }
            }
        }
        *total_combos = combo_index;
    }
    else if(star_count == 4) {
        for(int i = 1; i <= 9 && combo_index < 20; i++) {
            for(int j = i + 1; j <= 9 && combo_index < 20; j++) {
                for(int k = j + 1; k <= 9 && combo_index < 20; k++) {
                    for(int l = k + 1; l <= 9 && combo_index < 20; l++) {
                        positions[combo_index][0] = i;
                        positions[combo_index][1] = j;
                        positions[combo_index][2] = k;
                        positions[combo_index][3] = l;
                        combo_index++;
                    }
                }
            }
        }
        *total_combos = combo_index;
    }
}

char letters_by_freq[] = "ETAOINSHRDLCUMWFGYPBVKJXQZ0123456789.,!?;:'-\"()[]{}@#$%^&*+=<>/\\|`~_";

typedef struct {
    char letter;
    int star_count;
    int positions[4];
} LetterMapping;

void generate_cipher_table(const char* password, LetterMapping* table, int* table_size) {
    uint32_t hash[8];
    simple_hash(password, hash);
    
    int letter_index = 0;
    int mapping_index = 0;
    
    // Shuffle the entire alphabet using password hash
    char shuffled_letters[100];
    strcpy(shuffled_letters, letters_by_freq);
    int alphabet_size = strlen(shuffled_letters);
    
    seed_prng(hash[0]);
    for(int i = alphabet_size - 1; i > 0; i--) {
        int j = next_rand() % (i + 1);
        char temp = shuffled_letters[i];
        shuffled_letters[i] = shuffled_letters[j];
        shuffled_letters[j] = temp;
    }
    
    // Assign letters to star patterns
    for(int stars = 1; stars <= 4 && letter_index < alphabet_size; stars++) {
        
        int positions[200][4];
        int total_combos;
        generate_combinations(stars, positions, &total_combos);
        
        int indices[200];
        for(int i = 0; i < total_combos; i++) {
            indices[i] = i;
        }
        
        seed_prng(hash[stars]);
        shuffle_array(indices, total_combos);
        
        for(int i = 0; i < total_combos && letter_index < alphabet_size; i++) {
            table[mapping_index].letter = toupper(shuffled_letters[letter_index]);
            table[mapping_index].star_count = stars;
            
            for(int j = 0; j < 4; j++) {
                table[mapping_index].positions[j] = positions[indices[i]][j];
            }
            
            mapping_index++;
            letter_index++;
        }
    }
    
    *table_size = mapping_index;
}

// Calculate optimal image dimensions
void calculate_optimal_dimensions(const char* message, int* grid_size, int* square_size) {
    int char_count = strlen(message);
    
    *grid_size = 1;
    while((*grid_size) * (*grid_size) < char_count) {
        (*grid_size)++;
    }
    
    if(*grid_size <= 5) {
        *square_size = 160;
    } else if(*grid_size <= 10) {
        *square_size = 120;
    } else if(*grid_size <= 15) {
        *square_size = 100;
    } else if(*grid_size <= 20) {
        *square_size = 80;
    } else {
        *square_size = 60;
    }
    
    printf("Message: %d characters\n", char_count);
    printf("Grid: %dx%d (%d squares)\n", *grid_size, *grid_size, (*grid_size) * (*grid_size));
    printf("Square size: %d pixels\n", *square_size);
    printf("Image size: %dx%d pixels\n", (*grid_size) * (*square_size), (*grid_size) * (*square_size));
}

typedef struct {
    int width, height;
    int grid_x, grid_y;
    int square_size;
    uint8_t* pixels;
} StarPhoto;

StarPhoto* create_photo(int grid_x, int grid_y, int square_size) {
    StarPhoto* photo = malloc(sizeof(StarPhoto));
    photo->grid_x = grid_x;
    photo->grid_y = grid_y;
    photo->square_size = square_size;
    photo->width = grid_x * square_size;
    photo->height = grid_y * square_size;
    photo->pixels = calloc(photo->width * photo->height, sizeof(uint8_t));
    return photo;
}

void place_star(StarPhoto* photo, int grid_x, int grid_y, int mini_pos, int brightness) {
    int square_size = photo->square_size;
    int third_square = square_size / 3;
    
    // Convert mini_pos (1-9) to 3x3 grid coordinates
    int mini_row = (mini_pos - 1) / 3;
    int mini_col = (mini_pos - 1) % 3;
    
    int mini_x = mini_col * third_square;
    int mini_y = mini_row * third_square;
    
    int rand_range = third_square / 3;
    int center_offset = third_square / 2;
    
    int x_offset = center_offset + (rand() % rand_range) - rand_range/2;
    int y_offset = center_offset + (rand() % rand_range) - rand_range/2;
    
    int star_x = grid_x * square_size + mini_x + x_offset;
    int star_y = grid_y * square_size + mini_y + y_offset;
    
    if(star_x >= 0 && star_x < photo->width && star_y >= 0 && star_y < photo->height) {
        int pixel_index = star_y * photo->width + star_x;
        photo->pixels[pixel_index] = brightness;
        
        // Add glow effect
        for(int dy = -2; dy <= 2; dy++) {
            for(int dx = -2; dx <= 2; dx++) {
                int glow_x = star_x + dx;
                int glow_y = star_y + dy;
                if(glow_x >= 0 && glow_x < photo->width && glow_y >= 0 && glow_y < photo->height) {
                    int glow_index = glow_y * photo->width + glow_x;
                    int distance = abs(dx) + abs(dy);
                    int glow_brightness = brightness / (1 + distance);
                    if(photo->pixels[glow_index] < glow_brightness) {
                        photo->pixels[glow_index] = glow_brightness;
                    }
                }
            }
        }
    }
}

LetterMapping* find_letter_mapping(char letter, LetterMapping* table, int table_size) {
    char upper_letter = toupper(letter);
    for(int i = 0; i < table_size; i++) {
        if(table[i].letter == upper_letter) {
            return &table[i];
        }
    }
    return NULL;
}

void encode_message(const char* message, const char* password, StarPhoto* photo) {
    LetterMapping cipher_table[150];
    int table_size;
    generate_cipher_table(password, cipher_table, &table_size);
    
    int msg_len = strlen(message);
    int grid_pos = 0;
    
    for(int i = 0; i < msg_len && grid_pos < photo->grid_x * photo->grid_y; i++) {
        char current_char = message[i];
        
        if(current_char == ' ') {
            grid_pos++;
            continue;
        }
        
        LetterMapping* mapping = find_letter_mapping(current_char, cipher_table, table_size);
        if(!mapping) {
            printf("Warning: Character '%c' not found, skipping\n", current_char);
            continue;
        }
        
        int grid_x = grid_pos % photo->grid_x;
        int grid_y = grid_pos / photo->grid_x;
        
        int bright_star = -1;
        if(isupper(current_char)) {
            bright_star = rand() % mapping->star_count;
        }
        
        for(int star = 0; star < mapping->star_count; star++) {
            int brightness = (star == bright_star) ? STAR_BRIGHT : STAR_DIM;
            place_star(photo, grid_x, grid_y, mapping->positions[star], brightness);
        }
        
        grid_pos++;
    }
}

// BMP file structures
#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits;
    uint32_t compression;
    uint32_t imagesize;
    int32_t xresolution;
    int32_t yresolution;
    uint32_t ncolors;
    uint32_t importantcolors;
    uint32_t star_cipher_magic;
    uint32_t grid_size;
    uint32_t square_size;
    uint32_t message_length;
} BMPInfoHeader;
#pragma pack(pop)

void save_photo_bmp(StarPhoto* photo, const char* filename, const char* message) {
    FILE* file = fopen(filename, "wb");
    if(!file) {
        printf("Error: Could not create file %s\n", filename);
        return;
    }
    
    int row_size = ((photo->width * 3 + 3) / 4) * 4;
    int image_size = row_size * photo->height;
    
    BMPHeader header = {0};
    header.type = 0x4D42;
    header.size = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + image_size;
    header.offset = sizeof(BMPHeader) + sizeof(BMPInfoHeader);
    
    BMPInfoHeader info = {0};
    info.size = sizeof(BMPInfoHeader);
    info.width = photo->width;
    info.height = photo->height;
    info.planes = 1;
    info.bits = 24;
    info.compression = 0;
    info.imagesize = image_size;
    info.xresolution = 2835;
    info.yresolution = 2835;
    info.ncolors = 0;
    info.importantcolors = 0;
    info.star_cipher_magic = 0x53544152;
    info.grid_size = photo->grid_x;
    info.square_size = photo->square_size;
    info.message_length = strlen(message);
    
    fwrite(&header, sizeof(BMPHeader), 1, file);
    fwrite(&info, sizeof(BMPInfoHeader), 1, file);
    
    uint8_t padding[3] = {0, 0, 0};
    int padding_size = row_size - photo->width * 3;
    
    for(int y = photo->height - 1; y >= 0; y--) {
        for(int x = 0; x < photo->width; x++) {
            uint8_t pixel = photo->pixels[y * photo->width + x];
            uint8_t color[3] = {pixel, pixel, pixel};
            fwrite(color, 3, 1, file);
        }
        if(padding_size > 0) {
            fwrite(padding, padding_size, 1, file);
        }
    }
    
    fclose(file);
    printf("Image saved as %s\n", filename);
}

// Decoder structures and functions
typedef struct {
    int width, height;
    uint8_t* pixels;
} DecodedPhoto;

typedef struct {
    int mini_square;
    int brightness;
} DetectedStar;

DecodedPhoto* load_bmp(const char* filename, int* grid_size, int* square_size, int* message_length) {
    FILE* file = fopen(filename, "rb");
    if(!file) {
        printf("Error: Could not open file %s\n", filename);
        return NULL;
    }
    
    BMPHeader header;
    BMPInfoHeader info;
    
    fread(&header, sizeof(BMPHeader), 1, file);
    fread(&info, sizeof(BMPInfoHeader), 1, file);
    
    if(header.type != 0x4D42) {
        printf("Error: Not a valid BMP file\n");
        fclose(file);
        return NULL;
    }
    
    if(info.bits != 24) {
        printf("Error: Only 24-bit BMP files supported\n");
        fclose(file);
        return NULL;
    }
    
    if(info.star_cipher_magic == 0x53544152) {
        *grid_size = info.grid_size;
        *square_size = info.square_size;
        *message_length = info.message_length;
        printf("Star Cipher file detected\n");
    } else {
        printf("Warning: Not a Star Cipher file\n");
        fclose(file);
        return NULL;
    }
    
    DecodedPhoto* photo = malloc(sizeof(DecodedPhoto));
    photo->width = info.width;
    photo->height = info.height;
    photo->pixels = malloc(photo->width * photo->height);
    
    int row_size = ((photo->width * 3 + 3) / 4) * 4;
    int padding_size = row_size - photo->width * 3;
    
    fseek(file, header.offset, SEEK_SET);
    
    for(int y = photo->height - 1; y >= 0; y--) {
        for(int x = 0; x < photo->width; x++) {
            uint8_t bgr[3];
            fread(bgr, 3, 1, file);
            photo->pixels[y * photo->width + x] = bgr[0];
        }
        if(padding_size > 0) {
            fseek(file, padding_size, SEEK_CUR);
        }
    }
    
    fclose(file);
    printf("Loaded image: %dx%d pixels\n", photo->width, photo->height);
    return photo;
}

int detect_stars_in_square(DecodedPhoto* photo, int grid_x, int grid_y, int square_size, DetectedStar* stars) {
    int star_count = 0;
    int third_square = square_size / 3;
    
    for(int mini = 1; mini <= 9; mini++) {
        int mini_row = (mini - 1) / 3;
        int mini_col = (mini - 1) % 3;
        
        int mini_x = mini_col * third_square;
        int mini_y = mini_row * third_square;
        
        int max_brightness = 0;
        int start_x = grid_x * square_size + mini_x;
        int start_y = grid_y * square_size + mini_y;
        int end_x = start_x + third_square;
        int end_y = start_y + third_square;
        
        for(int y = start_y; y < end_y && y < photo->height; y++) {
            for(int x = start_x; x < end_x && x < photo->width; x++) {
                int brightness = photo->pixels[y * photo->width + x];
                if(brightness > max_brightness) {
                    max_brightness = brightness;
                }
            }
        }
        
        if(max_brightness > 40) {
            stars[star_count].mini_square = mini;
            stars[star_count].brightness = max_brightness;
            star_count++;
        }
    }
    
    return star_count;
}

int compare_positions(int* pos1, int* pos2, int count) {
    int sorted1[4] = {0}, sorted2[4] = {0};
    for(int i = 0; i < count; i++) {
        sorted1[i] = pos1[i];
        sorted2[i] = pos2[i];
    }
    
    for(int i = 0; i < count-1; i++) {
        for(int j = 0; j < count-1-i; j++) {
            if(sorted1[j] > sorted1[j+1]) {
                int temp = sorted1[j];
                sorted1[j] = sorted1[j+1];
                sorted1[j+1] = temp;
            }
            if(sorted2[j] > sorted2[j+1]) {
                int temp = sorted2[j];
                sorted2[j] = sorted2[j+1];
                sorted2[j+1] = temp;
            }
        }
    }
    
    for(int i = 0; i < count; i++) {
        if(sorted1[i] != sorted2[i]) {
            return 0;
        }
    }
    return 1;
}

char decode_square(DetectedStar* stars, int star_count, LetterMapping* cipher_table, int table_size, int* is_capital) {
    if(star_count == 0) {
        return ' ';
    }
    
    int detected_positions[4] = {0};
    int has_bright_star = 0;
    
    for(int i = 0; i < star_count && i < 4; i++) {
        detected_positions[i] = stars[i].mini_square;
        if(stars[i].brightness > STAR_BRIGHT_THRESHOLD) {
            has_bright_star = 1;
        }
    }
    
    for(int i = 0; i < table_size; i++) {
        if(cipher_table[i].star_count == star_count) {
            if(compare_positions(detected_positions, cipher_table[i].positions, star_count)) {
                *is_capital = has_bright_star;
                return cipher_table[i].letter;
            }
        }
    }
    
    return '?';
}

void decode_message(DecodedPhoto* photo, const char* password, char* decoded_message, int max_length, int grid_size, int square_size, int expected_length) {
    LetterMapping cipher_table[150];
    int table_size;
    generate_cipher_table(password, cipher_table, &table_size);
    
    printf("Using grid: %dx%d, square size: %d pixels\n", grid_size, grid_size, square_size);
    
    int msg_pos = 0;
    int max_chars = (expected_length > 0) ? expected_length : (grid_size * grid_size);
    
    for(int grid_pos = 0; grid_pos < max_chars && msg_pos < max_length - 1; grid_pos++) {
        int grid_x = grid_pos % grid_size;
        int grid_y = grid_pos / grid_size;
        
        DetectedStar stars[9];
        int star_count = detect_stars_in_square(photo, grid_x, grid_y, square_size, stars);
        
        int is_capital;
        char decoded_char = decode_square(stars, star_count, cipher_table, table_size, &is_capital);
        
        if(decoded_char != '\0' && decoded_char != '?') {
            if(decoded_char != ' ' && !is_capital) {
                decoded_char = tolower(decoded_char);
            }
            decoded_message[msg_pos++] = decoded_char;
        } else if(decoded_char == '?') {
            decoded_message[msg_pos++] = '?';
        }
    }
    
    decoded_message[msg_pos] = '\0';
    
    while(msg_pos > 0 && decoded_message[msg_pos-1] == ' ') {
        decoded_message[--msg_pos] = '\0';
    }
}

void free_star_photo(StarPhoto* photo) {
    if(photo) {
        free(photo->pixels);
        free(photo);
    }
}

void free_decoded_photo(DecodedPhoto* photo) {
    if(photo) {
        free(photo->pixels);
        free(photo);
    }
}

// Command line argument structure
typedef struct {
    int mode;           // 1=encode, 2=decode, 0=interactive
    char input_file[256];
    char output_file[256];
    char password[256];
    char message[1000];
    int help;
} Arguments;

void print_help() {
    printf("STAR CIPHER v1.0 - Hide messages in star patterns\n");
    printf("================================================\n\n");
    printf("USAGE:\n");
    printf("  star-cipher [OPTIONS]\n\n");
    printf("OPTIONS:\n");
    printf("  -e, --encode          Encode mode (default if no input file)\n");
    printf("  -d, --decode          Decode mode (default if input file given)\n");
    printf("  -i, --input FILE      Input BMP file to decode\n");
    printf("  -o, --output FILE     Output BMP file (default: encoded.bmp)\n");
    printf("  -p, --password PASS   Password for cipher\n");
    printf("  -m, --message TEXT    Message to encode\n");
    printf("  -h, --help            Show this help\n\n");
    printf("EXAMPLES:\n");
    printf("  star-cipher -e -m \"Hello World\" -p mypass -o secret.bmp\n");
    printf("  star-cipher -d -i secret.bmp -p mypass\n");
    printf("  star-cipher --encode --message \"Top Secret\" --password key123\n");
    printf("  star-cipher --decode --input encrypted.bmp\n\n");
    printf("INTERACTIVE MODE:\n");
    printf("  Run without arguments for interactive menu\n");
}

int parse_arguments(int argc, char* argv[], Arguments* args) {
    // Initialize defaults
    args->mode = 0;
    strcpy(args->output_file, "encoded.bmp");
    args->input_file[0] = '\0';
    args->password[0] = '\0';
    args->message[0] = '\0';
    args->help = 0;
    
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            args->help = 1;
            return 1;
        }
        else if(strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--encode") == 0) {
            args->mode = 1;
        }
        else if(strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--decode") == 0) {
            args->mode = 2;
        }
        else if((strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) && i + 1 < argc) {
            strcpy(args->input_file, argv[++i]);
            if(args->mode == 0) args->mode = 2; // Auto-detect decode mode
        }
        else if((strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) && i + 1 < argc) {
            strcpy(args->output_file, argv[++i]);
        }
        else if((strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--password") == 0) && i + 1 < argc) {
            strcpy(args->password, argv[++i]);
        }
        else if((strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--message") == 0) && i + 1 < argc) {
            strcpy(args->message, argv[++i]);
            if(args->mode == 0) args->mode = 1; // Auto-detect encode mode
        }
        else {
            printf("Error: Unknown argument '%s'\n", argv[i]);
            printf("Use -h or --help for usage information\n");
            return -1;
        }
    }
    
    return 0;
}

void get_password(char* password, int max_len) {
    printf("Password: ");
    fgets(password, max_len, stdin);
    password[strcspn(password, "\n")] = 0;
}

void get_message(char* message, int max_len) {
    printf("Message: ");
    fgets(message, max_len, stdin);
    message[strcspn(message, "\n")] = 0;
}
void run_encoder_cli(Arguments* args) {
    char password[256];
    char message[1000];
    
    // Get password if not provided
    if(strlen(args->password) == 0) {
        get_password(password, sizeof(password));
    } else {
        strcpy(password, args->password);
    }
    
    // Get message if not provided
    if(strlen(args->message) == 0) {
        get_message(message, sizeof(message));
    } else {
        strcpy(message, args->message);
    }
    
    int grid_size, square_size;
    calculate_optimal_dimensions(message, &grid_size, &square_size);
    
    StarPhoto* photo = create_photo(grid_size, grid_size, square_size);
    encode_message(message, password, photo);
    save_photo_bmp(photo, args->output_file, message);
    
    free_star_photo(photo);
    printf("Encoding complete: %s\n", args->output_file);
}

void run_decoder_cli(Arguments* args) {
    char password[256];
    char filename[256];
    
    // Get input filename
    if(strlen(args->input_file) == 0) {
        printf("BMP filename: ");
        fgets(filename, sizeof(filename), stdin);
        filename[strcspn(filename, "\n")] = 0;
    } else {
        strcpy(filename, args->input_file);
    }
    
    // Get password if not provided
    if(strlen(args->password) == 0) {
        get_password(password, sizeof(password));
    } else {
        strcpy(password, args->password);
    }
    
    int grid_size, square_size, message_length;
    DecodedPhoto* photo = load_bmp(filename, &grid_size, &square_size, &message_length);
    if(!photo) {
        return;
    }
    
    char decoded_message[1000];
    decode_message(photo, password, decoded_message, sizeof(decoded_message), grid_size, square_size, message_length);
    
    printf("Decoded message: %s\n", decoded_message);
    
    free_decoded_photo(photo);
}

void run_interactive() {
    printf("STAR CIPHER v1.0\n");
    printf("================\n");
    printf("1. Encode message\n");
    printf("2. Decode message\n");
    printf("3. Exit\n");
    printf("Choice: ");
    
    char choice;
    scanf("%c", &choice);
    getchar();
    
    Arguments args = {0};
    strcpy(args.output_file, "encoded.bmp");
    
    switch(choice) {
        case '1':
            args.mode = 1;
            run_encoder_cli(&args);
            break;
        case '2':
            args.mode = 2;
            run_decoder_cli(&args);
            break;
        case '3':
            printf("Goodbye.\n");
            break;
        default:
            printf("Invalid choice.\n");
            break;
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    
    Arguments args;
    int parse_result = parse_arguments(argc, argv, &args);
    
    if(parse_result == -1) {
        return 1; // Error in arguments
    }
    
    if(args.help) {
        print_help();
        return 0;
    }
    
    // If no arguments provided, run interactive mode
    if(argc == 1) {
        run_interactive();
        return 0;
    }
    
    // Validate mode selection
    if(args.mode == 0) {
        printf("Error: Must specify encode (-e) or decode (-d) mode\n");
        printf("Use -h for help\n");
        return 1;
    }
    
    // Run appropriate mode
    if(args.mode == 1) {
        run_encoder_cli(&args);
    } else if(args.mode == 2) {
        run_decoder_cli(&args);
    }
    
    return 0;
}
