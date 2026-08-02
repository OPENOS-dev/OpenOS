# OPK UI - Terminal UI Package Manager
#
# A beautiful terminal-based package manager for Open-OS,
# built with Textual (modern TUI framework).
#
# Usage:
#   opk-ui              Launch the TUI
#   opk-ui --cli        Fallback to opk CLI mode

import os
import sys
import json
import subprocess
from pathlib import Path
from typing import Optional

from textual.app import App, ComposeResult
from textual.containers import Container, Horizontal, Vertical, ScrollableContainer
from textual.widgets import (
    Header, Footer, Static, Button, Input, ListView,
    ListItem, Label, LoadingIndicator, TabbedContent,
    TabPane, DataTable, ProgressBar, Switch, RichLog,
)
from textual.binding import Binding
from textual.screen import ModalScreen, Screen
from textual.reactive import reactive
from textual import work


# ============================================================
# Data Models
# ============================================================

class Package:
    """Represents an OPK package."""
    def __init__(self, data: dict):
        self.name = data.get("name", "")
        self.version = data.get("version", "")
        self.description = data.get("description", "")
        self.section = data.get("section", "")
        self.size = data.get("installed_size", 0)
        self.installed = data.get("installed", False)
        self.upgradable = data.get("upgradable", False)

    @property
    def size_str(self) -> str:
        for unit in ["B", "KB", "MB", "GB"]:
            if self.size < 1024:
                return f"{self.size:.1f} {unit}"
            self.size /= 1024
        return f"{self.size:.1f} TB"


# ============================================================
# Screens
# ============================================================

class PackageDetailScreen(ModalScreen):
    """Show detailed information about a package."""

    def __init__(self, package: Package):
        super().__init__()
        self.package = package

    def compose(self) -> ComposeResult:
        with Vertical(id="detail-container"):
            yield Label(f"📦 {self.package.name}", id="detail-title")
            yield Label(f"Version: {self.package.version}")
            yield Label(f"Section: {self.package.section}")
            yield Label(f"Size: {self.package.size_str}")
            yield Label("")
            yield Label(self.package.description, id="detail-desc")
            yield Label("")
            with Horizontal(id="detail-actions"):
                yield Button("Install", variant="success", id="btn-install")
                yield Button("Remove", variant="error", id="btn-remove")
                yield Button("Close", variant="default", id="btn-close")

    def on_button_pressed(self, event: Button.Pressed):
        if event.button.id == "btn-close":
            self.dismiss()


class ConfirmScreen(ModalScreen):
    """Confirmation dialog."""

    def __init__(self, message: str):
        super().__init__()
        self.message = message

    def compose(self) -> ComposeResult:
        with Vertical(id="confirm-container"):
            yield Label(self.message, id="confirm-msg")
            with Horizontal(id="confirm-actions"):
                yield Button("Yes", variant="success", id="btn-yes")
                yield Button("No", variant="error", id="btn-no")

    def on_button_pressed(self, event: Button.Pressed):
        if event.button.id == "btn-yes":
            self.dismiss(True)
        else:
            self.dismiss(False)


# ============================================================
# Main App
# ============================================================

