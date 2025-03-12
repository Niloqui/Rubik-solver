#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <direct.h> // _mkdir

// https://github.com/jean553/c-simd-avx2-example
#include <immintrin.h>

#include "utilities.h"
#include "face.h"
#include "move.h"
//#include "cube.h"
#include "cube3.h"


#define CORNER_PERM_N (1*2*3*4*5*6*7*8)             // 40320
#define CORNER_ORI_N (3*3*3*3*3*3*3)                // 2187
#define EDGE_PERM_N (1*2*3*4*5*6*7*8*9*10*11*12)    // 479001600
#define EDGE_ORI_N (2*2*2*2*2*2*2*2*2*2*2)          // 2048

#define MAX_DIM MAX(MAX(CORNER_ORI_N,CORNER_PERM_N),MAX(EDGE_ORI_N, UDSLICE_N))

#define UDSLICE_N (9*10*11*12)                      // 11880
#define UDSLICE_EQUI_N 788                          // Equivalence classes

//#define SOLVER_SYMMETRIES_N 4*2*2               // 16
//#define SYMMETRIES_N 3*SOLVER_SYMMETRIES_N      // 48    // Defined in cube3.h (another value)


////////////////// DON'T CHANGE THESE
////////////////// DON'T CHANGE THESE
char corners_order_base[NUM_CORNERS] = {UFR, UFL, UBR, UBL, DFR, DFL, DBR, DBL};
char edges_order_base[NUM_EDGES] = {UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR};
char pieces_order_base[NUM_CORNERS + NUM_EDGES] = {
    UFR, UFL, UBR, UBL, DFR, DFL, DBR, DBL,
    UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR
};
////////////////// DON'T CHANGE THESE
////////////////// DON'T CHANGE THESE


int n_choose_k_table[NUM_EDGES][4] = {-1};

void setup_n_choose_k_table(){
    for(int n=0; n<NUM_EDGES; n++){
        int n_fat = factorial_table[n];
        
        for(int k=0; k<(n>4?4:n+1); k++){
            int val = n_fat / (factorial_table[k] * factorial_table[n-k]);
            n_choose_k_table[n][k] = val;
        }
    }
}


int ud_slice_sorted_sym_to_raw_table[UDSLICE_EQUI_N][SYMMETRIES_N];
int ud_slice_sorted_raw_to_sym_table[UDSLICE_N][2];

typedef struct {
    int cor_o; // Corner orientation
    int cor_p; // Corner permutation
    int edge_o; // Edge orientation
    //int edge_p; // Edge permutation
    
    // Edge permutation divided in three indexes
    // Based on UDSliceSorted
    int ud_slice, u_face, d_face;
    
    // Sym coordinates
    int ud_sym_c, ud_sym_s;
} coord_cube_t;

int get_coord_corner_orientation(cube3_t cube3){
    __m256i helper_v, data_v;
    int p[8];
    
    helper_v = _mm256_set_epi32(
        3*3*3*3*3*3, 3*3*3*3*3, 3*3*3*3, 3*3*3, 3*3, 3, 1, 0
    );
    
    data_v = _mm256_set_epi32(
        cube3.corners[UFR].orientation,
        cube3.corners[UFL].orientation,
        cube3.corners[UBR].orientation,
        cube3.corners[UBL].orientation,
        cube3.corners[DFR].orientation,
        cube3.corners[DFL].orientation,
        cube3.corners[DBR].orientation,
        0 //cube3.corners[DBL].orientation
    );
    
    data_v = _mm256_mullo_epi32(data_v, helper_v);
    
    int cor_o = 0;
    _mm256_store_si256((__m256i *)p, data_v);
    //p = (int *)&data_v;
    for(int i=0; i<8; i++){
        cor_o += p[i];
    }
    
    return cor_o;
}

int get_coord_corner_permutation(cube3_t cube3){
    int p[8];
    __m256i helper_v, data_v;
    
    helper_v = _mm256_set_epi32(
        1*2*3*4*5*6*7, 1*2*3*4*5*6, 1*2*3*4*5, 1*2*3*4, 1*2*3, 1*2, 1, 0
        //0, 1, 1*2, 1*2*3, 1*2*3*4, 1*2*3*4*5, 1*2*3*4*5*6, 1*2*3*4*5*6*7
    );
    
    for(int i=0; i<8; i++){
        // Loop starts with i=0 because we need to set p[i]=0
        p[i] = 0;
        for(int j=0; j<i; j++){
            if(cube3.corners[j].piece > cube3.corners[i].piece){
                p[i] += 1;
            }
        }
    }
    
    data_v = _mm256_load_si256((__m256i *)p);
    data_v = _mm256_mullo_epi32(data_v, helper_v);
    
    int cor_p = 0;
    _mm256_store_si256((__m256i *)p, data_v);
    for(int i=0; i<8; i++){
        cor_p += p[i];
    }
    
    return cor_p;
}

int get_coord_edge_orientation(cube3_t cube3){
    short sp[16];
    __m256i helper_v, data_v;
    
    helper_v = _mm256_set_epi16(
        2*2*2*2*2*2*2*2*2*2, 2*2*2*2*2*2*2*2*2,
        2*2*2*2*2*2*2*2, 2*2*2*2*2*2*2,
        2*2*2*2*2*2, 2*2*2*2*2,
        2*2*2*2, 2*2*2,
        2*2, 2, 1, 0, 0, 0, 0, 0
    );
    
    //UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR
    
    data_v = _mm256_set_epi16(
        cube3.edges[UR].orientation,
        cube3.edges[UF].orientation,
        cube3.edges[UL].orientation,
        cube3.edges[UB].orientation,
        cube3.edges[DR].orientation,
        cube3.edges[DF].orientation,
        cube3.edges[DL].orientation,
        cube3.edges[DB].orientation,
        cube3.edges[FR].orientation,
        cube3.edges[FL].orientation,
        cube3.edges[BL].orientation,
        0, //cube3.edges[BR].orientation,
        0, 0, 0, 0
    );
    
    data_v = _mm256_mullo_epi16(data_v, helper_v);
    
    int edge_o = 0;
    _mm256_store_si256((__m256i *)sp, data_v);
    for(int i=0+4; i<12+4; i++){
        edge_o += sp[i];
    }
    
    return edge_o;
}

