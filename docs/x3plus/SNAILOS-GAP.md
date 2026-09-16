# Snail OS / CrossPoint X3 capability gap

This document records the feature areas worth reimplementing independently for CrossPoint X3+.

## Already covered by CrossPoint

- EPUB 2/3 reader and plain text reading
- XTC/XTCH/TXT/BMP support
- Library and recent books
- Bookmarks, chapter navigation, dictionary, progress and reading controls
- Wi-Fi file transfer
- Web settings/API
- WebDAV
- OPDS browsing
- Calibre wireless flow
- OTA updates
- Themes, button remapping, sleep/display controls
- RTL and multilingual UI

## X3+ additions

### 1. Launcher and app runtime

- Dedicated X3+ home area
- App registry
- App lifecycle and navigation
- Versioned app metadata
- Safe persistent per-app state
- Optional SD-card app packages

### 2. Manga

- CBZ/ZIP image volumes
- RTL page order
- Full-page and fit-to-screen modes
- Volume/chapter browser
- Cover thumbnails
- Reading progress
- Next/previous chapter navigation
- Image caching designed for the X3 memory limits

### 3. Productivity

- Notes stored on SD
- Calendar month/day views
- Reminders/events model
- Clock and device time
- Pomodoro/timer
- Basic calculator

### 4. Internet

- Weather with bounded responses and offline cache
- RSS/Atom feed reader with feed cache
- Text-only browser/reader
- Article save-to-SD
- Wikipedia/text reference workflow

### 5. Games

- Sudoku
- Chess/Gomoku or other turn-based games
- Minesweeper
- 2048
- Small offline games only; no continuously animated UI

### 6. Connectivity

- Bluetooth utility/companion layer where X3 hardware support permits it
- Device notifications
- Phone/desktop companion protocol
- Optional account-backed sync without embedding long-lived credentials in SD-card apps

### 7. Store / plugin model

- On-device catalogue
- Install/update/uninstall for safe data-only application packages
- Optional interpreted app layer for small utilities
- Strict memory and execution budgets
- Signed/pinned catalogue transport

## Architectural rule

CrossPoint already owns hardware, storage, rendering and reader stability. X3+ applications should consume those services through narrow interfaces rather than directly changing the reader engine.

Networked features must use bounded buffers and cache results to SD. Large parsers and hardware-facing operations remain firmware-level services; small UI apps should not gain arbitrary file-system or native-code execution privileges.
