# Peta kode ke gamemode lokal

Semua baris merujuk ke `CDCNL/gamemodes/iansyah_FIXED.pwn` di PC lokal.

File GM ini ~87.975 baris. Sistem seatbelt + engine **tidak** dalam satu blok berurutan — terpencar karena menempel ke callback yang sudah ada.

## Variabel & forward

```
50-55    new bool: Seatbelt[MAX_PLAYERS];
         new Float: LastVehicleSpeed[MAX_PLAYERS];
         new SeatbeltWarningTimer[MAX_PLAYERS];
         new SeatbeltVehicle[MAX_PLAYERS];
         new SeatbeltSeat[MAX_PLAYERS];
57-60    forward IsVehicleSeatbeltSupported / ResetPlayerSeatbelt
         / SeatbeltAction / ForceSeatbeltBack
338      new Text: setbeltv;
```

`TurnOnEngine` dan `SeatbeltReminder` di-forward dekat implementasinya (~34905, ~34923), bukan di atas file.

## Panel textdraw

```
28922-28934   TextDrawCreate setbeltv (ld_beat:chit, selectable)
66384-66398   OnPlayerClickTextDraw
                mesinv  -> callcmd::engine
                setbeltv -> callcmd::sabuk
66404         hide setbeltv saat panel ditutup
87249         show setbeltv saat panel dibuka
```

## Inti stock + command

```
32263-32274   IsVehicleSeatbeltSupported
32276-32293   ResetPlayerSeatbelt
32296-32304   IsVehicleBus (431, 437)
32309-32328   VehicleHasUnbeltedOccupant
32330-32336   StopSeatbeltWarning
32341-32384   RefreshVehicleSeatbeltWarning
32386-32393   StartSeatbeltWarning
32395-32427   CMD:sabuk
32429-32465   SeatbeltAction
32467-32482   ForceSeatbeltBack
34840-34904   CMD:engine
34905-34922   TurnOnEngine
34923-34969   SeatbeltReminder
71531-71535   CloseEngine (481, 460, 509, 510)
```

## Hook wajib

```
9747-9751     RemovePlayerFromVehicleEx  -> ResetPlayerSeatbelt dulu
41444-41449   OnPlayerConnect            -> semua state = 0 / false
43146         OnPlayerDisconnect         -> ResetPlayerSeatbelt
43776         OnPlayerDeath              -> ResetPlayerSeatbelt
44305-44323   OnPlayerExitVehicle        -> 3 lapis anti-exit
44421-44457   OnPlayerStateChange        -> guard onfoot
48411-48465   OnPlayerUpdate             -> rem mendadak / terpental
```

## File lain di folder artifacts (bukan sumber ekstraksi ini)

| File | Catatan |
|---|---|
| `CDCNL/gamemodes/iansyah.pwn` | GM original, reminder 5 menit, tidak lengkap |
| `artifacts/iansyah.pwn` | copy pola lengkap |
| `artifacts/iansyah_speedo.pwn` | pola reminder lama |

Kalau compile error 017 `undefined symbol "IsVehicleSeatbeltSupported"` atau `"SeatbeltVehicle"`, berarti yang ke-load masih file original / potongan tidak lengkap. Pakai `iansyah_FIXED.pwn` atau include di repo ini + hooks.
