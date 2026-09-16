#include "ride_hailing.h"
#include <float.h>

int booking_counter = 1;
int sim_timestamp = 1000;

/* 1. addDriver */
void addDriver(DriverNode **root, int id, char name[], int type, int x, int y)
{
    if (type != 0 && type != 1)
    {
        printf("[ERROR] Invalid vehicle type %d. Use 0 (cab) or 1 (bike).\n", type);
        return;
    }
    if (search_driver(*root, id))
    {
        printf("[ERROR] Driver with ID %d already exists.\n", id);
        return;
    }
    DriverNode *node = new_driver(id, name, type, x, y);
    *root = insert_driver(*root, node);
    printf("[OK] Driver '%s' (ID=%d, %s) added at (%d,%d).\n",
           name, id, type ? "Bike" : "Cab", x, y);

    saveDrivers(*root);
}

/* 2. addPassenger*/
void addPassenger(PassengerNode **root, int id, char name[], char mobile[])
{
    if (search_passenger(*root, id))
    {
        printf("[ERROR] Passenger with ID %d already exists.\n", id);
        return;
    }
    if (search_passenger_mobile(*root, mobile))
    {
        printf("[ERROR] Passenger with mobile %s already exists.\n", mobile);
        return;
    }
    PassengerNode *node = new_passenger(id, name, mobile);
    *root = insert_passenger(*root, node);
    printf("[OK] Passenger '%s' (ID=%d) registered.\n", name, id);

    savePassengers(*root);
}

/* 3. findNearestVehicle*/
/* We do inorder traversal collecting best candidate */
static DriverNode *best_driver = NULL;
static double best_dist = DBL_MAX;
static int pref_type_g;
static int px_g, _py_g;

static void check_driver(DriverNode *d)
{
    if (d->status != 0)
        return;
    if (pref_type_g != -1 && d->vehicle_type != pref_type_g)
        return;
    double dx = d->current_location.x - px_g;
    double dy = d->current_location.y - _py_g;
    double dist = sqrt(dx * dx + dy * dy);
    if (dist <= RADIUS && dist < best_dist)
    {
        best_dist = dist;
        best_driver = d;
    }
}

DriverNode *findNearestVehicle(DriverNode *root, int p_x, int p_y, int prefType)
{

    best_driver = NULL;
    best_dist = DBL_MAX;
    pref_type_g = prefType;
    px_g = p_x;
    _py_g = p_y;
    inorder_driver(root, check_driver);
    return best_driver;
}

/* 4. requestRide*/
int requestRide(DriverNode *dRoot, PassengerNode *pRoot, BookingNode **bRoot,
                int p_id, int p_x, int p_y, int prefType)
{
    PassengerNode *p_node = search_passenger(pRoot, p_id);

    if (!p_node)
    {
        printf("[ERROR] Passenger ID %d not found in database.\n", p_id);
        return -1;
    }

    DriverNode *d_node = findNearestVehicle(dRoot, p_x, p_y, prefType);

    if (!d_node)
    {
        printf("[INFO] No available %s within %.0f km of (%d,%d).\n",
               prefType == 0 ? "cab" : prefType == 1 ? "bike" : "vehicle",
               RADIUS, p_x, p_y);
        return -1;
    }

    int bid = booking_counter++;

    BookingNode *b = new_booking(
        bid,
        d_node->d_ID,
        p_id,
        d_node->vehicle_type,
        sim_timestamp++
    );

    *bRoot = insert_booking(*bRoot, b);

    d_node->status = 1;

    printf("[BOOKED] BookingID=%d | Driver: %s (ID=%d, %s) assigned to Passenger: %s (ID=%d)\n",
           bid,
           d_node->name,
           d_node->d_ID,
           d_node->vehicle_type ? "Bike" : "Cab",
           p_node->name,
           p_id);

    // Save updated data
    saveBookings(*bRoot);
    saveDrivers(dRoot);
    savePassengers(pRoot);

    return bid;
}

/* 5A. cancelRide */
void cancelRide(DriverNode *dRoot, BookingNode *bRoot, int booking_id)
{
    BookingNode *b = search_booking(bRoot, booking_id);

    if (!b)
    {
        printf("[ERROR] Booking ID %d not found.\n", booking_id);
        return;
    }

    /* Ride must still be active */
    if (b->status != 0)
    {
        printf("[ERROR] Booking %d is not an active ride.\n", booking_id);
        return;
    }

    /* Mark booking as cancelled */
    b->status = 2;

    /* Make driver available again */
    DriverNode *d = search_driver(dRoot, b->d_ID);

    if (d)
        d->status = 0;

    printf("[CANCELLED] Booking %d cancelled successfully.\n", booking_id);

    if (d)
        printf("Driver %s (ID=%d) is now available.\n",
               d->name, d->d_ID);

    /* Save updated data */
    saveBookings(bRoot);
    saveDrivers(dRoot);
}

static void updatePair(int d, int p);

