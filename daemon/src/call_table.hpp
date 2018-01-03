#include <string>
#include <vector>
#include "Sock.hpp"
struct CallCell
{
    void (*func)(unsigned int, std::string, Client*);
};
class CallTable
{
public:
    CallCell* table;
    int size;
    CallTable(int size);
};
