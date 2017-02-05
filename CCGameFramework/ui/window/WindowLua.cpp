#include "stdafx.h"
#include "Window.h"

int ui_clear_scene(lua_State *L)
{
    for (auto& timer : window->setTimer)
    {
        ::KillTimer(window->handle, timer);
    }
    window->ptrEle = 0;
    window->mapEle.clear();
    window->setTimer.clear();
    window->root->GetChildren().clear();
    return 0;
}

int ui_add_obj(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, -1, "type");
    auto type = ElementId(cint(luaL_checkinteger(L, -1)));
    lua_pop(L, 1);
    window->ptrEle++;
    auto ptr = window->ptrEle;
    while (window->mapEle.find(ptr) != window->mapEle.end())
    {
        ptr++;
        if (ptr > 0x10000000)
            ptr = 0;
    }
    window->ptrEle = ptr;
    lua_getfield(L, -1, "parent");
    auto parent = ElementId(cint(luaL_checkinteger(L, -1)));
    decltype(window->root) p;
    if (parent == -1)
        p = window->root;
    else if (window->mapEle.find(parent) != window->mapEle.end())
        p = window->mapEle[parent].get();
    else
        p = window->root;
    RefPtr<IGraphicsElement> obj;
    switch (type)
    {
    case SolidBackground:
    {
        obj = SolidBackgroundElement::Create();
        
    }
    break;
    case SolidLabel:
    {
        obj = SolidLabelElement::Create();
    }
    break;
    case GradientBackground:
    {
        obj = GradientBackgroundElement::Create();
    }
    break;
    default:
        return luaL_argerror(L, 1, "Invalid obj id");
    }
    obj->GetFlags().parent = p;
    p->GetChildren().push_back(obj);
    window->mapEle.insert(std::pair<decltype(ptr), decltype(obj)>(ptr, obj.get()));
    lua_pushinteger(L, ptr);
    return 1;
}

int ui_update_obj(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, -1, "type");
    auto type = ElementId(cint(luaL_checkinteger(L, -1))); lua_pop(L, 1);
    lua_getfield(L, -1, "handle");
    auto handle = cint(luaL_checkinteger(L, -1)); lua_pop(L, 1);
    if (window->mapEle.find(handle) == window->mapEle.end())
        return luaL_argerror(L, 1, "Invalid obj id");
    auto o = window->mapEle[handle];
    if (o->GetTypeId() != type)
        return luaL_argerror(L, 1, "Invalid obj type");
    {
        CRect rt;
        lua_getfield(L, -1, "left");
        rt.left = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        lua_getfield(L, -1, "top");
        rt.top = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        lua_getfield(L, -1, "right");
        rt.right = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        lua_getfield(L, -1, "bottom");
        rt.bottom = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        o->SetRenderRect(rt);
    }
    {
        lua_getfield(L, -1, "show_self");
        o->GetFlags().self_visible = lua_toboolean(L, -1) == 1; lua_pop(L, 1);
        lua_getfield(L, -1, "show_children");
        o->GetFlags().children_visible = lua_toboolean(L, -1) == 1; lua_pop(L, 1);
    }
    switch (type)
    {
    case SolidBackground:
    {
        auto obj = static_cast<SolidBackgroundElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checklstring(L, -1, NULL); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(color));
        }
    }
    break;
    case SolidLabel:
    {
        auto obj = static_cast<SolidLabelElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checklstring(L, -1, NULL); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(color));
        }
        {
            Font font;
            lua_getfield(L, -1, "size");
            font.size = cint(luaL_checkinteger(L, -1)); lua_pop(L, 1);
            lua_getfield(L, -1, "family");
            font.fontFamily = CString(luaL_checklstring(L, -1, NULL)); lua_pop(L, 1);
            obj->SetFont(font);
        }
        {
            lua_getfield(L, -1, "text");
            obj->SetText(CString(luaL_checklstring(L, -1, NULL))); lua_pop(L, 1);
        }
        {
            lua_getfield(L, -1, "align");
            obj->SetHorizontalAlignment(Alignment(cint(luaL_checkinteger(L, -1)))); lua_pop(L, 1);
            lua_getfield(L, -1, "valign");
            obj->SetVerticalAlignment(Alignment(cint(luaL_checkinteger(L, -1)))); lua_pop(L, 1);
        }
    }
    break;
    case GradientBackground:
    {
        auto obj = static_cast<GradientBackgroundElement*>(o.get());
        {
            lua_getfield(L, -1, "color1");
            auto color1 = luaL_checklstring(L, -1, NULL); lua_pop(L, 1);
            obj->SetColor1(CColor::Parse(color1));
            lua_getfield(L, -1, "color2");
            auto color2 = luaL_checklstring(L, -1, NULL); lua_pop(L, 1);
            obj->SetColor2(CColor::Parse(color2));
        }
        {
            lua_getfield(L, -1, "direction");
            obj->SetDirection(GradientBackgroundElement::Direction(cint(luaL_checkinteger(L, -1)))); lua_pop(L, 1);
        }
    }
    break;
    default:
        break;
    }
    return 0;
}

int ui_info(lua_State *L)
{
    lua_newtable(L);
    auto size = window->GetClientWindowSize();
    lua_pushstring(L, "width");
    lua_pushinteger(L, size.cx);
    lua_settable(L, -3);
    lua_pushstring(L, "height");
    lua_pushinteger(L, size.cy);
    lua_settable(L, -3);
    return 1;
}

int ui_paint(lua_State *L)
{
    window->Redraw();
    return 0;
}

int ui_set_timer(lua_State *L)
{
    auto id = cint(luaL_checkinteger(L, 1));
    auto elapse = cint(luaL_checkinteger(L, 2));
    window->SetTimer(id, elapse);
    return 0;
}

int ui_kill_timer(lua_State *L)
{
    auto id = cint(luaL_checkinteger(L, 1));
    window->KillTimer(id);
    return 0;
}