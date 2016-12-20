#pragma once

#include "misc/mem_pool.hpp"

#include "_module_entry.hpp"
#include "_logdata_define.hpp"
#include "tooltip_window.h"

#include "listview_color.h"
#include "column_selection.h"
#include "event_container.h"
#include "event_detail.h"
#include "module_manager.h"
#include "result_filter.h"
#include "json_visual.h"
#include "lua_console.h"

#include "log/controller.h"
#include "log/consumer.h"
#include "log/dbgview.h"

namespace taoetw {

struct LoggerMessage {
    enum Value {
        __Start = 0,
        LogEtwMsg,
        LogDbgMsg,
        AllocLogUI,
    };
};

struct LogSysType
{
    enum Value
    {
        __Start,
        EventTracing,
        DebugView,
        __End,
    };
};

class MainWindow : public taowin::window_creator
{
private:
    static const UINT kDoLog = WM_USER + 1;

public:
    MainWindow(LogSysType::Value type)
        : _logsystype(type)
        , _last_search_line(-1)
    {
        _tipwnd = new TooltipWindow;
    }

    ~MainWindow()
    {
        // 日志结构体是由内存池管理的
        // 所以要强制手动释放，不能等到智能指针析构的时候进行
        _clear_results();

        _tipwnd = nullptr;
    }

    bool isetw() const { return _logsystype == LogSysType::EventTracing; }
    bool isdbg() const { return _logsystype == LogSysType::DebugView; }
    
protected:
    LogSysType::Value   _logsystype;
    JsonWrapper         _config;

    taowin::listview*   _listview;
    taowin::button*     _btn_start;
    taowin::button*     _btn_clear;
    taowin::button*     _btn_modules;
    taowin::button*     _btn_filter;
    taowin::button*     _btn_topmost;
    taowin::edit*       _edt_search;
    taowin::combobox*   _cbo_search_filter;
    taowin::button*     _btn_colors;
    taowin::button*     _btn_export2file;
    taowin::combobox*   _cbo_sel_flt;
    taowin::combobox*   _cbo_prj;
    taowin::button*     _btn_tools;

    TooltipWindow*      _tipwnd;

    HACCEL              _accels;
    taowin::menu_manager _lvmenu;
    taowin::menu_manager _tools_menu;

    MapColors           _colors;
    ColumnManager       _columns;

    ModuleContainer     _modules;
    ModuleEntry*        _current_project;

    std::map<ModuleEntry*, EventPair> _projects;

    EventContainer*     _events;
    EventContainerS*    _filters;
    EventContainer*     _current_filter;

    ModuleLevelMap      _level_maps;

    int                 _last_search_line;
    std::wstring        _last_search_string;
    bool                _last_search_matched_cols[LogDataUI::cols()];

    Controller          _controller;
    Consumer            _consumer;
    DebugView           _dbgview;

    Guid2Module         _guid2mod;

    MemPoolT<LogDataUI, 8192>
                        _log_pool;

protected:
    virtual LPCTSTR get_skin_xml() const override;
    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT control_message(taowin::syscontrol* ctl, UINT umsg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT on_menu(const taowin::MenuIds& ids) override;
    virtual LRESULT on_accel(int id) override;
    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) override;
    virtual bool filter_special_key(int vk) override;
    virtual void on_final_message() override { __super::on_final_message(); delete this; }
    virtual bool filter_message(MSG* msg) override;

protected:
    bool _start();
    bool _stop();

    void _init_listview();
    void _init_config();

    // 从配置文件初始化模块，并映射空的过滤器
    void _init_projects();

    // 从配置文件初始化模块过滤器
    void _init_filters();

    void _init_project_events();
    void _init_filter_events();
    void _init_logger_events();
    void _view_detail(int i);
    void _manage_modules();
    void _show_filters();
    bool _do_search(const std::wstring& s, int line, int col);
    void _clear_results();
    void _set_top_most(bool top);

    // 更新主界面搜索栏可用的列
    void _update_search_filter();

    // 导出当前过滤器内容到文件
    void _export2file();

    // 更新主界面过滤器列表
    void _update_filter_list(EventContainer* p);

    // 更新项目列表
    void _update_project_list(ModuleEntry* m);


    // 复制当前选中的行内容到剪贴板
    // 只复制第 1 个选中的行
    void _copy_selected_item();

protected:
    void _save_filters();

protected:
    LRESULT _on_create();
    LRESULT _on_close();
    LRESULT _on_log(LoggerMessage::Value msg, LPARAM lParam);
    LRESULT _on_custom_draw_listview(NMHDR* hdr);
    LRESULT _on_get_dispinfo(NMHDR* hdr);
    LRESULT _on_select_column();
    LRESULT _on_drag_column(NMHDR* hdr);
	void	_on_drop_column();
    LRESULT _on_contextmenu(HWND hSender, int x, int y);
    LRESULT _on_init_popupmenu(HMENU hPopup);

protected:
    ModuleEntry* _module_from_guid(const GUID& guid);
};

}
