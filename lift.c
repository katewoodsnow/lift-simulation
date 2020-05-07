/**\file lift.c
 * Submission for project 3

 * This program is the solution to project 3: It will implement the lift structure and contents

 *\author Kate Wood <kate.wood@hotmail.co.uk>
 *\version 1
 *\date 03 febuary 2017
 */


/** \file lift.c
 *  This file contains the functions required to create, manipulate, or inspect Lift
 *  structures. Lifts are the actual cars inside shafts - they have associated information
 *  such as their current position within the shaft, the speed they move at, the top floor
 *  they can stop at, and which floors they should stop and and open their doors.
 *
 *  The behaviour of lifts is determined primarily by a 'Finite State Machine' (FSM)
 *  implemented in the update_lift() function. A FSM is a system that consists of two
 *  things: state information, and rules about how and when to change that state
 *  information. The state information for the lifts is stored in the 'state', 'direction'
 *  and 'time' fields in the Lift structure, while the rules that implement the FSM
 *  are coded in the update_lift() function. The following is a description of the
 *  finite state machine that determines the behaviour of lifts:
 *
 *  <code>When created, 'state' starts out as STATE_IDLE, 'direction' starts as DIR_NONE, and 'time' at 0.
 *  Every time 'state' changes, 'time' is reset to zero.</code>
 *
 *  Each time update_lift() is called, the following should happen:
 *
 *  <pre>increase 'time' before anything else is done
 *  if 'state' is STATE_IDLE
 *       if the lift has been called to a floor (one or more entries in 'stops' is set)
 *           'state' changes to STATE_MOVING
 *           if the nearest stop is above the lift, 'direction' is set to DIR_UP
 *           if the nearest stop is below the lift, 'direction' is set to DIR_DOWN
 *  else if the lift is in STATE_MOVING
 *       if the lift is at a stop floor
 *           clear the stop marker on the floor
 *           'state' changes to STATE_OPENING
 *       otherwise
 *           move the lift
 *  else if 'state' is STATE_OPENING
 *       if 'time' is OPENING_TIME then 'state' changes to STATE_OPEN
 *  else if 'state' is STATE_OPEN
 *       if 'time' is OPEN_TIME then 'state' changes to STATE_CLOSING
 *  else if 'state' is STATE_CLOSING
 *       if 'time' is CLOSING_TIME then 'state' changes to STATE_WAIT
 *  else if 'state' is STATE_WAIT
 *       if 'time' is WAIT_TIME then
 *           if there are more stops the lift needs to go to in any direction
 *               'state' changes to STATE_MOVING
 *               if there are no more stops left in the current direction (above the lift for DIR_UP, below it for DIR_DOWN)
 *                   If the nearest stop is above the lift, 'direction' is set to DIR_UP
 *                   If the nearest stop is below the lift, 'direction' is set to DIR_DOWN
 *           otherwise
 *               'state' changes to STATE_IDLE
 *               'direction' changes to DIR_NONE
 *  else
 *       the finite state machine has entered an illegal state, print an error and exit</pre>
 *
 * IMPORTANT: Note that 'direction' should NOT be changed when changing to STATE_OPENING
 *            The only points at which direction should be changed are immediately following
 *            changing 'state' to STATE_MOVING, or when 'state' is set to STATE_IDLE.
 *
 * The Finite State Machine can be implemented either as a series of if() {} else if() {} else {}
 * statements, or as a switch(). The former may be easier to follow.
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
#include "lift.h"

/* ============================================================================ *
 * Prototypes for functions only visible within this file                       *
 * ============================================================================ */

static void time_tick(Lift *car);
static void move_lift(Lift *car);
static int nearest_stop(Lift *car, Moving constrain);
static int distance_to_last_stop(Lift *car);


/* ============================================================================ *
 * Functions the student must fill in                                           *
 * ============================================================================ */

/** Allocate a new Lift. This will allocate space for a new lift, and its
 *  internal list of stops.
 *
 *  \param topfloor The number of the top floor the lift can stop at.
 *  \param speed    The speed at which the lift moves.
 *  \return A pointer to an initialised idle Lift.
 */
