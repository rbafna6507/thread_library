#include "pizza.h"
#include "cv.h"
#include <vector>
#include <string>

extern std::vector<cv> driverCVs;
extern std::vector<cv> customerCVs;

struct CommandLineArgs {
    int argc;
    char** argv;
};

struct CustomerArgs {
    unsigned int id;
    std::string filename;
};

enum Status{
    NONE,
    READY,
    WAITING,
    DRIVING,
    TO_ACT,
};

class Matcher {
public:
    unsigned int finishedCustomers = 0;
    std::vector<Status> driverStates;
    std::vector<Status> customerStates;
    std::vector<location_t> driverLocs;
    std::vector<location_t> customerLocs;
    std::vector<unsigned int> driverToCustomer; // matched cust -> driver
    std::vector<unsigned int> customerToDriver; // matched driver -> cust
    void setup(unsigned int numCustomers, unsigned int numDrivers);
    bool can_match();
    void make_match();
private:
    unsigned int dist(unsigned int customerID, unsigned int driverID);
};


