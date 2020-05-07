/**\file lift.c
 * Submission for project 3

 * This program is the solution to project 3: It will implement the lift shaft.

 *\author Kate Wood <kate.wood@hotmail.co.uk>
 *\version 1
 *\date 03 february 2017
 */



/** \file shaft.c
 *  This file contains the implementation of a lift shaft. Lift shafts contain one lift,
 *  and are primarily required to simplify the process of displaying the state of the
 *  simulation.
 *
 * \author Chris Page <chris@starforge.co.uk>
 * \date 4 July 2008
 * \version 1
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "shaft.h"

// Include a header needed for the print_shafts function if compiling on windows
#ifdef _WIN32
    #include <windows.h>
#endif


/* ============================================================================ *
 * Prototypes for functions only visible within this file                       *
 * ============================================================================ */

static const char *get_section(Shaft *shaft, int section);
static Lift *get_car(Shaft *shaft);


/* ============================================================================ *
 * Functions the student must fill in                                           *
 * ============================================================================ */

/** Call a lift to a floor. Checks all the available shafts for suitable lifts,
 *  select the lift that can get to the call in the shortest period, and set the
 *  call floor in the lift's stop list.
 *
 *  \param shafts     A pointer to a block of memory containing pointers to Shafts.
 *  \param shaftcount The number of shafts pointed to by 'shafts'
 *  \param tofloor    The floor the call was received on.
 *  \param direction  The direction the caller wants to go in.
 */
void call_lift(Shaft **shafts, int shaftcount, int tofloor, Moving direction)
{
    // you will need two variables to return the best positive and negative times,
    // and two variables to keep track of which shafts they correspond to. Set the
    // initial times to insane values (I'd suggest -32767 and 32768 respectively)
    // and set the best shaft numbers to something like -1 to indicate they haven't
    // been set
    printf ("\n entered call lift. \n");
    getchar();
    int bestpos_time = 32768;
    int bestneg_time = -32767;

    int bestpos_shaftnum = -1;
    int bestneg_shaftnum = -1;

    int i;
    int service_time;

    // Check with each lift to see whether it can service the call, we want the one with
    // the service time closest to zero, preferring negatives to positives:
    // If service time is negative, check it against the best negative time (ie: the time
    // closest to zero).If the current time is less than the recorded best, update the
    // recorded best and shaft number to be the current time and shaft.
    // If the service time is positive then the best lift to pick is the one
    // with the smallest service time left before it can get to the call

    for(i=0; i<shaftcount; i++)
    {
        service_time = service_call(shafts[i]->car, tofloor, direction);
        if(service_time < 0 && service_time > bestneg_time){
            bestneg_time = service_time;
            bestneg_shaftnum=i;
        }
        else if(service_time >= 0 && service_time < bestpos_time){
            bestpos_time = service_time;
            bestpos_shaftnum=i;
        }
    }
    // Pick the best shaft to use - if there is a best shaft for the negative time
    // (that is, the best shaft for the negative times is not -1) , use that
    // if there is no best shaft for the negative times, try the positives (for which
    // there should always be one - if both positive and negative shafts are -1, something
    // has gone Badly Wrong)

    // Obtain the car for the selected shaft, and set a call at tofloor in it

    if(bestneg_shaftnum != -1){
        set_stop(shafts[bestneg_shaftnum]->car, tofloor);
    }
    else if(bestpos_shaftnum != -1)
    {
        set_stop(shafts[bestpos_shaftnum]->car, tofloor);
    }
    else{
        printf("/nSomething has gone badly wrong!");
    }
}


/** Given an array of shaftcount Shaft pointers, update the finite state machine for
 *  each lift in the shafts.
 *
 *  \param shafts A pointer to a block of memory containing shaftcount pointers of Shaft structures.
 *  \param shaftcount The number of Shaft structures pointed to by 'shafts'.
 */
