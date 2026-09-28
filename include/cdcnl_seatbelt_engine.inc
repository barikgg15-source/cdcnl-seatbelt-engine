/*
 * CDCNL Seatbelt + Engine
 * Diekstrak dari: CDCNL/gamemodes/iansyah_FIXED.pwn (versi lokal yang lebih lengkap)
 *
 * Sumber asli (baris di iansyah_FIXED.pwn):
 *   variabel + forward     : 50-60
 *   setbeltv TD            : 338, 28922-28934
 *   RemovePlayerFromVehicleEx reset : 9747-9751
 *   stocks + CMD:sabuk     : 32263-32481
 *   CMD:engine + TurnOn    : 34840-34921
 *   SeatbeltReminder       : 34923-34968
 *   OnPlayerConnect reset  : 41444-41449
 *   OnPlayerDisconnect     : 43146
 *   OnPlayerDeath          : 43776
 *   OnPlayerExitVehicle    : 44305-44323
 *   OnPlayerStateChange    : 44421-44457
 *   OnPlayerUpdate crash   : 48411-48465
 *   panel click setbeltv   : 66396-66398
 *   CloseEngine            : 71531-71535
 *
 * Dependensi CDCNL yang sudah ada di GM:
 *   foreach, ShowError, ShowWarning, ShowSucces, ShowInfo, ShowProgressbar,
 *   ProxDetector, Name, SCM, COLOR_*, PlayerInfo[pProgressBar], Fuell[],
 *   VehInfo[][vFr], IsAGang, IsVehicleHaveFuel, FRACTION_LSPD, String,
 *   GetVehicleSpeed, IsABicycle, callcmd
 *
 * Cara pakai: #include <cdcnl_seatbelt_engine>
 * Jangan duplikat variabel / CMD / stock yang sama di gamemode.
 */

#if defined _CDCNL_SEATBELT_ENGINE_INCLUDED
    #endinput
#endif
#define _CDCNL_SEATBELT_ENGINE_INCLUDED

#define SEATBELT_SFX_CLICK      "http://b.top4top.io/m_38644325f1.mp3"
#define SEATBELT_SFX_REMINDER   "http://h.top4top.io/m_3863604rq1.mp3"
#define SEATBELT_REMINDER_MS    7000
#define SEATBELT_ACTION_MS      2000

new bool: Seatbelt[MAX_PLAYERS];
new Float: LastVehicleSpeed[MAX_PLAYERS];
new SeatbeltWarningTimer[MAX_PLAYERS];
new SeatbeltVehicle[MAX_PLAYERS];
new SeatbeltSeat[MAX_PLAYERS];

forward IsVehicleSeatbeltSupported(vehicleid);
forward ResetPlayerSeatbelt(playerid);
forward SeatbeltAction(playerid, action);
forward ForceSeatbeltBack(playerid, vehicleid, seat);
forward TurnOnEngine(playerid, vehicleid);
forward SeatbeltReminder(playerid);

stock IsVehicleSeatbeltSupported(vehicleid)
{
    new model = GetVehicleModel(vehicleid);
    switch(model)
    {
        case 481, 509, 510,
             461, 462, 463, 468, 521, 522, 581, 586:
            return 0;
    }
    return 1;
}

stock IsVehicleBus(vehicleid)
{
    new model = GetVehicleModel(vehicleid);
    switch(model)
    {
        case 431, 437: return 1;
    }
    return 0;
}

stock CloseEngine(carid)
{
    switch(GetVehicleModel(carid)) { case 481, 460, 509, 510: return true; }
    return false;
}

stock ResetPlayerSeatbelt(playerid)
{
    if(Seatbelt[playerid])
    {
        new vid = SeatbeltVehicle[playerid];
        if(vid > 0 && GetVehicleModel(vid) > 0)
            SetVehicleParamsForPlayer(vid, playerid, 0, 0);
    }

    Seatbelt[playerid] = false;
    SeatbeltVehicle[playerid] = 0;
    SeatbeltSeat[playerid] = 0;
    KillTimer(SeatbeltWarningTimer[playerid]);
    SeatbeltWarningTimer[playerid] = 0;
    StopAudioStreamForPlayer(playerid);
    return 1;
}

stock StopSeatbeltWarning(playerid)
{
    KillTimer(SeatbeltWarningTimer[playerid]);
    SeatbeltWarningTimer[playerid] = 0;
    StopAudioStreamForPlayer(playerid);
    return 1;
}

stock VehicleHasUnbeltedOccupant(vehicleid)
{
    if(!IsVehicleSeatbeltSupported(vehicleid)) return 0;

    new bool:is_bus = bool:IsVehicleBus(vehicleid);

    foreach(new i : Player)
    {
        if(!IsPlayerInVehicle(i, vehicleid)) continue;

        new stateid = GetPlayerState(i);
        if(stateid != PLAYER_STATE_DRIVER && stateid != PLAYER_STATE_PASSENGER) continue;

        if(is_bus && stateid != PLAYER_STATE_DRIVER) continue;

        if(!Seatbelt[i]) return 1;
    }
    return 0;
}

