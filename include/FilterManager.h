#ifndef FILTERMANAGER_H
#define FILTERMANAGER_H

#include "EMPCACore.h"
#include "Types.h"

#include <string>
#include <vector>

class InterfaceFilter;

class FilterManager {
    public:
        ~FilterManager();
        struct action_answer {
            ACTION action_;
            std::string arg_;
        };
        static FilterManager* getInstance();

        bool addFilter(const filter_rule filter);
        struct FilterManager::action_answer* handleFilter(uint32_t mark);

        void addRequest(const request* rq);

        void addCore(EMPCACore* core);

        InterfaceFilter *createNewInterfaceFilter(const char* dev);

    private:
        FilterManager();

        ACTION stringToAction(std::string action_string);

        struct mark_rule{
            uint32_t mark_;
            ACTION action_;
            std::string arg_;
        };

        static FilterManager* instance_;
        EMPCACore* core_;

        std::vector<mark_rule*> filter_;
        std::vector<InterfaceFilter*> interface_filter_;
        uint16_t queue_number_;
};

#endif // FILTERMANAGER_H
