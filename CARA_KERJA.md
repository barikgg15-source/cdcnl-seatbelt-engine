# Cara Kerja Sistem Seatbelt + Engine CDCNL

Sumber: `CDCNL/gamemodes/iansyah_FIXED.pwn` (versi lokal yang sudah diperbaiki).
File lama `CDCNL/gamemodes/iansyah.pwn` masih pakai reminder **300000 ms (5 menit)** dan belum punya `ForceSeatbeltBack` / alarm per-kendaraan.

Sistem ini **dua modul yang saling terkait**:

1. **Engine** — nyala/mati mesin, syarat BBM + health.
2. **Seatbelt** — pasang/lepas sabuk, kunci pintu per-player, alarm 7 detik, damage rem mendadak.

Titik temu: setiap kali mesin **nyala** atau **mati**, dan setiap kali seseorang **pasang/lepas/turun**, GM memanggil `RefreshVehicleSeatbeltWarning(vehicleid)`. Fungsi itu yang memutuskan alarm hidup atau mati untuk **seluruh occupant** kendaraan itu.

---

## 1. Variabel state

| Variabel | Tipe | Arti |
|---|---|---|
| `Seatbelt[playerid]` | bool | `true` kalau sabuk terpasang |
| `SeatbeltVehicle[playerid]` | int | ID kendaraan tempat sabuk dipasang |
| `SeatbeltSeat[playerid]` | int | Nomor kursi saat pasang (dipakai `PutPlayerInVehicle`) |
| `SeatbeltWarningTimer[playerid]` | int | Handle timer alarm 7 detik |
| `LastVehicleSpeed[playerid]` | float | Kecepatan frame sebelumnya, untuk deteksi rem mendadak |

Kalau hanya 3 variabel pertama yang ada (versi bug lama), compiler error 017:
`undefined symbol "IsVehicleSeatbeltSupported"` dan `"SeatbeltVehicle"`.

Kenapa `SeatbeltVehicle` + `SeatbeltSeat` wajib? Karena saat player tekan F, `GetPlayerVehicleID` sudah 0. Guard kedua (`OnPlayerStateChange`) butuh ID kendaraan + kursi yang **tersimpan** supaya bisa `PutPlayerInVehicle` lagi.

---

## 2. Filter kendaraan

`IsVehicleSeatbeltSupported` **menolak**:

- Sepeda: `481, 509, 510`
- Motor: `461, 462, 463, 468, 521, 522, 581, 586`

Model lain (mobil, truk, bus, helikopter, dll.) dianggap punya sabuk.

`IsVehicleBus` khusus `431` (Bus) dan `437` (Coach). Di bus, alarm + cek belum sabuk **hanya sopir**. Penumpang bus tidak dihitung dan tidak dengar alarm.

`CloseEngine` (beda fungsi) menandai kendaraan yang **tidak pakai sistem mesin BBM**:

- `481, 509, 510` sepeda
- `460` Skimmer (perahu/pesawat kecil)

Kalau `CloseEngine` true, `/engine` langsung `return 1` — tidak toggle, tidak cek BBM.

---

## 3. Alur `/sabuk`

Bisa dipanggil dari chat atau klik textdraw `setbeltv`.

```
Player ketik /sabuk  ATAU  klik textdraw setbeltv
        |
        +-- tidak di kendaraan / bukan driver-penumpang  --> error
        +-- sepeda / motor                               --> tidak memiliki sabuk
        +-- progress bar masih jalan                     --> tunggu
        |
        +-- belum sabuk --> progress 2 detik + SFX click
        |                   SeatbeltAction(playerid, 1)
        |                     Seatbelt = true
        |                     simpan vehicle + seat
        |                     SetVehicleParamsForPlayer(..., lock doors = 1)
        |                     RefreshVehicleSeatbeltWarning
        |
        +-- sudah sabuk --> progress 2 detik
                            SeatbeltAction(playerid, 0)
                              unlock pintu player ini
                              Seatbelt = false
                              RefreshVehicleSeatbeltWarning
```

`SetVehicleParamsForPlayer(vehicleid, playerid, objective, doorslock)`
mengunci pintu **hanya untuk player itu**. Penumpang lain tetap bisa keluar kalau mereka belum / sudah lepas sabuk sendiri.

Progress 2 detik **tidak memblokir gerakan**. Kalau player turun sebelum timer `SeatbeltAction` jalan, `SeatbeltAction` cek `IsPlayerInAnyVehicle` lalu batal — state sabuk tidak berubah.

