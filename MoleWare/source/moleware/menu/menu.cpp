#include "menu.h"
#include "../config/config.h"

#include <iostream>
#include <vector>
#include "../config/configmanager.h"

#include "../keybinds/keybinds.h"

#include "../utils/logging/log.h"

void ApplyImGuiTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Color definitions
    const ImVec4 accentPrimary = ImVec4(0.38f, 0.15f, 0.78f, 1.00f);  // Purple
    const ImVec4 accentSecondary = ImVec4(0.28f, 0.10f, 0.58f, 1.00f);  // Darker Purple
    const ImVec4 textPrimary = ImVec4(0.95f, 0.95f, 0.98f, 1.00f);
    const ImVec4 backgroundDark = ImVec4(0.08f, 0.08f, 0.10f, 0.90f); // Semi-transparent
    const ImVec4 backgroundMedium = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);

    // Main colors
    colors[ImGuiCol_WindowBg] = backgroundDark;
    colors[ImGuiCol_ChildBg] = backgroundMedium;
    colors[ImGuiCol_Border] = ImVec4(0.15f, 0.15f, 0.20f, 0.60f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.20f, 0.60f);
    colors[ImGuiCol_FrameBgHovered] = accentSecondary;
    colors[ImGuiCol_FrameBgActive] = accentPrimary;
    colors[ImGuiCol_TitleBg] = backgroundDark;
    colors[ImGuiCol_CheckMark] = accentPrimary;
    colors[ImGuiCol_SliderGrab] = accentPrimary;
    colors[ImGuiCol_SliderGrabActive] = accentSecondary;
    colors[ImGuiCol_Button] = accentSecondary;
    colors[ImGuiCol_ButtonHovered] = accentPrimary;
    colors[ImGuiCol_Header] = accentSecondary;
    colors[ImGuiCol_HeaderHovered] = accentPrimary;
    colors[ImGuiCol_Separator] = ImVec4(0.15f, 0.15f, 0.20f, 0.60f);
    colors[ImGuiCol_Text] = textPrimary;
    colors[ImGuiCol_Tab] = backgroundMedium;
    colors[ImGuiCol_TabHovered] = accentSecondary;
    colors[ImGuiCol_TabActive] = accentPrimary;

    // Style adjustments
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.WindowRounding = 8.f;
    style.ChildRounding = 8.f;
    style.FrameRounding = 4.f;
    style.ScrollbarRounding = 4.f;
    style.GrabRounding = 4.f;
    style.TabRounding = 8.f;
}



Menu::Menu() {
    activeTab = 0;
    showMenu = true;
}

void Menu::init(HWND& window, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11RenderTargetView* mainRenderTargetView) {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(pDevice, pContext);

    ApplyImGuiTheme();

    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f);

    std::cout << "initialized menu\n";
}