int get_coord_edge_permutation(cube3_t cube3){
    int p[2][8] = {0};
    __m256i data_v;
    __m256i edge_p_helpers[2] = {
        _mm256_set_epi32(
            //1*2*3*4*5*6*7, 1*2*3*4*5*6, 1*2*3*4*5, 1*2*3*4, 1*2*3, 1*2, 1, 0
            //0, 1, 1*2, 1*2*3, 1*2*3*4, 1*2*3*4*5, 0, 0
            0, 0, 1*2*3*4*5, 1*2*3*4, 1*2*3, 1*2, 1, 0
        ),
        _mm256_set_epi32(
            0, 0, 
            1*2*3*4*5*6*7*8*9*10*11, 1*2*3*4*5*6*7*8*9*10,
            1*2*3*4*5*6*7*8*9, 1*2*3*4*5*6*7*8,
            1*2*3*4*5*6*7, 1*2*3*4*5*6
        )
    };
    
    /* 
    for(int i=0; i<2; i++){
        for(int j=0; j<8; j++){
            p[i][j] = 0;
        }
    }
     */
    
    for(int i=1; i<12; i++){
        for(int j=0; j<i; j++){
            if(cube3.edges[j].piece > cube3.edges[i].piece){
                p[i/6][i%6] += 1;
            }
        }
    }
    
    int edge_p = 0;
    for(int i=0; i<2; i++){
        data_v = _mm256_load_si256((__m256i *)p[i]);
        data_v = _mm256_mullo_epi32(data_v, edge_p_helpers[i]);
        
        _mm256_store_si256((__m256i *)p[i], data_v);
        
        for(int j=0; j<6; j++){
            edge_p += p[i][j];
        }
    }
    
    return edge_p;
}

int get_coord_4_edges_permutation(cube3_t cube3, char offset){
    int coord = 0;
    int k = -1;
    int indexes[4];
    
    for(int n=0; n<NUM_EDGES; n++){
        int i = (n + NUM_EDGES - offset) % NUM_EDGES;
        if((cube3.edges[i].piece + offset) % NUM_EDGES >= FR){
            k++;
            indexes[k] = i;
        }
        else if(k>=0){
            int nck = n_choose_k_table[n][k];
            
            coord += nck;
        }
    }
    coord *= 24;
    
    for(int i=1; i<4; i++){
        int val = 0;
        
        for(int j=0; j<i; j++){
            if((cube3.edges[indexes[j]].piece + offset) % NUM_EDGES > 
                    (cube3.edges[indexes[i]].piece + offset) % NUM_EDGES){
                val++;
            }
        }
        
        coord += val * factorial_table[i];
    }
    
    return coord;
}

int get_coord_ud_slice_permutation(cube3_t cube3){
    return get_coord_4_edges_permutation(cube3, 0);
}
int get_coord_u_face_permutation(cube3_t cube3){
    return get_coord_4_edges_permutation(cube3, 8);
}
int get_coord_d_face_permutation(cube3_t cube3){
    return get_coord_4_edges_permutation(cube3, 4);
}

coord_cube_t create_coord_cube_from_cube3(const cube3_t cube3){
    coord_cube_t coords;
    //int p[2][8];
    //short sp[16];
    //__m256i helper_v, data_v;
    
    coords.cor_o = get_coord_corner_orientation(cube3);
    coords.cor_p = get_coord_corner_permutation(cube3);    
    coords.edge_o = get_coord_edge_orientation(cube3);
    //coords.edge_p = get_coord_edge_permutation(cube3);
    
    coords.ud_slice = get_coord_ud_slice_permutation(cube3);
    coords.u_face = get_coord_u_face_permutation(cube3);
    coords.d_face = get_coord_d_face_permutation(cube3);
    
    coords.ud_sym_c = ud_slice_sorted_raw_to_sym_table[coords.ud_slice][0];
    coords.ud_sym_s = ud_slice_sorted_raw_to_sym_table[coords.ud_slice][1];
    
    return coords;
}

coord_cube_t create_coord_cube(){
    coord_cube_t coords;
    
    coords.cor_o = coords.cor_p = coords.edge_o = 0;
    //coords.edge_p = 0;
    coords.ud_slice = coords.d_face = coords.u_face = 0;
    
    coords.ud_sym_c = coords.ud_sym_s = 0;
    
    return coords;
}

void get_inverse_coord_4_edges_permutation(cube3_t *cube3_p, int coord, int offset){
    //coord = coord_cube.ud_slice;
    int coord_p = coord % 24; // Permutation part of the index
    int coord_c = coord / 24; // Combination part of the index
    
    int indexes[4] = {0,0,0,0};
    int used_pieces[4] = {0,0,0,0};
    
    for(int n=NUM_EDGES-1, k=3; n>=0 && k>=0; n--){
        if(coord_c - n_choose_k_table[n][k] >= 0){
            coord_c -= n_choose_k_table[n][k];
        }
        else{
            // "+ NUM_EDGES" so it doesn't go into the negatives
            indexes[k] = (n + NUM_EDGES - offset) % NUM_EDGES;
            k--;
        }
    }
    
    for(int i=4-1; i>=0; i--){
        int val = (coord_p / factorial_table[i]);
        
        for(edges_e j=4-1; j>=0; j--){
            if(used_pieces[j] == 0){
                if(val == 0){
                    used_pieces[j] = 1;
                    cube3_p->edges[indexes[i]].piece = (j + 8 + NUM_EDGES - offset) % NUM_EDGES;
                    break;
                }
                else{
                    val--;
                }
            }
        }
        
        coord_p = coord_p % factorial_table[i];
    }
}

