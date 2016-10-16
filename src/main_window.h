#pragma once

#include <taowin/src/tw_taowin.h>

#include "etwlogger.h"

#include "_module_entry.hpp"
#include "event_container.h"

#include "column_selection.h"
#include "module_manager.h"
#include "event_detail.h"
#include "result_filter.h"

#include "controller.h"
#include "consumer.h"

namespace taoetw {

class MainWindow : public taowin::window_creator
{
public:
    struct ItemColor
    { 
        COLORREF fg;
        COLORREF bg;

        ItemColor(COLORREF fg_ = RGB(0,0,0), COLORREF bg_=RGB(255,255,255))
            : fg(fg_)
            , bg(bg_)
        { }
    };

    typedef std::map<int /* level */, ItemColor> MapColors;

private:
    static const UINT kDoLog = WM_USER + 1;

public:
    MainWindow()
        : _listview(nullptr)
        , _btn_start(nullptr)
        , _btn_stop(nullptr)
        , _btn_modules(nullptr)
        , _last_search_index(-1)
    {

    }
    
private:
    taowin::listview*   _listview;
    taowin::button*     _btn_start;
    taowin::button*     _btn_stop;
    taowin::button*     _btn_modules;
    taowin::button*     _btn_filter;
    taowin::button*     _btn_topmost;
    taowin::edit*       _edt_search;

    MapColors           _colors;
    ColumnContainer     _columns;

    ModuleContainer     _modules;
    EventContainer      _events;
    EventContainerS     _filters;
    EventContainer*     _current_filter;
    ModuleLevelMap      _level_maps;

    int                 _last_search_index;
    std::wstring        _last_search_string;

    Controller          _controller;
    Consumer            _consumer;

    Guid2Module         _guid2mod;

protected:
    virtual LPCTSTR get_skin_xml() const override;
    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT on_menu(int id, bool is_accel = false);
    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) override;
    virtual bool filter_special_key(int vk) override;

protected:
    bool _start();
    bool _stop();

    void _init_listview();
    void _init_menu();
    void _view_detail(int i);
    void _manage_modules();
    void _show_filters();
    bool _do_search(const std::wstring& s, int start);

    LRESULT _on_create();
    LRESULT _on_log(ETWLogger::LogDataUI* log);
    LRESULT _on_custom_draw_listview(NMHDR* hdr);
    LRESULT _on_get_dispinfo(NMHDR* hdr);
    LRESULT _on_select_column();
    LRESULT _on_drag_column(NMHDR* hdr);

protected:
    void _module_from_guid(const GUID& guid, std::wstring* name, const std::wstring** root);
};

}
