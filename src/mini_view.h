#pragma once

namespace taoetw {

class MiniView : public taowin::window_creator
{
private:
    EventContainer&     _events;
    taowin::listview*   _listview;
    std::function<void()> _on_close;

public:
    MiniView(EventContainer& events)
        : _events(events)
    {
    }

    ~MiniView()
    {
    }

    void update();
    void on_close(std::function<void()> fn) { _on_close = fn; }

protected:
    virtual LPCTSTR get_skin_xml() const override;
    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) override;
    virtual void on_final_message() override { __super::on_final_message(); delete this; }
};

}