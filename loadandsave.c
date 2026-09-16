/*
 *  meta.txt          →  booking_counter|sim_timestamp
 *
 *  drivers.txt       →  d_ID|name|vehicle_type|x|y|status|total_earnings
 *  passengers.txt    →  p_ID|name|mobile_no|frequency
 *  bookings.txt      →  booking_id|d_ID|p_ID|vehicle_type|distance|fare|timestamp|status
 */

#include "ride_hailing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ───────── GLOBAL FILE POINTER (SAFE VERSION) ───────── */
static FILE *fp;

/* ───────── HELPER: READ VALID LINE ───────── */
static int next_line(FILE *f, char *buf, int size)
{
    while (fgets(buf, size, f))
    {
        if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r')
            continue;
        buf[strcspn(buf, "\r\n")] = '\0';
        return 1;
    }
    return -1;
}

/* ───────── WRITE FUNCTIONS (USED IN INORDER) ───────── */

void write_driver_node(DriverNode *n)
{
    fprintf(fp, "%d|%s|%d|%d|%d|%d|%.2f\n",
            n->d_ID, n->name, n->vehicle_type,
            n->current_location.x, n->current_location.y,
            n->status, n->total_earnings);
}

void write_passenger_node(PassengerNode *n)
{
    fprintf(fp, "%d|%s|%s|%d\n",
            n->p_ID, n->name, n->mobile_no, n->frequency);
}

void write_booking_node(BookingNode *n)
{
    fprintf(fp, "%d|%d|%d|%d|%.2f|%.2f|%d|%d\n",
            n->booking_id, n->d_ID, n->p_ID,
            n->vehicle_type, n->distance_travelled,
            n->fare, n->timestamp, n->status);
}

/* ───────── SAVE FUNCTIONS ───────── */

void saveDrivers(DriverNode *root)
{

    /* Save meta */
    FILE *mf = fopen(META_FILE, "w");
    if (!mf)
    {
        perror("meta");
        return;
    }
    fprintf(mf, "%d|%d\n", booking_counter, sim_timestamp);
    fclose(mf);

    fp = fopen(DRIVER_FILE, "w");
    if (!fp)
    {
        perror("drivers");
        return;
    }

    fprintf(fp, "# d_ID|name|vehicle_type|x|y|status|total_earnings\n");
    inorder_driver(root, write_driver_node);

    fclose(fp);
}

void savePassengers(PassengerNode *root)
{
    fp = fopen(PASS_FILE, "w");
    if (!fp)
    {
        perror("passengers");
        return;
    }

    fprintf(fp, "# p_ID|name|mobile_no|frequency\n");
    inorder_passenger(root, write_passenger_node);

    fclose(fp);
}

void saveBookings(BookingNode *bRoot)
{
    fp = fopen(BOOKING_FILE, "w");
    if (!fp)
    {
        perror("bookings");
        return;
    }

   fprintf(fp, "# booking_id|d_ID|p_ID|vehicle_type|distance|fare|timestamp|status\n");
    inorder_booking(bRoot, write_booking_node);

    fclose(fp);
}

/* ───────── LOAD FUNCTIONS ───────── */

DriverNode *loadDrivers(void)
{

    /* Load meta */
    FILE *mf = fopen(META_FILE, "r");
    if (mf)
    {
        char buf[128];
        if (next_line(mf, buf, sizeof(buf)) == 1)
        {
            sscanf(buf, "%d|%d", &booking_counter, &sim_timestamp);
        }
        fclose(mf);
    }

    FILE *f = fopen(DRIVER_FILE, "r");
    if (!f)
        return NULL;

    DriverNode *root = NULL;
    char buf[256];
    int count = 0;

    while (next_line(f, buf, sizeof(buf)) == 1)
    {

        int id, vtype, x, y, status;
        float earn;
        char name[MAX_NAME];

        if (sscanf(buf, "%d|%[^|]|%d|%d|%d|%d|%f",
                   &id, name, &vtype, &x, &y, &status, &earn) != 7)
            continue;

        DriverNode *n = new_driver(id, name, vtype, x, y);
        n->status = status;
        n->total_earnings = earn;

        root = insert_driver(root, n);
        count++;
    }

    fclose(f);
    printf("[LOAD] %d drivers loaded\n", count);
    return root;
}

PassengerNode *loadPassengers(void)
{
    FILE *f = fopen(PASS_FILE, "r");
    if (!f)
        return NULL;

    PassengerNode *root = NULL;
    char buf[256];
    int count = 0;

    while (next_line(f, buf, sizeof(buf)) == 1)
    {
        int id, freq;
        char name[MAX_NAME], mobile[MAX_MOBILE];

        if (sscanf(buf, "%d|%[^|]|%[^|]|%d",
                   &id, name, mobile, &freq) != 4)
            continue;

        PassengerNode *n = new_passenger(id, name, mobile);
        n->frequency = freq;

        root = insert_passenger(root, n);
        count++;
    }

    fclose(f);

    printf("[LOAD] %d passengers loaded\n", count);
    return root;
}

BookingNode *loadBookings(void)
{
    FILE *f = fopen(BOOKING_FILE, "r");
    if (!f)
        return NULL;

    BookingNode *root = NULL;
    char buf[256];
    int count = 0;

    while (next_line(f, buf, sizeof(buf)) == 1)
    {
        int bid, did, pid, vtype, ts;
        int status;
        float dist, fare;

        /* Try new 8-field format first */
        if (sscanf(buf, "%d|%d|%d|%d|%f|%f|%d|%d",
                   &bid, &did, &pid, &vtype,
                   &dist, &fare, &ts, &status) == 8)
        {
            BookingNode *n = new_booking(bid, did, pid, vtype, ts);

            n->distance_travelled = dist;
            n->fare = fare;
            n->status = status;

            root = insert_booking(root, n);
            count++;
        }
        /* Support old 7-field format */
        else if (sscanf(buf, "%d|%d|%d|%d|%f|%f|%d",
                        &bid, &did, &pid, &vtype,
                        &dist, &fare, &ts) == 7)
        {
            BookingNode *n = new_booking(bid, did, pid, vtype, ts);

            n->distance_travelled = dist;
            n->fare = fare;

            /*
             * Old records with a fare/distance are considered completed.
             * Old records with zero distance are considered active.
             */
            if (dist > 0)
                n->status = 1;
            else
                n->status = 0;

            root = insert_booking(root, n);
            count++;
        }
    }

    fclose(f);

    printf("[LOAD] %d bookings loaded\n", count);
    return root;
}