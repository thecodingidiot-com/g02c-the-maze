#!/bin/bash
# g02c — The Maze / test.sh
#
# Builds the real game, then checks the logic deterministically: map.c,
# player.c and camera.c are compiled and linked with libtci ALONE — no
# -lSDL2, no -lSDL2_image, no display. They include SDL2 headers for
# Uint8 and the scancode constants, but they call no SDL function, which
# is what makes a headless tester possible at all.
#
# Copy this file and fixtures/ into your working directory, build with
# 'make re', then run:
#
#   bash test.sh

set -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FIXTURES="${SCRIPT_DIR}/fixtures"

if [[ ! -t 1 ]]; then
    C_GREEN=""; C_RED=""; C_BOLD=""; C_RESET=""
else
    C_GREEN="\033[0;32m"; C_RED="\033[0;31m"; C_BOLD="\033[1m"; C_RESET="\033[0m"
fi

pass_count=0
fail_count=0
WORK_DIR=$(mktemp -d)
trap 'rm -rf "$WORK_DIR"' EXIT

hr() { echo "────────────────────────────────────────────────────────────────"; }
pass() { printf "  ${C_GREEN}PASS${C_RESET}  %s\n" "$1"; pass_count=$((pass_count + 1)); }
fail() {
    printf "  ${C_RED}FAIL${C_RESET}  %s\n" "$1"
    [[ -n "${2:-}" ]] && echo "        $2"
    fail_count=$((fail_count + 1))
}

hr; echo "  g02c — The Maze / test.sh"; hr

# ── build ────────────────────────────────────────────────────────────────────

echo "Building..."
build_log=$(make re 2>&1)
if [[ "$?" -ne 0 ]]; then
    fail "build succeeds" "$build_log"
    exit 1
fi
pass "build succeeds"

if echo "$build_log" | grep -qi "warning"; then
    fail "build produces no warnings" "$(echo "$build_log" | grep -i warning)"
else
    pass "build produces no warnings"
fi

[[ -x ./maze ]] && pass "maze binary exists" || fail "maze binary exists"

# ── the SDL-free logic tester ────────────────────────────────────────────────

if [[ ! -f "${FIXTURES}/maze1.txt" ]]; then
    fail "fixtures/maze1.txt found" "keep the g02c-the-maze clone alongside your working directory"
    exit 1
fi
cp "${FIXTURES}/maze1.txt" "$WORK_DIR/maze1.txt"

# a map that is not a whole number of rooms, for the loader to refuse
{ echo "10 5"; for _ in $(seq 5); do echo "##########"; done; } > "$WORK_DIR/ragged.txt"

cat > "$WORK_DIR/test_logic.c" <<'TESTC'
#include <stdio.h>
#include "camera.h"
#include "map.h"
#include "player.h"

static int  g_pass = 0;
static int  g_fail = 0;

static void check(char const *label, int got, int want)
{
    if (got == want) {
        printf("PASS  %s (got %d)\n", label, got);
        g_pass++;
    } else {
        printf("FAIL  %s (got %d, want %d)\n", label, got, want);
        g_fail++;
    }
}

