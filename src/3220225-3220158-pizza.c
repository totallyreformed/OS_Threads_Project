#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "3220225-3220158-pizza.h"

// Variables for Income and Statistics
int income = 0;           // Total income
int margarita_count = 0;  // Count of Margarita pizzas sold
int pepperoni_count = 0;  // Count of Pepperoni pizzas sold
int special_count = 0;    // Count of Special pizzas sold
int successful_orders = 0; // Count of successful orders
int failed_orders = 0;     // Count of failed orders
int counter[100] = {0};         // Counter of time taken for each customer
int cooling_counter[100] = {0}; // Counter of cooling time taken for each customer
int counterX[100] = {0};    // Time taken until delivery is packed
int counterY[100] = {0};    // Time taken until delivery is delivered (lmao)
int max_service_time = 0; // Maximum service time
int max_cooling_time = 0; // Maximum cooling time
int total_service_time = 0; // Total service time
int total_cooling_time = 0; // Total cooling time

// Mutexes for Resource Availability
pthread_mutex_t phone_mutex;
pthread_mutex_t cook_mutex;
pthread_mutex_t oven_mutex;
pthread_mutex_t deliverer_mutex;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cooling_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t counterX_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t counterY_mutex = PTHREAD_MUTEX_INITIALIZER;

// Condition variables for Resource Availability
pthread_cond_t phone_cond;
pthread_cond_t cook_cond;
pthread_cond_t oven_cond;
pthread_cond_t deliverer_cond;

// Resource Counters
int available_phones = Ntel;
int available_cooks = Ncook;
int available_ovens = Noven;
int available_deliverers = Ndeliverer;

// Pizza Names
const char *pizza_names[] = {"", "Margarita", "Pepperoni", "Special"};

int main(int argc, char **argv) {
    // Check if the correct number of arguments are passed
    if (argc != 3) {
        printf("Usage: %s <number_of_customers> <seed>\n", argv[0]);
        return 1;
    }

    // Seed the random number generator
    srand(atoi(argv[2]));

    // Convert the number of customers to an integer
    int num_customers = atoi(argv[1]);

    pthread_t *threads;

    // Memory Allocation
    threads = malloc(num_customers * sizeof(pthread_t));

    // Initialize mutexes
    pthread_mutex_init(&phone_mutex, NULL);
    pthread_mutex_init(&cook_mutex, NULL);
    pthread_mutex_init(&oven_mutex, NULL);
    pthread_mutex_init(&deliverer_mutex, NULL);

    // Initialize mutex conditions
    pthread_cond_init(&phone_cond, NULL);
    pthread_cond_init(&cook_cond, NULL);
    pthread_cond_init(&oven_cond, NULL);
    pthread_cond_init(&deliverer_cond, NULL);

    // Create an array of customer threads
    pthread_t customers[num_customers];

    // Create and start customer threads
    for (int i = 0; i < num_customers; i++) {
        pthread_create(&customers[i], NULL, customer_thread, (void *)i);
    }

    // Wait for all customer threads to finish
    for (int i = 0; i < num_customers; i++) {
        pthread_join(customers[i], NULL);
    }


    // Calculate max service time and max cooling time
    for (int i = 0; i < num_customers; i++) {
        if (counter[i] > max_service_time) {
            max_service_time = counter[i];
        }

        if (cooling_counter[i] > max_cooling_time) {
            max_cooling_time = cooling_counter[i];
        }
    }

    // Calculate the average service time
    for (int i = 0; i < num_customers; i++) {
        total_service_time += counter[i];
        total_cooling_time += cooling_counter[i];
    }

    int avg_service_time = total_service_time / num_customers;
    int avg_cooling_time = total_cooling_time / num_customers;

    // Print customer statistics
    printf("\nCustomer Statistics\n");
    printf("-------------------\n");
    for (int i = 0; i < num_customers; i++) {
        printf("Customer with ID %d:\n", i);
        printf("Total Service Time: %d minutes\n", counter[i]);
        printf("Time elapsed for packing: %d minutes\n", counterX[i]);
        printf("Time elapsed for delivery: %d minutes\n\n", counterY[i]);
    }

    // Print shop statistics
    printf("\nShop Statistics\n");
    printf("----------------\n");
    printf("Number of successful orders: %d\n", successful_orders);
    printf("Number of failed orders: %d\n", failed_orders);
    printf("\nTotal Income: %d\n", income);
    printf("\nMargarita pizzas sold: %d\n", margarita_count);
    printf("Pepperoni pizzas sold: %d\n", pepperoni_count);
    printf("Special pizzas sold: %d\n", special_count);
    printf("\nMaximum service time: %d\n", max_service_time);
    printf("Average service time: %d\n", avg_service_time);
    printf("\nMaximum cooling time: %d\n", max_cooling_time);
    printf("Average cooling time: %d\n", avg_cooling_time);
    
    // Destroy mutexes
    pthread_mutex_destroy(&phone_mutex);
    pthread_mutex_destroy(&cook_mutex);
    pthread_mutex_destroy(&oven_mutex);
    pthread_mutex_destroy(&deliverer_mutex);
    pthread_mutex_destroy(&counter_mutex);
    pthread_mutex_destroy(&cooling_counter_mutex);
    pthread_mutex_destroy(&counterX_mutex);
    pthread_mutex_destroy(&counterY_mutex);

    // Destroy condition vairables
    pthread_cond_destroy(&phone_cond);
    pthread_cond_destroy(&cook_cond);
    pthread_cond_destroy(&oven_cond);
    pthread_cond_destroy(&deliverer_cond);
    
    // Free the memory allocated space
    free(threads);

    return 1;
}

