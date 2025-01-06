#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/timeb.h>

typedef enum {
    R = 0,
    U = R+1,
    F = R+2,
    exchange = R+3,
    L = R+4,
    D = U+4,
    B = F+4,
    empty = exchange+4,
} face_t;

const face_t faces[] = {R, U, F, L, D, B};

char get_face_char(face_t face){
    switch (face){
    case R:
        return 'R';
    case U:
        return 'U';
    case F:
        return 'F';
    case L:
        return 'L';
    case D:
        return 'D';
    case B:
        return 'B';
    case empty:
        return ' ';
    //case exchange:
    default:
        return '_';
    }
}

face_t get_facet_from_char(char c){
    switch (c){
    case 'R':
        return R;
    case 'U':
        return U;
    case 'F':
        return F;
    case 'L':
        return L;
    case 'D':
        return D;
    case 'B':
        return B;
    case ' ':
        return empty;
    //case '_':
    default:
        return exchange;
    }
}

typedef union {
    struct {
        // How many layers
        // R   -> 1
        // Rw  -> 2
        // 3Rw -> 3
        // x   -> 3 (if the cube is a 3x3)
        unsigned int layers : 3;
        
        // How much the face is turning
        // U  -> 1
        // U2 -> 2
        // U' -> 3
        unsigned int rotation : 2;
        
        face_t face : 3;
    };
    char val;
    unsigned char u_val;
} move_t;

int get_move_axis(move_t move){
    return move.face % 4;
}

typedef struct {
    int num_layers;
    int num_stickers_face;
    int faces_len;
    face_t *faces;
    
} cube_t;

cube_t create_cube(int num_layers){
    cube_t cube;
    cube.num_layers = num_layers;
    cube.num_stickers_face = num_layers*num_layers;
    cube.faces_len = cube.num_stickers_face * (6+1); // 6 faces plus 1 to apply moves
    
    cube.faces = (face_t*)malloc(cube.faces_len * sizeof(face_t));
    if(cube.faces == NULL){
        printf_s("Error in memory allocation.\n");
        return cube;
    }
    
    for(int i=0; i<7; i++){
        //char face_val = get_face_char(i);
        
        for(int j=0; j<cube.num_stickers_face; j++){
            cube.faces[i*cube.num_stickers_face + j] = i; // face_val
        }
    }
    
    return cube;
}

char* cube_to_formatted_string(const cube_t cube){
    char *str;
    face_t layers[] = {empty, U, empty, empty,
                           L, F, R, B,
                       empty, D, empty, empty};
    
    str = (char*)malloc((cube.num_stickers_face*12 + cube.num_layers*3 + 1) * sizeof(char));
    if(str == NULL){
        printf_s("Error in memory allocation.\n");
        return str; // return NULL
    }
    
    int ins=0;
    for(int layer=0; layer<3; layer++){
        for(int row=0; row<cube.num_layers; row++){
            for(int face=0; face<4; face++){
                face_t current_face = layers[layer*4 + face];
                
                for(int col=0; col<cube.num_layers; col++){
                    if(current_face == empty){
                        str[ins] = ' ';
                    }
                    else{
                        str[ins] = get_face_char(
                            cube.faces[current_face*cube.num_stickers_face + row*cube.num_layers + col]
                        );
                    }
                    
                    ins++;
                }
            }
            
            str[ins] = '\n';
            ins++;
        }
    }
    str[ins] = '\0';
    
    return str;
}

char* cube_to_one_line_string(const cube_t cube){
    char *str;
    
    str = malloc(cube.num_stickers_face*6 + 1); // last character must be '\0'
    if(str == NULL){
        printf_s("Error in memory allocation.\n");
        return str; // return NULL
    }
    
    face_t val;
    int ins=0;
    for(int i=0; i<cube.faces_len; i++){
        val = cube.faces[i];
        
        if(val != empty && val != exchange){
            str[ins] = get_face_char(val);
            ins++;
        }
    }
    str[ins] = '\0';
    
    return str;
}

