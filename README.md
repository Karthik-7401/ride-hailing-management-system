# Ride-Hailing Management System

A command-line ride-hailing management system developed in C using AVL trees, hash tables, dynamic memory allocation, and file-based persistence.

## Features

- Add and manage drivers and passengers
- Request, complete, and cancel rides
- Find the nearest available vehicle within a fixed radius
- Track driver earnings
- Display top 3 drivers by earnings
- Track the most frequent driver-passenger pair using a hash table
- Display available vehicles
- Update driver locations
- Delete drivers safely
- Display booking history with ride status
- Search passengers within an ID range
- Save and load data using text files

## Data Structures

### AVL Trees
Three separate AVL trees are used for efficient management of:

- Drivers
- Passengers
- Bookings

Each tree supports insertion, searching, balancing, and in-order traversal.

### Hash Table

A hash table with separate chaining stores driver-passenger ride frequencies. The pair count is updated when a ride is completed.

## Project Structure

```text
ride-hailing-management-system/
├── main.c
├── avl.c
├── functions.c
├── loadandsave.c
├── ride_hailing.h
├── Makefile
├── README.md
├── .gitignore
├── drivers.txt
├── passengers.txt
├── bookings.txt
└── meta.txt
```

## Booking Lifecycle

```text
Request Ride
     |
     v
   Active
   /   \
  v     v
Complete Cancel
  |       |
  v       v
Completed Cancelled
```

When a ride is completed, the driver's earnings and passenger ride frequency are updated, and the driver becomes available again.

## File Persistence

The system stores application data in:

- `drivers.txt` - driver information and earnings
- `passengers.txt` - passenger information and ride frequency
- `bookings.txt` - booking records, fares, distances, timestamps, and status
- `meta.txt` - booking counter and simulation timestamp

Data is loaded when the program starts and saved after relevant operations.

## Compilation

Using GCC:

```bash
make
```

Or directly:

```bash
gcc main.c avl.c functions.c loadandsave.c -o app -lm
```

## Run

Linux/macOS:

```bash
./app
```

Windows PowerShell:

```powershell
.pp.exe
```

## Technologies

- C
- AVL Trees
- Hash Tables
- File Handling
- Dynamic Memory Allocation
- Recursion
- Command-Line Interface

## Author

Karthik Y  
B.Tech Computer Science and Engineering, VNIT Nagpur