void get_inverse_corner_orientation(cube3_t *cube3_p, int coord){
    char total_orientation = 0, orientation;
    
    for(int i=NUM_CORNERS-2; i>=0; i--){
        orientation = coord % 3;
        
        cube3_p->corners[i].orientation = orientation;
        total_orientation += orientation;
        
        coord /= 3;
    }
    cube3_p->corners[NUM_CORNERS-1].orientation = (total_orientation*2)%3;
}

void get_inverse_edge_orientation(cube3_t *cube3_p, int coord){
    char total_orientation = 0, orientation;
    
    for(int i=NUM_EDGES-2; i>=0; i--){
        orientation = coord % 2;
        
        cube3_p->edges[i].orientation = orientation;
        total_orientation += orientation;
        
        coord /= 2;
    }
    cube3_p->edges[NUM_EDGES-1].orientation = total_orientation%2;
}

cube3_t create_cube3_from_coord_cube(const coord_cube_t coord_cube){
    cube3_t cube3 = create_cube3();
    int coord;
    //int used_pieces[MAX(NUM_CORNERS,NUM_EDGES)];
    int used_pieces[NUM_CORNERS];
    
    
    // Corner orientation
    get_inverse_corner_orientation(&cube3, coord_cube.cor_o);
    
    
    // Edge orientation
    get_inverse_edge_orientation(&cube3, coord_cube.edge_o);
    
    
    // Corner permutation
    coord = coord_cube.cor_p;
    for(int i=0; i<NUM_CORNERS; i++){
        used_pieces[i] = 0;
    }
    
    for(int i=NUM_CORNERS-1; i>=0; i--){
        int val = (coord / factorial_table[i]);
        
        //used_pieces[corner] = 1;
        
        for(corners_e j=NUM_CORNERS-1; j>=0; j--){
            if(used_pieces[j] == 0){
                if(val == 0){
                    used_pieces[j] = 1;
                    cube3.corners[i].piece = j;
                    break;
                }
                else{
                    val--;
                }
            }
            /* 
            if(val == 0 && used_pieces[j] > 0){
                used_pieces[j] = 1;
                cube3.corners[i].piece = j;
                break;
            }
            if(used_pieces[j] == 0){
                val--;
            } */
        }
        
        coord = coord % factorial_table[i];
    }
    
    for(int i=0; i<NUM_EDGES; i++){
        cube3.edges[i].piece = EDGE_LAST;
    }
    
    // UD Slice
    get_inverse_coord_4_edges_permutation(&cube3, coord_cube.ud_slice, 0);
    
    // U Face
    get_inverse_coord_4_edges_permutation(&cube3, coord_cube.u_face, 8);
    
    // D Face
    get_inverse_coord_4_edges_permutation(&cube3, coord_cube.d_face, 4);
    
    return cube3;
}

int is_coord_cube_solved(const coord_cube_t coords){
    return !(coords.cor_o || coords.cor_p || coords.edge_o ||
            coords.ud_slice || coords.u_face || coords.d_face);
}

char* get_str_from_coord_cube(const coord_cube_t coord_cube){
    char *str;
    
    // 1024 characters should be more than enough for this,
    // so no checks on out of bounds writes :)
    str = (char*)malloc(1024);
    if(str == NULL){
        printf_s("Error in memory allocation.\n");
        return str; // return NULL
    }
    
    sprintf_s(str, 1024, 
        "cor_o = %i\n"
        "cor_p = %i\n"
        "edge_o = %i\n"
        "u_face = %i\n"
        "d_face = %i\n"
        "ud_slice = %i\n"
        "ud_sym_c = %i\n"
        "ud_sym_s = %i\n\n",
        coord_cube.cor_o, coord_cube.cor_p, coord_cube.edge_o,
        coord_cube.u_face, coord_cube.d_face, coord_cube.ud_slice,
        coord_cube.ud_sym_c, coord_cube.ud_sym_s
    );
    
    //ud_sym_c, ud_sym_s
    
    return str;
}



////////////////////////////////////////////////////////////////////////////
////////////////////////////// Movement Tables /////////////////////////////
////////////////////////////////////////////////////////////////////////////

typedef struct{
    //edges_e edges_order[12];
    //corners_e corners_order[8];
    
    void *main_index;
    int *cor_o; // cor_o[CORNER_ORI_N][NUM_MOVES]
    int *cor_p; // cor_p[CORNER_PERM_N][NUM_MOVES]
    int *edge_o; // edge_o[EDGE_ORI_N][NUM_MOVES]
    //int *edge_p;
    int *ud_slice, *u_face, *d_face; // ud_slice[UDSLICE_N][NUM_MOVES]
    
    int *ud_slice_sym; // ud_slice_sym[UDSLICE_EQUI_N][NUM_MOVES][2]
} movement_table_t;

movement_table_t move_tbs;

typedef struct{
    int coord; // Coordinate
    int num_moves;
    cube3_t cube_state;
} table_entry_t;

