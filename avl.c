#include "ride_hailing.h"

// We duplicate tiny helpers per type to keep typesafety in C.

static int imax(int a, int b) { return a > b ? a : b; }

// DRIVER AVL

static int dheight(DriverNode *n) { return n ? n->height : 0; }

static void dupdate(DriverNode *n)
{
    if (n)
        n->height = 1 + imax(dheight(n->left), dheight(n->right));
}

//diff of balance factor
//to compare height(left subtree and right subtree)
static int dbf(DriverNode *n)
{
    return n ? dheight(n->left) - dheight(n->right) : 0;
}

static DriverNode *drotR(DriverNode *y)
{
    DriverNode *x = y->left, *T = x->right;
    x->right = y;
    y->left = T;
    dupdate(y);
    dupdate(x);
    return x;
}

static DriverNode *drotL(DriverNode *x)
{
    DriverNode *y = x->right, *T = y->left;
    y->left = x;
    x->right = T;
    dupdate(x);
    dupdate(y);
    return y;
}

static DriverNode *dbalance(DriverNode *n)
{
    dupdate(n);
    int bf = dbf(n);
    if (bf > 1)
    {
        if (dbf(n->left) < 0)
            n->left = drotL(n->left);
        return drotR(n);
    }
    if (bf < -1)
    {
        if (dbf(n->right) > 0)
            n->right = drotR(n->right);
        return drotL(n);
    }
    return n;
}

DriverNode *new_driver(int id, char name[], int type, int x, int y)
{
    DriverNode *n = (DriverNode *)calloc(1, sizeof(DriverNode));
    if (!n)
    {
        perror("malloc");
        exit(1);
    }
    n->d_ID = id;
    strncpy(n->name, name, MAX_NAME - 1);
    n->vehicle_type = type;
    n->current_location.x = x;
    n->current_location.y = y;
    n->status = 0;
    n->total_earnings = 0.0f;
    n->height = 1;
    return n;
}

DriverNode *insert_driver(DriverNode *root, DriverNode *node)
{
    if (!root)
        return node;
    if (node->d_ID < root->d_ID)
        root->left = insert_driver(root->left, node);
    else if (node->d_ID > root->d_ID)
        root->right = insert_driver(root->right, node);
    else
    {
        // duplicate ID – free and ignore
        free(node);
        return root;
    }
    return dbalance(root);
}

DriverNode *search_driver(DriverNode *root, int id)
{
    if (!root)
        return NULL;
    if (id == root->d_ID)
        return root;
    return id < root->d_ID ? search_driver(root->left, id) : search_driver(root->right, id);
}

// find in-order successor (leftmost of right subtree)
static DriverNode *dmin(DriverNode *n)
{
    while (n->left)
        n = n->left;
    return n;
}

DriverNode *delete_driver(DriverNode *root, int id)
{
    if (!root)
        return NULL;
    if (id < root->d_ID)
    {
        root->left = delete_driver(root->left, id);
    }
    else if (id > root->d_ID)
    {
        root->right = delete_driver(root->right, id);
    }
    else
    {
        // found
        if (!root->left || !root->right)
        {
            DriverNode *tmp = root->left ? root->left : root->right;
            free(root);
            return tmp;
        }
        DriverNode *succ = dmin(root->right);
        root->d_ID = succ->d_ID;
        strncpy(root->name, succ->name, MAX_NAME - 1);
        root->vehicle_type = succ->vehicle_type;
        root->current_location = succ->current_location;
        root->status = succ->status;
        root->total_earnings = succ->total_earnings;
        root->right = delete_driver(root->right, succ->d_ID);
    }
    return dbalance(root);
}

void inorder_driver(DriverNode *root, void (*check)(DriverNode *))
{
    if (!root)
        return;
    inorder_driver(root->left, check);
    check(root);
    inorder_driver(root->right, check);
}

// PASSENGER AVL

static int pheight(PassengerNode *n) { return n ? n->height : 0; }
static void pupdate(PassengerNode *n)
{
    if (n)
        n->height = 1 + imax(pheight(n->left), pheight(n->right));
}
static int pbf(PassengerNode *n) { return n ? pheight(n->left) - pheight(n->right) : 0; }

static PassengerNode *protR(PassengerNode *y)
{
    PassengerNode *x = y->left, *T = x->right;
    x->right = y;
    y->left = T;
    pupdate(y);
    pupdate(x);
    return x;
}
static PassengerNode *protL(PassengerNode *x)
{
    PassengerNode *y = x->right, *T = y->left;
    y->left = x;
    x->right = T;
    pupdate(x);
    pupdate(y);
    return y;
}
static PassengerNode *pbalance(PassengerNode *n)
{
    pupdate(n);
    int bf = pbf(n);
    if (bf > 1)
    {
        if (pbf(n->left) < 0)
            n->left = protL(n->left);
        return protR(n);
    }
    if (bf < -1)
    {
        if (pbf(n->right) > 0)
            n->right = protR(n->right);
        return protL(n);
    }
    return n;
}

