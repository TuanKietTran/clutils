# clutils — Swiss Army Knife for the Terminal

Fast, single-binary CLI toolbox for daily dev tasks — UUIDs, hex/base64/URL encoding, HMAC, and more.  
Everything is auto-copied to clipboard.

### Features
- `clutils uuid` → generate UUID (copied)
- `clutils hex …` / `unhex …`
- `clutils b64 …` / `unb64 …` / `b64url` / `unb64url`
- `clutils url …` → URL-encode
- `clutils rot13 …`
- `clutils hmac <key> <data>` → HMAC-SHA256 (hex)
- All results instantly copied to clipboard

### (Actually-Not-)One-command setup (macOS / Linux / WSL)

```bash
git clone https://github.com/yourname/clutils.git
cd clutils
./setup.sh          # clones fresh vcpkg, builds, done!
./build/clutils uuid