int generate_move_table(int *table_p, int table_dim, int (*get_coord_f)(cube3_t),
                        table_entry_t *cube_queue, unsigned char *visited_nodes){
    //table_entry_t *cube_queue;
    //unsigned char *visited_nodes;
    // 0: not visited
    // 1: in list
    // 2: visited
    
    for(int i=0; i<table_dim; i++){
        visited_nodes[i] = 0;
    }
    
    cube3_t cube3 = create_cube3();
    int coord = (*get_coord_f)(cube3);
    int num_moves = 0;
    
    cube_queue[0].coord = coord;
    cube_queue[0].num_moves = num_moves;
    cube_queue[0].cube_state = cube3;
    visited_nodes[coord] = 1;
    
    int entry=0, num_entries=1;
    while(entry!=num_entries){
        coord = cube_queue[entry].coord;
        num_moves = cube_queue[entry].num_moves;
        cube3 = cube_queue[entry].cube_state;
        
        for(fast_move_t fast_m=0; fast_m<LAST_FAST_MOVE; fast_m++){
            move_t move = fast_to_move_table[fast_m];
                
            cube3_t new_cube3 = apply_move_to_cube3(cube3, move);
            int new_coord = (*get_coord_f)(new_cube3);
            
            table_p[coord*NUM_MOVES + fast_m] = new_coord;
            
            if(visited_nodes[new_coord] == 0){
                visited_nodes[new_coord] = 1;
                cube_queue[num_entries].cube_state = new_cube3;
                cube_queue[num_entries].coord = new_coord;
                cube_queue[num_entries].num_moves = num_moves + 1;
                num_entries++;
            }
        }
        
        visited_nodes[coord] = 2;
        entry++;
    }
    /* 
    printf_s("visited_nodes:\n");
    for(int i=0; i<CORNER_ORI_N * sizeof(unsigned char); i++){
        printf_s("%i - %i\n", i, visited_nodes[i]);
    } */
    
    //free(cube_queue);
    return 0;
}

int allocate_move_table(const char *file_name, char *base_check_array, int check_length,
                        int *table_p, int table_dim, int (*get_coord_f)(cube3_t),
                        table_entry_t *cube_queue, unsigned char *visited_nodes){
    // If the table file exists, this function reads it
    // If it doesn't, this function generate a new one and saves it in a file
    
    char check_order[MAX(NUM_CORNERS,NUM_EDGES)]; // = {-1};
    
    FILE *f;
    int error;
    
    printf_s("Checking if \"%s\" exists.\n", file_name);
    error = fopen_s(&f, file_name, "rb");
    
    if(error == 0){
        // File exists, now we must check for the validity.
        fread_s(check_order, check_length, sizeof(char), check_length, f);
        
        for(int i=0; i<check_length; i++){
            if(check_order[i] != base_check_array[i]){
                error = 1;
                break;
            }
        }
    }
    
    // error can be modified in the previous section
    if(error){
        // File doesn't exist or it has wrong piece order.
        // The table must be computed here.
        printf_s("File \"%s\" doesn't exist or it is not valid.\n", file_name);
        
        // Closing the file, it must be reopened in writing mode
        fclose(f);
        
        //for(int i=0; i<table_dim*NUM_MOVES; i++){
        //    table_p[i] = -1;
        //}
        
        error = generate_move_table(table_p, table_dim, get_coord_f,
                                    cube_queue, visited_nodes);
        if(error){
            return 1;
        }
        
        error = fopen_s(&f, file_name, "wb");
        fwrite(base_check_array, sizeof(char), check_length, f);
        fwrite(table_p, sizeof(int), table_dim*NUM_MOVES, f);
    }
    else{
        // File exists and it's ok, now we must read the data.
        printf_s("File \"%s\" is valid.\n", file_name);
        
        // The check_order has been already read
        fread_s(table_p, table_dim*NUM_MOVES*sizeof(int),
                sizeof(int), table_dim*NUM_MOVES, f);
    }
    
    printf_s("\n");
    fclose(f);
    return 0;
}


void setup_ud_slice_sym_table(){
    cube3_t cube3 = create_cube3(), temp_cube3;
    
    // sym_c = symmetry coordinate
    // symmetry = index in symmetry table
    int i, raw, sym_c, new_raw, new_sym_c, symmetry;
    
    // "ud_slice_sorted_raw_to_sym_table" is used as visited array
    for(raw=0; raw<UDSLICE_N; raw++){
        ud_slice_sorted_raw_to_sym_table[raw][0] = -1;
        ud_slice_sorted_raw_to_sym_table[raw][1] = -1;
    }
    
    //// First phase: initiliaze "ud_slice_sorted_sym_to_raw_table"
    //                       and "ud_slice_sorted_raw_to_sym_table"
    sym_c = 0;
    for(raw=0; raw<UDSLICE_N; raw++){
        if(ud_slice_sorted_raw_to_sym_table[raw][0] >= 0){
            // Already visited state
            continue;
        }
        
        // Cleaning the cube pieces.
        // This is done because the "get_inverse_coord_4_edges_permutation" function
        // only modifies the values for the UD slice pieces,
        // leaving everything else the same.
        // Normally this function is used also for U face and D face pieces (so all edges),
        // so this problem never happens, but in this case it's only being used for ud slice
        // pieces. If the cube is not cleaned, there could be more UD slice pieces in the
        // edges array, breaking the function.
        for(i=0; i<NUM_EDGES; i++){
            cube3.edges[i].piece = UR;
        }
        
        //if(sym_c == 228){
        //    printf_s("");
        //}
        
        get_inverse_coord_4_edges_permutation(&cube3, raw, 0);
        
        // j goes backwards because, in case the same value for ud_slice_coord
        // appears in more than one symmetry,
        // the symmetry with the lowest value is saved in the table.
        //for(j=0; j<SYMMETRIES_N; j++){
        for(symmetry=SYMMETRIES_N-1; symmetry>=0; symmetry--){
            temp_cube3 = multiply_cube3(symmetries_inverse_table[symmetry], cube3);
            temp_cube3 = multiply_cube3(temp_cube3, symmetries_table[symmetry]);
            
            new_raw = get_coord_ud_slice_permutation(temp_cube3);
            
            //ud_slice_sorted_raw_to_sym_table[ud_slice_coord] = idx*SYMMETRIES_N + j;
            ud_slice_sorted_raw_to_sym_table[new_raw][0] = sym_c;
            ud_slice_sorted_raw_to_sym_table[new_raw][1] = symmetry;
            
            ud_slice_sorted_sym_to_raw_table[sym_c][symmetry] = new_raw;
        }
        
        //ud_slice_sorted_sym_to_raw_table[sym_c] = raw;
        sym_c++;
    }
    
    //// Second phase: set up "move_tbs.ud_slice_sym"
    //restore_cube3(&cube3);
    /* 
    for(sym_c=0; sym_c<UDSLICE_EQUI_N; sym_c++){
        raw = ud_slice_sorted_sym_to_raw_table[sym_c];
        
        // Cleaning the cube pieces.
        // Same reasons as in the first phase.
        for(i=0; i<NUM_EDGES; i++){
            cube3.edges[i].piece = UR;
        }
        get_inverse_coord_4_edges_permutation(&cube3, raw, 0);
        
        for(fast_move_t fast_m=0; fast_m<LAST_FAST_MOVE; fast_m++){
            temp_cube3 = apply_move_to_cube3(cube3, fast_to_move_table[fast_m]);
            
            new_raw = get_coord_ud_slice_permutation(temp_cube3);
            
            new_sym_c = ud_slice_sorted_raw_to_sym_table[new_raw][0];
            symmetry = ud_slice_sorted_raw_to_sym_table[new_raw][1];
            
            // [UDSLICE_EQUI_N][NUM_MOVES][2]
            i = sym_c * NUM_MOVES * 2 + fast_m * 2;
            
            move_tbs.ud_slice_sym[i] = new_sym_c;
            move_tbs.ud_slice_sym[i + 1] = symmetry;
        }
    }
     */
}