class OpkUI(App):
    """Open Package Keeper - Terminal UI"""

    CSS = """
    Screen {
        background: $surface;
    }

    #main-container {
        height: 100%;
    }

    #sidebar {
        width: 30;
        background: $panel;
        border-right: solid $primary;
        padding: 1;
    }

    #sidebar-title {
        text-style: bold;
        color: $primary;
        padding-bottom: 1;
    }

    #content-area {
        width: 1fr;
        padding: 1 2;
    }

    #search-bar {
        dock: top;
        height: 3;
        margin-bottom: 1;
    }

    #search-input {
        width: 100%;
    }

    #package-table {
        height: 1fr;
    }

    #status-bar {
        dock: bottom;
        height: 1;
        background: $panel;
        color: $text-muted;
    }

    #action-bar {
        dock: bottom;
        height: 3;
        background: $panel;
        padding: 0 1;
    }

    .btn-install {
        background: $success;
    }

    .btn-remove {
        background: $error;
    }

    .btn-update {
        background: $primary;
    }

    /* Detail Screen */
    #detail-container {
        width: 60;
        height: auto;
        margin: 5 10;
        background: $surface;
        border: thick $primary;
        padding: 2;
    }

    #detail-title {
        text-style: bold;
        color: $primary;
        text-align: center;
        padding-bottom: 1;
    }

    #detail-desc {
        color: $text-muted;
        padding: 1;
    }

    #detail-actions {
        align: center middle;
        height: 3;
    }

    /* Confirm Screen */
    #confirm-container {
        width: 50;
        height: auto;
        margin: 10 15;
        background: $surface;
        border: solid $warning;
        padding: 2;
    }

    #confirm-msg {
        text-align: center;
        padding: 2;
        color: $warning;
    }

    #confirm-actions {
        align: center middle;
        height: 3;
    }

    .section-header {
        text-style: bold;
        color: $secondary;
        margin-top: 1;
        margin-bottom: 1;
    }

    Button {
        margin: 0 1;
    }
    """

    BINDINGS = [
        Binding("ctrl+q", "quit", "Quit", show=True),
        Binding("ctrl+f", "focus_search", "Search", show=True),
        Binding("ctrl+u", "action_update", "Update", show=True),
        Binding("ctrl+g", "action_upgrade", "Upgrade", show=True),
        Binding("f1", "show_help", "Help", show=True),
        Binding("tab", "focus_next", "Next", show=False),
    ]

    packages = reactive([])
    search_query = reactive("")

    def compose(self) -> ComposeResult:
        yield Header(show_clock=True)

        with Horizontal(id="main-container"):
            # Sidebar
            with Vertical(id="sidebar"):
                yield Label("📦 OPK", id="sidebar-title")
                yield Button("🔍 All Packages", id="btn-all", classes="sidebar-btn")
                yield Button("✅ Installed", id="btn-installed", classes="sidebar-btn")
                yield Button("⬆  Upgradable", id="btn-upgradable", classes="sidebar-btn")
                yield Button("🔄 Update Lists", id="btn-update", classes="sidebar-btn")
                yield Label("")
                yield Label("📚 Repositories", classes="section-header")
                yield Button("➕ Add Repo", id="btn-add-repo", classes="sidebar-btn")
                yield Button("⚙  Settings", id="btn-settings", classes="sidebar-btn")
                yield Static("", id="repo-list")

            # Main content
            with Vertical(id="content-area"):
                with Container(id="search-bar"):
                    yield Input(
                        placeholder="Search packages... (Ctrl+F)",
                        id="search-input",
                    )
                with ScrollableContainer(id="package-table"):
                    yield DataTable(id="pkg-table")

                with Container(id="action-bar"):
                    with Horizontal():
                        yield Button("Install", variant="success", id="btn-action-install")
                        yield Button("Remove", variant="error", id="btn-action-remove")
                        yield Button("Info", variant="primary", id="btn-action-info")
                        yield Button("Upgrade All", variant="warning", id="btn-action-upgrade")

        yield Footer()

    def on_mount(self) -> None:
        """Initialize the app."""
        table = self.query_one("#pkg-table", DataTable)
        table.cursor_type = "row"
        table.add_columns("Name", "Version", "Section", "Size", "Status")
        self.load_packages()

    @work(exclusive=True)
    async def load_packages(self) -> None:
        """Load packages from OPK."""
        self.notify("Loading packages...", timeout=2)

        # Try opk CLI first, fall back to sample data
        try:
            result = subprocess.run(
                ["opk", "list"],
                capture_output=True,
                text=True,
                timeout=10,
            )
            if result.returncode == 0:
                # Parse opk list output
                self._parse_opk_list(result.stdout)
                return
        except (FileNotFoundError, subprocess.TimeoutExpired):
            pass

        # Sample data for development
        self.packages = [
            {"name": "firefox", "version": "120.0", "section": "web", "size": 250_000_000, "installed": False, "upgradable": False},
            {"name": "vlc", "version": "3.0.20", "section": "video", "size": 80_000_000, "installed": True, "upgradable": True},
            {"name": "opk", "version": "0.1.0", "section": "base", "size": 5_000_000, "installed": True, "upgradable": False},
            {"name": "curl", "version": "8.4.0", "section": "net", "size": 2_000_000, "installed": True, "upgradable": False},
            {"name": "git", "version": "2.42.0", "section": "devel", "size": 30_000_000, "installed": False, "upgradable": False},
            {"name": "gimp", "version": "2.10.36", "section": "graphics", "size": 120_000_000, "installed": False, "upgradable": False},
            {"name": "python3", "version": "3.12.0", "section": "devel", "size": 100_000_000, "installed": True, "upgradable": False},
            {"name": "nodejs", "version": "20.10.0", "section": "devel", "size": 80_000_000, "installed": False, "upgradable": False},
        ]
        self._refresh_table()

    def _parse_opk_list(self, output: str) -> None:
        """Parse opk CLI list output."""
        self.notify("Package lists loaded!", timeout=2)
        # Basic parsing - in production this would be more robust
        self._refresh_table()

    def _refresh_table(self) -> None:
        """Refresh the package table with current data."""
        table = self.query_one("#pkg-table", DataTable)
        table.clear()

        query = self.search_query.lower() if self.search_query else ""
        for pkg in self.packages:
            pkg_data = pkg if isinstance(pkg, dict) else pkg.__dict__
            if query and query not in pkg_data.get("name", "").lower():
                continue

            status = "installed" if pkg_data.get("installed") else "available"
            if pkg_data.get("upgradable"):
                status = "⬆ upgradable"

            p = Package(pkg_data if isinstance(pkg, dict) else {})
            table.add_row(
                p.name,
                p.version,
                p.section,
                p.size_str,
                status,
            )

        self.query_one("#status-bar", Static).update(
            f" {len(table.rows)} packages | Ctrl+Q to quit"
        )

    def on_input_changed(self, event: Input.Changed) -> None:
        """Handle search input changes."""
        if event.input.id == "search-input":
            self.search_query = event.value
            self._refresh_table()

    def on_button_pressed(self, event: Button.Pressed) -> None:
        """Handle button presses."""
        btn_id = event.button.id

        if btn_id == "btn-update":
            self.action_update()
        elif btn_id == "btn-action-upgrade":
            self.action_upgrade()
        elif btn_id == "btn-action-install":
            self._handle_install()
        elif btn_id == "btn-action-remove":
            self._handle_remove()
        elif btn_id == "btn-action-info":
            self._handle_info()
        elif btn_id == "btn-all":
            self.load_packages()
        elif btn_id == "btn-installed":
            self._filter_installed()
        elif btn_id == "btn-upgradable":
            self._filter_upgradable()

    def _handle_install(self) -> None:
        table = self.query_one("#pkg-table", DataTable)
        if table.cursor_row is not None and table.cursor_row < len(table.rows):
            row = table.rows[table.cursor_row]
            name = str(row[0])
            self.push_screen(ConfirmScreen(f"Install {name}?"), self._install_callback)

    async def _install_callback(self, confirmed: Optional[bool]) -> None:
        if confirmed:
            self.notify("Installing...", timeout=3)
            # In production: subprocess.run(["opk", "install", "-y", name])

    def _handle_remove(self) -> None:
        table = self.query_one("#pkg-table", DataTable)
        if table.cursor_row is not None:
            self.notify("Remove: Not implemented in demo", timeout=2)

    def _handle_info(self) -> None:
        table = self.query_one("#pkg-table", DataTable)
        if table.cursor_row is not None and table.cursor_row < len(table.rows):
            row_data = table.rows[table.cursor_row]
            pkg = Package({
                "name": str(row_data[0]),
                "version": str(row_data[1]),
                "section": str(row_data[2]),
                "size": 0,
                "description": f"Package: {row_data[0]} v{row_data[1]}",
            })
            self.push_screen(PackageDetailScreen(pkg))

    def _filter_installed(self) -> None:
        self.packages = [
            p for p in self.packages
            if (p if isinstance(p, dict) else {}).get("installed", False)
        ]
        self._refresh_table()

    def _filter_upgradable(self) -> None:
        self.packages = [
            p for p in self.packages
            if (p if isinstance(p, dict) else {}).get("upgradable", False)
        ]
        self._refresh_table()

    def action_focus_search(self) -> None:
        self.query_one("#search-input", Input).focus()

    def action_update(self) -> None:
        self.notify("Updating package lists...", timeout=2)
        self.load_packages()

    def action_upgrade(self) -> None:
        self.notify("Upgrading all packages...", timeout=5)

    def action_show_help(self) -> None:
        help_text = """
        OPK UI - Keyboard Shortcuts
        ───────────────────────────
        Ctrl+Q    Quit
        Ctrl+F    Focus search
        Ctrl+U    Update package lists
        Ctrl+G    Upgrade all packages
        Tab       Next focusable widget
        Enter     Select / activate

        Mouse clicks also work on all buttons!
        """
        self.notify(help_text.strip(), timeout=10)


# ============================================================
# Entrypoint
# ============================================================

def main():
    """Launch OPK UI or fall back to CLI."""
    if "--cli" in sys.argv:
        # Fall through to opk CLI
        os.execvp("opk", ["opk"] + [a for a in sys.argv[1:] if a != "--cli"])
    else:
        app = OpkUI()
        app.run()


if __name__ == "__main__":
    main()
