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
    static float get_sys_active_cpu_time(vector<string> values);
    static float get_sys_idle_cpu_time(vector<string>values);
};

// TODO: Define all of the above functions below:
bool ProcessParser::isPidExisting(string pid){
    std::ifstream fstream;
    try{
        Util::getStream(Path::basePath() + pid, fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    return true;
}
string ProcessParser::getCmd(string pid){
    std::ifstream fstream;
    try{
        Util::getStream(Path::basePath() + pid + Path::cmdPath(), fstream);
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
    std::ifstream fstream;
    float result;
    string line;
    string inputString = "VmData";

    try{
        Util::getStream(Path::basePath() + pid + Path::statusPath(), fstream);
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
    std::ifstream fstream;
    try{
        Util::getStream(Path::basePath() + pid + "/" + Path::statPath(), fstream);
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
    std::ifstream fstream;
    try{
        Util::getStream(Path::basePath() + Path::upTimePath(), fstream);
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
    std::ifstream fstream;
    try{
        Util::getStream(Path::basePath() + pid + "/" + Path::statPath(), fstream);
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

    std::ifstream fstream;
    try{
        Util::getStream(Path::basePath() + pid + Path::statusPath(), fstream);
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
    string line;
    string name = "cpu" + coreNumber;
    std::ifstream fstream;
    try{
        Util::getStream(Path::basePath() + Path::statPath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

     while (std::getline(fstream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            // set of cpu data active and idle times;
            return values;
        }
    }
    return (vector<string>());

}

float ProcessParser::getSysRamPercent(){
    std::ifstream fstream;
    try{
        Util::getStream(Path::basePath() + Path::memInfoPath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    string line;
    string name0 = "MemTotal:";
    string name1 = "MemFree:";
    string name2 = "MemAvailable:";
    string name3 = "Buffers:";

    float totalMem = 0.0;
    float memFree = 0.0;
    float memAval = 0.0;
    float buffers = 0.0;

    while (std::getline(fstream, line))
    {
        if(line.compare(0, name0.size(),name0) == 0){
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);

            totalMem = stof(values[1]);
        }

        if(line.compare(0, name1.size(),name0) == 0){
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);

            memFree = stof(values[1]);
        }

        if(line.compare(0, name2.size(),name0) == 0){
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);

            memAval = stof(values[1]);
        }

        if(line.compare(0, name3.size(),name0) == 0){
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);

            buffers = stof(values[1]);

            // I do not like this :( 
            break;
        }
    }
    


    return float(100.0 * (1 - (memFree/(memAval-buffers))));
}

string ProcessParser::getSysKernelVersion(){
    std::ifstream fstream;
    try{
        Util::getStream(Path::basePath() + Path::versionPath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    string version = "Linux";
    string line;

    while(std::getline(fstream,line)){
        if(line.compare(0, version.size(), version) == 0){
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);

            return values[2];

        }
    }

    return "";
}

int ProcessParser::getTotalThreads(){
    string line;
    int result = 0;
    string name = "Threads:";
    vector<string> _list = ProcessParser::getPidList();
    for (int i=0 ; i<_list.size();i++) {
        string pid = _list[i];
        //getting every process and reading their number of their threads
        std::ifstream fstream ;
        try{
            Util::getStream(Path::basePath() + pid + Path::statusPath(), fstream);
        } catch (std::string &exp) {
            std::cout << exp << std::endl;
        }
        while (std::getline(fstream, line)) {
            if (line.compare(0, name.size(), name) == 0) {
                istringstream buf(line);
                istream_iterator<string> beg(buf), end;
                vector<string> values(beg, end);
                result += stoi(values[1]);
                break;
            }
        }
    }
    return result;

}

int ProcessParser::getTotalNumberOfProcesses(){
    string line;
    int result = 0;
    string name = "processes";
    ifstream fstream ;
    try{
        Util::getStream(Path::basePath() + Path::statPath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }
    while (std::getline(fstream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;

}
    
int ProcessParser::getNumberOfRunningProcesses(){
    string line;
    int result = 0;
    string name = "procs_running";
    ifstream fstream ;
    try{
        Util::getStream(Path::basePath() + Path::statPath(), fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }
    while (std::getline(fstream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;

}

string ProcessParser::getOSName(){
    std::ifstream fstream;
    try{
        // Util::getStream(Path::basePath() + Path::osNamePath(), fstream);
        // Reading os name from a different source
        Util::getStream("/etc/os-release", fstream);
    } catch (std::string &exp) {
        std::cout << exp << std::endl;
    }

    string name = "PRETTY_NAME=";
    string line;

    while(std::getline(fstream,line)){
        if(line.compare(0, name.size(), name) == 0){
            // istringstream buf(line);
            // istream_iterator<string> beg(buf), end;
            // vector<string> values(beg, end);

            // return values[1] + values[2];
            std::size_t found = line.find("=");
            found++;
            string result = line.substr(found);
            result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
            return result;
        }
    }

    return "";
}

std::string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2){
    float activeTime = ProcessParser::get_sys_active_cpu_time(values2) - ProcessParser::get_sys_active_cpu_time(values1);
    float idleTime = ProcessParser::get_sys_idle_cpu_time(values2) - ProcessParser::get_sys_idle_cpu_time(values1);
    float totalTime = activeTime + idleTime;
    float result = 100.0 * (activeTime/totalTime);

    std::string time = to_string(result);

    return time;

}

int ProcessParser::getNumberOfCores(){
    string line;
    string name = "cpu cores";

    std::ifstream fstream;
    try{
        Util::getStream(Path::basePath() + "cpuinfo", fstream);
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

/* These functions for calculating active and idle time are a direct extension of the system CPU percentage. 
They sort and categorize a newly created string vector,
which contains parsed raw data from file. Because most of the data is recorded as time, 
we are selecting and summing all active and idle time. */

float ProcessParser::get_sys_active_cpu_time(vector<string> values)
{
    return (stof(values[S_USER]) +
            stof(values[S_NICE]) +
            stof(values[S_SYSTEM]) +
            stof(values[S_IRQ]) +
            stof(values[S_SOFTIRQ]) +
            stof(values[S_STEAL]) +
            stof(values[S_GUEST]) +
            stof(values[S_GUEST_NICE]));
}

float ProcessParser::get_sys_idle_cpu_time(vector<string>values)
{
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}