/*
void rotate_single_face(cube_t cube, face_t face, unsigned char rotation){
    // Questo funziona sole per il 3x3
    rotation = rotation % 4;
    
    switch (rotation){
    case 1:
        cube.faces[exchange*cube.num_stickers_face + 0] = cube.faces[face*cube.num_stickers_face + 0];
        cube.faces[exchange*cube.num_stickers_face + 1] = cube.faces[face*cube.num_stickers_face + 1];
        
        cube.faces[face*cube.num_stickers_face + 0] = cube.faces[face*cube.num_stickers_face + 6];
        cube.faces[face*cube.num_stickers_face + 1] = cube.faces[face*cube.num_stickers_face + 3];
        
        cube.faces[face*cube.num_stickers_face + 6] = cube.faces[face*cube.num_stickers_face + 8];
        cube.faces[face*cube.num_stickers_face + 3] = cube.faces[face*cube.num_stickers_face + 7];
        
        cube.faces[face*cube.num_stickers_face + 8] = cube.faces[face*cube.num_stickers_face + 2];
        cube.faces[face*cube.num_stickers_face + 7] = cube.faces[face*cube.num_stickers_face + 5];
        
        cube.faces[face*cube.num_stickers_face + 2] = cube.faces[exchange*cube.num_stickers_face + 0];
        cube.faces[face*cube.num_stickers_face + 5] = cube.faces[exchange*cube.num_stickers_face + 1];
        
        cube.faces[exchange*cube.num_stickers_face + 0] = exchange;
        cube.faces[exchange*cube.num_stickers_face + 1] = exchange;
        break;
    case 2:
        cube.faces[exchange*cube.num_stickers_face + 0] = cube.faces[face*cube.num_stickers_face + 0];
        cube.faces[exchange*cube.num_stickers_face + 1] = cube.faces[face*cube.num_stickers_face + 1];
        cube.faces[exchange*cube.num_stickers_face + 2] = cube.faces[face*cube.num_stickers_face + 2];
        cube.faces[exchange*cube.num_stickers_face + 5] = cube.faces[face*cube.num_stickers_face + 5];
        
        cube.faces[face*cube.num_stickers_face + 0] = cube.faces[face*cube.num_stickers_face + 8];
        cube.faces[face*cube.num_stickers_face + 1] = cube.faces[face*cube.num_stickers_face + 7];
        cube.faces[face*cube.num_stickers_face + 2] = cube.faces[face*cube.num_stickers_face + 6];
        cube.faces[face*cube.num_stickers_face + 5] = cube.faces[face*cube.num_stickers_face + 3];
        
        cube.faces[face*cube.num_stickers_face + 8] = cube.faces[exchange*cube.num_stickers_face + 0];
        cube.faces[face*cube.num_stickers_face + 7] = cube.faces[exchange*cube.num_stickers_face + 1];
        cube.faces[face*cube.num_stickers_face + 6] = cube.faces[exchange*cube.num_stickers_face + 2];
        cube.faces[face*cube.num_stickers_face + 3] = cube.faces[exchange*cube.num_stickers_face + 5];
        
        cube.faces[exchange*cube.num_stickers_face + 0] = exchange;
        cube.faces[exchange*cube.num_stickers_face + 1] = exchange;
        cube.faces[exchange*cube.num_stickers_face + 2] = exchange;
        cube.faces[exchange*cube.num_stickers_face + 5] = exchange;
        break;
    case 3:
        cube.faces[exchange*cube.num_stickers_face + 0] = cube.faces[face*cube.num_stickers_face + 0];
        cube.faces[exchange*cube.num_stickers_face + 1] = cube.faces[face*cube.num_stickers_face + 1];
        
        cube.faces[face*cube.num_stickers_face + 0] = cube.faces[face*cube.num_stickers_face + 2];
        cube.faces[face*cube.num_stickers_face + 1] = cube.faces[face*cube.num_stickers_face + 5];
        
        cube.faces[face*cube.num_stickers_face + 2] = cube.faces[face*cube.num_stickers_face + 8];
        cube.faces[face*cube.num_stickers_face + 5] = cube.faces[face*cube.num_stickers_face + 7];
        
        cube.faces[face*cube.num_stickers_face + 8] = cube.faces[face*cube.num_stickers_face + 6];
        cube.faces[face*cube.num_stickers_face + 7] = cube.faces[face*cube.num_stickers_face + 3];
        
        cube.faces[face*cube.num_stickers_face + 6] = cube.faces[exchange*cube.num_stickers_face + 0];
        cube.faces[face*cube.num_stickers_face + 3] = cube.faces[exchange*cube.num_stickers_face + 1];
        
        cube.faces[exchange*cube.num_stickers_face + 0] = exchange;
        cube.faces[exchange*cube.num_stickers_face + 1] = exchange;
        break;
    case 0: // No rotation
        break;
    }
}
*/
void rotate_single_face(cube_t cube, const move_t move){
    face_t tmp;
    face_t *face_matrix;
    int n_i, n_j;
    int n=cube.num_layers;
    
    face_t face = move.face;
    unsigned char rotation = move.rotation;
    
    face_matrix = cube.faces + face*cube.num_stickers_face;
    rotation = rotation % 4;
    
    switch (rotation){
    case 1:
        n_i = cube.num_layers / 2;
        n_j = (cube.num_layers + 1) / 2;
        
        for(int i=0; i<n_i; i++){
            for(int j=0; j<n_j; j++){
                tmp = face_matrix[(i)*n + (j)];
                face_matrix[(i)*n + (j)] = face_matrix[(n-1-j)*n + (i)];
                face_matrix[(n-1-j)*n + (i)] = face_matrix[(n-1-i)*n + (n-1-j)];
                face_matrix[(n-1-i)*n + (n-1-j)] = face_matrix[(j)*n + (n-1-i)];
                face_matrix[(j)*n + (n-1-i)] = tmp; // face_matrix[(i)*n + (j)];
            }
        }
        break;
    case 2:
        for(int i=0, j=cube.num_stickers_face-1; i<j; i++, j--){
            tmp = face_matrix[i];            
            face_matrix[i] = face_matrix[j];
            face_matrix[j] = tmp;
        }
        break;
    case 3:
        n_i = cube.num_layers / 2;
        n_j = (cube.num_layers + 1) / 2;
        
        for(int i=0; i<n_i; i++){
            for(int j=0; j<n_j; j++){
                tmp = face_matrix[(i)*n + (j)];
                face_matrix[(i)*n + (j)] = face_matrix[(j)*n + (n-1-i)];
                face_matrix[(j)*n + (n-1-i)] = face_matrix[(n-1-i)*n + (n-1-j)];
                face_matrix[(n-1-i)*n + (n-1-j)] = face_matrix[(n-1-j)*n + (i)];
                face_matrix[(n-1-j)*n + (i)] = tmp; // face_matrix[(i)*n + (j)];
            }
        }
        break;
    case 0: // No rotation
        break;
    }
}

