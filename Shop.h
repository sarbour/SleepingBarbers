#ifndef Shop_H_
#define Shop_H_
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
#include <vector>
using namespace std;

#define kDefaultNumChairs 3
#define kDefaultBarbers 1

class Shop 
{
public:
   Shop(int num_chairs, int num_barbers) : max_waiting_cust_((num_chairs > 0 ) ? num_chairs : kDefaultNumChairs),
      customer_in_chair_((num_barbers > 0 ) ? num_barbers : kDefaultBarbers, 0), 
      in_service_((num_barbers > 0 ) ? num_barbers : kDefaultBarbers, false),
      money_paid_((num_barbers > 0 ) ? num_barbers : kDefaultBarbers, false),
      nBarbThreads_((num_barbers > 0 ) ? num_barbers : kDefaultBarbers), cust_drops_(0)
   { 
      init(); 
   };
   
   Shop() : max_waiting_cust_(kDefaultNumChairs), nBarbThreads_(kDefaultBarbers),
      customer_in_chair_(kDefaultBarbers, 0), in_service_(kDefaultBarbers, 0),
      money_paid_(kDefaultBarbers, 0), cust_drops_(0)
   { 
      init();
   };

   ~Shop(){
      destroy();
   }

   int visitShop(int id);   // return true only when a customer got a service
   void leaveShop(int id, int barber_id);
   void helloCustomer(int barber_id);
   void byeCustomer(int barber_id);
   int get_cust_drops() const;
   void checkCustomerInChair();
   void signalAllBarberThreads();

 private:
   const int max_waiting_cust_;  // the max number of threads that can wait
   const int nBarbThreads_;      // # of barber threads
   vector<int> customer_in_chair_;
   vector<bool> in_service_;
   vector<bool> money_paid_;
   queue<int> waiting_chairs_;   // includes the ids of all waiting threads
   queue<int> available_barbers_;
   int cust_drops_;

   // Mutexes and condition variables to coordinate threads
   // mutex_ is used in conjuction with all conditional variables
   pthread_mutex_t mutex_;
   pthread_cond_t  cond_customers_waiting_;
   pthread_cond_t  *cond_customer_served_;
   pthread_cond_t  *cond_barber_paid_;
   pthread_cond_t  *cond_barber_sleeping_;

   void init();
   string int2string(int i);
   void print(int person, string message);

   void destroy();
};

#endif