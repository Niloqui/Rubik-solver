#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// https://github.com/jean553/c-simd-avx2-example
//#include <immintrin.h>

#include "face.h"
#include "move.h"
#include "cube.h"

/* 
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
};

fast_move_t get_fast_from_move(move_t move){
    return move.face*3 + move.rotation-1 -(move.face>exchange?3:0);
}
*/

typedef enum {
    UFR, UFL, UBR, UBL, DFR, DFL, DBR, DBL, CORNER_LAST,
    URF = UFR,
    ULB = UBL,
    DLF = DFL,
    DRB = DBR,
} corners_e;

typedef enum {
    UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR, EDGE_LAST
} edges_e;

/* 
typedef struct {
    face_t color;
    // char orientation;
} center_t;
*/

typedef struct {
    edges_e piece;
    char orientation; // 0 or 1
} edge_t;

typedef struct {
    corners_e piece;
    char orientation; // 0, 1 or 2
} corner_t;

typedef struct {
    face_t colors[6];
    char orientation;
    
    char piece_type;
    /* 1 = center
     * 2 = edge
     * 3 = corner
     */
} cubie_t;

typedef struct {
    // UFR, UFL, UBR, UBL, DFR, DFL, DBR, DBL
    corner_t corners[8];
    
    // UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR
    edge_t edges[12];
} cube3_t;

// TO-DO: find a better solution to this
face_t edge_face_table[][2] = {
    [UR] = {U,R},
    [UF] = {U,F},
    [UL] = {U,L},
    [UB] = {U,B},
    [DR] = {D,R},
    [DF] = {D,F},
    [DL] = {D,L},
    [DB] = {D,B},
    [FR] = {F,R},
    [FL] = {F,L},
    [BL] = {B,L},
    [BR] = {B,R},
};

face_t corners_face_table[][3] = {
    [UFR] = {U,R,F},
    [UFL] = {U,F,L},
    [UBR] = {U,B,R},
    [UBL] = {U,L,B},
    [DFR] = {D,F,R},
    [DFL] = {D,L,F},
    [DBR] = {D,R,B},
    [DBL] = {D,B,L},
};

cube3_t create_cube3(cube_t face_cube){
    // This function assumes that face_cube has num_layers == 3 and
    // that the cube is in the correct orientation (U in U, F in F)
    
    cube3_t cube;
    
    face_t corners[8][3];
    face_t edges[12][3];
    
    
    { // Oh God (edges)
        edges[UL][1] = face_cube.faces[L*9 + 1];
        edges[BL][1] = face_cube.faces[L*9 + 3];
        edges[FL][1] = face_cube.faces[L*9 + 5];
        edges[DL][1] = face_cube.faces[L*9 + 7];
        
        edges[UR][1] = face_cube.faces[R*9 + 1];
        edges[FR][1] = face_cube.faces[R*9 + 3];
        edges[BR][1] = face_cube.faces[R*9 + 5];
        edges[DR][1] = face_cube.faces[R*9 + 7];
        
        edges[UB][1] = face_cube.faces[B*9 + 1];
        edges[BR][0] = face_cube.faces[B*9 + 3];
        edges[BL][0] = face_cube.faces[B*9 + 5];
        edges[DB][1] = face_cube.faces[B*9 + 7];
        
        edges[UF][1] = face_cube.faces[F*9 + 1];
        edges[FL][0] = face_cube.faces[F*9 + 3];
        edges[FR][0] = face_cube.faces[F*9 + 5];
        edges[DF][1] = face_cube.faces[F*9 + 7];
        
        edges[DF][0] = face_cube.faces[D*9 + 1];
        edges[DL][0] = face_cube.faces[D*9 + 3];
        edges[DR][0] = face_cube.faces[D*9 + 5];
        edges[DB][0] = face_cube.faces[D*9 + 7];
        
        edges[UB][0] = face_cube.faces[U*9 + 1];
        edges[UL][0] = face_cube.faces[U*9 + 3];
        edges[UR][0] = face_cube.faces[U*9 + 5];
        edges[UF][0] = face_cube.faces[U*9 + 7];
    }
    
    { // Oh God (corners)
        corners[UBL][1] = face_cube.faces[L*9 + 0];
        corners[UFL][2] = face_cube.faces[L*9 + 2];
        corners[DBL][2] = face_cube.faces[L*9 + 6];
        corners[DFL][1] = face_cube.faces[L*9 + 8];
        
        corners[UFR][1] = face_cube.faces[R*9 + 0];
        corners[UBR][2] = face_cube.faces[R*9 + 2];
        corners[DFR][2] = face_cube.faces[R*9 + 6];
        corners[DBR][1] = face_cube.faces[R*9 + 8];
        
        corners[UBR][1] = face_cube.faces[B*9 + 0];
        corners[UBL][2] = face_cube.faces[B*9 + 2];
        corners[DBR][2] = face_cube.faces[B*9 + 6];
        corners[DBL][1] = face_cube.faces[B*9 + 8];
        
        corners[UFL][1] = face_cube.faces[F*9 + 0];
        corners[UFR][2] = face_cube.faces[F*9 + 2];
        corners[DFL][2] = face_cube.faces[F*9 + 6];
        corners[DFR][1] = face_cube.faces[F*9 + 8];
        
        corners[DFL][0] = face_cube.faces[D*9 + 0];
        corners[DFR][0] = face_cube.faces[D*9 + 2];
        corners[DBL][0] = face_cube.faces[D*9 + 6];
        corners[DBR][0] = face_cube.faces[D*9 + 8];
        
        corners[UBL][0] = face_cube.faces[U*9 + 0];
        corners[UBR][0] = face_cube.faces[U*9 + 2];
        corners[UFL][0] = face_cube.faces[U*9 + 6];
        corners[UFR][0] = face_cube.faces[U*9 + 8];
    }
    
    for(edges_e i=0; i<EDGE_LAST; i++){
        face_t face_1 = edges[i][0];
        face_t face_2 = edges[i][1];
        
        if(face_1 == D || face_1 == U){
            cube.edges[i].orientation = 0;
        }
        else if(face_1 == R || face_1 == L){
            cube.edges[i].orientation = 1;
        }
        else{ // face_1 == F || face_1 == B
            if(face_2 == D || face_2 == U){
                cube.edges[i].orientation = 1;
            }
            else{
                cube.edges[i].orientation = 0;
            }
        }
        
        for(edges_e j=0; j<EDGE_LAST; j++){
            
            if((edge_face_table[j][0] == face_1 && edge_face_table[j][1] == face_2)
                    || (edge_face_table[j][1] == face_1 && edge_face_table[j][0] == face_2)){
                cube.edges[i].piece = j;
            }
        }
    }
    
    for(corners_e i=0; i<CORNER_LAST; i++){
        corners_e corner=0;
        
        for(char j=0; j<3; j++){
            face_t face = corners[i][j];
            
            if(face == U){
                cube.corners[i].orientation = j;
            }
            else if(face == D){
                cube.corners[i].orientation = j;
                corner += 4;
            }
            else if(face == B){
                corner += 2;
            }
            else if(face == L){
                corner += 1;
            }
        }
        
        cube.corners[i].piece = corner;
    }
    
    return cube;
}