/* 5. completeRide*/
void completeRide(DriverNode *dRoot, PassengerNode *pRoot,
                  BookingNode *bRoot, int booking_id, float distance)
{
    BookingNode *b = search_booking(bRoot, booking_id);

    if (!b)
    {
        printf("[ERROR] Booking ID %d not found.\n", booking_id);
        return;
    }

    if (b->status != 0)
    {
        printf("[ERROR] Booking %d is not an active ride.\n", booking_id);
        return;
    }

    if (distance <= 0)
    {
        printf("[ERROR] Distance must be greater than 0.\n");
        return;
    }

    float rate = (b->vehicle_type == 0) ? CAB_RATE : BIKE_RATE;
    float fare = distance * rate;

    b->distance_travelled = distance;
    b->fare = fare;
    b->status = 1;

    DriverNode *d = search_driver(dRoot, b->d_ID);

    if (d)
    {
        d->total_earnings += fare;
        d->status = 0;
    }

    PassengerNode *p = search_passenger(pRoot, b->p_ID);

    if (p)
        p->frequency++;

    updatePair(b->d_ID, b->p_ID);

    printf("[COMPLETED] Booking %d | Distance: %.2f km | Fare: Rs %.2f | Driver: %s | Passenger: %s\n",
           booking_id,
           distance,
           fare,
           d ? d->name : "N/A",
           p ? p->name : "N/A");

    saveBookings(bRoot);
    saveDrivers(dRoot);
    savePassengers(pRoot);
}

/* 6. calculateDriverEarnings*/
static float earn_sum;
static int earn_did;

static void sum_booking(BookingNode *b)
{
    if (b->d_ID == earn_did)
        earn_sum += b->fare;
}

float calculateDriverEarnings(DriverNode *dRoot, BookingNode *bRoot, int d_id)
{
    if (!search_driver(dRoot, d_id))
    {
        printf("[ERROR] Driver ID %d not found.\n", d_id);
        return -1.0f;
    }
    earn_sum = 0.0f;
    earn_did = d_id;
    inorder_booking(bRoot, sum_booking);
    return earn_sum;
}

/* 7. displayTopDrivers*/
typedef struct
{
    int d_ID;
    char name[MAX_NAME];
    float earn;
} DriverTop;

// Top 3 drivers
static DriverTop top1 = {0}, top2 = {0}, top3 = {0};

// Helper: insert into top-3
static void updateTop3(DriverNode *d)
{
    float e = d->total_earnings;

    if (e > top1.earn)
    {
        top3 = top2;
        top2 = top1;
        top1.d_ID = d->d_ID;
        strncpy(top1.name, d->name, MAX_NAME - 1);
        top1.name[MAX_NAME - 1] = '\0';
        top1.earn = e;
    }
    else if (e > top2.earn)
    {
        top3 = top2;
        top2.d_ID = d->d_ID;
        strncpy(top2.name, d->name, MAX_NAME - 1);
        top2.name[MAX_NAME - 1] = '\0';
        top2.earn = e;
    }
    else if (e > top3.earn)
    {
        top3.d_ID = d->d_ID;
        strncpy(top3.name, d->name, MAX_NAME - 1);
        top3.name[MAX_NAME - 1] = '\0';
        top3.earn = e;
    }
}

void displayTopDrivers(DriverNode *root)
{

    top1.earn = top2.earn = top3.earn = -1.0f;

    // Traverse AVL
    inorder_driver(root, updateTop3);

    if (top1.earn < 0)
    {
        printf("[INFO] No drivers found.\n");
        return;
    }

    printf("\n=== TOP DRIVERS ===\n");

    if (top1.earn >= 0)
        printf("#1  %-20s  Rs %.2f\n", top1.name, top1.earn);

    if (top2.earn >= 0)
        printf("#2  %-20s  Rs %.2f\n", top2.name, top2.earn);

    if (top3.earn >= 0)
        printf("#3  %-20s  Rs %.2f\n", top3.name, top3.earn);
}

/* 8. displayFrequentPairs */
#define TABLE_SIZE 4096

typedef struct PairNode
{
    int d_ID;
    int p_ID;
    int count;
    struct PairNode *next;
} PairNode;

static PairNode *htable[TABLE_SIZE];

// hash function
static int hash(int d, int p)
{
    unsigned int key;

    key = (unsigned int)d * 31u + (unsigned int)p;

    return key % TABLE_SIZE;
}

// Insert or update pair
static void updatePair(int d, int p)
{
    int h = hash(d, p);
    PairNode *curr = htable[h];

    while (curr != NULL)
    {
        if (curr->d_ID == d && curr->p_ID == p)
        {
            curr->count++;
            return;
        }

        curr = curr->next;
    }

    PairNode *node = (PairNode *)malloc(sizeof(PairNode));

    if (!node)
    {
        perror("malloc");
        exit(1);
    }

    node->d_ID = d;
    node->p_ID = p;
    node->count = 1;

    /* Insert at beginning of chain */
    node->next = htable[h];
    htable[h] = node;
}

static PairNode *findPair(int d, int p)
{
    int h = hash(d, p);
    PairNode *curr = htable[h];

    while (curr != NULL)
    {
        if (curr->d_ID == d && curr->p_ID == p)
            return curr;

        curr = curr->next;
    }

    return NULL;
}

