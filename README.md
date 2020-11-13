beehive-v0.9
Stand 11-2020
# LoRaWAN Bienenwaage 2 (experimentell)

Das Projekt soll den Bau einer Waage ermöglichen, die unter einen Bienenstock gestellt wird.
Die gewonnen Daten (Gewicht und Temperatur) geben Informationen, anhand derer die Bienen mit weniger und dafür gezielten Eingriffen bewirtschaftet werden können.

Das aktuelle Projekt soll die Anbindung per LoRaWAN ermöglichen.
- es muss ein LoRaWAN-Gateway in Reichweite sein
- ein TTN Account ist erforderlich, eine App und die Schlüssel müssen nutzbar sein

Rückfragen dauern etwas länger, da das Projekt ausschließlich als Hobby in Freizeit entsteht.
Viel Freude beim Probieren!

Teile:
- Heltec CubeCell (HTCC AB01) ardunokompatibel
- HX711 A/D für Loadcell
- H30A Loadcell Bosche alternativ alle Loadcell-Varianten die als Vollbrücke arbeiten z.B. 4x Human_Loadcell je 50kg
- 1 DS18B20 Temperatursensor
- Solarzelle 5V 0,5W
- LiPo Akku 18650 mit Halterung
- 1 Widerstande 4K7 (für DS18B20)
- ggf. 100uF 10V Kondensator für Stabilierung der Eingangsspannung beim Sendevorgang
- Lochrasterplatte 70x50mm, passende Steckleisten, Drähte, Lötzubehör

Die Daten werden üder MQTT von TTN abgeholt und Lokal (Synology NAS DS218+ mit Docker-Containern) verarbeitet

-> MQTT (TTN) -> NodeRed (umarbeiten für INFLUX-DB) - speichern in INFLUX-DB -> Anzeige in GRAFANA DashBoards

ToDo: abholen über HTTP-Integration von TTN in Webspace (Hoster mit mySQL), Anzeige auf eigener Website z.B. mit highcharts