int setup_movement_tables_3x3x3(){
    int error = 0;
    
    move_tbs.main_index = malloc(
        sizeof(int) * (
            CORNER_PERM_N * NUM_MOVES +
            CORNER_ORI_N * NUM_MOVES +
            EDGE_ORI_N * NUM_MOVES +
            UDSLICE_N * 3 * NUM_MOVES +
            UDSLICE_EQUI_N * 2 * NUM_MOVES
        )
    );
    
    if(move_tbs.main_index == NULL){
        return -1;
    }
    
    move_tbs.cor_o = move_tbs.main_index;
    move_tbs.cor_p = move_tbs.cor_o + CORNER_ORI_N * NUM_MOVES;
    move_tbs.edge_o = move_tbs.cor_p + CORNER_PERM_N * NUM_MOVES;
    move_tbs.ud_slice = move_tbs.edge_o + EDGE_ORI_N * NUM_MOVES;
    move_tbs.u_face = move_tbs.ud_slice + UDSLICE_N * NUM_MOVES;
    move_tbs.d_face = move_tbs.u_face + UDSLICE_N * NUM_MOVES;
    move_tbs.ud_slice_sym = move_tbs.d_face + UDSLICE_N * NUM_MOVES;
    
    // Creating folders
    _mkdir("tables");
    _mkdir("tables\\3x3x3");
    
    //printf_s("MAX_DIM = %i\n", MAX_DIM);
    void *helper = malloc(MAX_DIM * sizeof(table_entry_t) + 
                          MAX_DIM * sizeof(unsigned char));
    if(helper == NULL){
        return -2;
    }
    
    table_entry_t *cube_queue;
    unsigned char *visited_nodes;
    
    cube_queue = (table_entry_t *)helper;
    visited_nodes = (unsigned char *)helper + MAX_DIM * sizeof(table_entry_t);
    
    error += allocate_move_table("tables\\3x3x3\\movement.cor_o",
                                corners_order_base, NUM_CORNERS,
                                move_tbs.cor_o, CORNER_ORI_N,
                                get_coord_corner_orientation,
                                cube_queue, visited_nodes);
    
    error += allocate_move_table("tables\\3x3x3\\movement.cor_p",
                                corners_order_base, NUM_CORNERS,
                                move_tbs.cor_p, CORNER_PERM_N,
                                get_coord_corner_permutation,
                                cube_queue, visited_nodes);
    
    error += allocate_move_table("tables\\3x3x3\\movement.edge_o",
                                edges_order_base, NUM_EDGES,
                                move_tbs.edge_o, EDGE_ORI_N,
                                get_coord_edge_orientation,
                                cube_queue, visited_nodes);
    
    error += allocate_move_table("tables\\3x3x3\\movement.ud_slice",
                                edges_order_base, NUM_EDGES,
                                move_tbs.ud_slice, UDSLICE_N,
                                get_coord_ud_slice_permutation,
                                cube_queue, visited_nodes);
    
    error += allocate_move_table("tables\\3x3x3\\movement.u_face",
                                edges_order_base, NUM_EDGES,
                                move_tbs.u_face, UDSLICE_N,
                                get_coord_u_face_permutation,
                                cube_queue, visited_nodes);
    
    error += allocate_move_table("tables\\3x3x3\\movement.d_face",
                                edges_order_base, NUM_EDGES,
                                move_tbs.d_face, UDSLICE_N,
                                get_coord_d_face_permutation,
                                cube_queue, visited_nodes);
    
    printf_s("Number of errors while allocating the movement tables: %i\n\n", error);
    
    free(helper);
    return error;
}



////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Moves ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////

coord_cube_t apply_fast_move_to_coord_cube(coord_cube_t coords, const fast_move_t fast_m){
    coords.cor_o = move_tbs.cor_o[coords.cor_o*NUM_MOVES + fast_m];
    coords.cor_p = move_tbs.cor_p[coords.cor_p*NUM_MOVES + fast_m];
    coords.edge_o = move_tbs.edge_o[coords.edge_o*NUM_MOVES + fast_m];
    
    coords.ud_slice = move_tbs.ud_slice[coords.ud_slice*NUM_MOVES + fast_m];
    coords.u_face = move_tbs.u_face[coords.u_face*NUM_MOVES + fast_m];
    coords.d_face = move_tbs.d_face[coords.d_face*NUM_MOVES + fast_m];
    
    // Sym-coordinates "movement"
    coords.ud_sym_c = ud_slice_sorted_raw_to_sym_table[coords.ud_slice][0];
    coords.ud_sym_s = ud_slice_sorted_raw_to_sym_table[coords.ud_slice][1];
    
    return coords;
}