Lift *create_lift(int topfloor, int speed)
{

    Lift *newlift = (Lift *)malloc(sizeof(Lift)); // Allocate space for a new lift.

    // Check memory
    if(!newlift) {
        fprintf(stderr, "Unable to allocate space for new lift.\n");
        exit(1);
    }

    // Allocate space for the stop marker array - which will occupy
    // (topfloors + 1) * sizeof(char) in memory - and set all the markers to zero. Store a pointer to the
    // marker in 'stops'.

    //Each Lift car contains a pointer to an array of stops - the stops field in
    //the Lift structure. That should be an area of memory allocated as part of
    //create_lift() that has enough space to store topfloor + 1 stop markers.
    //Initially it should be all zeros.

    newlift -> stops = (char *)calloc(topfloor + 1, sizeof(char));

    /* Check there is enough memory */
    if (!newlift -> stops){
        fprintf(stderr, "Unable to allocate space for the stop marker array.\n");
        exit(1);
    }

    //Store the top floor number in the Lift, and fill in the other fields
    // (Lifts start out idle, not moving, with 0 time, and on the ground floor. Store the lift
    // speed,
    newlift -> topfloor = topfloor;
    newlift -> direction = DIR_NONE;
    newlift -> state = STATE_IDLE;
    newlift -> time = 0;
    newlift -> position = 0;
    newlift -> speed = speed;

    return newlift;                              //return a pointer to the new Lift structure.
}


/** Release the resources used by a Lift. This will free the lift stop list, and
 *  the memory allocated for the Lift.
 *
 *  \param car A pointer to the Lift to free.
 */
void free_lift(Lift *car)
{
    free(car -> stops);
    free(car);
}


/** Set the direction the lift is moving in.
 *
 * \param car A pointer to the lift to modify.
 * \param direction The direction in which the lift should be moving. Valid values are
 *                  DIR_NONE to stop the lift, DIR_DOWN to move the lift down and DIR_UP
 *                  to move the lift upwards.
 */
void set_direction(Lift *car, Moving direction)
{
    car -> direction = direction;
}


/** Obtain the direction the specified lift is moving in. The value returned will be
 *  either DIR_NONE if the lift is stationary, DIR_UP if it is rising, and DIR_DOWN
 *  if it is falling.
 *
 *  \param car A pointer the the lift to inspect.
 *  \return The direction that the lift is moving in.
 */
Moving get_direction(Lift *car)
{
    return car -> direction;
}


/** Set the current state of the specified lift's finite state machine, resetting its
 *  time to zero in the process.
 *
 *  \param car The lift to modify.
 *  \param state The new state the FSM should be set to.
 */
void set_state(Lift *car, State state)
{
    car -> state = state;
}


/** Obtain the current state of the specified lift's finite state machine.
 *
 *  \param car The lift to inspect.
 *  \return The state the lift is currently in.
 */
State get_state(Lift *car)
{
    return car -> state;
}


/** Obtain the speed at which the lift moves per update.
 *
 *  \param car A pointer to the lift to inspect.
 *  \return The speed at which the lift moves.
 */
int get_speed(Lift *car)
{
    return car -> speed;
}


/** Obtain the current value of the specified lift's timer.
 *
 *  \param car A pointer to the lift to inspect.
 *  \return The current value of the lift's timer.
 */
int get_time(Lift *car)
{
    return car -> time;
}


/** Increase the specified lift's timer.
 *
 *  \param car A pointer to the lift to modify.
 */
static void time_tick(Lift *car)
{
        car -> time++;
}

/** Obtain the lift's position in the lift shaft. Note that the value
 *  returned by this is in the range 0 to topfloor * FLOOR_HEIGHT - it
 *  is <i>not</i> the floor number of the lift!
 *
 *  \param car A pointer to the lift to inspect.
 *  \return The position of the lift in the shaft.
 */
int get_position(Lift *car)
{
    return car -> position;
}


/** Set the location of the lift in the shaft. The value should be
 *  in the range 0 to topfloor * FLOOR_HEIGHT - <i>be careful not to
 *  just set this to a floor number</i>. If you want to set the lift
 *  to be on the third floor, then you should set the position to
 *  3 * FLOOR_HEIGHT.
 *
 *  \param car A pointer to the lift to move.
 *  \param position The position of the lift in the shaft.
 */
