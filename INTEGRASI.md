# Integrasi ke Gamemode CDCNL

Sistem ini **sudah hidup** di `CDCNL/gamemodes/iansyah_FIXED.pwn`.
Repo ini hanya mengekstraknya supaya mudah dibaca, di-backup, dan dipasang ulang.

## Opsi A — sudah ada di FIXED (disarankan)

Tidak perlu include baru. Pakai `iansyah_FIXED.pwn` sebagai gamemode.

Compile:
```
pawno/pawncc.exe gamemodes/iansyah_FIXED.pwn
```
Lalu di `server.cfg`:
```
gamemode0 iansyah_FIXED 1
```

## Opsi B — pindahkan ke include

1. Copy `include/cdcnl_seatbelt_engine.inc` ke `pawno/include/`
2. Di gamemode, **hapus** duplikat:
   - `new bool: Seatbelt[...]`
   - `LastVehicleSpeed`, `SeatbeltWarningTimer`, `SeatbeltVehicle`, `SeatbeltSeat`
   - `forward IsVehicleSeatbeltSupported` / `ResetPlayerSeatbelt` / `SeatbeltAction` / `ForceSeatbeltBack`
   - stock `IsVehicleSeatbeltSupported`, `ResetPlayerSeatbelt`, `IsVehicleBus`, `VehicleHasUnbeltedOccupant`, `StopSeatbeltWarning`, `RefreshVehicleSeatbeltWarning`, `StartSeatbeltWarning`
   - `CMD:sabuk`, `CMD:engine`, `TurnOnEngine`, `SeatbeltReminder`, `ForceSeatbeltBack`
   - `stock CloseEngine` (kalau sudah ada di include)
3. Tambahkan **setelah** include CDCNL lain (foreach, cmd, dll):
   ```pawn
   #include <cdcnl_seatbelt_engine>
   ```
4. Sisakan hook di callback GM. Contoh ada di `snippets/hooks.pwn`.
   Jangan dobel-dobel `CMD:engine` / `CMD:sabuk`.

## Dependensi yang harus sudah ada

- `foreach`
- `ShowError` / `ShowWarning` / `ShowSucces` / `ShowInfo` / `ShowProgressbar`
- `ProxDetector`, `Name`, `SCM`
- `PlayerInfo[playerid][pProgressBar]`
- `Fuell[vehicleid]`, `IsVehicleHaveFuel`
- `VehInfo[vehicleid][vFr]`, `IsAGang`, `FRACTION_LSPD`
- `GetVehicleSpeed`
- `callcmd` (untuk tombol panel)
- variabel global `String`, `COLOR_PURPLE`, `COLOR_GREY`

## Tes cepat in-game

1. Naik mobil (bukan motor/sepeda), `/engine` → progress 2 detik, mesin nyala.
2. Jangan `/sabuk` → audio alarm tiap 7 detik, warning muncul.
3. Penumpang ikut dengar alarm (mobil biasa).
4. `/sabuk` → progress 2 detik, "TERPASANG", alarm berhenti kalau semua sudah sabuk.
5. Tekan F → pesan "Anda masih memakai sabuk pengaman", kembali ke kursi.
6. `/sabuk` lagi → lepas, baru bisa turun.
7. Naik motor → `/sabuk` ditolak.
8. Naik bus 431/437 → hanya sopir yang di-alarm.
9. Ngebut lalu rem mendadak tanpa sabuk → HP berkurang; kalau cukup parah terpental.
10. Klik icon `setbeltv` di panel = sama dengan `/sabuk`. Klik `mesinv` = `/engine`.

## Yang tidak ikut diekstrak

- Seluruh HBE / speedo / GPS / KTP
- Tuning `CMD:style` / `vEngineTune` (berbeda topik, cuma kebetulan dekat `CMD:engine`)
- Audio welcome `OnPlayerConnect` (`m_3841bblty1.mp3`) — itu intro server, bukan alarm sabuk
