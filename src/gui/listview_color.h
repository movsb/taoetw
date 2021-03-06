#pragma once

namespace taolog {

struct ItemColor
{ 
    COLORREF fg;
    COLORREF bg;

    ItemColor(COLORREF fg_ = RGB(0,0,0), COLORREF bg_=RGB(255,255,255))
        : fg(fg_)
        , bg(bg_)
    { }
};

typedef std::unordered_map<int /* level */, ItemColor> MapColors;

class ListviewColor : public taowin::WindowCreator
{
public:
    typedef std::function<void(int i, bool fg)> OnColorChange;

public:
    ListviewColor(MapColors* colors, ModuleLevelMap* levels ,OnColorChange set)
        : _colors(colors)
        , _levels(levels)
        , _set(set)
    {

    }

private:
    MapColors*          _colors;
    ModuleLevelMap*     _levels;
    OnColorChange       _set;
    std::map<HWND, int> _label_maps;
    std::map<HWND, HBRUSH> _brush_maps;

protected:
    void choose_color_for(int i, bool fg);


protected:
    virtual void get_metas(WindowMeta* metas) override;
    virtual LPCTSTR get_skin_xml() const override;
    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT on_notify(HWND hwnd, taowin::Control* pc, int code, NMHDR* hdr) override;
    virtual void on_final_message() override { __super::on_final_message(); delete this; }
};

}
