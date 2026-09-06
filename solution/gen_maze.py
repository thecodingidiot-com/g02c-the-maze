#!/usr/bin/env python3
"""Generate the maze fixture for g02c.

The map is a grid of ROOMS, each exactly one screen: 25x19 tiles at 32px
is 800x608, which is the window. Rooms are joined by doorways carved
through their shared walls, and the room graph is a spanning tree plus a
couple of extra links, so the maze is connected but not a single corridor.

Deterministic: a fixed seed, so the committed fixture is stable and its
diff is reviewable. Verified solvable by BFS before it is written.
"""
import random
from collections import deque

ROOM_W, ROOM_H = 25, 19
ROOMS_X, ROOMS_Y = 3, 3
W, H = ROOM_W * ROOMS_X, ROOM_H * ROOMS_Y
SEED = 20260906

WALL, FLOOR = "#", "."


def room_graph(rng):
    """Spanning tree over the room grid, plus two extra edges for loops."""
    nodes = [(x, y) for y in range(ROOMS_Y) for x in range(ROOMS_X)]
    unvisited, visited = set(nodes), set()
    cur = (0, 0)
    visited.add(cur)
    unvisited.remove(cur)
    edges = set()
    while unvisited:
        nbrs = [(cur[0] + dx, cur[1] + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))]
        nbrs = [n for n in nbrs if n in unvisited]
        if not nbrs:
            cur = rng.choice(sorted(visited))
            continue
        nxt = rng.choice(sorted(nbrs))
        edges.add((min(cur, nxt), max(cur, nxt)))
        visited.add(nxt)
        unvisited.remove(nxt)
        cur = nxt
    all_adj = set()
    for (x, y) in nodes:
        for dx, dy in ((1, 0), (0, 1)):
            n = (x + dx, y + dy)
            if n in nodes:
                all_adj.add(((x, y), n))
    extra = sorted(all_adj - edges)
    rng.shuffle(extra)
    edges.update(extra[:2])
    return edges


def build(rng):
    g = [[FLOOR] * W for _ in range(H)]
    # every room gets a full wall border; doorways are carved after
    for ry in range(ROOMS_Y):
        for rx in range(ROOMS_X):
            ox, oy = rx * ROOM_W, ry * ROOM_H
            for x in range(ROOM_W):
                g[oy][ox + x] = WALL
                g[oy + ROOM_H - 1][ox + x] = WALL
            for y in range(ROOM_H):
                g[oy + y][ox] = WALL
                g[oy + y][ox + ROOM_W - 1] = WALL
    for a, b in sorted(room_graph(rng)):
        (ax, ay), (bx, by) = a, b
        if ax == bx:            # vertical neighbours -> horizontal doorway
            wy = max(ay, by) * ROOM_H
            cx = ax * ROOM_W + ROOM_W // 2
            for d in (-1, 0, 1):
                g[wy][cx + d] = FLOOR
                g[wy - 1][cx + d] = FLOOR
        else:                   # horizontal neighbours -> vertical doorway
            wx = max(ax, bx) * ROOM_W
            cy = ay * ROOM_H + ROOM_H // 2
            for d in (-1, 0, 1):
                g[cy + d][wx] = FLOOR
                g[cy + d][wx - 1] = FLOOR
    # interior obstacles: short segments, kept away from walls and doorways
    for ry in range(ROOMS_Y):
        for rx in range(ROOMS_X):
            ox, oy = rx * ROOM_W, ry * ROOM_H
            for _ in range(rng.randint(3, 5)):
                horiz = rng.random() < 0.5
                ln = rng.randint(3, 6)
                sx = rng.randint(ox + 3, ox + ROOM_W - 4 - (ln if horiz else 0))
                sy = rng.randint(oy + 3, oy + ROOM_H - 4 - (0 if horiz else ln))
                for i in range(ln):
                    x, y = (sx + i, sy) if horiz else (sx, sy + i)
                    g[y][x] = WALL
    return g


def reachable(g, start, goal):
    q, seen = deque([start]), {start}
    while q:
        x, y = q.popleft()
        if (x, y) == goal:
            return True
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (x + dx, y + dy)
            if 0 <= n[0] < W and 0 <= n[1] < H and n not in seen and g[n[1]][n[0]] != WALL:
                seen.add(n)
                q.append(n)
    return False


def main():
    start = (ROOM_W // 2, ROOM_H // 2)
    goal = (2 * ROOM_W + ROOM_W // 2, 2 * ROOM_H + ROOM_H // 2)
    for attempt in range(200):
        rng = random.Random(SEED + attempt)
        g = build(rng)
        g[start[1]][start[0]] = FLOOR
        g[goal[1]][goal[0]] = FLOOR
        if reachable(g, start, goal):
            g[start[1]][start[0]] = "S"
            g[goal[1]][goal[0]] = "E"
            with open("../fixtures/maze1.txt", "w") as f:
                f.write(f"{W} {H}\n")
                f.write("\n".join("".join(r) for r in g) + "\n")
            print(f"wrote {W}x{H} ({ROOMS_X}x{ROOMS_Y} rooms), seed offset {attempt}, solvable")
            return
    raise SystemExit("no solvable maze generated")


main()
