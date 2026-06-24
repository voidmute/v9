#include "includes.hpp"

namespace
{
    int g_page = 0;
    bool g_minimized = false;
    ImVec2 g_menuPos(0.0f, 0.0f);
    bool g_menuPosInit = false;
    bool g_prevMenuOpen = false;

    float Clamp(float v, float lo, float hi)
    {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }
}

void Menu::Init()
{
    if (cfg->bMenuOpen && !g_prevMenuOpen)
    {
        g_minimized = false;
        UiTheme::ResetNavIndicator();
    }
    g_prevMenuOpen = cfg->bMenuOpen;

    UiTheme::BeginFrame();
    UiTheme::SetMenuOpen(cfg->bMenuOpen);
    UiTheme::ApplyStyle();

    const float slide = UiTheme::Lerp(12.0f, 0.0f, UiTheme::EaseOut(UiTheme::MenuOpenProgress()));
    const float titleH = UiTheme::TitleBarHeight();
    const float winW = g_minimized ? UiTheme::Scale(280.0f) : UiTheme::Scale(520.0f);
    const float footerH = UiTheme::Scale(58.0f);
    const float winH = g_minimized ? (titleH + UiTheme::Scale(2.0f)) : UiTheme::Scale(580.0f);
    const float sidebarW = UiTheme::Scale(156.0f);

    if (!g_menuPosInit)
    {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        g_menuPos = ImVec2(center.x - winW * 0.5f, center.y - winH * 0.5f + slide);
        g_menuPosInit = true;
    }

    const ImVec2 vpSize = ImGui::GetMainViewport()->Size;
    g_menuPos.x = Clamp(g_menuPos.x, 0.0f, (vpSize.x > winW) ? (vpSize.x - winW) : 0.0f);
    g_menuPos.y = Clamp(g_menuPos.y, 0.0f, (vpSize.y > winH) ? (vpSize.y - winH) : 0.0f);

    ImGui::SetNextWindowSize(ImVec2(winW, winH), ImGuiCond_Always);
    ImGui::SetNextWindowPos(g_menuPos, ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("##v9_menu", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

    ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
    const UiTheme::TitleBarResult titleBar = UiTheme::DrawTitleBar("v9", winW, g_minimized);
    if (titleBar.dragged)
    {
        g_menuPos.x += titleBar.dragDelta.x;
        g_menuPos.y += titleBar.dragDelta.y;
        ImGui::SetWindowPos(g_menuPos);
    }

    const ImVec2 wpos = ImGui::GetWindowPos();
    const ImVec2 wsize = ImGui::GetWindowSize();
    g_menuPos = wpos;
    UiTheme::DrawWindowChrome(wpos, wsize);

    if (titleBar.close)
    {
        cfg->bMenuOpen = false;
        g_minimized = false;
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        return;
    }
    if (titleBar.minimize)
        g_minimized = true;
    if (titleBar.restore)
        g_minimized = false;

    if (g_minimized)
    {
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        return;
    }

    const float bodyTop = titleH;
    const float bodyH = wsize.y - titleH - footerH;

    ImGui::SetCursorPos(ImVec2(0.0f, bodyTop));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.024f, 0.024f, 0.024f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.16f, 0.16f, 0.16f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(UiTheme::Scale(6.0f), UiTheme::Scale(6.0f)));
    ImGui::BeginChild("##sidebar_col", ImVec2(sidebarW, bodyH), true, ImGuiWindowFlags_NoScrollbar);

    UiTheme::SidebarItem("Визуал", 0, &g_page);
    UiTheme::SidebarItem("Игроки", 1, &g_page);
    UiTheme::SidebarItem("Цвета", 2, &g_page);
    UiTheme::FinalizeSidebarNav(g_page);

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - UiTheme::Scale(44.0f));
    UiTheme::StatusBadge(cfg->bInitHooks ? "ESP ВКЛ" : "ESP ВЫКЛ", cfg->bInitHooks);

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    ImGui::SetCursorPos(ImVec2(sidebarW, bodyTop));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.04f, 0.04f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(UiTheme::Scale(14.0f), UiTheme::Scale(12.0f)));
    ImGui::BeginChild("##main", ImVec2(wsize.x - sidebarW, bodyH), true, ImGuiWindowFlags_NoScrollbar);

    UiTheme::SetActivePage(g_page);
    UiTheme::BeginPageContent();

    if (g_page == 0)
    {
        UiTheme::SectionHeader("Оверлей", "Выберите, что рисовать в мире");
        UiTheme::BeginSettingCard("overlay");
        if (ImGui::BeginTable("##overlay_tbl", 2, ImGuiTableFlags_SizingStretchSame))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); UiTheme::AnimatedToggle("box", "Рамка", &cfg->bBox);
            ImGui::TableNextColumn(); UiTheme::AnimatedToggle("skel", "Скелет", &cfg->bSkeleton);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); UiTheme::AnimatedToggle("lines", "Линии", &cfg->bLines);
            ImGui::TableNextColumn(); UiTheme::AnimatedToggle("names", "Имена", &cfg->bNames);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); UiTheme::AnimatedToggle("roles", "Роли", &cfg->bRoles);
            ImGui::TableNextColumn(); UiTheme::AnimatedToggle("dist", "Дистанция", &cfg->bDistance);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); UiTheme::AnimatedToggle("enemy", "Только враги", &cfg->bEnemyOnly);
            ImGui::TableNextColumn();
            ImGui::EndTable();
        }
        UiTheme::EndSettingCard();

        UiTheme::SectionHeader("Камера", nullptr);
        UiTheme::BeginSettingCard("camera");
        UiTheme::AnimatedToggle("fov", "Свой FOV", &cfg->bFovChanger);
        if (cfg->bFovChanger)
            UiTheme::AnimatedSliderFloat("fov", "Угол обзора", &cfg->fFovValue, 70.0f, 120.0f, "%.0f");
        UiTheme::EndSettingCard();
    }
    else if (g_page == 1)
    {
        UiTheme::SectionHeader("Игроки", "Телепорт (лучше на хосте)");
        ImGui::TextDisabled("В игре: %d", static_cast<int>(cheat->PlayerInfos.size()));
        if (cheat->PlayerInfos.empty())
        {
            ImGui::Spacing();
            ImGui::TextWrapped("Включите ESP и зайдите в матч.");
        }
        else
        {
            ImGui::BeginChild("##plist_scroll", ImVec2(0, 0), false);
            for (int i = 0; i < static_cast<int>(cheat->PlayerInfos.size()); i++)
            {
                ImGui::PushID(i);
                bool tp = false;
                if (UiTheme::PlayerRow("row", cheat->PlayerInfos[i].Name.c_str(), &tp) && tp)
                    cheat->RequestTeleport(cheat->PlayerInfos[i].Actor);
                ImGui::PopID();
            }
            ImGui::EndChild();
        }
    }
    else
    {
        UiTheme::SectionHeader("Палитра", nullptr);
        UiTheme::BeginSettingCard("palette");
        UiTheme::ColorSwatch("vis", "Видимые", cfg->colVisible);
        UiTheme::ColorSwatch("hid", "Скрытые", cfg->colNotVisible);
        UiTheme::ColorSwatch("line", "Линии к целям", cfg->colLines);
        ImGui::Dummy(ImVec2(0, UiTheme::Scale(6.0f)));
        if (UiTheme::PrimaryButton("Сохранить")) cfg->SaveSettings();
        ImGui::SameLine();
        if (UiTheme::PrimaryButton("Загрузить")) cfg->LoadSettings();
        ImGui::SameLine();
        if (UiTheme::PrimaryButton("Сброс")) cfg->InitializeSettings();
        UiTheme::EndSettingCard();
    }

    UiTheme::EndPageContent();
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    ImGui::SetCursorPos(ImVec2(0.0f, wsize.y - footerH));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.03f, 0.03f, 0.03f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(UiTheme::Scale(12.0f), UiTheme::Scale(8.0f)));
    ImGui::BeginChild("##footer", ImVec2(wsize.x, footerH), false, ImGuiWindowFlags_NoScrollbar);

    static const char* kEnableEsp = "Включить ESP";
    const float padX = UiTheme::Scale(12.0f);
    const float padY = UiTheme::Scale(8.0f);
    const float toggleH = UiTheme::Scale(38.0f);
    const float hintH = ImGui::GetFontSize() + UiTheme::Scale(8.0f);
    const float rowH = toggleH > hintH ? toggleH : hintH;
    const float contentH = footerH - padY * 2.0f;
    const float rowY = padY + (contentH - rowH) * 0.5f;

    const ImVec2 espLabel = ImGui::CalcTextSize(kEnableEsp);
    const float espW = UiTheme::Scale(14.0f) + espLabel.x + UiTheme::Scale(12.0f) + UiTheme::Scale(44.0f) + UiTheme::Scale(14.0f);
    const float hintW = UiTheme::Scale(210.0f);
    const float gap = UiTheme::Scale(18.0f);
    const float totalW = espW + gap + hintW;
    const float contentW = wsize.x - padX * 2.0f;
    const float startX = padX + (contentW - totalW) * 0.5f;

    ImGui::SetCursorPos(ImVec2(startX, rowY));
    UiTheme::EspMasterToggle("master", kEnableEsp, &cfg->bInitHooks);
    ImGui::SameLine(0.0f, gap);
    ImGui::SetCursorPosY(rowY + (toggleH - hintH) * 0.5f);
    UiTheme::KeyHint("INSERT", "меню", "END", "выгрузка");

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
