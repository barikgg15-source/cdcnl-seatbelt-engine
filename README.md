# CDCNL Seatbelt + Engine

Sistem **sabuk pengaman** dan **mesin kendaraan** dari gamemode lokal **CDCNL / PALAPA CITY**.

Diekstrak dari `CDCNL/gamemodes/iansyah_FIXED.pwn` (versi lokal yang lebih lengkap).
Bukan dari `iansyah.pwn` original — file itu masih pakai reminder 5 menit dan error 017 (`IsVehicleSeatbeltSupported` / `SeatbeltVehicle` undefined).

Repo: [barikgg15-source/cdcnl-seatbelt-engine](https://github.com/barikgg15-source/cdcnl-seatbelt-engine)

## Isi repo

| File | Isi |
|---|---|
| `include/cdcnl_seatbelt_engine.inc` | Variabel, stock, `/sabuk`, `/engine`, timer alarm |
| `snippets/hooks.pwn` | Potongan callback yang harus nempel di GM |
| `CARA_KERJA.md` | Penjelasan alur, state, diagram, rumus damage |
| `INTEGRASI.md` | Cara pasang, dependensi, tes in-game |
| `PETA_KODE.md` | Peta baris ke file gamemode lokal |

## Fitur

- `/sabuk` pasang/lepas (progress 2 detik + SFX klik)
- Tidak bisa turun kendaraan selama sabuk terpasang (lock per-player + 2 callback + timer 150 ms)
- `/engine` delay 2 detik, tolak jika health kendaraan < 350, butuh BBM
- Alarm audio tiap **7 detik** selama mesin ON dan masih ada yang belum sabuk
- Mobil: 1 orang belum sabuk → semua dengar. Bus 431/437: hanya sopir
- Motor/sepeda tidak punya sabuk
- Rem mendadak tanpa sabuk = damage HP, bisa terpental
- Tombol panel `setbeltv` / `mesinv`
- Reset aman di connect / death / disconnect / `RemovePlayerFromVehicleEx`

## Perintah

```
/sabuk     pasang atau lepas sabuk
/engine    nyalakan atau matikan mesin (hanya driver)
```

Panel kendaraan:
- klik `setbeltv` = `/sabuk`
- klik `mesinv` = `/engine`

## Audio

| Event | URL |
|---|---|
| Klik pasang sabuk | `http://b.top4top.io/m_38644325f1.mp3` |
| Alarm 7 detik | `http://h.top4top.io/m_3863604rq1.mp3` |

## Baca dulu

1. [CARA_KERJA.md](CARA_KERJA.md) — bagaimana sistem ini jalan
2. [INTEGRASI.md](INTEGRASI.md) — cara pasang ke gamemode
3. [PETA_KODE.md](PETA_KODE.md) — baris sumber di `iansyah_FIXED.pwn`

## Sumber di local PC

```
CDCNL/gamemodes/iansyah_FIXED.pwn
  baris 50-60      variabel + forward
  baris 338        Text: setbeltv
  baris 9747-9751  RemovePlayerFromVehicleEx reset
  baris 28922+     TextDraw setbeltv
  baris 32263+     stocks + CMD:sabuk + ForceSeatbeltBack
  baris 34840+     CMD:engine + TurnOnEngine + SeatbeltReminder
  baris 41444+     OnPlayerConnect reset
  baris 43146      OnPlayerDisconnect
  baris 43776      OnPlayerDeath
  baris 44305+     OnPlayerExitVehicle
  baris 44421+     OnPlayerStateChange
  baris 48411+     OnPlayerUpdate (kecelakaan)
  baris 66384+     klik mesinv / setbeltv
  baris 71531+     CloseEngine
```