// Traverse bookings and fill hash table
static void collect_pairs_hash(BookingNode *b)
{
    if (b->status == 1)
        updatePair(b->d_ID, b->p_ID);
}

// Main function
void displayFrequentPairs(DriverNode *d, PassengerNode *p, BookingNode *b)
{   // Fill hash table
    inorder_booking(b, collect_pairs_hash);

    // Find max
    PairNode *best = NULL;

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        PairNode *curr = htable[i];
        while (curr)
        {
            if (!best || curr->count > best->count)
                best = curr;
            curr = curr->next;
        }
    }

    if (!best)
    {
        printf("[INFO] No bookings yet.\n");
        return;
    }

    // Lookup names
    DriverNode *d_node = search_driver(d, best->d_ID);
    PassengerNode *p_node = search_passenger(p, best->p_ID);

    printf("\n=== MOST FREQUENT PAIR ===\n");

    printf("Driver    : %s (ID=%d)\n", d_node ? d_node->name : "Unknown", best->d_ID);

    printf("Passenger : %s (ID=%d)\n", p_node ? p_node->name : "Unknown", best->p_ID);

    printf("Rides Together: %d\n", best->count);
}
/* 9. displayAvailableVehicles */
static int avail_count;
static void print_free(DriverNode *d)
{
    if (d->status == 0)
    {
        printf("  ID:%-4d | %-20s | %-4s | Location:(%3d,%3d)\n", d->d_ID, d->name, d->vehicle_type ? "Bike" : "Cab", d->current_location.x, d->current_location.y);
        avail_count++;
    }
}
void displayAvailableVehicles(DriverNode *root)
{
    printf("\n[AVAILABLE VEHICLES]\n");
    printf("  %-4s | %-20s | %-4s | %-16s\n", "ID", "Name", "Type", "Location");
    printf("  %s\n", "-------------------------------------------------------");
    avail_count = 0;
    inorder_driver(root, print_free);
    if (avail_count == 0)
        printf("  (none)\n");
    printf("  Total: %d\n", avail_count);
}

/* 10. updateDriverLocation */
void updateDriverLocation(DriverNode *root, int d_id, int new_x, int new_y)
{
    DriverNode *d = search_driver(root, d_id);
    if (!d)
    {
        printf("[ERROR] Driver ID %d not found.\n", d_id);
        return;
    }
    if (d->status == 1)
    {
        printf("[WARN] Driver %d is currently booked. Location update allowed but noted.\n", d_id);
    }
    d->current_location.x = new_x;
    d->current_location.y = new_y;
    printf("[OK] Driver %s (ID=%d) location updated to (%d,%d).\n",
           d->name, d_id, new_x, new_y);
}

/* 11. deleteDriver */
void deleteDriver(DriverNode **root, int d_id)
{
    DriverNode *d = search_driver(*root, d_id);
    if (!d)
    {
        printf("[ERROR] Driver ID %d not found.\n", d_id);
        return;
    }
    if (d->status == 1)
    {
        printf("[ERROR] Driver %d is currently booked. Cannot delete.\n", d_id);
        return;
    }
    printf("[OK] Driver '%s' (ID=%d) removed from system.\n", d->name, d_id);
    *root = delete_driver(*root, d_id);

    saveDrivers(*root);
}

/* 12. displayBookingHistory */
static int bh_cnt;
static void _print_booking(BookingNode *b)
{
   const char *status_text;

if (b->status == 0)
    status_text = "Active";
else if (b->status == 1)
    status_text = "Completed";
else
    status_text = "Cancelled";

printf("  BID:%-4d | DID:%-4d | PID:%-4d | %-4s | Dist:%-8.2f | Fare:Rs%-9.2f | TS:%-5d | Status:%s\n",
       b->booking_id, b->d_ID, b->p_ID,
       b->vehicle_type ? "Bike" : "Cab",
       b->distance_travelled, b->fare,
       b->timestamp, status_text);
    bh_cnt++;
}
void displayBookingHistory(BookingNode *root)
{
    printf("\n[BOOKING HISTORY]\n");
    printf("  %-4s | %-4s | %-4s | %-4s | %-10s | %-12s | %s\n", "BID", "DID", "PID", "Type", "Distance", "Fare", "Timestamp");
    printf("  %s\n", "----------------------------------------------------------------");
    bh_cnt = 0;
    inorder_booking(root, _print_booking);
    if (bh_cnt == 0)
        printf("  (no bookings yet)\n");
    printf("  Total bookings: %d\n", bh_cnt);
}

/* 13. rangeSearchPassengers*/
void rangeSearchPassengers(PassengerNode *root, int pid1, int pid2)
{
    if (pid1 > pid2)
    {
        int t = pid1;
        pid1 = pid2;
        pid2 = t;
    }
    printf("\n[RANGE SEARCH] Passengers with ID in [%d, %d]\n", pid1, pid2);
    printf("  %-4s | %-20s | %-15s | Rides\n", "PID", "Name", "Mobile");
    printf("  %s\n", "------------------------------------------------");
    range_passenger(root, pid1, pid2);
}
