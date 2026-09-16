#include "ride_hailing.h"
// gcc main.c avl.c functions.c loadandsave.c -o app -lm
static void banner(void)
{
    printf("\n");
    printf("  ===================================================\n");
    printf("  |             RIDE-HAILING SYSTEM                  |\n");
    printf("  |                                                  | \n");
    printf("  ===================================================\n");
}

static void menu(void)
{
    printf("\n");
    printf("  ===================================================\n");
    printf("  =              MAIN MENU                          =\n");
    printf("  ===================================================\n");
    printf("  =  1.  Add Driver                                 =\n");
    printf("  =  2.  Add Passenger                              =\n");
    printf("  =  3.  Request a Ride                             =\n");
    printf("  =  4.  Complete a Ride                            =\n");
    printf("  =  5.  Display Top 3 Drivers                      =\n");
    printf("  =  6.  Display Most Frequent Driver-Passenger Pair=\n");
    printf("  =  7.  Display Available Vehicles                 =\n");
    printf("  =  8.  Update Driver Location                     =\n");
    printf("  =  9.  Delete Driver                              =\n");
    printf("  =  10. Display Booking History                    =\n");
    printf("  =  11. Calculate Driver Earnings (by traversal)   =\n");
    printf("  =  12. Range Search Passengers (by ID range)      =\n");
    printf("  =  13. Cancel Ride                                 =\n");
    printf("  =  14. Save & Exit                                =\n");
    printf("  ==================================================\n");
    printf("  Enter choice: ");
}



// MAIN
int main(void)
{
    DriverNode *dRoot = NULL;
    PassengerNode *pRoot = NULL;
    BookingNode *bRoot = NULL;

    // load persisted data
    dRoot = loadDrivers();
    pRoot = loadPassengers();
    bRoot = loadBookings();

    banner();

    int choice;
    char name[MAX_NAME], mobile[MAX_MOBILE];
    int id, x, y, type, p_id, pref, booking_id, pid1, pid2;
    float dist;

   choice = 0;

do {
    menu();
    scanf("%d", &choice);

    switch (choice)
    {

    // 1. Add Driver
    case 1:
        printf("\n  [ADD DRIVER]\n");

        printf("  Driver ID   : ");
        scanf("%d", &id);
        if (id < 0) { printf("  [ERROR] ID cannot be negative\n"); break; }

        printf("  Name        : ");
        scanf(" %[^\n]", name);

        printf("  Vehicle (0=Cab, 1=Bike): ");
        scanf("%d", &type);
        if (type != 0 && type != 1) { printf("  [ERROR] Invalid type\n"); break; }

        printf("  Location X  : ");
        scanf("%d", &x);
        if (x < 0) { printf("  [ERROR] Invalid X\n"); break; }

        printf("  Location Y  : ");
        scanf("%d", &y);
        if (y < 0) { printf("  [ERROR] Invalid Y\n"); break; }

        addDriver(&dRoot, id, name, type, x, y);
        break;

    // 2. Add Passenger
    case 2:
        printf("\n  [ADD PASSENGER]\n");

        printf("  Passenger ID : ");
        scanf("%d", &id);
        if (id < 0) { printf("  [ERROR] ID cannot be negative\n"); break; }

        printf("  Name         : ");
        scanf(" %[^\n]", name);

        printf("  Mobile No    : ");
        scanf("%s", mobile);

        addPassenger(&pRoot, id, name, mobile);
        break;

    // 3. Request Ride
    case 3:
        printf("\n  [REQUEST RIDE]\n");

        printf("  Passenger ID : ");
        scanf("%d", &p_id);
        if (p_id < 0) { printf("  [ERROR] Invalid ID\n"); break; }

        printf("  Location X   : ");
        scanf("%d", &x);
        printf("  Location Y   : ");
        scanf("%d", &y);

        printf("  Preferred type (0=Cab,1=Bike,-1=Any): ");
        scanf("%d", &pref);

        requestRide(dRoot, pRoot, &bRoot, p_id, x, y, pref);
        break;

    // 4. Complete Ride
    case 4:
        printf("\n  [COMPLETE RIDE]\n");

        printf("  Booking ID   : ");
        scanf("%d", &booking_id);

        printf("  Distance (km): ");
        scanf("%f", &dist);
        if (dist < 0) { printf("  [ERROR] Distance cannot be negative\n"); break; }

        completeRide(dRoot, pRoot, bRoot, booking_id, dist);
        break;

    // 5–13 same as before (no input changes needed)

    case 5:
        displayTopDrivers(dRoot);
        break;

    case 6:
        displayFrequentPairs(dRoot, pRoot, bRoot);
        break;

    case 7:
        displayAvailableVehicles(dRoot);
        break;

    case 8:
        printf("\n  [UPDATE DRIVER LOCATION]\n");

        printf("  Driver ID : ");
        scanf("%d", &id);

        printf("  New X     : ");
        scanf("%d", &x);

        printf("  New Y     : ");
        scanf("%d", &y);

        if (x < 0 || y < 0) {
            printf("  [ERROR] Invalid coordinates\n");
            break;
        }

        updateDriverLocation(dRoot, id, x, y);
        break;

    case 9:
        printf("\n  [DELETE DRIVER]\n");

        printf("  Driver ID : ");
        scanf("%d", &id);

        deleteDriver(&dRoot, id);
        break;

    case 10:
        displayBookingHistory(bRoot);
        break;

    case 11:
        printf("\n  [DRIVER EARNINGS]\n");

        printf("  Driver ID : ");
        scanf("%d", &id);

        {
            float e = calculateDriverEarnings(dRoot, bRoot, id);
            if (e >= 0)
                printf("  Earnings: Rs %.2f\n", e);
        }
        break;

    case 12:
        printf("\n  [RANGE SEARCH]\n");

        printf("  From ID : ");
        scanf("%d", &pid1);

        printf("  To ID   : ");
        scanf("%d", &pid2);

        rangeSearchPassengers(pRoot, pid1, pid2);
        break;

    case 13:
{
    int booking_id;

    printf("Enter Booking ID to cancel: ");
    scanf("%d", &booking_id);

    cancelRide(dRoot, bRoot, booking_id);
    break;
}

    case 14:
        saveDrivers(dRoot);
        savePassengers(pRoot);
        saveBookings(bRoot);
        printf("\n  [DONE] Saved & Exiting\n");
        break;

    default:
        printf("  [WARN] Invalid choice\n");
    }

} while (choice != 14);

    return 0;
}
