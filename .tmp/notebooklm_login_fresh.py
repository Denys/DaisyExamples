import json
import time
from pathlib import Path

from playwright.sync_api import sync_playwright


def main() -> None:
    cli_home = Path.home() / ".notebooklm-daisybrain"
    fresh_home = Path.home() / ".notebooklm-daisybrain-fresh"
    storage_path = cli_home / "storage_state.json"
    profile_path = fresh_home / "browser_profile"
    signal_file = fresh_home / "save_signal"
    log_file = fresh_home / "login_status.txt"

    signal_file.unlink(missing_ok=True)
    fresh_home.mkdir(parents=True, exist_ok=True)
    cli_home.mkdir(parents=True, exist_ok=True)

    log_file.write_text(
        "Opening browser for NotebookLM login.\n"
        "Sign in to Google and leave notebooklm.google.com open.\n"
        "Waiting for save signal...\n",
        encoding="utf-8",
    )

    with sync_playwright() as p:
        browser = p.chromium.launch_persistent_context(
            user_data_dir=str(profile_path),
            headless=False,
            args=["--disable-blink-features=AutomationControlled"],
        )
        page = browser.pages[0] if browser.pages else browser.new_page()
        page.goto("https://notebooklm.google.com/")

        while not signal_file.exists():
            time.sleep(1)

        storage = browser.storage_state()
        storage_path.write_text(json.dumps(storage), encoding="utf-8")
        log_file.write_text(
            f"Authentication saved to: {storage_path}\n",
            encoding="utf-8",
        )
        browser.close()

    signal_file.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
