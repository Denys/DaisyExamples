from pathlib import Path
from playwright.sync_api import sync_playwright

profile = Path.home() / ".notebooklm" / "browser_profile"

with sync_playwright() as p:
    context = p.chromium.launch_persistent_context(
        user_data_dir=str(profile),
        channel="msedge",
        headless=False,
        args=["--disable-blink-features=AutomationControlled"],
    )
    page = context.pages[0] if context.pages else context.new_page()
    page.goto("https://notebooklm.google.com/", wait_until="networkidle", timeout=120000)
    print("TITLE:", page.title())
    texts = []
    for locator in [page.get_by_role("button"), page.get_by_role("link")]:
        count = min(locator.count(), 20)
        for i in range(count):
            try:
                txt = locator.nth(i).inner_text(timeout=2000).strip()
            except Exception:
                txt = ""
            if txt:
                texts.append(txt)
    print("TEXTS:")
    for t in texts[:40]:
        print("-", t)
    context.close()