const face_t turn_side_order[7][6] = { //   0, 1, 2, 3, 4, 5,
                            /* R  */ exchange, F, U, B, D, exchange,
                            /* U  */ exchange, F, L, B, R, exchange,
                            /* F  */ exchange, U, R, D, L, exchange,
                            /* ex */ exchange, exchange, exchange, exchange, exchange, exchange,
                            /* L  */ exchange, F, D, B, U, exchange,
                            /* D  */ exchange, F, R, B, L, exchange,
                            /* B  */ exchange, U, L, D, R, exchange,
};

/* RRR    000    RR1    RRR    3RR
 * RRR    RRR    RR1    RRR    3RR
 * RRR    RRR    RR1    222    3RR
*/
const unsigned char turn_side_table[7][7] = { //  R,  U,  F, ex,  L,  D,  B,
                                        /* R  */ -1,  1,  1,  0, -1,  1,  3,
                                        /* U  */  0, -1,  0,  0,  0, -1,  0,
                                        /* F  */  3,  2, -1,  0,  1,  0, -1,
                                        /* ex */ -1, -1, -1, -1, -1, -1, -1,
                                        /* L  */ -1,  3,  3,  0, -1,  3,  1,
                                        /* D  */  2, -1,  2,  0,  2, -1,  2,
                                        /* B  */  1,  0, -1,  0,  3,  2, -1,
};

const int turn_side_index_modifier[][6] = { //    i↕, i↔, n_i,  j↕, j↔, n_j,
                                        /* 0 */    1,  0,   0,   0,  1,   0,
                                        /* 1 */    0, -1,   1,   1,  0,   0,
                                        /* 2 */   -1,  0,   1,   0, -1,   1,
                                        /* 3 */    0,  1,   0,  -1,  0,   1,
};

