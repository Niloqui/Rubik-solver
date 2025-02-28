#pragma once

#include "face.h"

#define NUM_MOVES 3*NUM_FACES // 18

typedef union {
    // With "unsigned int" the total space is 8 bytes for one move.
    // With "unsigned char" the total space is 1 byte.
    struct {
        // How many layers
        // R   -> 1
        // Rw  -> 2
        // 3Rw -> 3
        // x   -> 3 (if the cube is a 3x3)
        unsigned char layers : 3;
        
        // How much the face is turning
        // U  -> 1
        // U2 -> 2
        // U' -> 3
        unsigned char rotation : 2;
        
        face_t face : 3;
    };
    char val;
    unsigned char u_val;
} move_t;

move_t get_inverse_move(move_t move){
    move.rotation = (move.rotation * 3) % 4;
    return move;
}

int get_axis_from_move(move_t move){
    return move.face % 4;
}

typedef struct {
    move_t *moves;
    int len;
} moves_seq_t;

typedef struct {
    moves_seq_t initial_rotations;
    moves_seq_t solution;
} solution_t;


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
    
    // Slice moves
    case 'M':
    case 'E':
    case 'S':
        return 6;
    
    // Space between moves
    case ' ':
    //case '\t':
        return -1;
    
    default: // Not a valid character for a move
        return 0;
    }
}

move_t get_move_from_substring(int cube_num_layers, const char *str, int len){
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
            move.layers = cube_num_layers;
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
        case 6: // Slice moves
            switch (str[i]){
                case 'M':
                    move.face = L;
                    break;
                case 'E':
                    move.face = D;
                    break;
                case 'S':
                    move.face = F;
                    break;
                }
            move.layers = cube_num_layers - 1;
            face_found = 1;
            break;
        }
    }
    
    if(move.layers > cube_num_layers){
        move.layers = cube_num_layers;
    }
    
    if(move.layers == cube_num_layers && move.val < 0){
        move.rotation = (move.rotation * 3) % 4;
        move.face = move.face % 4;
    }
    
    //printf_s("| %i . %i . %i . %i\n", move.face, move.layers, move.rotation, move.val);
    return move;
}

moves_seq_t get_move_seq_from_str(int cube_num_layers, const char *str){
    moves_seq_t seq;
    seq.len = 0;
    seq.moves = NULL;
    
    int i, last_space=-1, str_len=0, type_of_move, slice_move;
    
    for(i=0; type_of_move_char(str[i]); i++){
        if(str[i] == ' '){
            if(i > last_space+1){
                seq.len++;
            }
            last_space = i;
        }
        
        if(str[i] == 'M' || str[i] == 'E' || str[i] == 'S'){
            seq.len++;
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
    
    slice_move = 0;
    last_space = -1;
    // Loop condition includes the \0 at the end (hence why <= instead of just <)
    // That's because the last move would be lost otherwise
    for(i=0; i<=str_len; i++){
        type_of_move = type_of_move_char(str[i]);
        
        if(type_of_move == 6){ // Slice move
            slice_move = 1;
            continue;
        }
        
        if(type_of_move > 0){ // Substring not completely found yet
            continue;
        }
        
        if(i > last_space+1){
            move = get_move_from_substring(cube_num_layers, str+last_space+1, i-last_space-1);
            seq.moves[move_index] = move;
            move_index++;
            
            if(slice_move){
                move.layers = 1;
                move.rotation = (move.rotation * 3) % 4;
                seq.moves[move_index] = move;
                move_index++;
            }
            
            slice_move = 0;
        }
        last_space = i;
    }
    
    return seq;
}

char* get_str_from_move_seq(int cube_num_layers, moves_seq_t seq){
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
        
        if(move.layers == cube_num_layers){ // Cube rotation
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
            
            buff[i] = get_char_from_facet(move.face);
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
    
    if(j > 0){
        str[j-1] = '\0';
    }
    
    return str;
}



typedef enum {
    move_R, move_R2, move_Rp,
    move_U, move_U2, move_Up,
    move_F, move_F2, move_Fp,
    move_L, move_L2, move_Lp,
    move_D, move_D2, move_Dp,
    move_B, move_B2, move_Bp,
    LAST_FAST_MOVE
} fast_move_t;

move_t fast_to_move_table[] = {
    [move_R]  = {.face = R, .rotation = 1, .layers = 1},
    [move_R2] = {.face = R, .rotation = 2, .layers = 1},
    [move_Rp] = {.face = R, .rotation = 3, .layers = 1},
    [move_U]  = {.face = U, .rotation = 1, .layers = 1},
    [move_U2] = {.face = U, .rotation = 2, .layers = 1},
    [move_Up] = {.face = U, .rotation = 3, .layers = 1},
    [move_F]  = {.face = F, .rotation = 1, .layers = 1},
    [move_F2] = {.face = F, .rotation = 2, .layers = 1},
    [move_Fp] = {.face = F, .rotation = 3, .layers = 1},
    [move_L]  = {.face = L, .rotation = 1, .layers = 1},
    [move_L2] = {.face = L, .rotation = 2, .layers = 1},
    [move_Lp] = {.face = L, .rotation = 3, .layers = 1},
    [move_D]  = {.face = D, .rotation = 1, .layers = 1},
    [move_D2] = {.face = D, .rotation = 2, .layers = 1},
    [move_Dp] = {.face = D, .rotation = 3, .layers = 1},
    [move_B]  = {.face = B, .rotation = 1, .layers = 1},
    [move_B2] = {.face = B, .rotation = 2, .layers = 1},
    [move_Bp] = {.face = B, .rotation = 3, .layers = 1},
    [LAST_FAST_MOVE] = {.face = R, .rotation = 0, .layers = 1},
};

fast_move_t get_fast_from_move(move_t move){
    return move.face*3 + move.rotation-1 -(move.face>exchange?3:0);
}





