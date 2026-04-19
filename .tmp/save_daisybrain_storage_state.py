from pathlib import Path

from playwright.sync_api import sync_playwright


home = Path.home() / ".notebooklm-daisybrain"
profile = home / "browser_profile"
storage = home / "storage_state.json"

home.mkdir(parents=True, exist_ok=True)

with sync_playwright() as p:
    context = p.chromium.launch_persistent_context(
        user_data_dir=str(profile),
        headless=False,
        args=[
            "--disable-blink-features=AutomationControlled",
            "--password-store=basic",
        ],
        ignore_default_args=["--enable-automation"],
    )
    context.storage_state(path=str(storage))
    print(f"Saved storage state to: {storage}")
    context.close()
