# X3+ Manga subsystem

## Fase 1 — Library e indice
- `/Manga` is the user-facing manga folder.
- `.cbz` and `.zip` archives are detected automatically.
- ZIP central-directory entries are indexed to SD without loading the archive in RAM.
- Only image pages (`jpg`, `jpeg`, `png`, `bmp`) are indexed.

## Fase 2 — Automatic display adaptation
- JPEG and PNG pages are converted through the existing CrossPoint stream converters.
- Conversion receives the logical X3 display bounds so large source images are reduced before rendering.
- The final bitmap is rendered with a second viewport safety-fit.
- Aspect ratio is preserved; smaller pages are centered.
- This is intentionally SD-first to keep ESP32-C3 heap pressure low.

## Fase 3 — Reader e progress
- Full-screen manga reader activity.
- Previous/next navigation on X3 page buttons.
- Reading progress is stored per archive in `/.crosspoint/x3plus/manga/<cache>/progress.bin`.
- Returning to a manga resumes from the last stored page.
- A minimal HUD can show page `N / total` without permanently reducing the reading area.

## Fase 4 — Reader preferences
`/.crosspoint/x3plus/manga/settings.cfg` stores:
- `rtl`: logical page direction.
- `fitMode`: page, width, height or smart.
- `prefetchNext`: reserve for look-ahead decoding.
- `keepPreviousCache`: reserve for cache retention policy.

The first UI exposes these settings from the Manga library.

## Fase 5 — X3 UX and cache policy
Target design:
- keep the current page plus a small adjacent-page cache on SD;
- avoid multi-page RAM buffers;
- add optional double-page mode when the device is rotated;
- add long-page segmentation for webtoon-style images;
- add cover extraction and metadata presentation;
- add archive refresh/invalidation when a file changes;
- add benchmark reporting for decode, render and SD latency.

## Fase 6 — Rendering avanzato e robustezza
Implemented:
- Natural page ordering (`page2` before `page10`) while keeping deterministic archive order for ties.
- ZIP archives are explicitly opened again before page extraction, preventing first-page extraction failures after indexing.
- Cache directories are created defensively through `/.crosspoint` → `/.crosspoint/x3plus` → manga cache.
- Render cache filenames include their target dimensions, so changing display mode cannot silently reuse an incompatible decode.
- `Fit Page`, `Fit Width`, `Fit Height` and `Smart` now produce different decode/render bounds.
- Adjacent-page prefetch uses the active fit mode, avoiding a second decode when the next page is opened.
- Corrupt/truncated index entries are rejected instead of being counted as valid pages.

Remaining manga work for the next phase:
- long-page/webtoon segmentation;
- double-page/spread mode;
- cover thumbnails and archive metadata;
- bounded cache pruning based on reader policy;
- non-blocking decode/SD jobs and benchmark telemetry.

The X3+ implementation must remain independent from proprietary SnailOS source code.