int main(void)
{
    t_map       map;
    t_map       ragged;
    t_player    p;
    t_camera    cam;
    float       before;

    check("maze1.txt loads", map_load(&map, "maze1.txt"), 1);
    check("map is 75 tiles wide", map.width, 75);
    check("map is 57 tiles tall", map.height, 57);
    check("that is 3 rooms across", map_rooms_x(&map), 3);
    check("and 3 rooms down", map_rooms_y(&map), 3);

    /* A map that is not a whole number of rooms would leave a strip the
     * camera can never rest on, so the loader has to refuse it. */
    check("a map that is not whole rooms is refused",
        map_load(&ragged, "ragged.txt"), 0);

    check("the border is solid", map_is_solid(&map, 0, 0), 1);
    check("the start cell is not solid",
        map_is_solid(&map, map.start_x / TILE_SIZE, map.start_y / TILE_SIZE), 0);

    /* The camera holds a room, not a pixel offset. */
    player_init(&p, (float)map.start_x, (float)map.start_y);
    camera_init(&cam, &p);
    check("camera starts in the room holding the player, x", cam.room_x, 0);
    check("camera starts in the room holding the player, y", cam.room_y, 0);
    check("a room origin is room-aligned, x", camera_origin_x(&cam) % (ROOM_W * TILE_SIZE), 0);
    check("a room origin is room-aligned, y", camera_origin_y(&cam) % (ROOM_H * TILE_SIZE), 0);

    /* Moving inside a room must NOT report a change... */
    p.x += TILE_SIZE;
    check("moving within a room does not change the room",
        camera_update(&cam, &p), 0);

    /* ...and crossing into the next one must, exactly once. */
    p.x = (float)((ROOM_W + 2) * TILE_SIZE);
    check("crossing a boundary reports the change once",
        camera_update(&cam, &p), 1);
    check("and lands in the next room", cam.room_x, 1);
    check("a second look at the same position reports nothing",
        camera_update(&cam, &p), 0);

    /* Walls stop movement. Put the player against the map's own border
     * and push into it: the position must not change. */
    {
        Uint8 keys[SDL_NUM_SCANCODES];
        int   i = 0;

        while (i < SDL_NUM_SCANCODES)
            keys[i++] = 0;
        player_init(&p, (float)TILE_SIZE, (float)(2 * TILE_SIZE));
        keys[SDL_SCANCODE_LEFT] = 1;
        before = p.x;
        player_handle_input(&p, keys, &map);
        player_handle_input(&p, keys, &map);
        check("a wall stops movement into it", p.x == before, 1);

        /* Sliding: pressing into that same wall AND downward should still
         * move on the free axis. This is what separating the two axes
         * buys — a corner does not stop you dead. */
        keys[SDL_SCANCODE_DOWN] = 1;
        before = p.y;
        player_handle_input(&p, keys, &map);
        check("pressing into a wall still slides along it", p.y > before, 1);
    }

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return (g_fail > 0);
}
TESTC

logic_log=$(gcc -Wall -Wextra -I . -I libtci -o "$WORK_DIR/test_logic" \
    "$WORK_DIR/test_logic.c" map.c player.c camera.c \
    libtci/libtci.a libtci/libtciutil.a 2>&1)
if [[ "$?" -ne 0 ]]; then
    fail "logic tester builds with no SDL2 linkage" "$logic_log"
    exit 1
fi
pass "logic tester builds with no SDL2 linkage"
[[ -z "$logic_log" ]] && pass "logic tester builds with no warnings" \
    || fail "logic tester builds with no warnings" "$logic_log"

echo
echo "Running the logic tester..."
logic_out=$(cd "$WORK_DIR" && ./test_logic)
logic_status=$?
echo "$logic_out" | grep -E "^PASS|^FAIL" | while read -r line; do echo "  $line"; done
pass_count=$((pass_count + $(echo "$logic_out" | grep -c "^PASS")))
fail_count=$((fail_count + $(echo "$logic_out" | grep -c "^FAIL")))
[[ "$logic_status" -ne 0 ]] && fail "all logic assertions pass" "see failures above"

# ── headless smoke test ──────────────────────────────────────────────────────

echo
echo "Running maze headless (2s)..."
SDL_VIDEODRIVER=dummy timeout 2 ./maze "${FIXTURES}/maze1.txt"
rc=$?
if [[ "$rc" -eq 124 ]]; then
    pass "maze runs its event loop for 2s without crashing"
else
    fail "maze runs its event loop for 2s without crashing" "exit code: $rc"
fi

# ── leak report ─────────────────────────────────────────────────────────────────
#
# Runs one representative invocation under valgrind and REPORTS what it finds.
# It never changes the pass/fail count. A leak is something to look at, not a
# reason to refuse your work — but you should see it, because a program that
# leaks is a program that will eventually be killed by the machine it runs on.
#
# Leaks are split by whose code lost the memory. A loss record whose stack
# names one of your own .c files is yours. One that lives entirely inside
# SDL, Mesa or glibc is not, and there is nothing for you to fix there.