Chat bubble `* Memasang / Melepas sabuk pengaman *` tampil 3 detik ke pemain sekitar.

---

## 4. Kenapa player tidak bisa turun saat sabuk terpasang

SA-MP tidak punya cancel exit yang 100% andal. Native lock kadang lolos kalau player spam F. Karena itu ada **3 lapis**:

1. `SetVehicleParamsForPlayer(..., doorslock=1)` — native lock per-player
2. `OnPlayerExitVehicle` — kalau tetap keluar, langsung `PutPlayerInVehicle` + timer `ForceSeatbeltBack` 150 ms, pesan `Anda masih memakai sabuk pengaman`
3. `OnPlayerStateChange` ke `PLAYER_STATE_ONFOOT` — guard kedua, logic sama

`ForceSeatbeltBack` mengecek lagi 150 ms kemudian. Kalau player sudah di luar kendaraan padahal `Seatbelt=true`, dia dimasukkan kembali ke kursi yang tersimpan, pintu dikunci lagi.

Pengecualian: `RemovePlayerFromVehicleEx` **wajib** `ResetPlayerSeatbelt` dulu. Tanpa itu, admin/script yang nendang player dari mobil akan membuat player masuk lagi lewat hook state-change.

Reset juga dipanggil di:

- `OnPlayerConnect` — bersihkan sisa data slot player
- `OnPlayerDisconnect`
- `OnPlayerDeath`

Saat keluar **sah** (sabuk sudah lepas):

1. `StopSeatbeltWarning` player itu
2. `ResetPlayerSeatbelt`
3. `RefreshVehicleSeatbeltWarning` kendaraan — sisa occupant dievaluasi ulang

---

## 5. Alur `/engine`

Hanya **DRIVER**. Tombol panel `mesinv` = `callcmd::engine`.

```
Hanya DRIVER
        |
CloseEngine? (sepeda 481/509/510, perahu 460) --> diam, tidak pakai BBM
Tidak punya sistem BBM                         --> toggle engine langsung
BBM < 1                                        --> error
Gang naik kendaraan LSPD                       --> diam (anti-steal)
        |
Mesin SUDAH NYALA --> matikan + RefreshVehicleSeatbeltWarning (alarm berhenti)
Mesin MATI        --> health < 350? ditolak kendaraan rusak
                      health >= 350 --> progress 2 detik
                      timer TurnOnEngine
                        masih di kendaraan itu?
                        mesin belum nyala?
                        SetVehicleParamsEx engine=1
                        RefreshVehicleSeatbeltWarning
```

Catatan penting dari source asli:

Cabang `health <= 350` delay 4 detik **tidak pernah tercapai**, karena baris sebelumnya sudah `return` saat `health < 350`.
Logic yang hidup:

- sehat (`health >= 350`) = delay 2 detik + progress Menghidupkan Mesin
- rusak parah (`health < 350`) = ditolak, suruh repair

Kalau player turun sebelum `TurnOnEngine` jalan, mesin **tidak** nyala (`IsPlayerInVehicle` gagal).

Saat mesin mati, `RefreshVehicleSeatbeltWarning` melihat `engine != 1` maka `need_warn = false` maka semua timer alarm di kendaraan itu di-kill.

---

## 6. Alarm sabuk (reminder)

Pemicu tunggal: `RefreshVehicleSeatbeltWarning(vehicleid)`

Dipanggil dari:

- `SeatbeltAction` (pasang / lepas)
- `TurnOnEngine` (mesin baru nyala)
- `CMD:engine` saat mesin dimatikan
- `OnPlayerExitVehicle` / `OnPlayerStateChange` setelah reset
- `StartSeatbeltWarning` (wrapper 1 player)

Syarat alarm ON:

- kendaraan mendukung sabuk
- `engine == 1`
- `VehicleHasUnbeltedOccupant` = masih ada orang yang belum sabuk (bus: hanya sopir)

Perilaku:

- **Mobil biasa**: 1 orang belum sabuk maka **semua** occupant dengar audio + dapat warning
  - yang belum sabuk: `Kamu belum memasang sabuk pengaman!`
  - yang sudah sabuk: `Ada orang di kendaraan ini yang belum memakai sabuk pengaman!`
- **Bus (431 / 437)**: hanya sopir yang dihitung dan yang dengar alarm

Audio: `http://h.top4top.io/m_3863604rq1.mp3`
Timer: `SetTimerEx("SeatbeltReminder", 7000, true, "i", i)` — **ulang tiap 7 detik**