void copy_face_section_to_another_face(cube_t cube, move_t move, 
                                    face_t source_face, face_t dest_face,
                                    int turn_side_index_source, int turn_side_index_dest){
    int i, j;
    int n = cube.num_layers;
    
    // You cannot turn non-existing layers, 
    int layers = move.layers<cube.num_layers?move.layers:cube.num_layers;
    
    face_t *source_face_p, *dest_face_p;
    source_face_p = cube.faces + source_face*cube.num_stickers_face;
    dest_face_p = cube.faces + dest_face*cube.num_stickers_face;
    
    int si_rm, si_cm, sni_m, sj_rm, sj_cm, snj_m;
    int di_rm, di_cm, dni_m, dj_rm, dj_cm, dnj_m;
    { // Initialize the variables above
        si_rm = turn_side_index_modifier[turn_side_index_source][0];
        si_cm = turn_side_index_modifier[turn_side_index_source][1];
        sni_m = turn_side_index_modifier[turn_side_index_source][2];
        
        sj_rm = turn_side_index_modifier[turn_side_index_source][3];
        sj_cm = turn_side_index_modifier[turn_side_index_source][4];
        snj_m = turn_side_index_modifier[turn_side_index_source][5];
        
        di_rm = turn_side_index_modifier[turn_side_index_dest][0];
        di_cm = turn_side_index_modifier[turn_side_index_dest][1];
        dni_m = turn_side_index_modifier[turn_side_index_dest][2];
        
        dj_rm = turn_side_index_modifier[turn_side_index_dest][3];
        dj_cm = turn_side_index_modifier[turn_side_index_dest][4];
        dnj_m = turn_side_index_modifier[turn_side_index_dest][5];
    }
    
    int sc, sr, dc, dr;
    sc = sr = dc = dr = -666;
    
    //printf_s("si_rm = %i\t si_cm = %i\t sni_m = %i\n", si_rm, si_cm, sni_m);
    //printf_s("sj_rm = %i\t sj_cm = %i\t snj_m = %i\n\n\n", sj_rm, sj_cm, snj_m);
    
    for(i=0; i<layers; i++){
        for(j=0; j<n; j++){
            
            // These values adjust the matrix in
            sr = si_rm*(i -(n-1)*sni_m) + sj_rm*(j -(n-1)*snj_m);
            sc = si_cm*(i -(n-1)*sni_m) + sj_cm*(j -(n-1)*snj_m);
            
            dr = di_rm*(i -(n-1)*dni_m) + dj_rm*(j -(n-1)*dnj_m);
            dc = di_cm*(i -(n-1)*dni_m) + dj_cm*(j -(n-1)*dnj_m);
            
            dest_face_p[dr*cube.num_layers + dc] = source_face_p[sr*cube.num_layers + sc];
            
            /*
            printf_s("i = %i\tj = %i\n", i, j);
            printf_s("source_face = %i\n", source_face);
            printf_s("turn_side_index_source = %i\n", turn_side_index_source);
            printf_s("sr = %i\n", sr);
            printf_s("sc = %i\n", sc);
            printf_s("turn_side_index_dest = %i\n", turn_side_index_dest);
            printf_s("dr = %i\n", dr);
            printf_s("dc = %i\n\n\n", dc);
            */
        }
    }
}