int is_cube3_solved(const cube3_t cube3){
    for(edges_e i=0; i<EDGE_LAST; i++){
        if(cube3.edges[i].orientation != 0 || cube3.edges[i].piece != i){
            return 0;
        }
    }
    
    for(corners_e i=0; i<CORNER_LAST; i++){
        if(cube3.corners[i].orientation != 0 || cube3.corners[i].piece != i){
            return 0;
        }
    }
    
    return 1;
}

char* get_one_line_str_from_cube3(const cube3_t cube, int use_num_flag){
    char *str;
    
    // 1024 characters should be more than enough for this,
    // so no checks on out of bounds writes :)
    str = (char*)malloc(1024);
    if(str == NULL){
        printf_s("Error in memory allocation.\n");
        return str; // return NULL
    }
    
    char buff[64];
    int pos=0;
    
    for(corners_e i=0; i<CORNER_LAST; i++){
        if(use_num_flag){
            sprintf_s(buff, 64, "%i-%i, ", cube.corners[i].piece, cube.corners[i].orientation);
        }
        else{
            sprintf_s(buff, 64, "%c%c%c-%i, ", 
                get_char_from_facet(corners_face_table[cube.corners[i].piece][0]),
                get_char_from_facet(corners_face_table[cube.corners[i].piece][1]),
                get_char_from_facet(corners_face_table[cube.corners[i].piece][2]),
                cube.corners[i].orientation);
        }
        
        for(int i=0; buff[i]; i++){
            str[pos] = buff[i];
            pos++;
        }
    }
    
    pos -= 2;
    pos += sprintf_s(str+pos, 1024-pos, " --- ");
    
    for(edges_e i=0; i<EDGE_LAST; i++){
        if(use_num_flag){
            sprintf_s(buff, 64, "%2i-%i, ", cube.edges[i].piece, cube.edges[i].orientation);
        }
        else{
            sprintf_s(buff, 64, "%c%c-%i, ", 
                get_char_from_facet(edge_face_table[cube.edges[i].piece][0]),
                get_char_from_facet(edge_face_table[cube.edges[i].piece][1]),
                cube.edges[i].orientation);
        }
        
        for(int i=0; buff[i]; i++){
            str[pos] = buff[i];
            pos++;
        }
    }
    
    pos -= 2;
    str[pos] = '\0';
    
    return str;
}

corners_e corners_move_table[][4] = {
                /* R  */  UFR, UBR, DBR, DFR,
                /* U  */  UFR, UFL, UBL, UBR,
                /* F  */  UFL, UFR, DFR, DFL,
                /* ex */  UFR, UFR, UFR, UFR,
                /* L  */  UBL, UFL, DFL, DBL,
                /* D  */  DFL, DFR, DBR, DBL,
                /* B  */  UBR, UBL, DBL, DBR,
};
#define cmt corners_move_table