`SeatbeltReminder` hanya memutar ulang audio. Keputusan stop ada di sini:

- player disconnect / turun
- mesin mati
- semua yang dihitung sudah sabuk
- kendaraan tidak support sabuk
- di bus tapi dia penumpang

Ini beda jauh dengan GM lama (`iansyah.pwn` original) yang timer-nya **300000 ms sekali** dan tidak refresh per-kendaraan.

---

## 7. Damage kecelakaan (`OnPlayerUpdate`)

Hanya **driver**. Setiap frame:

```
speed_sekarang = GetVehicleSpeed(vehicleid)
selisih        = LastVehicleSpeed[playerid] - speed_sekarang
```

Artinya `selisih` besar = rem / tabrak mendadak (kecepatan turun tajam).

Jika `selisih > 30` **dan tidak pakai sabuk**:

- damage HP = `selisih * 0.75`
- notifikasi:
  - selisih >= 100 → kecelakaan sangat parah
  - >= 60 → berat
  - >= 35 → terluka

Jika sebelumnya kecepatan > 55 **dan** selisih > 40 **dan** tidak sabuk:

- `RemovePlayerFromVehicle` (native, **bukan** `RemovePlayerFromVehicleEx`)
- naikkan Z +1.2 agar tidak stuck di body mobil
- dorong velocity kendaraan * 1.6 + Z 0.9
- animasi `PED / KO_shot_stomp`
- pesan terpental

Kalau sabuk terpasang, blok damage + terpental **tidak jalan**.

`LastVehicleSpeed` di-update di akhir blok, jadi frame berikutnya punya acuan baru.

Catatan: eject pakai `RemovePlayerFromVehicle` native. Hook `OnPlayerStateChange` bisa mencoba memasukkan kembali kalau `Seatbelt` masih true. Di jalur ini `Seatbelt` memang false (syarat blok), jadi player tetap terpental.

---

## 8. Panel textdraw

`setbeltv` dibuat di `TextDrawCreate(460.000, 277.000, "ld_beat:chit")`, selectable.
Klik → `callcmd::sabuk(playerid, "")`.

`mesinv` → `callcmd::engine(playerid)`.

Label `Panel_Kendaraan[25] = "SEATBELT"`.

Saat panel ditutup (`closev`), `setbeltv` / `mesinv` di-hide bersama tombol lain.

---

## 9. Perbandingan file lokal

| File | Status |
|---|---|
| `CDCNL/gamemodes/iansyah.pwn` | lama: reminder 5 menit, belum lengkap |
| `CDCNL/gamemodes/iansyah_FIXED.pwn` | lengkap (sumber ekstraksi ini) |
| `artifacts/iansyah.pwn` | copy lengkap yang sama polanya |
| `artifacts/iansyah_speedo.pwn` | masih pola reminder lama |

---

## 10. Diagram ringkas

```
          /engine ON
               |
               v
    RefreshVehicleSeatbeltWarning
               |
     ada yang belum sabuk?
          /          \
        YA           TIDAK
         |             |
   audio 7s         stop audio
   lock? tidak      semua tenang
         |
    player /sabuk
         |
   lock pintu dirinya
   kalau semua sudah sabuk → alarm mobil mati
         |
    tekan F / exit
         |
   masih sabuk? --YA--> PutPlayerInVehicle + ForceSeatbeltBack
         |
        TIDAK
         |
   Reset + refresh sisa penumpang
```

---

## 11. Urutan event yang sering bikin bingung

**Kasus A — sopir nyalakan mesin, penumpang belum sabuk**

1. Sopir `/engine` → delay 2 dtk → `TurnOnEngine`
2. `Refresh` lihat ada unbelted → sopir + penumpang dengar alarm
3. Penumpang `/sabuk` → lock pintu penumpang saja
4. Kalau sopir sudah sabuk juga, alarm semua mati

**Kasus B — spam F pakai sabuk**

1. Native lock menahan sebagian besar
2. Kalau lolos, `OnPlayerExitVehicle` memasukkan kembali + timer 150 ms
3. Kalau state sudah onfoot sebelum exit callback, `OnPlayerStateChange` yang merespons

**Kasus C — admin kick dari mobil**

Harus lewat `RemovePlayerFromVehicleEx` yang sudah `ResetPlayerSeatbelt`. Kalau pakai native `RemovePlayerFromVehicle` saat sabuk masih true, player akan ditarik masuk lagi.