coord_cube_t apply_move_to_coord_cube(coord_cube_t coord_cube, const move_t move){
    return apply_fast_move_to_coord_cube(coord_cube, get_fast_from_move(move));
}

coord_cube_t apply_seq_to_coord_cube(coord_cube_t coord_cube, const moves_seq_t seq){
    for(int i=0; i<seq.len; i++){
        coord_cube = apply_move_to_coord_cube(coord_cube, seq.moves[i]);
    }
    
    return coord_cube;
}

coord_cube_t apply_inverse_seq_to_coord_cube(coord_cube_t coord_cube, const moves_seq_t seq){
    for(int i=seq.len-1; i>=0; i--){
        move_t move = get_inverse_move(seq.moves[i]);
        coord_cube = apply_move_to_coord_cube(coord_cube, move);
    }
    
    return coord_cube;
}


void stress_test_coord_cube(coord_cube_t coord_cube, const char *str, int repetition){
    struct timespec start, end, diff;
    
    moves_seq_t seq = get_move_seq_from_str(3, str);
    
    timespec_get(&start, TIME_UTC);
    for(int i=0; i<repetition; i++){
        coord_cube = apply_seq_to_coord_cube(coord_cube, seq);
    }
    timespec_get(&end, TIME_UTC);
    
    diff = compute_timespec_difference(end, start);
    printf_s("\nIt took %u.%09u seconds to apply \"%s\" %d times (coord_cube_t).\n",
            diff.tv_sec, diff.tv_nsec, str, repetition
    );
    
    char *result = get_str_from_coord_cube(coord_cube);
    printf_s("%s\n", result);
    
    free(seq.moves);
}



////////////////////////////////////////////////////////////////////////////
////////////////////////////// Pruning Tables //////////////////////////////
////////////////////////////////////////////////////////////////////////////

typedef struct{
    void *main_index;
    char *huge; // huge[UDSLICE_EQUI_N][CORNER_ORI_N][EDGE_ORI_N]
    long long int huge_dim;
} pruning_table_t;

pruning_table_t prune_tbs;

int corner_orientantion_symmetries[CORNER_ORI_N][SYMMETRIES_N];
int edge_orientantion_symmetries[UDSLICE_EQUI_N][EDGE_ORI_N][SYMMETRIES_N];

int check_file_validity_and_read(void *table_p, long long int table_dim, int elem_size, 
                                char *file_name, char *base_check_array, int check_length){
    char check_order[NUM_CORNERS + NUM_EDGES]; // = {-1};
    
    FILE *f;
    int error;
    
    printf_s("Checking if \"%s\" exists.\n", file_name);
    error = fopen_s(&f, file_name, "rb");
    
    if(error == 0){
        // File exists, now we must check for the validity.
        fread_s(check_order, check_length, sizeof(char), check_length, f);
        
        for(int i=0; i<check_length; i++){
            if(check_order[i] != base_check_array[i]){
                error = 1;
                break;
            }
        }
    }
    
    // error can be modified in the previous section
    if(error == 0){
        // File exists and it's ok, now we must read the data.
        printf_s("File \"%s\" is valid.\n", file_name);
        
        // The check_order has been already read
        fread_s(table_p, table_dim*elem_size, elem_size, table_dim, f);
    }
    else{
        // File doesn't exist or it has wrong piece order.
        printf_s("File \"%s\" doesn't exist or it is not valid.\n", file_name);
    }
    
    fclose(f);
    return error;
}

void setup_corner_orientantion_symmetries(){
    char file_name[] = "tables\\3x3x3\\symmetries.cor_o";
    
    if(check_file_validity_and_read((int *)corner_orientantion_symmetries,
            CORNER_ORI_N*SYMMETRIES_N, sizeof(int), file_name,
            corners_order_base, NUM_CORNERS) == 0){
        // File exists and it's ok
        // File has been already read
        printf_s("\n");
        return;
    }
    
    // File doesn't exist or it has wrong piece order.
    // The table must be computed here.
    cube3_t cube3 = create_cube3();
    cube3_t new_cube3;
    int coord, sym;
    int new_coord;
    
    for(coord=0; coord<CORNER_ORI_N; coord++){
        get_inverse_corner_orientation(&cube3, coord);
        
        for(sym=0; sym<SYMMETRIES_N; sym++){
            //new_cube3 = multiply_cube3(symmetries_table[sym], cube3);
            //new_cube3 = multiply_cube3(new_cube3, symmetries_inverse_table[sym]);
            new_cube3 = multiply_cube3(symmetries_inverse_table[sym], cube3);
            new_cube3 = multiply_cube3(new_cube3, symmetries_table[sym]);
            
            new_coord = get_coord_corner_orientation(new_cube3);
            
            //corner_orientantion_symmetries[coord][sym] = new_coord;
            corner_orientantion_symmetries[new_coord][sym] = coord;
        }
    }
    
    // Saving the table into a file
    FILE *f;
    int error;
    
    error = fopen_s(&f, file_name, "wb");
    fwrite(corners_order_base, sizeof(char), NUM_CORNERS, f);
    fwrite(corner_orientantion_symmetries, sizeof(int), CORNER_ORI_N*SYMMETRIES_N, f);
    
    printf_s("\n");
    fclose(f);
}

