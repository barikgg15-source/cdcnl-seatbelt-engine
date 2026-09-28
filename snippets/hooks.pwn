/*
 * Potongan hook yang HARUS ada di gamemode.
 * Di iansyah_FIXED.pwn sudah terpasang. File ini hanya referensi
 * kalau kamu merakit ulang / pindah ke GM lain.
 */

// ------------------------------------------------------------
// OnPlayerConnect  (iansyah_FIXED.pwn ~41444)
// ------------------------------------------------------------
public OnPlayerConnect(playerid)
{
    Seatbelt[playerid] = false;
    SeatbeltVehicle[playerid] = 0;
    SeatbeltSeat[playerid] = 0;
    SeatbeltWarningTimer[playerid] = 0;
    LastVehicleSpeed[playerid] = 0.0;
    // ... sisa connect GM
    return 1;
}

// ------------------------------------------------------------
// OnPlayerDisconnect (~43146)
// ------------------------------------------------------------
public OnPlayerDisconnect(playerid, reason)
{
    ResetPlayerSeatbelt(playerid);
    return 1;
}

// ------------------------------------------------------------
// OnPlayerDeath (~43776)
// ------------------------------------------------------------
public OnPlayerDeath(playerid, killerid, reason)
{
    ResetPlayerSeatbelt(playerid);
    return 1;
}

// ------------------------------------------------------------
// RemovePlayerFromVehicleEx (~9747)
// Wajib reset dulu, kalau tidak OnPlayerStateChange akan
// memasukkan player kembali ke kendaraan.
// ------------------------------------------------------------
stock RemovePlayerFromVehicleEx(playerid)
{
    if(GetPlayerState(playerid) != PLAYER_STATE_DRIVER && GetPlayerState(playerid) != PLAYER_STATE_PASSENGER) return 0;
    ResetPlayerSeatbelt(playerid);
    // ... sisa logic GM
    return 1;
}

// ------------------------------------------------------------
// OnPlayerExitVehicle (~44305)
// ------------------------------------------------------------
public OnPlayerExitVehicle(playerid, vehicleid)
{
    if(Seatbelt[playerid])
    {
        new seat = SeatbeltSeat[playerid];
        if(seat < 0) seat = 0;
        PutPlayerInVehicle(playerid, vehicleid, seat);
        SetVehicleParamsForPlayer(vehicleid, playerid, 0, 1);
        ShowError(playerid, "Anda masih memakai sabuk pengaman");
        SetTimerEx("ForceSeatbeltBack", 150, false, "iii", playerid, vehicleid, seat);
        return 0;
    }

    StopSeatbeltWarning(playerid);
    ResetPlayerSeatbelt(playerid);
    RefreshVehicleSeatbeltWarning(vehicleid);
    return 1;
}

// ------------------------------------------------------------
// OnPlayerStateChange  (blok keluar kendaraan ~44421)
// Double-guard: F + exit native kadang lolos OnPlayerExitVehicle
// ------------------------------------------------------------
public OnPlayerStateChange(playerid, newstate, oldstate)
{
    if(newstate == PLAYER_STATE_ONFOOT)
    {
        if(Seatbelt[playerid] && (oldstate == PLAYER_STATE_DRIVER || oldstate == PLAYER_STATE_PASSENGER))
        {
            new vehicleid = SeatbeltVehicle[playerid];
            new seat = SeatbeltSeat[playerid];
            if(seat < 0) seat = 0;

            if(vehicleid > 0 && GetVehicleModel(vehicleid) > 0)
            {
                PutPlayerInVehicle(playerid, vehicleid, seat);
                SetVehicleParamsForPlayer(vehicleid, playerid, 0, 1);
                ShowError(playerid, "Anda masih memakai sabuk pengaman");
                SetTimerEx("ForceSeatbeltBack", 150, false, "iii", playerid, vehicleid, seat);
                return 1;
            }
            else
            {
                ResetPlayerSeatbelt(playerid);
            }
        }
        else
        {
            new old_vid = 0;
            if(oldstate == PLAYER_STATE_DRIVER || oldstate == PLAYER_STATE_PASSENGER)
                old_vid = SeatbeltVehicle[playerid];

            ResetPlayerSeatbelt(playerid);
            StopSeatbeltWarning(playerid);

            if(old_vid > 0 && GetVehicleModel(old_vid) > 0)
                RefreshVehicleSeatbeltWarning(old_vid);
        }
    }
    return 1;
}

// ------------------------------------------------------------
// OnPlayerUpdate — deteksi rem mendadak / kecelakaan (~48411)
// Hanya driver. Damage & terpental jika TIDAK pakai sabuk.
// ------------------------------------------------------------
public OnPlayerUpdate(playerid)
{
    if(IsPlayerInAnyVehicle(playerid) && GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
    {
        new vehicleid = GetPlayerVehicleID(playerid);
        new Float: speed = GetVehicleSpeed(vehicleid);
        new Float: selisih = LastVehicleSpeed[playerid] - speed;

        if(selisih > 30.0 && !Seatbelt[playerid])
        {
            new Float: damage = selisih * 0.75;
            new Float: health;
            GetPlayerHealth(playerid, health);
            SetPlayerHealth(playerid, health - damage);

            if(selisih >= 100.0)
                ShowError(playerid, "Kecelakaan sangat parah! Kamu terluka parah karena tidak memakai sabuk.");
            else if(selisih >= 60.0)
                ShowWarning(playerid, "Kecelakaan berat! Kamu mengalami luka serius.");
            else if(selisih >= 35.0)
                ShowInfo(playerid, "Kamu terluka karena tidak memakai sabuk pengaman.");

            if(LastVehicleSpeed[playerid] > 55.0 && selisih > 40.0 && !Seatbelt[playerid])
            {
                new Float: vx, Float: vy, Float: vz;
                GetVehicleVelocity(vehicleid, vx, vy, vz);

                RemovePlayerFromVehicle(playerid);

                new Float: x, Float: y, Float: z;
                GetPlayerPos(playerid, x, y, z);
                SetPlayerPos(playerid, x, y, z + 1.2);
                SetPlayerVelocity(playerid, vx * 1.6, vy * 1.6, 0.9);
                ApplyAnimation(playerid, "PED", "KO_shot_stomp", 4.0, 2, 1, 1, 1, 0, 1);

                ShowError(playerid, "Kamu terpental keluar dari kendaraan karena tidak memakai sabuk pengaman!");
            }
        }

        LastVehicleSpeed[playerid] = speed;
    }
    return 1;
}

// ------------------------------------------------------------
// Panel kendaraan: tombol setbeltv memanggil /sabuk (~66396)
// mesinv memanggil /engine (~66384)
// ------------------------------------------------------------
public OnPlayerClickTextDraw(playerid, Text:clickedid)
{
    if(clickedid == mesinv)  callcmd::engine(playerid);
    if(clickedid == setbeltv) callcmd::sabuk(playerid, "");
    return 1;
}