void turn_face_side(cube_t cube, const move_t move){
    int index_mod_source, index_mod_dest;
    face_t source_face, dest_face;
    
    face_t face = move.face;
    unsigned char rotation = move.rotation;
    
    int case3 = 0; // false
    switch (rotation){
    case 3:
        case3 = 1; // true
    case 1:
        for(int i=(case3?1:4); i>=0 && i<6; i=i+(case3?1:-1)){
            source_face = turn_side_order[move.face][i];
            dest_face = turn_side_order[move.face][i+(case3?-1:1)];
            
            index_mod_source = turn_side_table[face][source_face];
            index_mod_dest = turn_side_table[face][dest_face];
            
            copy_face_section_to_another_face(cube, move, source_face, dest_face,
                                    index_mod_source, index_mod_dest);
        }
        break;
    case 2:
        const face_t turn_side_order_opposite[2][4] = {
            exchange, turn_side_order[move.face][1], turn_side_order[move.face][3], exchange,
            exchange, turn_side_order[move.face][2], turn_side_order[move.face][4], exchange,
        };
        
        for(int i=0; i<2; i++){
            for(int j=0; j<3; j++){
                source_face = turn_side_order_opposite[i][j+1];
                dest_face = turn_side_order_opposite[i][j];
                
                index_mod_source = turn_side_table[face][source_face];
                index_mod_dest = turn_side_table[face][dest_face];
                
                copy_face_section_to_another_face(cube, move, source_face, dest_face,
                                        index_mod_source, index_mod_dest);
            }
        }
        break;;
    case 0: // No rotation
        break;
    }
    
    // Assign the exchange value to the exchange face
    for(int i=0; i<cube.num_stickers_face; i++){
        cube.faces[exchange*cube.num_stickers_face + i] = exchange;
    }
    
}

void turn_layer(cube_t cube, const move_t move){
    turn_face_side(cube, move);
    rotate_single_face(cube, move);
    
    if(move.layers == cube.num_layers){
        move_t m = move;
        m.face = (m.face + 4) % 8;
        m.rotation = (m.rotation * 3) % 4;
        
        rotate_single_face(cube, m);
    }
}


typedef struct {
    move_t *moves;
    int len;
} moves_seq_t;

int type_of_move_char(char c){
    switch (c){
    // Face moves
    case 'R':
    case 'U':
    case 'F':
    case 'L':
    case 'D':
    case 'B':
        return 1;
    
    // Wide turns
    case 'w':
        return 2;
    
    // Axis rotation
    case 'x':
    case 'y':
    case 'z':
        return 3;
    
    
    // Prime moves
    case '\'':
        return 4;
    
    // Numbers
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return 5;
    
    // Space between moves
    case ' ':
    //case '\t':
        return -1;
    
    default: // Not a valid character for a move
        return 0;
    }
}

move_t get_move_from_substring(const cube_t cube, const char *str, int len){
    move_t move;
    move.face = R;
    move.layers = 1;
    move.rotation = 1;
    
    int face_found = 0;
    
    for(int i=0; i<len; i++){
        //printf_s("%c -- ", str[i]);
        
        switch (type_of_move_char(str[i])){
        case 1: // Face moves
            move.face = get_facet_from_char(str[i]);
            face_found = 1;
            break;
        case 2: // Wide turns
            // A number before the face letter may already indicate the number of layers
            // to move, so we must change the number of layers only if this number is not
            // there.
            if(move.layers == 1){
                move.layers = 2;
            }
            break;
        case 3: // Axis rotation
            switch (str[i]){
            case 'x':
                move.face = R;
                break;
            case 'y':
                move.face = U;
                break;
            case 'z':
                move.face = F;
                break;
            }
            move.layers = cube.num_layers;
            break;
        case 4: // Prime moves
            move.rotation = (move.rotation * 3) % 4;
            break;
        case 5: // Numbers
            if(face_found){ // Rotation number
                move.rotation = (str[i] - '0') % 4;
            }
            else{ // Layers number
                move.layers = str[i] - '0';
            }
            break;
        }
    }
    
    if(move.layers > cube.num_layers){
        move.layers = cube.num_layers;
    }
    
    if(move.layers == cube.num_layers && move.val < 0){
        move.rotation = (move.rotation * 3) % 4;
        move.face = move.face % 4;
    }
    
    //printf_s("| %i . %i . %i . %i\n", move.face, move.layers, move.rotation, move.val);
    return move;
}

