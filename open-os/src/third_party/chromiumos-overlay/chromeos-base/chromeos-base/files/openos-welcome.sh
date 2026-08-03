#!/bin/bash
# OPENOS Welcome — TUI Animation (first boot) / Static Banner (subsequent)
# First boot: typewriter-style character animation.
# Subsequent boots (--static): prints the same art instantly.
# Runs from /etc/profile.d/ and Crosh.
#
# Copyright 2026 OCS (Open Code Studio)
# License: GPL-3.0

FIRST_BOOT_FLAG="/var/lib/openos/first-boot-done"
MODE_STATIC=false

# Parse arguments
if [ "${1:-}" = "--static" ]; then
    MODE_STATIC=true
fi

# Always require a real terminal
[ -t 0 ] || exit 0
[ "$TERM" = "dumb" ] && exit 0

# Animated mode only runs on first boot
if ! $MODE_STATIC; then
    [ -f "$FIRST_BOOT_FLAG" ] && exit 0
fi

# ── Animation helpers ──────────────────────────────────────────────

# Color palette
C_RESET="\033[0m"
C_BOLD="\033[1m"
C_DIM="\033[2m"
C_RED="\033[31m"
C_GREEN="\033[32m"
C_YELLOW="\033[33m"
C_BLUE="\033[34m"
C_MAGENTA="\033[35m"
C_CYAN="\033[36m"
C_WHITE="\033[37m"
C_BRIGHT_GREEN="\033[92m"
C_BRIGHT_CYAN="\033[96m"
C_OPENOS_GREEN="\033[38;2;74;162;111m"

# Column width
COLS=$(tput cols 2>/dev/null || echo 80)
ROWS=$(tput lines 2>/dev/null || echo 24)
WIDTH=$((COLS > 80 ? 80 : COLS))
PAD=$(( (COLS - WIDTH) / 2 ))
[ "$PAD" -lt 0 ] && PAD=0

hide_cursor()  { printf "\033[?25l"; }
show_cursor()  { printf "\033[?25h"; }
move_to()      { printf "\033[%d;%dH" "$1" "$2"; }
clear_screen() { printf "\033[2J\033[H"; }

# Draw a character at position with delay
type_char() {
    local row="$1" col="$2" char="$3" color="$4" delay="$5"
    move_to "$row" "$col"
    printf "${color}%s${C_RESET}" "$char"
    sleep "$delay"
}