void update_shafts(Shaft **shafts, int shaftcount)
{
    int i;
    // for each shaft in shafts
    // get a pointer to the shaft's lift
    // call update_lift() on the lift.
    for (i = 0; i < shaftcount; i++) {
        update_lift(get_car(shafts[i]));
    }
}


/** Obtain a pointer to the car in the specified shaft. This will return a pointer
 *  to the lift contained within the provided shaft.
 *
 *  \param shaft The shaft containing the lift we are interested in.
 *  \return A pointer to the lift.
 */
static Lift *get_car(Shaft *shaft)
{
    return shaft -> car;
}


/* ============================================================================ *
 * Functions provided for use by the student                                    *
 * ============================================================================ */

/** Allocate and initialise a new Shaft structure. This will create a new shaft
 *  containing a Lift that can service calls from floor 0 to 'topfloor', and the
 *  lift moves at car_speed (note that car_speed must be an integer factor of
 *  FLOOR_HEIGHT or the simulation will break!)
 *
 *  \param topfloor The top floor that the lift in the shaft can service.
 *  \param car_speed The speed at which the car moves, in shaft sections per update.
 *  \return A pointer to a new Shaft structure.
 */
Shaft *create_shaft(int topfloor, int car_speed)
{
    char *buffer;
    int offset;

    // allocate a new shaft structure first
    Shaft *newshaft = (Shaft *)malloc(sizeof(Shaft));
    if(!newshaft) {
        fprintf(stderr, "unable to allocate space for a new shaft!\n");
        exit(1);
    }

    // And fill in the easy stuff
    newshaft -> car = create_lift(topfloor, car_speed);
    newshaft -> topfloor = topfloor;

    // What follows is a complete cheat to reduce memory fragmentation. Do not try
    // this at home without adult supervision. If this summons one of the Old Gods
    // of Computing then It's Not My Fault, Honest.

    // First, allocate enough space for all the 3 char + '\0' strings for the shaft
    // representation
    buffer = (char *)calloc((FLOOR_HEIGHT * topfloor) + 1, 4 * sizeof(char));
    if(!buffer) {
        fprintf(stderr, "Unable to allocate floorrep buffer.\n");
        free(newshaft);
        exit(1);
    }

    // Now allocate enough space for pointer for each step
    newshaft -> floorrep = (char **)malloc(((FLOOR_HEIGHT * topfloor) + 1) * sizeof(char *));
    if(!newshaft -> floorrep) {
        fprintf(stderr, "Unable to allocate floorrep pointer array\n");
        free(newshaft);
        free(buffer);
        exit(1);
    }

    // now set up pointers into the buffer
    for(offset = 0; offset <= (FLOOR_HEIGHT * topfloor); ++offset) {
        newshaft -> floorrep[offset] = buffer + (offset * 4); // 3 characters, plus '\0'
    }

    return newshaft;
}


/** Release the memory used by a shaft object. This will release all the memory used by
 *  the shaft, and the lift it contains.
 *
 *  \param release The shaft to free.
 */
void free_shaft(Shaft *release)
{
    // the first floorrep pointer is a pointer to the whole string block ('buffer' in create_shaft)
    free(release -> floorrep[0]);

    // Now release the pointers into the string block
    free(release -> floorrep);

    // And now the lift...
    free_lift(release -> car);

    // ... finally, the shaft itself
    free(release);
}


/** Obtain a section of a shaft's floor representation array. This will return a pointer to a
 *  3 character string that represents the shaft at the specified level.
 *
 *  \param shaft   The shaft to obtain the section from.
 *  \param section The section number to return, must be in the range 0 to topfloor*FLOOR_HEIGHT inclusive.
 *  \return A pointer to the string representing the shaft at the specified level.
 */
static const char *get_section(Shaft *shaft, int section)
{
    // If the section requested is over the top of the lift, return a 'no shaft' string
    if(section > ((shaft -> topfloor * FLOOR_HEIGHT) + 1)) {
        return "###";
    }

    return shaft -> floorrep[section];
}


