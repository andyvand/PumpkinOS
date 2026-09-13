# Mail for PumpkinOS

An e-mail client that runs as a native PumpkinOS application. It reads mail from
an IMAP4rev1 mailbox and sends mail through SMTP, over connections secured by the
PumpkinOS secure provider (TLS).

* Message list of the newest messages of a folder (unread messages in bold),
  with a folder popup fed by the server's `LIST` response.
* Message view: From / To / Subject / Date and the text of the message in a
  scrollable field. `multipart/*` messages are walked recursively, `text/plain`
  is preferred over `text/html` (which is stripped down to text), base64 and
  quoted-printable are decoded, UTF-8 is converted to the Latin-1 character set
  of the PalmOS fonts, RFC 2047 encoded headers are decoded, and attachments are
  listed by name and size. Opening a message marks it `\Seen`.
* Reply (with quoted text and `In-Reply-To`), Forward, Delete (`\Deleted` +
  `UID EXPUNGE`, falling back to `EXPUNGE`).
* Compose: To, Cc, Subject and a body field; messages are sent as
  `text/plain; charset=ISO-8859-1` in quoted-printable, with non-ASCII headers
  RFC 2047 encoded.
* One account, stored in the application preferences: name, address, login,
  IMAP server/port/security, SMTP server/port/security and the number of headers
  to fetch.
* All network work runs in a worker thread; a small modal dialog shows the
  progress and lets the user cancel.

## Security

TLS is not part of PumpkinOS itself: it comes from the *secure provider* library
that the startup script loads before `libos` (`liblopenssl` with OpenSSL, or
`libls2n`, see `script/pumpkin.lua`; on the ESP32 firmware it is
`components/liblmbedtls`, enabled with `ENABLE_HTTPS` in menuconfig). The Mail
application obtains the provider with `pumpkin_get_secure()` and uses it for:

* **SSL/TLS** — implicit TLS on connect (IMAPS 993, SMTPS 465). This is the default.
* **STARTTLS** — plain connection, upgraded with the `STARTTLS` command
  (IMAP 143, SMTP submission 587). The client refuses to continue when the server
  does not offer STARTTLS.
* **None** — unencrypted. Only meant for local test servers; the password is
  sent in clear text.

Certificates are verified by the provider (system CA bundle on the desktop,
the ESP-IDF certificate bundle on the ESP32). A failed handshake or verification
is reported as "Secure connection failed"; details are in `pumpkin.log`
(lines tagged `MAIL` and `SECURE`). When no provider is loaded, the secure modes
fail with an explanatory message.

Authentication uses IMAP `LOGIN` and SMTP `AUTH PLAIN` (falling back to
`AUTH LOGIN`). Providers such as Gmail need an app-specific password.

The password is stored in the saved application preferences, unencrypted, like
the other settings of a Palm OS application.

## Files

| File | Purpose |
|---|---|
| `Mail.c` | User interface (forms, event handlers, worker-thread job control) |
| `mailnet.c` / `mailnet.h` | Connection layer over the secure provider, IMAP, SMTP, MIME and header decoding, message construction. Only depends on libpit. |
| `resource.rcp` / `resource.h` | Forms, menus and alerts |
| `mail32.bmp` / `mail64.bmp` | Application icon |
| `esp32.h`, `make_esp32_prc.sh` | ESP32 firmware build support |

## Building

Desktop (from the PumpkinOS `src` directory, with the libraries and tools built):

    make -f GNUMakefile -C palmmail

This produces `Mail.prc` and copies it into `vfs/app_install`, from where
PumpkinOS installs it on the next start. `palmmail` is part of `BASE_MODULES` in
`src/GNUMakefile`, so the top-level build includes it. The application needs the
`pumpkin_get_secure()` accessor added to `libpumpkin`, so rebuild `libpumpkin`
when updating an older tree.

ESP32: the sources are compiled into the firmware through
`src/components/palmmail` (symlinks into this directory) and the creator ID
`PMal` is mapped to `MailPilotMain` in `Launcher.c`. The resources-only
`Mail.prc` for the LittleFS image is produced by

    ./make_esp32_prc.sh

which copies it into `src/data/vfs/app_install/`.

## Limits

* Messages are downloaded up to 128 KB (48 KB on the ESP32) and the text shown is
  cut at 30000 characters; the rest is marked as truncated.
* Attachments are listed but cannot be opened or sent.
* Only one account and one folder at a time; no local copies of messages are kept.