void Menu::render() {
    keybind.pollInputs();
    if (showMenu) {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar;

        ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiCond_Once);
        ImGui::SetNextWindowBgAlpha(0.9f); // Overall window transparency

        ImGui::Begin("MoleWare | Internal DOMISEK", nullptr, window_flags);

        // Header with gradient line
        {
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();

            // Main content background with transparency
            ImGui::GetWindowDrawList()->AddRectFilledMultiColor(
                windowPos,
                ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
                ImColor(0.08f, 0.08f, 0.10f, 0.0f),
                ImColor(0.08f, 0.08f, 0.10f, 0.0f),
                ImColor(0.08f, 0.08f, 0.10f, 0.9f),
                ImColor(0.08f, 0.08f, 0.10f, 0.9f)
            );

            // Header text
            float windowWidth = ImGui::GetWindowWidth();
            ImGui::TextColored(accentPrimary, "MoleWare");
            ImGui::SameLine(windowWidth - ImGui::CalcTextSize("discord.gg/moleware").x - 10);
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.9f, 1.0f), "discord.gg/moleware");

            // Separator line
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(windowPos.x, windowPos.y + ImGui::GetCursorPosY()),
                ImVec2(windowPos.x + windowWidth, windowPos.y + ImGui::GetCursorPosY()),
                ImColor(accentPrimary), 1.0f
            );
            ImGui::Dummy(ImVec2(0, 10));
        }

        // Vertical tabs with Neverlose style
        ImGui::BeginChild("LeftTabs", ImVec2(120, 0), true);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 2));

        const char* tabNames[] = { "AIM", "VISUALS", "MISC", "CONFIGS" };
        for (int i = 0; i < IM_ARRAYSIZE(tabNames); i++) {
            bool isSelected = (activeTab == i);
            ImVec2 size = ImVec2(-1, 40);

            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, accentPrimary);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            }

            if (ImGui::Button(tabNames[i], size)) activeTab = i;
            if (isSelected) ImGui::PopStyleColor(2);
            else ImGui::PopStyleColor();
        }

        ImGui::PopStyleVar();
        ImGui::EndChild();

        // Main content area with clean layout
        ImGui::SameLine();
        ImGui::BeginChild("ContentRegion", ImVec2(0, 0), true);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));

        switch (activeTab) {
        case 0:  // Aim Tab
        {
            ImGui::BeginChild("Aim", ImVec2(0, 0), true);
            {
                // General Section
                ImGui::TextColored(accentPrimary, "GENERAL SETTINGS");
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 5));

                // AimBot Toggle
                ImGui::Checkbox("Enable AimBot", &Config::aimbot);
                ImGui::SameLine(ImGui::GetWindowWidth() - 120);
                keybind.menuButton(Config::aimbot);

                // Main Settings
                ImGui::Spacing();
                ImGui::SliderFloat("Aim FOV", &Config::aimbot_fov, 0.f, 90.f, "%.1f°");
                ImGui::SliderFloat("Smoothness", &Config::aimbotSmoothness, 1.0f, 15.0f, "%.1fpx");

                // Additional Features
                ImGui::Dummy(ImVec2(0, 5));
                ImGui::Checkbox("Team Check", &Config::aimbotTeamCheck);
                ImGui::Checkbox("Recoil Control System", &Config::rcs);

                // FOV Circle
                ImGui::Checkbox("Draw FOV Circle", &Config::fov_circle);
                if (Config::fov_circle) {
                    ImGui::ColorEdit4("Circle Color##FovColor", (float*)&Config::fovCircleColor);
                }

                // Triggerbot Section
                ImGui::Dummy(ImVec2(0, 10));
                ImGui::TextColored(accentPrimary, "TRIGGERBOT");
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 5));

                // Triggerbot Settings
                ImGui::Checkbox("Enable Triggerbot", &Config::triggerBot);
                ImGui::SameLine(ImGui::GetWindowWidth() - 120);
                keybind.menuButton(Config::triggerBot);

                ImGui::SliderInt("Delay (ms)", &Config::triggerBotDelay, 0, 90, "%d ms");
            }
            ImGui::EndChild();
        }
        break;

        case 1:  // Visuals Tab
        {
            // Left Column - Player ESP
            ImGui::BeginChild("VisualsLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 5, 0), true);
            {
                // Player ESP Section
                ImGui::TextColored(ImVec4(0.38f, 0.15f, 0.78f, 1.00f), "PLAYER ESP");
                ImGui::Text("TEST - CAN YOU SEE THIS?");
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 5));

                ImGui::Columns(2, "##espColumns", false);
                ImGui::SetColumnWidth(0, 120);

                // Left Column - Toggles
                ImGui::Checkbox("Box", &Config::esp);
                ImGui::Checkbox("Box Fill", &Config::espFill);
                ImGui::Checkbox("Team Check", &Config::teamCheck);
                ImGui::Checkbox("Health Bar", &Config::showHealth);
                ImGui::Checkbox("Armor Bar", &Config::showArmor);
                ImGui::Checkbox("Show Dead", &Config::showDead);  // Added missing element
                ImGui::Checkbox("Name Tags", &Config::showNameTags);  // Added missing element

                // Right Column - Settings
                ImGui::NextColumn();
                if (Config::esp) {
                    ImGui::SliderFloat("Thickness##esp", &Config::espThickness, 1.0f, 5.0f);
                }
                if (Config::espFill) {
                    ImGui::SliderFloat("Opacity##espFill", &Config::espFillOpacity, 0.0f, 1.0f);
                }
                ImGui::ColorEdit4("Color##esp", (float*)&Config::espColor);

                ImGui::Columns(1);
                ImGui::Dummy(ImVec2(0, 10));

                // Additional ESP Features
                ImGui::TextColored(accentPrimary, "EXTRA FEATURES");
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 5));

                ImGui::Checkbox("Show Head Dot", &Config::showHead);
                ImGui::SameLine(ImGui::GetWindowWidth() - 120);
                ImGui::SliderFloat("Radius##head", &Config::headDotRadius, 1.0f, 5.0f);

                ImGui::ColorEdit4("Head Color##dot", (float*)&Config::headDotColor);
                ImGui::Checkbox("Show Distance", &Config::distance);
                ImGui::Checkbox("Show Weapon", &Config::showWeapon);

                // World Settings
                ImGui::Dummy(ImVec2(0, 10));
                ImGui::TextColored(accentPrimary, "WORLD");
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 5));

                ImGui::Checkbox("Night Mode", &Config::Night);
                if (Config::Night) {
                    ImGui::ColorEdit4("Night Color##world", (float*)&Config::NightColor);
                }

                ImGui::Checkbox("Custom FOV", &Config::fovEnabled);
                if (Config::fovEnabled) {
                    ImGui::SliderFloat("FOV Value##world", &Config::fov, 20.0f, 160.0f, "%.0f°");
                }
            }
            ImGui::EndChild();

            // Right Column - Chams
            ImGui::SameLine();
            ImGui::BeginChild("VisualsRight", ImVec2(0, 0), true);
            {
                // Chams Settings
                ImGui::TextColored(accentPrimary, "CHAMS");
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 5));

                ImGui::Checkbox("Enable Chams", &Config::enemyChams);
                ImGui::SameLine(ImGui::GetWindowWidth() - 100);
                ImGui::Combo("Material##chams", &Config::chamsMaterial, "Flat\0Illuminate\0Glow\0");

                if (Config::enemyChams) {
                    ImGui::ColorEdit4("Color##chams", (float*)&Config::colVisualChams);
                    ImGui::Checkbox("XQZ Chams", &Config::enemyChamsInvisible);
                    if (Config::enemyChamsInvisible) {
                        ImGui::ColorEdit4("XQZ Color##chams", (float*)&Config::colVisualChamsIgnoreZ);
                    }

                    // Added team chams controls
                    ImGui::Checkbox("Team Chams", &Config::teamChams);
                    if (Config::teamChams) {
                        ImGui::ColorEdit4("Team Color##chams", (float*)&Config::teamcolVisualChams);
                        ImGui::Checkbox("Team XQZ", &Config::teamChamsInvisible);
                        if (Config::teamChamsInvisible) {
                            ImGui::ColorEdit4("Team XQZ Color##chams", (float*)&Config::teamcolVisualChamsIgnoreZ);
                        }
                    }
                }

                // Hand Chams
                ImGui::Dummy(ImVec2(0, 10));
                ImGui::TextColored(accentPrimary, "LOCAL CHAMS");
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 5));

                ImGui::Checkbox("Hand Chams", &Config::armChams);
                if (Config::armChams) {
                    ImGui::ColorEdit4("Color##hand", (float*)&Config::colArmChams);
                }

                ImGui::Checkbox("Viewmodel Chams", &Config::viewmodelChams);
                if (Config::viewmodelChams) {
                    ImGui::ColorEdit4("Color##viewmodel", (float*)&Config::colViewmodelChams);
                }

                // Removals
                ImGui::Dummy(ImVec2(0, 10));
                ImGui::TextColored(accentPrimary, "VISUAL REMOVALS");
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 5));

                ImGui::Checkbox("Anti Flash", &Config::antiflash);

                // Added missing glow control
                ImGui::Checkbox("Glow Effect", &Config::glow);
            }
            ImGui::EndChild();
        }
        break;