// Customer thread function
void *customer_thread(void *arg) {
    int customer_id = (int)arg;

    printf("Customer with ID %d is calling...\n", customer_id);

    take_order(customer_id);

    return NULL;
}

/** Function to generate a random number between low and high
 * @param low The lower bound of the random number
 * @param high The upper bound of the random number
 * @return The random number generated
*/
int generate_random(int low, int high) {
    int range = high - low + 1;
    int random_num = rand() % range + low;
    return random_num;
}

/** Function to process the payment for the order
 * @param num_pizzas The number of pizzas ordered
 * @param choice_id The choice of pizza
 * @param customer_id The ID of the customer
 * @return True if the payment is successful, false otherwise
*/
bool process_payment(int num_pizzas, int choice_id, int customer_id) {
    // Calculate random number for the possibility the payment fails (out of 100)
    int failure_possibility = generate_random(1, 100);

    // Simulate time taken for payment
    int random_num = generate_random(Tpaymentlow, Tpaymenthigh);
    sleep(random_num);

    if (failure_possibility <= Pfail) {
        return false;
    }

    // Add to counter array for given customer
    pthread_mutex_lock(&counter_mutex);
    counter[customer_id] += random_num;
    pthread_mutex_unlock(&counter_mutex);

    // Add time taken for payment to counterX and counterY arrays
    pthread_mutex_lock(&counterX_mutex);
    counterX[customer_id] += random_num;
    pthread_mutex_unlock(&counterX_mutex);

    pthread_mutex_lock(&counterY_mutex);
    counterY[customer_id] += random_num;
    pthread_mutex_unlock(&counterY_mutex);

    HELPER_process_payment(random_num);

    return true;
}

/** Helper function to return the time taken for the payment
 * @param random_num The random number generated for the payment time
 * @return The time taken for the payment
*/
int HELPER_process_payment(random_num) {
    return random_num;
}

