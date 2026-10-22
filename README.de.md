# ESPHome BRMesh Gateway (Kurzfassung)

*Vollständige Anleitung auf Englisch: [README.md](README.md)*

Steuert **BRMesh-/Fastcon-BLE-Leuchten** (eingerichtet mit der App *BRMesh*)
über einen ESP32 mit **ESPHome** aus **Home Assistant** – inklusive der
**nativen Farbeffekte** der Leuchten. Kein Cloud-Dienst, kein Handy, kein
MQTT: Die Leuchten erscheinen über die normale ESPHome-Integration.

## Funktionen

- Ein/Aus, Helligkeit, RGB-Farbe und Weiß je Leuchte
- **Native Effekte**, die die Leuchte selbst ausführt (ein BLE-Befehl genügt)
- Alle **21 Effekte der BRMesh-App** plus eigene Effekte als einfache
  Farbliste in YAML
- **Effektgeschwindigkeit** (1–100) als Number-Entität
- Für ESPHome **2026.10.0** und neuer

## Getestete Hardware

- Board: ESP32-S3 Mini Development Board – [Amazon](https://www.amazon.de/dp/B0F1CR5RHV)
- Leuchten: DUSKTEC LED-Einbaustrahler RGB, 7 W, 230 V (BRMesh-App) – [Amazon](https://www.amazon.de/dp/B09FL59QJ2)

Andere ESP32 mit Bluetooth LE sollten funktionieren, der ESP32-S2 nicht
(kein Bluetooth).

## Schnellstart

1. Leuchten wie gewohnt in der BRMesh-App (Android) einrichten.
2. Mesh-Key und Light-IDs per `adb logcat` ermitteln – Befehle in der
   [README.md](README.md#finding-the-mesh-key).
3. [`example/brmesh-gateway.yaml`](example/brmesh-gateway.yaml) in den
   ESPHome-Ordner kopieren, `secrets.example.yaml` als `secrets.yaml`
   anlegen und ausfüllen.
4. `light_id` und Namen der Leuchten anpassen, Firmware installieren.

Der Mesh-Key ist wie ein Passwort zu behandeln – nie in Issues oder Logs
veröffentlichen.

## Einschränkungen

- Nur Senden: Die Leuchten melden keinen Zustand zurück.
- Jede Leuchte muss in BLE-Reichweite des Gateways sein.
- Während eines Effekts lässt sich die Helligkeit nicht ändern.

## Lizenz und Hinweise

[MIT](LICENSE), wie das Ursprungsprojekt
[esphome-fastcon](https://github.com/scross01/esphome-fastcon) (Details:
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)).

Unabhängiges Hobbyprojekt ohne Verbindung zu den Herstellern der App oder
der Leuchten; Markennamen gehören ihren Inhabern. Nutzung auf eigene Gefahr.
**230 V:** Installation und Anschluss netzbetriebener Leuchten nur durch eine
Elektrofachkraft.