void set_position(Lift *car, int position)
{
    car -> position = position;
}


/** Move the lift up or down the shaft. If the lift direction is DIR_UP
 *  then the lift will move upwards by get_speed(car), if the direction
 *  is DIR_DOWN then the lift will move downwards by get_speed(car).
 *
 *  \param car A pointer to the car to move.
 */
static void move_lift(Lift *car)
{
    if(get_direction(car) == DIR_UP){
        set_position(car, get_position(car) + get_speed(car));
    } else if(get_direction(car) == DIR_DOWN){
        set_position(car, get_position(car) - get_speed(car));
    }
}
/** Obtain the top floor number that the lift serves.
 *
 *  \return The top floor the lift goes to.
 */
int get_topfloor(Lift *car)
{
    return car -> topfloor;
}


/** Mark a floor as one at which the lift should stop. Note that the specified
 *  value Should be the <i>floor number</i> at which the lift should stop,
 *  <b>not</b> a shaft position (ie: it should be in the range 0 to topfloor
 *  inclusive, not 0 to topfloor * FLOOR_HEIGHT).
 *
 *  \param car The lift to set a stop floor for.
 *  \param floor The floor the lift should stop at.
 */
void set_stop(Lift *car, int floor)
{
    car -> stops[floor] = 1;
}


/** Remove a stop marker from the Lift's list of stops. As with set_stop() the floor
 *  provided should be a floor number, not a lift shaft position.
 *
 *  \param car The lift to clear a dtop floor from.
 *  \param floor The floor to remove from the stop list.
 */
void clear_stop(Lift *car, int floor)
{
    car -> stops[floor] = 0;
}


/** Determine whether the lift is at a stop floor. This returns true
 *  if the lift is at a floor, and that floor is the nearest stop,
 *  and false otherwise.
 *
 *  \param car The lift to inspect.
 *  \return true if the lift is at a stop floor, false if it is not.
 */
int at_stop(Lift *car)
{
    int current_floor = at_floor(car);

    if(current_floor!=NOT_AT_FLOOR){
        if (car->stops[current_floor] == 1){  //1 = stopmarker
            return 1;
            }
    }
    return 0;
}


/** Can this lift easily service a lift call? If it's idle, it can always service
 *  it. If it's moving towards the floor the call is on, then it can service the
 *  call. Otherwise this should return the time it would take to service the call.
 *
 *  \param car        The lift to inspect
 *  \param call_floor The floor a call has been made on
 *  \param direction  The direction the caller wants to go in
 *  \return How long (in simulation steps) it would take the specified lift to
 *          service the call. If the lift can service the call easily (it is
 *          idle, or moving towards the call) then the value returned is
 *          -1 * the time to service. If the lift can not easily service the call
 *          (it is moving away from it) then the value returned is the time
 *          to service.
 */
int service_call(Lift *car, int call_floor, Moving direction)
{
    // First calculate the distance between the car and call floor
    int distance = (call_floor * FLOOR_HEIGHT) - get_position(car);
    int time_to_service = (distance/get_speed(car));
    // Idle cars can always service calls, regardless of direction or
    // floor. Determine how far the car is from the call floor and return the time
    // to service * -1. Note that the time to service is the distance between the
    // lift and call divided by the speed of the lift!
    if (get_state(car) == STATE_IDLE){
        return time_to_service * CAN_SERVICE;
    }

    // Now, if the lift is moving in the direction of the call already, is it
    // moving towards the call floor?
    // If the lift is below the call, and moving up, it can service it - return the time to service * -1
    // If the lift is above the call, and moving down, it can service the call - return the time to service * -1

    // otherwise, we want to return twice the sum of the time it would take for the lift
    // to return to its current position (ie: service its last call, and get back to where
    // it is now) plus the time it would take to get from the current position to the call
    direction = get_direction(car);

    if ((distance > 0 && direction == DIR_UP) || (distance < 0 && direction == DIR_DOWN)) {
            return time_to_service * CAN_SERVICE; //-
    }else{
         return ((distance_to_last_stop(car)/get_speed(car))*2) + time_to_service;
    }
}


