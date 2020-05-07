/**\file wood_project3.c
 * Submission for project 3

 * This program is the solution to project 3: It will simulate a lift.

 *\author Kate Wood <kate.wood@hotmail.co.uk>
 *\version 1
 *\date 03 february 2017
 */



#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "shaft.h"
#include "lift.h"


 int main(int argc, char **argv){

    int i;
    int car_speed = 2;
    int shaft_count;
    int shaft_height;
    string_to_int(argv[1], &shaft_count);
    string_to_int(argv[2], &shaft_height);

    //Allocate space for a number of lift shaft pointers, the number of which should be provided on the command line.
    Shaft *shafts[shaft_count];

    //Create enough shafts for each pointer, the height of the shafts should be the same, and the height should be provided on the command line.
    for(i = 0; i < shaft_count; ++i) {
        shafts[i] = create_shaft(shaft_height, car_speed);
    }

    //Enter an infinite loop.
    while(1) {
    //Each time through the loop, update the shafts, print the shafts, and prompt the user for input.
        for(i = 0; i < shaft_count; ++i){
            update_lift(shafts[i]->car);
        }
        print_shafts(shafts, shaft_count);
        prompt_user(shafts, shaft_count, shaft_height);
    }
    return 0;
}