void setup_edge_orientantion_symmetries(){
    char file_name[] = "tables\\3x3x3\\symmetries.edge_o";
    
    if(check_file_validity_and_read((int *)edge_orientantion_symmetries,
            UDSLICE_EQUI_N*EDGE_ORI_N*SYMMETRIES_N, sizeof(int),
            file_name, edges_order_base, NUM_EDGES) == 0){
        // File exists and it's ok
        // File has been already read
        printf_s("\n");
        return;
    }
    
    // File doesn't exist or it has wrong piece order.
    // The table must be computed here.
    cube3_t cube3 = create_cube3();
    cube3_t new_cube3;
    int edge_o, sym_c, sym_s, raw;
    int new_coord;
    
    for(sym_c=0; sym_c<UDSLICE_EQUI_N; sym_c++){
        for(int i=0; i<NUM_EDGES; i++){
            cube3.edges[i].piece = UR;
        }
        
        raw = ud_slice_sorted_sym_to_raw_table[sym_c][0];
        get_inverse_coord_4_edges_permutation(&cube3, raw, 0);
        
        for(edge_o=0; edge_o<EDGE_ORI_N; edge_o++){
            get_inverse_edge_orientation(&cube3, edge_o);
            
            if(sym_c == 228 && edge_o == 0){
                printf_s("\n--%i--%i--\n\n", sym_c, edge_o);
            }
            
            if(sym_c == 228 && edge_o == 550){
                printf_s("\n--%i--%i--\n\n", sym_c, edge_o);
            }
            
            if(sym_c == 228 && edge_o == 1092){
                printf_s("\n--%i--%i--\n\n", sym_c, edge_o);
            }
            
            if(sym_c == 645 && edge_o == 550){
                printf_s("\n--%i--%i--\n\n", sym_c, edge_o);
            }
            
            if(sym_c == 645 && edge_o == 1570){
                printf_s("\n--%i--%i--\n\n", sym_c, edge_o);
            }
            
            for(sym_s=SYMMETRIES_N-1; sym_s>=0; sym_s--){
                //new_cube3 = multiply_cube3(symmetries_table[sym_s], cube3);
                //new_cube3 = multiply_cube3(new_cube3, symmetries_inverse_table[sym_s]);
                new_cube3 = multiply_cube3(symmetries_inverse_table[sym_s], cube3);
                new_cube3 = multiply_cube3(new_cube3, symmetries_table[sym_s]);
                
                new_coord = get_coord_edge_orientation(new_cube3);
                
                //edge_orientantion_symmetries[sym_c][edge_o][sym_s] = new_coord;
                edge_orientantion_symmetries[sym_c][new_coord][sym_s] = edge_o;
            }
        }
    }
    
    // Saving the table into a file
    FILE *f;
    int error;
    
    error = fopen_s(&f, file_name, "wb");
    fwrite(edges_order_base, sizeof(char), NUM_EDGES, f);
    fwrite(edge_orientantion_symmetries, sizeof(int),
            UDSLICE_EQUI_N*EDGE_ORI_N*SYMMETRIES_N, f);
    
    printf_s("\n");
    fclose(f);
}


