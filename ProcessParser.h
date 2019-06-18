#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"
/* include filesystem header for new directory iterator
https://stackoverflow.com/questions/45867379/why-does-gcc-not-seem-to-have-the-filesystem-standard-library 
To link with the library you need to add -lstdc++fs to the command line.*/
#include <experimental/filesystem>

using namespace std;

class ProcessParser{
private:
    std::ifstream stream;
    public:
    static string getCmd(string pid);
    static vector<string> getPidList();
    static std::string getVmSize(string pid);
    static std::string getCpuPercent(string pid);
    static long int getSysUpTime();
    static std::string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
    static bool isPidExisting(string pid);
    static int getNumberOfCores();
};

// TODO: Define all of the above functions below:
bool ProcessParser::isPidExisting(string pid){
    std::ifstream fstream(Path::basePath() + pid);
    try{
        Util::getStream(Path::basePath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    return true;
}
string ProcessParser::getCmd(string pid){
    std::ifstream fstream(Path::basePath() + pid + Path::cmdPath());
    try{
        Util::getStream(Path::basePath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    string cmd;
    std::getline(fstream,cmd);

    return cmd;
}

// RE-implement this function using the filesystem lib
vector<string> ProcessParser::getPidList()
{
    DIR* dir;
    // Basically, we are scanning /proc dir for all directories with numbers as their names
    // If we get valid check we store dir names in vector as list of machine pids
    vector<string> container;
    if(!(dir = opendir("/proc")))
        throw std::runtime_error(std::strerror(errno));

    while (dirent* dirp = readdir(dir)) {
        // is this a directory?
        if(dirp->d_type != DT_DIR)
            continue;
        // Is every character of the name a digit?
        if (all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), [](char c){ return std::isdigit(c); })) {
            container.push_back(dirp->d_name);
        }
    }
    //Validating process of directory closing
    if(closedir(dir))
        throw std::runtime_error(std::strerror(errno));
    return container;
}

std::string ProcessParser::getVmSize(string pid){
    std::ifstream fstream(Path::basePath() + pid + Path::statusPath());
    float result;
    string line;
    string inputString = "VmData";

    try{
        Util::getStream(Path::basePath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    while (getline(fstream,line))
    {
        if(line.compare(0,inputString.size(),inputString) == 0){
            istringstream str(line);
            istream_iterator<string> beg(str),end;
            vector<string> str_val (beg,end);

            // convert Kb to Gb
            result = (stof(str_val[1])/float(1024));
            break;
        }
    }

    return to_string(result);
    
}

std::string ProcessParser::getCpuPercent(string pid){
    std::ifstream fstream(Path::basePath() + pid + "/" + Path::statPath());
    try{
        Util::getStream(Path::basePath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    string line;
    float result;

    getline(fstream,line);
    istringstream str(line);
    istream_iterator<string> beg(str),end;
    vector<string> vlaues(beg,end);

    float utime = stof(ProcessParser::getProcUpTime(pid));
    float uptime = ProcessParser::getSysUpTime();
    float stime = stof(vlaues[14]);
    float cutime = stof(vlaues[15]);
    float cstime = stof(vlaues[16]);
    float starttime = stof(vlaues[21]);

    float frequency = sysconf(_SC_CLK_TCK);

    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime/frequency);
    result = 100.0 * ((total_time/frequency)/seconds);

    return to_string(result);

}

long int ProcessParser::getSysUpTime(){
    std::ifstream fstream(Path::basePath() + Path::upTimePath());
    try{
        Util::getStream(Path::basePath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    string upTime;
    std::getline(fstream,upTime);
    istringstream str(upTime);
    istream_iterator<string> beg(str),end;
    vector<string> vlaues(beg,end);

    return stol(vlaues[0]);
}

std::string ProcessParser::getProcUpTime(string pid){
    std::ifstream fstream(Path::basePath() + pid + "/" + Path::statPath());
    try{
        Util::getStream(Path::basePath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    string line;
    float result;

    getline(fstream,line);
    istringstream str(line);
    istream_iterator<string> beg(str),end;
    vector<string> vlaues(beg,end);

    result = stof(vlaues[13])/sysconf(_SC_CLK_TCK);
    return to_string(result);

}
    
string ProcessParser::getProcUser(string pid){
    string line;
    string name = "Uid:";
    string result = "";

    std::ifstream fstream(Path::basePath() + pid + Path::statusPath());
    try{
        Util::getStream(Path::basePath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    // reading Uid line from status file
    while (std::getline(fstream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result =  values[1];
            break;
        }
    }

    // Finding equivilant user to Uid in /etc/passwd file
    try{
        Util::getStream("/etc/passwd", fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    // Searching for name of the user with selected UID
    while (std::getline(fstream, line)) {
        if (line.find(name) != std::string::npos) {
            result = line.substr(0, line.find(":"));
            return result;
        }
    }
    return "";

}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber){

}

float ProcessParser::getSysRamPercent(){

}

string ProcessParser::getSysKernelVersion(){
    std::ifstream fstream(Path::basePath() + Path::versionPath());
    try{
        Util::getStream(Path::basePath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    string version;
    std::getline(fstream,version);

    return version;
}

int ProcessParser::getTotalThreads(){

}

int ProcessParser::getTotalNumberOfProcesses(){

}
    
int ProcessParser::getNumberOfRunningProcesses(){

}

string ProcessParser::getOSName(){
    std::ifstream fstream(Path::basePath() + Path::osNamePath());
    try{
        Util::getStream(Path::basePath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    string osName;
    std::getline(fstream,osName);

    return osName;
}

std::string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2){

}

int ProcessParser::getNumberOfCores(){
    string line;
    string name = "cpu cores";

    std::ifstream fstream(Path::basePath() + "cpuinfo");
    try{
        Util::getStream(Path::basePath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }
    
    while (std::getline(fstream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return stoi(values[3]);
        }
    }
    return 0;
}