# Projekt Saper - Gra Sieciowa

**Saper** to gra sieciowa oparta na architekturze klient-serwer, w której dwóch graczy rywalizuje w odkrywaniu pól na planszy. (Program wspiera wiele równoległych rozgrywek między parami graczy.)

---

## **Opis gry**

- Gra działa na planszy o rozmiarze 10x10.
- Gracze na zmianę wykonują swoje ruchy, odkrywając pola.
- Celem gry jest unikanie min i odkrycie wszystkich bezpiecznych pól.
- Możliwe wyniki gry:
  - **WIN** – jeśli gracz wygra.
  - **LOSE** – jeśli gracz przegra, odkrywając minę.
  - **DRAW** – w przypadku remisu, jeżeli cała plansza zostanie odkryta i żaden z graczy nie odkrył miny.

---

## **Wymagania**

Aby uruchomić grę, upewnij się, że posiadasz:
- System Linux (projekt testowany na Ubuntu).
- Skonfigurowane środowisko zainstalowanym kompilatorem C++ oraz bibliotekami sieciowymi.

---

## **Instrukcja obsługi**

### **Uruchomienie serwera**

1. Otwórz terminal i przejdź do folderu `Server`:
   ```bash
   cd Server

2. Uruchom serwer:
   ```bash
   ./ser

### **Uruchomienie Klienta**

1. Otwórz terminal i przejdź do folderu `Client`:
   ```bash
   cd Client

2. Uruchom klienta:
   ```bash
   ./cli
Uruchom przynajmniej dwóch graczy, aby rozpocząć rozgrywkę.
### **Rozgrywka**
1. Po uruchomieniu gry, serwer tworzy dla nich planszę.
2. Każdy z graczy w swojej turze może poruszać się po planszy używając klawiszy:
- **`w`** – porusz się w górę
- **`a`** – porusz się w lewo
- **`s`** – porusz się w dół
- **`d`** – porusz się w prawo
- **`spacja`** – postaw flagę
- **`enter`** – odkryj pole
3. Po każdym ruchu plansza jest aktualizowana i wysyłana do obu graczy.
4. Gra kończy się wynikiem WIN, LOSE lub DRAW, który zostaje wyświetlony na ekranie.

---
## **Struktura projektu**
- Server/server.cpp - kod serwera obsługujący grę oraz graczy do niej podłączonych.
- Server/Saper.hpp - klasa obsługująca logikę gry oraz wygląd planszy w konsoli.
- Client/client.cpp - kod klienta umożliwiający mu na połączenie się z serwerem a następnie grę.

---
## **Autor**
Piotr Nowacki