PassengerNode *new_passenger(int id, char name[], char mobile[])
{
    PassengerNode *n = (PassengerNode *)calloc(1, sizeof(PassengerNode));
    if (!n)
    {
        perror("malloc");
        exit(1);
    }
    n->p_ID = id;
    strncpy(n->name, name, MAX_NAME - 1);
    strncpy(n->mobile_no, mobile, MAX_MOBILE - 1);
    n->frequency = 0;
    n->height = 1;
    return n;
}

PassengerNode *insert_passenger(PassengerNode *root, PassengerNode *node)
{
    if (!root)
        return node;
    if (node->p_ID < root->p_ID)
        root->left = insert_passenger(root->left, node);
    else if (node->p_ID > root->p_ID)
        root->right = insert_passenger(root->right, node);
    else
    {
        free(node);
        return root;
    }
    return pbalance(root);
}

PassengerNode *search_passenger(PassengerNode *root, int id)
{
    if (!root)
        return NULL;
    if (id == root->p_ID)
        return root;
    return id < root->p_ID ? search_passenger(root->left, id): search_passenger(root->right, id);
}

// linear search by mobile (inorder)
static PassengerNode *found_p = NULL;
static char target_mobile[MAX_MOBILE];
static void search_mobile(PassengerNode *n)
{
    if (strcmp(n->mobile_no, target_mobile) == 0)
        found_p = n;
}
PassengerNode *search_passenger_mobile(PassengerNode *root, char mobile[])
{
    found_p = NULL;
    strncpy(target_mobile, mobile, MAX_MOBILE - 1);
    inorder_passenger(root, (void (*)(PassengerNode *))search_mobile);
    return found_p;
}

void inorder_passenger(PassengerNode *root, void (*visit)(PassengerNode *))
{
    if (!root)
        return;
    inorder_passenger(root->left, visit);
    visit(root);
    inorder_passenger(root->right, visit);
}

void range_passenger(PassengerNode *root, int lo, int hi)
{
    if (!root)
        return;
    if (root->p_ID > lo)
        range_passenger(root->left, lo, hi);
    if (root->p_ID >= lo && root->p_ID <= hi)
    {
        printf("  PID: %-4d | Name: %-20s | Mobile: %-15s | Rides: %d\n", root->p_ID, root->name, root->mobile_no, root->frequency);
    }
    if (root->p_ID < hi)
        range_passenger(root->right, lo, hi);
}

// BOOKING AVL

static int bheight(BookingNode *n) { return n ? n->height : 0; }
static void bupdate(BookingNode *n)
{
    if (n)
        n->height = 1 + imax(bheight(n->left), bheight(n->right));
}
static int bbf(BookingNode *n) { return n ? bheight(n->left) - bheight(n->right) : 0; }
static BookingNode *brotR(BookingNode *y)
{
    BookingNode *x = y->left, *T = x->right;
    x->right = y;
    y->left = T;
    bupdate(y);
    bupdate(x);
    return x;
}
static BookingNode *brotL(BookingNode *x)
{
    BookingNode *y = x->right, *T = y->left;
    y->left = x;
    x->right = T;
    bupdate(x);
    bupdate(y);
    return y;
}
static BookingNode *bbalance(BookingNode *n)
{
    bupdate(n);
    int bf = bbf(n);
    if (bf > 1)
    {
        if (bbf(n->left) < 0)
            n->left = brotL(n->left);
        return brotR(n);
    }
    if (bf < -1)
    {
        if (bbf(n->right) > 0)
            n->right = brotR(n->right);
        return brotL(n);
    }
    return n;
}

BookingNode *new_booking(int bid, int did, int pid, int vtype, int ts)
{
    BookingNode *n = (BookingNode *)calloc(1, sizeof(BookingNode));
    if (!n)
    {
        perror("malloc");
        exit(1);
    }
    n->booking_id = bid;
    n->d_ID = did;
    n->p_ID = pid;
    n->vehicle_type = vtype;
    n->timestamp = ts;
    n->distance_travelled = 0;
    n->fare = 0;
    n->height = 1;
    return n;
}

BookingNode *insert_booking(BookingNode *root, BookingNode *node)
{
    if (!root)
        return node;
    if (node->booking_id < root->booking_id)
        root->left = insert_booking(root->left, node);
    else if (node->booking_id > root->booking_id)
        root->right = insert_booking(root->right, node);
    else
    {
        free(node);
        return root;
    }
    return bbalance(root);
}

BookingNode *search_booking(BookingNode *root, int bid)
{
    if (!root)
        return NULL;
    if (bid == root->booking_id)
        return root;
    return bid < root->booking_id ? search_booking(root->left, bid) : search_booking(root->right, bid);
}

void inorder_booking(BookingNode *root, void (*visit)(BookingNode *))
{
    if (!root)
        return;
    inorder_booking(root->left, visit);
    visit(root);
    inorder_booking(root->right, visit);
}