leak_report() {
    local label="$1"; shift
    local log="${WORK_DIR:-/tmp}/leaks.$$.log"
    local mine=0 theirs=0 rec frames

    if ! command -v valgrind >/dev/null 2>&1; then
        printf "  ${C_BOLD}NOTE${C_RESET}  %s: valgrind is not installed, skipping\n" "$label"
        return 0
    fi

    valgrind --leak-check=full --show-leak-kinds=definite,indirect \
             --error-exitcode=0 --log-file="$log" "$@" >/dev/null 2>&1

    if [[ ! -s "$log" ]]; then
        printf "  ${C_BOLD}NOTE${C_RESET}  %s: valgrind produced no output\n" "$label"
        return 0
    fi

    # Split the log into loss records and ask, of each, whether any frame
    # points at a source file sitting in this directory.
    while IFS= read -r rec; do
        frames=$(sed -n "${rec}"',/^==[0-9]*== *$/p' "$log")
        # Every record carries valgrind's own malloc frame; that is not yours.
        # A frame is yours only if it names a source file sitting right here.
        local f owned=0
        for f in $(grep -oE '\(([A-Za-z0-9_-]+\.c):[0-9]+\)' <<<"$frames" \
                   | tr -d '()' | cut -d: -f1 | sort -u); do
            [[ "$f" == vg_replace_malloc.c ]] && continue
            [[ -f "$f" ]] && owned=1
        done
        if (( owned )); then
            mine=$((mine + 1))
            if (( mine == 1 )); then
                printf "  ${C_RED}LEAK${C_RESET}  %s — memory lost by your code:\n" "$label"
            fi
            grep -E 'bytes in [0-9,]+ blocks are (definitely|indirectly)' <<<"$frames" \
                | sed 's/^==[0-9]*== /        /'
            grep -oE '\(([A-Za-z0-9_-]+\.c:[0-9]+)\)' <<<"$frames" \
                | grep -v vg_replace_malloc | head -3 | tr -d '()' \
                | sed 's/^/          at /'
        else
            theirs=$((theirs + 1))
        fi
    done < <(grep -nE 'bytes in [0-9,]+ blocks are (definitely|indirectly) lost' "$log" | cut -d: -f1)

    if (( mine == 0 )); then
        printf "  ${C_GREEN}OK${C_RESET}    %s — no memory lost by your code" "$label"
        if (( theirs > 0 )); then
            printf ' (%d leak(s) inside libraries you did not write)' "$theirs"
        fi
        printf '\n'
    else
        printf '        this does not fail the tester — fix it anyway\n'
    fi
    rm -f "$log"
    return 0
}

# The graphical chapters run until you quit them, and a program killed
# mid-loop reports everything it has not freed yet as "lost" -- which would be
# a lie. So this starts a virtual display, lets the program run, sends it a
# 'q', and measures the clean exit.
leak_report_gui() {
    local label="$1"; shift
    if ! command -v valgrind >/dev/null 2>&1; then
        printf "  ${C_BOLD}NOTE${C_RESET}  %s: valgrind is not installed, skipping\n" "$label"
        return 0
    fi
    if ! command -v xvfb-run >/dev/null 2>&1 || ! command -v xte >/dev/null 2>&1; then
        printf "  ${C_BOLD}NOTE${C_RESET}  %s: needs xvfb-run and xte for a clean exit, skipping\n" "$label"
        return 0
    fi
    printf "  ${C_BOLD}....${C_RESET}  %s: running under valgrind, this takes a minute\n" "$label"
    local inner="${WORK_DIR:-/tmp}/leak_gui.$$.sh"
    {
        echo "C_GREEN=\"${C_GREEN}\"; C_RED=\"${C_RED}\"; C_BOLD=\"${C_BOLD}\"; C_RESET=\"${C_RESET}\""
        echo "WORK_DIR=\"${WORK_DIR:-/tmp}\""
        declare -f leak_report
        echo '( sleep 12; xte "key q" 2>/dev/null; sleep 5; xte "key q" 2>/dev/null ) &'
        printf 'leak_report %q' "$label"
        printf ' %q' "$@"
        printf '\n'
    } > "$inner"
    timeout 240 xvfb-run -a bash "$inner"
    local rc=$?
    rm -f "$inner"
    if (( rc == 124 )); then
        printf "  ${C_BOLD}NOTE${C_RESET}  %s: the program never exited, so there is nothing honest to measure\n" "$label"
        printf "        (a program killed mid-loop reports everything it holds as lost)\n"
    fi
    return 0
}

echo
leak_report_gui "maze" ./maze "${FIXTURES}/maze1.txt"

echo
hr
printf "  ${C_BOLD}%d passed, %d failed${C_RESET}\n" "$pass_count" "$fail_count"
hr
[[ "$fail_count" -gt 0 ]] && exit 1
exit 0