moves_seq_t from_str_to_moves_sequence(const cube_t cube, const char *str){
    moves_seq_t seq;
    seq.len = 0;
    seq.moves = NULL;
    
    int i, last_space=-1, str_len=0;
    
    for(i=0; type_of_move_char(str[i]); i++){
        if(str[i] == ' '){
            if(i > last_space+1){
                seq.len++;
            }
            last_space = i;
        }
    }
    if(i > last_space+1){
        seq.len++;
    }
    str_len = i;
    
    if(str_len <= 0){
        printf_s("Invalid sequence.\n");
        return seq;
    }
    
    seq.moves = malloc(sizeof(move_t) * seq.len);
    if(seq.moves == NULL){
        seq.len = 0;
        printf_s("Memory allocation error.\n");
        return seq;
    }
    
    int move_index=0, j;
    move_t move;
    
    last_space = -1;
    // Loop condition includes the \0 at the end (hence why <= instead of just <)
    // That's because the last move would be lost otherwise
    for(i=0; i<=str_len; i++){
        if(type_of_move_char(str[i]) > 0){
            continue;
        }
        
        if(i > last_space+1){
            move = get_move_from_substring(cube, str+last_space+1, i-last_space-1);
            seq.moves[move_index] = move;
            move_index++;
        }
        last_space = i;
    }
    
    return seq;
}

void apply_moves_to_cube(cube_t cube, moves_seq_t seq){
    for(int i=0; i<seq.len; i++){
        turn_layer(cube, seq.moves[i]);
    }
}

void stress_test(cube_t cube, const char *str, int repetition){
    struct timeb start, end;
    
    moves_seq_t seq = from_str_to_moves_sequence(cube, str);
    
    ftime(&start);
    for(int i=0; i<repetition; i++){
        apply_moves_to_cube(cube, seq);
    }
    ftime(&end);
    
    printf("\nIt took %u.%3u seconds to apply \"%s\" %d times.\n",
            end.time - start.time, end.millitm - start.millitm, str, repetition
    );
    
    free(seq.moves);
}





int main(int argc, char **argv) {
    //char scramble[] = "R U2 R' U2 R U2 L' U R' U' L";
    char *scramble, *str;
    cube_t cube = create_cube(3);
    
    if(cube.faces == NULL){
        printf("Error in memory allocation.\n");
        return -1;
    }
    
    scramble = argv[1];
    //moves_seq_t seq = from_str_to_moves_sequence(cube, scramble);
    
    printf_s("\n");
    str = cube_to_one_line_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    str = cube_to_formatted_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    stress_test(cube, scramble, atoi(argv[2]));
    
    printf_s("\n");
    str = cube_to_one_line_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    str = cube_to_formatted_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    
    
    
    /*
    char *str;
    cube_t cube = create_cube(3);
    
    if(cube.faces == NULL){
        printf("Error in memory allocation.\n");
        return -1;
    }
    
    printf_s("\n");
    
    str = cube_to_one_line_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    str = cube_to_formatted_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    */
    
    
    /*
    cube.faces[R*cube.num_stickers_face + 0] = F;
    cube.faces[R*cube.num_stickers_face + 1] = U;
    cube.faces[R*cube.num_stickers_face + 2] = B;
    
    cube.faces[L*cube.num_stickers_face + 0] = B;
    cube.faces[L*cube.num_stickers_face + 1] = U;
    cube.faces[L*cube.num_stickers_face + 2] = F;
    
    str = cube_to_one_line_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    str = cube_to_formatted_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    */
    
    /*
    for(int i=0; i<1; i++){
        printf_s("\n\nRotation = %i\n", rotations[i]);
        
        move.face = L;
        move.layers = 1;
        move.rotation = rotations[i];
        
        turn_layer(cube, move);
        
        str = cube_to_one_line_string(cube);
        printf_s("%s\n\n", str);
        free(str);
        
        str = cube_to_formatted_string(cube);
        printf_s("%s\n\n", str);
        free(str);
    }
    */
    
    /*
    face_t faces_rotation[] =   {R, R, U, R, R, U, R, R, U, R, R, F};
    unsigned char layers[] =    {1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 3};
    unsigned char rotations[] = {2, 2, 1, 2, 2, 2, 2, 2, 1, 2, 2, 1};
    //unsigned char rotations[] = {1, 3, 1, 2};
    //unsigned char layers[] =    {1, 2, 3, 1};
    move_t move;
    
    for(int i=0; i<12; i++){
        move.face = faces_rotation[i];
        move.layers = layers[i];
        move.rotation = rotations[i];
        
        turn_layer(cube, move);
    
        str = cube_to_one_line_string(cube);
        printf_s("%s\n\n", str);
        free(str);
        
        str = cube_to_formatted_string(cube);
        printf_s("%s\n\n", str);
        free(str);
    }
    */
    
    
    //free(cube.faces);
    return 0;
}
































