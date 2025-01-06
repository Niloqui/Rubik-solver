#include <iostream>
#include <string>
#include <bitset>

enum face_t : unsigned char {
    R = 1,
    U = R+1,
    F = R+2,
    L = R+4,
    D = U+4,
    B = F+4
};

face_t faces[] = {R, U, F, L, D, B};

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
    default:
        return '\0';
    }
}

union move_t {
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
        unsigned char rotations : 2;
        
        face_t face : 3;
    };
    char val;
    unsigned char u_val;
};

int get_axis(move_t move){
    return move.face % 4;
}



int main() {
    std::cout << std::endl;
    
    for(face_t face : faces){
        std::cout << std::oct << int(face) << '\t' 
                << std::dec << int(face) << '\t'
                << get_face_char(face) << '\n';
    }
    
    move_t moves[6*3*3];
    
    int m=0, rot, lay;
    for(face_t face : faces){
        for(rot=1; rot<=3; rot++){
            for(lay=1; lay<=3; lay++){
                moves[m].face = face;
                moves[m].rotations = rot;
                moves[m].layers = lay;
                m++;
            }
        }
    }
    std::cout << "\n\n" << sizeof(move_t) << "\n\n";
    std::cout << "\n\n" << m << "\n\n";
    
    std::cout << "val" << '\t'
              << "u_val" << '\t'
              << "axis" << '\t'
              << "face" << '\t'
              << "face_c" << '\t'
              << "rot" << '\t'
              << "layer" << '\t'
              << "bit_val" << '\t'
              << '\n';
    for(move_t move : moves){
        std::cout << (int)move.val << '\t' 
                  << (int)move.u_val << '\t' 
                  << get_axis(move) << '\t'
                  << move.face << '\t'
                  << get_face_char(move.face) << '\t'
                  << (int)move.rotations << '\t'
                  << (int)move.layers << '\t'
                  << std::bitset<8>(move.val) << '\t'
                  << '\n';
    }
    
    
    
    
    
    
    return 0;
}