int setup_pruning_tables_3x3x3(){
    int error = 0, huge_type;
    long long int huge_dim;
    long long int i=0, j=0;
    
    
    
    char file_name[128] = "tables\\3x3x3\\pruning.huge_";
    huge_type = 3;
    
    switch (huge_type){
    case 1:
        // No sym-coordinates, each byte represents a single value
        huge_dim = (long long int)UDSLICE_N * CORNER_ORI_N * EDGE_ORI_N;
        strcat(file_name, "1");
        break;
    case 2:
        // No sym-coordinates, each byte is divided in 4 parts
        huge_dim = (long long int)UDSLICE_N * CORNER_ORI_N * EDGE_ORI_N / 4;
        strcat(file_name, "2");
        break;
    default:
    case 3:
        // Sym-coordinates for UD-Slice, each byte represents a single value
        huge_dim = (long long int)UDSLICE_EQUI_N * CORNER_ORI_N * EDGE_ORI_N;
        strcat(file_name, "3");
        break;
    }
    
    if(huge_type != 3){
        printf_s("Type %i for the huge table is still not supported :(\n", huge_type);
        return -2;
    }
    
    
    prune_tbs.main_index = malloc(
        sizeof(char) * (huge_dim)
    );
    
    if(prune_tbs.main_index == NULL){
        printf_s("Failed to allocate %lli bytes for the huge prune table allocated.\n",
                huge_dim);
        return -1;
    }
    prune_tbs.huge = prune_tbs.main_index;
    prune_tbs.huge_dim = huge_dim;
    
    printf_s("Huge prune table allocated (%lli bytes).\n", huge_dim);
    
    
    if(check_file_validity_and_read(prune_tbs.huge, huge_dim, sizeof(char), file_name,
            pieces_order_base, NUM_CORNERS + NUM_EDGES) == 0){
        // File exists and it's ok
        // File has been already read
        printf_s("\n");
        return -1;
    }
    
    // File doesn't exist or it has wrong piece order.
    // The table must be computed here.
    char current_num_moves, next_num_moves;
    long long int num_computed_total, num_computed_loop, num_visited_total, num_visited_loop;
    long long int ud_slice, udsl_sym_c, udsl_sym_s, cor_o, edge_o;
    long long int new_ud_slice, new_udsl_sym_c, new_udsl_sym_s, new_cor_o, new_edge_o;
    long long int mid_cor_o, mid_edge_o;
    //int symmetry;
    
    cube3_t cube3 = create_cube3();
    fast_move_t fast_m;
    long long int prune_values_distribution[GOD_N_3X3+1];
    for(i=0; i<GOD_N_3X3+1; i++){
        prune_values_distribution[i] = 0;
    }
    //prune_values_distribution[0] = 1;
    
    printf_s("Initializing the table memory.\n");
    for(i=1; i<huge_dim; i++){
        //prune_tbs.huge[i] = 0xAA; // 0xAA = 1010 1010
        prune_tbs.huge[i] = -1;
    }
    prune_tbs.huge[0] = 0;
    
    current_num_moves = 0;
    next_num_moves = 1;
    num_computed_total = 0;
    num_visited_total = 0;
    do{
        num_computed_loop = 0;
        num_visited_loop = 0;
        printf_s("Current number of moves: %2i --- ", current_num_moves);
        
        for(i=0; i<huge_dim; i++){
            if(prune_tbs.huge[i] != current_num_moves){
                continue;
            }
            num_visited_loop++;
            num_visited_total++;
            
            edge_o = i % EDGE_ORI_N;
            cor_o = (i / EDGE_ORI_N) % CORNER_ORI_N;
            udsl_sym_c = i / (EDGE_ORI_N * CORNER_ORI_N);
            //udsl_sym_s = 0;
            
            ud_slice = ud_slice_sorted_sym_to_raw_table[udsl_sym_c][0];
            
            //if(udsl_sym_c == 228 && cor_o == 1236 && edge_o == 550){
            //    printf_s("");
            //}
            
            
            //if(udsl_sym_c == 188 && cor_o == 1953 && edge_o == 534){
            //    printf_s("");
            //}
            
            //if(udsl_sym_c == 777 && cor_o == 1268 && edge_o == 687){
            //    printf_s("");
            //}
            
            
            
            for(fast_m=0; fast_m<LAST_FAST_MOVE; fast_m++){
                new_ud_slice = move_tbs.ud_slice[ud_slice*NUM_MOVES + fast_m];
                mid_cor_o = move_tbs.cor_o[cor_o*NUM_MOVES + fast_m];
                mid_edge_o = move_tbs.edge_o[edge_o*NUM_MOVES + fast_m];
                
                new_udsl_sym_c = ud_slice_sorted_raw_to_sym_table[new_ud_slice][0];
                //new_udsl_sym_s = ud_slice_sorted_raw_to_sym_table[new_ud_slice][1];
                
                for(new_udsl_sym_s=0; new_udsl_sym_s<SYMMETRIES_N; new_udsl_sym_s++){
                    if(ud_slice_sorted_sym_to_raw_table[new_udsl_sym_c][new_udsl_sym_s] != new_ud_slice){
                        continue;
                    }
                    
                    new_cor_o = corner_orientantion_symmetries[mid_cor_o][new_udsl_sym_s];
                    new_edge_o = edge_orientantion_symmetries[new_udsl_sym_c][mid_edge_o][new_udsl_sym_s];
                    
                    // huge[UDSLICE_EQUI_N][CORNER_ORI_N][EDGE_ORI_N]
                    j = (new_udsl_sym_c * CORNER_ORI_N + new_cor_o) * EDGE_ORI_N + new_edge_o;
                    
                    if(prune_tbs.huge[j] < 0){
                        prune_tbs.huge[j] = next_num_moves;
                        num_computed_total++;
                        num_computed_loop++;
                    }
                }
                
            }
        }
        
        //prune_values_distribution[next_num_moves] = num_computed_loop;
        printf_s("%10lli / %10lli\n", num_visited_loop, num_visited_total);
        
        current_num_moves++;
        next_num_moves++;
    //} while(num_computed_loop);
    } while(num_computed_total < huge_dim && num_computed_loop);
    
    
    long long int empty_slots = 0;
    for(i=0; i<huge_dim; i++){
        if(prune_tbs.huge[i] >= 0){
            prune_values_distribution[prune_tbs.huge[i]] += 1;
        }
        else{
            empty_slots++;
        }
    }
    
    printf_s("\nDistribution of the Pruning Values in the Pruning Tables:\n");
    for(i=0; i<GOD_N_3X3+1; i++){
        printf_s("%2i -- %10lli\n", i, prune_values_distribution[i]);
    }
    printf_s("\n");
    printf_s("Empty slots: %lli\n", empty_slots);
    printf_s("Number of computed values: %lli\n", num_computed_total);
    printf_s("Huge dim: %lli\n", huge_dim);
    
    
    // Saving the table into a file
    FILE *f;
    //int error;
    
    error = fopen_s(&f, file_name, "wb");
    fwrite(pieces_order_base, sizeof(char), NUM_CORNERS + NUM_EDGES, f);
    fwrite(prune_tbs.huge, sizeof(char),
            (long long int)UDSLICE_EQUI_N*CORNER_ORI_N*EDGE_ORI_N, f);
            
    printf_s("\n");
    fclose(f);
    
    printf_s("Number of errors while allocating the pruning tables: %i\n\n", error);
    return 0;
}

char get_pruning_huge_value(coord_cube_t coords){
    long long int i;
    long long int udsl_sym_c, udsl_sym_s, cor_o, edge_o;
    
    udsl_sym_c = ud_slice_sorted_raw_to_sym_table[coords.ud_slice][0];
    udsl_sym_s = ud_slice_sorted_raw_to_sym_table[coords.ud_slice][1];
    
    cor_o = corner_orientantion_symmetries[coords.cor_o][udsl_sym_s];
    edge_o = edge_orientantion_symmetries[udsl_sym_c][coords.edge_o][udsl_sym_s];
    
    if(prune_tbs.huge == NULL){
        return -1;
    }
    
    i = (udsl_sym_c * CORNER_ORI_N + cor_o) * EDGE_ORI_N + edge_o;
    return prune_tbs.huge[i];
}


/*

    for(new_udsl_sym_s=0; new_udsl_sym_s<SYMMETRIES_N; new_udsl_sym_s++){
        if(ud_slice_sorted_sym_to_raw_table[new_udsl_sym_c][new_udsl_sym_s] != new_ud_slice){
            continue;
        }
        
        new_cor_o = corner_orientantion_symmetries[mid_cor_o][new_udsl_sym_s];
        new_edge_o = edge_orientantion_symmetries[new_udsl_sym_c][mid_edge_o][new_udsl_sym_s];
        
        // huge[UDSLICE_EQUI_N][CORNER_ORI_N][EDGE_ORI_N]
        j = (new_udsl_sym_c * CORNER_ORI_N + new_cor_o) * EDGE_ORI_N + new_edge_o;
        
        if(prune_tbs.huge[j] < 0){
            prune_tbs.huge[j] = next_num_moves;
            num_computed_total++;
            num_computed_loop++;
        }
    }
*/