stock RefreshVehicleSeatbeltWarning(vehicleid)
{
    if(vehicleid <= 0 || GetVehicleModel(vehicleid) <= 0) return 0;
    if(!IsVehicleSeatbeltSupported(vehicleid)) return 0;

    new engine, lights, alarm, doors, bonnet, boot, objective;
    GetVehicleParamsEx(vehicleid, engine, lights, alarm, doors, bonnet, boot, objective);

    new bool:is_bus = bool:IsVehicleBus(vehicleid);
    new bool:need_warn = (engine == 1 && VehicleHasUnbeltedOccupant(vehicleid));

    foreach(new i : Player)
    {
        if(!IsPlayerInVehicle(i, vehicleid)) continue;

        new stateid = GetPlayerState(i);
        if(stateid != PLAYER_STATE_DRIVER && stateid != PLAYER_STATE_PASSENGER) continue;

        if(is_bus && stateid != PLAYER_STATE_DRIVER)
        {
            StopSeatbeltWarning(i);
            continue;
        }

        if(need_warn)
        {
            if(!Seatbelt[i])
                ShowWarning(i, "Kamu belum memasang sabuk pengaman!");
            else
                ShowWarning(i, "Ada orang di kendaraan ini yang belum memakai sabuk pengaman!");

            PlayAudioStreamForPlayer(i, SEATBELT_SFX_REMINDER);

            KillTimer(SeatbeltWarningTimer[i]);
            SeatbeltWarningTimer[i] = SetTimerEx("SeatbeltReminder", SEATBELT_REMINDER_MS, true, "i", i);
        }
        else
        {
            StopSeatbeltWarning(i);
        }
    }
    return 1;
}

stock StartSeatbeltWarning(playerid)
{
    if(!IsPlayerConnected(playerid)) return 0;
    if(!IsPlayerInAnyVehicle(playerid)) return 0;
    return RefreshVehicleSeatbeltWarning(GetPlayerVehicleID(playerid));
}

CMD:sabuk(playerid, params[])
{
    if(!IsPlayerInAnyVehicle(playerid))
        return ShowError(playerid, "Kamu harus berada di dalam kendaraan.");

    if(GetPlayerState(playerid) != PLAYER_STATE_DRIVER && GetPlayerState(playerid) != PLAYER_STATE_PASSENGER)
        return ShowError(playerid, "Kamu harus berada di kursi pengemudi atau penumpang.");

    if(PlayerInfo[playerid][pProgressBar] == 1)
        return ShowInfo(playerid, "Tunggu sebentar, masih ada progress.");

    new vehicleid = GetPlayerVehicleID(playerid);

    if(!IsVehicleSeatbeltSupported(vehicleid))
        return ShowError(playerid, "Kendaraan ini tidak memiliki sabuk pengaman.");

    if(Seatbelt[playerid])
    {
        ShowProgressbar(playerid, "Melepas sabuk pengaman...", 2);
        SetPlayerChatBubble(playerid, "* Melepas sabuk pengaman *", COLOR_PURPLE, 20.0, 3000);
        SetTimerEx("SeatbeltAction", SEATBELT_ACTION_MS, false, "ii", playerid, 0);
    }
    else
    {
        ShowProgressbar(playerid, "Memasang sabuk pengaman...", 2);
        PlayAudioStreamForPlayer(playerid, SEATBELT_SFX_CLICK);
        SetPlayerChatBubble(playerid, "* Memasang sabuk pengaman *", COLOR_PURPLE, 20.0, 3000);
        SetTimerEx("SeatbeltAction", SEATBELT_ACTION_MS, false, "ii", playerid, 1);
    }
    return 1;
}

public SeatbeltAction(playerid, action)
{
    if(!IsPlayerConnected(playerid)) return 1;
    if(!IsPlayerInAnyVehicle(playerid)) return 1;

    new vehicleid = GetPlayerVehicleID(playerid);

    if(action == 1)
    {
        Seatbelt[playerid] = true;
        SeatbeltVehicle[playerid] = vehicleid;
        SeatbeltSeat[playerid] = GetPlayerVehicleSeat(playerid);
        SetVehicleParamsForPlayer(vehicleid, playerid, 0, 1);
        ShowSucces(playerid, "Sabuk pengaman TERPASANG. Kamu tidak bisa keluar kendaraan.");
        RefreshVehicleSeatbeltWarning(vehicleid);
    }
    else
    {
        SetVehicleParamsForPlayer(vehicleid, playerid, 0, 0);
        Seatbelt[playerid] = false;
        SeatbeltVehicle[playerid] = 0;
        SeatbeltSeat[playerid] = 0;
        ShowWarning(playerid, "Sabuk pengaman DILEPAS. Sekarang kamu bisa keluar kendaraan.");
        RefreshVehicleSeatbeltWarning(vehicleid);
    }
    return 1;
}

