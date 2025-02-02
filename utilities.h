#pragma once

#include <stdio.h>
#include <time.h> // struct timespec

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


void print_separator(){
    printf_s("\n------------------------------------------------------------------\n\n");
}






