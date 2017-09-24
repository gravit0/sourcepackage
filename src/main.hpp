#include <vector>
#include <string>
#include <list>
struct FileAction
{
    std::string filename;
    int action;
};
class Package
{
public:
    std::string name;
    std::string version;
    std::string author;
    std::string dir;
    std::vector<std::string> dependencies;
    std::list<FileAction> files;
    std::vector<Package*> dependencie;
    bool isInstalled;
    bool isDependence;
    bool isStartInstall;
    void install();
    void remove_();
};
extern Package* get_pack(std::string dir);
extern Package* find_pack(std::string name);