/** Fill in the string representation of the specified shaft. Each shaft has its
 *  own 'floorrep' array which stores a string representation of each level in the
 *  shaft. This function fills in that array with the current status of the shaft.
 *
 *  \param current  The shaft to generate the string representation for.
 */
static void shaft_to_string(Shaft *current)
{
    int floorpos;

    // First, fill in the floor representation array with the 'normal' building info
    for(floorpos = 0; floorpos <= (FLOOR_HEIGHT * current -> topfloor); ++floorpos) {
        if((floorpos % FLOOR_HEIGHT == 0) && get_car(current) -> stops[floorpos / FLOOR_HEIGHT]) {
            strcpy(current -> floorrep[floorpos], "|!|");
        } else {
            strcpy(current -> floorrep[floorpos], "| |");
        }
    }

    // Now work out where the lift should be...
    int liftpos = get_position(current -> car);
    strcpy(current -> floorrep[liftpos], lift_to_string(current -> car));
}


/** Print out the shaft representations for the specified shafts. This will attempt
 *  to clear the terminal prior to printing out the shafts, and provision has been
 *  made for Windows terminals (which do not support the standard methods of clearing
 *  terminals). This code contains some OS-specific Magic which you are not expected
 *  to understand, or even read closely - feel free to treat this function as a
 *  black box.
 *
 *  \param shafts A pointer to a block of memory containing pointers to Shaft structures.
 *  \param shaftcount The number of shaft pointers in shafts.
 */
void print_shafts(Shaft **shafts, int shaftcount)
{
    int shaftnum, floornum, floorpos;
    int maxfloors = 0;

    // start by determining how many lines we need to output. In theory, all the
    // shafts will be the same height, but it's best to be certain...
    for(shaftnum = 0; shaftnum < shaftcount; ++shaftnum) {
        if(shafts[shaftnum] -> topfloor > maxfloors) {
            maxfloors = shafts[shaftnum] -> topfloor;
        }

        // may as well make the string representations while we're here
        shaft_to_string(shafts[shaftnum]);
    }

    // Now we need to clear the lift display area

#ifndef _WIN32  // If we're not compiling on windows...
    // This incantation clears the terminal, and moves the cursor to the top left
    // It should work on any Remotely Sane terminal that understands ANSI
    printf("\033[2J\033[H");

#else // otherwise, we are on windows...
    // So does this mess, the equivalent for windows (windows command terminals do
    // not support ANSI or VT100 escape code sequences, and operations that would
    // be performed via them on other operating systems must be done directly via
    // the Windows API.)

    // A bunch of variables first...
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = {0, 0};
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD written;

    // How many characters are there in the console?
    GetConsoleScreenBufferInfo(handle, &csbi);
    count = csbi.dwSize.X * csbi.dwSize.Y;

    // Now clear that many characters.
    // WARNING: This code is Full Of Spiders. You are not required or expected to
    // know what this is doing, and in fact your sanity is safter if you do not
    // attempt to.
    FillConsoleOutputCharacter(handle, ' ', count, coord, &written);
    GetConsoleScreenBufferInfo(handle, &csbi );
    FillConsoleOutputAttribute(handle, csbi.wAttributes, count, coord, &written);
    SetConsoleCursorPosition(handle, coord);
#endif

    // Now we get to the actual printing out part

    // Header first, showing shaft numbers...
    printf("    ");
    for(shaftnum = 0; shaftnum < shaftcount; ++shaftnum) {
        printf("%d   ", shaftnum);
    }
    printf("\n");

    // Now we want to print out. Remember that 0 is at the bottom of the terminal, not
    // at the top!
    for(floorpos = (FLOOR_HEIGHT * maxfloors); floorpos >= 0; -- floorpos) {
        // If the current position corresponds to a floor, print out a floor number
        if(floorpos % FLOOR_HEIGHT == 0) {
            floornum = floorpos / FLOOR_HEIGHT;
            printf("%2d ", floornum);

        // Otherwise print out a bunch of spaces to align the rest of the line
        } else {
            printf("   ");
        }

        // Go through each shaft we have been supplied with printing out the shaft
        // representation for the current level.
        for(shaftnum = 0; shaftnum < shaftcount; ++shaftnum) {
            printf("%s ", get_section(shafts[shaftnum], floorpos));
        }
        printf("\n");
    }

}