public ForceSeatbeltBack(playerid, vehicleid, seat)
{
    if(!IsPlayerConnected(playerid)) return 1;
    if(!Seatbelt[playerid]) return 1;

    if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleID(playerid) != vehicleid)
    {
        if(vehicleid > 0 && GetVehicleModel(vehicleid) > 0)
        {
            if(seat < 0) seat = 0;
            PutPlayerInVehicle(playerid, vehicleid, seat);
            SetVehicleParamsForPlayer(vehicleid, playerid, 0, 1);
        }
    }
    return 1;
}

CMD:engine(playerid, params[])
{
    if(GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
        return SCM(playerid, COLOR_GREY, "Anda tidak berada di belakang kursi pengemudi.");

    new vehicleid = GetPlayerVehicleID(playerid);

    if(CloseEngine(vehicleid)) return 1;

    if(!IsVehicleHaveFuel(vehicleid))
    {
        new engine, lights, alarm, doors, bonnet, boot, objective;
        GetVehicleParamsEx(vehicleid, engine, lights, alarm, doors, bonnet, boot, objective);
        return SetVehicleParamsEx(vehicleid, !engine, lights, alarm, doors, bonnet, boot, objective);
    }

    if(Fuell[vehicleid] < 1.0)
        return ShowError(playerid, "Tidak ada bahan bakar dalam transportasi.");

    if(IsAGang(playerid) && VehInfo[vehicleid][vFr] == FRACTION_LSPD) return 1;

    new engine, lights, alarm, doors, bonnet, boot, objective;
    GetVehicleParamsEx(vehicleid, engine, lights, alarm, doors, bonnet, boot, objective);

    if(engine == 1)
    {
        SetPlayerChatBubble(playerid, "Mematikan mesin", COLOR_PURPLE, 30.0, 10000);
        SetVehicleParamsEx(vehicleid, 0, lights, alarm, doors, bonnet, boot, objective);
        RefreshVehicleSeatbeltWarning(vehicleid);
        return 1;
    }

    new Float:health;
    GetVehicleHealth(vehicleid, health);

    if(health < 350.0)
        return ShowError(playerid, "Kendaraan anda rusak. Segera repair atau service ke mekanik.");

    new delayTime;
    if(health <= 350.0)
    {
        delayTime = 4000;
        ShowProgressbar(playerid, "Menghidupkan Mesin (Kritis)", 4);
        ShowWarning(playerid, "Kendaraan dalam kondisi kritis. Segera servis kendaraan anda!");
    }
    else
    {
        delayTime = 2000;
        ShowProgressbar(playerid, "Menghidupkan Mesin", 2);
    }

    format(String, 188, "%s Berusaha Menghidupkan Kendaraan", Name(playerid));
    ProxDetector(20.0, playerid, String, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE);
    SetPlayerChatBubble(playerid, "Menyalakan mesin...", COLOR_PURPLE, 30.0, 10000);
    SetTimerEx("TurnOnEngine", delayTime, false, "ii", playerid, vehicleid);
    return 1;
}

public TurnOnEngine(playerid, vehicleid)
{
    if(!IsPlayerInVehicle(playerid, vehicleid)) return 1;

    new engine, lights, alarm, doors, bonnet, boot, objective;
    GetVehicleParamsEx(vehicleid, engine, lights, alarm, doors, bonnet, boot, objective);
    if(engine == 1) return 1;

    SetVehicleParamsEx(vehicleid, 1, lights, alarm, doors, bonnet, boot, objective);
    SetPlayerChatBubble(playerid, "Mesin menyala", COLOR_PURPLE, 30.0, 3000);
    RefreshVehicleSeatbeltWarning(vehicleid);
    return 1;
}

public SeatbeltReminder(playerid)
{
    if(!IsPlayerConnected(playerid))
    {
        KillTimer(SeatbeltWarningTimer[playerid]);
        SeatbeltWarningTimer[playerid] = 0;
        return 1;
    }

    new stateid = GetPlayerState(playerid);

    if(!IsPlayerInAnyVehicle(playerid) || (stateid != PLAYER_STATE_DRIVER && stateid != PLAYER_STATE_PASSENGER))
    {
        StopSeatbeltWarning(playerid);
        return 1;
    }

    new vehicleid = GetPlayerVehicleID(playerid);

    if(IsVehicleBus(vehicleid) && stateid != PLAYER_STATE_DRIVER)
    {
        StopSeatbeltWarning(playerid);
        return 1;
    }

    if(!IsVehicleSeatbeltSupported(vehicleid))
    {
        StopSeatbeltWarning(playerid);
        return 1;
    }

    new engine, lights, alarm, doors, bonnet, boot, objective;
    GetVehicleParamsEx(vehicleid, engine, lights, alarm, doors, bonnet, boot, objective);
    if(engine != 1 || !VehicleHasUnbeltedOccupant(vehicleid))
    {
        StopSeatbeltWarning(playerid);
        return 1;
    }

    PlayAudioStreamForPlayer(playerid, SEATBELT_SFX_REMINDER);
    return 1;
}
