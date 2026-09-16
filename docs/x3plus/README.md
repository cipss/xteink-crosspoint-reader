# CrossPoint X3+

CrossPoint X3+ is an independent extension of CrossPoint Reader for the Xteink X3.

The project keeps the existing reading engine and hardware support while adding an application-oriented layer inspired by the capabilities observed in other X3 software, including Snail OS. It does **not** copy proprietary firmware or proprietary source code.

## Goals

- Preserve CrossPoint EPUB/TXT/XTC reading stability.
- Add an X3+ launcher and application framework.
- Add Manga, Notes, Weather, Calendar, RSS/News, Browser and Games.
- Add device utilities, notifications, Bluetooth/companion integration and an extensible app model where the X3 hardware permits it.
- Keep memory, flash, SD I/O and e-ink refresh costs explicit in the architecture.

## Development principles

1. Reader reliability takes priority over application features.
2. Features that need persistent storage, background work, raw hardware access or large memory allocations belong in firmware-level C++ services.
3. Small user applications should be isolated from the core runtime.
4. Network responses must be bounded and cache-friendly.
5. Every feature gets a host-side test/simulation path where practical before hardware flashing.

## Current milestone

**Foundation: X3+ launcher shell**

The first milestone adds a launcher entry from Home and a dedicated X3+ app screen. Individual apps are intentionally stubbed until the core navigation and memory-safe application boundary are stable.