/** Prompt the user to enter a call floor, or to press return to contine. This will
 *  return the floor number the user requested, or NO_STOPS if the user just pressed
 *  the return key without entering anything.
 *
 *  \param topfloor The top floor that lifts can service.
 *  \return The floor number a call has been made on, or NO_STOPS if no calls are made.
 */
int request_call(int topfloor)
{
    char promptbuff[10];
    int request;

    printf("request call: Enter a number and press return to call a lift, or press return: ");

    // loop forever (the returns will break us out of this...)
    while(1) {
        // Wait for input from the user
        fgets(promptbuff, 10, stdin);
        // Does the string contain a number?
        if(string_to_int(promptbuff, &request)) {
            // Is the number in range?
            if(request >= 0 && request <= topfloor) {
                return request; // Yes, return the number
            } else {
                // The floor number is not in ranfe, prompt the user to try again
                printf("Floor out of range, try again: ");
            }

        // Otherwise, return NO_STOPS
        } else {
            return NO_STOPS;
        }
    }
}


/** Prompt the user to enter a direction for the call. This will wait for the user to enter
 *  either U, u, D, or D and return DIR_UP or DIR_DOWN depending on which character was entered.
 *  Any other input forces the function to reissue the prompt to the user to try again.
 *
 *  \param call_floor The floor the call was made at
 *  \param topfloor   The top floor the lift may stop at.
 *  \return DIR_UP if the user selects an up call (or the lift is at the bottom of the shaft,
 *          forcing and up call), or DIR_DOWN if the user selects a down call (or the lift is
 *          at the top of the shaft.)
 */
Moving request_direction(int call_floor, int topfloor)
{
    char promptbuffer[2];

    // can only go up from 0
    if(call_floor == 0) {
        return DIR_UP;
    }

    // can only go down from the top floor
    if(call_floor == topfloor) {
        return DIR_DOWN;
    }

    printf("Enter a lift direction for the call [U/D]: ");
    // Otherwise, request a direction
    while(1) {
        fgets(promptbuffer, 2, stdin);

        if(toupper(promptbuffer[0]) == 'U') {
            return DIR_UP;
        } else if(toupper(promptbuffer[0]) == 'D') {
            return DIR_DOWN;
        } else {
            printf("Direction not recognised. U = Up, D = Down: ");
        }
    }
}


/** Prompt the user to enter stops for the lifts. If one or more lifts
 *  are stopped at floors with their doors open then this will ask the user
 *  to enter a stop for the lift (or press return if no new stops should be
 *  added), once all the lifts have been checked the user is prompted to
 *  enter a call floor, or just press return to continue with the simulation.
 *  This function should be called in the main loop, after the lift FSM has
 *  been updated, and the status of the simulation printed to the user.
 *
 *  \param shafts A pointer to a block of memory containing pointers to Shaft structures.
 *  \param shaftcount The number of shafts pointed to within 'shafts'
 *  \param topfloor   The top floor that lifts can service.
 */
void prompt_user(Shaft **shafts, int shaftcount, int topfloor)
{
    int shaftnum;
    int request;

    // Start by checking whether any lifts are open
    for(shaftnum = 0; shaftnum < shaftcount; ++shaftnum) {
        if(get_state(get_car(shafts[shaftnum])) == STATE_OPEN) {
            request = request_stop(get_car(shafts[shaftnum]), shaftnum);

            if(request != NO_STOPS) {
                set_stop(get_car(shafts[shaftnum]), request);
            }
        }
    }

    // Now, ask for calls...
    request = request_call(topfloor);
    if(request != NO_STOPS) {
        call_lift(shafts, shaftcount, request, request_direction(request, topfloor));
    }
}
