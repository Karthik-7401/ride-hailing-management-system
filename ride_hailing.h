#ifndef RIDE_HAILING_H
#define RIDE_HAILING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*  Constants  */
#define MAX_NAME 64
#define MAX_MOBILE 16
#define RADIUS 5.0
#define CAB_RATE 10.0f
#define BIKE_RATE 5.0f
#define DRIVER_FILE "drivers.txt"
#define PASS_FILE "passengers.txt"
#define BOOKING_FILE "bookings.txt"
#define META_FILE "meta.txt"

/* Location */
typedef struct
{
    int x, y;
} Location;

/*  Driver AVL Node */
typedef struct DriverNode
{
    int d_ID;
    char name[MAX_NAME];
    int vehicle_type; /* 0 = cab, 1 = bike */
    Location current_location;
    int status; /* 0 = free, 1 = booked */
    float total_earnings;
    int height;
    struct DriverNode *left, *right;
} DriverNode;

/*  Passenger AVL Node  */
typedef struct PassengerNode
{
    int p_ID;
    char name[MAX_NAME];
    char mobile_no[MAX_MOBILE];
    int frequency;
    int height;
    struct PassengerNode *left, *right;
} PassengerNode;

/* Booking AVL Node */
typedef struct BookingNode
{
    int booking_id;
    int d_ID;
    int p_ID;
    int vehicle_type;
    float distance_travelled;
    float fare;
    int timestamp;
    int status;  /* 0 = requested, 1 = completed, 2 = cancelled */
    int height;
    struct BookingNode *left, *right;
} BookingNode;

//--------------------------------------------------------------------------------------------------------------------------------------------
/*  AVL Helpers  */
int avl_height(void *node);

/* Driver AVL */
DriverNode *new_driver(int id, char name[], int type, int x, int y);
DriverNode *insert_driver(DriverNode *root, DriverNode *node);
DriverNode *search_driver(DriverNode *root, int id);
DriverNode *delete_driver(DriverNode *root, int id);
void inorder_driver(DriverNode *root, void (*visit)(DriverNode *));

/* Passenger AVL */
PassengerNode *new_passenger(int id, char name[], char mobile[]);
PassengerNode *insert_passenger(PassengerNode *root, PassengerNode *node);
PassengerNode *search_passenger(PassengerNode *root, int id);
PassengerNode *search_passenger_mobile(PassengerNode *root, char mobile[]);
void inorder_passenger(PassengerNode *root, void (*visit)(PassengerNode *));
void range_passenger(PassengerNode *root, int lo, int hi);

/* Booking AVL */
BookingNode *new_booking(int bid, int did, int pid, int vtype, int ts);
BookingNode *insert_booking(BookingNode *root, BookingNode *node);
BookingNode *search_booking(BookingNode *root, int bid);
void inorder_booking(BookingNode *root, void (*visit)(BookingNode *));
void cancelRide(DriverNode *dRoot, BookingNode *bRoot, int booking_id);
//--------------------------------------------------------------------------------------------------------------------------------------------
/*  Core Functions */
void addDriver(DriverNode **root, int id, char name[], int type, int x, int y);
void addPassenger(PassengerNode **root, int id, char name[], char mobile[]);
DriverNode *findNearestVehicle(DriverNode *root, int p_x, int p_y, int prefType);
int requestRide(DriverNode *dRoot, PassengerNode *pRoot, BookingNode **bRoot,
                int p_id, int p_x, int p_y, int prefType);
void completeRide(DriverNode *dRoot, PassengerNode *pRoot, BookingNode *bRoot,
                  int booking_id, float distance);
float calculateDriverEarnings(DriverNode *dRoot, BookingNode *bRoot, int d_id);
void displayTopDrivers(DriverNode *dRoot);
void displayFrequentPairs(DriverNode *dRoot, PassengerNode *pRoot, BookingNode *bRoot);
void displayAvailableVehicles(DriverNode *root);
void updateDriverLocation(DriverNode *root, int d_id, int new_x, int new_y);
void deleteDriver(DriverNode **root, int d_id);
void displayBookingHistory(BookingNode *root);
void rangeSearchPassengers(PassengerNode *root, int pid1, int pid2);
//--------------------------------------------------------------------------------------------------------------------------------------------

void saveDrivers(DriverNode *root);
void savePassengers(PassengerNode *root);
void saveBookings(BookingNode *root);
DriverNode *loadDrivers(void);
PassengerNode *loadPassengers(void);
BookingNode *loadBookings(void);

/*  Globals (booking counter) */
extern int booking_counter;
extern int sim_timestamp;

#endif
