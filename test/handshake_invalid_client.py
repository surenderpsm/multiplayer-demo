import socket
import time

SERVER_IP = "127.0.0.1"
SERVER_PORT = 9000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# 🔧 Spoofed values (you can change client_id to an invalid or valid one)
client_id = 1  # Change to something registered to test mismatch
x, y = 100, 200

while True:
    msg = f"UPDATE:{client_id}:{x}:{y}"
    sock.sendto(msg.encode(), (SERVER_IP, SERVER_PORT))
    print(f"[SENT] {msg}")
    time.sleep(0.1)

    # Optionally listen for broadcast
    try:
        sock.settimeout(0.1)
        data, _ = sock.recvfrom(1024)
        print(f"[RECV] {data.decode()}")
    except socket.timeout:
        continue
