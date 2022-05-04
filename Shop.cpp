
#include "Shop.h"

void Shop::init() 
{
   cond_customer_served_ = new pthread_cond_t[nBarbThreads_];
   cond_barber_paid_ = new pthread_cond_t[nBarbThreads_];
   cond_barber_sleeping_ = new pthread_cond_t[nBarbThreads_];
   pthread_mutex_init(&mutex_, NULL);
   pthread_cond_init(&cond_customers_waiting_, NULL);
   for (int i = 0; i < nBarbThreads_; i++){
      pthread_cond_init(&cond_customer_served_[i], NULL);
      pthread_cond_init(&cond_barber_paid_[i], NULL);
      pthread_cond_init(&cond_barber_sleeping_[i], NULL);
   }
}

string Shop::int2string(int i) 
{
   stringstream out;
   out << i;
   return out.str( );
}

void Shop::print(int person, string message)
{
   cout << ((person != -barber) ? "customer[" : "barber  [" )
            << ((person < 0) ? -person : person ) << "]: " << message << endl;
}

int Shop::get_cust_drops() const
{
    return cust_drops_;
}

bool Shop::customerInAllChairs()
{
   for(int i = 0; i < nBarbThreads_; i++){
      if(customer_in_chair_[i] == 0) return false;
   }
   return true;
}

bool Shop::allBarbersInService()
{
   for(int i = 0; i < nBarbThreads_; i++){
      if(!in_service_[i]) return false;
   }
   return true;
}

int Shop::availableBarber()
{
   for(int i = 0; i < nBarbThreads_; i++){
      if(!in_service_[i]) return i;
   }
   return -1;
}

int Shop::visitShop(int id) 
{
   pthread_mutex_lock(&mutex_);
   
   // If all chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_) 
   {
      print(id, "leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return -1;
   }
   
   // If someone is being served or transitioning waiting to service chair
   // then take a chair and wait for service
   if (customerInAllChairs() || !waiting_chairs_.empty()) 
   {
      waiting_chairs_.push(id);
      print(id, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   }
   
   while (customerInAllChairs()){
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);
   }

   if(waiting_chairs_.size() > 0){
      id = waiting_chairs_.front();
      waiting_chairs_.pop();
   }

   // wake up the barber just in case if he is sleeping
   pthread_cond_signal(&cond_barber_sleeping_);

   barber = availableBarber();
   cout << "Available Barber = " << availableBarber() << endl;

   print(id, "moves to the service chair[" + int2string(barber) + "]. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   customer_in_chair_[barber] = id;
   in_service_[barber] = true;

   pthread_mutex_unlock(&mutex_); 
   return barber;
}

void Shop::leaveShop(int id, int barber_id) 
{
   pthread_mutex_lock( &mutex_ );
   // Wait for service to be completed
   print(id, "wait for the hair-cut to be done");
   while (in_service_[barber_id] == true)
   {
      pthread_cond_wait(&cond_customer_served_, &mutex_);
   }
   // Pay the barber and signal barber appropriately
   money_paid_[barber_id] = true;
   print(id, "says good-bye to barber[" + int2string(barber_id) + "]" );
   pthread_cond_signal(&cond_barber_paid_[barber_id]);
   pthread_mutex_unlock(&mutex_);
}

void Shop::helloCustomer(int barber_id) 
{
   pthread_mutex_lock(&mutex_);
   // If no customers then barber can sleep
   barber = barber_id;
   if (waiting_chairs_.empty() ){
      print(-barber_id, "sleeps because of no customers.");
   }
   while (customer_in_chair_[barber_id] == 0) 
   {
      pthread_cond_wait(&cond_barber_sleeping_, &mutex_);
   }
   barber = barber_id;
   print(-barber_id, "starts a hair-cut service for customer[" + int2string(customer_in_chair_[barber]) + "]");
   pthread_mutex_unlock( &mutex_ );
}

void Shop::byeCustomer(int barber_id) 
{
  pthread_mutex_lock(&mutex_);

  // Hair Cut-Service is done so signal customer and wait for payment
  in_service_[barber_id] = false;
  barber = barber_id;
  print(-barber_id, "says he's done with a hair-cut service for customer[" + int2string(customer_in_chair_[barber_id]) + "]");
  money_paid_[barber_id] = false;
  pthread_cond_signal(&cond_customer_served_);
  while (money_paid_[barber_id] == false)
  {
      pthread_cond_wait(&cond_barber_paid_[barber_id], &mutex_);
  }
  //Signal to customer to get next one
  customer_in_chair_[barber_id] = 0;
  barber = barber_id;
  print(-barber_id, "calls in another customer");
  pthread_cond_signal( &cond_customers_waiting_ );

  pthread_mutex_unlock( &mutex_ );  // unlock
}
