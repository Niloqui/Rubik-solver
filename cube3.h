#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// https://github.com/jean553/c-simd-avx2-example
#include <immintrin.h>

#include "face.h"
#include "move.h"
#include "cube.h"


typedef struct {
    face_t colors[1];
    char orientation;
} center_t;

typedef struct {
    face_t colors[2];
    char orientation;
} edge_t;

typedef struct {
    face_t colors[3];
    char orientation;
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
    int num_layers;
    int num_stickers_face;
    int faces_len;
    
    cubie_t pieces[27];
} cube3_t;


cube3_t create_cube3(cube_t facelet_cube){
    // This function assumes that facelet_cube has num_layers == 3 and
    // that the cube is in the correct orientation (U in U, F in F)
    
    cube3_t cube;
    
    
    
    
    
    
    
    
    
    return cube;
}

/* TO-DO

char* get_str_from_cube(const cube_t cube)

char* get_one_line_str_from_cube(const cube_t cube)

void apply_moves_to_cube(cube_t cube, moves_seq_t seq)

int is_cube_solved(cube_t cube)



*/





