import socket
import time
import argparse
import threading
import re

SERVER_IP = "127.0.0.1"
SERVER_PORT = 9000
INTERVAL = 0.1  # 100ms
TICK_PATTERN = re.compile(r'TICK=(\d+)')

def stress_client(client_num, duration):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(0.2)
    server_addr = (SERVER_IP, SERVER_PORT)

    # --- Step 1: HELLO handshake ---
    sock.sendto(b"HELLO", server_addr)
    try:
        data, _ = sock.recvfrom(1024)
        if not data.startswith(b"WELCOME:"):
            print(f"[Client {client_num}] Invalid welcome")
            return
        client_id = int(data.decode().split(":")[1])
    except socket.timeout:
        print(f"[Client {client_num}] No welcome")
        return

    x, y = 0, 0
    start_time = time.time()
    end_time = start_time + duration

    tick_seen = set()
    first_tick = None
    last_tick = None
    updates_sent = 0
    game_started = threading.Event()
    lock = threading.Lock()

    def receiver():
        nonlocal first_tick, last_tick
        while time.time() < end_time:
            try:
                data, _ = sock.recvfrom(4096)
                decoded = data.decode()
                if "GAME:STARTED" in decoded:
                    game_started.set()
                    match = TICK_PATTERN.search(decoded)
                    if match:
                        tick = int(match.group(1))
                        with lock:
                            tick_seen.add(tick)
                            if first_tick is None:
                                first_tick = tick
                            last_tick = tick
            except socket.timeout:
                continue

    recv_thread = threading.Thread(target=receiver)
    recv_thread.start()

    while time.time() < end_time:
        if game_started.is_set():
            msg = f"UPDATE:{client_id}:{x}:{y}"
            x += 1
            y += 1
            updates_sent += 1
        else:
            msg = f"PING:{client_id}"
        sock.sendto(msg.encode(), server_addr)
        time.sleep(INTERVAL)

    recv_thread.join()
    sock.close()

    # --- Metrics ---
    with lock:
        if first_tick is not None and last_tick is not None:
            expected = last_tick - first_tick + 1
            received = len(tick_seen)
            loss = 100 * (expected - received) / expected if expected > 0 else 0
        else:
            expected = received = loss = 0

    print(f"[Client {client_num}] Updates Sent: {updates_sent}, Ticks Expected: {expected}, Received: {received}, Loss: {loss:.2f}%")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--clients", type=int, default=10, help="Number of clients")
    parser.add_argument("--duration", type=int, default=10, help="Duration of test in seconds")
    args = parser.parse_args()

    threads = []
    for i in range(args.clients):
        t = threading.Thread(target=stress_client, args=(i + 1, args.duration))
        t.start()
        threads.append(t)
        time.sleep(0.01)  # slight stagger to avoid overwhelming the server

    for t in threads:
        t.join()