/** Update the finite state machine for the specified lift. This is the function that
 *  actually defines the behaviour of the lift, and it should implement the finite state
 *  machine described in the file header comment and project description.
 *
 *  \param car A pointer to the lift to update.
 */
void update_lift(Lift *car)
{
    int lift_called = 0;
    int i;
    // Implement the FSM as described in the header here
    //<pre>increase 'time' before anything else is done

    time_tick(car);

    // if 'state' is STATE_IDLE
    if (get_state(car) == STATE_IDLE) {
        //if the lift has been called to a floor (one or more entries in 'stops' is set)
        for(i=0; i<=get_topfloor(car); i++) {
            if(car->stops[i] == 1){
                lift_called = 1;
            }
        }

        if (lift_called){
            set_state(car, STATE_MOVING);

            //if the nearest stop is above the lift, 'direction' is set to DIR_UP
            if (nearest_stop (car, DIR_UP)) {
                set_direction(car, DIR_UP);
            }
            //if the nearest stop is below the lift, 'direction' is set to DIR_DOWN
            if (nearest_stop (car, DIR_DOWN)) {
                set_direction(car, DIR_DOWN);
            }
        }
    }
    // else if the lift is in STATE_MOVING
    else if (get_state(car) == STATE_MOVING) {
        //if the lift is at a stop floor
        if (at_stop(car)) {
            //clear the stop marker on the floor
            clear_stop(car, (get_position(car)/FLOOR_HEIGHT));
            //'state' changes to STATE_OPENING
            set_state(car, STATE_OPENING);
        }
        else {
            //move the lift
            move_lift(car);
        }
    }
    //else if 'state' is STATE_OPENING
    else if (get_state(car) == STATE_OPENING) {
        //if 'time' is OPENING_TIME then 'state' changes to STATE_OPEN
        if (get_time(car) == OPENING_TIME) {
            set_state(car, STATE_OPEN);
        }
    }
    //else if 'state' is STATE_OPEN
    else if (get_state(car) == STATE_OPEN) {
        //if 'time' is OPEN_TIME then 'state' changes to STATE_CLOSING
        if (get_time(car) == OPEN_TIME) {
            set_state(car, STATE_CLOSING);
        }
    }
    //else if 'state' is STATE_CLOSING
    else if (get_state(car) == STATE_CLOSING) {
        //if 'time' is CLOSING_TIME then 'state' changes to STATE_WAIT
        if (get_time(car) == CLOSING_TIME) {
            set_state(car, STATE_WAIT);
        }
    }
    //else if 'state' is STATE_WAIT
    else if (get_state(car) == STATE_WAIT) {
        //if 'time' is WAIT_TIME then
        if (get_time(car) == WAIT_TIME) {
            //if there are more stops the lift needs to go to in any direction
            if (nearest_stop(car, DIR_NONE) != NO_STOPS) {
                //'state' changes to STATE_MOVING
                set_state(car, STATE_MOVING);
                    //if there are no more stops left in the current direction (above the lift for DIR_UP, below it for DIR_DOWN)
                    if (nearest_stop(car, get_direction(car))== NO_STOPS)
                    {
                       //If the nearest stop is above the lift, 'direction' is set to DIR_UP
                        if (get_direction(car) == DIR_UP) {
                            set_direction(car, DIR_DOWN);
                        }
                       //If the nearest stop is below the lift, 'direction' is set to DIR_DOWN
                        if (get_direction(car) == DIR_DOWN) {
                            set_direction(car, DIR_UP);
                        }
                    }
            }
            else {
                //'state' changes to STATE_IDLE
                set_state(car, STATE_IDLE);
                //'direction' changes to DIR_NONE
                set_direction(car, DIR_NONE);
            }
        }
    }
    else{
        //the finite state machine has entered an illegal state, print an error and exit</pre>
        fprintf(stderr, "the finite state machine has entered an illegal state.\n");
        getchar;
        exit(1);
    }
}

