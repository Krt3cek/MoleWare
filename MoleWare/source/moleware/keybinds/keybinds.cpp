#include "keybinds.h"
#include <Windows.h>
#include <cstring>   // For strcpy_s and sprintf_s
#include <cstdio>    // For sprintf_s

#include "../../../external/imgui/imgui.h"
#include "../config/config.h"

Keybind::Keybind(bool& v, int k, KeybindMode m)
    : var(v), key(k), mode(m), isListening(false), skipFrame(false) {}

Keybinds::Keybinds() {
    keybinds.emplace_back(Keybind(Config::aimbot, VK_XBUTTON1, TOGGLE));
    keybinds.emplace_back(Keybind(Config::triggerBot, VK_XBUTTON1, TOGGLE));
}

void Keybinds::pollInputs() {
    for (Keybind& k : keybinds) {
        if (k.key == 0) continue;

        const bool currentState = GetAsyncKeyState(k.key) & 0x8000;
        bool previousState = keyStates[k.key];
        keyStates[k.key] = currentState;

        switch (k.mode) {
        case TOGGLE:
            if (currentState && !previousState) {
                k.var = !k.var;
            }
            break;

        case HOLD:
            k.var = currentState;
            break;
        }
    }
}

void Keybinds::menuButton(bool& var) {
    for (auto& kb : keybinds) {
        if (&kb.var != &var) continue;

        char keyName[32] = "None";
        if (kb.key != 0) {
            switch (kb.key) {
            case VK_INSERT: strcpy_s(keyName, "INSERT"); break;
            case VK_DELETE: strcpy_s(keyName, "DELETE"); break;
            case VK_HOME: strcpy_s(keyName, "HOME"); break;
            case VK_END: strcpy_s(keyName, "END"); break;
            case VK_PRIOR: strcpy_s(keyName, "PAGE UP"); break;
            case VK_NEXT: strcpy_s(keyName, "PAGE DOWN"); break;
            case VK_LBUTTON: strcpy_s(keyName, "MOUSE1"); break;
            case VK_RBUTTON: strcpy_s(keyName, "MOUSE2"); break;
            case VK_MBUTTON: strcpy_s(keyName, "MOUSE3"); break;
            case VK_XBUTTON1: strcpy_s(keyName, "MOUSE4"); break;
            case VK_XBUTTON2: strcpy_s(keyName, "MOUSE5"); break;
            default:
                if (kb.key >= 'A' && kb.key <= 'Z') {
                    sprintf_s(keyName, "%c", kb.key);
                }
                else if (kb.key >= '0' && kb.key <= '9') {
                    sprintf_s(keyName, "%c", kb.key);
                }
                else {
                    sprintf_s(keyName, "0x%X", kb.key);
                }
                break;
            }
        }

        if (!kb.isListening) {
            ImGui::PushID(&kb);

            // Use columns for better layout control
            ImGui::Columns(3, "##keybind_columns", false);

            // First column: Key name
            ImGui::SetColumnWidth(0, 80);  // Fixed width for key name
            ImGui::Text("[%s]", keyName);
            ImGui::NextColumn();

            // Second column: Change button
            ImGui::SetColumnWidth(1, 80);  // Fixed width for button
            bool clicked = ImGui::Button("Change##Bind");
            ImGui::NextColumn();

            // Third column: Mode selector
            ImGui::SetColumnWidth(2, 120);  // Fixed width for combo
            const char* modes[] = { "Toggle", "Hold" };
            if (ImGui::Combo("##Mode", (int*)&kb.mode, modes, IM_ARRAYSIZE(modes))) {
                if (kb.mode == HOLD) kb.var = false;
            }

            ImGui::Columns(1);
            ImGui::PopID();

            if (clicked) {
                kb.isListening = true;
                kb.skipFrame = true;
            }
        }
        else {
            ImGui::Text("Press any key...");
            ImGui::SameLine();

            if (ImGui::Button("Cancel") || (GetAsyncKeyState(VK_ESCAPE) & 0x8000)) {
                kb.isListening = false;
                return;
            }

            if (!kb.skipFrame) {
                // Improved mouse button detection
                constexpr int mouseButtons[] = {
                    VK_LBUTTON, VK_RBUTTON, VK_MBUTTON,
                    VK_XBUTTON1, VK_XBUTTON2
                };

                // Check mouse buttons first
                for (int btn : mouseButtons) {
                    if (GetAsyncKeyState(btn) & 0x8000) {
                        kb.key = btn;
                        kb.isListening = false;
                        return;
                    }
                }

                // Then check keyboard keys
                for (int keyCode = 8; keyCode < 256; ++keyCode) { // Start from backspace
                    // Skip mouse buttons we already checked
                    if (std::find(std::begin(mouseButtons), std::end(mouseButtons), keyCode) != std::end(mouseButtons))
                        continue;

                    if (GetAsyncKeyState(keyCode) & 0x8000) {
                        kb.key = keyCode;
                        kb.isListening = false;
                        return;
                    }
                }
            }
            else {
                kb.skipFrame = false;
            }
        }
    }
}


Keybinds keybind;
