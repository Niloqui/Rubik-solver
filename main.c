#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h> // struct timespec
// #include <windows.h> // Sleep()

struct timespec compute_timespec_difference(struct timespec end, struct timespec start){
    struct timespec diff;
    
    diff.tv_sec = end.tv_sec - start.tv_sec;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec;
    if(diff.tv_nsec < 0){
        diff.tv_nsec += 1000000000; // 999999999 + 1
        diff.tv_sec -= 1;
    }
    
    return diff;
}


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

int get_face_axis(face_t face){
    return face % 4;
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
    //sc = sr = dc = dr = -666;
    
    //printf_s("si_rm = %i\t si_cm = %i\t sni_m = %i\n", si_rm, si_cm, sni_m);
    //printf_s("sj_rm = %i\t sj_cm = %i\t snj_m = %i\n\n\n", sj_rm, sj_cm, snj_m);
    
    for(i=0; i<layers; i++){
        for(j=0; j<n; j++){
            // These values adjust the matrix indexes
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
        case3 = 2; // true
    case 1:
        for(int i=(case3?1:4); i>=0 && i<6; i=i-1+case3){
            source_face = turn_side_order[move.face][i];
            dest_face = turn_side_order[move.face][i+1-case3];
            
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
            face_found = 1;
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

char* from_moves_sequence_to_str(const cube_t cube, moves_seq_t seq){
    int max_len = 256;
    char *str = calloc(max_len, sizeof(char)), *realloc_str;
    if(str == NULL){
        printf_s("Error in memory allocation.\n");
        return NULL;
    }
    
    int i, m, j=0;
    char buff[16];
    
    for(m=0; m<seq.len; m++){
        move_t move = seq.moves[m];
        
        if(move.rotation == 0){
            // There is no reason to write a move that has no rotation.
            continue;
        }
        
        // Convert the move from move_t into buff
        i = 0;
        
        if(move.layers == cube.num_layers){ // Cube rotation
            switch (move.face){
            case R:
                buff[i] = 'x';
                break;
            case U:
                buff[i] = 'y';
                break;
            case F:
                buff[i] = 'z';
                break;
            }
            i++;
        }
        else{ // Normal move
            if(move.layers >= 3){
                buff[i] = '0' + move.layers;
                i++;
            }
            
            buff[i] = get_face_char(move.face);
            i++;
            
            if(move.layers >= 2){
                buff[i] = 'w';
                i++;
            }
        }
        
        switch (move.rotation){ // Adding prime or 2 at the end
        case 2:
            buff[i] = '2';
            i++;
            break;
        case 3:
            buff[i] = '\'';
            i++;
            break;
        case 1:
            break;
        }
        
        buff[i] = ' ';
        buff[i+1] = '\0';
        
        // Copy the move from buff into str
        i=0;
        
        while(buff[i]){
            if(j == max_len-1){
                max_len *= 2;
                realloc_str = realloc(str, max_len);
                if(realloc_str == NULL){
                    printf_s("Error in memory reallocation.\n");
                    free(str);
                    return NULL;
                }
                else{
                    str = realloc_str;
                }
            }
            
            str[j] = buff[i];
            
            j++;
            i++;
        }
    }
    
    str[j] = '\0';
    
    return str;
}


void apply_moves_to_cube(cube_t cube, moves_seq_t seq){
    for(int i=0; i<seq.len; i++){
        turn_layer(cube, seq.moves[i]);
    }
}

void stress_test(cube_t cube, const char *str, int repetition){
    struct timespec start, end, diff;
    
    moves_seq_t seq = from_str_to_moves_sequence(cube, str);
    
    timespec_get(&start, TIME_UTC);
    for(int i=0; i<repetition; i++){
        apply_moves_to_cube(cube, seq);
    }
    timespec_get(&end, TIME_UTC);
    
    diff = compute_timespec_difference(end, start);
    printf_s("\nIt took %u.%09u seconds to apply \"%s\" %d times.\n",
            diff.tv_sec, diff.tv_nsec, str, repetition
    );
    
    free(seq.moves);
}

int is_cube_solved(cube_t cube){
    /* The cube must be in the standard position (F in F, U in U, etc.) 
     * to be considered solved.
     * If the cube is off by a rotation (e.g. "x"), the cube is not considered solved.
    */
    face_t face;
    
    for(int f=0; f<6; f++){
        face = faces[f];
        
        for(int i=0; i<cube.num_stickers_face; i++){
            if(cube.faces[face*cube.num_stickers_face + i] != face){
                return 0;
            }
        }
    }
    return 1;
}

typedef struct {
    moves_seq_t initial_rotations;
    moves_seq_t solution;
} solution_t;

int solve_2x2x2_recursion(cube_t cube, moves_seq_t *seq, int previous_axis,
                        int max_depth, int current_depth){
    int solution_found = 0;
    
    if(current_depth == max_depth){
        solution_found = is_cube_solved(cube);
    }
    else{
        for(face_t face=R; face<=F && !solution_found; face++){
            if(get_face_axis(face) == previous_axis){
                continue; 
            }
            
            move_t move_base;
            move_base.face = face;
            move_base.layers = 1;
            move_base.rotation = 1;
            
            seq->moves[current_depth] = move_base;
            
            for(int rot=1; rot<4 && !solution_found; rot++){
                seq->moves[current_depth].rotation = rot;
                turn_layer(cube, move_base);
                
                solution_found = solve_2x2x2_recursion(cube, seq, get_face_axis(face),
                                                            max_depth, current_depth+1);
            }
            if(!solution_found){
                turn_layer(cube, move_base);
            }
        }
    }
    
    return solution_found;
}

solution_t solve_2x2x2(cube_t cube){
    // This function assumes that cube.num_layers == 2
    int i, j, k;
    int solution_found;
    int max_depth;
    
    solution_t sol;
    sol.initial_rotations.len = 0;
    sol.initial_rotations.moves = NULL;
    sol.solution.len = 0;
    sol.solution.moves = NULL;
    
    /* First of all we need to rotate the cube in such a way that
     * the back bottom left corner is solved.
     *    UU              XX
     *    UU              XX
     *  LLFFRRBB        XXXXXXXX
     *  LLFFRRBB        LXXXXXXB
     *    DD              XX
     *    DD              DX
    */
    sol.initial_rotations.moves = malloc(sizeof(move_t) * 3);
    if(sol.initial_rotations.moves == NULL){
        printf_s("Error in memory allocation: sol.initial_rotations.moves\n");
        return sol;
    }
    sol.initial_rotations.len = 3;
    
    for(i=0; i<3; i++){
        sol.initial_rotations.moves[i].face = faces[i];
        sol.initial_rotations.moves[i].layers = 2;
        sol.initial_rotations.moves[i].rotation = 0;
    }
    
    move_t x, y, z;
    x.face = R;
    y.face = U;
    z.face = F;
    x.rotation = y.rotation = z.rotation = 1;
    x.layers = y.layers = z.layers = 2;
    
    solution_found = 0;
    for(i=0; i<4 && !solution_found; i++){
        sol.initial_rotations.moves[0].rotation = i;
        
        for(j=0; j<4 && !solution_found; j++){
            sol.initial_rotations.moves[1].rotation = j;
            
            for(k=0; k<4 && !solution_found; k++){
                sol.initial_rotations.moves[2].rotation = k;
                
                solution_found = cube.faces[L*cube.num_stickers_face + 2] == L &&
                                 cube.faces[B*cube.num_stickers_face + 3] == B &&
                                 cube.faces[D*cube.num_stickers_face + 2] == D;
                
                if(!solution_found){
                    turn_layer(cube, z);
                }
            }
            
            if(!solution_found){
                turn_layer(cube, y);
            }
        }
        
        if(!solution_found){
            turn_layer(cube, x);
        }
    }
    
    if(!solution_found){
        printf_s("Error: it was not possible to bring the back bottom left corner "
                "into its solved position during the first phase of the algorithm.\n");
        free(sol.initial_rotations.moves);
        sol.initial_rotations.moves = NULL;
        sol.initial_rotations.len = 0;
        return sol;
    }
    
    /* The main part of the algorithm is a recursion function
     * TO-DO: create a table with all the cube states and use that to obtain
     * the solution.
    */
    sol.solution.moves = malloc(sizeof(move_t) * 11); // 11 is the 2x2 God's Number
    if(sol.solution.moves == NULL){
        printf_s("Error in memory allocation: sol.initial_rotations.moves\n");
        return sol;
    }
    //sol.solution.len = 11;
    
    /*
    for(i=0; i<11; i++){
        sol.solution.moves[i].face = D;
        sol.solution.moves[i].layers = 7;
        sol.solution.moves[i].rotation = 0;
    }*/
    
    max_depth = 0;
    
    if(is_cube_solved(cube)){
        solution_found = 1;
    }
    else{
        solution_found = 0;
    }
    
    while(!solution_found){
        max_depth++;
        solution_found = solve_2x2x2_recursion(cube, &sol.solution,
                                            get_face_axis(empty), max_depth, 0);
    }
    sol.solution.len = max_depth;
    
    
    
    return sol;
}



int solve_3x3x3_recursion(cube_t cube, moves_seq_t *seq, int previous_axis,
                        int max_depth, int current_depth){
    // TO-DO
    // TO-DO
    // TO-DO
    // TO-DO
    // TO-DO
    // TO-DO
    // TO-DO
    int solution_found = 0;
    
    if(current_depth == max_depth){
        solution_found = is_cube_solved(cube);
    }
    else{
        for(face_t face=R; face<=F && !solution_found; face++){
            if(get_face_axis(face) == previous_axis){
                continue; 
            }
            
            move_t move_base;
            move_base.face = face;
            move_base.layers = 1;
            move_base.rotation = 1;
            
            seq->moves[current_depth] = move_base;
            
            for(int rot=1; rot<4 && !solution_found; rot++){
                seq->moves[current_depth].rotation = rot;
                turn_layer(cube, move_base);
                
                solution_found = solve_2x2x2_recursion(cube, seq, get_face_axis(face),
                                                            max_depth, current_depth+1);
            }
            if(!solution_found){
                turn_layer(cube, move_base);
            }
        }
    }
    
    return solution_found;
}

solution_t solve_3x3x3(cube_t cube){
    // TO-DO
    // TO-DO
    // TO-DO
    // TO-DO
    // TO-DO
    // TO-DO
    
    
    // This function assumes that cube.num_layers == 3
    int i, j, k;
    int solution_found;
    int max_depth;
    
    solution_t sol;
    sol.initial_rotations.len = 0;
    sol.initial_rotations.moves = NULL;
    sol.solution.len = 0;
    sol.solution.moves = NULL;
    
    /* First of all we need to rotate the cube in such a way that
     * the U face is in U and the F face is in F.
     *     UUU                 XXX
     *     UUU                 XUX
     *     UUU                 XXX
     *  LLLFFFRRRBBB        XXXXXXXXXXXX
     *  LLLFFFRRRBBB        XLXXFXXRXXBX
     *  LLLFFFRRRBBB        XXXXXXXXXXXX
     *     DDD                 XXX
     *     DDD                 XDX
     *     DDD                 XXX
    */
    
    sol.initial_rotations.moves = malloc(sizeof(move_t) * 2);
    if(sol.initial_rotations.moves == NULL){
        printf_s("Error in memory allocation: sol.initial_rotations.moves\n");
        return sol;
    }
    //sol.initial_rotations.len = 2;
    
    for(i=0; i<2; i++){
        sol.initial_rotations.moves[i].face = faces[i];
        sol.initial_rotations.moves[i].layers = 3;
        sol.initial_rotations.moves[i].rotation = 0;
    }
    
    move_t move;
    i = 0;
    
    // First step is moving U in U
    if(cube.faces[U*cube.num_stickers_face + 4] != U){
        // Do something only if U is not already on top
        move.layers = 3;
        
        if(cube.faces[R*cube.num_stickers_face + 4] == U){ // z'
            move.face = F;
            move.rotation = 3;
        }
        else if(cube.faces[L*cube.num_stickers_face + 4] == U){ // z
            move.face = F;
            move.rotation = 1;
        }
        else if(cube.faces[F*cube.num_stickers_face + 4] == U){ // x
            move.face = R;
            move.rotation = 1;
        }
        else if(cube.faces[B*cube.num_stickers_face + 4] == U){ // x'
            move.face = R;
            move.rotation = 3;
        }
        else if(cube.faces[D*cube.num_stickers_face + 4] == U){
            if(cube.faces[F*cube.num_stickers_face + 4] == F){ // z2
                move.face = F;
                move.rotation = 2;
            }
            else{ // x2
                move.face = R;
                move.rotation = 2;
            }
        }
        
        sol.initial_rotations.moves[i] = move;
        turn_layer(cube, move);
        i++;
    }
    
    // Second step is moving F in F
    if(cube.faces[F*cube.num_stickers_face + 4] != F){
        // Do something only if U is not already on top
        move.layers = 3;
        move.face = U;
        
        if(cube.faces[R*cube.num_stickers_face + 4] == F){ // y
            move.rotation = 1;
        }
        else if(cube.faces[B*cube.num_stickers_face + 4] == F){ // y2
            move.rotation = 2;
        }
        //else if(cube.faces[L*cube.num_stickers_face + 4] == F){ // y'
        else{ // y'
            move.rotation = 3;
        }
        
        sol.initial_rotations.moves[i] = move;
        turn_layer(cube, move);
        i++;
    }
    sol.initial_rotations.len = i;
    
    /* The main part of the algorithm is a recursion function
     * TO-DO: create a table with some cube states and use that to help find
     * the solution.
    */
    sol.solution.moves = malloc(sizeof(move_t) * 20); // 20 is the 3x3 God's Number
    if(sol.solution.moves == NULL){
        printf_s("Error in memory allocation: sol.initial_rotations.moves\n");
        return sol;
    }
    //sol.solution.len = 11;
    
    for(i=0; i<20; i++){
        sol.solution.moves[i].face = D;
        sol.solution.moves[i].layers = 7;
        sol.solution.moves[i].rotation = 0;
    }
    
    max_depth = 0;
    solution_found = 0;
    while(!solution_found){
        max_depth++;
        solution_found = solve_3x3x3_recursion(cube, &sol.solution,
                                            get_face_axis(empty), max_depth, 0);
    }
    sol.solution.len = max_depth;
    
    return sol;
}



solution_t solve_cube(cube_t cube){
    solution_t sol;
    sol.initial_rotations.len = 0;
    sol.initial_rotations.moves = NULL;
    sol.solution.len = 0;
    sol.solution.moves = NULL;
    
    if(cube.num_layers < 2){
        printf_s("Cube is too small / Error in the data.\n");
    }
    else if(cube.num_layers >= 4){
        printf_s("Cube is too big (%i number of layers), no solver implemented yet.\n",
                cube.num_layers);
    }
    else if(cube.num_layers == 2){
        sol = solve_2x2x2(cube);
    }
    else if(cube.num_layers == 3){
        // TO-DO
    }
    
    
    
    
    
    
    return sol;
}





void print_separator(){
    printf_s("\n-------------------------------------------------------------\n\n");
}

int main(int argc, char **argv) {
    struct timespec start, end, diff;
    timespec_get(&start, TIME_UTC);
    
    //char scramble[] = "R U2 R' U2 R U2 L' U R' U' L";
    char *scramble, *str;
    cube_t cube = create_cube(3);
    
    
    if(cube.faces == NULL){
        printf("Error in memory allocation.\n");
        return -1;
    }
    
    scramble = argv[1];
    moves_seq_t seq = from_str_to_moves_sequence(cube, scramble);
    
    str = cube_to_one_line_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    str = cube_to_formatted_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    //int repetitions = atoi(argv[2]);
    //int cube_solved = 0;
    
    timespec_get(&end, TIME_UTC);
    
    diff = compute_timespec_difference(end, start);
    printf_s("It took %u.%09u seconds to set things up.\n", diff.tv_sec, diff.tv_nsec);
    
    //////////////////////////////
    print_separator();
    
    
    apply_moves_to_cube(cube, seq);
    
    str = cube_to_one_line_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    str = cube_to_formatted_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    
    solution_t sol;
    
    timespec_get(&start, TIME_UTC);
    sol = solve_cube(cube);
    timespec_get(&end, TIME_UTC);
    
    diff = compute_timespec_difference(end, start);
    printf_s("\nIt took %u.%09u seconds to solve the 2x2.\n",
            diff.tv_sec, diff.tv_nsec
    );
    
    printf_s("\n\n"
            "Scramble: %s (%2i moves)\n"
            "Solution: ", scramble, seq.len);
    
    str = from_moves_sequence_to_str(cube, sol.initial_rotations);
    printf_s("%s", str);
    free(str);
    str = from_moves_sequence_to_str(cube, sol.solution);
    printf_s("%s (%2i moves)\n", str, sol.solution.len);
    free(str);
    
    printf_s("\n");
    str = cube_to_one_line_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    str = cube_to_formatted_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    
    
    
    
    
    
    
    /*
    for(int i=1; i<=repetitions; i++){
        print_separator();
        
        apply_moves_to_cube(cube, seq);
        
        str = cube_to_one_line_string(cube);
        printf_s("%s\n\n", str);
        free(str);
        str = cube_to_formatted_string(cube);
        printf_s("%s\n\n", str);
        free(str);
        
        printf_s("We have applied \"%s\" %d time(s).\n", scramble, i);
        
        timespec_get(&start, TIME_UTC);
        cube_solved = is_cube_solved(cube);
        timespec_get(&end, TIME_UTC);
        
        if(cube_solved){
            printf_s("Cube is SOLVED!\n");
        }
        else{
            printf_s("Cube is not solved.\n");
        }
        diff = compute_timespec_difference(end, start);
        printf_s("It took %u.%09u seconds to check if the cube is solved.\n",
                diff.tv_sec, diff.tv_nsec);
    }
    
    print_separator();
    
    printf_s("\n");
    str = cube_to_one_line_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    str = cube_to_formatted_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    stress_test(cube, scramble, atoi(argv[3]));
    
    printf_s("\n");
    str = cube_to_one_line_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    str = cube_to_formatted_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    */
    
    
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
    
    
    free(cube.faces);
    free(seq.moves);
    
    print_separator();
    print_separator();
    return 0;
}
































