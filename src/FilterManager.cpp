#include "FilterManager.h"
#include "InterfaceFilter.h"
#include "Types.h"

FilterManager* FilterManager::instance_ = 0;

FilterManager* FilterManager::getInstance() {
    if (instance_ == NULL) {
        instance_ = new FilterManager();
    }

    return instance_;
}

FilterManager::FilterManager():
    queue_number_(0)
{
}

FilterManager::~FilterManager() {
    for(auto filter : filter_) {
        delete filter;
    }
}

bool FilterManager::addFilter(const filter_rule filter) {

    struct mark_rule* mr = new mark_rule;

    mr->action_ = stringToAction(filter.action_);
    mr->arg_ = filter.action_param_;

    if(((mr->action_ == ACTION::RUNSCRIPT || mr->action_ == ACTION::STARTVM) && mr->arg_ == "") || mr->action_ == ACTION::UNKOWN) {
        delete mr;
        return false;
    }

    filter_.push_back(mr);
    return true;
}

struct FilterManager::action_answer* FilterManager::handleFilter(uint32_t mark) {
    struct action_answer* aw = new action_answer;
    for( auto filter : filter_) {
        if(filter->mark_ == mark) {
            aw->action_ = filter->action_;
            aw->arg_ = filter->arg_;
            return aw;
        }
    }
    aw->action_ = ACTION::UNKOWN;
    return aw;
}

void FilterManager::addRequest(const request *rq)
{
    core_->addRequest(*rq);
}

void FilterManager::addCore(EMPCACore *core)
{
    core_ = core;
}

InterfaceFilter *FilterManager::createNewInterfaceFilter(const char* dev)
{
    InterfaceFilter* inter = new InterfaceFilter(queue_number_, dev);

    queue_number_++;

    interface_filter_.push_back(inter);

    return inter;
}

ACTION FilterManager::stringToAction(std::string action_string)
{
    if(action_string == "DROP") {
        return ACTION::DROP;
    } else if(action_string == "FREEZEALL") {
      return ACTION::FREEZEALL;
    } else if(action_string == "FREEZESRC") {
        return ACTION::FREEZESRC;
    }else if(action_string == "RUNSCRIPT") {
        return ACTION::RUNSCRIPT;
    } else if(action_string == "STARTVM") {
        return ACTION::STARTVM;
    }
    return ACTION::UNKOWN;
}
