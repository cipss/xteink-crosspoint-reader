# X3+ Calendar integration

X3+ uses the iCalendar (`.ics`) format as its compatibility layer. The device can read local `.ics` files from `/Calendar` and can subscribe to a remote iCalendar URL.

## iPhone / iCloud

Apple Calendar keeps iCloud calendars synchronized across devices. X3+ can consume an iCalendar subscription URL when a calendar is published for subscription. The URL is stored locally on the device and refreshed over Wi-Fi.

For a private iCloud workflow, the X3+ project intentionally does not request an Apple Account password. A future companion service may provide authenticated CalDAV synchronization without placing credentials on the X3.

## Google Calendar

Google Calendar exposes an iCal address under a calendar's integration settings. The secret iCal address can be used by another calendar application, and Google warns that it should not be shared. X3+ treats such a URL as a read-only feed and stores the downloaded `.ics` copy locally.

## Current implementation

- local `.ics` import;
- remote iCalendar subscription URL;
- HTTPS fetch over Wi-Fi;
- VEVENT parsing for UID, SUMMARY, DTSTART and DTEND;
- date/time display;
- event sorting by start time;
- cached remote calendar for offline display.

## Future write support

Writing events back to Apple/Google requires an authenticated provider protocol. That is intentionally separated from the read-only ICS client so the firmware never needs to embed an account password or OAuth secret.