edges_e edges_move_table[][4] = {
    /* R  */  UR, BR, DR, FR,
    /* U  */  UR, UF, UL, UB,
    /* F  */  UF, FR, DF, FL,
    /* ex */  UR, UR, UR, UR,
    /* L  */  UL, FL, DL, BL,
    /* D  */  DL, DF, DB, DL,
    /* B  */  UB, BL, DB, BR,
};
#define emt edges_move_table


cube3_t turn_layer_cube3(cube3_t cube3, const move_t move){
    // Moves with multiple layers will be treated as single layer moves
    // e.g. Rw -> R
    // Moves with 0 layers will still be ignored
    
    if(move.layers == 0){
        return cube3;
    }
    
    //fast_move_t fast_m = get_fast_from_move(move);
    edge_t edge;
    corner_t corner;
    face_t face = move.face;
    
    // Move pieces in their new position
    switch (move.rotation){
    case 1:
        edge = cube3.edges[emt[face][3]];
        cube3.edges[emt[face][3]] = cube3.edges[emt[face][2]];
        cube3.edges[emt[face][2]] = cube3.edges[emt[face][1]];
        cube3.edges[emt[face][1]] = cube3.edges[emt[face][0]];
        cube3.edges[emt[face][0]] = edge;
        
        corner = cube3.corners[cmt[face][3]];
        cube3.corners[cmt[face][3]] = cube3.corners[cmt[face][2]];
        cube3.corners[cmt[face][2]] = cube3.corners[cmt[face][1]];
        cube3.corners[cmt[face][1]] = cube3.corners[cmt[face][0]];
        cube3.corners[cmt[face][0]] = corner;
        
        break;
    case 2:
        edge = cube3.edges[emt[face][0]];
        cube3.edges[emt[face][0]] = cube3.edges[emt[face][2]];
        cube3.edges[emt[face][2]] = edge;
        
        edge = cube3.edges[emt[face][1]];
        cube3.edges[emt[face][1]] = cube3.edges[emt[face][3]];
        cube3.edges[emt[face][3]] = edge;
        
        corner = cube3.corners[cmt[face][0]];
        cube3.corners[cmt[face][0]] = cube3.corners[cmt[face][2]];
        cube3.corners[cmt[face][2]] = corner;
        
        corner = cube3.corners[cmt[face][1]];
        cube3.corners[cmt[face][1]] = cube3.corners[cmt[face][3]];
        cube3.corners[cmt[face][3]] = corner;
        
        break;
    case 3:
        edge = cube3.edges[emt[face][0]];
        cube3.edges[emt[face][0]] = cube3.edges[emt[face][1]];
        cube3.edges[emt[face][1]] = cube3.edges[emt[face][2]];
        cube3.edges[emt[face][2]] = cube3.edges[emt[face][3]];
        cube3.edges[emt[face][3]] = edge;
        
        corner = cube3.corners[cmt[face][0]];
        cube3.corners[cmt[face][0]] = cube3.corners[cmt[face][1]];
        cube3.corners[cmt[face][1]] = cube3.corners[cmt[face][2]];
        cube3.corners[cmt[face][2]] = cube3.corners[cmt[face][3]];
        cube3.corners[cmt[face][3]] = corner;
        
        break;
    default: // case 0
        break;
    }
    
    // Twisting the pieces depending of the new position
    if(move.rotation % 2 == 1){
        if(face != U && face != D){
            for(int i=0; i<4; i++){
                cube3.corners[cmt[face][i]].orientation =
                        (cube3.corners[cmt[face][i]].orientation + (i%2?1:2)) % 3;
            }
        }
        if(face == F || face == B){
            for(int i=0; i<4; i++){
                cube3.edges[emt[face][i]].orientation =
                        (cube3.edges[emt[face][i]].orientation + 1) % 2;
            }
        }
    }
    
    return cube3;
}

cube3_t apply_seq_to_cube3(cube3_t cube3, const moves_seq_t seq){
    for(int i=0; i<seq.len; i++){
        cube3 = turn_layer_cube3(cube3, seq.moves[i]);
    }
    
    return cube3;
}

cube3_t apply_inverse_seq_to_cube3(cube3_t cube3, const moves_seq_t seq){
    for(int i=seq.len-1; i>=0; i--){
        move_t move = seq.moves[i];
        move.rotation = (move.rotation * 3) % 4;
        cube3 = turn_layer_cube3(cube3, move);
    }
    
    return cube3;
}


void stress_test_cube3(cube3_t cube3, const char *str, int repetition){
    struct timespec start, end, diff;
    
    moves_seq_t seq = get_move_seq_from_str(3, str);
    
    timespec_get(&start, TIME_UTC);
    for(int i=0; i<repetition; i++){
        cube3 = apply_seq_to_cube3(cube3, seq);
    }
    timespec_get(&end, TIME_UTC);
    
    diff = compute_timespec_difference(end, start);
    printf_s("\nIt took %u.%09u seconds to apply \"%s\" %d times (cube3_t).\n",
            diff.tv_sec, diff.tv_nsec, str, repetition
    );
    
    free(seq.moves);
}





/* TO-DO

char* get_str_from_cube(const cube_t cube)





*/





