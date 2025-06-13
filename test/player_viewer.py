import pygame
import socket
import game_pb2
import threading

# === Config ===
WINDOW_WIDTH = 800
WINDOW_HEIGHT = 600
UDP_PORT = 9999

# === Pygame setup ===
pygame.init()
screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.RESIZABLE)
pygame.display.set_caption("Live Player Viewer")
font = pygame.font.SysFont("Arial", 16)

# player_id: (prev_x, prev_y, target_x, target_y)
players = {}
lock = threading.Lock()
max_x_seen = 1
max_y_seen = 1

interp_t = 0.0
interp_speed = 0.1  # Controls smoothness

def lerp(a, b, t):
    return a + (b - a) * t

def udp_listener():
    global max_x_seen, max_y_seen, interp_t
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", UDP_PORT))
    while True:
        data, _ = sock.recvfrom(4096)
        pkt = game_pb2.Packet()
        if pkt.ParseFromString(data) and pkt.HasField("state_packet"):
            with lock:
                current_ids = set()
                for p in pkt.state_packet.players:
                    current_ids.add(p.id)
                    if p.id in players:
                        _, _, old_x, old_y = players[p.id]
                        players[p.id] = (old_x, old_y, p.x, p.y)
                    else:
                        players[p.id] = (p.x, p.y, p.x, p.y)
                    max_x_seen = max(max_x_seen, p.x)
                    max_y_seen = max(max_y_seen, p.y)

                # 🔥 Remove players not in current packet
                stale_ids = set(players.keys()) - current_ids
                for pid in stale_ids:
                    del players[pid]

                interp_t = 0.0  # reset interpolation progress


threading.Thread(target=udp_listener, daemon=True).start()

clock = pygame.time.Clock()
running = True

while running:
    WINDOW_WIDTH, WINDOW_HEIGHT = screen.get_size()
    screen.fill((25, 25, 25))

    with lock:
        interp_t = min(interp_t + interp_speed, 1.0)
        for pid, (prev_x, prev_y, target_x, target_y) in players.items():
            x = lerp(prev_x, target_x, interp_t)
            y = lerp(prev_y, target_y, interp_t)

            if max_x_seen > 0 and max_y_seen > 0:
                px = int((x / max_x_seen) * (WINDOW_WIDTH - 20))
                py = int((y / max_y_seen) * (WINDOW_HEIGHT - 20))
                pygame.draw.circle(screen, (0, 200, 0), (px, py), 8)
                screen.blit(font.render(str(pid), True, (255, 255, 255)), (px + 10, py))

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    pygame.display.flip()
    clock.tick(30)

pygame.quit()