/** Function to take the order from the customer
 * @param customer_id The ID of the customer
 * @return void
*/
void take_order(int customer_id) {
    // Wait until phone available
    pthread_mutex_lock(&phone_mutex);
    while (available_phones == 0) {
        pthread_cond_wait(&phone_cond, &phone_mutex);
    }
    available_phones--;
    pthread_mutex_unlock(&phone_mutex);    

    printf("Customer with ID %d is placing an order.\n", customer_id);

    // Simulate time taken for the next phone call
    int random_num = generate_random(Torderlow, Torderhigh);
    sleep(random_num);

    // Add time taken to total service time
    pthread_mutex_lock(&counter_mutex);
    counter[customer_id] += random_num;
    pthread_mutex_unlock(&counter_mutex);

    // Add time taken for the order to counterX and counterY arrays
    pthread_mutex_lock(&counterX_mutex);
    counterX[customer_id] += random_num;
    pthread_mutex_unlock(&counterX_mutex);

    pthread_mutex_lock(&counterY_mutex);
    counterY[customer_id] += random_num;
    pthread_mutex_unlock(&counterY_mutex);

    printf("Customer with ID %d has placed an order.\n", customer_id);

    // Random number of pizzas ordered
    int num_pizzas = rand() % (Norderhigh - Norderlow + 1) + Norderlow;

    /* Random choice of pizza
        1) Random Number Generator
        2) Generate a random number between 1 and 100
        3) Check the probability ranges and choose the type of pizza accordingly
    */

    int choice_id = NULL;
    int m_id = 1;
    int p_id = 2;
    int s_id = 3;

    srand(time(NULL));

    // Signal phone availability
    pthread_mutex_lock(&phone_mutex);
    available_phones++;
    pthread_cond_signal(&phone_cond);
    pthread_mutex_unlock(&phone_mutex);

    // If the payment is successful, prepare the pizzas, otherwise increment the failed orders counter
    if (process_payment(num_pizzas, choice_id, customer_id)) {
        printf("Payment for Customer with ID %d is successful (Time Elapsed: %d minutes)\n", customer_id, HELPER_process_payment());

        // Increment counter for successful orders
        successful_orders++;

        // Prepare pizzas
        for (int i = 0; i < num_pizzas; i++) {
            int random_num = rand() % 100 + 1;

            if (random_num <= Pm) {
                choice_id = m_id;
            } else if (random_num <= Pm + Pp) {
                choice_id = p_id;
            } else {
                choice_id = s_id;
            }

            // Update income
            income += num_pizzas * (choice_id == 1 ? Cm : choice_id == 2 ? Cp : Cs);

            // Update pizza count
            if (choice_id == 1) {
                margarita_count += num_pizzas;
            } else if (choice_id == 2) {
                pepperoni_count += num_pizzas;
            } else {
                special_count += num_pizzas;
            }

            prepare_pizza(customer_id, i + 1, choice_id);
        }
    } else {
        // Increment counter for failed orders
        failed_orders++;
        printf("Payment for Customer with ID %d has failed.\n", customer_id);
    }
}

/** Function to prepare the pizza
 * @param customer_id The ID of the customer
 * @param pizza_id The ID of the pizza
 * @param choice_id The choice of pizza
 * @return void
*/
void prepare_pizza(int customer_id, int pizza_id, int choice_id) {
    // Wait until cook available
    pthread_mutex_lock(&cook_mutex);
    while (available_cooks == 0) {
        pthread_cond_wait(&cook_cond, &cook_mutex);
    }
    available_cooks--;
    pthread_mutex_unlock(&cook_mutex);

    printf("Cook is preparing a %s pizza with ID %d for customer %d.\n", pizza_names[choice_id], pizza_id, customer_id);

    // Simulate pizza preparation time
    sleep(Tprep);

    // Add preparation time to counterX and counterY arrays for given customer
    pthread_mutex_lock(&counterX_mutex);
    counterX[customer_id] += Tprep;
    pthread_mutex_unlock(&counterX_mutex);

    pthread_mutex_lock(&counterY_mutex);
    counterY[customer_id] += Tprep;
    pthread_mutex_unlock(&counterY_mutex);

    // Add to counter array for given customer
    pthread_mutex_lock(&counter_mutex);
    counter[customer_id] += Tprep;
    pthread_mutex_unlock(&counter_mutex);

    printf("%s pizza with ID %d has been prepared for Customer with ID %d (Time Elapsed: %d minute)\n", pizza_names[choice_id], pizza_id, customer_id, Tprep);

    // Signal cook availability
    pthread_mutex_lock(&cook_mutex);
    available_cooks++;
    pthread_cond_signal(&cook_cond);
    pthread_mutex_unlock(&cook_mutex);

    bake_pizza(customer_id, pizza_id, choice_id);
}

