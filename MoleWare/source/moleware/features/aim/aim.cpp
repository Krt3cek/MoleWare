#include "../../../cs2/entity/C_CSPlayerPawn/C_CSPlayerPawn.h"
#include "../../../moleware/interfaces/CGameEntitySystem/CGameEntitySystem.h"
#include "../../../moleware/interfaces/interfaces.h"
#include "../../../moleware/hooks/hooks.h"
#include "../../../moleware/config/config.h"

#include <chrono>
#include <thread>
#include <Windows.h>

// Literally the most autistic code ive ever written in my life
// Please dont ever make me do this again

Vector_t GetEntityEyePos(const C_CSPlayerPawn* Entity) {
    if (!Entity)
        return {};

    uintptr_t game_scene_node = *reinterpret_cast<uintptr_t*>((uintptr_t)Entity + SchemaFinder::Get(hash_32_fnv1a_const("C_BaseEntity->m_pGameSceneNode")));

    auto Origin = *reinterpret_cast<Vector_t*>(game_scene_node + SchemaFinder::Get(hash_32_fnv1a_const("CGameSceneNode->m_vecAbsOrigin")));
    auto ViewOffset = *reinterpret_cast<Vector_t*>((uintptr_t)Entity + SchemaFinder::Get(hash_32_fnv1a_const("C_BaseModelEntity->m_vecViewOffset")));

    Vector_t Result = Origin + ViewOffset;
    if (!std::isfinite(Result.x) || !std::isfinite(Result.y) || !std::isfinite(Result.z))
        return {};

    return Result;
}

inline QAngle_t CalcAngles(Vector_t viewPos, Vector_t aimPos)
{
    QAngle_t angle = { 0, 0, 0 };

    Vector_t delta = aimPos - viewPos;

    angle.x = -asin(delta.z / delta.Length()) * (180.0f / 3.141592654f);
    angle.y = atan2(delta.y, delta.x) * (180.0f / 3.141592654f);

    return angle;
}

inline float GetFov(const QAngle_t& viewAngle, const QAngle_t& aimAngle)
{
    QAngle_t delta = (aimAngle - viewAngle).Normalize();

    return sqrtf(powf(delta.x, 2.0f) + powf(delta.y, 2.0f));
}

void Triggerbot() {
    if (!Config::triggerBot)
        return;

    C_CSPlayerPawn* localPlayer = H::oGetLocalPlayer(0);
    if (!localPlayer || localPlayer->m_iHealth() <= 0)
        return;

    // Get eye position and view angles
    Vector_t localEyePos = GetEntityEyePos(localPlayer);
    QAngle_t* viewAngles = (QAngle_t*)(modules.getModule("client") + 0x1A933C0);

    // Best target tracking
    float bestFov = Config::aimbot_fov;
    C_CSPlayerPawn* bestTarget = nullptr;

    // Entity loop
    for (int i = 1; i <= I::GameEntity->Instance->GetHighestEntityIndex(); i++) {
        auto entity = I::GameEntity->Instance->Get(i);
        if (!entity || !entity->handle().valid())
            continue;

        SchemaClassInfoData_t* classInfo;
        entity->dump_class_info(&classInfo);
        if (!classInfo || HASH(classInfo->szName) != HASH("C_CSPlayerPawn"))
            continue;

        C_CSPlayerPawn* pawn = reinterpret_cast<C_CSPlayerPawn*>(entity);

        // Validity checks
        if (pawn->getTeam() == localPlayer->getTeam() ||
            pawn->getHealth() <= 0 ||
            pawn->get_entity_by_handle() == localPlayer->get_entity_by_handle()) {
            continue;
        }

        // Calculate angle to head
        Vector_t targetPos = GetEntityEyePos(pawn);
        QAngle_t targetAngle = CalcAngles(localEyePos, targetPos);
        targetAngle.x *= -1.f;
        targetAngle.y += 180.f;

        // Calculate FOV difference
        float currentFov = GetFov(*viewAngles, targetAngle);

        if (currentFov < bestFov) {
            bestFov = currentFov;
            bestTarget = pawn;
        }
    }

    // Shooting logic
    static auto lastShot = std::chrono::steady_clock::now();
    if (bestTarget) {
        auto now = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastShot).count() > Config::triggerBotDelay) {
            printf("strili");
            static uintptr_t dwForceAttack = modules.getModule("client") + 0x186C850;
            *reinterpret_cast<int*>(dwForceAttack) = 65537; // Attack start
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            *reinterpret_cast<int*>(dwForceAttack) = 256; // Attack release

            lastShot = now;
        }
    }
}

void Aimbot() {
    if (!Config::aimbot)
        return;

    int nMaxHighestEntity = I::GameEntity->Instance->GetHighestEntityIndex();
    C_CSPlayerPawn* lp = H::oGetLocalPlayer(0);

    if (!lp) return;

    Vector_t lep = GetEntityEyePos(lp);
    QAngle_t* viewangles = (QAngle_t*)(modules.getModule("client") + 0x1A933C0);

    for (int i = 1; i <= nMaxHighestEntity; i++) {
        auto Entity = I::GameEntity->Instance->Get(i);
        if (!Entity || !Entity->handle().valid())
            continue;

        SchemaClassInfoData_t* _class = nullptr;
        Entity->dump_class_info(&_class);
        if (!_class || HASH(_class->szName) != HASH("C_CSPlayerPawn"))
            continue;

        C_CSPlayerPawn* pawn = (C_CSPlayerPawn*)Entity;

        // Team check
        if (Config::aimbotTeamCheck && pawn->getTeam() == lp->getTeam())
            continue;

        if (pawn->get_entity_by_handle() == lp->get_entity_by_handle() ||
            pawn->getHealth() < 1)
            continue;

        Vector_t eye_pos = GetEntityEyePos(pawn);
        QAngle_t angle = CalcAngles(eye_pos, lep);
        angle.x *= -1.f;
        angle.y += 180.f;

        const float fov = GetFov(*viewangles, angle);
        if (!std::isfinite(fov) || fov > Config::aimbot_fov)
            continue;

        // Recoil control
        QAngle_t ang_punch_angle = *(QAngle_t*)((uintptr_t)lp +
            SchemaFinder::Get(hash_32_fnv1a_const("C_CSPlayerPawn->m_aimPunchAngle")));

        if (Config::rcs)
            angle -= ang_punch_angle * 2.f;

        // Smooth aiming
        QAngle_t current_angle = *viewangles;
        QAngle_t delta = (angle - current_angle).Normalize();

        // Apply smoothing
        if (Config::aimbotSmoothness > 1.0f) {
            delta.x /= Config::aimbotSmoothness;
            delta.y /= Config::aimbotSmoothness;
        }

        angle = current_angle + delta;
        angle.z = 0.f;
        angle = angle.Normalize();

        *viewangles = angle;
        break;
    }
}