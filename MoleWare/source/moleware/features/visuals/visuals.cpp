#include "visuals.h"
#include <algorithm>
#include <iostream>
#include "../../hooks/hooks.h"
#include "../../players/players.h"
#include "../../utils/memory/patternscan/patternscan.h"
#include "../../utils/memory/gaa/gaa.h"
#include "../../../../external/imgui/imgui.h"
#include "../../interfaces/interfaces.h"
#include "../../config/config.h"
#include "../../menu/menu.h"
using namespace Esp;

LocalPlayerCached cached_local;
std::vector<PlayerCache> cached_players;

void Visuals::init() {
    viewMatrix.viewMatrix = (viewmatrix_t*)M::getAbsoluteAddress(M::patternScan("client", "48 8D 0D ? ? ? ? 48 C1 E0 06"), 3, 0);
}

void Esp::cache()
{
    if (!I::EngineClient->valid())
        return;

    /*  Old method READ ME
    * @ // @here: manually cache once all existing entitys
        // to avoid issues when injecting mid game and hkAddEnt not called by game on existing Entity's

       int highest_index = I::GameEntity->Instance->GetHighestEntityIndex();
        for (int i = 1; i <= highest_index; i++) {
            auto entity = I::GameEntity->Instance->Get(i);
            if (!entity)
                continue;

            uintptr_t entityPointer = reinterpret_cast<uintptr_t>(entity);

            SchemaClassInfoData_t* entityInfo = nullptr;
            GetSchemaClassInfo(entityPointer, &entityInfo);
            if (!entityInfo) continue;

            if (strcmp(entityInfo->szName, "C_CSPlayerPawn") == 0) {
                bool exists = std::any_of(Players::pawns.begin(), Players::pawns.end(),
                    [&](const C_CSPlayerPawn& pawn) { return pawn.getAddress() == entityPointer; });
                if (!exists) {
                    Players::pawns.emplace_back(entityPointer);
                    std::cout << "Added pawn " << Players::pawns.size() << "\n";
                }
                continue;
            }

            if (strcmp(entityInfo->szName, "CCSPlayerController") == 0) {
                bool exists = std::any_of(Players::controllers.begin(), Players::controllers.end(),
                    [&](const CCSPlayerController& controller) { return controller.getAddress() == entityPointer; });
                if (!exists) {
                    Players::controllers.emplace_back(entityPointer);
                }
                continue;
            }
    }*/

    cached_players.clear();

    int nMaxHighestEntity = I::GameEntity->Instance->GetHighestEntityIndex();

    for (int i = 1; i <= nMaxHighestEntity; i++)
    {
        auto Entity = I::GameEntity->Instance->Get(i);
        if (!Entity)
            continue;

        if (!Entity->handle().valid())
            continue;

        SchemaClassInfoData_t* _class = nullptr;
        Entity->dump_class_info(&_class);
        if (!_class)
            continue;

        const uint32_t hash = HASH(_class->szName);

        PlayerType_t type = none;

        if (hash == HASH("CCSPlayerController"))
        {

            type = none; int health = 0; int armor = 0;
            Vector_t position; Vector_t viewOffset;
            const char* name = "none"; const char* weapon_name = "none";

            CCSPlayerController* Controller = reinterpret_cast<CCSPlayerController*>(Entity);
            if (!Controller)
                continue;

            if (!Controller->m_hPawn().valid())
                continue;

            //@handle caching local player 
            if (Controller->IsLocalPlayer()) {
                auto LocalPlayer = I::GameEntity->Instance->Get<C_CSPlayerPawn>(Controller->m_hPawn().index());
                if (!LocalPlayer) {
                    cached_local.reset();
                    continue;
                }

                cached_local.alive = LocalPlayer->m_iHealth() > 0;
                if (LocalPlayer->m_iHealth() > 0) {
                    cached_local.poisition = LocalPlayer->m_vOldOrigin();
                    cached_local.health = LocalPlayer->m_iHealth();
                    cached_local.handle = LocalPlayer->handle().index();
                    cached_local.team = LocalPlayer->m_iTeamNum();
                }
                else {
                    cached_local.reset();
                }
            }
            else // @handle only players
            {
                auto Player = I::GameEntity->Instance->Get<C_CSPlayerPawn>(Controller->m_hPawn().index());
                if (!Player)
                    continue;

                if (Player->m_iHealth() <= 0 && !Config::showDead)
                    continue;

                health = Player->m_iHealth();
                name = Controller->m_sSanitizedPlayerName();
                position = Player->m_vOldOrigin(); viewOffset = Player->m_vecViewOffset();

                // Add to cache
                cached_players.emplace_back(Entity, Player, Player->handle(),
                    type, health, name,
                    weapon_name, position, viewOffset, Player->m_iTeamNum());
            }
        }
    }
}

