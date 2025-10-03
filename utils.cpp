#include "utils.h"
#include "pizza.h"
#include <unordered_map>
#include <vector>
#include <iostream>

 /** 
    Calculates manhattan/rectilinear distance between a customer and a driver.
    
    @param customerID ID/index of the customer
    @param driverID ID/index of the driver
    @return The distance between the provided customer and driver.
*/
unsigned int Matcher::dist(unsigned int customerID, unsigned int driverID){  
    location_t a = this->customerLocs[customerID], b = this->driverLocs[driverID];
    unsigned int dx = (a.x > b.x) ? (a.x - b.x) : (b.x - a.x);
    unsigned int dy = (a.y > b.y) ? (a.y - b.y) : (b.y - a.y);
    return dx + dy;
}

 /**
  Pre-allocates Matcher’s structures to handle the specified
  number of customers and drivers.

  @param numCustomers The number of customers to prepare.
  @param numDrivers The number of drivers to prepare.
  @return void
 */
void Matcher::setup(unsigned int numCustomers, unsigned int numDrivers){
    this->customerLocs.resize(numCustomers);

    this->driverLocs.reserve(numDrivers);
    for(unsigned int i = 0; i < numDrivers; ++i){
        this->driverLocs.push_back({0, 0});
    }

    this->customerStates.resize(numCustomers);
    this->driverStates.resize(numDrivers);
    this->driverToCustomer.resize(numDrivers);
    this->customerToDriver.resize(numCustomers);
}

 /**
    Checks if there is at least one customer and one driver who are both ready.

    @return True if both a ready customer and driver exist, false otherwise.
*/
bool Matcher::can_match(){
    bool readyCust = false, readyDriver = false;
    for(auto status: this->customerStates){
        if(status == Status::READY){
            readyCust = true;
            break;
        }
    }
    for(auto status: this->driverStates){
        if(status == Status::READY){
            readyDriver= true;
            break;
        }
    }

    return readyCust && readyDriver;
}

/**
  Matches a ready customer to a ready driver based on proximity.

  This function identifies all ready customers and drivers, calculates the closest 
  matches based on distance, and establishes a match if the closest driver 
  for a customer is also the closest customer for that driver. Upon successful matching, 
  it updates the states of both the customer and driver, broadcasts the match, 
  and invokes pizza's match() function

  @throws std::runtime_error If no match is successfuly made.
 */
void Matcher::make_match(){
    // find all ready drivers
    std::vector<unsigned int> readyCustomerIDs;
    std::vector<unsigned int> readyDriverIDs;

    for (unsigned int i = 0; i < static_cast<unsigned int>(this->customerStates.size()); ++i) {
        if (this->customerStates[i] == Status::READY) {
            readyCustomerIDs.push_back(i);
        }
    }

    for (unsigned int i = 0; i < static_cast<unsigned int>(this->driverStates.size()); ++i) {
        if (this->driverStates[i] == Status::READY) {
            readyDriverIDs.push_back(i);
        }
    }

    // create a map of closest drivers for each customer
    std::unordered_map<unsigned int, unsigned int> closestDriver;
    for (unsigned int customerIndex: readyCustomerIDs){
        unsigned int minDist = this->dist(customerIndex, readyDriverIDs[0]);
        unsigned int minDistDriver = readyDriverIDs[0];
        for (unsigned int driverIndex: readyDriverIDs){
            unsigned int currDist = this->dist(customerIndex, driverIndex);
            if (currDist < minDist){
                minDist =  currDist;
                minDistDriver = driverIndex;
            }
        }
        closestDriver[customerIndex] = minDistDriver;
    }

    // find closest customer for every driver
    for(auto driverIndex: readyDriverIDs){
        unsigned int minDist = this->dist(readyCustomerIDs[0], driverIndex);
        unsigned int minDistCustomer = readyCustomerIDs[0];
        for (auto customerIndex: readyCustomerIDs){
            unsigned int currDist = this->dist(customerIndex, driverIndex);
            if (currDist < minDist){
                minDist =  currDist;
                minDistCustomer = customerIndex;
            }
        }

        // if we have a match, make the match and return - guaranteed to happen if canMatch() is true.
        if (closestDriver[minDistCustomer] == driverIndex){
            this->driverToCustomer[driverIndex] = minDistCustomer;
            this->customerToDriver[minDistCustomer] = driverIndex;
            this->driverStates[driverIndex] = Status::TO_ACT;
            this->customerStates[minDistCustomer] = Status::TO_ACT;
            match(minDistCustomer, driverIndex);
            driverCVs[driverIndex].broadcast();
            customerCVs[minDistCustomer].broadcast();
            return;
        }
    }
    std::cerr << "Failed to make a match." << std::endl;
    exit(1);
}
