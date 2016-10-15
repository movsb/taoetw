#pragma once

#include <string>
#include <vector>
#include <functional>
#include <regex>

#include "etwlogger.h"

namespace taoetw {

class EventContainer
{
protected:
    typedef ETWLogger::LogDataUI EVENT;
    typedef std::function<bool(const EVENT*)> FILTER;
    typedef std::vector<EVENT*> EVENTS;

public:
    EventContainer()
        : base_int(0)
    {}

    EventContainer(const std::wstring& name, std::wstring& base, const std::wstring regex, int base_int)
        : name(name)
        , base(base)
        , rule(regex)
        , base_int(base_int)
    {
        _reobj = std::wregex(regex, std::regex_constants::icase);
        _filter = [&](const EVENT* evt) {
            return !std::regex_search(evt->text, _reobj);
        };
    }

public:
    bool add(EVENT* evt);
    bool filter_results(EventContainer* container);
    EVENT* operator[](int index) { return _events[index]; }
    size_t size() const { return _events.size(); }

public:
    std::wstring name;
    std::wstring base;
    int          base_int;
    std::wstring rule;

protected:

    EVENTS          _events;
    FILTER          _filter;

    std::wregex     _reobj;
};

typedef std::vector<EventContainer*> EventContainerS;

}
