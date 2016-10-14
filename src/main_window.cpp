
#include <cassert>

#include <string>
#include <unordered_map>

#include <windows.h>
#include <evntrace.h>

#include "etwlogger.h"

#include <taowin/src/tw_taowin.h>

#include "main_window.h"

static const wchar_t* g_etw_session = L"taoetw-session";

namespace taoetw {

Consumer* g_Consumer;

LPCTSTR MainWindow::get_skin_xml() const
{
    LPCTSTR json = LR"tw(
<window title="ETW Log Viewer" size="820,600">
    <res>
        <font name="default" face="微软雅黑" size="12"/>
        <font name="12" face="微软雅黑" size="12"/>
    </res>
    <root>
        <vertical padding="5,5,5,5">
            <listview name="lv" style="singlesel,showselalways,ownerdata" exstyle="clientedge">  </listview>
        </vertical>
    </root>
</window>
)tw";
    return json;
}

LRESULT MainWindow::handle_message(UINT umsg, WPARAM wparam, LPARAM lparam)
{
    switch (umsg) {
    case WM_CREATE:
    {
        _listview = _root->find<taowin::listview>(L"lv");

        _columns.push_back({ L"时间", true, 160 });
        _columns.push_back({ L"进程", true, 50 });
        _columns.push_back({ L"线程", true, 50, });
        _columns.push_back({ L"项目", true, 100, });
        _columns.push_back({ L"文件", true, 200, });
        _columns.push_back({ L"函数", true, 100, });
        _columns.push_back({ L"行号", true, 50, });
        _columns.push_back({ L"日志", true, 300, });

        for (int i = 0; i < (int)_columns.size(); i++) {
            auto& col = _columns[i];
            _listview->insert_column(col.name.c_str(), col.width, i);
        }

        _colors[TRACE_LEVEL_INFORMATION]    = {RGB(  0,   0,   0), RGB(255, 255, 255)};
        _colors[TRACE_LEVEL_WARNING]        = {RGB(255, 128,   0), RGB(255, 255, 255)};
        _colors[TRACE_LEVEL_ERROR]          = {RGB(255,   0,   0), RGB(255, 255, 255)};
        _colors[TRACE_LEVEL_CRITICAL]       = {RGB(255, 255, 255), RGB(255,   0,   0)};
        _colors[TRACE_LEVEL_VERBOSE]        = {RGB(  0,   0,   0), RGB(255, 255, 255)};

        MenuEntry menu;

        menu.name = L"模块管理";
        menu.onclick = [&]() {
            auto* mgr = new ModuleManager(_modules);
            mgr->domodal(this);
        };

        _menus.push_back(menu);

        HMENU hMenu = ::CreateMenu();
        for (int i = 0; i < (int)_menus.size(); i++)
            ::AppendMenu(hMenu, MF_STRING, i, _menus[i].name.c_str());
        ::SetMenu(_hwnd, hMenu);

        _controller.start(g_etw_session);

        GUID guid = { 0x630514b5, 0x7b96, 0x4b74,{ 0x9d, 0xb6, 0x66, 0xbd, 0x62, 0x1f, 0x93, 0x86 } };
        _controller.enable(guid, true, 0);

        taoetw::g_Consumer = &_consumer;

        _consumer.init(_hwnd, WM_USER + 0);
        _consumer.start(g_etw_session);

        return 0;
    }

    case WM_USER + 0:
    {
        auto item = reinterpret_cast<ETWLogger::LogDataUI*>(lparam);

        const std::wstring* root = nullptr;

        _module_from_guid(item->guid, &item->strProject, &root);

        // 相对路径
        if (*item->file && root) {
            if (::wcsnicmp(item->file, root->c_str(), root->size()) == 0) {
                item->offset_of_file = (int)root->size();
            }
        }
        else {
            item->offset_of_file = 0;
        }

        _events.push_back(item);
        _listview->set_item_count(_events.size(), LVSICF_NOINVALIDATEALL);

        break;
    }
    }
    return __super::handle_message(umsg, wparam, lparam);
}

LRESULT MainWindow::on_menu(int id, bool is_accel)
{
    if (id < (int)_menus.size()) {
        _menus[id].onclick();
    }
    return 0;
}

LRESULT MainWindow::on_notify(HWND hwnd, taowin::control * pc, int code, NMHDR * hdr)
{
    if (!pc) {
        if (hwnd == _listview->get_header()) {
            if (code == NM_RCLICK) {
                auto colsel = new ColumnSelection(_columns);

                colsel->OnToggle([&](int i) {
                    auto& col = _columns[i];
                    _listview->set_column_width(i, col.show ? col.width : 0);
                });

                colsel->domodal(this);

                return 0;
            }
            else if (code == HDN_ENDTRACK) {
                auto nmhdr = (NMHEADER*)hdr;
                auto& item = nmhdr->pitem;
                auto& col = _columns[nmhdr->iItem];

                col.show = item->cxy != 0;
                if (item->cxy) col.width = item->cxy;

                return 0;
            }
        }

        return 0;
    }

    if (pc->name() == _T("lv")) {
        if (code == LVN_GETDISPINFO) {
            auto pdi = reinterpret_cast<NMLVDISPINFO*>(hdr);
            auto evt = _events[pdi->item.iItem];
            auto lit = &pdi->item;

            const TCHAR* value = _T("");

            switch (lit->iSubItem) {
            case 0: value = evt->strTime.c_str();               break;
            case 1: value = evt->strPid.c_str();                break;
            case 2: value = evt->strTid.c_str();                break;
            case 3: value = evt->strProject.c_str();            break;
            case 4: value = evt->file + evt->offset_of_file;    break;
            case 5: value = evt->func;                          break;
            case 6: value = evt->strLine.c_str();               break;
            case 7: value = evt->text;                          break;
            }

            lit->pszText = const_cast<TCHAR*>(value);

            return 0;
        }
        else if (code == NM_CUSTOMDRAW) {
            LRESULT lr = CDRF_DODEFAULT;
            ETWLogger::LogDataUI* log;

            auto lvcd = (LPNMLVCUSTOMDRAW)hdr;

            switch (lvcd->nmcd.dwDrawStage) {
            case CDDS_PREPAINT:
                lr = CDRF_NOTIFYITEMDRAW;
                break;

            case CDDS_ITEMPREPAINT:
                log = _events[lvcd->nmcd.dwItemSpec];
                lvcd->clrText = _colors[log->level].fg;
                lvcd->clrTextBk = _colors[log->level].bg;
                lr = CDRF_NEWFONT;
                break;
            }

            return lr;
        }
        else if (code == NM_DBLCLK) {
            auto nmlv = reinterpret_cast<NMITEMACTIVATE*>(hdr);

            if (nmlv->iItem != -1) {
                auto evt = _events[nmlv->iItem];
                auto& cr = _colors[evt->level];
                auto detail_window = new EventDetail(evt, cr.fg, cr.bg);
                detail_window->create();
                detail_window->show();
            }

            return 0;
        }
    }
    return 0;
}

void MainWindow::_module_from_guid(const GUID & guid, std::wstring * name, const std::wstring ** root)
{
    if (!_guid2mod.count(guid)) {
        for (auto& item : _modules) {
            if (::IsEqualGUID(item->guid, guid)) {
                _guid2mod[guid] = item;
                break;
            }
        }
    }

    auto it = _guid2mod.find(guid);

    if (it != _guid2mod.cend()) {
        *name = it->second->name;
        *root = &it->second->root;
    }
    else {
        *name = L"<unknown>";
        *root = nullptr;
    }
}

}
