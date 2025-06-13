import socket
import time
import argparse
import threading
import re
import os
import csv
import game_pb2  # Assuming game_pb2 is the module for the PBF game server

SERVER_IP = "127.0.0.1"
SERVER_PORT = 9000
INTERVAL = 0.1  # 100ms

import random

def simulate_movement(x, y, canvas_width=1500, canvas_height=1500, step=2):
    dx = random.choice([-step, -20, 0, 20, step])
    dy = random.choice([-step, -20, 0, 20, step])
    new_x = max(0, min(x + dx, canvas_width))
    new_y = max(0, min(y + dy, canvas_height))
    return new_x, new_y


def stress_client(client_num, duration, results, packet_sizes):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(0.2)
    server_addr = (SERVER_IP, SERVER_PORT)

    # Send Hello (Protobuf)
    hello = game_pb2.Packet()
    hello.hello.SetInParent()
    sock.sendto(hello.SerializeToString(), server_addr)

    try:
        data, _ = sock.recvfrom(1024)
        welcome = game_pb2.Packet()
        if not welcome.ParseFromString(data) or welcome.WhichOneof("payload") != "welcome":
            print(f"[Client {client_num}] Invalid welcome")
            return
        client_id = welcome.welcome.id
    except socket.timeout:
        print(f"[Client {client_num}] No welcome")
        return

    x = random.randint(0, 1500)
    y = random.randint(0, 1500)
    
    start_time = time.time()
    end_time = start_time + duration

    tick_seen = set()
    first_tick = None
    last_tick = None
    updates_sent = 0
    game_started = threading.Event()
    lock = threading.Lock()
    total_packet_size = 0
    packets_counted = 0

    def receiver():
        nonlocal first_tick, last_tick, total_packet_size, packets_counted
        while time.time() < end_time:
            try:
                data, _ = sock.recvfrom(4096)
                packet = game_pb2.Packet()
                if packet.ParseFromString(data) and packet.WhichOneof("payload") == "state_packet":
                    sp = packet.state_packet
                    total_packet_size += len(data)
                    packets_counted += 1
                    if sp.state == game_pb2.GameState.STARTED:
                        game_started.set()
                        tick = sp.tick
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
        out_packet = game_pb2.Packet()
        if game_started.is_set():
            upd = out_packet.client_update
            upd.id = client_id
            upd.x = x
            upd.y = y
            x, y = simulate_movement(x, y)
            updates_sent += 1
        else:
            ping = out_packet.ping
            ping.id = client_id

        sock.sendto(out_packet.SerializeToString(), server_addr)
        time.sleep(INTERVAL)

    recv_thread.join()
    sock.close()

    with lock:
        if first_tick is not None and last_tick is not None:
            expected = last_tick - first_tick + 1
            received = len(tick_seen)
            loss = 100 * (expected - received) / expected if expected > 0 else 0
        else:
            expected = received = loss = 0

    avg_packet_size = total_packet_size / packets_counted if packets_counted > 0 else 0

    print(f"[Client {client_num}] Updates Sent: {updates_sent}, Ticks Expected: {expected}, Received: {received}, Loss: {loss:.2f}%")

    results.append((client_num, updates_sent, expected, received, loss))
    packet_sizes.append((client_num, avg_packet_size))

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--clients", type=int, default=10, help="Number of clients")
    parser.add_argument("--duration", type=int, default=10, help="Duration of test in seconds")
    args = parser.parse_args()

    threads = []
    results = []
    packet_sizes = []

    if not os.path.exists("logs"):
        os.makedirs("logs")

    for i in range(args.clients):
        t = threading.Thread(target=stress_client, args=(i + 1, args.duration, results, packet_sizes))
        t.start()
        threads.append(t)
        time.sleep(0.01)

    for t in threads:
        t.join()

    with open(f"logs/loss_clients_{args.clients}.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["ClientID", "UpdatesSent", "TicksExpected", "TicksReceived", "LossPercentage"])
        writer.writerows(results)

    with open(f"logs/packet_sizes_{args.clients}.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["ClientID", "AvgPacketSizeBytes"])
        writer.writerows(packet_sizes)