/** Prompt the user to enter a stop floor for a lift with open doors.

This will
 *  print a message indicating that the doors are open (eg: "Lift 2 doors are open")
 *  and, if the lift has a direction set it will append a direction indicator
 *  (", the lift is going up"). Then the user is prompted to enter a floor number
 *  and press return, or just press return to skip floor selection. If the user
 *  enters a valid floor, this returns the floor number, otherwise it returns
 *  NO_STOPS.
 *
 *  \param car The lift that is awaiting a destination floor.
 *  \param shaftnum The number of the lift shaft that houses the lift.
 *  \return The floor to stop at, or NO_STOPS if no floor has been selected.
 */


int request_stop(Lift *car, int shaftnum)
{

    char promptbuff[10];
    int request;

    if (get_state(car) == STATE_OPEN) {
        printf ("Lift %d doors are open", shaftnum);

        if (get_direction(car)==DIR_UP) {
            printf (", the lift is going up");
        }
        if (get_direction(car)==DIR_DOWN) {
            printf (", the lift is going down");
        }

        printf("Enter a floor number and press return, or just press return to skip floor selection: ");

        // Wait for input from the user
        fgets(promptbuff, 10, stdin);

        // Does the string contain a number?
        if(string_to_int(promptbuff, &request)) {
            // Is the number in range?
            if(request >= 0 && request <= car->topfloor) {
                return request; // Yes, return the number
            }
        }
    }
    return NO_STOPS;
}


/* ============================================================================ *
 * Functions provided for use by the student                                    *
 * ============================================================================ */

/** Convert a string into an integer. This will attempt to convert the provided
 *  string into an int, and store the result in the variable pointed to by the
 *  second argument. If the conversion is successful then the variable is changed
 *  and the function return true. If the conversion fails (the string does not
 *  contain, or at least start with, a digit) the variable is not changed, and
 *  this function returns false.
 *
 *  \param string The string to convert to an int.
 *  \param value  A pointer to an int into which the converted value should be stored.
 *  \return true if the variable has been set to contain a converted value, false
 *          if the conversion failed and the variable has not been changed.
 */
int string_to_int(char *string, int *value)
{
    char *inc, *outc;

    // Allocate space for a copy...
    char *nospaces = (char *)malloc(strlen(string) + 1);
    if(!nospaces) {
        fprintf(stderr, "Unable to allocate space for temporary string storage!\n");
        exit(1);
    }

    // only copy over spaces...
    inc = string; outc = nospaces;
    while(*inc) {
        if(!isspace(*inc)) {
            *outc++ = *inc;
        }
        ++inc;
    }
    *outc = '\0'; // make sure the output is nul terminated

    // Is anything left?
    if(strlen(nospaces) > 0) {
        char *endptr;
        // Do the conversion into an int
        int parsed = (int)strtol(nospaces, &endptr, 10);

        // If endptr is not the same as nospaces, at least
        // some kind of number could be parsed out.
        if(endptr != nospaces) {
            *value = parsed;
            free(nospaces);
            return 1;
        }
    }

    free(nospaces);
    return 0;
}


/** Determine whether a lift is at a floor, and return the floor number if
 *  it is. If the lift is not currently at a floor - it is between floors -
 *  then this will return NOT_AT_FLOOR instead of a floor
 *
 *  \param car The Lift to inspect.
 *  \return The floor number, or NOT_AT_FLOOR if the lift is not at a floor.
 */
int at_floor(Lift *car)
{
    int current = get_position(car);
    // If the lift position can be divided by floor height with no remainder,
    // then the lift car is at a floor
    if(current % FLOOR_HEIGHT == 0) {
        return (current / FLOOR_HEIGHT);
    }

    // Otherwise, we aren't at a floor yet.
    return NOT_AT_FLOOR;
}


/** Obtain the nearest stop floor to the current location of the lift. If a direction
 *  of travel is specified then this will only search for stops in the direction of
 *  travel, otherwise the search will look in either direction from the lift.
 *
 *  \param car The lift to inspect.
 *  \param constrain DIR_NONE to search in any direction, DIR_UP to search above the
 *                   lift, or DIR_DOWN to search below the lift.
 *  \return The floor number of the nearst stop request, or NO_STOPS if none are
 *          found in the specified direction.
 */