/** Function to bake the pizza
 * @param customer_id The ID of the customer
 * @param pizza_id The ID of the pizza
 * @param choice_id The choice of pizza
 * @return void
*/
void bake_pizza(int customer_id, int pizza_id, int choice_id) {
    // Wait until oven available
    pthread_mutex_lock(&oven_mutex);
    while (available_ovens == 0) {
        pthread_cond_wait(&oven_cond, &oven_mutex);
    }
    available_ovens--;
    pthread_mutex_unlock(&oven_mutex);

    printf("%s pizza with ID %d is being baked for Customer with ID %d.\n", pizza_names[choice_id], pizza_id, customer_id);

    // Simulate baking time
    sleep(Tbake);

    // Add baking time to counterX and counterY arrays for given customer
    pthread_mutex_lock(&counterX_mutex);
    counterX[customer_id] += Tbake;
    pthread_mutex_unlock(&counterX_mutex);

    pthread_mutex_lock(&counterY_mutex);
    counterY[customer_id] += Tbake;
    pthread_mutex_unlock(&counterY_mutex);

    // Add to counter array for given customer
    pthread_mutex_lock(&counter_mutex);
    counter[customer_id] += Tbake;
    pthread_mutex_unlock(&counter_mutex);

    printf("%s pizza with ID %d has been baked for Customer with ID %d (Time Elapsed: %d minutes)\n", pizza_names[choice_id], pizza_id, customer_id, Tbake);
    
    // Signal oven availability
    pthread_mutex_lock(&oven_mutex);
    available_ovens++;
    pthread_cond_signal(&oven_cond);
    pthread_mutex_unlock(&oven_mutex);

    pack_and_deliver(customer_id, pizza_id, choice_id);
}

/** Function to pack and deliver the pizza
 * @param customer_id The ID of the customer
 * @param pizza_id The ID of the pizza
 * @param choice_id The choice of pizza
 * @return void
*/
void pack_and_deliver(int customer_id, int pizza_id, int choice_id) {
    // Wait until deliverer available
    pthread_mutex_lock(&deliverer_mutex);
    while (available_deliverers == 0) {
        pthread_cond_wait(&deliverer_cond, &deliverer_mutex);
    }
    available_deliverers--;
    pthread_mutex_unlock(&deliverer_mutex);

    printf("%s pizza with ID %d is being packed for Customer with ID %d.\n", pizza_names[choice_id], pizza_id, customer_id);

    // Simulate packing time
    sleep(Tpack);

    // Add to counter array for given customer
    pthread_mutex_lock(&cooling_counter_mutex);
    cooling_counter[customer_id] += Tpack;
    pthread_mutex_unlock(&cooling_counter_mutex);

    pthread_mutex_lock(&counter_mutex);
    counter[customer_id] += Tpack;
    pthread_mutex_unlock(&counter_mutex);

    // Add packing time to counterX and counterY arrays for given customer
    pthread_mutex_lock(&counterX_mutex);
    counterX[customer_id] += Tpack;
    pthread_mutex_unlock(&counterX_mutex);

    pthread_mutex_lock(&counterY_mutex);
    counterY[customer_id] += Tpack;
    pthread_mutex_unlock(&counterY_mutex);

    printf("%s pizza with ID %d has been packed for Customer with ID %d (Time Elapsed: %d minute)\n", pizza_names[choice_id], pizza_id, customer_id, Tpack);

    printf("%s pizza with ID %d is being delivered to Customer with ID %d.\n", pizza_names[choice_id], pizza_id, customer_id);

    // Simulate delivery time
    int Tdeliver = generate_random(Tdellow, Tdelhigh);
    sleep(Tdeliver);

    // Add time taken for the delivery to the counterY array for given customer
    pthread_mutex_lock(&counterY_mutex);
    counterY[customer_id] += Tdeliver;
    pthread_mutex_unlock(&counterY_mutex);

    // Add to counter array for given customer
    pthread_mutex_lock(&counter_mutex);
    counter[customer_id] += Tdeliver;
    pthread_mutex_unlock(&counter_mutex);

    pthread_mutex_lock(&cooling_counter_mutex);
    cooling_counter[customer_id] += Tdeliver;
    pthread_mutex_unlock(&cooling_counter_mutex);

    printf("%s pizza with ID %d has been delivered to Customer with ID %d (Time Elapsed: %d minutes)\n", pizza_names[choice_id], pizza_id, customer_id, Tdeliver);

    // Simulate the time it takes for deliverer to come back
    sleep(Tdeliver);

    // Signal deliverer availability
    pthread_mutex_lock(&deliverer_mutex);
    available_deliverers++;
    pthread_cond_signal(&deliverer_cond);
    pthread_mutex_unlock(&deliverer_mutex);
}