# Typewriter line (character-by-character)
type_line() {
    local row="$1" col="$2" text="$3" color="$4" delay="${5:-0.03}"
    local i=0
    while [ $i -lt ${#text} ]; do
        local char="${text:$i:1}"
        type_char "$row" $((col + i)) "$char" "$color" "$delay"
        i=$((i + 1))
    done
}

# ── Progress bar ───────────────────────────────────────────────────

draw_progress() {
    local row="$1" col="$2" width="$3" pct="$4"
    local filled=$(( width * pct / 100 ))
    local empty=$(( width - filled ))
    move_to "$row" "$col"
    printf "${C_DIM}[${C_RESET}"
    printf "${C_BRIGHT_GREEN}"
    local i
    for i in $(seq 1 $filled); do printf "="; done
    if [ "$pct" -lt 100 ]; then
        printf ">"
        empty=$((empty - 1))
    fi
    printf "${C_RESET}${C_DIM}"
    for i in $(seq 1 $empty); do printf " "; done
    printf "]${C_RESET} %d%%" "$pct"
}

animate_progress() {
    local row="$1" col="$2" width="$3"
    local pct=0
    while [ $pct -le 100 ]; do
        draw_progress "$row" "$col" "$width" "$pct"
        sleep 0.02
        pct=$((pct + 2))
    done
}

# ── Large ASCII art for OPENOS ─────────────────────────────────────
# Each letter is typed character-by-character (typewriter effect).
# All 6 letters (O P E N O S) on a single line, 7 rows high.

# Per-letter ASCII art, 7 rows each (shorter letters padded with spaces)
O1_LINES=(
    " ██████╗ "
    "██╔═══██╗"
    "██║   ██║"
    "██║   ██║"
    "██║   ██║"
    "██╚═══██╝"
    " ╚█████╔╝ "
)
P_LINES=(
    "██████╗ "
    "██╔══██╗"
    "██████╔╝"
    "██╔═══╝ "
    "██║     "
    "██║     "
    "╚═╝     "
)
E_LINES=(
    "███████╗"
    "██╔════╝"
    "█████╗  "
    "██╔══╝  "
    "███████╗"
    "╚══════╝"
    "         "
)
N_LINES=(
    "███╗   ██╗"
    "████╗  ██║"
    "██╔██╗ ██║"
    "██║╚██╗██║"
    "██║ ╚████║"
    "██║  ╚███║"
    "╚═╝   ╚══╝"
)
O2_LINES=(
    " █████╗ "
    "██╔══██╗"
    "██║  ██║"
    "██║  ██║"
    "╚█████╔╝"
    " ╚════╝ "
    "         "
)
S_LINES=(
    "███████╗"
    "██╔════╝"
    "███████╗"
    "╚════██║"
    "███████║"
    "╚══════╝"
    "         "
)

# O = green (#4AA26F), all other letters = white
O1_COLORS=("$C_OPENOS_GREEN" "$C_OPENOS_GREEN" "$C_OPENOS_GREEN" "$C_OPENOS_GREEN" "$C_OPENOS_GREEN" "$C_OPENOS_GREEN" "$C_OPENOS_GREEN")
P_COLORS=( "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE")
E_COLORS=( "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE")
N_COLORS=( "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE")
O2_COLORS=("$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE")
S_COLORS=( "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE" "$C_WHITE")

# Type one row of the big OPENOS text, all 6 letters concatenated side-by-side
type_big_row() {
    local row="$1" col="$2" idx="$3"
    local text="${O1_LINES[$idx]}${P_LINES[$idx]}${E_LINES[$idx]}${N_LINES[$idx]}${O2_LINES[$idx]}${S_LINES[$idx]}"
    local i=0 len=${#text}
    local pos
    while [ $i -lt $len ]; do
        local char="${text:$i:1}"
        pos=$((col + i))
        # Determine which letter this character belongs to
        if   [ $i -lt 9 ];  then type_char "$row" "$pos" "$char" "${O1_COLORS[$idx]}" 0.012
        elif [ $i -lt 17 ]; then type_char "$row" "$pos" "$char" "${P_COLORS[$idx]}" 0.012
        elif [ $i -lt 25 ]; then type_char "$row" "$pos" "$char" "${E_COLORS[$idx]}" 0.012
        elif [ $i -lt 35 ]; then type_char "$row" "$pos" "$char" "${N_COLORS[$idx]}" 0.012
        elif [ $i -lt 44 ]; then type_char "$row" "$pos" "$char" "${O2_COLORS[$idx]}" 0.012
        else                       type_char "$row" "$pos" "$char" "${S_COLORS[$idx]}" 0.012
        fi
        i=$((i + 1))
    done
}

draw_big_openos() {
    local base_row="$1"
    local pad_col=$(( PAD + 4 ))
    local i
    for i in 0 1 2 3 4 5 6; do
        type_big_row $((base_row + i)) "$pad_col" "$i"
    done
}

# ── Static (instant) banner ─────────────────────────────────────────
# Prints the same content as the animation, but all at once with printf.
# Used on subsequent boots so the art is always visible, just no animation.

print_banner_static() {
    local pad_col=$(( PAD + 4 ))
    local i text

    # Clear screen and print OPENOS big ASCII
    clear_screen
    for i in 0 1 2 3 4 5 6; do
        printf "%${pad_col}s" ""
        printf "${O1_COLORS[$i]}%s${C_RESET}" "${O1_LINES[$i]}"
        printf "${P_COLORS[$i]}%s${C_RESET}"  "${P_LINES[$i]}"
        printf "${E_COLORS[$i]}%s${C_RESET}"  "${E_LINES[$i]}"
        printf "${N_COLORS[$i]}%s${C_RESET}"  "${N_LINES[$i]}"
        printf "${O2_COLORS[$i]}%s${C_RESET}" "${O2_LINES[$i]}"
        printf "${S_COLORS[$i]}%s${C_RESET}\n" "${S_LINES[$i]}"
    done

    # Tagline
    printf "\n%$((pad_col + 6))s${C_DIM}%s${C_RESET}\n" "" "The open-source operating system."

    # Version line
    local ver="OPENOS 1.0.0  ·  Linux $(uname -r | cut -d'-' -f1)  ·  $(uname -m)"
    printf "%$((pad_col + 3))s${C_CYAN}%s${C_RESET}\n" "" "$ver"

    # Progress bar (100%)
    local bar_width=70
    printf "%$((pad_col + 3))s${C_DIM}[${C_BRIGHT_GREEN}" ""
    local j
    for j in $(seq 1 $bar_width); do printf "="; done
    printf "${C_DIM}]${C_RESET} 100%%\n"

    # Welcome message
    printf "%$((pad_col + 3))s${C_BOLD}${C_BRIGHT_CYAN}Welcome to OPENOS!${C_RESET}\n" ""
    printf "%$((pad_col + 3))s${C_DIM}Developer mode is always ON.  Type 'help' for available commands.${C_RESET}\n" ""
    printf "%$((pad_col + 3))s${C_DIM}Press Ctrl+Alt+T to open Crosh  ·  Ctrl+Alt+F2 for VT2 console${C_RESET}\n" ""
}

# ── Main animation sequence ────────────────────────────────────────

run_animation() {
    trap 'show_cursor; printf "\033[?7h"; exit' INT TERM

    hide_cursor
    clear_screen

    # ── Phase 1: Big ASCII "OPENOS" typewriter ──
    draw_big_openos 2

    sleep 0.3

    # ── Phase 2: Tagline ──
    local tag_row=$((2 + 7 + 1))
    type_line $tag_row $((PAD + 6)) "The open-source operating system." "$C_DIM" 0.035

    # ── Phase 3: Version line ──
    local ver_row=$((tag_row + 2))
    type_line $ver_row $((PAD+3)) "OPENOS 1.0.0  ·  Linux $(uname -r | cut -d'-' -f1)  ·  $(uname -m)" "$C_CYAN" 0.02

    # ── Phase 4: Progress bar ──
    local bar_row=$((ver_row + 1))
    animate_progress "$bar_row" $((PAD+3)) 70

    sleep 0.5

    # ── Phase 5: Blinking prompt ──
    local prompt_row=$((bar_row + 2))
    move_to "$prompt_row" $((PAD+3))
    printf "${C_GREEN}chronos@openos${C_RESET}:${C_BLUE}~${C_RESET}\$ ${C_BOLD}${C_WHITE}_${C_RESET}"
    sleep 0.4
    local flash_count=4
    while [ $flash_count -gt 0 ]; do
        move_to "$prompt_row" $((PAD+3))
        printf "${C_GREEN}chronos@openos${C_RESET}:${C_BLUE}~${C_RESET}\$ ${C_BOLD}${C_WHITE}_${C_RESET}"
        sleep 0.3
        move_to "$prompt_row" $((PAD+3))
        printf "${C_GREEN}chronos@openos${C_RESET}:${C_BLUE}~${C_RESET}\$  "
        sleep 0.3
        flash_count=$((flash_count - 1))
    done

    # ── Phase 6: Welcome message ──
    move_to "$((prompt_row + 2))" $((PAD+3))
    printf "${C_BOLD}${C_BRIGHT_CYAN}Welcome to OPENOS!${C_RESET}"
    move_to "$((prompt_row + 3))" $((PAD+3))
    printf "${C_DIM}Developer mode is always ON.  Type 'help' for available commands.${C_RESET}"
    move_to "$((prompt_row + 4))" $((PAD+3))
    printf "${C_DIM}Press Ctrl+Alt+T to open Crosh  ·  Ctrl+Alt+F2 for VT2 console${C_RESET}"

    sleep 1.5

    # ── Phase 7: Cleanup ──
    move_to "$((prompt_row + 6))" 1
    printf "${C_RESET}"
    show_cursor
    printf "\033[?7h"
}

# ── Entry point ────────────────────────────────────────────────────

if $MODE_STATIC; then
    # Static mode: instant print, no animation, no flag modification
    print_banner_static
else
    # Animated mode: first-boot typewriter animation
    run_animation

    # Mark first boot as done (create flag file)
    mkdir -p /var/lib/openos 2>/dev/null
    touch "$FIRST_BOOT_FLAG" 2>/dev/null || true
fi
