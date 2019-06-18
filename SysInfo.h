#include <string>
#include <iostream>
#include <vector>
#include "ProcessParser.h"
class SysInfo {
private:
    std::vector<std::string> lastCpuStats;
    std::vector<std::string> currentCpuStats;
    std::vector<std::string> coresStats;
    std::vector<std::vector<std::string>>lastCpuCoresStats;
    std::vector<std::vector<std::string>>currentCpuCoresStats;
    std::string cpuPercent;
    float memPercent;
    std::string OSname;
    std::string kernelVer;
    long upTime;
    int totalProc;
    int runningProc;
    int threads;
public:

    SysInfo(){
    /*
    Getting initial info about system
    Initial data for individual cores is set
    System data is set
    */
        this->getOtherCores(ProcessParser::getNumberOfCores());
        this->setLastCpuMeasures();
        this->setAttributes();
        this-> OSname = ProcessParser::getOSName();
        this-> kernelVer = ProcessParser::getSysKernelVersion();
    }
    void setAttributes();
    void setLastCpuMeasures();
    std::string getMemPercent()const;
    long getUpTime()const;
    std::string getThreads()const;
    std::string getTotalProc()const;
    std::string getRunningProc()const;
    std::string getKernelVersion()const;
    std::string getOSName()const;
    std::string getCpuPercent()const;
    void getOtherCores(int _size);
    void setCpuCoresStats();
    std::vector<std::string> getCoresStats()const;
};

/*This method initializes attributes in SysInfo class. It takes size, or number of cores, and initializes the object. 
Besides that, this method sets previous data for a specific CPU core.*/
void SysInfo::getOtherCores(int _size){
//when number of cores is detected, vectors are modified to fit incoming data
        this->coresStats = std::vector<std::string>();
        this->coresStats.resize(_size);
        this->lastCpuCoresStats = std::vector<std::vector<std::string>>();
        this->lastCpuCoresStats.resize(_size);
        this->currentCpuCoresStats = std::vector<std::vector<std::string>>();
        this->currentCpuCoresStats.resize(_size);
    for(int i=0;i<_size;i++){
        this->lastCpuCoresStats[i] = ProcessParser::getSysCpuPercent(to_string(i));
    }
}

/* This method updates and creates new datasets for CPU calculation. 
Every core is updated and previous data becomes the current data of calculated measures.*/
void SysInfo::setLastCpuMeasures(){
 this->lastCpuStats = ProcessParser::getSysCpuPercent();
}
void SysInfo::setCpuCoresStats(){
// Getting data from files (previous data is required)
    for(int i=0;i<this->currentCpuCoresStats.size();i++){
        this->currentCpuCoresStats[i] = ProcessParser::getSysCpuPercent(to_string(i));
    }
    for(int i=0;i<this->currentCpuCoresStats.size();i++){
    // after acquirement of data we are calculating every core percentage of usage
        this->coresStats[i] = ProcessParser::PrintCpuStats(this->lastCpuCoresStats[i],this->currentCpuCoresStats[i]);
    }
    this->lastCpuCoresStats = this->currentCpuCoresStats;
}

/*This function initializes or refreshes an object.*/
void SysInfo::setAttributes(){
// getting parsed data
    this-> memPercent = ProcessParser::getSysRamPercent();
    this-> upTime = ProcessParser::getSysUpTime();
    this-> totalProc = ProcessParser::getTotalNumberOfProcesses();
    this-> runningProc = ProcessParser::getNumberOfRunningProcesses();
    this-> threads = ProcessParser::getTotalThreads();
    this->currentCpuStats = ProcessParser::getSysCpuPercent();
    this->cpuPercent = ProcessParser::PrintCpuStats(this->lastCpuStats,this->currentCpuStats);
    this->lastCpuStats = this->currentCpuStats;
    this->setCpuCoresStats();

}
// Constructing string for every core data display
/*This method creates a string with a progress bar. The bar shows the current status of aggregate CPU utilization, or the utilization of a selected core.*/
std::vector<std::string> SysInfo::getCoresStats()const{
    std::vector<std::string> result= std::vector<std::string>();
    for(int i=0;i<this->coresStats.size();i++){
        std::string temp =("cpu" + to_string(i) +": ");
        float check;
        if(!this->coresStats[i].empty())
            check = stof(this->coresStats[i]);
        if(!check || this->coresStats[i] == "nan"){
            return std::vector<std::string>();
        }
        temp += Util::getProgressBar(this->coresStats[i]);
        result.push_back(temp);
    }
    return result;
}
std::string SysInfo::getCpuPercent()const {
    return this->cpuPercent;
}
std::string SysInfo::getMemPercent()const {
    return to_string(this->memPercent);
}
long SysInfo::getUpTime()const {
    return this->upTime;
}
std::string SysInfo::getKernelVersion()const {
    return this->kernelVer;
}
std::string SysInfo::getTotalProc()const {
    return to_string(this->totalProc);
}
std::string SysInfo::getRunningProc()const {
    return to_string(this->runningProc);
}
std::string SysInfo::getThreads()const {
    return to_string(this->threads);
}
std::string SysInfo::getOSName()const {
    return this->OSname;
}