case 2:  // Misc Tab
{
    ImGui::BeginChild("MiscLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 5, 0), true);
    {
        ImGui::TextColored(accentPrimary, "MOVEMENT");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 5));

        ImGui::Checkbox("Auto Bunny Hop", &Config::bunnyHop);
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Automatically bunny hop when holding jump key");
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("MiscRight", ImVec2(0, 0), true);
    {
        ImGui::TextColored(accentPrimary, "OTHER FEATURES");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 20));

        ImGui::TextDisabled("No additional settings available");
        ImGui::SameLine(ImGui::GetWindowWidth() - 30);
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "¯\\_(ツ)_/¯");
    }
    ImGui::EndChild();
}
break;

        case 3:  // Config Tab
        {
            static char configName[128] = "";
            static std::vector<std::string> configList = internal_config::ConfigManager::ListConfigs();
            static int selectedConfigIndex = -1;

            ImGui::BeginChild("ConfigLeft", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 8, 0), true);
            {
                ImGui::TextColored(accentPrimary, "CONFIG MANAGEMENT");
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 10));

                // Config name input with placeholder
                ImGui::InputTextWithHint("##configname", "Enter config name...", configName, IM_ARRAYSIZE(configName));

                // Action buttons in grid layout
                ImGui::Dummy(ImVec2(0, 5));
                ImGui::BeginGroup();
                {
                    const float buttonWidth = (ImGui::GetContentRegionAvail().x - (3 * ImGui::GetStyle().ItemSpacing.x)) / 4;

                    if (ImGui::Button("Refresh", ImVec2(buttonWidth, 0))) {
                        configList = internal_config::ConfigManager::ListConfigs();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Load", ImVec2(buttonWidth, 0))) {
                        internal_config::ConfigManager::Load(configName);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Save", ImVec2(buttonWidth, 0))) {
                        internal_config::ConfigManager::Save(configName);
                        configList = internal_config::ConfigManager::ListConfigs();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Delete", ImVec2(buttonWidth, 0))) {
                        internal_config::ConfigManager::Remove(configName);
                        configList = internal_config::ConfigManager::ListConfigs();
                    }
                }
                ImGui::EndGroup();

                // Status message area
                ImGui::Dummy(ImVec2(0, 10));
                ImGui::TextDisabled(configList.empty() ? "No configs available" : "Select a config from the list");
            }
            ImGui::EndChild();

            ImGui::SameLine();
            ImGui::BeginChild("ConfigRight", ImVec2(0, 0), true);
            {
                ImGui::TextColored(accentPrimary, "SAVED CONFIGURATIONS");
                ImGui::Separator();
                ImGui::Dummy(ImVec2(0, 10));

                // Config list with hover/selection effects
                ImGuiListClipper clipper;
                clipper.Begin(static_cast<int>(configList.size()));

                while (clipper.Step()) {
                    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                        const bool isSelected = (selectedConfigIndex == i);
                        ImVec2 pos = ImGui::GetCursorScreenPos();

                        // Highlight selection
                        if (isSelected) {
                            ImGui::PushStyleColor(ImGuiCol_Header, accentPrimary);
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        }

                        if (ImGui::Selectable(configList[i].c_str(), isSelected)) {
                            selectedConfigIndex = i;
                            strncpy_s(configName, sizeof(configName), configList[i].c_str(), _TRUNCATE);
                        }

                        // Hover effect
                        if (ImGui::IsItemHovered() && !isSelected) {
                            ImGui::GetWindowDrawList()->AddRectFilled(
                                pos,
                                ImVec2(pos.x + ImGui::GetWindowWidth(), pos.y + ImGui::GetTextLineHeight()),
                                ImColor(0.15f, 0.15f, 0.15f, 0.3f)
                            );
                        }

                        if (isSelected) {
                            ImGui::PopStyleColor(2);
                        }
                    }
                }
                clipper.End();
            }
            ImGui::EndChild();
        }
        break;
        }

        ImGui::PopStyleVar();
        ImGui::EndChild();  // End content region
        ImGui::End();  // End window
    }
}



void Menu::toggleMenu() {
    showMenu = !showMenu;
}