static int nearest_stop(Lift *car, Moving constrain)
{
    int start = get_position(car);
    int end, move;

    // If we need to move upward, set tne end to the top of the shaft, and the move
    // direction to be upward
    if(constrain == DIR_UP) {
        end  = (car -> topfloor * FLOOR_HEIGHT) + 1;
        move = 1;
    }

    // If we need to move downward, set the end to the bottom of the shaft, and the
    // move direction to be downward
    if(constrain == DIR_DOWN) {
        end = -1;
        move = -1;
    }

    // If we are moving either up or down, search...
    if(constrain != DIR_NONE) {
        for(; start !=  end; start += move) {
            // If we're on a floor, check if there is a stop there...
            if((start % FLOOR_HEIGHT == 0) && car -> stops[start / FLOOR_HEIGHT]) {
                return start / FLOOR_HEIGHT;
            }
        }
    } else {
        // check above and below, and work out where the nearest stop is
        int above = nearest_stop(car, DIR_UP);
        int below = nearest_stop(car, DIR_DOWN);

        // We can stop right here if there are no stops above or below
        if(above == NO_STOPS && below == NO_STOPS) {
            return NO_STOPS;

        // If there are no stops below, there must be one above
        } else if(below == NO_STOPS) {
            return above;

        // otherwise, there must be stops below
        } else {
            return below;
        }
    }

    // Get here, and there are no stops for this lift.
    return NO_STOPS;
}


/** Determine the distance to the last stop this car has to service.
 *  If the lift is idle, or no stops remain in its direction of travel
 *  then this should return 0.
 *
 *  \param car The lift to inspect.
 *  \return The distance to the last stop the lift has to go to, or
 *          zero if there are no more stops left.
 */
static int distance_to_last_stop(Lift *car)
{
    int end = get_position(car);
    int start, move;

    // If the lift is idle, we can return immediately
    if(get_state(car) == STATE_IDLE) {
        return 0;
    }

    // if the lift is going down, search from the ground up
    if(get_direction(car) == DIR_DOWN) {
        start = 0;
        move  = 1;

    // If the lift is going up, search from the top down
    } else if(get_direction(car) == DIR_UP) {
        start = car -> topfloor * FLOOR_HEIGHT;
        move  = -1;

    // If there is no direction, return 0 - this should not happen...
    } else {
        return 0;
    }

    // Locate the first stop we come across
    for(; start != end; start += move) {
        // Are we at a floor, and is there a stop there?
        if((start % FLOOR_HEIGHT == 0) && car -> stops[start / FLOOR_HEIGHT]) {
            break; // yes, break immediately
        }
    }

    // In theory, start is now either on the floor where the most distant stop is,
    // or it is on the same level as the car (in which case there either are no
    // more stops, or the car is at its last stop now) This means we can easily
    // just take the absolute of the difference of the start and end, and we're good...
    return abs(end - start);
}


/** Produce a string representation of the Lift, encoding its current state in a
 *  recognisible way. This string is guaranteed to be 3 characters in length (plus
 *  the nul terminator). You may not directly modify the string returned by this
 *  function, and it does not need to be free()ed.
 *
 *  \param car A pointer to the lift to produce a string representation for.
 *  \return A pointer to a string of characters encoding the Lift state.
 */
const char *lift_to_string(Lift *car)
{
    // This could probably be done with an array of strings, but it works well enough...
    static const char *going_up   = "[^]";
    static const char *going_down = "[v]";
    static const char *idle       = "[?]";
    static const char *opening    = "< >";
    static const char *open       = "] [";
    static const char *closing    = "> <";
    static const char *waiting    = "[|]";
    static const char *badstate   = "BAD";

    // Use the car state to determine which string to return.
    switch(get_state(car)) {
        case STATE_MOVING: if(get_direction(car) == DIR_DOWN) {
                               return going_down;
                           } else if(get_direction(car) == DIR_UP) {
                               return going_up;
                           }
                           // If the directions is not _UP or _DOWN then fall through to STATE_IDLE
        case STATE_IDLE   : return idle;    break;
        case STATE_OPENING: return opening; break;
        case STATE_OPEN   : return open;    break;
        case STATE_CLOSING: return closing; break;
        case STATE_WAIT   : return waiting; break;
        default: return badstate;
    }
}