void Visuals::esp() {
    if (!Config::esp && !Config::showHealth && !Config::showArmor && !Config::espFill && !Config::showNameTags && !Config::showHead && !Config::distance) {
        return;
    }

    C_CSPlayerPawn* localPawn = H::oGetLocalPlayer(0);
    if (!localPawn) {
        return;
    }

    if (cached_players.empty())
        return;

    for (const auto& Player : cached_players) {
        if (!Player.handle.valid() || Player.handle.index() == INVALID_EHANDLE_INDEX)
            continue;

        if (Config::teamCheck && (Player.team_num == cached_local.team))
            continue;

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        Vector_t feetPos = Player.position;
        Vector_t headPos = Player.position + Player.viewOffset;

        Vector_t feetScreen, headScreen;
        if (!viewMatrix.WorldToScreen(feetPos, feetScreen) ||
            !viewMatrix.WorldToScreen(headPos, headScreen))
            continue;

        float boxHeight = (feetScreen.y - headScreen.y) * 1.3f;
        float boxWidth = boxHeight / 2.0f;

        float centerX = (feetScreen.x + headScreen.x) / 2.0f;
        float boxX = centerX - (boxWidth / 2.0f);
        float boxY = headScreen.y - (boxHeight - (feetScreen.y - headScreen.y)) / 2.0f;

        ImVec4 espColorWithAlpha = Config::espColor;
        espColorWithAlpha.w = Config::espFillOpacity;
        ImU32 boxColor = ImGui::ColorConvertFloat4ToU32(Config::espColor);
        ImU32 fillColor = ImGui::ColorConvertFloat4ToU32(espColorWithAlpha);

        if (Config::espFill) {
            drawList->AddRectFilled(
                ImVec2(boxX, boxY),
                ImVec2(boxX + boxWidth, boxY + boxHeight),
                fillColor
            );
        }

        if (Config::esp) {
            drawList->AddRect(
                ImVec2(boxX, boxY),
                ImVec2(boxX + boxWidth, boxY + boxHeight),
                boxColor,
                0.0f,
                0,
                Config::espThickness
            );
        }

        ImVec2 textSize(0.0f, 0.0f);
        float textY = boxY + boxHeight + 2;

        if (Config::showHealth || Config::showArmor) {
            const float barSpacing = 2.0f;
            const float totalBarWidth = (2 * 2) + barSpacing;
            float currentBarX = boxX - totalBarWidth - 4.0f; // 4px offset from box

            // Health Bar
            if (Config::showHealth) {
                float healthHeight = boxHeight * (Player.health / 100.0f);

                // Background
                drawList->AddRectFilled(
                    ImVec2(currentBarX, boxY),
                    ImVec2(currentBarX + 2, boxY + boxHeight),
                    IM_COL32(50, 50, 50, 150)
                );

                // Foreground - Solid green
                drawList->AddRectFilled(
                    ImVec2(currentBarX, boxY + (boxHeight - healthHeight)),
                    ImVec2(currentBarX + 2, boxY + boxHeight),
                    IM_COL32(0, 255, 0, 255) // Solid green color
                );

                currentBarX += 2 + barSpacing;
            }

            // Armor Bar
            if (Config::showArmor) {
                float armorHeight = boxHeight * (Player.health / 100.0f);

                // Background
                drawList->AddRectFilled(
                    ImVec2(currentBarX, boxY),
                    ImVec2(currentBarX + 2, boxY + boxHeight),
                    IM_COL32(50, 50, 50, 150)
                );

                // Foreground - Solid blue
                drawList->AddRectFilled(
                    ImVec2(currentBarX, boxY + (boxHeight - armorHeight)),
                    ImVec2(currentBarX + 2, boxY + boxHeight),
                    IM_COL32(0, 0, 255, 255) // Solid blue color
                );
            }
        }

        if (Config::showNameTags) {
            std::string playerName = Player.name;
            ImVec2 nameSize = ImGui::CalcTextSize(playerName.c_str());
            float nameX = boxX + (boxWidth - nameSize.x) / 2;
            float nameY = boxY - nameSize.y - 2;

            drawList->AddText(
                ImVec2(nameX + 1, nameY + 1),
                IM_COL32(0, 0, 0, 255),
                playerName.c_str()
            );
            drawList->AddText(
                ImVec2(nameX, nameY),
                IM_COL32(255, 255, 255, 255),
                playerName.c_str()
            );
        }

        if (Config::showWeapon) {
            bool weapon = Player.weapon_name;
            std::string displayText = "[" + std::to_string(weapon) + "]";
            textSize = ImGui::CalcTextSize(displayText.c_str());
            float textX = boxX + (boxWidth - textSize.x) / 2;

            drawList->AddText(
                ImVec2(textX + 1, textY + 1),
                IM_COL32(0, 0, 0, 255),
                displayText.c_str()
            );
            drawList->AddText(
                ImVec2(textX, textY),
                IM_COL32(255, 255, 255, 255),
                displayText.c_str()
            );
        }

        if (Config::distance) {
            Vector_t localPos = cached_local.poisition;
            Vector_t enemyPos = Player.position;

            float dx = enemyPos.x - localPos.x;
            float dy = enemyPos.y - localPos.y;
            float dz = enemyPos.z - localPos.z;
            float distance = sqrtf(dx * dx + dy * dy + dz * dz);
            float distanceMeters = distance / 39.37f;

            std::string distanceText = std::to_string(static_cast<int>(distanceMeters)) + "m";
            ImVec2 distanceSize = ImGui::CalcTextSize(distanceText.c_str());
            float distanceX = boxX + (boxWidth - distanceSize.x) / 2;
            float distanceY;

            if (Config::showHealth) {
                distanceY = textY + textSize.y + 2;
            }
            else {
                distanceY = textY;
            }

            drawList->AddText(
                ImVec2(distanceX + 1, distanceY + 1),
                IM_COL32(0, 0, 0, 255),
                distanceText.c_str()
            );
            drawList->AddText(
                ImVec2(distanceX, distanceY),
                IM_COL32(255, 255, 255, 255),
                distanceText.c_str()
            );
        }

        if (Config::showHead) {
            ImU32 dotColor = ImGui::ColorConvertFloat4ToU32(Config::headDotColor);
            drawList->AddCircleFilled(
                ImVec2(headScreen.x, headScreen.y),
                Config::headDotRadius,
                dotColor
            );
        }
    }
}
