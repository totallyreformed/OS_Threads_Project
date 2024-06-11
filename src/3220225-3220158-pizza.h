#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define Ntel 2
#define Ncook 2
#define Noven 10
#define Ndeliverer 10
#define Torderlow 1
#define Torderhigh 5
#define Norderlow 1
#define Norderhigh 5
#define Pm 35
#define Pp 25
#define Ps 40
#define Tpaymentlow 1
#define Tpaymenthigh 3
#define Pfail 5
#define Cm 10
#define Cp 11
#define Cs 12
#define Tprep 1
#define Tbake 10
#define Tpack 1
#define Tdellow 5
#define Tdelhigh 15

void* customer_thread(void* arg);
int generate_random(int low, int high);
bool process_payment(int num_pizzas, int choice_id, int customer_id);
void take_order(int customer_id);
void prepare_pizza(int customer_id, int pizza_id, int choice_id);
void bake_pizza(int customer_id, int pizza_id, int choice_id);
void pack_and_deliver(int customer_id, int pizza_id, int choice_id);